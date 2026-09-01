// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArenaShooterEnemyCharacter.h"

#include "Combat/ArenaShooterHealthComponent.h"
#include "Combat/ArenaShooterMeleeAttackComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MotionWarpingComponent.h"
#include "TimerManager.h"

AArenaShooterEnemyCharacter::AArenaShooterEnemyCharacter()
{
	// Nothing to advance per frame. The controller drives movement and the animation blueprint
	// reads the result on its own tick.
	PrimaryActorTick.bCanEverTick = false;

	// The body follows wherever the controller is looking, which is the path ahead while moving
	// and the focus target once stopped. Orienting to movement instead would leave the body stuck
	// facing its last direction of travel, because a standing character has no travel direction.
	//
	// FaceRotation is not used for this: it assigns the control rotation outright, so the body
	// would snap around. Going through the movement component keeps it inside RotationRate.
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	// Matches the running sample in the locomotion blend space, so the feet do not slide.
	GetCharacterMovement()->MaxWalkSpeed = 420.0f;

	// Two pieces, two questions. The capsule walks the world and blocks other characters; the mesh
	// is what a shot hits. Shots and the camera boom pass straight through the capsule, so hits are
	// decided against the silhouette and a character at close range does not shove the camera about.
	//
	// Profiles rather than response overrides: a Blueprint that stores a profile name overrides every
	// response set from here, silently and with no sign of it in the details panel.
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("ArenaShooterPawnCapsule"));
	GetMesh()->SetCollisionProfileName(TEXT("ArenaShooterPawnMesh"));

	Health = CreateDefaultSubobject<UArenaShooterHealthComponent>(TEXT("Health"));
	MeleeAttack = CreateDefaultSubobject<UArenaShooterMeleeAttackComponent>(TEXT("MeleeAttack"));
	MotionWarping = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarping"));
}

void AArenaShooterEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	Health->OnDeath.AddDynamic(this, &AArenaShooterEnemyCharacter::HandleDeath);
}

void AArenaShooterEnemyCharacter::HandleDeath()
{
	// Before anything else. The death montage blends the attack montage out rather than cutting it,
	// and a montage blending out keeps running, so a swing left in flight can still land on a corpse.
	MeleeAttack->CancelAttack();

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
	// The mind goes with the body. Unpossessing on its own leaves the controller in the world with a
	// live perception component, still running sight queries from where its pawn fell, and waves
	// would pile up one of those per kill.
	//
	// Done here rather than from OnUnPossess, which AController::UnPossess keeps using afterwards.
	if (AController* MyController = GetController())
	{
		MyController->Destroy();
	}

	Destroy();
}
