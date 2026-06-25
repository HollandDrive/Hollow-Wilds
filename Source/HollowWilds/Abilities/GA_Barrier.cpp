// Copyright Kepler Interactive. All Rights Reserved.

#include "Abilities/GA_Barrier.h"

#include "AbilitySystemComponent.h"
#include "Attributes/HWAttributeSet.h"
#include "Core/HWGameplayTags.h"
#include "TimerManager.h"

UGA_Barrier::UGA_Barrier()
{
	FGameplayTagContainer Tags;
	Tags.AddTag(HWGameplayTags::Ability_Barrier);
	SetAssetTags(Tags);
}

void UGA_Barrier::ActivateAbility(
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

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	const UHWAttributeSet* AttrSet = ASC
		? Cast<UHWAttributeSet>(ASC->GetAttributeSet(UHWAttributeSet::StaticClass()))
		: nullptr;
	if (!ASC || !AttrSet)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Pay the essence cost directly.
	{
		const float NewEssence = FMath::Max(0.f, AttrSet->GetEssence() - BarrierCost);
		ASC->SetNumericAttributeBase(UHWAttributeSet::GetEssenceAttribute(), NewEssence);
	}

	// Manual cooldown: add the cooldown tag now, remove it after BarrierCooldown.
	ASC->AddLooseGameplayTag(HWGameplayTags::Cooldown_Barrier);
	if (UWorld* World = GetWorld())
	{
		FTimerHandle CooldownHandle;
		FTimerDelegate CooldownDel = FTimerDelegate::CreateWeakLambda(this, [ASC]()
		{
			if (ASC)
			{
				ASC->RemoveLooseGameplayTag(HWGameplayTags::Cooldown_Barrier);
			}
		});
		World->GetTimerManager().SetTimer(CooldownHandle, CooldownDel, FMath::Max(0.01f, BarrierCooldown), false);
	}

	// Raise the shield: top up the Absorb pool. Damage routing through Absorb is
	// already handled by UHWAttributeSet, so we only need to grow the pool here.
	GrantedAbsorb = FMath::Max(0.f, BarrierAbsorb);
	const float NewAbsorb = AttrSet->GetAbsorb() + GrantedAbsorb;
	ASC->SetNumericAttributeBase(UHWAttributeSet::GetAbsorbAttribute(), NewAbsorb);

	// Mark the shield as up. Ability.Barrier doubles as the "barrier active" state
	// marker (no dedicated State.Barrier tag exists in the registry yet), and is
	// granted as a loose tag so other systems can query it while the shield holds.
	ASC->AddLooseGameplayTag(HWGameplayTags::Ability_Barrier);

	// End the shield after its duration.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			BarrierTimerHandle, this, &UGA_Barrier::OnBarrierFinished,
			FMath::Max(0.01f, BarrierDuration), false);
	}
	else
	{
		OnBarrierFinished();
	}
}

void UGA_Barrier::OnBarrierFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Barrier::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BarrierTimerHandle);
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(HWGameplayTags::Ability_Barrier);

		// Drain back whatever of our granted Absorb still remains, so an expired
		// shield doesn't leave a permanent pool. Damage taken meanwhile has already
		// reduced Absorb, so clamp at 0.
		if (GrantedAbsorb > 0.f)
		{
			if (const UHWAttributeSet* AttrSet = Cast<UHWAttributeSet>(
					ASC->GetAttributeSet(UHWAttributeSet::StaticClass())))
			{
				const float Remaining = FMath::Max(0.f, AttrSet->GetAbsorb() - GrantedAbsorb);
				ASC->SetNumericAttributeBase(UHWAttributeSet::GetAbsorbAttribute(), Remaining);
			}
			GrantedAbsorb = 0.f;
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
