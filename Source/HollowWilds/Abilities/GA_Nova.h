// Copyright Kepler Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/HWGameplayAbility.h"
#include "GA_Nova.generated.h"

/**
 * UGA_Nova (V)
 *
 * A radial elemental burst. Mirrors the Unity AbilityCaster.Nova: overlaps a sphere of
 * NovaRadius around the witch and, for every damageable actor caught, applies direct
 * damage and a Wind-style lift (Status.Lift) crowd-control. It also applies the witch's
 * current element status to each target.
 *
 * Cost (Essence) and cooldown (Cooldown.Nova) are applied directly in C++ so no
 * Blueprint GameplayEffect assets are required.
 *
 * Unity defaults: radius 5 m (~500 uu), damage 28, lift 1.1 s, cooldown 6 s, cost 30.
 */
UCLASS()
class HOLLOWWILDS_API UGA_Nova : public UHWGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Nova();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	/** Radius of the nova burst, in Unreal units (5 m). */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Nova")
	float NovaRadius = 500.f;

	/** Direct damage dealt to every actor caught in the burst. */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Nova")
	float NovaDamage = 28.f;

	/** How long enemies are lifted/floated by the burst (seconds). */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Nova")
	float NovaLiftDuration = 1.1f;

	/** Magnitude passed with the lift status. */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Nova")
	float NovaLiftMagnitude = 1.f;

	/** Cooldown after a nova (seconds). */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Nova")
	float NovaCooldown = 6.f;

	/** Essence cost per nova. */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Nova")
	float NovaCost = 30.f;
};
