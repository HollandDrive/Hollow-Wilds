// Copyright Kepler Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/HWGameplayAbility.h"
#include "GA_Barrier.generated.h"

/**
 * UGA_Barrier (R)
 *
 * A timed absorb shield. Mirrors the Unity AbilityCaster.Barrier: tops up the
 * avatar's Absorb attribute pool (incoming-damage routing through Absorb is handled
 * in UHWAttributeSet) and grants a barrier State tag for the duration. After
 * BarrierDuration the granted Absorb is drained back out and the tag removed.
 *
 * Cost (Essence) and cooldown (Cooldown.Barrier) are applied directly in C++ so no
 * Blueprint GameplayEffect assets are required.
 *
 * Unity defaults: duration 4 s, absorb pool 40, cooldown 9 s, cost 22.
 */
UCLASS()
class HOLLOWWILDS_API UGA_Barrier : public UHWGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Barrier();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:
	/** How long the shield lasts (seconds). */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Barrier")
	float BarrierDuration = 4.f;

	/** Flat Absorb pool added to the avatar that soaks damage before Health. */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Barrier")
	float BarrierAbsorb = 40.f;

	/** Cooldown measured from when the shield is raised (seconds). */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Barrier")
	float BarrierCooldown = 9.f;

	/** Essence cost to raise the shield. */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Barrier")
	float BarrierCost = 22.f;

private:
	/** Timer that ends the barrier window. */
	FTimerHandle BarrierTimerHandle;

	/** How much Absorb this activation actually granted (drained back on end). */
	float GrantedAbsorb = 0.f;

	/** Called when the barrier window elapses. */
	void OnBarrierFinished();
};
