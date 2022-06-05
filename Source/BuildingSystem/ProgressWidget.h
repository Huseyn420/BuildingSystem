// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ProgressWidget.generated.h"

UCLASS()
class BUILDINGSYSTEM_API UProgressWidget : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Widget, meta = (AllowPrivateAccess = "true"))
	class UImage* Image;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Widget, meta = (AllowPrivateAccess = "true"))
	class UMaterialInterface* Material;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Widget, meta = (AllowPrivateAccess = "true"))
	class UMaterialInstanceDynamic* MaterialInstance;

	UPROPERTY(VisibleAnywhere, Category = Widget)
	float ProgressValue;

protected:
	TSharedRef<SWidget> RebuildWidget() override;

public:
	UProgressWidget(const FObjectInitializer& ObjectInitializer);

	void SetProgress(float Value);
};
