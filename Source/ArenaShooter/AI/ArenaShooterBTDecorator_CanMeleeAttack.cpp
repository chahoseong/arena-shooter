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

	// A distance raises no event, so the condition is polled and the tree asked to reconsider only
	// when the answer changes -- the same shape UBTDecorator_ConeCheck uses.
	//
	// Lower priority alone: coming within reach has to interrupt the approach running below, but
	// leaving it does not need an abort of its own, because the attack task gives the branch up as
	// soon as the target is out of reach. Adding Self would put a second way to restart this branch
	// next to that one, and two of them fighting is what makes a tree flicker.
	bAllowAbortLowerPri = true;
	bAllowAbortChildNodes = false;
	bAllowAbortNone = false;
	FlowAbortMode = EBTFlowAbortMode::LowerPriority;

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
	return Melee->IsInReach(Target);
}

void UArenaShooterBTDecorator_CanMeleeAttack::InitializeMemory(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
	FArenaShooterCanMeleeAttackMemory* Memory = CastInstanceNodeMemory<FArenaShooterCanMeleeAttackMemory>(NodeMemory);
	Memory->bLastResult = false;
}

void UArenaShooterBTDecorator_CanMeleeAttack::OnBecomeRelevant(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Seeded from the condition as it stands, not from false. Starting out disagreeing with reality
	// makes the first tick report a change that never happened, and that spurious change asks the
	// tree to reconsider -- once every time this becomes relevant, which is once per swing.
	FArenaShooterCanMeleeAttackMemory* Memory = CastInstanceNodeMemory<FArenaShooterCanMeleeAttackMemory>(NodeMemory);
	Memory->bLastResult = CalculateRawConditionValue(OwnerComp, NodeMemory);
}

void UArenaShooterBTDecorator_CanMeleeAttack::TickNode(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FArenaShooterCanMeleeAttackMemory* Memory = CastInstanceNodeMemory<FArenaShooterCanMeleeAttackMemory>(NodeMemory);

	const bool bResult = CalculateRawConditionValue(OwnerComp, NodeMemory);
	if (bResult != Memory->bLastResult)
	{
		Memory->bLastResult = bResult;
		OwnerComp.RequestExecution(this);
	}
}

FString UArenaShooterBTDecorator_CanMeleeAttack::GetStaticDescription() const
{
	return FString::Printf(TEXT("Can melee attack %s"), *BlackboardKey.SelectedKeyName.ToString());
}
