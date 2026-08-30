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

	/**
	 * How far above or below the horizon the character is looking, -90..90. Drives the aim offset
	 * so that looking up or down is visible in the pose. Yaw has no equivalent: the body turns to
	 * face the view immediately, so there is never a difference for the upper body to absorb.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Aim")
	float Pitch = 0.0f;

private:
	UPROPERTY(Transient)
	TObjectPtr<ACharacter> OwningCharacter;
};
