// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/ArenaShooterPlayerController.h"

#include "EnhancedInputComponent.h"
#include "Engine/World.h"
#include "Game/ArenaShooterGameMode.h"
#include "Kismet/GameplayStatics.h"

void AArenaShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent);
	if (Input == nullptr || RestartAction == nullptr)
	{
		return;
	}

	Input->BindAction(RestartAction, ETriggerEvent::Started, this, &AArenaShooterPlayerController::HandleRestart);
}

void AArenaShooterPlayerController::HandleRestart()
{
	const AArenaShooterGameMode* GameMode = Cast<AArenaShooterGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode == nullptr || !GameMode->HasMatchEnded())
	{
		return;
	}

	UE_LOG(LogArenaShooterMatch, Log, TEXT("Restarting."));

	// Loading the level again rather than putting the pieces back: health, invulnerability stamps
	// and the speeds captured on BeginPlay all reset only by being begun again.
	//
	// The prefix has to come off the name. In the editor the level is called UEDPIE_0_L_Default, and
	// asking to open that opens nothing.
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this, true)));
}
