// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ArmadilloPlatformerGameMode.h"
#include "ArmadilloPlatformerCharacter.h"
#include "UObject/ConstructorHelpers.h"

float AArmadilloPlatformerGameMode::GetAngleBetween(const FVector& pos1, const FVector& pos2)
{
	return FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(pos1, pos2) / (pos1.Size() * pos2.Size())));
}

AArmadilloPlatformerGameMode::AArmadilloPlatformerGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/SideScrollerCPP/Blueprints/SideScrollerCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
