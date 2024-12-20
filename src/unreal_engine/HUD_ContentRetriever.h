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


public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void ResponseFileRead();
	void UpdateTextBlocks();
	void ProcessNewResponse(const FString& NewResponse);
	FString prevResponse;
	FString newResponse;
	TQueue<FString> ResponseQueue;
	TArray<class UTextBlock*> GameStateTexts;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (BindWidget))
	class UTextBlock* GameStateText_0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (BindWidget))
	class UTextBlock* GameStateText_1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (BindWidget))
	class UTextBlock* GameStateText_2;
};
