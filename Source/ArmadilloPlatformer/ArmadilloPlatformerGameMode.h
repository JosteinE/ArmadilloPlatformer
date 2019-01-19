// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ArmadilloPlatformerGameMode.generated.h"

UCLASS(minimalapi)
class AArmadilloPlatformerGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AArmadilloPlatformerGameMode();

	// Borrowed from myself
	static float GetAngleBetween(const FVector& pos1, const FVector& pos2);
};



