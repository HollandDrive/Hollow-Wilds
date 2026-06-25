// Copyright Kepler Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/HWGameplayAbility.h"
#include "Abilities/HWElement.h"
#include "GA_ElementalBolt.generated.h"

class AHWProjectile;
class UHWElementProfile;

/**
 * UGA_ElementalBolt
 *
 * Spawns an elemental bolt in the witch's aim direction, tuned by her currently
 * selected element. Looks up tuning from assigned UHWElementProfile assets if any,
 * otherwise falls back to hardcoded defaults mirroring the Unity ElementProfiles.
 * Spends Essence (fizzles if too low). Mirrors WandCaster.FireBolt.
 */
UCLASS()
class HOLLOWWILDS_API UGA_ElementalBolt : public UHWGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_ElementalBolt();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	/** Projectile class to spawn for the bolt. */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Bolt")
	TSubclassOf<AHWProjectile> ProjectileClass;

	/**
	 * Optional per-element tuning. If a profile for the active element is present here
	 * it overrides the hardcoded defaults. Leave empty to use the Unity-matching defaults.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Bolt")
	TArray<TObjectPtr<UHWElementProfile>> ElementProfiles;

	/** Muzzle offset relative to the avatar (x=forward, y=right, z=up). Mirrors muzzleOffset. */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Bolt")
	FVector MuzzleOffset = FVector(80.f, 0.f, 140.f);

	/** Bolt travel speed (cm/s). */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Bolt")
	float BoltSpeed = 6000.f;

	/** Bolt lifetime if it hits nothing (seconds). */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Bolt")
	float BoltLifeSeconds = 3.f;

private:
	/** Per-element tuning resolved for a cast (mirrors ElementProfiles.Get). */
	struct FBoltTuning
	{
		float Damage = 0.f;
		float StatusMagnitude = 0.f;
		float StatusDuration = 0.f;
		float Cost = 0.f;
	};

	/** Resolve tuning for an element: assigned profile first, else hardcoded default. */
	FBoltTuning ResolveTuning(EHWElement Element) const;
};
