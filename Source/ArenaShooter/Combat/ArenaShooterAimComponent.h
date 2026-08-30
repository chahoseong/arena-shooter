// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ArenaShooterAimComponent.generated.h"

class UCameraComponent;
class USpringArmComponent;

/**
 * What aiming is: whether it is held, how far the transition has run, and the values it asks for.
 *
 * Applies directly to the camera it was given, since nothing else touches those. Speed and look
 * sensitivity are only offered as multipliers, because other features will want a say in them and
 * the owner has to combine the contributions.
 */
UCLASS(ClassGroup = (ArenaShooter), meta = (BlueprintSpawnableComponent))
class ARENASHOOTER_API UArenaShooterAimComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/**
	 * Each setter takes one thing and records what it looked like unaimed. They are separate
	 * because pulling the camera in and narrowing the view are independent effects: given only
	 * one, aiming still drives that one. Whatever is not given is left alone.
	 */
	void SetSpringArm(USpringArmComponent* InSpringArm);
	void SetCamera(UCameraComponent* InCamera);

	void StartAiming();
	void StopAiming();
	bool IsAiming() const { return bIsAiming; }

	/** Advances the transition and applies it. Driven by the owner so the order is explicit. */
	void Update(float DeltaSeconds);

	float GetSpeedMultiplier() const;
	float GetLookScale() const;

private:
	/** Boom length while aiming. The length outside aim is whatever the Blueprint set. */
	UPROPERTY(EditDefaultsOnly, Category = "Aim", meta = (ClampMin = "0.0"))
	float AimArmLength = 150.0f;

	/** Sideways camera offset while aiming, so the character does not cover the aim point. */
	UPROPERTY(EditDefaultsOnly, Category = "Aim")
	float AimShoulderOffset = 50.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Aim", meta = (ClampMin = "5.0", ClampMax = "170.0"))
	float AimFieldOfView = 65.0f;

	/**
	 * Movement speed while aiming, as a fraction of the unaimed speed. A multiplier rather than an
	 * absolute so that later contributors, such as a dash, can compose with it.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Aim", meta = (ClampMin = "0.0"))
	float AimSpeedMultiplier = 0.5f;

	/** Look input is scaled by this while aiming, so the view turns more slowly. */
	UPROPERTY(EditDefaultsOnly, Category = "Aim", meta = (ClampMin = "0.01"))
	float AimLookScale = 0.5f;

	/** How quickly the transition runs in either direction. */
	UPROPERTY(EditDefaultsOnly, Category = "Aim", meta = (ClampMin = "0.1"))
	float AimBlendSpeed = 10.0f;

	// UPROPERTY so the garbage collector can see these. Without it the references are untracked:
	// they would neither keep the components alive nor be cleared when they go away. Transient
	// because they are handed over at run time and have no business being serialised.
	UPROPERTY(Transient)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> Camera;

	// Captured when each is handed over, rather than hardcoded, so the Blueprint stays authoritative.
	float UnaimedArmLength = 0.0f;
	float UnaimedShoulderOffset = 0.0f;
	float UnaimedFieldOfView = 0.0f;

	bool bIsAiming = false;

	/** 0 when fully unaimed, 1 when fully aimed. Every aim value is read off this. */
	float Alpha = 0.0f;
};
