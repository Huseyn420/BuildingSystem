   // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "UObject/Interface.h"
#include "BuildInterface.generated.h"

UINTERFACE(MinimalAPI, meta = (Blueprintable))
class UBuildInterface : public UInterface
{
	GENERATED_BODY()
};

class BUILDINGSYSTEM_API IBuildInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	TArray<UBoxComponent*> ReturnBoxes();

	UFUNCTION(BlueprintNativeEvent)
	int GetGroupId();

	UFUNCTION(BlueprintNativeEvent)
	void InteractWithBuild();

	UFUNCTION(BlueprintNativeEvent)
	void SetBaseData(UDestructibleMesh* Mesh, float MaxHealth, int Group);

	UFUNCTION(BlueprintNativeEvent)
	void DealDamage(float Damage, FVector HitLocation, FVector ImpulseDir, float ImpulseStrength);

	UFUNCTION(BlueprintNativeEvent)
	void AddBuildChild(AActor* Actor);
};
