#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "ArenaShooterBTTask_MeleeAttack.generated.h"

UCLASS()
class ARENASHOOTER_API UArenaShooterBTTask_MeleeAttack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UArenaShooterBTTask_MeleeAttack();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
