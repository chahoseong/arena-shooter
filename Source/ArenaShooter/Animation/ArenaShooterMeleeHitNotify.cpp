// Copyright Epic Games, Inc. All Rights Reserved.

#include "Animation/ArenaShooterMeleeHitNotify.h"

#include "Combat/ArenaShooterMeleeAttackComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

void UArenaShooterMeleeHitNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (Owner == nullptr)
	{
		return;
	}

	if (UArenaShooterMeleeAttackComponent* Melee = Owner->FindComponentByClass<UArenaShooterMeleeAttackComponent>())
	{
		Melee->ResolveHit();
	}
}

FString UArenaShooterMeleeHitNotify::GetNotifyName_Implementation() const
{
	return TEXT("Melee Hit");
}
