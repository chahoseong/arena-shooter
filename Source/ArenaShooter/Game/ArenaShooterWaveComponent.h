// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ArenaShooterWaveComponent.generated.h"

class AArenaShooterEnemyCharacter;
class AArenaShooterEnemySpawnPoint;

ARENASHOOTER_API DECLARE_LOG_CATEGORY_EXTERN(LogArenaShooterWave, Log, All);

/** Fires once, when the last enemy of the wave has died. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FArenaShooterWaveClearedSignature);

/**
 * One wave: a given number of enemies put into the arena, and the answer to whether they are all
 * dead yet.
 *
 * It does not know which wave it is, or whether another follows. Running waves in order, and what
 * happens after the last one, belong to whatever starts them.
 *
 * The enemy class and the pacing are set in a Blueprint subclass of the game mode.
 */
UCLASS(ClassGroup = (ArenaShooter), meta = (BlueprintSpawnableComponent))
class ARENASHOOTER_API UArenaShooterWaveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UArenaShooterWaveComponent();

	/**
	 * Begins a wave of EnemyCount enemies of InEnemyClass. Does nothing if one is already running.
	 *
	 * Both come in from outside, because what a wave is made of and how big it is are the same kind
	 * of decision, and neither belongs to a component that only knows how to send one out.
	 */
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StartWave(int32 EnemyCount, TSubclassOf<AArenaShooterEnemyCharacter> InEnemyClass);

	UPROPERTY(BlueprintAssignable, Category = "Wave")
	FArenaShooterWaveClearedSignature OnWaveCleared;

	/**
	 * Enemies this wave still has to lose before it is cleared: those alive, and those yet to come
	 * out. Counted from what was asked for, less any spawn that failed, so it starts at the wave's
	 * size and only ever comes down.
	 */
	UFUNCTION(BlueprintPure, Category = "Wave")
	int32 GetRemainingCount() const { return AliveCount + PendingCount; }

protected:
	virtual void BeginPlay() override;

private:
	/** Puts out the next enemy and schedules the one after it. */
	void SpawnNext();

	UFUNCTION()
	void HandleEnemyDeath();

	/** Ends the wave once nothing is left to come out and nothing that came out is still alive. */
	void CheckCleared();

	/** What this wave is sending out. Given at the start and kept for the ones still to come. */
	UPROPERTY(Transient)
	TSubclassOf<AArenaShooterEnemyCharacter> EnemyClass;

	/** Seconds between one enemy coming out and the next. */
	UPROPERTY(EditDefaultsOnly, Category = "Wave", meta = (ClampMin = "0.0"))
	float SpawnInterval = 1.0f;

	/** Gathered on BeginPlay: the game mode is not in the level and cannot be handed level actors. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<AArenaShooterEnemySpawnPoint>> SpawnPoints;

	/** Still to come out. */
	int32 PendingCount = 0;

	/** Came out and still alive. Counted from what actually spawned, not from what was asked for. */
	int32 AliveCount = 0;

	/** Where the next enemy comes from, so that a wave is spread evenly over the placed points. */
	int32 NextPointIndex = 0;

	FTimerHandle SpawnTimer;
};
