#include "ArenaShooterBTTask_MeleeAttack.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Combat/ArenaShooterMeleeAttackComponent.h"

UArenaShooterBTTask_MeleeAttack::UArenaShooterBTTask_MeleeAttack()
{
	NodeName = TEXT("Melee Attack");
	
	INIT_TASK_NODE_NOTIFY_FLAGS();
	
	BlackboardKey.AddObjectFilter(
		this, GET_MEMBER_NAME_CHECKED(ThisClass, BlackboardKey), AActor::StaticClass());
}

EBTNodeResult::Type UArenaShooterBTTask_MeleeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	const AAIController* Controller = OwnerComp.GetAIOwner();
	const APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	if (Blackboard == nullptr || Pawn == nullptr)
	{
		return EBTNodeResult::Failed;
	}
	
	UArenaShooterMeleeAttackComponent* Melee =
		Pawn->FindComponentByClass<UArenaShooterMeleeAttackComponent>();
	if (Melee == nullptr)
	{
		return EBTNodeResult::Failed;
	}
	
	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(BlackboardKey.SelectedKeyName));
	return Melee->StartAttack(Target) ? EBTNodeResult::InProgress : EBTNodeResult::Failed;
}

void UArenaShooterBTTask_MeleeAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	
	const AAIController* Controller = OwnerComp.GetAIOwner();
	const APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	if (Pawn == nullptr)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	UArenaShooterMeleeAttackComponent* Melee =
		Pawn->FindComponentByClass<UArenaShooterMeleeAttackComponent>();
	if (Melee == nullptr)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	if (!Melee->IsAttacking())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UArenaShooterBTTask_MeleeAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::AbortTask(OwnerComp, NodeMemory);
	
	const AAIController* Controller = OwnerComp.GetAIOwner();
	const APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	if (Pawn == nullptr)
	{
		return Result;
	}
	
	UArenaShooterMeleeAttackComponent* Melee =
		Pawn->FindComponentByClass<UArenaShooterMeleeAttackComponent>();
	if (Melee == nullptr)
	{
		return Result;
	}
	
	Melee->CancelAttack();
	
	return Result;
}
