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
	FTimerHandle TimerHandle;
	void GetActors();
	void PerformTracking();
	void TickForGetWorld();
	void WholeWorldJson();
	void GetPlayerRelativity(const AActor* TargetActor);
	void SendPayload(const FString& Payload);
	FString DetermineQuadrant(const FVector& RelativeVector);
	float CalculateAngle(const FVector& RelativeVector);
	FString GetRelativePosition(const float& ForwardDot, const float& RightDot);

	//void SendPayload(FString Payload);


	struct FTrackedObject {
		AActor* Actor;
		FString ActorName;
		FVector PreviousPosition;
		bool IsPlayer;
	};

	TArray<FTrackedObject> TrackedObjects;

	UHttpHandler_Get* HttpHandler;
	void httpSendReq(FString *PayloadJson);


protected:
    // beginplay is overriden when the functions above are called
    virtual void BeginPlay() override;
};

