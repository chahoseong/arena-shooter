// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ArenaShooterWeaponComponent.generated.h"

class UAnimMontage;

/**
 * Where a shot is headed. Supplied per update rather than stored on the weapon, so there is no
 * standing contract that someone keeps it fresh.
 */
USTRUCT()
struct FArenaShooterAimTarget
{
	GENERATED_BODY()

	/** What the centre of the screen points at. */
	FVector Point = FVector::ZeroVector;

	/** Used when the aim point ends up behind the muzzle, which would otherwise fire backwards. */
	FVector Direction = FVector::ForwardVector;
};

/**
 * How a shot works: cadence, trace, damage and the recoil montage. Knows its owner only as an
 * ACharacter, and nothing about cameras or input.
 */
UCLASS(ClassGroup = (ArenaShooter), meta = (BlueprintSpawnableComponent))
class ARENASHOOTER_API UArenaShooterWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Begins firing. The first shot leaves immediately rather than after one interval. */
	void StartFiring();
	void StopFiring();

	/** Advances the cadence and fires if one is due. Driven by the owner, not by a timer. */
	void Update(float DeltaSeconds, const FArenaShooterAimTarget& AimTarget);

	float GetRange() const { return FireRange; }

private:
	void Fire(const FArenaShooterAimTarget& AimTarget);

	/** Seconds between shots while the fire input is held. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = "0.01"))
	float FireInterval = 0.5f;

	/**
	 * How far a shot reaches, measured from the muzzle. Raising this past the arena's diagonal
	 * makes the out-of-range case impossible to test inside the arena.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = "0.0"))
	float FireRange = 8000.0f;

	/** Payload for the hit notification. Placeholder: balance belongs to health, hit and death. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float Damage = 10.0f;

	/** Upper-body montage played once per shot. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TObjectPtr<UAnimMontage> FireMontage;

	/**
	 * Socket on the owner's mesh that shots start from. Bone-attached, so it follows animation.
	 * Left unset here: the socket name belongs to whichever mesh the Blueprint picks.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName MuzzleSocketName;

	bool bWantsToFire = false;
	float TimeSinceLastShot = 0.0f;
};
