// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStateStoryGen.h"
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
#include "Blueprint/UserWidget.h"
#include "JsonUtilities.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpModule.h"
#include "HAL/PlatformFilemanager.h"
#include "Engine/Engine.h"
#include "Misc/DateTime.h"
#include "Components/TextBlock.h"


void AGameStateStoryGen::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("GAMESTATE TRIGGERED"));

	PrimaryActorTick.bCanEverTick = true;

	FString FilePath = FPaths::ProjectDir() + TEXT("LLM_Response/LLM_response.txt");
	if (FFileHelper::SaveStringToFile(TEXT(""), *FilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("File content cleared successfully: %s"), *FilePath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to clear file content: %s"), *FilePath);
	}

	GetActors();
	StartEnviroment = GenerateStartEnvironment();
	TickObjectMovement();
}

void AGameStateStoryGen::TickObjectMovement()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,                       // Timer Handle
			this,                              // Object that owns the function
			&AGameStateStoryGen::PerformTracking,    // Function to call
			3.0f,                              // Delay (seconds)
			true                               // Loop? (true = repeat every 5 seconds)
		);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GetWorld is null. Timer not set."));
	}
}


TSharedPtr<FJsonObject> AGameStateStoryGen::GenerateStartEnvironment()
{
	/*
	Game_title:
	Space
		- id
		- dimensions:
			- width
			- depth
			- height
	(Interaction zone):
		- id
		- spawn position
		- dimensions
	theme: Woods something
	Objects:
		- id (bluecube)
		- type (tree, cube, rock)
		- position (location)
		- dimension
		- Contents:
			- actorname
			- description
			- Interactable

	*/
	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject());
	RootObject->SetStringField(TEXT("game_title"), TEXT("Woods Adventure"));

	RootObject->SetStringField(TEXT("theme"), TEXT("Mystical Forest"));

	RootObject->SetStringField(TEXT("description"), TEXT("a mystical forest with plenty of trees, boulders and plants, surronding the forest are 4 walls. outside of the walls, there are 2 mountains"));


	if (GetWorld())
	{
		// Calculate bounds of the environment
		FVector MinExtent(FLT_MAX);
		FVector MaxExtent(-FLT_MAX);

		TSharedPtr<FJsonObject> Space = MakeShareable(new FJsonObject());
		TSharedPtr<FJsonObject> dimensions = MakeShareable(new FJsonObject());
		TSharedPtr<FJsonObject> bounds = MakeShareable(new FJsonObject());
		

		FVector EnvironmentSize = MaxExtent - MinExtent;
		dimensions->SetNumberField(TEXT("width"), EnvironmentSize.X);
		dimensions->SetNumberField(TEXT("depth"), EnvironmentSize.Y);
		dimensions->SetNumberField(TEXT("height"), EnvironmentSize.Z);

		bounds->SetNumberField(TEXT("x_min"), (-1540));
		bounds->SetNumberField(TEXT("x_max"), (4150));
		bounds->SetNumberField(TEXT("y_min"), (-730));
		bounds->SetNumberField(TEXT("y_max"), (-3830));

		//Space_Content->SetObjectField(TEXT("dimenstions"), dimensions);
		Space->SetStringField(TEXT("id"), "World outline");
		Space->SetObjectField(TEXT("dimensions"), dimensions);
		Space->SetObjectField(TEXT("bounds"), bounds);


		FString SpaceSer;
		TSharedRef<TJsonWriter<>> Writer_ = TJsonWriterFactory<>::Create(&SpaceSer);
		FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer_);

		UE_LOG(LogTemp, Log, TEXT("Space: %s"), *SpaceSer);

		RootObject->SetObjectField(TEXT("space"), Space);

		TArray<AActor*> AllActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
		
		UE_LOG(LogTemp, Log, TEXT("Number of actors: %d"), AllActors.Num());
		TArray<TSharedPtr<FJsonValue>> ObjectsArray;

		//TMap<FString, TArray<TSharedPtr<FJsonValue>>> GroupedObjects;


		for (AActor* Actor : AllActors)
		{

			if (Actor)
			{
				for (const FName& TypeTag : Actor->Tags)
				{
					FString TypeTagString = TypeTag.ToString();

					if (TypeTagString.StartsWith(TEXT("Type:")))
					{
						FVector Origin, BoxExtent;
						Actor->GetActorBounds(true, Origin, BoxExtent);

						// Filter out actors with unrealistic bounds
						if (BoxExtent.IsNearlyZero() || BoxExtent.X > 10000.0f || BoxExtent.Y > 10000.0f || BoxExtent.Z > 10000.0f)
						{
							continue; // Ignore invalid or extreme extents
						}

						// Update environment bounds
						MinExtent = FVector::Min(MinExtent, Origin - BoxExtent);
						MaxExtent = FVector::Max(MaxExtent, Origin + BoxExtent);

						// Get the actor's label
						FString Label = Actor->GetActorLabel();
						if (Label.IsEmpty())
						{
							Label = Actor->GetName();
						}

						FString Description = TEXT("No description available");
						FString Type = TEXT("generic");
						FString Interactable = TEXT("false");

						for (const FName& Tag : Actor->Tags)
						{
							FString TagString = Tag.ToString();
							if (TagString.StartsWith(TEXT("Description:")))
							{
								Description = TagString.Mid(12).TrimStartAndEnd();
							}
							else if (TagString.StartsWith(TEXT("Type:")))
							{
								Type = TagString.Mid(5).TrimStartAndEnd();
							}
							else if (TagString.StartsWith(TEXT("Interactable:")))
							{
								Interactable = TagString.Mid(13).TrimStartAndEnd();
							}
						}

						// Filter out unnecessary actors
						FString LowerCaseLabel = Label.ToLower();
						if (LowerCaseLabel.Contains(TEXT("floor")) || LowerCaseLabel.Contains(TEXT("wall")) ||
							LowerCaseLabel.Contains(TEXT("light")) || LowerCaseLabel.Contains(TEXT("debug")) ||
							LowerCaseLabel.Contains(TEXT("volume")) || BoxExtent.IsNearlyZero())
						{
							continue;
						}

						// Create JSON for each valid actor
						TSharedPtr<FJsonObject> ObjectJson = MakeShareable(new FJsonObject());
						ObjectJson->SetStringField(TEXT("name"), Label); // Use actor label
						ObjectJson->SetStringField(TEXT("description"), Description); // Add description
						ObjectJson->SetStringField(TEXT("type"), Type); // Add type
						ObjectJson->SetStringField(TEXT("Interactable"), Interactable);
						ObjectJson->SetObjectField(TEXT("position"), SerializeVector(Actor->GetActorLocation()));
						ObjectJson->SetObjectField(TEXT("dimensions"), SerializeVector(BoxExtent * 2)); // Full size (width, depth, height)

						ObjectsArray.Add(MakeShareable(new FJsonValueObject(ObjectJson)));
					
					}
				}
				RootObject->SetArrayField(TEXT("objects"), ObjectsArray);
			}
		}
	}

	FString SerializedJson;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&SerializedJson);

	if (FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer))
	{
		UE_LOG(LogTemp, Log, TEXT("Envrionment: "), *SerializedJson);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to serialize JSON object."));
	}

	//UE_LOG(LogTemp, Log, TEXT("Envrionment: "), *SerializedJson);

	return RootObject;
}

void AGameStateStoryGen::GetActors()
{

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
}

void AGameStateStoryGen::PerformTracking()
{
	/*
	This logic should understand movements relative to the player.
	- Creates a Json-like string which is our payload.
	- Logs every movement data in the gameworld

	*/

	FVector PlayerPosition = FVector::ZeroVector;

	// Find player position
	for (const FTrackedObject& Object : TrackedObjects)
	{
		if (Object.IsPlayer && Object.Actor)
		{
			PlayerPosition = Object.Actor->GetActorLocation();
			UE_LOG(LogTemp, Log, TEXT("PLAYER LOCATION: %s"), *PlayerPosition.ToString());
			break;
		}
	}

	// Check and log movements for all tracked objects
	for (FTrackedObject& TrackedObject : TrackedObjects)
	{
		if (TrackedObject.Actor && IsValid(TrackedObject.Actor) &&
			!TrackedObject.Actor->IsA(ACharacter::StaticClass()) &&
			!TrackedObject.Actor->IsA(APlayerCameraManager::StaticClass()))
		{
			FVector CurrentPosition = TrackedObject.Actor->GetActorLocation();

			// If the object has moved, log the interaction
			if (!CurrentPosition.Equals(TrackedObject.PreviousPosition, KINDA_SMALL_NUMBER))
			{
				GetPlayerRelativity(TrackedObject.Actor);
				TrackedObject.PreviousPosition = CurrentPosition; // Update the previous position
			}
		}
	}
}

TSharedPtr<FJsonObject> AGameStateStoryGen::SerializeVector(const FVector& Vector)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetNumberField(TEXT("x"), Vector.X);
	JsonObject->SetNumberField(TEXT("y"), Vector.Y);
	JsonObject->SetNumberField(TEXT("z"), Vector.Z);
	return JsonObject;
}

void AGameStateStoryGen::GetPlayerRelativity(const AActor* TargetActor)
{
	UE_LOG(LogTemp, Error, TEXT("GETPLAYER RELATIVITY Triggered"));
	TSharedPtr<FJsonObject> EventObject = MakeShareable(new FJsonObject());

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
	double Distance = RelativeVector.Size();
	UE_LOG(LogTemp, Log, TEXT("Distances: %.2f | %s"), Distance, *name);

	FVector NormalizedRelativeVector = RelativeVector.GetSafeNormal();

	// playerforward and playerright are already normalized
	double ForwardDot = FVector::DotProduct(PlayerForward, NormalizedRelativeVector);
	double RightDot = FVector::DotProduct(PlayerRight, NormalizedRelativeVector);
	double VerticalDot = FVector::DotProduct(PlayerUp, NormalizedRelativeVector);

	UE_LOG(LogTemp, Log, TEXT("Normalized Vector Forward: %.2f"), ForwardDot);
	UE_LOG(LogTemp, Log, TEXT("Normalized Vector Right: %.2f"), RightDot);
	UE_LOG(LogTemp, Log, TEXT("Normalized Vector Vertical: %.2f"), VerticalDot);

	if (Distance < 800)
	{
		FString RelativePosition = GetRelativePosition(ForwardDot, RightDot, VerticalDot);
		UE_LOG(LogTemp, Log, TEXT("Relativity Data: %s"), *RelativePosition);

		EventObject->SetStringField(TEXT("Object"), *TargetActor->GetActorLabel());
		EventObject->SetNumberField(TEXT("Distance from player"), Distance);
		EventObject->SetStringField(TEXT("Relative Position to player"), RelativePosition);

		FDateTime CurrentTime = FDateTime::Now();
		FString Timestamp = CurrentTime.ToString(TEXT("%Y-%m-%d %H:%M:%S"));
		EventObject->SetStringField(TEXT("TimeStamp"), Timestamp);

		//DrawDebugLine(GetWorld(), PlayerPosition, ActorLocation, FColor::Green, false, 5.0f, 0, 5.0f);

		FString SerializedJson;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&SerializedJson);

		if (FJsonSerializer::Serialize(EventObject.ToSharedRef(), Writer))
		{
			UE_LOG(LogTemp, Log, TEXT("Get Relativity JSON: %s"), *SerializedJson);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to serialize JSON object."));
		}

		SendPayload(EventObject);

	}
	else
	{
		//DrawDebugLine(GetWorld(), PlayerPosition, ActorLocation, FColor::Red, false, 5.0f, 0, 5.0f);
	}
}


FString AGameStateStoryGen::GetRelativePosition(const double& ForwardDot, const double& RightDot, const double& VerticalDot)
{
	FString Position;

	const double VerticalThreshold = 0.8f;
	const double HorizontalThreshold = 0.8f;

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

void AGameStateStoryGen::SendPayload(const TSharedPtr<FJsonObject>& Event)
{
	int32 History_Size = 10;
	Event_Count++;
	if (Event_Count < History_Size)
	{
		EventHistoryArray.Add(Event);
	}
	else
	{
		Event_Count = 0;
		CurrentEnviroment = GenerateStartEnvironment();
		httpSendReq();
	}

	if (EventHistoryArray.Num() >= History_Size)
	{
		EventHistoryArray.RemoveAt(0);
	}
}

void AGameStateStoryGen::httpSendReq()
{
	UE_LOG(LogTemp, Log, TEXT("httpSendReq triggered"));
	FString apiKey = "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";

	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();

	TSharedPtr<FJsonObject> JsonPayload = MakeShareable(new FJsonObject);
	JsonPayload->SetStringField(TEXT("model"), TEXT("gpt-4o"));

	TArray<TSharedPtr<FJsonValue>> MessagesArray;
	TSharedPtr<FJsonObject> SystemMessage = MakeShareable(new FJsonObject);

	SystemMessage->SetStringField(TEXT("role"), TEXT("system"));
	SystemMessage->SetStringField(TEXT("content"), TEXT("Use the json string to generate stories for the game it's retreived from the wanderers point of view in third person. Focus on the last events in Event_History to understand the changes made from Start Environment. Try to not use other concepts or imagination outside the given parameters. If you recieve a history of LLM_responeses, continue on that story. Get straight to the story."));
	MessagesArray.Add(MakeShareable(new FJsonValueObject(SystemMessage)));


	// Create json array of LLM Responses
	TArray<TSharedPtr<FJsonValue>> LLMResponseJsonArray;
	for (const FString& Response : LLMResponseArray)
	{
		LLMResponseJsonArray.Add(MakeShareable(new FJsonValueString(Response)));
	}


	TSharedPtr<FJsonObject> UserMessage = MakeShareable(new FJsonObject);
	UserMessage->SetStringField(TEXT("role"), TEXT("user"));

	// Create json array of Events
	TArray<TSharedPtr<FJsonValue>> EventJson;
	for (const TSharedPtr<FJsonObject>& Event : EventHistoryArray)
	{
		EventJson.Add(MakeShareable(new FJsonValueObject(Event)));
	}

	// Define user json string - message
	TSharedPtr<FJsonObject> User_Content = MakeShareable(new FJsonObject);
	User_Content->SetObjectField(TEXT("Start_environment"), StartEnviroment);
	User_Content->SetObjectField(TEXT("Current_environment"), CurrentEnviroment);
	User_Content->SetArrayField(TEXT("Event_History"), EventJson);
	User_Content->SetArrayField(TEXT("Old_AI_Responses"), LLMResponseJsonArray);
	
	FString SerializedContent;
	TSharedRef<TJsonWriter<>> Writer_ = TJsonWriterFactory<>::Create(&SerializedContent);
	FJsonSerializer::Serialize(User_Content.ToSharedRef(), Writer_);

	FString WrappedContent = TEXT("\"") + SerializedContent + TEXT("\"");
	UserMessage->SetStringField(TEXT("content"), WrappedContent);

	MessagesArray.Add(MakeShareable(new FJsonValueObject(UserMessage)));
	JsonPayload->SetArrayField(TEXT("messages"), MessagesArray);

	JsonPayload->SetNumberField(TEXT("temperature"), 0.7);
	JsonPayload->SetNumberField(TEXT("max_tokens"), 150);

	FString SerializedPayload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&SerializedPayload);
	FJsonSerializer::Serialize(JsonPayload.ToSharedRef(), Writer);

	UE_LOG(LogTemp, Log, TEXT("OPENAI PAYLOAD: %s"), *SerializedPayload);

	Request->SetURL(TEXT("https://api.openai.com/v1/chat/completions"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
	Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *apiKey));
	Request->SetContentAsString(SerializedPayload);

	Request->OnProcessRequestComplete().BindUObject(this, &AGameStateStoryGen::OnResponseReceived);
	if (Request->ProcessRequest())
	{
		UE_LOG(LogTemp, Log, TEXT("Payload sent to OpenAI: %s"), *SerializedPayload);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to send payload to OpenAI: %s"), *SerializedPayload);
	}


}

void AGameStateStoryGen::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("Respons function triggered"));
	if (bWasSuccessful && Response.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("Respons is valid: "));


		FString ResponseString = Response->GetContentAsString();
		UE_LOG(LogTemp, Log, TEXT("Raw Response: %s"), *ResponseString);

		// Parse the JSON string into a JSON object
		TSharedPtr<FJsonObject> JsonResponse;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);

		if (!FJsonSerializer::Deserialize(Reader, JsonResponse) || !JsonResponse.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON response."));
			return;
		}

		// Navigate to the `choices` array
		const TArray<TSharedPtr<FJsonValue>>* ChoicesArray;
		if (JsonResponse->TryGetArrayField(TEXT("choices"), ChoicesArray) && ChoicesArray->Num() > 0)
		{
			// Get the first object in the `choices` array
			TSharedPtr<FJsonObject> ChoiceObject = (*ChoicesArray)[0]->AsObject();
			if (ChoiceObject.IsValid())
			{
				// Extract the `message` object
				TSharedPtr<FJsonObject> MessageObject = ChoiceObject->GetObjectField(TEXT("message"));
				if (MessageObject.IsValid())
				{
					// Get the `content` field from the `message` object
					FString Content = MessageObject->GetStringField(TEXT("content"));
					UE_LOG(LogTemp, Log, TEXT("Assistant Response: %s"), *Content);

					FString FilePath = FPaths::ProjectDir() + TEXT("LLM_Response/LLM_response.txt");

					if (FFileHelper::SaveStringToFile(Content, *FilePath))
					{
						UE_LOG(LogTemp, Log, TEXT("File written successfully to: %s"), *FilePath);
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("Failed to write to file: %s"), *FilePath);
					}

					LLMResponseArray.Add(Content);
				}
			}
		}
	}
}

FString AGameStateStoryGen::TrimResponse(const FString& InputString)
{
	// not used
	FString Result = InputString;

	while (Result.StartsWith(TEXT("\n\n")))
	{
		Result = Result.RightChop(2);
	}

	while (Result.EndsWith(TEXT("\n\n")))
	{
		Result = Result.LeftChop(2);
	}

	return Result;
}

void AGameStateStoryGen::RecordLLMResponse(const FString& Response)
{
	const int MaxResponses = 10;

	if (LLMResponseArray.Num() >= MaxResponses)
	{
		LLMResponseArray.RemoveAt(0);
	}

}
