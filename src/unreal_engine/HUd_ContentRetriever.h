// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUD_ContentRetreiver.generated.h"

/**
 *
 */
UCLASS()
class PROJECT_API UHUD_ContentRetreiver : public UUserWidget
{
	GENERATED_BODY()


private:



public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;


	void ProcessNewResponse(const FString& NewResponse);
	void UpdateBorderVisibility();


	FString prevResponse;
	FString newResponse;
	FString prevWholeResponse;
	float TimeSinceLastBorderUpdate = 0.0f;
	TArray<bool> BordersChanged;

	TQueue<FString> ResponseQueue;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (BindWidget))
	class UTextBlock* GameStateText;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (BindWidget))
	class UBorder* GameStateBorder;


	UPROPERTY(BlueprintReadOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeOutAnimation;
};

