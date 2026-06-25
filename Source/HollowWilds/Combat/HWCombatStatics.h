// Copyright Kepler Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Abilities/HWElement.h"
#include "HWCombatStatics.generated.h"

class UAbilitySystemComponent;

/**
 * UHWCombatStatics
 *
 * Shared, self-contained combat helpers built directly on GAS — no Blueprint
 * GameplayEffect assets required. Abilities and projectiles route their damage
 * and status application through here so the rules live in one place.
 *
 * Mirrors the Unity damage/status pipeline (Health.TakeDamage + EnemyController
 * status methods) on top of UHWAttributeSet:
 *   - ApplyDamage feeds the IncomingDamage meta attribute, which the attribute set
 *     drains through Absorb then Health in PostGameplayEffectExecute.
 *   - ApplyElementStatus builds runtime GameplayEffects in C++ for the per-element
 *     status (Fire/Earth DoT, Ice slow, Wind lift) plus the matching Status.* tag.
 */
UCLASS()
class HOLLOWWILDS_API UHWCombatStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Route Damage into Target's UHWAttributeSet via the IncomingDamage meta attribute.
	 * Self-contained: builds a runtime instant GameplayEffectSpec, no BP asset needed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Hollow Wilds|Combat")
	static void ApplyDamage(AActor* Source, AActor* Target, float Damage);

	/**
	 * Apply an element's signature status to Target for Duration seconds:
	 *   Fire  = burn DoT  (periodic damage of Magnitude/sec) + Status.Burn
	 *   Ice   = slow       (MoveSpeedMultiplier *= Magnitude)  + Status.Slow
	 *   Wind  = lift        (Status.Lift tag only)
	 *   Earth = lava DoT  (periodic damage of Magnitude/sec) + Status.Lava
	 * Builds runtime GameplayEffects in C++; no BP assets required.
	 */
	UFUNCTION(BlueprintCallable, Category = "Hollow Wilds|Combat")
	static void ApplyElementStatus(AActor* Source, AActor* Target, EHWElement Element, float Magnitude, float Duration);

	/** Resolve an actor's AbilitySystemComponent (via IAbilitySystemInterface or a global lookup). */
	static UAbilitySystemComponent* GetASC(AActor* Actor);
};
