// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/ArenaShooterWeaponComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Physics/ArenaShooterCollisionChannels.h"

void UArenaShooterWeaponComponent::StartFiring()
{
	bWantsToFire = true;

	// Fill the accumulator so the press itself fires rather than waiting out an interval.
	TimeSinceLastShot = FireInterval;
}

void UArenaShooterWeaponComponent::StopFiring()
{
	bWantsToFire = false;
}

void UArenaShooterWeaponComponent::Update(float DeltaSeconds, const FArenaShooterAimTarget& AimTarget)
{
	TimeSinceLastShot += DeltaSeconds;

	if (!bWantsToFire || TimeSinceLastShot < FireInterval)
	{
		return;
	}

	// At most one shot per update. A long frame can accumulate several intervals, but firing them
	// all would stack shots on one aim point; losing the rate for a moment reads better.
	Fire(AimTarget);
	TimeSinceLastShot = 0.0f;
}

void UArenaShooterWeaponComponent::Fire(const FArenaShooterAimTarget& AimTarget)
{
	UWorld* World = GetWorld();
	ACharacter* Owner = GetOwner<ACharacter>();
	if (World == nullptr || Owner == nullptr || Owner->GetMesh() == nullptr)
	{
		return;
	}

	// One montage play per shot. Replaying the same montage restarts it, so the recoil reads
	// once per trigger pull rather than continuing from where the last shot left off.
	if (FireMontage)
	{
		Owner->PlayAnimMontage(FireMontage);
	}

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	// Starting at the muzzle is what makes cover between the gun and the target stop the shot,
	// which a camera-origin trace would see straight over.
	const FVector MuzzleLocation = Owner->GetMesh()->GetSocketLocation(MuzzleSocketName);
	FVector ShotDirection = (AimTarget.Point - MuzzleLocation).GetSafeNormal();

	// A target pressed against the character can put the aim point behind the muzzle, which would
	// fire backwards. Fall back to the direction the aim came from when that happens.
	if (ShotDirection.IsNearlyZero() || FVector::DotProduct(ShotDirection, AimTarget.Direction) <= 0.0)
	{
		ShotDirection = AimTarget.Direction;
	}

	const FVector ShotEnd = MuzzleLocation + ShotDirection * FireRange;

	FHitResult ShotHit;
	const bool bShotHit = World->LineTraceSingleByChannel(
		ShotHit, MuzzleLocation, ShotEnd, ArenaShooter_TraceChannel_Weapon, Params);

	if (bShotHit && ShotHit.GetActor() != nullptr)
	{
		UGameplayStatics::ApplyPointDamage(
			ShotHit.GetActor(), Damage, ShotDirection, ShotHit, Owner->GetController(), Owner, nullptr);
	}

	// Verification aid, not presentation: one marker per shot makes the fire rate, the impact point
	// and the range limit observable while nothing else draws the shot.
	DrawDebugSphere(World, bShotHit ? ShotHit.ImpactPoint : ShotEnd, 12.0f, 8, FColor::Yellow, false, 1.0f);
}
