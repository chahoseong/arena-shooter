// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Combat/ArenaShooterWeaponComponent.h"
#include "GameFramework/Character.h"
#include "ArenaShooterPlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UAnimMontage;
class UInputAction;
class UArenaShooterAimComponent;
class UArenaShooterHealthComponent;
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
	UFUNCTION()
	void HandleDeath();

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	/** What the centre of the screen points at, and the direction it was found along. */
	FArenaShooterAimTarget GetAimTarget() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UArenaShooterWeaponComponent> Weapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aim", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UArenaShooterAimComponent> Aim;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UArenaShooterHealthComponent> Health;

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

	/**
	 * Played once on death. Its slot has to be the full-body one, or the legs keep running the
	 * locomotion pose underneath. Left unset here: the animation belongs to whichever mesh the
	 * Blueprint picks.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> DeathMontage;

	/** Unaimed speed, captured on BeginPlay. Contributions are multiplied onto it. */
	float BaseWalkSpeed = 0.0f;

};
