// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArenaShooterEnemyCharacter.h"

#include "Combat/ArenaShooterHealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

AArenaShooterEnemyCharacter::AArenaShooterEnemyCharacter()
{
	// Nothing to advance per frame. The controller drives movement and the animation blueprint
	// reads the result on its own tick.
	PrimaryActorTick.bCanEverTick = false;

	// The body faces where it is going, so a single forward run set covers every direction.
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	// Matches the running sample in the locomotion blend space, so the feet do not slide.
	GetCharacterMovement()->MaxWalkSpeed = 420.0f;

	Health = CreateDefaultSubobject<UArenaShooterHealthComponent>(TEXT("Health"));
}

void AArenaShooterEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	Health->OnDeath.AddDynamic(this, &AArenaShooterEnemyCharacter::HandleDeath);
}

void AArenaShooterEnemyCharacter::HandleDeath()
{
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	// Both the capsule and the mesh block the weapon trace, so leaving either one on keeps the
	// corpse soaking up shots that should carry on to whatever is behind it.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	float TimeUntilRemoval = BodyLingerTime;
	if (DeathMontage)
	{
		// Returns the play length, which is zero if the montage could not start.
		TimeUntilRemoval += PlayAnimMontage(DeathMontage);
	}

	if (TimeUntilRemoval <= 0.0f)
	{
		RemoveBody();
		return;
	}

	GetWorldTimerManager().SetTimer(
		RemoveBodyTimer, this, &AArenaShooterEnemyCharacter::RemoveBody, TimeUntilRemoval, false);
}

void AArenaShooterEnemyCharacter::RemoveBody()
{
	Destroy();
}
