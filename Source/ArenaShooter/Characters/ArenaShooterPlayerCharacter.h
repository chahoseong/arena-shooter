// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ArenaShooterPlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

/**
 * Player character. Moves relative to the camera and always faces the camera's horizontal
 * direction, so sideways and backward input strafes instead of turning the body.
 *
 * Input assets and tuning values are exposed as properties and set in a Blueprint subclass.
 */
UCLASS()
class ARENASHOOTER_API AArenaShooterPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AArenaShooterPlayerCharacter();

	virtual void NotifyControllerChanged() override;

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	void StartFiring();
	void StopFiring();
	void Fire();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	int32 DefaultMappingPriority = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> FireAction;

	/** Seconds between shots while the fire input is held. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float FireInterval = 0.15f;

	/**
	 * How far a shot reaches, measured from the muzzle. Raising this past the arena's diagonal
	 * makes the out-of-range case impossible to test inside the arena.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float FireRange = 8000.0f;

	/** Payload for the hit notification. Placeholder: balance belongs to health, hit and death. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	float Damage = 10.0f;

	/**
	 * Socket on the character mesh that shots start from. Bone-attached, so it follows animation.
	 * Left unset here: the socket name belongs to whichever mesh the Blueprint picks.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	FName MuzzleSocketName;

	FTimerHandle FireTimerHandle;
};
