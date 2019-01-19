// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ArmadilloPlatformerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ArmadilloPlatformerGameMode.h"

AArmadilloPlatformerCharacter::AArmadilloPlatformerCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Create a camera boom attached to the root (capsule)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bAbsoluteRotation = true; // Rotation of the character should not affect rotation of boom
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->TargetArmLength = DefaultCameraDistance;
	CameraBoom->SocketOffset = FVector(0.f,0.f,75.f);
	CameraBoom->RelativeRotation = DefaultCameraRotation;

	// Create a camera and attach to boom
	SideViewCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("SideViewCamera"));
	SideViewCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	SideViewCameraComponent->bUsePawnControlRotation = false; // We don't want the controller rotating the camera

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Face in the direction we are moving..
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->GravityScale = 2.f;
	GetCharacterMovement()->AirControl = 0.80f;
	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->GroundFriction = 3.f;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->MaxFlySpeed = 600.f;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

// Called when the game starts or when spawned
void AArmadilloPlatformerCharacter::BeginPlay()
{
	Super::BeginPlay();

	//Show system cursor
	PcMouse = GetWorld()->GetFirstPlayerController();
	PcMouse->bShowMouseCursor = true;
	PcMouse->bEnableClickEvents = true;
	PcMouse->bEnableMouseOverEvents = true;
}
void AArmadilloPlatformerCharacter::Tick(float deltaTime)
{
	if (ThirdpersonCam) {
		CameraBoom->bUsePawnControlRotation = true;
		/*FRotator cameraWorldRotation{ GetActorRotation() + CameraBoom->RelativeRotation };
		UE_LOG(LogTemp, Warning, TEXT("Angle is %f"), AArmadilloPlatformerGameMode::GetAngleBetween(cameraWorldRotation.Quaternion().Vector(), GetControlRotation().Quaternion().Vector()));
		if (AArmadilloPlatformerGameMode::GetAngleBetween(cameraWorldRotation.Quaternion().Vector(), GetControlRotation().Quaternion().Vector()) < 10.f) {
			
			CameraBoom->bUsePawnControlRotation = true;
		}
		else {
			CameraBoom->SetWorldRotation(FRotator{ FQuat::FastLerp(cameraWorldRotation.Quaternion(), GetControlRotation().Quaternion(), 0.4f) }, true);
		}*/
	}
	else {

		CameraBoom->bUsePawnControlRotation = false;

		CameraBoom->SetRelativeRotation(FRotator{ FQuat::FastLerp(CameraBoom->RelativeRotation.Quaternion(), DefaultCameraRotation.Quaternion(), 0.4f) }, true);
		CameraBoom->TargetArmLength = FMath::Lerp(CameraBoom->TargetArmLength, DefaultCameraDistance, 0.4f);
	}
}
//////////////////////////////////////////////////////////////////////////
// Input

void AArmadilloPlatformerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAxis("MoveRight", this, &AArmadilloPlatformerCharacter::MoveRight);
	PlayerInputComponent->BindAxis("CameraRight", this, &AArmadilloPlatformerCharacter::MouseRight);
	PlayerInputComponent->BindAxis("CameraUp", this, &AArmadilloPlatformerCharacter::MouseUp);

	PlayerInputComponent->BindAction("LeftMouseBDown", IE_Pressed, this, &AArmadilloPlatformerCharacter::LeftMouseBDown);
	PlayerInputComponent->BindAction("LeftMouseBDown", IE_Released, this, &AArmadilloPlatformerCharacter::LeftMouseBUp);
	PlayerInputComponent->BindAction("RightMouseBDown", IE_Pressed, this, &AArmadilloPlatformerCharacter::RightMouseBDown);
	PlayerInputComponent->BindAction("RightMouseBDown", IE_Released, this, &AArmadilloPlatformerCharacter::RightMouseBUp);

	PlayerInputComponent->BindAction("ChangeMode", IE_Pressed, this, &AArmadilloPlatformerCharacter::ChangeCameraPerspective);
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AArmadilloPlatformerCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AArmadilloPlatformerCharacter::TouchStopped);
}

void AArmadilloPlatformerCharacter::ChangeCameraPerspective()
{
	// CameraBoom
	// SideViewCameraComponent

	// UE_LOG(LogTemp, Warning, TEXT("Switched modes!"));

	ThirdpersonCam = !ThirdpersonCam;

	/*CameraBoom->bUsePawnControlRotation = ThirdpersonCam;
	if (ThirdpersonCam) {
			
	}
	else {
		CameraBoom->RelativeRotation = DefaultCameraRotation;
		CameraBoom->TargetArmLength = DefaultCameraDistance;
	}*/
}

void AArmadilloPlatformerCharacter::Tongue()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	FHitResult TraceResult(ForceInit);

	if (PlayerController->GetHitResultUnderCursor(ECollisionChannel::ECC_GameTraceChannel3, false, TraceResult)) 
	{
		FVector TongueDirection = GetActorLocation();
		TongueDirection.Y -= TraceResult.ImpactPoint.Y;
		TongueDirection.Z -= TraceResult.ImpactPoint.Z;

		if (bLeftMouseBDown && TongueDirection.Size() < maxTongueRange)
		{
			//do rope things
		}
	}
}

void AArmadilloPlatformerCharacter::MoveRight(float Value)
{
	// add movement in that direction
	AddMovementInput(FVector(0.f,-1.f,0.f), Value);
}

void AArmadilloPlatformerCharacter::MouseRight(float val)
{
	AddControllerYawInput(val * UGameplayStatics::GetWorldDeltaSeconds(this) * CameraTurnRate);
}

void AArmadilloPlatformerCharacter::MouseUp(float val)
{
	AddControllerPitchInput(val * UGameplayStatics::GetWorldDeltaSeconds(this) * CameraTurnRate);
}

void AArmadilloPlatformerCharacter::LeftMouseBDown()
{
	bLeftMouseBDown = true;
}

void AArmadilloPlatformerCharacter::LeftMouseBUp()
{
	bLeftMouseBDown = false;
}

void AArmadilloPlatformerCharacter::RightMouseBDown()
{
	bRightMouseBDown = true;
}

void AArmadilloPlatformerCharacter::RightMouseBUp()
{
	bRightMouseBDown = false;
}

void AArmadilloPlatformerCharacter::TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// jump on any touch
	Jump();
}

void AArmadilloPlatformerCharacter::TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	StopJumping();
}
