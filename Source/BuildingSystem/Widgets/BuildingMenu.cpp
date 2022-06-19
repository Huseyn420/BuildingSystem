// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildingMenu.h"
#include "BuildingMenuSegment.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/UniformGridPanel.h"
#include "Kismet/GameplayStatics.h"

UBuildingMenu::UBuildingMenu(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Segments = TArray<UBuildingMenuSegment*>();
	bIsFocusable = true;
}


TSharedRef<SWidget> UBuildingMenu::RebuildWidget()
{
	UPanelWidget* RootWidget = Cast<UPanelWidget>(GetRootWidget());
	if (RootWidget == nullptr)
	{
		RootWidget = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
		UCanvasPanelSlot* RootWidgetSlot = Cast<UCanvasPanelSlot>(RootWidget->Slot);
		if (RootWidgetSlot)
		{
			RootWidgetSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
			RootWidgetSlot->SetOffsets(FMargin(0.0f, 0.0f));
		}
		WidgetTree->RootWidget = RootWidget;
	}

	if (RootWidget)
	{
		UImage* BackgroundImage = NewObject<UImage>(this);
		UScrollBox* ScrollBox = NewObject<UScrollBox>(this);
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		UUniformGridPanel* UniformGridPanel = NewObject<UUniformGridPanel>(this);

		FString PathToLoad = "Texture2D'/Game/BuildingSystem/Textures/T_MenuBackground.T_MenuBackground'";
		UTexture2D* BackgroundTexture = LoadTextureFromPath(PathToLoad);
		BackgroundImage->SetBrushFromTexture(BackgroundTexture);

		RootWidget->AddChild(BackgroundImage);
		UCanvasPanelSlot* BackgroundSlot = Cast<UCanvasPanelSlot>(BackgroundImage->Slot);
		if (BackgroundSlot)
		{
			BackgroundSlot->ZOrder = 0;
			BackgroundSlot->SetAnchors(FAnchors(0.5f, 0.5f));
			BackgroundSlot->SetOffsets(FMargin(-BackgroundImageSize.X / 2.0f, -BackgroundImageSize.Y / 2.0f));
			BackgroundSlot->SetSize(BackgroundImageSize);
		}

		int cols = floor(GridSize.X / CellSize.X);
		for (int i = 0; i < Buildables->Num(); i++)
		{
			UBuildingMenuSegment* BuildingMenuSegment = CreateWidget<UBuildingMenuSegment>(PlayerController, UBuildingMenuSegment::StaticClass());
			int row = i / cols;
			int col = i % cols;

			UniformGridPanel->AddChildToUniformGrid(BuildingMenuSegment, row, col);
			UniformGridPanel->SetSlotPadding(FMargin(5.0f));

			FBuildable Buildable = (*Buildables)[i];
			BuildingMenuSegment->SetData(MenuManager, i, Buildable.Name, Buildable.Icon, CellSize, BuildId == i);
		}
		
		ScrollBox->AddChild(UniformGridPanel);
		UScrollBoxSlot* GridSlot = Cast<UScrollBoxSlot>(UniformGridPanel->Slot);
		if (GridSlot)
		{
			GridSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
			GridSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
		}

		RootWidget->AddChild(ScrollBox);
		UCanvasPanelSlot* WidgetSlot = Cast<UCanvasPanelSlot>(ScrollBox->Slot);
		if (WidgetSlot)
		{
			WidgetSlot->SetAnchors(FAnchors(0.5f, 0.5f));
			WidgetSlot->SetOffsets(FMargin(-GridSize.X / 2.0f, -GridSize.Y / 2.0f));
			WidgetSlot->SetSize(GridSize);
		}
	}
	else
	{
		IMenuManagerInterface::Execute_CloseBuildingMenu(MenuManager->_getUObject(), BuildId);
	}

	TSharedRef<SWidget> Widget = Super::RebuildWidget();
	return Widget;
}


void UBuildingMenu::SetData(IMenuManagerInterface* Manager, TArray<FBuildable>* Data, int BuildGhostId, TEnumAsByte<EBuildableGroup> BuildableGroup)
{
	MenuManager = Manager;
	Buildables = Data;
	BuildId = BuildGhostId;
	Group = BuildableGroup;
}


void UBuildingMenu::SetBuildId(int BuildGhostId)
{
	BuildId = BuildGhostId;
	RebuildWidget();
}


UTexture2D* UBuildingMenu::LoadTextureFromPath(const FString& Path)
{
	if (Path.IsEmpty()) return NULL;

	return Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), NULL, *(Path)));
}