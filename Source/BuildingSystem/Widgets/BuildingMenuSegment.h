// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include <BuildingSystem/Interfaces/MenuManagerInterface.h>
#include "BuildingMenuSegment.generated.h"

UCLASS()
class BUILDINGSYSTEM_API UBuildingMenuSegment : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = MenuSegment, meta = (AllowPrivateAccess = "true"))
	class UImage* Icon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = MenuSegment, meta = (AllowPrivateAccess = "true"))
	class UTextBlock* Name;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = MenuSegment, meta = (AllowPrivateAccess = "true"))
	class UButton* Button;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = MenuSegment, meta = (AllowPrivateAccess = "true"))
	int Id;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = MenuSegment, meta = (AllowPrivateAccess = "true"))
	bool bIsSelected;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = MenuSegment, meta = (AllowPrivateAccess = "true"))
	FVector2D Size;

	IMenuManagerInterface* MenuManager;

protected:
	TSharedRef<SWidget> RebuildWidget() override;

	UFUNCTION()
	void OnButtonWasClicked();

public:
	UBuildingMenuSegment(const FObjectInitializer& ObjectInitializer);

	void SetData(IMenuManagerInterface* Manager, int BuildableId, FString BuildableName, UTexture2D* BuildableIcon, FVector2D BoxSize, bool bBuildableSelected);
};
