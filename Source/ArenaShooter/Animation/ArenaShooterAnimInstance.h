// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ArenaShooterAnimInstance.generated.h"

class ACharacter;

/**
 * Locomotion data for an animation blueprint to read. Nothing here is specific to the player,
 * so the enemy can share it once it exists.
 */
UCLASS()
class ARENASHOOTER_API UArenaShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	/** Horizontal speed. Vertical speed is left out so that jumping does not read as running. */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float Speed = 0.0f;

	/** Direction of travel relative to where the character faces, -180..180. */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float Direction = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bIsInAir = false;

private:
	UPROPERTY(Transient)
	TObjectPtr<ACharacter> OwningCharacter;
};
