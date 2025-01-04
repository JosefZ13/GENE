// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "GameFramework/GameStateBase.h"
#include "GameStateStoryGen.generated.h"

UCLASS()
class PROJECT_API AGameStateStoryGen : public AGameStateBase
{
	GENERATED_BODY()

public:
	// Tick Functions
	void TickObjectMovement();

	// Json String Whole environment
	TSharedPtr<FJsonObject> GenerateStartEnvironment();

	// Actor Tracking
	void GetActors();
	void PerformTracking();

	// For determining relativity
	void GetPlayerRelativity(const AActor* TargetActor);
	FString GetRelativePosition(const double& ForwardDot, const double& RightDot, const double& VerticalDot);
	void RecordLLMResponse(const FString& Response);

	// Http communication with LLMs inference script
	static FString TrimResponse(const FString& InputString);

	//FString question_prompt(const FString& indicator);
	void SendPayload(const TSharedPtr<FJsonObject>& Event);
	void httpSendReq();

	TSharedPtr<FJsonObject> SerializeVector(const FVector& Vector);

	// Other
	virtual void BeginPlay() override;

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

	// Other variables
	FString LLM_response;
	bool generate_stories = true;
	TArray<TSharedPtr<FJsonObject>> EventHistoryArray;
	TArray<FString> LLMResponseArray;
	TSharedPtr<FJsonObject> StartEnviroment;
	TSharedPtr<FJsonObject> CurrentEnviroment;
	int Event_Count = 0;

	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

};
