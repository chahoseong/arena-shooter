#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "ArenaShooterBTTask_MeleeAttack.generated.h"

/** Per-instance, because behaviour tree nodes are shared between every enemy running the tree. */
struct FArenaShooterMeleeAttackMemory
{
	/** Whether the swing this run is waiting on has actually begun, or the cadence is still owed. */
	bool bSwingStarted = false;
};

/**
 * Gets a swing in at the actor named by the key, and holds the branch while the cadence is owed
 * rather than failing. Waiting here is deliberate: failing would hand the branch back to the
 * approach, which would spend every gap between swings closing what is left of the distance.
 */
UCLASS()
class ARENASHOOTER_API UArenaShooterBTTask_MeleeAttack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UArenaShooterBTTask_MeleeAttack();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FArenaShooterMeleeAttackMemory); }

	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
		EBTMemoryInit::Type InitType) const override;
	
protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
