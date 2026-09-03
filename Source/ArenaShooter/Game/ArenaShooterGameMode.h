// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ArenaShooterGameMode.generated.h"

class AArenaShooterObjective;
class UArenaShooterMatchData;
class UArenaShooterWaveComponent;

ARENASHOOTER_API DECLARE_LOG_CATEGORY_EXTERN(LogArenaShooterMatch, Log, All);

/** Carries the wave's number counted from 1, so a listener can name it without asking back. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FArenaShooterWaveStartedSignature, int32, WaveNumber);

UENUM()
enum class EArenaShooterMatchState : uint8
{
	InProgress,
	Won,
	Lost
};

/**
 * The match: waves in order, and the two ways it can stop. Holds that state itself rather than in a
 * component of its own, because the wave table, which wave is running and how it ended are one piece
 * of knowledge, and because a game mode is already what a match is here -- there is no replication
 * to answer to that would ask for a game state instead.
 *
 * The default pawn, the controller class and the match data are set in a Blueprint subclass.
 */
UCLASS()
class ARENASHOOTER_API AArenaShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AArenaShooterGameMode();

	/**
	 * Fires as each wave begins, after its first enemy has been sent for. A moment rather than a
	 * value: what an announcement listens for, since polling the wave number would only ever show
	 * that it had changed at some point.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Match")
	FArenaShooterWaveStartedSignature OnWaveStarted;

	/**
	 * Whether the match has stopped, either way. What the restart input asks before doing anything.
	 *
	 * The engine already has this question and means the same thing by it, so this answers it rather
	 * than adding a second one beside it. The base implementation defers to the game state, which
	 * has nothing to say without replication to carry it.
	 */
	virtual bool HasMatchEnded() const override { return State != EArenaShooterMatchState::InProgress; }

	/** Which wave is running, counted from 1 the way a player counts. Zero until the first one starts. */
	UFUNCTION(BlueprintPure, Category = "Match")
	int32 GetCurrentWaveNumber() const { return CurrentWaveIndex + 1; }

	/** How many waves the match has in all. Zero without match data, which also means nothing runs. */
	UFUNCTION(BlueprintPure, Category = "Match")
	int32 GetWaveCount() const;

	/**
	 * Seconds until the next wave begins, or a negative number when none is coming: while a wave is
	 * running, after the match has ended, and during the wait that follows the last wave. That last
	 * wait runs on the same timer as the others but ends in a win, not a wave, so a countdown shown
	 * over it would be counting to nothing.
	 */
	UFUNCTION(BlueprintPure, Category = "Match")
	float GetSecondsUntilNextWave() const;

	/** Enemies the running wave still has to lose, including any yet to come out. Zero between waves. */
	UFUNCTION(BlueprintPure, Category = "Match")
	int32 GetRemainingEnemyCount() const;

	/**
	 * What the match is defending, as found on BeginPlay. Null before that, if the level has none,
	 * and -- for anyone checking with IsValid -- once it has been shot down, since it destroys itself.
	 */
	UFUNCTION(BlueprintPure, Category = "Match")
	AArenaShooterObjective* GetBase() const { return Base; }

protected:
	virtual void BeginPlay() override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

private:
	void StartNextWave();

	/** Schedules the next wave, or starts it now when no wait was asked for. */
	void ScheduleNextWave(float Delay);

	UFUNCTION()
	void HandleWaveCleared();

	UFUNCTION()
	void HandleBaseDestroyed();

	UFUNCTION()
	void HandlePlayerDeath();

	/** Ends the match, once. Later triggers find it already over and change nothing. */
	void EndMatch(EArenaShooterMatchState Result);

	/** Quiets whatever is still walking around, so a finished match stops moving. */
	void StopEnemies();

	/** How many waves and how big. Set in a Blueprint subclass. */
	UPROPERTY(EditDefaultsOnly, Category = "Match")
	TObjectPtr<UArenaShooterMatchData> MatchData;

	/** One wave at a time; this is what puts them in order. */
	UPROPERTY(VisibleAnywhere, Category = "Match")
	TObjectPtr<UArenaShooterWaveComponent> Wave;

	/** What the match is defending. Found on BeginPlay, since it is a level actor. */
	UPROPERTY(Transient)
	TObjectPtr<AArenaShooterObjective> Base;

	/** Index into the wave table. INDEX_NONE until the first wave starts. */
	int32 CurrentWaveIndex = INDEX_NONE;

	EArenaShooterMatchState State = EArenaShooterMatchState::InProgress;

	FTimerHandle NextWaveTimer;
};
