// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Combat/ArenaShooterWeaponComponent.h"
#include "GameFramework/Character.h"
#include "ArenaShooterPlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UArenaShooterWeaponComponent;
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
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	void StartAiming();
	void StopAiming();

	/** What the centre of the screen points at, and the direction it was found along. */
	FArenaShooterAimTarget GetAimTarget() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UArenaShooterWeaponComponent> Weapon;

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

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> AimAction;

	/** Boom length while aiming. The length outside aim is whatever the Blueprint set. */
	UPROPERTY(EditDefaultsOnly, Category = "Aim", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float AimArmLength = 150.0f;

	/** Sideways camera offset while aiming, so the character does not cover the aim point. */
	UPROPERTY(EditDefaultsOnly, Category = "Aim", meta = (AllowPrivateAccess = "true"))
	float AimShoulderOffset = 50.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Aim", meta = (AllowPrivateAccess = "true", ClampMin = "5.0", ClampMax = "170.0"))
	float AimFieldOfView = 65.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Aim", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float AimWalkSpeed = 300.0f;

	/** Look input is scaled by this while aiming, so the view turns more slowly. */
	UPROPERTY(EditDefaultsOnly, Category = "Aim", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float AimLookScale = 0.5f;

	/** How quickly the camera, speed and field of view reach their aimed or unaimed values. */
	UPROPERTY(EditDefaultsOnly, Category = "Aim", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float AimBlendSpeed = 10.0f;

	/**
	 * Draws a marker on whatever the centre of the screen points at. Verification instrumentation
	 * rather than presentation: without a reference point there is no way to judge whether the
	 * character covers the aim point. Remove it once UI / Feedback provides a real crosshair.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Aim", meta = (AllowPrivateAccess = "true"))
	bool bDrawAimDebug = true;

	bool bIsAiming = false;

	// Captured on BeginPlay rather than hardcoded, so the Blueprint's values stay authoritative.
	float DefaultArmLength = 0.0f;
	float DefaultShoulderOffset = 0.0f;
	float DefaultFieldOfView = 0.0f;
	float DefaultWalkSpeed = 0.0f;

};
