// Copyright Kepler Interactive. All Rights Reserved.

#include "Core/HWPlayerState.h"
#include "Abilities/HWAbilitySystemComponent.h"
#include "Attributes/HWAttributeSet.h"

AHWPlayerState::AHWPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UHWAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UHWAttributeSet>(TEXT("AttributeSet"));

	// PlayerState is replicated frequently so attribute/ability changes feel responsive.
	SetNetUpdateFrequency(100.f);
}

UAbilitySystemComponent* AHWPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UHWAbilitySystemComponent* AHWPlayerState::GetHWAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UHWAttributeSet* AHWPlayerState::GetAttributeSet() const
{
	return AttributeSet;
}
