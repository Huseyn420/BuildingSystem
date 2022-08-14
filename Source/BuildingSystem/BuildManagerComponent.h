// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DestructibleMesh.h"
#include "Engine/AssetManager.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Interfaces/MenuManagerInterface.h"
#include "BuildManagerComponent.generated.h"


UENUM(BlueprintType)
enum EBuildableGroup
{
	Exterior UMETA(DisplayName = "Exterior"),
	Bathroom UMETA(DisplayName = "Bathroom"),
	Bedroom UMETA(DisplayName = "Bedroom"),
	Lamps UMETA(DisplayName = "Lamps"),
	Furniture UMETA(DisplayName = "Furniture"),
};


USTRUCT(BlueprintType)
struct FTableInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TEnumAsByte<EBuildableGroup> Group;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TAssetPtr<UDataTable> Table;
};


USTRUCT(BlueprintType)
struct FBuildable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TAssetPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TAssetPtr<UDestructibleMesh> DestructibleMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TEnumAsByte<ETraceTypeQuery> TraceChannel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TAssetSubclassOf<AActor> Actor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int AssetId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BoxCollisionOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BoxCollisionSizeFactor;
};


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BUILDINGSYSTEM_API UBuildManagerComponent : public UActorComponent, public IMenuManagerInterface
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Building, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* BuildGhost;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Building, meta = (AllowPrivateAccess = "true"))
	class AActor* HitActor;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Building, meta = (AllowPrivateAccess = "true"))
	class UBuildingMenu* BuildingMenu;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Building, meta = (AllowPrivateAccess = "true"))
	FRotator BuildGhostRotator;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Building, meta = (AllowPrivateAccess = "true"))
	FVector BuildGhostOffset;

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

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly)
	TEnumAsByte<EBuildableGroup> Group;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly)
	bool bWidgetOpen;

	TArray<FTableInfo> Tables;
	TArray<FBuildable> Buildables;

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

	UStaticMesh* GetBuildMesh();
	UDataTable* GetDataTable(FTableInfo TableInfo);
	AActor* CreateBuildActor(FTransform Transform);

public:
	void OpenBuildingMenu_Implementation() override;
	void CloseBuildingMenu_Implementation(int NewBuildId) override;

	void LaunchBuildMode();
	void SpawnBuild();
	void UpdateMesh();
	void DealDamage();
	void InteractWithBuild();
	void LoadBuildables(int TableId);
	void MoveBuildGhost(float Value);
	void RotateBuildGhost(float Value);
};
