// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArenaShooterGameMode.h"

#include "AI/ArenaShooterEnemyController.h"
#include "Characters/ArenaShooterEnemyCharacter.h"
#include "Combat/ArenaShooterHealthComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Game/ArenaShooterMatchData.h"
#include "Game/ArenaShooterObjective.h"
#include "Game/ArenaShooterWaveComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogArenaShooterMatch);

AArenaShooterGameMode::AArenaShooterGameMode()
{
	Wave = CreateDefaultSubobject<UArenaShooterWaveComponent>(TEXT("Wave"));
}

void AArenaShooterGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (Wave)
	{
		Wave->OnWaveCleared.AddDynamic(this, &AArenaShooterGameMode::HandleWaveCleared);
	}

	// The base is a level actor, so it is found rather than handed over -- the same reason the wave
	// gathers its spawn points and an enemy looks up what it is advancing on.
	Base = Cast<AArenaShooterObjective>(
		UGameplayStatics::GetActorOfClass(this, AArenaShooterObjective::StaticClass()));
	if (Base)
	{
		Base->OnObjectiveDestroyed.AddDynamic(this, &AArenaShooterGameMode::HandleBaseDestroyed);
	}
	else
	{
		UE_LOG(LogArenaShooterMatch, Warning,
			TEXT("No base in the level; the match can only be lost by dying."));
	}

	if (MatchData == nullptr || MatchData->WaveEnemyCounts.IsEmpty())
	{
		UE_LOG(LogArenaShooterMatch, Error,
			TEXT("No match data, or none with any waves in it; nothing will be sent out."));
		return;
	}

	if (MatchData->EnemyClass == nullptr)
	{
		UE_LOG(LogArenaShooterMatch, Error, TEXT("Match data names no enemy class; nothing will be sent out."));
		return;
	}

	UE_LOG(LogArenaShooterMatch, Log, TEXT("Match starting: %d waves, first in %.1fs."),
		MatchData->WaveEnemyCounts.Num(), MatchData->FirstWaveDelay);

	// Through a timer rather than straight away. Asking on this frame is asking before the world can
	// answer, and the first enemy of the first wave goes missing.
	ScheduleNextWave(MatchData->FirstWaveDelay);
}

void AArenaShooterGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	// Here, and not in OnPostLogin or BeginPlay: this is the call that spawns the pawn, so it is the
	// first moment there is one to take health from. OnPostLogin runs just before it and would find
	// nothing, failing quietly -- the player would die and the match would carry on.
	const APawn* Pawn = NewPlayer ? NewPlayer->GetPawn() : nullptr;
	UArenaShooterHealthComponent* Health =
		Pawn ? Pawn->FindComponentByClass<UArenaShooterHealthComponent>() : nullptr;

	if (Health == nullptr)
	{
		UE_LOG(LogArenaShooterMatch, Warning,
			TEXT("The player has no health component; the match cannot be lost by dying."));
		return;
	}

	Health->OnDeath.AddUniqueDynamic(this, &AArenaShooterGameMode::HandlePlayerDeath);
}

int32 AArenaShooterGameMode::GetWaveCount() const
{
	return MatchData ? MatchData->WaveEnemyCounts.Num() : 0;
}

int32 AArenaShooterGameMode::GetRemainingEnemyCount() const
{
	return Wave ? Wave->GetRemainingCount() : 0;
}

float AArenaShooterGameMode::GetSecondsUntilNextWave() const
{
	// The timer alone cannot answer this. It is also what carries the match to its win after the last
	// wave, and it can outlive a match that ended some other way if a later change forgets to clear
	// it. So the timer only counts when there is a wave for it to be counting to.
	if (State != EArenaShooterMatchState::InProgress || MatchData == nullptr
		|| !MatchData->WaveEnemyCounts.IsValidIndex(CurrentWaveIndex + 1))
	{
		return -1.0f;
	}

	// Negative on its own when the handle is not active, which is what a wave in progress looks like.
	return GetWorldTimerManager().GetTimerRemaining(NextWaveTimer);
}

void AArenaShooterGameMode::StartNextWave()
{
	if (State != EArenaShooterMatchState::InProgress || MatchData == nullptr || Wave == nullptr)
	{
		return;
	}

	++CurrentWaveIndex;

	if (!MatchData->WaveEnemyCounts.IsValidIndex(CurrentWaveIndex))
	{
		EndMatch(EArenaShooterMatchState::Won);
		return;
	}

	const int32 EnemyCount = MatchData->WaveEnemyCounts[CurrentWaveIndex];

	UE_LOG(LogArenaShooterMatch, Log, TEXT("Wave %d of %d: %d enemies."),
		CurrentWaveIndex + 1, MatchData->WaveEnemyCounts.Num(), EnemyCount);

	Wave->StartWave(EnemyCount, MatchData->EnemyClass);

	OnWaveStarted.Broadcast(GetCurrentWaveNumber());
}

void AArenaShooterGameMode::HandleWaveCleared()
{
	if (State != EArenaShooterMatchState::InProgress || MatchData == nullptr)
	{
		return;
	}

	UE_LOG(LogArenaShooterMatch, Log, TEXT("Wave %d cleared."), CurrentWaveIndex + 1);

	// Through the delay even for the last wave, so that winning does not land on the same frame as
	// the final kill. Whether another wave follows is StartNextWave's question to answer.
	ScheduleNextWave(MatchData->DelayBetweenWaves);
}

void AArenaShooterGameMode::ScheduleNextWave(float Delay)
{
	// A timer set to zero never fires, so asking for no wait would leave the match sitting there
	// with nothing coming. Zero means now, not never.
	if (Delay <= 0.0f)
	{
		StartNextWave();
		return;
	}

	GetWorldTimerManager().SetTimer(
		NextWaveTimer, this, &AArenaShooterGameMode::StartNextWave, Delay, false);
}

void AArenaShooterGameMode::HandleBaseDestroyed()
{
	EndMatch(EArenaShooterMatchState::Lost);
}

void AArenaShooterGameMode::HandlePlayerDeath()
{
	EndMatch(EArenaShooterMatchState::Lost);
}

void AArenaShooterGameMode::EndMatch(EArenaShooterMatchState Result)
{
	// Once. Losing the base and then dying is one defeat, and a win cannot be overwritten by what
	// the enemies do on their way to a stop.
	if (State != EArenaShooterMatchState::InProgress)
	{
		return;
	}

	State = Result;

	GetWorldTimerManager().ClearTimer(NextWaveTimer);
	StopEnemies();

	UE_LOG(LogArenaShooterMatch, Log, TEXT("Match over: %s."),
		Result == EArenaShooterMatchState::Won ? TEXT("won") : TEXT("lost"));
}

void AArenaShooterGameMode::StopEnemies()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// The bodies stay where they are, and so does the base. Only the minds stop, which leaves
	// something for a defeat screen to sit over.
	int32 Stopped = 0;
	for (TActorIterator<AArenaShooterEnemyController> It(World); It; ++It)
	{
		It->StopBehaviour();
		++Stopped;
	}

	UE_LOG(LogArenaShooterMatch, Log, TEXT("Stopped %d enemies."), Stopped);
}
