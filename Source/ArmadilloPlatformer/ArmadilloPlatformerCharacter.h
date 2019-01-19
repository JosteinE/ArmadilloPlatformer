// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ArmadilloPlatformerCharacter.generated.h"

UCLASS(config=Game)
class AArmadilloPlatformerCharacter : public ACharacter
{
	GENERATED_BODY()

private:
	/** Side view camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* SideViewCameraComponent;

	/** Camera boom positioning the camera beside the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	bool ThirdpersonCam{ false };
	const FRotator DefaultCameraRotation{ 0.f, 180.f, 0.f };
	const float DefaultCameraDistance{ 500.f };

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called for side to side input */
	void MoveRight(float Val);

	void MouseRight(float val);
	void MouseUp(float val);

	/** Handle touch inputs. */
	void TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location);

	/** Handle touch stop event. */
	void TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location);

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

	void BeginPlay() override;

	void ChangeCameraPerspective();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float CameraTurnRate{ 1.f };

	AArmadilloPlatformerCharacter();

	/** Returns SideViewCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetSideViewCameraComponent() const { return SideViewCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	//Input variables
	APlayerController* PcMouse;
};
