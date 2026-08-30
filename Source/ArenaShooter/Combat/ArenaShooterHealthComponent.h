// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ArenaShooterHealthComponent.generated.h"

/** Reports who dealt the damage, so a listener can turn on whoever shot it. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FArenaShooterDamagedSignature, AActor*, DamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FArenaShooterDeathSignature);

/**
 * How much punishment an actor takes before it dies. Subscribes to the owner's damage event
 * rather than exposing a damage entry point of its own, so anything that already deals damage
 * through the engine reaches it without knowing this component exists.
 *
 * Attached to whatever needs health. Nothing here is specific to the enemy, so the player can
 * share it once it needs one; that is why it lives here and not on a common character base.
 */
UCLASS(ClassGroup = (ArenaShooter), meta = (BlueprintSpawnableComponent))
class ARENASHOOTER_API UArenaShooterHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UArenaShooterHealthComponent();

	/** Fires on every hit that takes health off, including the one that kills. */
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FArenaShooterDamagedSignature OnDamaged;

	/** Fires once, after the killing blow's OnDamaged. */
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FArenaShooterDeathSignature OnDeath;

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return Health <= 0.0f; }

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
		class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(EditDefaultsOnly, Category = "Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	/** Set from MaxHealth on BeginPlay so the Blueprint value is the one that counts. */
	float Health = 0.0f;
};
