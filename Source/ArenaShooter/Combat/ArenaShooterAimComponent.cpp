// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/ArenaShooterAimComponent.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

void UArenaShooterAimComponent::SetSpringArm(USpringArmComponent* InSpringArm)
{
	SpringArm = InSpringArm;
	if (SpringArm)
	{
		UnaimedArmLength = SpringArm->TargetArmLength;
		UnaimedShoulderOffset = SpringArm->SocketOffset.Y;
	}
}

void UArenaShooterAimComponent::SetCamera(UCameraComponent* InCamera)
{
	Camera = InCamera;
	if (Camera)
	{
		UnaimedFieldOfView = Camera->FieldOfView;
	}
}

void UArenaShooterAimComponent::StartAiming()
{
	bIsAiming = true;
}

void UArenaShooterAimComponent::StopAiming()
{
	bIsAiming = false;
}

void UArenaShooterAimComponent::Update(float DeltaSeconds)
{
	// One transition drives everything, so the camera, speed and sensitivity stay in step.
	// Interpolating the alpha is equivalent to interpolating each value separately, since
	// FInterpTo steps proportionally to the remaining distance.
	Alpha = FMath::FInterpTo(Alpha, bIsAiming ? 1.0f : 0.0f, DeltaSeconds, AimBlendSpeed);

	if (SpringArm)
	{
		SpringArm->TargetArmLength = FMath::Lerp(UnaimedArmLength, AimArmLength, Alpha);
		SpringArm->SocketOffset.Y = FMath::Lerp(UnaimedShoulderOffset, AimShoulderOffset, Alpha);
	}

	if (Camera)
	{
		Camera->SetFieldOfView(FMath::Lerp(UnaimedFieldOfView, AimFieldOfView, Alpha));
	}
}

float UArenaShooterAimComponent::GetSpeedMultiplier() const
{
	return FMath::Lerp(1.0f, AimSpeedMultiplier, Alpha);
}

float UArenaShooterAimComponent::GetLookScale() const
{
	return FMath::Lerp(1.0f, AimLookScale, Alpha);
}
