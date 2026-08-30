// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/ArenaShooterMeleeAttackComponent.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UArenaShooterMeleeAttackComponent::UArenaShooterMeleeAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UArenaShooterMeleeAttackComponent::CanAttack(const AActor* Target) const
{
	const AActor* Owner = GetOwner();
	const UWorld* World = GetWorld();
	if (Target == nullptr || Owner == nullptr || World == nullptr)
	{
		return false;
	}

	if (World->GetTimeSeconds() - LastAttackStartTime < AttackInterval)
	{
		return false;
	}

	const FVector ToTarget = Target->GetActorLocation() - Owner->GetActorLocation();
	return ToTarget.Size2D() <= AttackRange;
}

bool UArenaShooterMeleeAttackComponent::StartAttack(AActor* Target)
{
	UWorld* World = GetWorld();
	if (Target == nullptr || World == nullptr)
	{
		return false;
	}

	AttackTarget = Target;
	bIsAttacking = true;
	LastAttackStartTime = World->GetTimeSeconds();

	return true;
}

void UArenaShooterMeleeAttackComponent::ResolveHit()
{
	// A swing that was dropped must not still land. Death drops it, and the death montage does not
	// cut the attack montage short, so its hit notify can arrive after the enemy is gone.
	if (!bIsAttacking)
	{
		return;
	}

	// One decision per swing, whatever else happens to the montage afterwards.
	bIsAttacking = false;

	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();
	if (World == nullptr || Owner == nullptr || AttackTarget == nullptr)
	{
		return;
	}

	const FVector Origin = Owner->GetActorLocation();

	// Taken from the actor, not the mesh: the mesh carries a yaw offset to face its model forwards.
	const FVector Forward = Owner->GetActorForwardVector();

	// Flat, because the arc is about stepping aside rather than about height.
	const FVector ToTarget = AttackTarget->GetActorLocation() - Origin;
	const FVector Direction = ToTarget.GetSafeNormal2D();

	const bool bInRange = ToTarget.Size2D() <= AttackRange;
	const bool bInAngle = FVector::DotProduct(Forward, Direction)
		>= FMath::Cos(FMath::DegreesToRadians(AttackAngle));
	const bool bHit = bInRange && bInAngle;

	if (bHit)
	{
		UGameplayStatics::ApplyDamage(AttackTarget, Damage, Owner->GetInstigatorController(), Owner, nullptr);
	}

	if (bDrawAttackDebug)
	{
		const FColor Verdict = bHit ? FColor::Green : FColor::Red;
		const float Angle = FMath::DegreesToRadians(AttackAngle);

		// The arc as it was tested, opening along the facing the swing committed to.
		DrawDebugCone(World, Origin, Forward, AttackRange, Angle, Angle, 16, Verdict, false, 2.0f);

		// Where the target stood when the swing was decided. On a miss this is the whole story:
		// a line outside the arc means it stepped aside, one past the tip means it backed off.
		DrawDebugLine(World, Origin, AttackTarget->GetActorLocation(), Verdict, false, 2.0f, 0, 2.0f);
	}
}

void UArenaShooterMeleeAttackComponent::CancelAttack()
{
	// Reached from the character's death handling and from the behaviour tree aborting its task,
	// in no fixed order, so dropping nothing has to be a no-op rather than a mistake.
	bIsAttacking = false;
	AttackTarget = nullptr;
}
