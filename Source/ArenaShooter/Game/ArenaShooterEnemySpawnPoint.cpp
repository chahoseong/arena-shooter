// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/ArenaShooterEnemySpawnPoint.h"

#include "Characters/ArenaShooterEnemyCharacter.h"
#include "Components/BillboardComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "NavigationSystem.h"

DEFINE_LOG_CATEGORY(LogArenaShooterSpawn);

AArenaShooterEnemySpawnPoint::AArenaShooterEnemySpawnPoint()
{
	Marker = CreateDefaultSubobject<UBillboardComponent>(TEXT("Marker"));
	RootComponent = Marker;

	// Only to draw the radius while the level is being authored; see Tick.
	PrimaryActorTick.bCanEverTick = true;
}

void AArenaShooterEnemySpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	for (int32 Index = 0; Index < SpawnOnBeginPlay; ++Index)
	{
		SpawnEnemy(DebugEnemyClass);
	}
}

AArenaShooterEnemyCharacter* AArenaShooterEnemySpawnPoint::SpawnEnemy(
	TSubclassOf<AArenaShooterEnemyCharacter> EnemyClass)
{
	UWorld* World = GetWorld();
	if (World == nullptr || EnemyClass == nullptr)
	{
		UE_LOG(LogArenaShooterSpawn, Error, TEXT("%s was asked for an enemy with no class to spawn"), *GetName());
		return nullptr;
	}

	// A reachable point rather than any point: somewhere on the navigation mesh that can actually be
	// walked away from, so an enemy never appears inside geometry or stranded on an island.
	FVector Location;
	if (!UNavigationSystemV1::K2_GetRandomReachablePointInRadius(this, GetActorLocation(), Location, Radius))
	{
		UE_LOG(LogArenaShooterSpawn, Warning,
			TEXT("%s found nowhere to put an enemy; is the navigation mesh under it?"), *GetName());
		return nullptr;
	}

	FActorSpawnParameters Params;

	// The default refuses to spawn when something is in the way, which would quietly hand back one
	// fewer enemy than was asked for.
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// The point's own facing, until the wave rules say what an enemy should be looking at.
	return World->SpawnActor<AArenaShooterEnemyCharacter>(EnemyClass, Location, GetActorRotation(), Params);
}

void AArenaShooterEnemySpawnPoint::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#if WITH_EDITOR
	// A radius has no gizmo of its own, so it is drawn while the level is being authored. Without
	// this its size is a number you have to picture.
	const UWorld* World = GetWorld();
	if (World == nullptr || World->IsGameWorld())
	{
		return;
	}

	DrawDebugCircle(World, GetActorLocation(), Radius, 32, FColor::Cyan, false, -1.0f, 0, 4.0f,
		FVector::ForwardVector, FVector::RightVector, false);
#endif
}
