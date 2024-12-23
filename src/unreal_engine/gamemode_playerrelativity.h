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
	// constructor
    AprojectGameMode();

	// World tracking
	void TickForWholeWorld();
	void WholeWorldJson();

	// Actor tracking
	void TickForGetWorld();
	void GetActors();
	void PerformTracking();

	// for serialization
	TSharedPtr<FJsonObject> SerializeVector(const FVector& Vector);

	// for determine relativity
	void GetPlayerRelativity(const AActor* TargetActor);
	FString DetermineQuadrant(const FVector& RelativeVector);
	float CalculateAngle(const FVector& RelativeVector);
	FString GetRelativePosition(const float& ForwardDot, const float& RightDot, const float& VerticalDot);


	// HHTP commication
	void SendPayload(const FString& Payload, const FString& indicator, const FString& context);
	FString question_prompt(const FString& indicator);
	UHttpHandler_Get* HttpHandler;
	void httpSendReq(FString Payload, FString question, FString context); 

	// Example variable for Blueprint access
	UPROPERTY(BlueprintReadWrite, Category = "CustomLogic")
	int32 Score;

private:
	// Timer
	FTimerHandle TimerHandle;

	// Tracked objects struct
	struct FTrackedObject {
		AActor* Actor;
		FString ActorName;
		FVector PreviousPosition;
		bool IsPlayer;
	};
	
	// Tracked objects array using the struct above
	TArray<FTrackedObject> TrackedObjects;

protected:
    // beginplay is overriden when the functions above are called
    virtual void BeginPlay() override;
};
