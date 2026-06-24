// Copyright Kepler Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "HWGameplayAbility.generated.h"

/** Controls how a Hollow Wilds gameplay ability is activated. */
UENUM(BlueprintType)
enum class EHWAbilityActivationPolicy : uint8
{
	/** Activate when the bound input is triggered (pressed). */
	OnInputTriggered,

	/** Try to activate continuously while the bound input is held. */
	WhileInputActive,

	/** Activate once when the owning avatar spawns / the ability is granted. */
	OnSpawn
};

/**
 * Base class for all Hollow Wilds gameplay abilities.
 * Activation logic is left to subclasses.
 */
UCLASS()
class HOLLOWWILDS_API UHWGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UHWGameplayAbility();

	/** How this ability is activated relative to its bound input. */
	EHWAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }

protected:
	/** Determines how this ability is activated. */
	UPROPERTY(EditDefaultsOnly, Category = "Hollow Wilds|Ability")
	EHWAbilityActivationPolicy ActivationPolicy = EHWAbilityActivationPolicy::OnInputTriggered;
};
