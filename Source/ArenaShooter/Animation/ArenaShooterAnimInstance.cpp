// Copyright Epic Games, Inc. All Rights Reserved.

#include "Animation/ArenaShooterAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"

void UArenaShooterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningCharacter = Cast<ACharacter>(TryGetPawnOwner());
}

void UArenaShooterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (OwningCharacter == nullptr)
	{
		// Initialisation can run before the mesh has an owner, so keep looking until it does.
		OwningCharacter = Cast<ACharacter>(TryGetPawnOwner());
		if (OwningCharacter == nullptr)
		{
			return;
		}
	}

	const FVector Velocity = OwningCharacter->GetVelocity();

	// Horizontal only: JumpZVelocity is 500, so a 3D length would read as running while airborne.
	Speed = Velocity.Size2D();
	Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, OwningCharacter->GetActorRotation());

	if (const UCharacterMovementComponent* Movement = OwningCharacter->GetCharacterMovement())
	{
		bIsInAir = Movement->IsFalling();
	}

	// How far the view has strayed from where the body faces. Yaw closes itself, since the body
	// turns to follow the view; pitch cannot, because the pawn is never pitched, so the aim
	// offset covers it in the pose instead.
	//
	// FRotator subtraction is per-axis, not a rotation composition, so this is only the true
	// local difference while the actor has no pitch or roll. That holds here. If the actor is
	// ever pitched, this needs the quaternion form: ActorQuat.Inverse() * FQuat(AimRotation).
	const FRotator AimDelta =
		(OwningCharacter->GetBaseAimRotation() - OwningCharacter->GetActorRotation()).GetNormalized();
	Pitch = AimDelta.Pitch;
}
