// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ArenaShooterEnemyController.generated.h"

class UArenaShooterMeleeAttackComponent;
class UBehaviorTree;
struct FAIStimulus;

ARENASHOOTER_API DECLARE_LOG_CATEGORY_EXTERN(LogArenaShooterAI, Log, All);

/**
 * Drives an enemy. Decides what the blackboard says; the behaviour tree decides what to do about
 * it. Choosing what to go for is the controller's job because it comes from senses and damage,
 * neither of which the tree can observe.
 *
 * There is always something to go for. The base is what an enemy heads for by default, the player
 * takes over while being sensed, and the base takes it back once the memory of the player runs out.
 * Because both end up in the same blackboard entry, the tree needs no branch of its own for either.
 *
 * The behaviour tree and the blackboard key names are set in a Blueprint subclass. Sight is tuned
 * on the Perception component there, which already exposes its own configuration.
 */
UCLASS()
class ARENASHOOTER_API AArenaShooterEnemyController : public AAIController
{
	GENERATED_BODY()

public:
	AArenaShooterEnemyController();

	/**
	 * Puts the mind down and leaves the body where it stands. Death does this so a corpse stops
	 * turning to face the player, and a finished match does it so the arena stops moving.
	 */
	void StopBehaviour();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

private:
	UFUNCTION()
	void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void HandleDamaged(AActor* DamageCauser);

	UFUNCTION()
	void HandleDeath();

	UFUNCTION()
	void HandleBaseDestroyed(AActor* DestroyedActor);

	/** Ignores anything that is not the player, so enemies do not turn on each other. */
	void StartPursuing(AActor* Target);

	/** Begins the countdown after which the player stops being the target. */
	void StartForgetting();

	/** Gives the target back to the base, unless the player turns out to still be in sight. */
	void ForgetTarget();

	/** Whether the player is being sensed right now, as opposed to merely remembered. */
	bool IsPursuedTargetSensed() const;

	/** Writes whichever of the two the enemy should be heading for. */
	void RefreshTarget();

	/** Derives how close the approach has to get from how far the swing reaches at Target. */
	void WriteApproachRadius(const AActor* Target);

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	/**
	 * Name of the blackboard entry this controller writes. It is the contract with a hand-authored
	 * asset, and a mismatch is silent: the tree simply never gets a value.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName TargetActorKey = TEXT("TargetActor");

	/** Where the derived approach distance is published for the tree's Move To to read. */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName ApproachRadiusKey = TEXT("ApproachRadius");

	/**
	 * How long the player stays the target after the last time it was sensed. Without it, stepping
	 * behind cover would send the whole wave back to the base, and an enemy sitting on the edge of
	 * vision would flick between the two.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "AI", meta = (ClampMin = "0.0"))
	float TargetMemoryDuration = 3.0f;

	/**
	 * How much of the swing's reach is given up so that the approach ends inside it rather than on
	 * top of it. Has to cover the enemy's own radius, which the reach test adds on; the rest is how
	 * much daylight is left between the two when the swing starts.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "AI", meta = (ClampMin = "0.0"))
	float ApproachMargin = 50.0f;

	/** What this enemy heads for when the player is neither seen nor remembered. */
	UPROPERTY(Transient)
	TObjectPtr<AActor> DefendedBase;

	/** The player, while sensed or remembered. Null the rest of the time. */
	UPROPERTY(Transient)
	TObjectPtr<AActor> PursuedTarget;

	/** The possessed enemy's swing, kept because the approach distance is derived from its reach. */
	UPROPERTY(Transient)
	TObjectPtr<UArenaShooterMeleeAttackComponent> Melee;

	FTimerHandle ForgetTargetTimer;
};
