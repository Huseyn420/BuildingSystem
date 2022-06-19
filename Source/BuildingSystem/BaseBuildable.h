// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/BuildInterface.h"
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
	UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDestructibleMesh* DestructibleMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDestructibleComponent* DestructibleComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int AssetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AActor*> BuildChildrens;

public:
	TArray<UBoxComponent*> ReturnBoxes_Implementation() override;
	int GetAssetId_Implementation() override;
	void SetBaseData_Implementation(UDestructibleMesh* Mesh, float MaxHealth, int Asset) override;
	void DealDamage_Implementation(float Damage, FVector HitLocation, FVector ImpulseDir, float ImpulseStrength) override;
	void SetBuildMesh_Implementation(UStaticMesh* Mesh, FTransform Transform) override;
	void AddBuildChild_Implementation(AActor* Actor) override;
};
