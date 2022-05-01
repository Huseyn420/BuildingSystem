// Copyright Epic Games, Inc. All Rights Reserved.

#include "BuildingSystemGameMode.h"
#include "BuildingSystemCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABuildingSystemGameMode::ABuildingSystemGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
