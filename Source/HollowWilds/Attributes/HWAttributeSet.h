#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "HWAttributeSet.generated.h"

/**
 * Standard GAS attribute accessor macro. Generates the property getter and the
 * value getter/setter/initter for a FGameplayAttributeData member.
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Attribute set for HollowWilds characters (the witch and enemies).
 *
 * Vitals:   Health/MaxHealth, Essence/MaxEssence (mana/spell resource), Poise/MaxPoise.
 * Defense:  Absorb (barrier shield pool consumed before Health).
 * Movement: MoveSpeedMultiplier (1.0 default; slow effects multiply this down).
 * Meta:     IncomingDamage (transient, server-side damage pipe; not replicated).
 */
UCLASS()
class HOLLOWWILDS_API UHWAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UHWAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// --- Health ---
	UPROPERTY(BlueprintReadOnly, Category = "Vitals", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UHWAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Vitals", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UHWAttributeSet, MaxHealth)

	// --- Essence (mana / spell resource) ---
	UPROPERTY(BlueprintReadOnly, Category = "Vitals", ReplicatedUsing = OnRep_Essence)
	FGameplayAttributeData Essence;
	ATTRIBUTE_ACCESSORS(UHWAttributeSet, Essence)

	UPROPERTY(BlueprintReadOnly, Category = "Vitals", ReplicatedUsing = OnRep_MaxEssence)
	FGameplayAttributeData MaxEssence;
	ATTRIBUTE_ACCESSORS(UHWAttributeSet, MaxEssence)

	// --- Poise ---
	UPROPERTY(BlueprintReadOnly, Category = "Vitals", ReplicatedUsing = OnRep_Poise)
	FGameplayAttributeData Poise;
	ATTRIBUTE_ACCESSORS(UHWAttributeSet, Poise)

	UPROPERTY(BlueprintReadOnly, Category = "Vitals", ReplicatedUsing = OnRep_MaxPoise)
	FGameplayAttributeData MaxPoise;
	ATTRIBUTE_ACCESSORS(UHWAttributeSet, MaxPoise)

	// --- Absorb (barrier shield pool) ---
	UPROPERTY(BlueprintReadOnly, Category = "Defense", ReplicatedUsing = OnRep_Absorb)
	FGameplayAttributeData Absorb;
	ATTRIBUTE_ACCESSORS(UHWAttributeSet, Absorb)

	// --- Movement ---
	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_MoveSpeedMultiplier)
	FGameplayAttributeData MoveSpeedMultiplier;
	ATTRIBUTE_ACCESSORS(UHWAttributeSet, MoveSpeedMultiplier)

	// --- Meta attributes (not replicated) ---
	UPROPERTY(BlueprintReadOnly, Category = "Meta")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UHWAttributeSet, IncomingDamage)

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	void OnRep_Essence(const FGameplayAttributeData& OldEssence);

	UFUNCTION()
	void OnRep_MaxEssence(const FGameplayAttributeData& OldMaxEssence);

	UFUNCTION()
	void OnRep_Poise(const FGameplayAttributeData& OldPoise);

	UFUNCTION()
	void OnRep_MaxPoise(const FGameplayAttributeData& OldMaxPoise);

	UFUNCTION()
	void OnRep_Absorb(const FGameplayAttributeData& OldAbsorb);

	UFUNCTION()
	void OnRep_MoveSpeedMultiplier(const FGameplayAttributeData& OldMoveSpeedMultiplier);
};
