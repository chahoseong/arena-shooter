// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ArenaShooterGameMode.generated.h"

class AArenaShooterObjective;
class UArenaShooterMatchData;
class UArenaShooterWaveComponent;

ARENASHOOTER_API DECLARE_LOG_CATEGORY_EXTERN(LogArenaShooterMatch, Log, All);

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
	 * Whether the match has stopped, either way. What the restart input asks before doing anything.
	 *
	 * The engine already has this question and means the same thing by it, so this answers it rather
	 * than adding a second one beside it. The base implementation defers to the game state, which
	 * has nothing to say without replication to carry it.
	 */
	virtual bool HasMatchEnded() const override { return State != EArenaShooterMatchState::InProgress; }

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
