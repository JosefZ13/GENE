// Fill out your copyright notice in the Description page of Project Settings.


#include "HttpHandler_Get.h"
#include "Components/TextBlock.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "JsonUtilities.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"

void UHttpHandler_Get::NativeConstruct()
{
    Super::NativeConstruct();

    FetchGameState();
}

void UHttpHandler_Get::httpSendReq(FString *PayloadJson)
{
    UE_LOG(LogTemp, Log, TEXT("Payload received"));
   // UE_LOG(LogTemp, Log, TEXT("Payload received: %s"), *PayloadJson);

   // FString prompt = FString::Printf(TEXT("What is the difference between the old json and the new json? \n"));

    // Send HTTP request with the generated game state
    FHttpModule* Http = &FHttpModule::Get();
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();

    //TSharedPtr<FJsonObject> JsonRequest = MakeShareable(new FJsonObject);
    //JsonRequest->SetStringField(TEXT("prompt"), prompt);

    //FString RequestBody;
    //TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
   // FJsonSerializer::Serialize(JsonRequest.ToSharedRef(), Writer);

    Request->SetURL(TEXT("http://127.0.0.1:1234/v1/completions")); // LM Studio URL
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
    Request->SetContentAsString(*PayloadJson);
    Request->OnProcessRequestComplete().BindUObject(this, &UHttpHandler_Get::OnResponseReceived);
    Request->ProcessRequest();
}

/*
FString UHttpHandler_Get::GenerateGameStateFromJson(const FString& JsonContent)
{
    FString GameState = "Game state:\n";

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonContent);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        // Extract "Actors" array
        const TArray<TSharedPtr<FJsonValue>>* Actors;
        if (JsonObject->TryGetArrayField(TEXT("Actors"), Actors))
        {
            for (const TSharedPtr<FJsonValue>& ActorValue : *Actors)
            {
                TSharedPtr<FJsonObject> Actor = ActorValue->AsObject();
                if (Actor.IsValid())
                {
                    FString Name = Actor->GetStringField(TEXT("Name"));
                    FString Class = Actor->GetStringField(TEXT("Class"));
                    TSharedPtr<FJsonObject> Location = Actor->GetObjectField(TEXT("Location"));

                    float X = Location->GetNumberField(TEXT("X"));
                    float Y = Location->GetNumberField(TEXT("Y"));
                    float Z = Location->GetNumberField(TEXT("Z"));

                    GameState += FString::Printf(TEXT("%s (%s) at (%.1f, %.1f, %.1f)\n"), *Name, *Class, X, Y, Z);
                }
            }
        }

        // Extract player details
        TSharedPtr<FJsonObject> Player = JsonObject->GetObjectField(TEXT("Player"));
        if (Player.IsValid())
        {
            FString PlayerName = Player->GetStringField(TEXT("Name"));
            FString Status = Player->GetObjectField(TEXT("State"))->GetStringField(TEXT("Status"));

            GameState += FString::Printf(TEXT("Player %s is currently %s.\n"), *PlayerName, *Status);
        }
    }

    return GameState;
}
*/
/*
FString UHttpHandler_Get::LoadJsonFromFile(const FString& FilePath)
{
    FString FullPath = FPaths::ProjectContentDir() + FilePath;
    FString JsonContent;

    // Check if the file exists and read it
    if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*FullPath))
    {
        if (FFileHelper::LoadFileToString(JsonContent, *FullPath))
        {
            UE_LOG(LogTemp, Warning, TEXT("Successfully loaded JSON from file: %s"), *FullPath);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to load JSON content from file: %s"), *FullPath);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("JSON file does not exist: %s"), *FullPath);
    }

    return JsonContent;
}*/

void UHttpHandler_Get::FetchGameState()
{

    FString prompt = FString::Printf(TEXT("What is the difference between the old json and the new json?\n"));

    // Send HTTP request with the generated game state
    FHttpModule* Http = &FHttpModule::Get();
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();

    Request->SetURL(TEXT("http://127.0.0.1:1234/v1/completions")); // LM Studio URL
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));

    TSharedPtr<FJsonObject> JsonRequest = MakeShareable(new FJsonObject);
    JsonRequest->SetStringField(TEXT("prompt"), prompt);

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonRequest.ToSharedRef(), Writer);
    Request->SetContentAsString(RequestBody);
    Request->OnProcessRequestComplete().BindUObject(this, &UHttpHandler_Get::OnResponseReceived);
    Request->ProcessRequest();

}

void UHttpHandler_Get::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("HTTP request was successful."));

        TSharedPtr<FJsonObject> JsonResponse;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

        if (FJsonSerializer::Deserialize(Reader, JsonResponse) && JsonResponse.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("JSON response deserialized successfully."));

            const TArray<TSharedPtr<FJsonValue>>* ChoicesArray;
            if (JsonResponse->TryGetArrayField(TEXT("choices"), ChoicesArray) && ChoicesArray && ChoicesArray->Num() > 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("'choices' array found with %d elements."), ChoicesArray->Num());

                // Access the first element of the "choices" array
                TSharedPtr<FJsonValue> ChoiceValue = (*ChoicesArray)[0];  // Access the first element
                if (ChoiceValue.IsValid())
                {
                    TSharedPtr<FJsonObject> ChoiceObject = ChoiceValue->AsObject();  // Convert it to a JsonObject
                    if (ChoiceObject.IsValid())
                    {
                        FString AIResponse;
                        if (ChoiceObject->TryGetStringField(TEXT("text"), AIResponse))
                        {
                            UE_LOG(LogTemp, Warning, TEXT("AI response: %s"), *AIResponse);

                            // Update the text block with the response
                            if (GameStateText)
                            {
                                GameStateText->SetText(FText::FromString(AIResponse));
                                UE_LOG(LogTemp, Warning, TEXT("GameStateText updated successfully."));
                            }
                            else
                            {
                                UE_LOG(LogTemp, Error, TEXT("GameStateText is null."));
                            }
                            return; // Successfully processed
                        }
                        else
                        {
                            UE_LOG(LogTemp, Error, TEXT("Failed to extract 'text' field from 'choices[0]'."));
                        }
                    }
                    else
                    {
                        UE_LOG(LogTemp, Error, TEXT("ChoiceObject is invalid."));
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("ChoiceValue is invalid."));
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to find or parse 'choices' array."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to deserialize JSON response."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("HTTP request failed or response is invalid."));
    }

    // Fallback: Handle errors and update UI with a failure message
    if (GameStateText)
    {
        GameStateText->SetText(FText::FromString(TEXT("Failed to fetch or parse game state.")));
        UE_LOG(LogTemp, Warning, TEXT("Fallback: Displayed failure message in GameStateText."));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameStateText is null in fallback."));
    }
}


