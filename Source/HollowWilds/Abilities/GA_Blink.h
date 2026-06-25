// Copyright Kepler Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/HWGameplayAbility.h"
#include "GA_Blink.generated.h"

/**
 * UGA_Blink (E)
 *
 * A short forward dash/teleport with a brief invulnerability window. Mirrors the
 * Unity AbilityCaster.Blink: launches the witch along her flattened forward vector
 * over a short duration so CharacterMovement sweeps her to a stop at walls (no
 * tunnelling), grants State.Invulnerable for the dash window, then ends.
 *
 * Cost (Essence) and cooldown (Cooldown.Blink) are applied directly in C++ so no
 * Blueprint GameplayEffect assets are required.
 *
 * Unity defaults: distance 5.5 m (~550 uu), duration 0.12 s, cooldown 2.5 s, cost 12.
 */
UCLASS()
class HOLLOWWILDS_API UGA_Blink : public UHWGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Blink();

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
	/** How far the blink travels, in Unreal units (5.5 m). */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Blink")
	float BlinkDistance = 550.f;

	/** How long the blink slide takes (seconds). Short = snappy. */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Blink")
	float BlinkDuration = 0.12f;

	/** Cooldown after a blink (seconds). */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Blink")
	float BlinkCooldown = 2.5f;

	/** Essence cost per blink. */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Blink")
	float BlinkCost = 12.f;

	/** Extra invulnerability granted past the dash end so the recovery frames are safe. */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Blink")
	float InvulnerabilityPadding = 0.04f;

private:
	/** Timer that ends the dash + invulnerability window. */
	FTimerHandle BlinkTimerHandle;

	/** Called when the dash window elapses. */
	void OnBlinkFinished();
};
