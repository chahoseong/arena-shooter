// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/ArenaShooterObjective.h"

#include "Combat/ArenaShooterHealthComponent.h"
#include "Components/StaticMeshComponent.h"

AArenaShooterObjective::AArenaShooterObjective()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	// The default profile already blocks what it should: shots stop here rather than carrying on to
	// whatever is behind, enemies walk around it, and it hides what stands behind it from their
	// sight. Only the camera is wrong -- the player fights at arm's length from this, and a boom
	// that pulls in for it would spend the whole wave doing so.
	Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	// Health here is the same health an enemy has, because being shot until you are gone is the
	// same thing whoever is doing it.
	Health = CreateDefaultSubobject<UArenaShooterHealthComponent>(TEXT("Health"));
}

void AArenaShooterObjective::BeginPlay()
{
	Super::BeginPlay();

	Health->OnDeath.AddDynamic(this, &AArenaShooterObjective::HandleDeath);
}

void AArenaShooterObjective::HandleDeath()
{
	OnObjectiveDestroyed.Broadcast();

	// Nothing to play out: it is a lump of geometry, and how a destroyed thing should look belongs
	// with the rest of the feedback for attacks, hits and deaths.
	Destroy();
}
