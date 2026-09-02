// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArenaShooterEnemySpawnPoint.generated.h"

class AArenaShooterEnemyCharacter;
class UBillboardComponent;

ARENASHOOTER_API DECLARE_LOG_CATEGORY_EXTERN(LogArenaShooterSpawn, Log, All);

/**
 * A patch of ground an enemy can come out of. Placed in the level, one per patch, because that is
 * how a place is authored: you drag it where you want it and the transform is the answer.
 *
 * Knows only how to put one enemy down somewhere it can stand. Which enemy, how many, and when are
 * all the caller's business.
 */
UCLASS()
class ARENASHOOTER_API AArenaShooterEnemySpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	AArenaShooterEnemySpawnPoint();

	/** Puts one enemy down within reach of here. Null when there was nowhere for it to stand. */
	AArenaShooterEnemyCharacter* SpawnEnemy(TSubclassOf<AArenaShooterEnemyCharacter> EnemyClass);

	virtual void Tick(float DeltaSeconds) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }

private:
	/**
	 * An actor with no root component cannot hold a position: the transform is dropped and every
	 * query answers with the origin. The billboard also gives the point something to click on, since
	 * it has nothing else to draw.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Spawning")
	TObjectPtr<UBillboardComponent> Marker;

	/** How far from here an enemy may appear. */
	UPROPERTY(EditAnywhere, Category = "Spawning", meta = (ClampMin = "0.0"))
	float Radius = 500.0f;

};
