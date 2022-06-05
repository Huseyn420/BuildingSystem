// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DestructibleMesh.h"
#include "Engine/AssetManager.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "BuildManagerComponent.generated.h"


USTRUCT(BlueprintType)
struct FBuildable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TAssetPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TAssetPtr<UDestructibleMesh> DestructibleMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TEnumAsByte<ETraceTypeQuery> TraceChannel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> Actor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int GroupId;
};


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BUILDINGSYSTEM_API UBuildManagerComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Building, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* BuildGhost;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Building, meta = (AllowPrivateAccess = "true"))
	class AActor* HitActor;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Building, meta = (AllowPrivateAccess = "true"))
	FRotator BuildGhostRotator;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Building, meta = (AllowPrivateAccess = "true"))
	bool bIsBuildModeOn;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Building, meta = (AllowPrivateAccess = "true"))
	bool bCanBuild;

	UPROPERTY(VisibleAnyWhere, Category = Materials)
	UMaterial* GreenColor;

	UPROPERTY(VisibleAnyWhere, Category = Materials)
	UMaterial* RedColor;

public:
	UBuildManagerComponent();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Scene)
	class ABuildingSystemCharacter* PlayerRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Scene)
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Scene)
	class USceneComponent* Scene;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly)
	int BuildId;

	TArray<FBuildable*> Buildables;

protected:
	virtual void BeginPlay() override;

	void BuildDelay();
	void BuildCycle();
	void GiveBuildColor(bool bIsGreen);
	void SpawnBuildGhost();
	void DestroyBuildGhost();
	
	bool IsBuildFloating();
	bool CheckForOverlap();
	bool DetectBuildBoxed(AActor* Actor, UPrimitiveComponent* Component);

public:
	void LaunchBuildMode();
	void SpawnBuild();
	void UpdateMesh();
	void DealDamage();
	void InteractWithBuild();
	void RotateBuildGhost(float Value);
};
