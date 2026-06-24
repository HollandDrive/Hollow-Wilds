#include "Attributes/HWAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UHWAttributeSet::UHWAttributeSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);

	InitEssence(100.f);
	InitMaxEssence(100.f);

	InitPoise(50.f);
	InitMaxPoise(50.f);

	InitAbsorb(0.f);

	InitMoveSpeedMultiplier(1.f);
}

void UHWAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UHWAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHWAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UHWAttributeSet, Essence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHWAttributeSet, MaxEssence, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UHWAttributeSet, Poise, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHWAttributeSet, MaxPoise, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UHWAttributeSet, Absorb, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UHWAttributeSet, MoveSpeedMultiplier, COND_None, REPNOTIFY_Always);
}

void UHWAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMoveSpeedMultiplierAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, 1.f);
	}
	else if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetEssenceAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxEssence());
	}
	else if (Attribute == GetPoiseAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxPoise());
	}
	else if (Attribute == GetAbsorbAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
}

void UHWAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float LocalIncomingDamage = GetIncomingDamage();
		SetIncomingDamage(0.f);

		if (LocalIncomingDamage > 0.f)
		{
			// Drain the barrier (Absorb) pool first, then apply the remainder to Health.
			float RemainingDamage = LocalIncomingDamage;

			const float CurrentAbsorb = GetAbsorb();
			if (CurrentAbsorb > 0.f)
			{
				const float AbsorbedAmount = FMath::Min(CurrentAbsorb, RemainingDamage);
				SetAbsorb(CurrentAbsorb - AbsorbedAmount);
				RemainingDamage -= AbsorbedAmount;
			}

			if (RemainingDamage > 0.f)
			{
				const float NewHealth = GetHealth() - RemainingDamage;
				SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));
			}
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetEssenceAttribute())
	{
		SetEssence(FMath::Clamp(GetEssence(), 0.f, GetMaxEssence()));
	}
	else if (Data.EvaluatedData.Attribute == GetPoiseAttribute())
	{
		SetPoise(FMath::Clamp(GetPoise(), 0.f, GetMaxPoise()));
	}
}

void UHWAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHWAttributeSet, Health, OldHealth);
}

void UHWAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHWAttributeSet, MaxHealth, OldMaxHealth);
}

void UHWAttributeSet::OnRep_Essence(const FGameplayAttributeData& OldEssence)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHWAttributeSet, Essence, OldEssence);
}

void UHWAttributeSet::OnRep_MaxEssence(const FGameplayAttributeData& OldMaxEssence)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHWAttributeSet, MaxEssence, OldMaxEssence);
}

void UHWAttributeSet::OnRep_Poise(const FGameplayAttributeData& OldPoise)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHWAttributeSet, Poise, OldPoise);
}

void UHWAttributeSet::OnRep_MaxPoise(const FGameplayAttributeData& OldMaxPoise)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHWAttributeSet, MaxPoise, OldMaxPoise);
}

void UHWAttributeSet::OnRep_Absorb(const FGameplayAttributeData& OldAbsorb)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHWAttributeSet, Absorb, OldAbsorb);
}

void UHWAttributeSet::OnRep_MoveSpeedMultiplier(const FGameplayAttributeData& OldMoveSpeedMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHWAttributeSet, MoveSpeedMultiplier, OldMoveSpeedMultiplier);
}
