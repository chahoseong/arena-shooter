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

	/** Begins a wave of EnemyCount enemies. Does nothing if one is already running. */
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StartWave(int32 EnemyCount);

	UPROPERTY(BlueprintAssignable, Category = "Wave")
	FArenaShooterWaveClearedSignature OnWaveCleared;

	/** Enemies that have been spawned and are still alive. Excludes any still waiting to come out. */
	UFUNCTION(BlueprintPure, Category = "Wave")
	int32 GetRemainingCount() const { return AliveCount; }

protected:
	virtual void BeginPlay() override;

private:
	/** Puts out the next enemy and schedules the one after it. */
	void SpawnNext();

	UFUNCTION()
	void HandleEnemyDeath();

	/** Ends the wave once nothing is left to come out and nothing that came out is still alive. */
	void CheckCleared();

	UPROPERTY(EditDefaultsOnly, Category = "Wave")
	TSubclassOf<AArenaShooterEnemyCharacter> EnemyClass;

	/** Seconds between one enemy coming out and the next. */
	UPROPERTY(EditDefaultsOnly, Category = "Wave", meta = (ClampMin = "0.0"))
	float SpawnInterval = 1.0f;

	/**
	 * Scaffolding for the period before anything starts a wave. Whatever runs the match becomes the
	 * caller, and when it does this goes back to nothing.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Wave|Debug", meta = (ClampMin = "0"))
	int32 StartOnBeginPlayCount = 0;

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
