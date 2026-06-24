// Copyright Kepler Interactive. All Rights Reserved.

#include "Abilities/HWGameplayAbility.h"

UHWGameplayAbility::UHWGameplayAbility()
{
	// One instance per owning actor; standard for most player-facing abilities.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}
