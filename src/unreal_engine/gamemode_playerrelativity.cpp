// Copyright Epic Games, Inc. All Rights Reserved.

#include "projectGameMode.h"
#include "projectCharacter.h"
#include "GameFramework/Actor.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/ConstructorHelpers.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Camera/CameraComponent.h"
#include "Engine/Texture.h"
#include "Kismet/GameplayStatics.h"
#include "HttpHandler_Get.h"

/*

Track player and all actors. Store actor and player data in a struct object.

In tick function:
- update player reference (location)
- loop through all objects:
	* check if position of object has changed
	* Calculate movement and compare it to the previous offsets to describe the movement.
- Create a json payload (not actual json object) with:
	* Actorname
	* Prev position
	* current poisiton
	* and relative movement description
- Update actor location

*/

AprojectGameMode::AprojectGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}

void AprojectGameMode::BeginPlay()
{
	Super::BeginPlay();

	FString FilePath = FPaths::ProjectDir() + TEXT("LLM_Response/LLM_response.txt"); 
	if (FFileHelper::SaveStringToFile(TEXT(""), *FilePath)) 
	{
		UE_LOG(LogTemp, Log, TEXT("File content cleared successfully: %s"), *FilePath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to clear file content: %s"), *FilePath);
	}
	WholeWorldJson();  
	PrimaryActorTick.bCanEverTick = true; 
	GetActors();
	TickForGetWorld();

}

void AprojectGameMode::TickForGetWorld()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,                       // Timer Handle
			this,                              // Object that owns the function
			&AprojectGameMode::PerformTracking,    // Function to call
			5.0f,                              // Delay (seconds)
			true                               // Loop? (true = repeat every 5 seconds)
		);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GetWorld is null. Timer not set."));
	}
}

void AprojectGameMode::WholeWorldJson()
{
 // there will be some code here for all game data

}

void AprojectGameMode::GetActors() {

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (PlayerPawn)
	{
		FTrackedObject PlayerObject;
		PlayerObject.Actor = PlayerPawn;
		PlayerObject.PreviousPosition = PlayerPawn->GetActorLocation();
		PlayerObject.ActorName = TEXT("Player");
		PlayerObject.IsPlayer = true;

		TrackedObjects.Add(PlayerObject);
	}

	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

	for (AActor* Actor : AllActors)
	{
		if (Actor)
		{
			if (!Cast<APawn>(Actor) && !Actor->FindComponentByClass<UCameraComponent>() && !Actor->IsA(ACharacter::StaticClass()) && !Actor->IsA(APlayerCameraManager::StaticClass()))
			{
				FTrackedObject NewTrackedObject;
				NewTrackedObject.Actor = Actor;
				NewTrackedObject.ActorName = Actor->GetActorLabel();
				NewTrackedObject.PreviousPosition = Actor->GetActorLocation();

				TrackedObjects.Add(NewTrackedObject);
			}
		}
	}
	UE_LOG(LogTemp, Log, TEXT("Tracking %d objects"), TrackedObjects.Num());
}

void AprojectGameMode::PerformTracking() {
	/*
	This logic should understand movements relative to the player.
	- Creates a Json-like string which is our payload.
	- Logs every movement data in the gameworld

	*/
	//Super::Tick(deltaTime);

	FVector PlayerPosition = FVector::ZeroVector;

	// Looping for Player
	for (const FTrackedObject& Object : TrackedObjects)
	{
		if (Object.IsPlayer && Object.Actor)
		{
			PlayerPosition = Object.Actor->GetActorLocation();
			
			break;
		}
	}
	// looping for all Actors
	for (FTrackedObject& TrackedObject : TrackedObjects)
	{
		if (TrackedObject.Actor && IsValid(TrackedObject.Actor))
		{
			FVector CurrentPosition = TrackedObject.Actor->GetActorLocation();

			if (!CurrentPosition.Equals(TrackedObject.PreviousPosition, KINDA_SMALL_NUMBER))
			{
				GetPlayerRelativity(TrackedObject.Actor);
				TrackedObject.PreviousPosition = CurrentPosition;
			}
		}
	}
}
void AprojectGameMode::SendPayload(const FString& Payload)
{
	if (UWorld* World = GetWorld())
	{
		HttpHandler = CreateWidget<UHttpHandler_Get>(World, UHttpHandler_Get::StaticClass());
		if (HttpHandler)
		{
			HttpHandler->AddToViewport();

			HttpHandler->httpSendReq(Payload);

		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("HttpHandler is null or Payload is empty!"));
		}
	}
}

void AprojectGameMode::GetPlayerRelativity(const AActor* TargetActor)
{
	if (!TargetActor || !TrackedObjects.Num())
		UE_LOG(LogTemp, Error, TEXT("Invalid actor or no tracked objects."));

	FVector PlayerPosition = FVector::ZeroVector;
	FVector PlayerForward = FVector::ZeroVector;
	FVector PlayerRight = FVector::ZeroVector;

	for (const FTrackedObject& Object : TrackedObjects)
	{
		if (Object.IsPlayer && Object.Actor)
		{
			PlayerPosition = Object.Actor->GetActorLocation();
			PlayerForward = Object.Actor->GetActorForwardVector();
			PlayerRight = Object.Actor->GetActorRightVector();
			break;
		}
	}
	FVector ActorLocation = TargetActor->GetActorLocation(); 
	FVector RelativeVector = TargetActor->GetActorLocation() - PlayerPosition;
	FString name = TargetActor->GetActorLabel();
	float Distance = RelativeVector.Size(); // Get magnitude of vector
	UE_LOG(LogTemp, Log, TEXT("Distances: %.2f | %s"), Distance, *name);

	float ForwardDot = FVector::DotProduct(PlayerForward, RelativeVector);
	float RightDot = FVector::DotProduct(PlayerRight, RelativeVector);

	if (Distance < 800)
	{
		FString Payload = GetRelativePosition(ForwardDot, RightDot);
		UE_LOG(LogTemp, Log, TEXT("Relativity Data: %s"), *Payload);

		FString Result = FString::Printf(TEXT(
			"{ \"TargetObject\": \"%s\", \"Distance\": %.2f, \"Relative Position\": \"%s\" }"),
			*TargetActor->GetActorLabel(),
			Distance,
			*Payload);
		UE_LOG(LogTemp, Log, TEXT("Relativity Data: %s"), *Result);

		DrawDebugLine(GetWorld(), PlayerPosition, ActorLocation, FColor::Green, false, 5.0f, 0, 5.0f);
		SendPayload(Result);
	}
	else
	{
		DrawDebugLine(GetWorld(), PlayerPosition, ActorLocation, FColor::Red, false, 5.0f, 0, 5.0f);
	}
}

FString AprojectGameMode::DetermineQuadrant(const FVector& RelativeVector)
{
	// Function not used right now
	if (RelativeVector.X >= 0 && RelativeVector.Y >= 0)
		return TEXT("North-East");
	else if (RelativeVector.X < 0 && RelativeVector.Y >= 0)
		return TEXT("North-West");
	else if (RelativeVector.X < 0 && RelativeVector.Y < 0)
		return TEXT("South-West");
	else
		return TEXT("South-East");
}

float AprojectGameMode::CalculateAngle(const FVector& RelativeVector)
{
	// Function not used right now
	return FMath::Atan2(RelativeVector.Y, RelativeVector.X) * (180.f / PI);
}

FString AprojectGameMode::GetRelativePosition(const float& ForwardDot, const float& RightDot)
{
	if (ForwardDot > 0) // front of player
	{
		if (RightDot > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("Actor is in front-right of the player."));
			return FString(TEXT("Actor is in front-right of the player."));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Actor is in front-left of the player."));
			return FString(TEXT("Actor is in front-left of the player"));
		}
	}
	else // behind player
	{
		if (RightDot > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("Actor is behind-right of the player."));
			return FString(TEXT("Actor is behind-right of the player."));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Actor is behind-left of the player."));
			return FString(TEXT("Actor is behind-left of the player."));
		}
	}
}
