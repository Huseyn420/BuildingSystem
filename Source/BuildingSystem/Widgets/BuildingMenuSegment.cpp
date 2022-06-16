// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildingMenuSegment.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/SizeBoxSlot.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"

UBuildingMenuSegment::UBuildingMenuSegment(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Button = NewObject<UButton>(this, "Button");
	Name = NewObject<UTextBlock>(this, FName("Name"));
	Icon = NewObject<UImage>(UImage::StaticClass());
	Size = FVector2D(170.0f, 210.0f);
	bIsSelected = false;
	Button->OnClicked.AddUniqueDynamic(this, &UBuildingMenuSegment::OnButtonWasClicked);

	static ConstructorHelpers::FObjectFinder<UTexture2D> Texture(TEXT("Texture2D'/Engine/EditorResources/SequenceRecorder/RecordingIndicator.RecordingIndicator'"));
	if (Texture.Succeeded())
	{
		Icon->SetBrushFromTexture(Texture.Object);
		Name->SetText(FText::FromString("No name"));
	}
}


TSharedRef<SWidget> UBuildingMenuSegment::RebuildWidget()
{
	USizeBox* SizeBox = NewObject<USizeBox>(this);
	UVerticalBox* VerticalBox = NewObject<UVerticalBox>(this);

	FColor Color = FColor(220, 220, 220);
	FColor BackgroundColor = bIsSelected ? FColor(255, 0, 0) : FColor(244, 169, 0);

	Name->ColorAndOpacity = FSlateColor(FLinearColor(Color));

	UPanelSlot* IconSlot = VerticalBox->AddChildToVerticalBox(Icon);
	UVerticalBoxSlot* IconBoxSlot = Cast<UVerticalBoxSlot>(IconSlot);
	if (IconBoxSlot)
	{
		FSlateChildSize SlotSize(ESlateSizeRule::Automatic);
		IconBoxSlot->SetSize(SlotSize);

		IconBoxSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
		IconBoxSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
	}

	UPanelSlot* NameSlot = VerticalBox->AddChildToVerticalBox(Name);
	UVerticalBoxSlot* NameBoxSlot = Cast<UVerticalBoxSlot>(NameSlot);
	if (NameBoxSlot)
	{
		FSlateChildSize SlotSize(ESlateSizeRule::Automatic);
		NameBoxSlot->SetSize(SlotSize);

		NameBoxSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
		NameBoxSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Bottom);
	}

	Button->AddChild(VerticalBox);
	Button->SetBackgroundColor(BackgroundColor);

	SizeBox->AddChild(Button);
	SizeBox->SetHeightOverride(Size.Y);
	SizeBox->SetWidthOverride(Size.X);
	
	TSharedRef<SWidget> Widget = SizeBox->TakeWidget();
	return Widget;
}


void UBuildingMenuSegment::SetData(IMenuManagerInterface* Manager, int BuildableId, FString BuildableName, UTexture2D* BuildableIcon, FVector2D BoxSize, bool bBuildableSelected)
{
	Icon->SetBrushFromTexture(BuildableIcon);
	Icon->SetBrushSize(FVector2D(Size.X));
	
	if (BuildableName.Len() > 10)
	{
		BuildableName.MidInline(0, 7);
		BuildableName += "...";
	}

	Name->SetText(FText::FromString(BuildableName));
	Id = BuildableId;
	Size = BoxSize;
	bIsSelected = bBuildableSelected;
	MenuManager = Manager;
}


void UBuildingMenuSegment::OnButtonWasClicked()
{
	if (MenuManager)
	{
		IMenuManagerInterface::Execute_CloseBuildingMenu(MenuManager->_getUObject(), Id);
	}
}