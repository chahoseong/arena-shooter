// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ArenaShooterPlayerController.generated.h"

class UInputAction;

/**
 * The player's controller. Exists for one thing so far: taking the input that starts the match over.
 *
 * That input cannot live on the pawn. Dying disables the pawn's input, and DisableInput takes only
 * that actor's input component off the stack -- a binding made here still fires, and the mapping
 * context the pawn added stays on the local player either way. A dead player has to be able to ask
 * for another go.
 *
 * The input action is set in a Blueprint subclass.
 */
UCLASS()
class ARENASHOOTER_API AArenaShooterPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void SetupInputComponent() override;

private:
	/** Loads the level again, but only once the match has actually finished. */
	void HandleRestart();

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> RestartAction;
};
