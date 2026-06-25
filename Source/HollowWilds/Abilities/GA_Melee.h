// Copyright Kepler Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/HWGameplayAbility.h"
#include "GA_Melee.generated.h"

/**
 * UGA_Melee
 *
 * A short forward staff swing: an overlap sweep in a sphere in front of the
 * avatar that deals direct damage to every enemy it touches (once each).
 * Mirrors WandCaster.SampleMeleeHits / the active hit window.
 */
UCLASS()
class HOLLOWWILDS_API UGA_Melee : public UHWGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Melee();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	/** Direct damage dealt to each enemy struck. Mirrors meleeDamage. */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Melee")
	float Damage = 20.f;

	/** How far in front of the avatar the swing centre sits (cm). Mirrors meleeReach. */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Melee")
	float Reach = 160.f;

	/** Radius of the swing sweep (cm). Mirrors meleeRadius. */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Melee")
	float Radius = 130.f;

	/** Vertical offset of the swing centre so it sweeps at torso height (cm). */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Melee")
	float HeightOffset = 90.f;

	/** Half-angle (degrees) of the forward cone; hits outside this arc are ignored. */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Melee")
	float ConeHalfAngleDegrees = 60.f;
};
