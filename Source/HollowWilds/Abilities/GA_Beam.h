// Copyright Kepler Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/HWGameplayAbility.h"
#include "GA_Beam.generated.h"

/**
 * UGA_Beam (Q, channelled)
 *
 * A held, channelled elemental ray. Mirrors the Unity AbilityCaster.Beam: while the
 * input is active, every BeamTickInterval seconds it sphere-traces forward up to
 * BeamRange and, on each enemy hit, applies direct damage and the witch's current
 * element status (via UHWCombatStatics). Each tick drains Essence; the beam ends when
 * the input is released, the ability is cancelled, or Essence runs out.
 *
 * Channelling is driven by a looping timer (kept simple and correct). No cooldown:
 * Beam is gated by its essence drain, matching the Unity design.
 *
 * Unity defaults: range 22 m (~2200 uu), trace radius 0.6 m (~60 uu), tick 0.12 s,
 * damage 6/tick, cost 2.5/tick.
 */
UCLASS()
class HOLLOWWILDS_API UGA_Beam : public UHWGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Beam();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	/** Input released -> stop channelling. */
	virtual void InputReleased(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:
	/** Max length of the beam, in Unreal units (22 m). */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Beam")
	float BeamRange = 2200.f;

	/** Sphere-trace radius (swathe width), in Unreal units (0.6 m). */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Beam")
	float BeamRadius = 60.f;

	/** Seconds between damage ticks while channelling. */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Beam")
	float BeamTickInterval = 0.12f;

	/** Direct damage per tick. */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Beam")
	float BeamDamagePerTick = 6.f;

	/** Essence drained per tick. The beam dies when essence runs out. */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Beam")
	float BeamCostPerTick = 2.5f;

	/** Element status magnitude applied per tick. */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Beam")
	float BeamStatusMagnitude = 4.f;

	/** Element status duration applied per tick (brief, refreshed each tick). */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Beam")
	float BeamStatusDuration = 0.4f;

	/** Vertical offset of the beam origin from the avatar location (eye/wand height). */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Beam")
	float BeamOriginHeight = 90.f;

private:
	/** Looping channel timer driving the ticks. */
	FTimerHandle BeamTickHandle;

	/** Performs one channel tick: pay essence, trace, damage + status hits. */
	void BeamTick();
};
