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
#include "Components/PrimitiveComponent.h"
#include "HttpHandler_Get.h"

bool generate_stories = true;

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
	WholeWorldJson();
	UE_LOG(LogTemp, Warning, TEXT("GAMEMODE TRIGGERED."));

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
			4.0f,                              // Delay (seconds)
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
	   /*
		Content:
		- GameTitle
		- Time in game?
		- Level
		- Active players
		- Weather
		- Player, ID, name, position,
		- health
		- Objects / Actors summary: Num of Actors, types of actors
		- Provide a summary of all the objects, so maybe the LLM can make out of what it is:


		*/
	
	FString prompt = "Give a summary in form of a story of the data provided.";

	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject());
	TArray<TSharedPtr<FJsonValue>> ActorsArray;

	UWorld* World = GetWorld();
	if (World)
	{
		// GameTitle, Time in game, Active Players, Level, Weather
		RootObject->SetStringField(TEXT("game_title"), GetWorld()->GetName());
		RootObject->SetStringField(TEXT("time_in_game"), FString::SanitizeFloat(GetWorld()->GetTimeSeconds()));
		RootObject->SetStringField(TEXT("level"), GetWorld()->GetMapName());

		// Make a block for getting weather data
		// RootObject->SetStringField(TEXT("weather"), TEXT("Unknown"));
	}
	//Player ID, name, position

	TArray<TSharedPtr<FJsonValue>> PlayerJsonArray; 
	TArray<AActor*> PlayerActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), PlayerActors);
	for (AActor* PlayerActor : PlayerActors)
	{
		if (APawn* Pawn = Cast<APawn>(PlayerActor))
		{
			if (Pawn->IsPlayerControlled())
			{
				TSharedPtr<FJsonObject> PlayerJson = MakeShareable(new FJsonObject);

				PlayerJson->SetStringField(TEXT("id"), PlayerActor->GetName());
				PlayerJson->SetObjectField(TEXT("position"), SerializeVector(Pawn->GetActorLocation())); 
				PlayerJsonArray.Add(MakeShareable(new FJsonValueObject(PlayerJson))); 
			}
			else
			{
				TSharedPtr<FJsonObject> NPCJson = MakeShareable(new FJsonObject);

				NPCJson->SetStringField(TEXT("id"), PlayerActor->GetName());
				NPCJson->SetObjectField(TEXT("position"), SerializeVector(Pawn->GetActorLocation()));
				PlayerJsonArray.Add(MakeShareable(new FJsonValueObject(NPCJson)));
			}
		}
	}

	TArray<TSharedPtr<FJsonValue>> ObjectsJsonArray;
	TArray<AActor*> WorldObjects;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), WorldObjects);
	for (AActor* WorldObject : WorldObjects)
	{
		if (WorldObject)
		{
			if (!WorldObject->IsA(APawn::StaticClass()))
			{
				TSharedPtr<FJsonObject> ObjectJson = MakeShareable(new FJsonObject);
				ObjectJson->SetStringField(TEXT("id"), WorldObject->GetActorLabel());
				ObjectJson->SetObjectField(TEXT("position"), SerializeVector(WorldObject->GetActorLocation()));

				if (WorldObject->ActorHasTag(TEXT("Interactable")))
				{
					ObjectJson->SetStringField(TEXT("type"), TEXT("Interactable"));
				}
				else
				{
					UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(WorldObject->GetRootComponent());
					if (RootComp && RootComp->IsSimulatingPhysics())
					{
						ObjectJson->SetStringField(TEXT("type"), TEXT("Movable"));
					}
					else
					{
						ObjectJson->SetStringField(TEXT("type"), TEXT("Static"));
					}
				}
				ObjectsJsonArray.Add(MakeShareable(new FJsonValueObject(ObjectJson)));

			}
		}
	}
	if (PlayerJsonArray.Num() > 0)
	{
		RootObject->SetArrayField(TEXT("Players"), PlayerJsonArray);
	}

	if (ObjectsJsonArray.Num() > 0)
	{
		RootObject->SetArrayField(TEXT("Objects"), ObjectsJsonArray);
	}

	FString WholeWorldJson;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&WholeWorldJson);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);
	UE_LOG(LogTemp, Log, TEXT("Generated JSON: %s"), *WholeWorldJson);

	FString context = "WholeGameState";

	if (generate_stories == false)
	{
		FString indicator = "ActorMovement";
		SendPayload(WholeWorldJson, indicator, context);
	}
	if (generate_stories == true)
	{
		FString indicator = "StoryActorMovement";
		SendPayload(WholeWorldJson, indicator, context);
	}
	
}

TSharedPtr<FJsonObject> AprojectGameMode::SerializeVector(const FVector& Vector)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetNumberField(TEXT("x"), Vector.X);
	JsonObject->SetNumberField(TEXT("y"), Vector.Y);
	JsonObject->SetNumberField(TEXT("z"), Vector.Z);
	return JsonObject;
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
			//UE_LOG(LogTemp, Log, TEXT("PLAYER LOCATION: %s"), *PlayerPosition.ToString());
			break;
		}
	}
	// looping for all Actors
	for (FTrackedObject& TrackedObject : TrackedObjects)
	{
		if (TrackedObject.Actor && IsValid(TrackedObject.Actor) && !TrackedObject.Actor->IsA(ACharacter::StaticClass()) && !TrackedObject.Actor->IsA(APlayerCameraManager::StaticClass()))
		{
			FVector CurrentPosition = TrackedObject.Actor->GetActorLocation();
			//UE_LOG(LogTemp, Log, TEXT("ACTOR LOCATION: %s"), *CurrentPosition.ToString());

			if (!CurrentPosition.Equals(TrackedObject.PreviousPosition, KINDA_SMALL_NUMBER))
			{
				GetPlayerRelativity(TrackedObject.Actor);
				TrackedObject.PreviousPosition = CurrentPosition;
			}
		}
	}
}
void AprojectGameMode::SendPayload(const FString& Payload, const FString& indicator, const FString& context)
{
	FString question = question_prompt(indicator);
	UE_LOG(LogTemp, Log, TEXT("SEND PAYLOAD FUNCTION TRIGGERED"));
	if (UWorld* World = GetWorld())
	{
		HttpHandler = CreateWidget<UHttpHandler_Get>(World, UHttpHandler_Get::StaticClass());
		if (HttpHandler)
		{
			HttpHandler->AddToViewport();
			UE_LOG(LogTemp, Log, TEXT("INSIDE SENDPAYLOAD BLOCK"));
			HttpHandler->httpSendReq(Payload, question, context);

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
	FVector PlayerUp = FVector::UpVector; 

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
	float Distance = RelativeVector.Size();
	UE_LOG(LogTemp, Log, TEXT("Distances: %.2f | %s"), Distance, *name);

	FVector NormalizedRelativeVector = RelativeVector.GetSafeNormal();

	// playerforward and playerright are already normalized
	float ForwardDot = FVector::DotProduct(PlayerForward, NormalizedRelativeVector);
	float RightDot = FVector::DotProduct(PlayerRight, NormalizedRelativeVector);
	float VerticalDot = FVector::DotProduct(PlayerUp, NormalizedRelativeVector);

	UE_LOG(LogTemp, Log, TEXT("Normalized Vector Forward: %.2f"), ForwardDot);
	UE_LOG(LogTemp, Log, TEXT("Normalized Vector Right: %.2f"), RightDot);
	UE_LOG(LogTemp, Log, TEXT("Normalized Vector Vertical: %.2f"), VerticalDot);

	if (Distance < 800)
	{
		FString Payload = GetRelativePosition(ForwardDot, RightDot, VerticalDot);
		UE_LOG(LogTemp, Log, TEXT("Relativity Data: %s"), *Payload);

		FString Result = FString::Printf(TEXT(
			"{ \"TargetObject\": \"%s\", \"Distance\": %.2f, \"Relative Position\": \"%s\" }"),
			*TargetActor->GetActorLabel(),
			Distance,
			*Payload);
		UE_LOG(LogTemp, Log, TEXT("Relativity Data: %s"), *Result);

		DrawDebugLine(GetWorld(), PlayerPosition, ActorLocation, FColor::Green, false, 5.0f, 0, 5.0f);

		if (generate_stories == false)
		{
			FString indicator = "ActorMovement";
			FString Context = "Actor_Relocation";
			SendPayload(Result, indicator, Context); 
		}
		if (generate_stories == true)
		{
			FString indicator = "StoryActorMovement";
			FString Context = "Actor_Relocation"; 
			SendPayload(Result, indicator, Context); 
		}
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

FString AprojectGameMode::GetRelativePosition(const float& ForwardDot, const float& RightDot, const float& VerticalDot)
{
	FString Position;

	const float VerticalThreshold = 0.8f;
	const float HorizontalThreshold = 0.8f; 

	// Cases for when object is directly to any direction:
	if (VerticalDot > VerticalThreshold)
	{
		Position = TEXT("directly above the player");
	}
	else if (VerticalDot < -VerticalThreshold)
	{
		Position = TEXT("directly below the player");
	}

	else if (RightDot > HorizontalThreshold)
	{
		Position = TEXT("directly to the right of the player");
	}
	else if (RightDot < -HorizontalThreshold)
	{
		Position = TEXT("directly to the left of the player");
	}

	else if (ForwardDot > HorizontalThreshold)
	{
		Position = TEXT("directly in front of the player");
	}
	else if (ForwardDot < -HorizontalThreshold)
	{
		Position = TEXT("directly behind the player");
	}
	else
	{
		// cases when its not in a direct direction
		if (ForwardDot > 0)
		{
			if (RightDot > 0)
			{
				if (VerticalDot > 0)
				{
					Position = TEXT("in front-right-above the player");
				}
				else
				{
					Position = TEXT("in front-right-below the player");
				}
			}
			else 
			{
				if (VerticalDot > 0) 
				{
					Position = TEXT("in front-left-above the player");
				}
				else
				{
					Position = TEXT("in front-left-below the player");
				}
			}
		}
		else 
		{
			if (RightDot > 0) 
			{
				if (VerticalDot > 0) 
				{
					Position = TEXT("behind-right-above the player");
				}
				else
				{
					Position = TEXT("behind-right-below the player");
				}
			}
			else 
			{
				if (VerticalDot > 0)
				{
					Position = TEXT("behind-left-above the player");
				}
				else 
				{
					Position = TEXT("behind-left-below the player");
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Actor is %s."), *Position);

	return FString::Printf(TEXT("Actor is %s."), *Position);

}

FString AprojectGameMode::question_prompt(const FString& indicator)
{
	if (indicator == "ActorMovement")
	{
		return TEXT("Tell the ObjectName, its distance and the relative position that is mentioned in the provided data. Stop when you have said it. Do not answer with json like format. Answer in regular text. \n");
	}
	else if (indicator == "WholeJson")
	{
		return TEXT("Tell the Game Title, weather, number of players, and number of objects from the provided data. Stop when you have said it. Do not answer with json like format. Answer in regular text.\n");
	}
	else if (indicator == "StoryActorMovement")
	{
		return TEXT("Write a short scene using the following details in 50 tokens or less.\n");
	}

	else if (indicator == "StoryWholeJson")
	{
		return TEXT("Write a short story using the following details without explaining your process.\n");
	}  
	else
	{
		if (indicator == "Story")
		{
			return TEXT("Generate a story based on the provided data. Stop when you have said it. Do not answer with json like format. Answer in regular text.\n");
		}
		else
		{
			return TEXT("Describe whats happening from the the provided data. Stop when you have said it. Do not answer with json like format. Answer in regular text.\n");
		}
	}
}
