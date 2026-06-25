// Copyright Kepler Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Abilities/HWElement.h"
#include "HWProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

/**
 * AHWProjectile
 *
 * The witch's magic bolt. Flies straight via a UProjectileMovementComponent,
 * carries an element's tuning, and on impact routes its damage + status through
 * UHWCombatStatics before destroying itself. Ignores its instigator so a bolt
 * never hits the caster. Mirrors the Unity Projectile.cs behaviour.
 */
UCLASS()
class HOLLOWWILDS_API AHWProjectile : public AActor
{
	GENERATED_BODY()

public:
	AHWProjectile();

	/** Direct impact damage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hollow Wilds|Projectile")
	float Damage = 16.f;

	/** Element this bolt carries (drives the on-impact status). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hollow Wilds|Projectile")
	EHWElement Element = EHWElement::Fire;

	/** Status value: Fire/Earth = DoT dmg/sec, Ice = move-speed multiplier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hollow Wilds|Projectile")
	float StatusMagnitude = 0.f;

	/** How long the applied status lasts (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hollow Wilds|Projectile")
	float StatusDuration = 0.f;

	/** Travel speed (cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hollow Wilds|Projectile")
	float Speed = 6000.f;

	/** Seconds before the bolt despawns if it hits nothing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hollow Wilds|Projectile")
	float LifeSeconds = 3.f;

	/** Optional visual mesh; may be left null (FX-only / VFX bolts). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hollow Wilds|Projectile")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** Spherical collision used for impact detection. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hollow Wilds|Projectile")
	TObjectPtr<USphereComponent> CollisionSphere;

	/** Drives the straight-line flight. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hollow Wilds|Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

protected:
	virtual void BeginPlay() override;

	/** Hit handler for blocking (swept) collisions. */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

	/** Overlap handler for trigger-style collisions. */
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	/** Apply damage + status to the hit actor, then destroy. Guards against double-trigger. */
	void HandleImpact(AActor* OtherActor);

	/** True once we have processed an impact so a sweep+overlap in one frame can't double-fire. */
	bool bImpacted = false;
};
