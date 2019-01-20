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
	GetCharacterMovement()->AirControl = 2.0f;
	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->GroundFriction = 3.f;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->MaxFlySpeed = 600.f;

	//Create a Azimuth Gimbal (for horizontal Camera Rotation)
	/*AzimuthGimbal = CreateDefaultSubobject<USceneComponent>(TEXT("AzimuthGimbal"));
	AzimuthGimbal->bAbsoluteRotation = true;
	AzimuthGimbal->SetupAttachment(RootComponent);*/

	// Create the tongue
	PlayerTongue = CreateDefaultSubobject<UCableComponent>(TEXT("Tongue")); //Added "CableComponent" in the Build.cs file to make the cable component work
	PlayerTongue->SetWorldLocation(FVector(100.f, 0.f, 0.f));
	PlayerTongue->bEnableCollision = true;
	PlayerTongue->bEnableStiffness = true;
	PlayerTongue->bAttachEnd = false;
	PlayerTongue->CableLength = 50;
	PlayerTongue->NumSegments = 20;
	PlayerTongue->SolverIterations = 16;
	PlayerTongue->SubstepTime = 0.005f;
	PlayerTongue->CollisionFriction = 1.0f;
	PlayerTongue->CableGravityScale = 1.2f;
	PlayerTongue->CableWidth = 3.0f;
	PlayerTongue->NumSides = 4.0f;
	PlayerTongue->CableGravityScale = 3.0f;
	//PlayerTongue->SetCollisionProfileName(TEXT("NoCollision"));

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

	if (bHooked) { Hooked(); };
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

	PlayerInputComponent->BindAction("ChangeStance", IE_Pressed, this, &AArmadilloPlatformerCharacter::ChangeStance);

	PlayerInputComponent->BindAction("ChangeCamera", IE_Pressed, this, &AArmadilloPlatformerCharacter::ChangeCameraPerspective);

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

void AArmadilloPlatformerCharacter::ChangeStance()
{
	if (bBallStance)
	{
		bBallStance = false;
		GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
		GetCharacterMovement()->MaxAcceleration = NormalAcceleration;
		GetCharacterMovement()->BrakingDecelerationWalking = 2048;
		GetCharacterMovement()->BrakingFrictionFactor = 2;
	}
	else
	{
		bBallStance = true;
		GetCharacterMovement()->MaxWalkSpeed = BallMovementSpeed;
		GetCharacterMovement()->MaxAcceleration = BallAcceleration;
		GetCharacterMovement()->BrakingDecelerationWalking = BallDeceleration;
		GetCharacterMovement()->BrakingFrictionFactor = BallFriction;
	}
}

void AArmadilloPlatformerCharacter::Tongue()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	FHitResult TraceResult(ForceInit);

	if (PlayerController->GetHitResultUnderCursor(ECollisionChannel::ECC_GameTraceChannel3, false, TraceResult)) 
	{
		FVector Start = GetActorLocation();
		FVector End = TraceResult.ImpactPoint;
		

		if (!ThirdpersonCam)
		{
			End.X = Start.X;
		}
		
		FCollisionQueryParams CollisionParams;

		DrawDebugLine(GetWorld(), Start, End, FColor::Green, true);

		if (GetWorld()->LineTraceSingleByChannel(TraceResult, Start, End, ECC_Visibility, CollisionParams))
		{
			if (TraceResult.bBlockingHit)
			{
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("You are hitting: %s"), *TraceResult.GetActor()->GetName()));
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Purple, FString::Printf(TEXT("Impact Point: %s"), *TraceResult.ImpactPoint.ToString()));
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, FString::Printf(TEXT("Normal Point: %s"), *TraceResult.ImpactNormal.ToString()));

				FVector TraceImpactPoint = TraceResult.ImpactPoint; //
				mouseHitLocation = TraceImpactPoint; // workaround. Game crashes otherwise

				PlayerTongue->bAttachEnd = true;
				bBallStance = true;
				bHooked = true;
				GetCharacterMovement()->bOrientRotationToMovement = false;
			}
		}
		else
		{
			BreakTongue();
		}
	}
}

void AArmadilloPlatformerCharacter::BreakTongue()
{
	PlayerTongue->EndLocation = FVector(100.f, 0.f, 0.f);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	PlayerTongue->CableLength = 50.f;
	GetCharacterMovement()->AirControl = 2.f;
	PlayerTongue->bAttachEnd = false;
	bBallStance = false;
	bHooked = false;
}

void AArmadilloPlatformerCharacter::Hooked()
{
	FVector TongueDirection;
	FVector PlayerLocation;
	//This was a mess...
	//The cable component end location seem to always be placed relative to itself.
	//Because our cable component is always facing in the direction of the characters forward vector
	//which uses X (horizontal) and the Z (vertical) axes,
	//I had to translate our inputs (the Y and Z locations) to work with the characters tongue. 
	if (GetActorRotation().Yaw < 0.f)
	{
		TongueDirection.X = -mouseHitLocation.Y;
		PlayerLocation.X = -GetActorLocation().Y;
	}
	else
	{
		TongueDirection.X = mouseHitLocation.Y;
		PlayerLocation.X = GetActorLocation().Y;
	}

	TongueDirection.Y = 0.f;
	TongueDirection.Z = mouseHitLocation.Z;

	PlayerLocation.Y = 0;
	PlayerLocation.Z = GetActorLocation().Z;

	PlayerTongue->EndLocation = TongueDirection - PlayerLocation;
	PlayerTongue->CableLength = FVector(TongueDirection - PlayerLocation).Size() - 300.f;
	GetCharacterMovement()->AirControl = 1.f;
	Swing();
}

void AArmadilloPlatformerCharacter::Swing()
{
	FVector swingVector = GetActorLocation() - mouseHitLocation;

	//swingVector = (swingVector.Normalize(FMath::Acos(FVector::DotProduct(swingVector, GetVelocity())))) * -2

	FVector normalizedSwingVector = swingVector;
	normalizedSwingVector.Normalize();
	swingVector = normalizedSwingVector * FVector::DotProduct(swingVector, GetVelocity()) * -2;
	
	GetCharacterMovement()->AddForce(swingVector);
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

	Tongue();
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
