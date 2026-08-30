// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_Blackboard.h"
#include "ArenaShooterBTDecorator_CanMeleeAttack.generated.h"

/**
 * Whether the pawn would start a swing at the actor named by the key. The condition itself lives on
 * the melee component, so that reach and cadence are tuned in one place rather than half here and
 * half in the Blueprint.
 *
 * Exists so that the reason a branch was skipped is visible in the behaviour tree debugger. The
 * attack task could refuse to run instead, but then the condition would be invisible.
 */
UCLASS(meta = (DisplayName = "Can Melee Attack"))
class ARENASHOOTER_API UArenaShooterBTDecorator_CanMeleeAttack : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()

public:
	UArenaShooterBTDecorator_CanMeleeAttack();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	virtual FString GetStaticDescription() const override;
};
