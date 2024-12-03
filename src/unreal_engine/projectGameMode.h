// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HttpHandler_Get.h"
#include "GameFramework/GameModeBase.h"
#include "projectGameMode.generated.h"

UCLASS(minimalapi)
class AprojectGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AprojectGameMode();
	
	FString prevJson;
	FString newJSON;

	FTimerHandle TimerHandle;
	int numberOfTicks;

	void TickForGetWorld();
	void wrapper();
	FString GetWorldDataAsJson();
	bool CompareJsonStrings();
	void UpdatePrevJson();
	void HandleWorldDataChange();

	UHttpHandler_Get* HttpHandler;
	void httpSendReq(FString *PayloadJson);


protected:
    // beginplay is overriden when the functions above are called
    virtual void BeginPlay() override;
};

