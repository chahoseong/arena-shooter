// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/ArenaShooterMeleeAttackComponent.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/PrimitiveComponent.h"
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
	return IsInReach(Target) && IsReady();
}

bool UArenaShooterMeleeAttackComponent::IsInReach(const AActor* Target) const
{
	const AActor* Owner = GetOwner();

	// IsValid, not a null check. A destroyed actor leaves this pointer standing until garbage
	// collection gets to it, and asking a torn-down actor where it is reads freed memory.
	if (!IsValid(Target) || Owner == nullptr)
	{
		return false;
	}

	const FVector ToTarget = Target->GetActorLocation() - Owner->GetActorLocation();
	return ToTarget.Size2D() <= GetReachTo(*Target);
}

bool UArenaShooterMeleeAttackComponent::IsReady() const
{
	const UWorld* World = GetWorld();
	return World != nullptr && World->GetTimeSeconds() - LastAttackStartTime >= AttackInterval;
}

float UArenaShooterMeleeAttackComponent::GetReachTo(const AActor& Target) const
{
	// Both bodies come out of the measurement, because AttackRange is the gap between them and the
	// distance it gets compared against is between their origins. Leaving either radius in makes
	// the same number mean a different fight depending on how wide the two happen to be -- which is
	// what the base, several times a character's girth, made impossible to ignore.
	const AActor* Owner = GetOwner();
	return AttackRange + (Owner ? GetPlanarRadius(*Owner) : 0.0f) + GetPlanarRadius(Target);
}

float UArenaShooterMeleeAttackComponent::GetPlanarRadius(const AActor& Actor)
{
	// Width in the plane, not AActor::GetSimpleCollisionRadius: that one comes back through
	// CalcBoundingCylinder with height folded in, which reads a tall base as far wider than it
	// stands and a character as twice its own girth. The swing is judged flat, so the only
	// question is how far the body goes out sideways.
	if (const UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(Actor.GetRootComponent()))
	{
		const FVector Extent = Root->Bounds.BoxExtent;
		return FMath::Max(Extent.X, Extent.Y);
	}

	return 0.0f;
}

bool UArenaShooterMeleeAttackComponent::StartAttack(AActor* Target)
{
	UWorld* World = GetWorld();
	ACharacter* Owner = GetOwner<ACharacter>();
	if (!IsValid(Target) || World == nullptr || Owner == nullptr || AttackMontage == nullptr)
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
	// The swing outlives what it was aimed at. A base shot down mid-swing, or an enemy removed while
	// one was in flight, is destroyed but still pointed at from here until garbage collection runs,
	// so the question is whether it is still valid rather than whether it is still set.
	if (World == nullptr || Owner == nullptr || !IsValid(AttackTarget))
	{
		return;
	}

	const FVector Origin = Owner->GetActorLocation();

	// Taken from the actor, not the mesh: the mesh carries a yaw offset to face its model forwards.
	const FVector Forward = Owner->GetActorForwardVector();

	// Read once and kept, because the swing can be the end of what it hits: applying damage below
	// runs the target's death inline, and a base that dies there is destroyed before this function
	// returns. Everything after that point works from the numbers, not from the actor.
	const FVector TargetLocation = AttackTarget->GetActorLocation();

	// Flat, because the arc is about stepping aside rather than about height.
	const FVector ToTarget = TargetLocation - Origin;
	const FVector Direction = ToTarget.GetSafeNormal2D();

	const float Reach = GetReachTo(*AttackTarget);

	const bool bInRange = ToTarget.Size2D() <= Reach;
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
		DrawDebugCone(World, Origin, Forward, Reach, Angle, Angle, 16, Verdict, false, 2.0f);

		// Where the target stood when the swing was decided. On a miss this is the whole story:
		// a line outside the arc means it stepped aside, one past the tip means it backed off.
		DrawDebugLine(World, Origin, TargetLocation, Verdict, false, 2.0f, 0, 2.0f);
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
