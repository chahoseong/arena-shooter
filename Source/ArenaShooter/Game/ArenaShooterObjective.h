// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArenaShooterObjective.generated.h"

class UArenaShooterHealthComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FArenaShooterObjectiveDestroyedSignature);

/**
 * What a wave is really about: the thing the player has to shoot down before the next one starts.
 * Enemies are what gets in the way of doing it.
 *
 * It does not fight back. Whatever pressure there is comes from the enemies, and from not being
 * able to take all the time in the world over it.
 *
 * The mesh and the tuning are set in a Blueprint subclass.
 */
UCLASS()
class ARENASHOOTER_API AArenaShooterObjective : public AActor
{
	GENERATED_BODY()

public:
	AArenaShooterObjective();

	/**
	 * Fires once, when the objective has been shot down. The wave sequence waits on this.
	 *
	 * Not OnDestroyed: AActor already has one of those, and it fires for any removal at all.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Objective")
	FArenaShooterObjectiveDestroyedSignature OnObjectiveDestroyed;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleDeath();

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Objective")
	TObjectPtr<UArenaShooterHealthComponent> Health;
};
