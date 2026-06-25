// Copyright Kepler Interactive. All Rights Reserved.

#include "Abilities/GA_Beam.h"

#include "AbilitySystemComponent.h"
#include "Attributes/HWAttributeSet.h"
#include "Abilities/HWElement.h"
#include "Character/HWWitchCharacter.h"
#include "Combat/HWCombatStatics.h"
#include "Core/HWGameplayTags.h"
#include "Engine/World.h"
#include "TimerManager.h"

UGA_Beam::UGA_Beam()
{
	FGameplayTagContainer Tags;
	Tags.AddTag(HWGameplayTags::Ability_Beam);
	SetAssetTags(Tags);

	// Replicate input release so the channel can be stopped on the server.
	bReplicateInputDirectly = true;
}

void UGA_Beam::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* Avatar = GetAvatarActorFromActorInfo();
	UWorld* World = GetWorld();
	if (!Avatar || !World)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Fire one tick immediately, then loop on the interval until released / out of essence.
	BeamTick();

	// BeamTick() may have already ended the ability (no essence). Only start the loop
	// if we're still active.
	if (IsActive())
	{
		World->GetTimerManager().SetTimer(
			BeamTickHandle, this, &UGA_Beam::BeamTick,
			FMath::Max(0.01f, BeamTickInterval), /*bLoop*/ true);
	}
}

void UGA_Beam::BeamTick()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	UWorld* World = GetWorld();
	if (!Avatar || !ASC || !World)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	const UHWAttributeSet* AttrSet = Cast<UHWAttributeSet>(
		ASC->GetAttributeSet(UHWAttributeSet::StaticClass()));
	if (!AttrSet)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	// Drain essence for this tick; fizzle the channel if we can't afford it.
	const float CurrentEssence = AttrSet->GetEssence();
	if (CurrentEssence < BeamCostPerTick)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	ASC->SetNumericAttributeBase(
		UHWAttributeSet::GetEssenceAttribute(),
		FMath::Max(0.f, CurrentEssence - BeamCostPerTick));

	// Resolve the witch's current element (defaults to Fire if not a witch).
	EHWElement Element = EHWElement::Fire;
	if (const AHWWitchCharacter* Witch = Cast<AHWWitchCharacter>(Avatar))
	{
		Element = Witch->GetCurrentElement();
	}

	// Sphere-trace forward from an eye/wand height along the avatar's facing.
	const FVector Origin = Avatar->GetActorLocation() + FVector(0.f, 0.f, BeamOriginHeight);
	const FVector Dir = Avatar->GetActorForwardVector().GetSafeNormal2D();
	const FVector End = Origin + Dir * BeamRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(HWBeamTrace), /*bTraceComplex*/ false);
	Params.AddIgnoredActor(Avatar);

	TArray<FHitResult> Hits;
	World->SweepMultiByChannel(
		Hits, Origin, End, FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(BeamRadius), Params);

	// Damage + status each unique target once per tick.
	TSet<AActor*> Done;
	for (const FHitResult& Hit : Hits)
	{
		AActor* Target = Hit.GetActor();
		if (!Target || Target == Avatar || Done.Contains(Target))
		{
			continue;
		}

		// Only affect things that have an ASC (i.e. damageable GAS actors).
		if (UHWCombatStatics::GetASC(Target) == nullptr)
		{
			continue;
		}

		Done.Add(Target);
		UHWCombatStatics::ApplyDamage(Avatar, Target, BeamDamagePerTick);
		UHWCombatStatics::ApplyElementStatus(
			Avatar, Target, Element, BeamStatusMagnitude, BeamStatusDuration);
	}
}

void UGA_Beam::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	// Releasing the channel input ends the beam.
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_Beam::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BeamTickHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
