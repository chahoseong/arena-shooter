// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/ArenaShooterHealthComponent.h"

#include "GameFramework/Actor.h"

UArenaShooterHealthComponent::UArenaShooterHealthComponent()
{
	// Nothing to advance per frame; health only changes when damage arrives.
	PrimaryComponentTick.bCanEverTick = false;
}

void UArenaShooterHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;

	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UArenaShooterHealthComponent::HandleTakeAnyDamage);
	}
}

void UArenaShooterHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage,
	const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	// A corpse takes no further damage, and must not announce a second death: whatever the first
	// one started, such as a montage and a removal timer, would be restarted by it.
	if (IsDead() || Damage <= 0.0f)
	{
		return;
	}

	Health = FMath::Max(Health - Damage, 0.0f);

	OnDamaged.Broadcast(DamageCauser);

	if (IsDead())
	{
		OnDeath.Broadcast();
	}
}
