// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/ArenaShooterEnemyController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "Characters/ArenaShooterEnemyCharacter.h"
#include "Combat/ArenaShooterHealthComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

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

	if (BehaviorTree)
	{
		RunBehaviorTree(BehaviorTree);
	}
}

void AArenaShooterEnemyController::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// Losing sight is reported through this same event. Ignoring it is what makes the chase
	// survive the player stepping behind cover.
	if (!Stimulus.WasSuccessfullySensed())
	{
		return;
	}

	StartPursuing(Actor);
}

void AArenaShooterEnemyController::HandleDamaged(AActor* DamageCauser)
{
	StartPursuing(DamageCauser);
}

void AArenaShooterEnemyController::HandleDeath()
{
	// The pawn outlives this moment by the length of the death montage. Without this the tree
	// keeps running over the corpse and turns it to face the player.
	if (BrainComponent)
	{
		BrainComponent->StopLogic(TEXT("Died"));
	}
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

	// Never cleared. Once the enemy has seen or felt the player, it keeps coming.
	if (Blackboard)
	{
		Blackboard->SetValueAsObject(TargetActorKey, Target);
	}
}
