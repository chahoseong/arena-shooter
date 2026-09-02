// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ArenaShooterMeleeAttackComponent.generated.h"

class UAnimMontage;

/**
 * A swing: how far it reaches, how often it comes, and whether it landed. Knows its owner only as
 * an ACharacter, and nothing about the behaviour tree that asks it to swing.
 *
 * There is no Update(DeltaSeconds) here, unlike the weapon component. Nothing needs advancing per
 * frame: the cadence is answered from world time when asked, and how far the swing has got is the
 * montage's business, not a counter kept here.
 */
UCLASS(ClassGroup = (ArenaShooter), meta = (BlueprintSpawnableComponent))
class ARENASHOOTER_API UArenaShooterMeleeAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UArenaShooterMeleeAttackComponent();

	/**
	 * Whether a swing at Target is worth starting: something to hit, close enough, and off cooldown.
	 *
	 * Deliberately does not test the angle. Facing is what the swing itself corrects during its
	 * wind-up, so refusing to start while the target is off to one side would mean never turning
	 * towards it at all.
	 */
	bool CanAttack(const AActor* Target) const;

	/**
	 * Whether Target is close enough to swing at, whether or not a swing is due yet.
	 *
	 * Separate from the cadence because they answer different questions and are asked by different
	 * things: this one decides where the approach ends, and holds true through the wait between
	 * swings. Folding the cadence in would let the enemy resume closing every time one finished.
	 */
	bool IsInReach(const AActor* Target) const;

	/** Whether enough time has passed since the last swing for another to start. */
	bool IsReady() const;

	/** Begins a swing at Target. False when there is nothing to swing at. */
	bool StartAttack(AActor* Target);

	bool IsAttacking() const { return bIsAttacking; }

	/**
	 * How far away from Target's centre a swing at it can still land. The approach distance is
	 * derived from this, so that where an enemy stops and where it can reach cannot drift apart.
	 */
	float GetReachTo(const AActor& Target) const;

	/**
	 * How far an actor's body reaches out sideways from its origin. Public because where an enemy
	 * stops is derived from the same measurement the swing is judged by.
	 */
	static float GetPlanarRadius(const AActor& Actor);

	/** Decides the swing where it stands: reach, angle, and the evidence that it happened. */
	void ResolveHit();

private:
	UFUNCTION()
	void HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted);

public:

	/** Drops a swing in flight without deciding it. Safe to call when there is nothing to drop. */
	void CancelAttack();

private:
	/**
	 * The gap the swing can cross: from this enemy's own body to the target's, not between their
	 * origins. Both radii are taken out of the measurement, so the number means the same thing
	 * against a player as against something as wide as the base.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Melee", meta = (ClampMin = "0.0"))
	float AttackRange = 125.0f;

	/** Half-angle of the arc in front of the enemy. This is what stepping aside escapes. */
	UPROPERTY(EditDefaultsOnly, Category = "Melee", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float AttackAngle = 45.0f;

	/** Seconds from one swing starting to the next being allowed. */
	UPROPERTY(EditDefaultsOnly, Category = "Melee", meta = (ClampMin = "0.01"))
	float AttackInterval = 2.0f;

	/** Payload for the hit notification. Nothing receives it until the player has health. */
	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	float Damage = 20.0f;

	/**
	 * The swing itself. Left unset here: the animation belongs to whichever mesh the Blueprint picks.
	 * Its slot has to be one the animation blueprint evaluates, and the hit notify lives on it.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	TObjectPtr<UAnimMontage> AttackMontage;

	/**
	 * Must match the Warp Target Name on the montage's Motion Warping notify. It is the contract with
	 * a hand-authored asset, and a mismatch is quiet: the swing plays and lands as usual, and only
	 * the turn towards the target goes missing.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	FName WarpTargetName = TEXT("MeleeTarget");

	/**
	 * Draws the arc the swing was decided against, and where the target stood at that moment.
	 * Verification instrumentation rather than presentation: being hit has no other visible
	 * consequence yet. Remove it once the player has health.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	bool bDrawAttackDebug = true;

	/** Who this swing was aimed at, captured when it started. */
	UPROPERTY(Transient)
	TObjectPtr<AActor> AttackTarget;

	/** A swing is in flight: the montage is still running. What the behaviour tree waits on. */
	bool bIsAttacking = false;

	/**
	 * This swing has already been decided. Separate from bIsAttacking because the decision lands
	 * partway through, and the swing keeps running afterwards.
	 */
	bool bHitResolved = false;

	/** Stamped rather than accumulated, since nothing advances this component per frame. */
	double LastAttackStartTime = -UE_BIG_NUMBER;
};
