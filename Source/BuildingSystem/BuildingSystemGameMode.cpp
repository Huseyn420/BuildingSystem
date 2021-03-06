// Copyright Epic Games, Inc. All Rights Reserved.

#include "BuildingSystemGameMode.h"
#include "BuildingSystemCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABuildingSystemGameMode::ABuildingSystemGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/BuildingSystem/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
