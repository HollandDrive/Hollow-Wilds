// Copyright Kepler Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "HWAbilitySystemComponent.generated.h"

/**
 * Hollow Wilds ability system component.
 * Minimal subclass for now; exists so we can extend ability/input handling later.
 */
UCLASS()
class HOLLOWWILDS_API UHWAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	/** Hook called once the actor info has been set on this ASC. */
	virtual void AbilityActorInfoSet();

	/** Called when an ability input tag is pressed. Stub for future input handling. */
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
};
