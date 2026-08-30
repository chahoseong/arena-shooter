// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/ArenaShooterBTDecorator_CanMeleeAttack.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Combat/ArenaShooterMeleeAttackComponent.h"
#include "GameFramework/Pawn.h"

UArenaShooterBTDecorator_CanMeleeAttack::UArenaShooterBTDecorator_CanMeleeAttack()
{
	NodeName = TEXT("Can Melee Attack");

	INIT_DECORATOR_NODE_NOTIFY_FLAGS();

	// The key only says who to measure against; the answer turns on a distance and a cooldown, and
	// neither of those raises an event. There is nothing to abort on, so the Selector re-asking on
	// its way back round to the top is the whole mechanism.
	bAllowAbortLowerPri = false;
	bAllowAbortChildNodes = false;
	bAllowAbortNone = false;
	FlowAbortMode = EBTFlowAbortMode::None;

	BlackboardKey.AddObjectFilter(
		this, GET_MEMBER_NAME_CHECKED(UArenaShooterBTDecorator_CanMeleeAttack, BlackboardKey), AActor::StaticClass());
}

bool UArenaShooterBTDecorator_CanMeleeAttack::CalculateRawConditionValue(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	const AAIController* Controller = OwnerComp.GetAIOwner();
	const APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	if (Blackboard == nullptr || Pawn == nullptr)
	{
		return false;
	}

	const UArenaShooterMeleeAttackComponent* Melee =
		Pawn->FindComponentByClass<UArenaShooterMeleeAttackComponent>();
	if (Melee == nullptr)
	{
		return false;
	}

	// A null target is a legitimate answer of "no", not a mistake: this decorator is also what
	// keeps the attack branch shut before anything has been noticed.
	const AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(BlackboardKey.SelectedKeyName));
	return Melee->CanAttack(Target);
}

FString UArenaShooterBTDecorator_CanMeleeAttack::GetStaticDescription() const
{
	return FString::Printf(TEXT("Can melee attack %s"), *BlackboardKey.SelectedKeyName.ToString());
}
