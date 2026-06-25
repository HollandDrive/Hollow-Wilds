// Copyright Kepler Interactive. All Rights Reserved.

#include "Abilities/GA_ElementalBolt.h"

#include "AbilitySystemComponent.h"

#include "Abilities/HWElementProfile.h"
#include "Attributes/HWAttributeSet.h"
#include "Character/HWWitchCharacter.h"
#include "Combat/HWProjectile.h"
#include "Core/HWGameplayTags.h"

UGA_ElementalBolt::UGA_ElementalBolt()
{
	// Tag this ability so input bindings / blocking can reference it.
	FGameplayTagContainer Tags;
	Tags.AddTag(HWGameplayTags::Ability_Spell_Bolt);
	SetAssetTags(Tags);
}

UGA_ElementalBolt::FBoltTuning UGA_ElementalBolt::ResolveTuning(EHWElement Element) const
{
	// Assigned profile wins if present for this element.
	for (const TObjectPtr<UHWElementProfile>& Profile : ElementProfiles)
	{
		if (Profile && Profile->Element == Element)
		{
			FBoltTuning Tuning;
			Tuning.Damage = Profile->Damage;
			Tuning.StatusMagnitude = Profile->StatusMagnitude;
			Tuning.StatusDuration = Profile->StatusDuration;
			Tuning.Cost = Profile->Cost;
			return Tuning;
		}
	}

	// Hardcoded fallback mirroring Unity ElementProfiles.Get.
	FBoltTuning Tuning;
	switch (Element)
	{
	case EHWElement::Fire:  Tuning = { 16.f, 8.f,  2.5f, 9.f };  break;
	case EHWElement::Ice:   Tuning = { 15.f, 0.4f, 2.5f, 11.f }; break;
	case EHWElement::Wind:  Tuning = { 11.f, 0.f,  1.3f, 14.f }; break;
	case EHWElement::Earth: Tuning = { 30.f, 0.f,  2.f,  18.f }; break;
	default:                Tuning = { 20.f, 0.f,  0.f,  10.f }; break;
	}
	return Tuning;
}

void UGA_ElementalBolt::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	AHWWitchCharacter* Witch = Cast<AHWWitchCharacter>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (Witch == nullptr || ASC == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility*/ true, /*bWasCancelled*/ true);
		return;
	}

	const EHWElement Element = Witch->GetCurrentElement();
	const FBoltTuning Tuning = ResolveTuning(Element);

	// Essence spend: fizzle if too low (mirrors Mana.TrySpend failing).
	const float CurrentEssence = ASC->GetNumericAttribute(UHWAttributeSet::GetEssenceAttribute());
	if (CurrentEssence < Tuning.Cost)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Commit any cost/cooldown GameplayEffects configured on the ability; spending
	// of Essence itself is done directly below so no BP cost asset is required.
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Subtract the element's essence cost directly (self-contained, no BP cost GE).
	ASC->ApplyModToAttribute(UHWAttributeSet::GetEssenceAttribute(), EGameplayModOp::Additive, -Tuning.Cost);

	// Only the authority spawns the bolt.
	if (Witch->HasAuthority() && ProjectileClass != nullptr)
	{
		// Aim from the control rotation (mirrors firing toward the screen-centre aim point).
		FRotator AimRotation = Witch->GetControlRotation();
		if (AimRotation.IsNearlyZero())
		{
			AimRotation = Witch->GetActorRotation();
		}
		const FVector Forward = AimRotation.Vector();

		// Muzzle: avatar location + offset expressed in the aim frame.
		const FVector RightDir = FRotationMatrix(AimRotation).GetUnitAxis(EAxis::Y);
		const FVector UpDir = FRotationMatrix(AimRotation).GetUnitAxis(EAxis::Z);
		const FVector Muzzle = Witch->GetActorLocation()
			+ Forward * MuzzleOffset.X
			+ RightDir * MuzzleOffset.Y
			+ UpDir * MuzzleOffset.Z;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Witch;
		SpawnParams.Instigator = Witch;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AHWProjectile* Bolt = GetWorld()->SpawnActor<AHWProjectile>(
			ProjectileClass, Muzzle, Forward.Rotation(), SpawnParams);
		if (Bolt)
		{
			Bolt->Element = Element;
			Bolt->Damage = Tuning.Damage;
			Bolt->StatusMagnitude = Tuning.StatusMagnitude;
			Bolt->StatusDuration = Tuning.StatusDuration;
			Bolt->Speed = BoltSpeed;
			Bolt->LifeSeconds = BoltLifeSeconds;
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility*/ true, /*bWasCancelled*/ false);
}
