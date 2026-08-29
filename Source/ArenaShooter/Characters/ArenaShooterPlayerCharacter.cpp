// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArenaShooterPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"

AArenaShooterPlayerCharacter::AArenaShooterPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// The body follows the camera's yaw, so movement input strafes instead of turning the body.
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bUsePawnControlRotation = true;
	// Pull the camera in when level geometry comes between it and the character.
	CameraBoom->bDoCollisionTest = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void AArenaShooterPlayerCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	if (DefaultMappingContext == nullptr)
	{
		return;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController == nullptr)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, DefaultMappingPriority);
	}
}

void AArenaShooterPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	if (MoveAction)
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AArenaShooterPlayerCharacter::Move);
	}

	if (LookAction)
	{
		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &AArenaShooterPlayerCharacter::Look);
	}

	if (JumpAction)
	{
		Input->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}
}

void AArenaShooterPlayerCharacter::Move(const FInputActionValue& Value)
{
	if (Controller == nullptr)
	{
		return;
	}

	const FVector2D MoveInput = Value.Get<FVector2D>();

	// Move relative to where the camera looks, ignoring its pitch so the character stays on the ground.
	const FRotator YawRotation(0.0, Controller->GetControlRotation().Yaw, 0.0);
	const FRotationMatrix YawMatrix(YawRotation);

	// The movement component clamps the accumulated input to length 1, so diagonal input is not faster.
	AddMovementInput(YawMatrix.GetUnitAxis(EAxis::X), MoveInput.Y);
	AddMovementInput(YawMatrix.GetUnitAxis(EAxis::Y), MoveInput.X);
}

void AArenaShooterPlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookInput = Value.Get<FVector2D>();

	// Pitch inversion is handled by a Negate modifier on the input action.
	AddControllerYawInput(LookInput.X);
	AddControllerPitchInput(LookInput.Y);
}
