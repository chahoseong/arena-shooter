// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/ArenaShooterHealthComponent.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY(LogArenaShooterHealth);

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

	// Above the subtraction, not below it: OnDamaged fires after health moves, so a check placed
	// later would still announce a hit that did nothing. Stamping the time here also collapses hits
	// that arrive in the same frame, which three enemies around one player produce regularly.
	const UWorld* World = GetWorld();
	const double Now = World ? World->GetTimeSeconds() : 0.0;
	if (Now - LastDamagedTime < InvulnerabilityDuration)
	{
		return;
	}
	LastDamagedTime = Now;

	Health = FMath::Max(Health - Damage, 0.0f);

	if (bLogHealthChanges)
	{
		UE_LOG(LogArenaShooterHealth, Log, TEXT("%s took %.0f from %s, %.0f/%.0f left"),
			*GetNameSafe(GetOwner()), Damage, *GetNameSafe(DamageCauser), Health, MaxHealth);
	}

	OnDamaged.Broadcast(DamageCauser);

	if (IsDead())
	{
		if (bLogHealthChanges)
		{
			UE_LOG(LogArenaShooterHealth, Log, TEXT("%s died"), *GetNameSafe(GetOwner()));
		}

		OnDeath.Broadcast();
	}
}
