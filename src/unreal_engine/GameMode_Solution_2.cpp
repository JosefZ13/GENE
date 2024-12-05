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
//#include "HttpHandler_Get.h"

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
FString AprojectGameMode::GetActors() {

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

void AprojectGameMode::Tick(float deltaTime) {
	/*
	This logic should understand movements relative to the player.
	- Creates a Json-like string which is our payload.
	- Logs every movement data in the gameworld

	*/
	Super::Tick(deltaTime);

	FVector PlayerPosition = FVector::ZeroVector;
	FVector PlayerForward = FVector::ZeroVector;
	FVector PlayerRight = FVector::ZeroVector;

	// Looping for Player
	for (const FTrackedObject& Object : TrackedObjects)
	{
		if (Object.IsPlayer && Object.Actor)
		{
			PlayerPosition = Object.Actor->GetActorLocation();
			FRotator PlayerRotation = Object.Actor->GetActorRotation();
			PlayerForward = PlayerRotation.Vector(); // Forward vector
			PlayerRight = FVector::CrossProduct(PlayerForward, FVector::UpVector); // Right vector
			break;
		}
	}
		// looping for all Actors
	for (FTrackedObject& TrackedObject : TrackedObjects)
	{
		if (TrackedObject.Actor && IsValid(TrackedObject.Actor))
		{
			FVector CurrentPosition = TrackedObject.Actor->GetActorLocation();

			FString MovementDescription = TEXT("");

			if (!CurrentPosition.Equals(TrackedObject.PreviousPosition, KINDA_SMALL_NUMBER))
			{
				FVector RelativePosition = CurrentPosition - PlayerPosition;


				// DotProduct is a math function in ue. It computes two 3D vectors like:
				// DotProduct(a, B) --> AxB = Ax * Bx + Ay * By + Az * Bz
				float ForwardOffset = FVector::DotProduct(RelativePosition, PlayerForward);
				float RightOffset = FVector::DotProduct(RelativePosition, PlayerRight);

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
					movement = TEXT("Player moved in the game world");
				}

				FString Payload = FString::Printf(TEXT(
					"{ \"ActorName\": \"%s\", \"From\": { \"X\": %.2f, \"Y\": %.2f, \"Z\": %.2f }, \"To\": { \"X\": %.2f, \"Y\": %.2f, \"Z\": %.2f }, \"RelativeMovement\": \"%s\" }"),
					*TrackedObject.ActorName,
					TrackedObject.PreviousPosition.X, TrackedObject.PreviousPosition.Y, TrackedObject.PreviousPosition.Z,
					CurrentPosition.X, CurrentPosition.Y, CurrentPosition.Z,
					*MovementDescription);

				UE_LOG(LogTemp, Log, TEXT("%s"), *Payload);

				TrackedObject.PreviousPosition = CurrentPosition;
			}
		}
	}
}

void AprojectGameMode::SendPayload(FString Payload){

	if (UWorld* World = GetWorld())
	{
		HttpHandler = CreateWidget<UHttpHandler_Get>(World, UHttpHandler_Get::StaticClass());
		if (HttpHandler && !Payload.IsEmpty())
		{
			HttpHandler->AddToViewport();

			HttpHandler->httpSendReq(&Payload);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("HttpHandler is null or Payload is empty!"));
		}
	}
}
