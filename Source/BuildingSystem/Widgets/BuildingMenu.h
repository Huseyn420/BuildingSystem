// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BuildingMenuSegment.h"
#include "BuildingSystem/BuildManagerComponent.h"
#include "Engine/DataTable.h"
#include "BuildingMenu.generated.h"

UCLASS()
class BUILDINGSYSTEM_API UBuildingMenu : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = MenuSegment, meta = (AllowPrivateAccess = "true"))
	TArray<UBuildingMenuSegment*> Segments;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = MenuSegment, meta = (AllowPrivateAccess = "true"))
	FVector2D BackgroundImageSize = FVector2D(1500.0f, 940.0f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = MenuSegment, meta = (AllowPrivateAccess = "true"))
	FVector2D GridSize = FVector2D(900.0f, 600.0f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = MenuSegment, meta = (AllowPrivateAccess = "true"))
	FVector2D CellSize = FVector2D(180.0f, 220.0f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = MenuSegment, meta = (AllowPrivateAccess = "true"))
	int BuildId;

	TEnumAsByte<EBuildableGroup> Group;

	TArray<FBuildable>* Buildables;
	IMenuManagerInterface* MenuManager;

protected:
	TSharedRef<SWidget> RebuildWidget() override;

	UTexture2D* LoadTextureFromPath(const FString& Path);

public:
	UBuildingMenu(const FObjectInitializer& ObjectInitializer);

	void SetData(IMenuManagerInterface* Manager, TArray<FBuildable>* Data, int BuildGhostId, TEnumAsByte<EBuildableGroup> BuildableGroup);
	void SetBuildId(int BuildGhostId);
};
