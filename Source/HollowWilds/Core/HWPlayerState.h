// Copyright Kepler Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "HWPlayerState.generated.h"

class UHWAbilitySystemComponent;
class UHWAttributeSet;

/**
 * Player state for Hollow Wilds.
 * Owns the player's AbilitySystemComponent and AttributeSet (standard for
 * player-controlled GAS setups so they persist across pawn respawns).
 */
UCLASS()
class HOLLOWWILDS_API AHWPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AHWPlayerState();

	//~ Begin IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End IAbilitySystemInterface

	/** Strongly-typed accessor for the Hollow Wilds ability system component. */
	UHWAbilitySystemComponent* GetHWAbilitySystemComponent() const;

	/** Accessor for the player's attribute set. */
	UHWAttributeSet* GetAttributeSet() const;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Hollow Wilds|Abilities")
	TObjectPtr<UHWAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, Category = "Hollow Wilds|Abilities")
	TObjectPtr<UHWAttributeSet> AttributeSet;
};
