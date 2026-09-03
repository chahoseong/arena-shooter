// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArenaShooterPlayerCharacter.h"

#include "Animation/AnimMontage.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Combat/ArenaShooterAimComponent.h"
#include "Combat/ArenaShooterHealthComponent.h"
#include "Combat/ArenaShooterWeaponComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "Physics/ArenaShooterCollisionChannels.h"

// Only reached if the weapon component is missing, which the constructor prevents.
static constexpr float FallbackAimDistance = 10000.0f;

AArenaShooterPlayerCharacter::AArenaShooterPlayerCharacter()
{
	// Aim blends the camera, speed and field of view towards their targets every frame.
	PrimaryActorTick.bCanEverTick = true;

	// The body follows the camera's yaw, so movement input strafes instead of turning the body.
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	// Two pieces, two questions. The capsule walks the world and blocks other characters; the mesh
	// is what a shot hits. Shots and the camera boom pass straight through the capsule, so hits are
	// decided against the silhouette and a character at close range does not shove the camera about.
	//
	// Profiles rather than response overrides: a Blueprint that stores a profile name overrides every
	// response set from here, silently and with no sign of it in the details panel.
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("ArenaShooterPawnCapsule"));
	GetMesh()->SetCollisionProfileName(TEXT("ArenaShooterPawnMesh"));

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bUsePawnControlRotation = true;
	// Pull the camera in when level geometry comes between it and the character.
	CameraBoom->bDoCollisionTest = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	Weapon = CreateDefaultSubobject<UArenaShooterWeaponComponent>(TEXT("Weapon"));
	Aim = CreateDefaultSubobject<UArenaShooterAimComponent>(TEXT("Aim"));
	Health = CreateDefaultSubobject<UArenaShooterHealthComponent>(TEXT("Health"));
}

void AArenaShooterPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Hand the camera over piece by piece; each is recorded as it is given.
	Aim->SetSpringArm(CameraBoom);
	Aim->SetCamera(FollowCamera);

	BaseWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

	Health->OnDeath.AddDynamic(this, &AArenaShooterPlayerCharacter::HandleDeath);
}

void AArenaShooterPlayerCharacter::HandleDeath()
{
	// Explicitly, even though Tick stops below and the weapon is only advanced from there. Wanting
	// to fire is latched and released by the input binding alone, so nothing else here would clear
	// it, and a later change to Tick should not be able to bring the shooting back.
	Weapon->StopFiring();

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	DisableInput(Cast<APlayerController>(GetController()));

	// The body stays. Its collision stays too: the capsule is what finds the floor, and Game Flow
	// needs something to put a defeat screen over.

	// Holding the final pose is the montage's own doing: its auto blend out is off, so it keeps the
	// last frame instead of letting the slot fall back to the locomotion pose and standing the body
	// up. The enemy sidesteps this by removing itself at that moment, which the player cannot do.
	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}
}

void AArenaShooterPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Everything below runs unconditionally and would carry on over a corpse: the camera would keep
	// blending, the walk speed would be rewritten every frame, and the aim trace would keep looking
	// for something to shoot. One gate covers them all, and covers whatever is added later.
	//
	// Reads correctly only because Tick runs after BeginPlay: health starts at zero and is filled in
	// there, so IsDead() answers true before that.
	if (Health->IsDead())
	{
		return;
	}

	Aim->Update(DeltaSeconds);

	// Speed is shared ground: aiming only offers a multiplier and the owner applies the result,
	// so a later contributor such as a dash has somewhere to be combined in.
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * Aim->GetSpeedMultiplier();

	Weapon->Update(DeltaSeconds, GetAimTarget());
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

	if (FireAction && Weapon)
	{
		// BindAction deduces the object type from a raw pointer; a TObjectPtr picks the FName overload.
		UArenaShooterWeaponComponent* WeaponPtr = Weapon.Get();
		Input->BindAction(FireAction, ETriggerEvent::Started, WeaponPtr, &UArenaShooterWeaponComponent::StartFiring);
		Input->BindAction(FireAction, ETriggerEvent::Completed, WeaponPtr, &UArenaShooterWeaponComponent::StopFiring);
		Input->BindAction(FireAction, ETriggerEvent::Canceled, WeaponPtr, &UArenaShooterWeaponComponent::StopFiring);
	}

	if (AimAction && Aim)
	{
		UArenaShooterAimComponent* AimPtr = Aim.Get();
		Input->BindAction(AimAction, ETriggerEvent::Started, AimPtr, &UArenaShooterAimComponent::StartAiming);
		Input->BindAction(AimAction, ETriggerEvent::Completed, AimPtr, &UArenaShooterAimComponent::StopAiming);
		Input->BindAction(AimAction, ETriggerEvent::Canceled, AimPtr, &UArenaShooterAimComponent::StopAiming);
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

	// Aiming turns the view more slowly so distant targets can be lined up. It follows the same
	// transition as the camera rather than switching outright.
	const float Scale = Aim->GetLookScale();

	// Pitch inversion is handled by a Negate modifier on the input action.
	AddControllerYawInput(LookInput.X * Scale);
	AddControllerPitchInput(LookInput.Y * Scale);
}

FArenaShooterAimTarget AArenaShooterPlayerCharacter::GetAimTarget() const
{
	const FVector CameraLocation = FollowCamera->GetComponentLocation();
	const FVector CameraForward = FollowCamera->GetForwardVector();

	// Reach past the weapon's range by the boom length, so the muzzle's own range stays intact.
	const float Range = Weapon ? Weapon->GetRange() : FallbackAimDistance;
	const FVector AimEnd = CameraLocation + CameraForward * (Range + CameraBoom->TargetArmLength);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FHitResult AimHit;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		AimHit, CameraLocation, AimEnd, ArenaShooter_TraceChannel_Weapon, Params);

	FArenaShooterAimTarget AimTarget;
	AimTarget.Point = bHit ? AimHit.ImpactPoint : AimEnd;
	AimTarget.Direction = CameraForward;
	return AimTarget;
}
