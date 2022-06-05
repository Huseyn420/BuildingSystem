// Fill out your copyright notice in the Description page of Project Settings.


#include "ProgressWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

UProgressWidget::UProgressWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialAsset(TEXT("Material'/Game/Materials/M_Progress.M_Progress'"));

	if (MaterialAsset.Succeeded())
	{
		Material = MaterialAsset.Object;
	}

	ProgressValue = 0.0f;
}


TSharedRef<SWidget> UProgressWidget::RebuildWidget()
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
	
	if (RootWidget && Material)
	{
		FVector2D Size = FVector2D(125.0f);

		MaterialInstance = UMaterialInstanceDynamic::Create(Material, this);
		MaterialInstance->SetScalarParameterValue("Progress", ProgressValue);


		Image = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		Image->SetBrushFromMaterial(MaterialInstance);
		Image->SetBrushSize(Size);
		RootWidget->AddChild(Image);

		UCanvasPanelSlot* WidgetSlot = Cast<UCanvasPanelSlot>(Image->Slot);
		if (WidgetSlot)
		{
			WidgetSlot->SetAnchors(FAnchors(0.5f, 0.5f));
			WidgetSlot->SetOffsets(FMargin(-Size.X / 2.0f, -Size.Y / 2.0f));
			WidgetSlot->SetAutoSize(true);
		}
	}

	TSharedRef<SWidget> Widget = Super::RebuildWidget();
	return Widget;
}


void UProgressWidget::SetProgress(float Value)
{
	check(MaterialInstance);
	MaterialInstance->SetScalarParameterValue("Progress", Value);
	ProgressValue = Value;
}
