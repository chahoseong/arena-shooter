// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/ArenaShooterEnemyController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "Characters/ArenaShooterEnemyCharacter.h"
#include "Combat/ArenaShooterHealthComponent.h"
#include "Combat/ArenaShooterMeleeAttackComponent.h"
#include "Engine/World.h"
#include "Game/ArenaShooterObjective.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISenseConfig_Sight.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogArenaShooterAI);

AArenaShooterEnemyController::AArenaShooterEnemyController()
{
	UAIPerceptionComponent* Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));
	UAISenseConfig_Sight* Sight = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight"));

	// Every one of these defaults to false, which would leave the enemy blind to everything.
	// Nothing here sets up teams, so the player counts as neutral rather than as an enemy.
	//
	// This is the reason the sense is built here instead of being added to the Perception
	// component in the Blueprint: forgetting it costs an enemy that never notices anything, with
	// no error to go on.
	Sight->DetectionByAffiliation = FAISenseAffiliationFilter(true, true, true);

	// Starting points. Tuned on the Perception component in the Blueprint subclass, which exposes
	// its sense configuration for editing, so there is nothing to mirror here.
	//
	// This radius is the dial for how readily an enemy gives up on the base: a wide one and the
	// player only has to be visible for the base to be safe, a narrow one and they have to be near.
	Sight->SightRadius = 1500.0f;
	Sight->LoseSightRadius = 1500.0f;
	Sight->PeripheralVisionAngleDegrees = 60.0f;

	Perception->ConfigureSense(*Sight);
	Perception->SetDominantSense(Sight->GetSenseImplementation());
	SetPerceptionComponent(*Perception);
}

void AArenaShooterEnemyController::BeginPlay()
{
	Super::BeginPlay();

	if (UAIPerceptionComponent* Perception = GetAIPerceptionComponent())
	{
		Perception->OnTargetPerceptionUpdated.AddDynamic(
			this, &AArenaShooterEnemyController::HandleTargetPerceptionUpdated);
	}

	// Found rather than handed over. There is one base and it does not move, and looking it up here
	// is what lets an enemy be dropped into the level and verified without a wave to spawn it.
	DefendedBase = UGameplayStatics::GetActorOfClass(this, AArenaShooterObjective::StaticClass());
	if (DefendedBase)
	{
		DefendedBase->OnDestroyed.AddDynamic(this, &AArenaShooterEnemyController::HandleBaseDestroyed);
	}
	else
	{
		UE_LOG(LogArenaShooterAI, Warning,
			TEXT("%s found no base in the level; this enemy will only move once it notices the player."),
			*GetName());
	}
}

void AArenaShooterEnemyController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AArenaShooterEnemyCharacter* Enemy = Cast<AArenaShooterEnemyCharacter>(InPawn);
	if (Enemy == nullptr)
	{
		return;
	}

	// Damage is not reported to perception on its own, and there is a health component already
	// receiving it, so being shot is heard from there rather than through a second sense.
	if (UArenaShooterHealthComponent* Health = Enemy->FindComponentByClass<UArenaShooterHealthComponent>())
	{
		Health->OnDamaged.AddDynamic(this, &AArenaShooterEnemyController::HandleDamaged);
		Health->OnDeath.AddDynamic(this, &AArenaShooterEnemyController::HandleDeath);
	}

	// Kept rather than looked up each time: how close to get is derived from how far it reaches.
	Melee = Enemy->FindComponentByClass<UArenaShooterMeleeAttackComponent>();

	if (BehaviorTree)
	{
		RunBehaviorTree(BehaviorTree);
	}

	// Only now, and not in BeginPlay: there is no blackboard until the tree is running, and
	// possession happens after the controller has already begun play.
	RefreshTarget();
}

void AArenaShooterEnemyController::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (Stimulus.WasSuccessfullySensed())
	{
		GetWorldTimerManager().ClearTimer(ForgetTargetTimer);
		StartPursuing(Actor);
		return;
	}

	// Losing sight does not drop the target, it starts the clock. Turning back the instant a corner
	// intervenes would make cover far stronger than it should be, and would leave an enemy on the
	// edge of vision switching every time the test flipped.
	if (PursuedTarget == Actor)
	{
		StartForgetting();
	}
}

void AArenaShooterEnemyController::HandleDamaged(AActor* DamageCauser)
{
	StartPursuing(DamageCauser);

	// Being shot from somewhere unseen leaves no sight event to expire later, so the clock has to
	// start here. If the player is visible after all, the timer finds that out when it fires.
	if (PursuedTarget == DamageCauser)
	{
		StartForgetting();
	}
}

void AArenaShooterEnemyController::HandleDeath()
{
	// The pawn outlives this moment by the length of the death montage. Without this the tree
	// keeps running over the corpse and turns it to face the player.
	StopBehaviour();
}

void AArenaShooterEnemyController::StopBehaviour()
{
	if (BrainComponent)
	{
		BrainComponent->StopLogic(TEXT("Stopped"));
	}

	// The tree is what asked for the move, but the move outlives it: stopping the logic alone leaves
	// the pawn coasting to wherever it was already heading.
	StopMovement();
}

void AArenaShooterEnemyController::HandleBaseDestroyed(AActor* DestroyedActor)
{
	// Leaving the entry pointing at a destroyed actor would have the tree go on pathing to it. What
	// should happen once the base is gone is the match's business, not this controller's.
	DefendedBase = nullptr;
	RefreshTarget();
}

void AArenaShooterEnemyController::StartPursuing(AActor* Target)
{
	// Affiliation is set to detect everything, so enemies see each other too. Only the player is
	// worth turning on.
	const APawn* TargetPawn = Cast<APawn>(Target);
	if (TargetPawn == nullptr || !TargetPawn->IsPlayerControlled())
	{
		return;
	}

	PursuedTarget = Target;
	RefreshTarget();
}

void AArenaShooterEnemyController::StartForgetting()
{
	if (TargetMemoryDuration <= 0.0f)
	{
		ForgetTarget();
		return;
	}

	GetWorldTimerManager().SetTimer(
		ForgetTargetTimer, this, &AArenaShooterEnemyController::ForgetTarget, TargetMemoryDuration, false);
}

void AArenaShooterEnemyController::ForgetTarget()
{
	// The damage path starts this clock without knowing whether the player can be seen, so the one
	// question that settles both paths is asked here. Still being sensed means the sight event that
	// ends the chase has not arrived yet, and it will start a fresh clock when it does.
	if (IsPursuedTargetSensed())
	{
		return;
	}

	PursuedTarget = nullptr;
	RefreshTarget();
}

bool AArenaShooterEnemyController::IsPursuedTargetSensed() const
{
	const UAIPerceptionComponent* Perception = GetAIPerceptionComponent();
	return PursuedTarget != nullptr && Perception != nullptr
		&& Perception->HasActiveStimulus(*PursuedTarget, UAISense::GetSenseID<UAISense_Sight>());
}

void AArenaShooterEnemyController::RefreshTarget()
{
	if (Blackboard == nullptr)
	{
		return;
	}

	AActor* NewTarget = PursuedTarget != nullptr ? PursuedTarget.Get() : DefendedBase.Get();
	const AActor* OldTarget = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey));
	if (NewTarget == OldTarget)
	{
		return;
	}

	// Before the target, not after: changing the target is what makes the tree abandon the move it
	// is on and ask for a new one, and the new one has to read a distance that already suits what
	// it is now heading for.
	WriteApproachRadius(NewTarget);
	Blackboard->SetValueAsObject(TargetActorKey, NewTarget);

	UE_LOG(LogArenaShooterAI, Log, TEXT("%s: target %s -> %s"),
		*GetName(),
		OldTarget ? *OldTarget->GetName() : TEXT("none"),
		NewTarget ? *NewTarget->GetName() : TEXT("none"));
}

void AArenaShooterEnemyController::WriteApproachRadius(const AActor* Target)
{
	if (Blackboard == nullptr || Melee == nullptr || Target == nullptr)
	{
		return;
	}

	// Where the enemy comes to rest is the acceptance radius plus a fraction over its own, measured
	// from whatever the path ended at. That holds for the base as much as for the player: the path
	// to the base stops at the edge of the hole its collision leaves in the navmesh, and the same
	// distance is then taken back off the acceptance radius (PathFollowingComponent.cpp:1376), so
	// the detour cancels and one formula covers both.
	//
	// So the distance to ask for is the reach, less enough to leave the two of them apart rather
	// than overlapping when the swing starts.
	//
	// Recomputed per target, because reach depends on how wide the target is, and derived here
	// rather than typed into the tree because the two have to agree: a number in the asset would go
	// on looking correct after AttackRange was retuned, and enemies would close to the wrong
	// distance with nothing on screen to say why.
	const float Reach = Melee->GetReachTo(*Target);
	Blackboard->SetValueAsFloat(ApproachRadiusKey, FMath::Max(0.0f, Reach - ApproachMargin));
}
