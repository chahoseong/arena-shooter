// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/ArenaShooterWaveComponent.h"

#include "Characters/ArenaShooterEnemyCharacter.h"
#include "Combat/ArenaShooterHealthComponent.h"
#include "Engine/World.h"
#include "Game/ArenaShooterEnemySpawnPoint.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogArenaShooterWave);

UArenaShooterWaveComponent::UArenaShooterWaveComponent()
{
	// Nothing to advance per frame: coming out is on a timer, and dying is an event.
	PrimaryComponentTick.bCanEverTick = false;
}

void UArenaShooterWaveComponent::BeginPlay()
{
	Super::BeginPlay();

	// Gathered rather than handed over. Spawn points are level actors and the game mode is not in
	// the level, so there is nothing that could hold references to them on this side.
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(this, AArenaShooterEnemySpawnPoint::StaticClass(), Found);
	for (AActor* Actor : Found)
	{
		SpawnPoints.Add(Cast<AArenaShooterEnemySpawnPoint>(Actor));
	}

	if (SpawnPoints.IsEmpty())
	{
		UE_LOG(LogArenaShooterWave, Warning,
			TEXT("No spawn points in the level; waves will start and clear without anything coming out."));
	}

	if (StartOnBeginPlayCount > 0)
	{
		StartWave(StartOnBeginPlayCount);
	}
}

void UArenaShooterWaveComponent::StartWave(int32 EnemyCount)
{
	if (PendingCount > 0 || AliveCount > 0)
	{
		UE_LOG(LogArenaShooterWave, Warning,
			TEXT("A wave is already running (%d to come, %d alive); ignoring a request for %d more."),
			PendingCount, AliveCount, EnemyCount);
		return;
	}

	UE_LOG(LogArenaShooterWave, Log, TEXT("Wave starting: %d enemies, one every %.1fs, over %d points."),
		EnemyCount, SpawnInterval, SpawnPoints.Num());

	PendingCount = FMath::Max(0, EnemyCount);
	AliveCount = 0;

	if (PendingCount == 0)
	{
		CheckCleared();
		return;
	}

	// The first one comes out now rather than after an interval, so that starting a wave is visible
	// immediately.
	SpawnNext();
}

void UArenaShooterWaveComponent::SpawnNext()
{
	if (PendingCount <= 0)
	{
		return;
	}

	--PendingCount;

	if (!SpawnPoints.IsEmpty())
	{
		// Round robin over whatever is placed, so a wave arrives from every direction at once and
		// changing the placement changes the shape of the pressure.
		AArenaShooterEnemySpawnPoint* Point = SpawnPoints[NextPointIndex % SpawnPoints.Num()];
		++NextPointIndex;

		AArenaShooterEnemyCharacter* Enemy = Point ? Point->SpawnEnemy(EnemyClass) : nullptr;
		if (Enemy)
		{
			// Counted from what came out, not from what was asked for. A spawn that found nowhere to
			// stand simply makes the wave smaller, rather than leaving it waiting on an enemy that
			// will never arrive.
			if (UArenaShooterHealthComponent* Health = Enemy->FindComponentByClass<UArenaShooterHealthComponent>())
			{
				Health->OnDeath.AddDynamic(this, &UArenaShooterWaveComponent::HandleEnemyDeath);
				++AliveCount;

				// Where it came from, because once it starts walking there is no telling: everything
				// converges on the base, so a position taken later says nothing about which point
				// put it there.
				UE_LOG(LogArenaShooterWave, Log, TEXT("Out of %s: %d alive, %d still to come."),
					*Point->GetName(), AliveCount, PendingCount);
			}
		}
		else
		{
			UE_LOG(LogArenaShooterWave, Warning, TEXT("A spawn failed; this wave is one enemy smaller."));
		}
	}

	if (PendingCount > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(
			SpawnTimer, this, &UArenaShooterWaveComponent::SpawnNext, SpawnInterval, false);
		return;
	}

	// Everything that was going to come out has. If none of it survived being spawned -- or all of
	// it died while the rest was still coming -- the wave is over now.
	CheckCleared();
}

void UArenaShooterWaveComponent::HandleEnemyDeath()
{
	AliveCount = FMath::Max(0, AliveCount - 1);

	UE_LOG(LogArenaShooterWave, Log, TEXT("Enemy down: %d alive, %d still to come."), AliveCount, PendingCount);

	CheckCleared();
}

void UArenaShooterWaveComponent::CheckCleared()
{
	// Both, because either alone is wrong. An early kill can empty the arena while enemies are still
	// queued, and a queue that has run dry says nothing about what is still walking around.
	if (AliveCount > 0 || PendingCount > 0)
	{
		return;
	}

	UE_LOG(LogArenaShooterWave, Log, TEXT("Wave cleared."));

	OnWaveCleared.Broadcast();
}
