// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ArenaShooterMatchData.generated.h"

class AArenaShooterEnemyCharacter;

/**
 * How long a match is and how hard it presses. Kept as its own asset rather than as values on the
 * game mode Blueprint: balance is retuned often, and a change here should not rewrite a file that
 * also carries the pawn and controller classes. Several of these can exist and be swapped by
 * pointing the game mode at a different one.
 */
UCLASS()
class ARENASHOOTER_API UArenaShooterMatchData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** What the waves are made of. One type for the whole match; more than one is a Non Goal. */
	UPROPERTY(EditDefaultsOnly, Category = "Match")
	TSubclassOf<AArenaShooterEnemyCharacter> EnemyClass;

	/** One entry per wave, in order, each the number of enemies that wave sends out. */
	UPROPERTY(EditDefaultsOnly, Category = "Match")
	TArray<int32> WaveEnemyCounts;

	/**
	 * Seconds between the level starting and the first wave.
	 *
	 * Not only a beat before the fighting starts: spawning asks the navigation mesh for somewhere an
	 * enemy can stand, and on the very first frame it does not answer yet, which quietly costs the
	 * wave its first enemy.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Match", meta = (ClampMin = "0.0"))
	float FirstWaveDelay = 1.0f;

	/** Seconds of empty arena between one wave being cleared and the next starting. */
	UPROPERTY(EditDefaultsOnly, Category = "Match", meta = (ClampMin = "0.0"))
	float DelayBetweenWaves = 3.0f;
};
