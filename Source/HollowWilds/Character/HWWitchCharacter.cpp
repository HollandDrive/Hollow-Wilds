// Copyright Kepler Interactive. All Rights Reserved.

#include "Character/HWWitchCharacter.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"

#include "AbilitySystemComponent.h"

#include "Core/HWPlayerState.h"
#include "Abilities/HWAbilitySystemComponent.h"
#include "Attributes/HWAttributeSet.h"

#include "Abilities/GameplayAbility.h"
#include "Abilities/GA_ElementalBolt.h"
#include "Abilities/GA_Melee.h"
#include "Abilities/GA_Blink.h"
#include "Abilities/GA_Barrier.h"
#include "Abilities/GA_Beam.h"
#include "Abilities/GA_Nova.h"

DEFINE_LOG_CATEGORY_STATIC(LogHWWitch, Log, All);

AHWWitchCharacter::AHWWitchCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// The controller rotates the camera (spring arm), not the pawn body, while moving.
	// Movement orients the body toward the velocity (Horizon-style steering).
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// ---- Camera boom (hip distance, orbits with control rotation) ----
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = HipArmLength; // ~350 for hip
	CameraBoom->SocketOffset = HipSocketOffset;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->bEnableCameraRotationLag = false;

	// ---- Follow camera on the boom's socket ----
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// ---- Movement (Horizon-style: orient to movement, default jog tier) ----
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bOrientRotationToMovement = true;
		Move->RotationRate = FRotator(0.f, 540.f, 0.f);
		Move->bUseControllerDesiredRotation = false;

		Move->MaxWalkSpeed = JogSpeed; // default run
		Move->MaxAcceleration = 2048.f;
		Move->BrakingDecelerationWalking = 2048.f;
		Move->GroundFriction = 8.f;

		Move->JumpZVelocity = 500.f;
		Move->AirControl = 0.35f;
	}

	// Default GAS abilities granted on possession (overridable in BP defaults).
	DefaultAbilities = {
		UGA_ElementalBolt::StaticClass(),
		UGA_Melee::StaticClass(),
		UGA_Blink::StaticClass(),
		UGA_Barrier::StaticClass(),
		UGA_Beam::StaticClass(),
		UGA_Nova::StaticClass()
	};
}

void AHWWitchCharacter::BeginPlay()
{
	Super::BeginPlay();

	AddDefaultMappingContext();
	ApplyMovementTier();
}

void AHWWitchCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	// Ensure the mapping context is (re)added when the pawn is restarted for a local player.
	AddDefaultMappingContext();
}

void AHWWitchCharacter::AddDefaultMappingContext()
{
	if (!DefaultMappingContext)
	{
		UE_LOG(LogHWWitch, Warning, TEXT("AddDefaultMappingContext: DefaultMappingContext is not set on the pawn."));
		return;
	}

	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
				UE_LOG(LogHWWitch, Log, TEXT("AddDefaultMappingContext: added '%s'."), *DefaultMappingContext->GetName());
			}
		}
	}
	else
	{
		UE_LOG(LogHWWitch, Verbose, TEXT("AddDefaultMappingContext: no PlayerController yet (deferred)."));
	}
}

//~ GAS ----------------------------------------------------------------------

void AHWWitchCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Server: the player state exists now, so wire up the ASC.
	InitAbilityActorInfoFromPlayerState();
}

void AHWWitchCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Client: the player state has replicated, so wire up the ASC.
	InitAbilityActorInfoFromPlayerState();
}

void AHWWitchCharacter::InitAbilityActorInfoFromPlayerState()
{
	AHWPlayerState* HWPS = GetPlayerState<AHWPlayerState>();
	if (!HWPS)
	{
		return;
	}

	HWAbilitySystemComponent = HWPS->GetHWAbilitySystemComponent();
	AbilitySystemComponent = HWPS->GetAbilitySystemComponent();
	AttributeSet = HWPS->GetAttributeSet();

	if (HWAbilitySystemComponent)
	{
		// Owner is the player state (owns the ASC + attributes); avatar is this character.
		HWAbilitySystemComponent->InitAbilityActorInfo(HWPS, this);

		// Server grants the default abilities exactly once.
		if (HasAuthority() && !bAbilitiesGranted)
		{
			for (const TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
			{
				if (AbilityClass)
				{
					HWAbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
				}
			}
			bAbilitiesGranted = true;
		}
	}
}

UAbilitySystemComponent* AHWWitchCharacter::GetAbilitySystemComponent() const
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent;
	}

	// Fallback resolve in case an accessor is hit before init (e.g. early UI queries).
	if (const AHWPlayerState* HWPS = GetPlayerState<AHWPlayerState>())
	{
		return HWPS->GetAbilitySystemComponent();
	}

	return nullptr;
}

//~ Input setup --------------------------------------------------------------

void AHWWitchCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC)
	{
		UE_LOG(LogHWWitch, Error,
			TEXT("HWWitchCharacter expects an EnhancedInputComponent. Set the default input class to EnhancedInputComponent in project settings."));
		return;
	}

	// Reliable point to (re)add the mapping context: the controller + local player are
	// valid here for the locally-possessed pawn (BeginPlay can run before possession).
	AddDefaultMappingContext();

	// ---- Movement & look (real) ----
	if (IA_Move)
	{
		EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AHWWitchCharacter::Move);
	}
	if (IA_Look)
	{
		EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AHWWitchCharacter::Look);
	}

	// ---- Jump (real, via ACharacter) ----
	if (IA_Jump)
	{
		EIC->BindAction(IA_Jump, ETriggerEvent::Started, this, &ACharacter::Jump);
		EIC->BindAction(IA_Jump, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}

	// ---- Movement tiers (real) ----
	if (IA_Sprint)
	{
		EIC->BindAction(IA_Sprint, ETriggerEvent::Started, this, &AHWWitchCharacter::StartSprint);
		EIC->BindAction(IA_Sprint, ETriggerEvent::Completed, this, &AHWWitchCharacter::StopSprint);
	}
	if (IA_Walk)
	{
		EIC->BindAction(IA_Walk, ETriggerEvent::Started, this, &AHWWitchCharacter::StartWalk);
		EIC->BindAction(IA_Walk, ETriggerEvent::Completed, this, &AHWWitchCharacter::StopWalk);
	}

	// ---- Aim (real) ----
	if (IA_Aim)
	{
		EIC->BindAction(IA_Aim, ETriggerEvent::Started, this, &AHWWitchCharacter::StartAim);
		EIC->BindAction(IA_Aim, ETriggerEvent::Completed, this, &AHWWitchCharacter::StopAim);
	}

	// ---- Element selection (real) ----
	if (IA_Element1)
	{
		EIC->BindAction(IA_Element1, ETriggerEvent::Started, this, &AHWWitchCharacter::SelectElement1);
	}
	if (IA_Element2)
	{
		EIC->BindAction(IA_Element2, ETriggerEvent::Started, this, &AHWWitchCharacter::SelectElement2);
	}
	if (IA_Element3)
	{
		EIC->BindAction(IA_Element3, ETriggerEvent::Started, this, &AHWWitchCharacter::SelectElement3);
	}

	// ---- Combat / evade (STUBS — route into GAS later) ----
	if (IA_Dodge)
	{
		EIC->BindAction(IA_Dodge, ETriggerEvent::Started, this, &AHWWitchCharacter::DoDodge);
	}
	if (IA_Slide)
	{
		EIC->BindAction(IA_Slide, ETriggerEvent::Started, this, &AHWWitchCharacter::DoSlide);
	}
	if (IA_Fire)
	{
		EIC->BindAction(IA_Fire, ETriggerEvent::Started, this, &AHWWitchCharacter::DoFire);
	}
}

//~ Movement / look (real) ---------------------------------------------------

void AHWWitchCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (!Controller || Axis.IsNearlyZero())
	{
		return;
	}

	// Camera-relative movement: yaw-only basis (matches the Unity flattened cam forward/right).
	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, Axis.Y);
	AddMovementInput(Right, Axis.X);
}

void AHWWitchCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (!Controller)
	{
		return;
	}

	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

//~ Movement tiers (real) ----------------------------------------------------

void AHWWitchCharacter::StartSprint(const FInputActionValue& /*Value*/)
{
	bSprintHeld = true;
	ApplyMovementTier();
}

void AHWWitchCharacter::StopSprint(const FInputActionValue& /*Value*/)
{
	bSprintHeld = false;
	ApplyMovementTier();
}

void AHWWitchCharacter::StartWalk(const FInputActionValue& /*Value*/)
{
	bWalkHeld = true;
	ApplyMovementTier();
}

void AHWWitchCharacter::StopWalk(const FInputActionValue& /*Value*/)
{
	bWalkHeld = false;
	ApplyMovementTier();
}

void AHWWitchCharacter::ApplyMovementTier()
{
	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (!Move)
	{
		return;
	}

	// Priority mirrors the Unity CurrentMoveSpeed():
	//   aiming -> brisk strafe; sprint held -> sprint; walk held -> walk; else jog (default run).
	float TargetSpeed;
	if (bIsAiming)
	{
		TargetSpeed = AimMoveSpeed; // sprint is locked out while aiming
	}
	else if (bSprintHeld)
	{
		TargetSpeed = SprintSpeed;
	}
	else if (bWalkHeld)
	{
		TargetSpeed = WalkSpeed;
	}
	else
	{
		TargetSpeed = JogSpeed;
	}

	Move->MaxWalkSpeed = TargetSpeed;
}

//~ Aim (real) ---------------------------------------------------------------

void AHWWitchCharacter::StartAim(const FInputActionValue& /*Value*/)
{
	bIsAiming = true;

	// Over-the-shoulder: pull the boom in and shift the socket to the right shoulder.
	if (CameraBoom)
	{
		CameraBoom->TargetArmLength = AimArmLength;
		CameraBoom->SocketOffset = AimSocketOffset;
	}

	// Face where the camera looks: the controller drives yaw, body stops auto-orienting to velocity.
	bUseControllerRotationYaw = true;
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bOrientRotationToMovement = false;
	}

	// Sprint is locked out while aiming; re-evaluate the tier (forces the aim strafe speed).
	ApplyMovementTier();
}

void AHWWitchCharacter::StopAim(const FInputActionValue& /*Value*/)
{
	bIsAiming = false;

	if (CameraBoom)
	{
		CameraBoom->TargetArmLength = HipArmLength;
		CameraBoom->SocketOffset = HipSocketOffset;
	}

	bUseControllerRotationYaw = false;
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bOrientRotationToMovement = true;
	}

	ApplyMovementTier();
}

//~ Element selection (real) -------------------------------------------------

void AHWWitchCharacter::SelectElement1(const FInputActionValue& /*Value*/)
{
	CurrentElement = EHWElement::Fire;
	UE_LOG(LogHWWitch, Verbose, TEXT("Element selected: Fire"));
}

void AHWWitchCharacter::SelectElement2(const FInputActionValue& /*Value*/)
{
	CurrentElement = EHWElement::Ice;
	UE_LOG(LogHWWitch, Verbose, TEXT("Element selected: Ice"));
}

void AHWWitchCharacter::SelectElement3(const FInputActionValue& /*Value*/)
{
	CurrentElement = EHWElement::Wind;
	UE_LOG(LogHWWitch, Verbose, TEXT("Element selected: Wind"));
}

//~ Combat / evade (STUBS) ---------------------------------------------------

void AHWWitchCharacter::DoDodge(const FInputActionValue& /*Value*/)
{
	// Blink = a short forward dash with an i-frame window — our dodge.
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->TryActivateAbilityByClass(UGA_Blink::StaticClass());
	}
}

void AHWWitchCharacter::DoSlide(const FInputActionValue& /*Value*/)
{
	// TODO(GAS): Replace this stub with a GameplayAbility activation for the slide
	// (Shift+C), a decelerating burst with i-frames; dodging mid-slide should cancel
	// into a roll (mirrors WitchController.StartSlide).
	UE_LOG(LogHWWitch, Log, TEXT("[STUB] DoSlide — TODO: activate slide GameplayAbility."));
}

void AHWWitchCharacter::DoFire(const FInputActionValue& /*Value*/)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// Aiming -> elemental bolt (uses CurrentElement); from the hip -> staff melee.
	// Mirrors the Unity WandCaster context-sensitive LMB.
	const TSubclassOf<UGameplayAbility> AbilityClass = bIsAiming
		? TSubclassOf<UGameplayAbility>(UGA_ElementalBolt::StaticClass())
		: TSubclassOf<UGameplayAbility>(UGA_Melee::StaticClass());
	AbilitySystemComponent->TryActivateAbilityByClass(AbilityClass);
}
