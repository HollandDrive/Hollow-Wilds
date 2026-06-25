// Copyright Kepler Interactive. All Rights Reserved.

#include "Combat/HWCombatStatics.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

#include "Attributes/HWAttributeSet.h"
#include "Core/HWGameplayTags.h"

UAbilitySystemComponent* UHWCombatStatics::GetASC(AActor* Actor)
{
	if (Actor == nullptr)
	{
		return nullptr;
	}

	// Prefer the interface, fall back to the global accessor (which also checks
	// for a component on the actor) so this works for any GAS-enabled actor.
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
}

void UHWCombatStatics::ApplyDamage(AActor* Source, AActor* Target, float Damage)
{
	if (Damage <= 0.f)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = GetASC(Target);
	if (TargetASC == nullptr)
	{
		return;
	}

	// Build a runtime instant GameplayEffect that adds Damage to the IncomingDamage
	// meta attribute. The attribute set's PostGameplayEffectExecute then drains it
	// through Absorb -> Health. Fully self-contained (no BP GameplayEffect asset).
	UGameplayEffect* DamageGE = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("HWRuntimeDamage")));
	DamageGE->DurationPolicy = EGameplayEffectDurationType::Instant;

	const int32 ModIndex = DamageGE->Modifiers.Num();
	DamageGE->Modifiers.SetNum(ModIndex + 1);
	FGameplayModifierInfo& ModInfo = DamageGE->Modifiers[ModIndex];
	ModInfo.Attribute = UHWAttributeSet::GetIncomingDamageAttribute();
	ModInfo.ModifierOp = EGameplayModOp::Additive;
	ModInfo.ModifierMagnitude = FScalableFloat(Damage);

	// Establish an effect context that credits the source/instigator.
	FGameplayEffectContextHandle Context = TargetASC->MakeEffectContext();
	Context.AddSourceObject(Source);
	if (Source != nullptr)
	{
		Context.AddInstigator(Source, Source);
	}

	FGameplayEffectSpec Spec(DamageGE, Context, 1.f);
	TargetASC->ApplyGameplayEffectSpecToSelf(Spec);
}

void UHWCombatStatics::ApplyElementStatus(AActor* Source, AActor* Target, EHWElement Element, float Magnitude, float Duration)
{
	UAbilitySystemComponent* TargetASC = GetASC(Target);
	if (TargetASC == nullptr || Duration <= 0.f)
	{
		return;
	}

	// Pick the matching Status.* tag and build the per-element runtime effect.
	FGameplayTag StatusTag;
	switch (Element)
	{
	case EHWElement::Fire:  StatusTag = HWGameplayTags::Status_Burn; break;
	case EHWElement::Ice:   StatusTag = HWGameplayTags::Status_Slow; break;
	case EHWElement::Wind:  StatusTag = HWGameplayTags::Status_Lift; break;
	case EHWElement::Earth: StatusTag = HWGameplayTags::Status_Lava; break;
	default: break;
	}

	UGameplayEffect* StatusGE = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("HWRuntimeElementStatus")));
	StatusGE->DurationPolicy = EGameplayEffectDurationType::HasDuration;
	StatusGE->DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(Duration));

	// Grant the Status.* tag for the lifetime of the effect. In UE 5.3+ granted
	// ("owned") tags are configured through the TargetTags gameplay effect component.
	if (StatusTag.IsValid())
	{
		UTargetTagsGameplayEffectComponent& TargetTagsComponent =
			StatusGE->FindOrAddComponent<UTargetTagsGameplayEffectComponent>();

		FInheritedTagContainer TagChanges;
		TagChanges.AddTag(StatusTag);
		TargetTagsComponent.SetAndApplyTargetTagChanges(TagChanges);
	}

	switch (Element)
	{
	case EHWElement::Fire:
	case EHWElement::Earth:
	{
		// Burn / Lava: periodic damage of Magnitude per second into IncomingDamage.
		if (Magnitude > 0.f)
		{
			StatusGE->Period = FScalableFloat(1.f);
			StatusGE->bExecutePeriodicEffectOnApplication = false;

			const int32 ModIndex = StatusGE->Modifiers.Num();
			StatusGE->Modifiers.SetNum(ModIndex + 1);
			FGameplayModifierInfo& ModInfo = StatusGE->Modifiers[ModIndex];
			ModInfo.Attribute = UHWAttributeSet::GetIncomingDamageAttribute();
			ModInfo.ModifierOp = EGameplayModOp::Additive;
			ModInfo.ModifierMagnitude = FScalableFloat(Magnitude);
		}
		break;
	}
	case EHWElement::Ice:
	{
		// Slow: multiply MoveSpeedMultiplier down by Magnitude for the duration.
		// The attribute set clamps MoveSpeedMultiplier to [0,1], so a multiplicative
		// op that lands the value below the default 1.0 is the slow.
		const float SlowMul = (Magnitude > 0.f) ? Magnitude : 1.f;

		const int32 ModIndex = StatusGE->Modifiers.Num();
		StatusGE->Modifiers.SetNum(ModIndex + 1);
		FGameplayModifierInfo& ModInfo = StatusGE->Modifiers[ModIndex];
		ModInfo.Attribute = UHWAttributeSet::GetMoveSpeedMultiplierAttribute();
		ModInfo.ModifierOp = EGameplayModOp::Multiplicitive;
		ModInfo.ModifierMagnitude = FScalableFloat(SlowMul);
		break;
	}
	case EHWElement::Wind:
		// Lift is pure crowd control: the Status.Lift tag (granted above) is the effect.
		break;
	default:
		break;
	}

	FGameplayEffectContextHandle Context = TargetASC->MakeEffectContext();
	Context.AddSourceObject(Source);
	if (Source != nullptr)
	{
		Context.AddInstigator(Source, Source);
	}

	FGameplayEffectSpec Spec(StatusGE, Context, 1.f);
	TargetASC->ApplyGameplayEffectSpecToSelf(Spec);
}
