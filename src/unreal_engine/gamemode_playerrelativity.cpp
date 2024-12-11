// Copyright Epic Games, Inc. All Rights Reserved.

#include "projectGameMode.h"
#include "projectCharacter.h"
#include "GameFramework/Actor.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/ConstructorHelpers.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
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
			FTrackedObject NewTrackedObject;
			NewTrackedObject.Actor = Actor;
			NewTrackedObject.ActorName = Actor->GetName();
			NewTrackedObject.PreviousPosition = Actor->GetActorLocation();

			TrackedObjects.Add(NewTrackedObject);
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
	//FVector PlayerForward = FVector::ZeroVector;
	//FVector PlayerRight = FVector::ZeroVector;

	// Looping for Player
	for (const FTrackedObject& Object : TrackedObjects)
	{
		if (Object.IsPlayer && Object.Actor)
		{
			PlayerPosition = Object.Actor->GetActorLocation();
			//FRotator PlayerRotation = Object.Actor->GetActorRotation();
			//PlayerForward = PlayerRotation.Vector(); // Forward vector
			//PlayerRight = FVector::CrossProduct(PlayerForward, FVector::UpVector); // Right vector
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
				//FVector RelativePosition = CurrentPosition - PlayerPosition;
				FString NewPayload = GetPlayerRelativity(TrackedObject.Actor);

				// DotProduct is a math function in ue. It computes two 3D vectors like:
				// DotProduct(a, B) --> AxB = Ax * Bx + Ay * By + Az * Bz
				//float ForwardOffset = FVector::DotProduct(RelativePosition, PlayerForward);
				//float RightOffset = FVector::DotProduct(RelativePosition, PlayerRight);
				UE_LOG(LogTemp, Log, TEXT("Relativity Data: %s"), *NewPayload);

				/*
				FString movement = TEXT("");

				if (!TrackedObject.IsPlayer)
				{
					FVector PreviousRelativePosition = TrackedObject.PreviousPosition - PlayerPosition;
					float PreviousForwardOffset = FVector::DotProduct(PreviousRelativePosition, PlayerForward);
					float PreviousRightOffset = FVector::DotProduct(PreviousRelativePosition, PlayerRight);

					if (ForwardOffset > PreviousForwardOffset)
						MovementDescription += TEXT("moved forward ");
					else if (ForwardOffset < PreviousForwardOffset)
						MovementDescription += TEXT("moved backward ");

					if (RightOffset > PreviousRightOffset)
						MovementDescription += TEXT("and to the right");
					else if (RightOffset < PreviousRightOffset)
						MovementDescription += TEXT("and to the left");

					if (MovementDescription.IsEmpty())
						MovementDescription = TEXT("did not move relative to the player?");
				}
				else
				{
					MovementDescription = TEXT("Player moved in the game world"); 
				}

				FString Payload = FString::Printf(TEXT(
					"{ \"ActorName\": \"%s\", \"From\": { \"X\": %.2f, \"Y\": %.2f, \"Z\": %.2f }, \"To\": { \"X\": %.2f, \"Y\": %.2f, \"Z\": %.2f }, \"RelativeMovement\": \"%s\" }"),
					*TrackedObject.ActorName,
					TrackedObject.PreviousPosition.X, TrackedObject.PreviousPosition.Y, TrackedObject.PreviousPosition.Z,
					CurrentPosition.X, CurrentPosition.Y, CurrentPosition.Z,
					*MovementDescription);
				*/

				if (UWorld* World = GetWorld())
				{
					HttpHandler = CreateWidget<UHttpHandler_Get>(World, UHttpHandler_Get::StaticClass());
					if (HttpHandler)
					{
						HttpHandler->AddToViewport();

						HttpHandler->httpSendReq(NewPayload);

					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("HttpHandler is null or Payload is empty!"));
					}
				}
			}
		}
	}
}

FString AprojectGameMode::GetPlayerRelativity(const AActor* TargetActor)
{
	if (!TargetActor || !TrackedObjects.Num())
		return TEXT("Invalid actor or no tracked objects.");

	FVector PlayerPosition = FVector::ZeroVector;

	for (const FTrackedObject& Object : TrackedObjects)
	{
		if (Object.IsPlayer && Object.Actor)
		{
			PlayerPosition = Object.Actor->GetActorLocation();
			break;
		}
	}

	FVector RelativeVector = TargetActor->GetActorLocation() - PlayerPosition;
	float Distance = RelativeVector.Size(); // Get magnitude of vector
	FString Quadrant = DetermineQuadrant(RelativeVector);
	float Angle = CalculateAngle(RelativeVector);

	FString Result = FString::Printf(TEXT(
		"{ \"TargetActor\": \"%s\", \"Distance\": %.2f, \"Angle\": %.2f, \"Quadrant\": \"%s\" }"),
		*TargetActor->GetName(),
		Distance,
		Angle,
		*Quadrant);

	return Result;
}

FString AprojectGameMode::DetermineQuadrant(const FVector& RelativeVector)
{
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
	return FMath::Atan2(RelativeVector.Y, RelativeVector.X) * (180.f / PI);
}
