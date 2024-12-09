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

void UHttpHandler_Get::httpSendReq(FString* PayloadJson)
{
    UE_LOG(LogTemp, Log, TEXT("Payload received"));

    // Combine Payload and Prompt
    FString prompt = TEXT("Describe the relative movement from this JSON object:\n");
    FString CombinedPayload = FString::Printf(TEXT("{\"prompt\": \"%s%s\"}"), *prompt, **PayloadJson);
    UE_LOG(LogTemp, Log, TEXT("COMBINED PAYLOAD %s"), *CombinedPayload);

    // Send HTTP Request
    FHttpModule* Http = &FHttpModule::Get();
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();

    Request->SetURL(TEXT("http://127.0.0.1:2345/v1/completions")); // Adjust URL as needed
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));

    Request->SetContentAsString(CombinedPayload);
    Request->OnProcessRequestComplete().BindUObject(this, &UHttpHandler_Get::OnResponseReceived);
    if (Request->ProcessRequest())
    {
        UE_LOG(LogTemp, Log, TEXT("LOG _____ HTTP  request successfully sent."));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("LOG _____ Failed to send HTTP request."));
    }
}

void UHttpHandler_Get::FetchGameState()
{
    UE_LOG(LogTemp, Log, TEXT("FetchGameState called."));
    FString ExamplePayload = TEXT("{ \"example\": \"game state data\" }"); // Example payload
    httpSendReq(&ExamplePayload);
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
            const TArray<TSharedPtr<FJsonValue>>* ChoicesArray;
            if (JsonResponse->TryGetArrayField(TEXT("choices"), ChoicesArray) && ChoicesArray && ChoicesArray->Num() > 0)
            {
                TSharedPtr<FJsonValue> ChoiceValue = (*ChoicesArray)[0];
                if (ChoiceValue.IsValid())
                {
                    TSharedPtr<FJsonObject> ChoiceObject = ChoiceValue->AsObject();
                    if (ChoiceObject.IsValid())
                    {
                        FString AIResponse;
                        if (ChoiceObject->TryGetStringField(TEXT("text"), AIResponse))
                        {
                            UE_LOG(LogTemp, Warning, TEXT("AI response: %s"), *AIResponse);

                            if (GameStateText)
                            {
                                GameStateText->SetText(FText::FromString(AIResponse));
                                return; // Successfully processed
                            }
                        }
                    }
                }
            }
        }
    }

    // Fallback in case of errors
    if (GameStateText)
    {
        GameStateText->SetText(FText::FromString(TEXT("Failed to process LLM response.")));
    }
}
