// Copyright Kepler Interactive. All Rights Reserved.

#include "Combat/HWProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "Combat/HWCombatStatics.h"

AHWProjectile::AHWProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// Sphere is the root + the collision used for impact detection.
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(16.f);
	CollisionSphere->SetCollisionProfileName(TEXT("Projectile"));
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Overlap);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionSphere->SetGenerateOverlapEvents(true);
	CollisionSphere->CanCharacterStepUpOn = ECB_No;
	RootComponent = CollisionSphere;

	// Optional cosmetic mesh; no collision of its own.
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Straight-line flight, no gravity.
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;

	InitialLifeSpan = 0.f; // set from LifeSeconds in BeginPlay so editor tweaks apply
}

void AHWProjectile::BeginPlay()
{
	Super::BeginPlay();

	// Keep the movement speed in sync with any per-spawn override of Speed.
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = Speed;
		ProjectileMovement->MaxSpeed = Speed;
		ProjectileMovement->Velocity = GetActorForwardVector() * Speed;
	}

	// Never collide with whoever fired us.
	if (AActor* MyInstigator = GetInstigator())
	{
		if (CollisionSphere)
		{
			CollisionSphere->IgnoreActorWhenMoving(MyInstigator, true);
		}
	}

	if (CollisionSphere)
	{
		CollisionSphere->OnComponentHit.AddDynamic(this, &AHWProjectile::OnHit);
		CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AHWProjectile::OnOverlap);
	}

	// Despawn after LifeSeconds if it hits nothing (mirrors Projectile.cs lifetime).
	if (LifeSeconds > 0.f)
	{
		SetLifeSpan(LifeSeconds);
	}
}

void AHWProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	HandleImpact(OtherActor);
}

void AHWProjectile::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	HandleImpact(OtherActor);
}

void AHWProjectile::HandleImpact(AActor* OtherActor)
{
	if (bImpacted)
	{
		return;
	}

	// Ignore self / instigator / no-target overlaps.
	if (OtherActor == nullptr || OtherActor == this || OtherActor == GetInstigator())
	{
		return;
	}

	bImpacted = true;

	AActor* Source = GetInstigator();
	if (Source == nullptr)
	{
		Source = GetOwner();
	}

	// Damage first, then the element's status (mirrors Projectile.OnTriggerEnter).
	UHWCombatStatics::ApplyDamage(Source, OtherActor, Damage);
	UHWCombatStatics::ApplyElementStatus(Source, OtherActor, Element, StatusMagnitude, StatusDuration);

	Destroy();
}
