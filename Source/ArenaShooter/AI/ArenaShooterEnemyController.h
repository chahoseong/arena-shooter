// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ArenaShooterEnemyController.generated.h"

class UBehaviorTree;
struct FAIStimulus;

/**
 * Drives an enemy. Decides what the blackboard says; the behaviour tree decides what to do about
 * it. Turning attention to the player is the controller's job because it comes from senses and
 * damage, neither of which the tree can observe.
 *
 * The behaviour tree and the blackboard key names are set in a Blueprint subclass. Sight is tuned
 * on the Perception component there, which already exposes its own configuration.
 */
UCLASS()
class ARENASHOOTER_API AArenaShooterEnemyController : public AAIController
{
	GENERATED_BODY()

public:
	AArenaShooterEnemyController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

private:
	UFUNCTION()
	void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void HandleDamaged(AActor* DamageCauser);

	UFUNCTION()
	void HandleDeath();

	/** Ignores anything that is not the player, so enemies do not turn on each other. */
	void StartPursuing(AActor* Target);

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	/**
	 * Names of the blackboard entries this controller writes. They are the contract with a
	 * hand-authored asset, and a mismatch is silent: the tree simply never gets a value.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName TargetActorKey = TEXT("TargetActor");

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName DestinationKey = TEXT("Destination");
};
