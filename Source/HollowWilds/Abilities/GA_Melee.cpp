// Copyright Kepler Interactive. All Rights Reserved.

#include "Abilities/GA_Melee.h"

#include "Engine/OverlapResult.h"
#include "GameFramework/Pawn.h"

#include "Combat/HWCombatStatics.h"
#include "Core/HWGameplayTags.h"

UGA_Melee::UGA_Melee()
{
	FGameplayTagContainer Tags;
	Tags.AddTag(HWGameplayTags::Ability_Melee);
	SetAssetTags(Tags);
}

void UGA_Melee::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	UWorld* World = Avatar ? Avatar->GetWorld() : nullptr;
	if (Avatar == nullptr || World == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility*/ true, /*bWasCancelled*/ true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Only the authority resolves hits.
	if (Avatar->HasAuthority())
	{
		const FVector Forward = Avatar->GetActorForwardVector();
		const FVector SweepCenter = Avatar->GetActorLocation()
			+ Forward * Reach
			+ FVector::UpVector * HeightOffset;

		// Overlap a sphere in front of the avatar and damage each unique enemy once.
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(HWMeleeSweep), /*bTraceComplex*/ false, Avatar);
		QueryParams.AddIgnoredActor(Avatar);

		TArray<FOverlapResult> Overlaps;
		World->OverlapMultiByObjectType(
			Overlaps,
			SweepCenter,
			FQuat::Identity,
			FCollisionObjectQueryParams(ECC_Pawn),
			FCollisionShape::MakeSphere(Radius),
			QueryParams);

		const float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(ConeHalfAngleDegrees));

		TSet<AActor*> AlreadyHit;
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* Target = Overlap.GetActor();
			if (Target == nullptr || Target == Avatar)
			{
				continue;
			}

			// One hit per actor per swing.
			bool bWasAlreadyHit = false;
			AlreadyHit.Add(Target, &bWasAlreadyHit);
			if (bWasAlreadyHit)
			{
				continue;
			}

			// Constrain to the forward cone so it reads as a swing, not a radial pulse.
			FVector ToTarget = Target->GetActorLocation() - Avatar->GetActorLocation();
			ToTarget.Z = 0.f;
			if (!ToTarget.Normalize())
			{
				continue;
			}
			if (FVector::DotProduct(Forward, ToTarget) < CosHalfAngle)
			{
				continue;
			}

			// Only damage actors that actually have a GAS attribute set.
			if (UHWCombatStatics::GetASC(Target) != nullptr)
			{
				UHWCombatStatics::ApplyDamage(Avatar, Target, Damage);
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility*/ true, /*bWasCancelled*/ false);
}
