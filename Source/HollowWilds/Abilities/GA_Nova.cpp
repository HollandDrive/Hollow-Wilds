// Copyright Kepler Interactive. All Rights Reserved.

#include "Abilities/GA_Nova.h"

#include "AbilitySystemComponent.h"
#include "Attributes/HWAttributeSet.h"
#include "Abilities/HWElement.h"
#include "Combat/HWCombatStatics.h"
#include "Core/HWGameplayTags.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "TimerManager.h"

UGA_Nova::UGA_Nova()
{
	FGameplayTagContainer Tags;
	Tags.AddTag(HWGameplayTags::Ability_Nova);
	SetAssetTags(Tags);
}

void UGA_Nova::ActivateAbility(
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
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	UWorld* World = GetWorld();
	if (!Avatar || !ASC || !World)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Pay the essence cost directly.
	if (const UHWAttributeSet* AttrSet = Cast<UHWAttributeSet>(
			ASC->GetAttributeSet(UHWAttributeSet::StaticClass())))
	{
		const float NewEssence = FMath::Max(0.f, AttrSet->GetEssence() - NovaCost);
		ASC->SetNumericAttributeBase(UHWAttributeSet::GetEssenceAttribute(), NewEssence);
	}

	// Manual cooldown: add the cooldown tag now, remove it after NovaCooldown.
	ASC->AddLooseGameplayTag(HWGameplayTags::Cooldown_Nova);
	{
		FTimerHandle CooldownHandle;
		FTimerDelegate CooldownDel = FTimerDelegate::CreateWeakLambda(this, [ASC]()
		{
			if (ASC)
			{
				ASC->RemoveLooseGameplayTag(HWGameplayTags::Cooldown_Nova);
			}
		});
		World->GetTimerManager().SetTimer(CooldownHandle, CooldownDel, FMath::Max(0.01f, NovaCooldown), false);
	}

	// Radial overlap around the witch.
	const FVector Center = Avatar->GetActorLocation();

	FCollisionQueryParams Params(SCENE_QUERY_STAT(HWNovaOverlap), /*bTraceComplex*/ false);
	Params.AddIgnoredActor(Avatar);

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByChannel(
		Overlaps, Center, FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(NovaRadius), Params);

	// Damage + lift every unique damageable actor caught in the burst.
	TSet<AActor*> Done;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Target = Overlap.GetActor();
		if (!Target || Target == Avatar || Done.Contains(Target))
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UHWCombatStatics::GetASC(Target);
		if (TargetASC == nullptr)
		{
			continue;
		}

		Done.Add(Target);

		UHWCombatStatics::ApplyDamage(Avatar, Target, NovaDamage);

		// Wind-style crowd control: route the lift through the Wind element status path
		// (Wind maps to Status.Lift), and also grant Status.Lift directly so the CC tag
		// is guaranteed present for the lift window.
		UHWCombatStatics::ApplyElementStatus(
			Avatar, Target, EHWElement::Wind, NovaLiftMagnitude, NovaLiftDuration);
		TargetASC->AddLooseGameplayTag(HWGameplayTags::Status_Lift);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
