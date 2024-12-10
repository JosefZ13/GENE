// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HttpModule.h"
#include "Components/TextBlock.h"
#include "HttpHandler_Get.generated.h"


UCLASS(Blueprintable)
class PROJECT_API UHttpHandler_Get : public UUserWidget
{

	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void httpSendReq(FString Payload);
	//void FetchGameState();

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* GameStateText;

private:
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
