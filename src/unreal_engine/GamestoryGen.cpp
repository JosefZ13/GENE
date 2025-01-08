// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStateStoryGen.h"
#include "projectCharacter.h"
#include "GameFramework/Actor.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/ConstructorHelpers.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Kismet/GameplayStatics.h"
#include "JsonUtilities.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Camera/CameraComponent.h"
#include "EngineUtils.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpModule.h"
#include "HAL/PlatformFilemanager.h"
#include "Engine/Engine.h"
#include "Misc/DateTime.h"

/**
 * # File: GameStateStoreGen.cpp
 *
 * ## Brief
 * Implements the core story generation system logic.
 *
 * This file contains the main functions for managing story generation.
 * 
 * Documentation is done with Markdown and Doxygen.
 */

AGameStateStoryGen::AGameStateStoryGen()
{
	//RetrievedApiKey = "DefaultApiKey";
}

void AGameStateStoryGen::BeginPlay()
{
	/**
	 * # Initializes the Game State
	 *
	 * ## Brief
	 * Initializes the game state when the game session starts.
	 *
	 * This method is called when the GameState is initiated at the start of the game.
	 *
	 * ## Tasks Performed
	 * - Clears the content of the `LLM_response.txt` file in the `LLM_Response` directory to reset logs.
	 * - Gathers all actors and players from the game world by calling `GetActors()`.
	 * - Creates the initial game environment representation by calling `GenerateStartEnvironment()`.
	 * - Starts `TickObjectMovement()`, which triggers `PerformTracking` periodically.
	 *
	 * ## Details
	 * The GameState is typically responsible for managing the overall state of the game world.
	 * In this implementation, additional support is provided for generating story narratives.
	 *
	 * The logic prepares tracking systems and initializes game world state data.
	 */
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
	/**
	 * # Function: TickObjectMovement
	 *
	 * ## Brief
	 * Sets up a repeating timer to trigger actor movement tracking.
	 *
	 * This method configures a timer that periodically calls the `PerformTracking` function
	 * to track and log actor movements in the game world. The timer is set with the following parameters:
	 *
	 * - **Timer Handle**: A reference to the timer used for this operation.
	 * - **Owner Object**: The current instance of `AGameStateStoryGen`.
	 * - **Function to Call**: `AGameStateStoryGen::PerformTracking`.
	 * - **Delay**: 3.0 seconds between each call.
	 * - **Looping**: Set to `true` for continuous repetition.
	 *
	 * ## Details
	 * The timer manager is accessed from the game world to set the timer. If the game world is unavailable
	 * (e.g., in a null state), a warning is logged, and the timer is not initialized.
	 *
	 * ## Notes
	 * This method is typically called during initialization (e.g., in `BeginPlay()`).
	 *
	 * ## See Also
	 * - `PerformTracking()`
	 */

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,    
			this,            
			&AGameStateStoryGen::PerformTracking, 
			3.0f,                             
			true                               
		);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GetWorld is null. Timer not set."));
	}
}


TSharedPtr<FJsonObject> AGameStateStoryGen::GenerateStartEnvironment()
{
	/**
	 * # Function: GenerateStartEnvironment
	 *
	 * ## Brief
	 * Generates and returns a JSON object representing the current environment state.
	 *
	 * ## Included Data
	 * ### General Data about the environment:
	 * - **Game Title**: The title of the environment.
	 * - **World Theme**: The theme of the environment.
	 * - **Space and Bounds**: Defines the interaction zone.
	 * - **World Description**: A description of the game world.
	 *
	 * ### Actors Data:
	 * - **Objects**:
	 *   - **Name**: The name of the object (string).
	 *   - **Description**: A brief description of the object (string).
	 *   - **Type**: The type of the object (string).
	 *   - **Interactable**: A string that specifies a boolean.
	 *   - **Position and Dimensions**: The object's coordinates in the world.
	 */

	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject());
	RootObject->SetStringField(TEXT("game_title"), GameTitle);

	RootObject->SetStringField(TEXT("theme"), Theme);

	RootObject->SetStringField(TEXT("description"), Description);
	UE_LOG(LogTemp, Log, TEXT("GAME TITLE: %s"), *GameTitle);


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

		Space->SetStringField(TEXT("id"), "World outline");
		Space->SetObjectField(TEXT("dimensions"), dimensions);
		Space->SetObjectField(TEXT("bounds"), bounds);

		FString SpaceSer;
		TSharedRef<TJsonWriter<>> Writer_ = TJsonWriterFactory<>::Create(&SpaceSer);
		FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer_);

		RootObject->SetObjectField(TEXT("space"), Space);

		// Get and store all relevant Actors in ObjectsArray
		TArray<AActor*> AllActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
		
		TArray<TSharedPtr<FJsonValue>> ObjectsArray;

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
							continue;
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

						// default values
						FString Description_Tag = TEXT("No description available");
						FString Type_Tag = TEXT("generic");
						FString Interactable_Tag = TEXT("false");

						for (const FName& Tag : Actor->Tags)
						{
							FString TagString = Tag.ToString();
							if (TagString.StartsWith(TEXT("Description:")))
							{
								Description_Tag = TagString.Mid(12).TrimStartAndEnd();
							}
							else if (TagString.StartsWith(TEXT("Type:")))
							{
								Type_Tag = TagString.Mid(5).TrimStartAndEnd();
							}
							else if (TagString.StartsWith(TEXT("Interactable:")))
							{
								Interactable_Tag = TagString.Mid(13).TrimStartAndEnd();
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

						TSharedPtr<FJsonObject> ObjectJson = MakeShareable(new FJsonObject());
						ObjectJson->SetStringField(TEXT("name"), Label);
						ObjectJson->SetStringField(TEXT("description"), Description_Tag);
						ObjectJson->SetStringField(TEXT("type"), Type_Tag);
						ObjectJson->SetStringField(TEXT("Interactable"), Interactable_Tag);
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
	return RootObject;
}

void AGameStateStoryGen::GetActors()
{
	/**
	 * # GetActors
	 *
	 * ## Brief
	 * Tracks and saves the player and relevant actors in the environment.
	 *
	 * ## Details
	 * This function identifies the player pawn's, mon-player, non-camera actors and adds it to the tracked objects list. 
	 * Player's pawn are marked wit the "IsPlayer" boolean.
	 *
	 * Actors are saved as a FTrackedObject struct in TrackedObjects Array. See headerfile for decleration.
	 * 
	 * See the header file for the declarations.
	 */
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
	/**
	* # Function: PerformTracking()
	* 
	* ## Brief
	* Tracks and logs movements of the player and relevant actors in the game world.
	* 
	* ## Details
	* This function performs the following tasks:
	* - Identifies the local player's position by checking tracked objects with the 'IsPlayer' boolean.
	* - Compares the curent and previous positions of tracked actors to detect movements.
	* - Logs the movement data for actors that have changed positions.
	* - Calls `GetPkayerRelaitivity()` for actors with movement, to determine their position relative to the player.
	* 
	* ## Notes
	* - Player position is identified using the local player's controler and tracked objects marked with 'IsPlayer'.
	* - Only actors that are considered valid are processed. (non-player, non-camera manager actors)
	* - Movement data is updated in the 'TrackedObjects' array.
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
	/**
	* # Function: SerializeVector()
	* 
	* ## Brief
	* Convers an FVector into a JSON object.
	*/
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetNumberField(TEXT("x"), Vector.X);
	JsonObject->SetNumberField(TEXT("y"), Vector.Y);
	JsonObject->SetNumberField(TEXT("z"), Vector.Z);
	return JsonObject;
}

void AGameStateStoryGen::GetPlayerRelativity(const AActor* TargetActor)
{
	/**
	* # Function: GetPlayerRelativity()
	*
	* ## Brief
	* Calcuates and logs the positoin of a target actor relative to the player's location and orientation.
	* 
	* ## Details
	* This function performs the following tasks:
	* - Retrive the local player's position, forward vector, and right vector.
	* - Calculates the relative position of the TargetActor using dotproduct with the player's orientation vectors.
	* - Logs the relative position, distance from the player and other details.
	* - Creates a JSON object containing the actor's relative position, distance and a timestamp.
	* - Sends the JSOn object by calling `SendPayload()` function if the movement was within a specified distance: 800 units in Unreal Engine.
	* 
	* ## Note
	* The specified distance can be changed in the condition at line 458.
	* 
	* ## See also
	* - GetRelativePosition()
	* - SendPayload()
	*/
	TSharedPtr<FJsonObject> EventObject = MakeShareable(new FJsonObject());

	if (!TargetActor || !TrackedObjects.Num())
		UE_LOG(LogTemp, Warning, TEXT("Invalid actor or no tracked objects."));

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

	FVector NormalizedRelativeVector = RelativeVector.GetSafeNormal();

	double ForwardDot = FVector::DotProduct(PlayerForward, NormalizedRelativeVector);
	double RightDot = FVector::DotProduct(PlayerRight, NormalizedRelativeVector);
	double VerticalDot = FVector::DotProduct(PlayerUp, NormalizedRelativeVector);

	if (Distance < 800)
	{
		FString RelativePosition = GetRelativePosition(ForwardDot, RightDot, VerticalDot);

		EventObject->SetStringField(TEXT("Object"), *TargetActor->GetActorLabel());
		EventObject->SetNumberField(TEXT("Distance from player"), Distance);
		EventObject->SetStringField(TEXT("Relative Position to player"), RelativePosition);

		FDateTime CurrentTime = FDateTime::Now();
		FString Timestamp = CurrentTime.ToString(TEXT("%Y-%m-%d %H:%M:%S"));
		EventObject->SetStringField(TEXT("TimeStamp"), Timestamp);

		FString SerializedJson;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&SerializedJson);
		SendPayload(EventObject);
	}
}

FString AGameStateStoryGen::GetRelativePosition(const double& ForwardDot, const double& RightDot, const double& VerticalDot)
{
	/**
	* # Function: GetRelativePosition()
	*
	* ## Brief
	* This function determines the direction relative position as a string description.
	* Returns a FString.
	*
	*/
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
		// cases for when its not in a direct direction
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
	return FString::Printf(TEXT("Actor is %s."), *Position);
}

void AGameStateStoryGen::SendPayload(const TSharedPtr<FJsonObject>& Event)
{
	/**
	* # Function: SendPayload()
	*
	* ## Brief
	* Adds a new event ot the event history and triggers a payload request if the history size reached.
	* 
	* ## Details
	* This function manages the 'EventHistoryArray' and calls the `httpSendReq()` method if the size of the array is reached.
	* 
	* It performs the following tasks:
	* - Adds the provided event to the EventHistoryArray.
	* - If the number of events exceeds the History_Size limit (10):
	*	- Restes the Event_Count.
	*	- Regenerates the current environment using GenerateStartEnvironment().
	*	- Calls httpSendReq()
	* - Ensure the event history does not exceed `History_Size` by removing the oldest entry.
	* 
	* ## See also
	* - GenerateStartEnvironment()
	* - httpSendReq()
	*
	*/
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
	/**
	* # Function: httpSendReq()
	* 
	* ## Brief
	* Sends an HTTP request to an external API with a JSON payload containing game environment and event data.
	* 
	* ## Details
	* This function constructs a JSON payload with the following compontents:
	* - **Start Environment**: The initial game environment state.
	* - **Current Environment**: The current game environment state.
	* - **Event History**: A list of recent events in the game.
	* - **Old AI Responses**: Previous responses from the AI for context.
	* 
	* The JSOn payload is serialized and sent as a POST request to the specified API endpoint.
	* - Sets up the HTTP headers and content for the requests.
	* - Binds the response to the `OnResponseReceieved` handler for processing the respons.
	* - Logs any failures.
	* 
	* ## Note
	* - The function uses OpenAI's api key for generating the narrative content based on the game state.
	* - Ensure the `apiKey` is correct.
	* - The SystemMessage content holds the question asked. Can me modified if needed. 
	* 
	* ## See also
	* - OnResponseReceieved()
	*/
	//FString apiKey = "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";
	UE_LOG(LogTemp, Log, TEXT("httpsendreq triggered"));
	UE_LOG(LogTemp, Log, TEXT("APIKEY: %s"), *RetrievedApiKey);

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

	Request->SetURL(TEXT("https://api.openai.com/v1/chat/completions"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
	Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *RetrievedApiKey));
	Request->SetContentAsString(SerializedPayload);

	Request->OnProcessRequestComplete().BindUObject(this, &AGameStateStoryGen::OnResponseReceived);
	if (Request->ProcessRequest())
	{
		UE_LOG(LogTemp, Log, TEXT("Payload sent to LLM-service (OpenAI)"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to send payload to LLM-service (OpenAI)"));
	}
}

void AGameStateStoryGen::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	/**
	 * # OnResponseReceived
	 *
	 * ## Brief
	 * Handles the HTTP response from an external API and processes the returned JSON data.
	 *
	 * ## Details
	 * This function performs the following tasks:
	 * 1. Checks if the response was successful and valid.
	 * 2. Parses the JSON string.
	 * 3. Extracts the respons from the `choices` array from the JSON object.
	 * 4. Logs the content and writes it to a file (`LLM_Response/LLM_response.txt`).
	 * 5. Adds the content to the `LLMResponseArray`.
	 *
	 * If the response is invalid or the JSON parsing fails, an error is logged, and no further processing is performed.
	 *
	 * ## Notes
	 * - The response is expected to follow a specific JSON structure with a `choices` array and a `content` field.
	 * - Logs detailed messages about success or failure, aiding in debugging.
	 * - Ensure the file path (`LLM_Response/LLM_response.txt`) exists and is writable.
	 *
	 * ## See Also
	 * - httpSendReq()
	 */
	if (bWasSuccessful && Response.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("LLM Respons is valid: "));
		FString ResponseString = Response->GetContentAsString();
		// parse the json string
		TSharedPtr<FJsonObject> JsonResponse;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);
		if (!FJsonSerializer::Deserialize(Reader, JsonResponse) || !JsonResponse.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON response."));
			return;
		}

		// Navigate to the choices field and retrieve the response
		const TArray<TSharedPtr<FJsonValue>>* ChoicesArray;
		if (JsonResponse->TryGetArrayField(TEXT("choices"), ChoicesArray) && ChoicesArray->Num() > 0)
		{
			TSharedPtr<FJsonObject> ChoiceObject = (*ChoicesArray)[0]->AsObject();
			if (ChoiceObject.IsValid())
			{
				TSharedPtr<FJsonObject> MessageObject = ChoiceObject->GetObjectField(TEXT("message"));
				if (MessageObject.IsValid())
				{
					// The respons in the Content variable:
					FString Content = MessageObject->GetStringField(TEXT("content"));
					FString FilePath = FPaths::ProjectDir() + TEXT("LLM_Response/LLM_response.txt");
					UE_LOG(LogTemp, Log, TEXT("LLM Response: %s"), *Content);

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
