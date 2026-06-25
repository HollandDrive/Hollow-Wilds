// Copyright Kepler Interactive. All Rights Reserved.

#include "Abilities/GA_Blink.h"

#include "AbilitySystemComponent.h"
#include "Attributes/HWAttributeSet.h"
#include "Core/HWGameplayTags.h"
#include "Combat/HWCombatStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

UGA_Blink::UGA_Blink()
{
	// Tag this ability so it can be granted / triggered by Ability.Blink.
	FGameplayTagContainer Tags;
	Tags.AddTag(HWGameplayTags::Ability_Blink);
	SetAssetTags(Tags);
}

void UGA_Blink::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicate*/ true, /*bWasCancelled*/ true);
		return;
	}

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!Character || !ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Pay the essence cost directly (no BP cost GE needed). No-op-friendly: clamp at 0.
	if (const UHWAttributeSet* AttrSet = Cast<UHWAttributeSet>(
			ASC->GetAttributeSet(UHWAttributeSet::StaticClass())))
	{
		const float NewEssence = FMath::Max(0.f, AttrSet->GetEssence() - BlinkCost);
		ASC->SetNumericAttributeBase(UHWAttributeSet::GetEssenceAttribute(), NewEssence);
	}

	// Start a manual cooldown by adding the cooldown tag and removing it after BlinkCooldown.
	// (Self-contained: avoids requiring a Blueprint cooldown GameplayEffect.)
	ASC->AddLooseGameplayTag(HWGameplayTags::Cooldown_Blink);
	if (UWorld* World = GetWorld())
	{
		FTimerHandle CooldownHandle;
		FTimerDelegate CooldownDel = FTimerDelegate::CreateWeakLambda(this, [ASC]()
		{
			if (ASC)
			{
				ASC->RemoveLooseGameplayTag(HWGameplayTags::Cooldown_Blink);
			}
		});
		World->GetTimerManager().SetTimer(CooldownHandle, CooldownDel, FMath::Max(0.01f, BlinkCooldown), false);
	}

	// Grant the invulnerability window for the dash (plus a little padding).
	ASC->AddLooseGameplayTag(HWGameplayTags::State_Invulnerable);

	// Blink along the witch's flattened forward vector.
	FVector Dir = Character->GetActorForwardVector();
	Dir.Z = 0.f;
	if (!Dir.Normalize())
	{
		Dir = FVector::ForwardVector;
	}

	// Constant velocity over the (short) duration covers BlinkDistance. LaunchCharacter
	// uses the movement sweep so we slide to a stop at walls instead of tunnelling.
	const float Speed = BlinkDistance / FMath::Max(BlinkDuration, 0.0001f);
	Character->LaunchCharacter(Dir * Speed, /*bXYOverride*/ true, /*bZOverride*/ false);

	// End the dash + invulnerability after the window.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			BlinkTimerHandle, this, &UGA_Blink::OnBlinkFinished,
			FMath::Max(0.01f, BlinkDuration + InvulnerabilityPadding), false);
	}
	else
	{
		OnBlinkFinished();
	}
}

void UGA_Blink::OnBlinkFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Blink::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// Always clear the invulnerability tag and any pending timer when the ability ends.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BlinkTimerHandle);
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(HWGameplayTags::State_Invulnerable);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
