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
	
	FArenaShooterMeleeAttackMemory* Memory = CastInstanceNodeMemory<FArenaShooterMeleeAttackMemory>(NodeMemory);
	Memory->bSwingStarted = false;

	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(BlackboardKey.SelectedKeyName));
	if (!Melee->IsInReach(Target))
	{
		return EBTNodeResult::Failed;
	}

	// Holding rather than swinging straight away when the cadence is still owed. The tick below
	// starts it the moment it is due, and until then the enemy stands where it is.
	if (Melee->IsReady())
	{
		if (!Melee->StartAttack(Target))
		{
			return EBTNodeResult::Failed;
		}

		Memory->bSwingStarted = true;
	}

	return EBTNodeResult::InProgress;
}

void UArenaShooterBTTask_MeleeAttack::InitializeMemory(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
	FArenaShooterMeleeAttackMemory* Memory = CastInstanceNodeMemory<FArenaShooterMeleeAttackMemory>(NodeMemory);
	Memory->bSwingStarted = false;
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
	
	FArenaShooterMeleeAttackMemory* Memory = CastInstanceNodeMemory<FArenaShooterMeleeAttackMemory>(NodeMemory);

	// A swing in flight is simply waited out, as before: the tree is what the attack reports back to.
	if (Memory->bSwingStarted)
	{
		if (!Melee->IsAttacking())
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}

		return;
	}

	// Otherwise the cadence is still owed. Give up only if the target has gone or moved out of
	// reach, which is also what lets a target swapped mid-wait be picked up.
	const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* Target = Blackboard ? Cast<AActor>(Blackboard->GetValueAsObject(BlackboardKey.SelectedKeyName)) : nullptr;
	if (!Melee->IsInReach(Target))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (Melee->IsReady())
	{
		if (Melee->StartAttack(Target))
		{
			Memory->bSwingStarted = true;
		}
		else
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		}
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
