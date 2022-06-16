// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MenuManagerInterface.generated.h"

UINTERFACE(MinimalAPI)
class UMenuManagerInterface : public UInterface
{
	GENERATED_BODY()
};

class BUILDINGSYSTEM_API IMenuManagerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void OpenBuildingMenu();

	UFUNCTION(BlueprintNativeEvent)
	void CloseBuildingMenu(int NewBuildId);
};
