// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildInterface.h"
#include "DestructibleComponent.h"
#include "BaseBuildable.generated.h"

UCLASS()
class BUILDINGSYSTEM_API ABaseBuildable : public AActor, public IBuildInterface
{
	GENERATED_BODY()
	
public:
	ABaseBuildable();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDestructibleMesh* DestructibleMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDestructibleComponent* DestructibleComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int GroupId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AActor*> BuildChildrens;

public:
	TArray<UBoxComponent*> ReturnBoxes_Implementation() override;
	int GetGroupId_Implementation() override;
	void SetBaseData_Implementation(UDestructibleMesh* Mesh, float MaxHealth, int Group) override;
	void DealDamage_Implementation(float Damage, FVector HitLocation, FVector ImpulseDir, float ImpulseStrength) override;
	void AddBuildChild_Implementation(AActor* Actor) override;
};
