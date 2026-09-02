// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ArenaShooterGameMode.generated.h"

class UArenaShooterWaveComponent;

/**
 * Game mode for the arena. Holds the state of the match, which is where it goes with no replication
 * to answer to; the default pawn and the wave tuning are set in a Blueprint subclass.
 */
UCLASS()
class ARENASHOOTER_API AArenaShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AArenaShooterGameMode();

private:
	/** One wave at a time. Running them in order belongs to whatever comes to own the match. */
	UPROPERTY(VisibleAnywhere, Category = "Arena")
	TObjectPtr<UArenaShooterWaveComponent> Wave;
};
