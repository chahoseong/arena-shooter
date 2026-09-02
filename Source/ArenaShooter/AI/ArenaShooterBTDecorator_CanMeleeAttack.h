// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_Blackboard.h"
#include "ArenaShooterBTDecorator_CanMeleeAttack.generated.h"

/** Per-instance, because behaviour tree nodes are shared between every enemy running the tree. */
struct FArenaShooterCanMeleeAttackMemory
{
	/** What the condition said last tick, so that only a change asks the tree to reconsider. */
	bool bLastResult = false;
};

/**
 * Whether the pawn is close enough to swing at the actor named by the key. The measurement itself
 * lives on the melee component, so that reach is defined in one place rather than half here and
 * half in the Blueprint.
 *
 * This is what ends the approach. It watches while the enemy is still walking and interrupts the
 * moment the target comes within reach, which is why it asks about reach alone and leaves the
 * cadence to the attack task: a condition that also went false between swings would hand the
 * branch back to the approach, and the enemy would close the rest of the way during the wait.
 *
 * Stopping here rather than at a distance set on the move task is what makes the two targets
 * behave alike. The path to something the enemy cannot stand on -- the base, which leaves a hole
 * in the navmesh -- is a partial one, and a partial path spends its acceptance radius on the gap
 * it could not cross (PathFollowingComponent.cpp:1378), so that distance stops being a dial at all.
 */
UCLASS(meta = (DisplayName = "Can Melee Attack"))
class ARENASHOOTER_API UArenaShooterBTDecorator_CanMeleeAttack : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()

public:
	UArenaShooterBTDecorator_CanMeleeAttack();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	virtual FString GetStaticDescription() const override;

	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FArenaShooterCanMeleeAttackMemory); }

	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
		EBTMemoryInit::Type InitType) const override;

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
