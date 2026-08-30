// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ArenaShooterEnemyCharacter.generated.h"

class UAnimMontage;
class UArenaShooterHealthComponent;

/**
 * Melee enemy. Turns to face wherever it is heading, unlike the player, who keeps facing the
 * camera and strafes; the minion animations have no sideways or backward set to strafe with.
 *
 * Movement itself belongs to the AI controller and is not part of this class.
 *
 * The mesh, the animation blueprint and tuning values are set in a Blueprint subclass.
 */
UCLASS()
class ARENASHOOTER_API AArenaShooterEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AArenaShooterEnemyCharacter();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleDeath();

	void RemoveBody();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UArenaShooterHealthComponent> Health;

	/** Played once on death. Left unset here: the animation belongs to whichever mesh the Blueprint picks. */
	UPROPERTY(EditDefaultsOnly, Category = "Death", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> DeathMontage;

	/**
	 * How long the body stays after the death montage ends. Zero by default because the montage
	 * blends back to the locomotion pose when it finishes, and a corpse standing up reads worse
	 * than one that vanishes. Raise it only alongside a montage that holds its final pose.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Death", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float BodyLingerTime = 0.0f;

	FTimerHandle RemoveBodyTimer;
};
