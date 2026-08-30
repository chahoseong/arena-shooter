// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ArenaShooterMeleeHitNotify.generated.h"

/**
 * Marks the frame a swing is decided on. Placed on the attack montage, where it can be lined up
 * against the pose by eye, rather than guessed at as a delay in code.
 *
 * A marker in time and nothing more: it finds the melee component and hands over.
 */
UCLASS(meta = (DisplayName = "Melee Hit"))
class ARENASHOOTER_API UArenaShooterMeleeHitNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;
};
