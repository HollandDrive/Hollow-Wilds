#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Abilities/HWElement.h"
#include "HWElementProfile.generated.h"

/**
 * Per-element tuning data asset: bolt colour, direct damage, a status value,
 * the essence cost, plus the AoE radius for the element's signature on-impact
 * effect. Mirrors the Unity ElementProfiles.Profile struct.
 *
 * Signature effects:
 *   Fire  = small AoE + a lingering ground-fire patch that burns foes (Status.Burn).
 *   Ice   = freezes/slows foes briefly (Status.Slow).
 *   Wind  = lifts/floats foes in a wind vortex (Status.Lift).
 *   Earth = cracks the ground into a lava fissure for heavy AoE (Status.Lava).
 *
 * StatusMagnitude meaning: Fire = burn dmg/sec, Ice = move-speed multiplier, others unused.
 * StatusDuration: Fire = burn time, Ice = slow time, Wind = lift time, Earth = lava time.
 *
 * Unity defaults (set per-instance in the editor, do not hardcode here):
 *   Fire  { Color(1, 0.45, 0.12),    Damage 16, StatusMagnitude 8,   StatusDuration 2.5, Cost 9,  AoeRadius 2.2, StatusTag Status.Burn }
 *   Ice   { Color(0.45, 0.8, 1),     Damage 15, StatusMagnitude 0.4, StatusDuration 2.5, Cost 11, AoeRadius 2.4, StatusTag Status.Slow }
 *   Wind  { Color(0.7, 1, 0.75),     Damage 11, StatusMagnitude 0,   StatusDuration 1.3, Cost 14, AoeRadius 2.6, StatusTag Status.Lift }
 *   Earth { Color(0.62, 0.42, 0.22), Damage 30, StatusMagnitude 0,   StatusDuration 2.0, Cost 18, AoeRadius 3.0, StatusTag Status.Lava }
 */
UCLASS(BlueprintType)
class HOLLOWWILDS_API UHWElementProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Which element this profile tunes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Element")
	EHWElement Element = EHWElement::Fire;

	/** Bolt / VFX colour for this element. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Element")
	FLinearColor Color = FLinearColor::White;

	/** Direct impact damage. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Element")
	float Damage = 0.f;

	/** Status value: Fire = burn dmg/sec, Ice = move-speed multiplier, others unused. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Element")
	float StatusMagnitude = 0.f;

	/** How long the applied status lasts (seconds). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Element")
	float StatusDuration = 0.f;

	/** Essence cost to cast. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Element")
	float Cost = 0.f;

	/** Radius of the element's signature on-impact AoE effect. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Element")
	float AoeRadius = 0.f;

	/** Which Status.* gameplay tag this element applies on impact. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Element")
	FGameplayTag StatusTag;
};
