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

	/** Begins a swing at Target. False when there is nothing to swing at. */
	bool StartAttack(AActor* Target);

	bool IsAttacking() const { return bIsAttacking; }

	/** Decides the swing where it stands: reach, angle, and the evidence that it happened. */
	void ResolveHit();

	/** Drops a swing in flight without deciding it. Safe to call when there is nothing to drop. */
	void CancelAttack();

private:
	/** How far the swing reaches, measured to the target's origin rather than to its collision. */
	UPROPERTY(EditDefaultsOnly, Category = "Melee", meta = (ClampMin = "0.0"))
	float AttackRange = 200.0f;

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
	 * Draws the arc the swing was decided against, and where the target stood at that moment.
	 * Verification instrumentation rather than presentation: being hit has no other visible
	 * consequence yet. Remove it once the player has health.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	bool bDrawAttackDebug = true;

	/** Who this swing was aimed at, captured when it started. */
	UPROPERTY(Transient)
	TObjectPtr<AActor> AttackTarget;

	bool bIsAttacking = false;

	/** Stamped rather than accumulated, since nothing advances this component per frame. */
	double LastAttackStartTime = -UE_BIG_NUMBER;
};
