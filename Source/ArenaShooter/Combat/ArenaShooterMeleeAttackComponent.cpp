// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/ArenaShooterMeleeAttackComponent.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "MotionWarpingComponent.h"

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
	ACharacter* Owner = GetOwner<ACharacter>();
	if (Target == nullptr || World == nullptr || Owner == nullptr || AttackMontage == nullptr)
	{
		return false;
	}

	// Before the montage, because the warping window can open on its very first frame and a modifier
	// that cannot find its target by name gives up for the rest of the montage.
	//
	// Where the target stands now, not a component to follow: committing to that spot is what makes
	// stepping aside during the wind-up a miss rather than something the swing tracks. The rotation
	// argument goes unused, since the modifier recomputes facing from the location each frame.
	if (UMotionWarpingComponent* Warping = Owner->FindComponentByClass<UMotionWarpingComponent>())
	{
		Warping->AddOrUpdateWarpTargetFromLocationAndRotation(
			WarpTargetName, Target->GetActorLocation(), FRotator::ZeroRotator);
	}

	// Zero means the montage could not start, usually because its slot is not one the animation
	// blueprint evaluates. Reporting failure keeps the tree from waiting out a swing that is not
	// happening.
	if (Owner->PlayAnimMontage(AttackMontage) <= 0.0f)
	{
		return false;
	}

	AttackTarget = Target;
	bIsAttacking = true;
	bHitResolved = false;
	LastAttackStartTime = World->GetTimeSeconds();

	// Asked rather than timed, so that a montage cut short is noticed rather than waited out.
	if (UAnimInstance* AnimInstance = Owner->GetMesh() ? Owner->GetMesh()->GetAnimInstance() : nullptr)
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &UArenaShooterMeleeAttackComponent::HandleMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);
	}

	return true;
}

void UArenaShooterMeleeAttackComponent::HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	CancelAttack();
}

void UArenaShooterMeleeAttackComponent::ResolveHit()
{
	// A swing that was dropped must not still land. Death drops it, and the death montage does not
	// cut the attack montage short, so its hit notify can arrive after the enemy is gone.
	//
	// bIsAttacking is not cleared here: the swing carries on to the end of the montage, and the tree
	// is waiting on that. Only the decision is spent.
	if (!bIsAttacking || bHitResolved)
	{
		return;
	}

	bHitResolved = true;

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

	if (AActor* Owner = GetOwner())
	{
		if (UMotionWarpingComponent* Warping = Owner->FindComponentByClass<UMotionWarpingComponent>())
		{
			Warping->RemoveWarpTarget(WarpTargetName);
		}
	}
}
