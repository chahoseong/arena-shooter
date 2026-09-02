// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArenaShooterGameMode.h"

#include "Game/ArenaShooterWaveComponent.h"

AArenaShooterGameMode::AArenaShooterGameMode()
{
	Wave = CreateDefaultSubobject<UArenaShooterWaveComponent>(TEXT("Wave"));
}
