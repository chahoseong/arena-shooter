// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ArenaShooterEnemyCharacter.generated.h"

class UAnimMontage;
class UArenaShooterHealthComponent;
class UArenaShooterMeleeAttackComponent;
class UMotionWarpingComponent;

/**
 * Melee enemy. Turns to face wherever its controller is looking, unlike the player, who keeps
 * facing the camera and strafes; the minion animations have no sideways or backward set to
 * strafe with.
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

	/**
	 * Where this enemy heads until something turns its attention to the player, in world space.
	 * Equals the enemy's own position when no destination was set, which reads as standing still.
	 */
	FVector GetDestinationLocation() const { return GetActorTransform().TransformPosition(Destination); }

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleDeath();

	void RemoveBody();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UArenaShooterHealthComponent> Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Melee", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UArenaShooterMeleeAttackComponent> MeleeAttack;

	/**
	 * Turns the swing towards its target during the attack montage's warping window. Builds its own
	 * character adapter when the component initialises, so there is nothing to hand it here.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Melee", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMotionWarpingComponent> MotionWarping;

	/**
	 * Set per placed enemy, so it lives on the pawn rather than the controller: controllers are
	 * spawned, not placed, and there is nothing in the level to point at one. The pawn only
	 * carries it; the controller reads it once and never asks again.
	 *
	 * MakeEditWidget puts a draggable handle in the viewport, so this can be placed by eye rather
	 * than typed. That handle is in the enemy's own space, so turning the enemy swings its
	 * destination with it.
	 */
	UPROPERTY(EditInstanceOnly, Category = "AI", meta = (MakeEditWidget, AllowPrivateAccess = "true"))
	FVector Destination = FVector::ZeroVector;

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
