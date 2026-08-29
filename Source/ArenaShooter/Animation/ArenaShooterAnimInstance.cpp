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
}
