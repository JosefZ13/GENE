// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "projectGameMode.generated.h"

UCLASS(minimalapi)
class AprojectGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AprojectGameMode();
	FString GetActors();
	void Tick(float deltaTime);
	void SendPayload(FString Payload);


	struct FTrackedObject {
		AActor* Actor;
		FString ActorName;
		FVector PreviousPosition;
		bool IsPlayer;
	};

	TArray<FTrackedObject> TrackedObjects;

};



