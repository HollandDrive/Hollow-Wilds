// Copyright Kepler Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Abilities/HWElement.h"
#include "HWWitchCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UAbilitySystemComponent;
class UHWAbilitySystemComponent;
class UHWAttributeSet;
class UGameplayAbility;
struct FInputActionValue;

/**
 * AHWWitchCharacter
 *
 * Third-person witch character mirroring the Unity WitchController + ThirdPersonCamera.
 * Horizon-style movement (walk/jog/sprint tiers, dodge-roll, slide, aim-aware steering)
 * driven by Enhanced Input, with the player's GAS AbilitySystemComponent resolved from
 * the AHWPlayerState.
 *
 * Movement and combat mapping (from the README control table):
 *   WASD move, Hold Shift sprint, Hold Alt walk, L-Ctrl/double-tap dodge-roll,
 *   Shift+C slide, Space jump, Hold RMB aim, LMB fire/melee, 1/2/3 element select, Mouse look.
 *
 * Combat actions (fire, dodge, slide) are intended to route into GAS abilities. Those
 * handlers are currently lightweight, clearly-marked stubs (see TODOs) until the matching
 * GameplayAbilities exist.
 */
UCLASS()
class HOLLOWWILDS_API AHWWitchCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AHWWitchCharacter();

	//~ Begin AActor interface
	virtual void BeginPlay() override;
	virtual void PawnClientRestart() override;
	//~ End AActor interface

	//~ Begin APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	//~ End APawn interface

	//~ Begin ACharacter interface
	virtual void OnRep_PlayerState() override;
	//~ End ACharacter interface

	//~ Begin IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End IAbilitySystemInterface

	/** Typed accessor for the project's ASC. */
	UFUNCTION(BlueprintPure, Category = "Abilities")
	UHWAbilitySystemComponent* GetHWAbilitySystemComponent() const { return HWAbilitySystemComponent; }

	/** Typed accessor for the project's AttributeSet. */
	UFUNCTION(BlueprintPure, Category = "Abilities")
	UHWAttributeSet* GetHWAttributeSet() const { return AttributeSet; }

	/** Currently selected casting element (Fire/Ice/Wind). */
	UFUNCTION(BlueprintPure, Category = "Abilities")
	EHWElement GetCurrentElement() const { return CurrentElement; }

	/** True while the player holds aim (over-the-shoulder). */
	UFUNCTION(BlueprintPure, Category = "Camera")
	bool IsAiming() const { return bIsAiming; }

protected:
	//~ Components ------------------------------------------------------------

	/** Spring arm holding the follow camera; uses pawn control rotation so the camera orbits with look input. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** Third-person follow camera attached to the spring arm. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	//~ Movement tuning -------------------------------------------------------

	/** Walk tier max speed (Hold Alt). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speeds")
	float WalkSpeed = 150.f;

	/** Jog tier max speed (default). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speeds")
	float JogSpeed = 400.f;

	/** Sprint tier max speed (Hold Shift). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speeds")
	float SprintSpeed = 650.f;

	/** Strafe max speed while aiming (brisk strafe, not a crawl). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speeds")
	float AimMoveSpeed = 300.f;

	//~ Camera tuning ---------------------------------------------------------

	/** Spring arm length in the hip (default) camera. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float HipArmLength = 350.f;

	/** Spring arm length while aiming (pulled in over the shoulder). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float AimArmLength = 150.f;

	/** Socket offset in the hip (default) camera. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FVector HipSocketOffset = FVector(0.f, 0.f, 60.f);

	/** Socket offset while aiming (over the right shoulder). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FVector AimSocketOffset = FVector(0.f, 60.f, 70.f);

	//~ Enhanced Input --------------------------------------------------------

	/** Mapping context added to the local player on possession/restart. */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Look;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Jump;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Sprint;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Walk;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Dodge;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Slide;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Aim;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Fire;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Element1;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Element2;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Element3;

	//~ Abilities -------------------------------------------------------------

	/** Abilities granted to the ASC when this character is possessed (server-authoritative). */
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	//~ Input handlers --------------------------------------------------------

	/** Camera-relative movement (real). */
	void Move(const FInputActionValue& Value);

	/** Mouse / right-stick look (real). */
	void Look(const FInputActionValue& Value);

	/** Sprint tier (real: speed change; sprint is locked out while aiming). */
	void StartSprint(const FInputActionValue& Value);
	void StopSprint(const FInputActionValue& Value);

	/** Walk tier (real: speed change). */
	void StartWalk(const FInputActionValue& Value);
	void StopWalk(const FInputActionValue& Value);

	/** Aim state toggle (real: camera + rotation mode change). */
	void StartAim(const FInputActionValue& Value);
	void StopAim(const FInputActionValue& Value);

	/** Element selection (real). */
	void SelectElement1(const FInputActionValue& Value);
	void SelectElement2(const FInputActionValue& Value);
	void SelectElement3(const FInputActionValue& Value);

	/** Combat / evade handlers — STUBS pending GAS abilities (see TODOs in .cpp). */
	void DoDodge(const FInputActionValue& Value);
	void DoSlide(const FInputActionValue& Value);
	void DoFire(const FInputActionValue& Value);

	/** Apply the current movement tier (walk/jog/sprint/aim) to the movement component. */
	void ApplyMovementTier();

private:
	/** Resolve the ASC + AttributeSet from the player state and init actor info. */
	void InitAbilityActorInfoFromPlayerState();

	/** Add the default mapping context to the local player subsystem, if available. */
	void AddDefaultMappingContext();

	//~ GAS cache -------------------------------------------------------------

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UHWAbilitySystemComponent> HWAbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UHWAttributeSet> AttributeSet;

	//~ State -----------------------------------------------------------------

	/** True while sprint is held. */
	bool bSprintHeld = false;

	/** True while walk is held. */
	bool bWalkHeld = false;

	/** True while aiming (over-the-shoulder). */
	bool bIsAiming = false;

	/** Guards one-time server-side ability granting. */
	bool bAbilitiesGranted = false;

	/** Selected casting element. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
	EHWElement CurrentElement = EHWElement::Fire;
};
