// Fill out your copyright notice in the Description page of Project Settings.


#include "HttpHandler_Get.h"
#include "Components/TextBlock.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "JsonUtilities.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "Engine/Engine.h"

void UHttpHandler_Get::NativeConstruct()
{
    Super::NativeConstruct();
   
   // FetchGameState();
    if (GameStateText)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameStateText is bound correctly!"));
        GameStateText->SetText(FText::FromString("Widget Initialized"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameStateText is null in NativeConstruct."));
    }
}

void UHttpHandler_Get::httpSendReq(FString Payload)
{
    FHttpModule* Http = &FHttpModule::Get();
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();

    FString question = FString::Printf(TEXT("Tell the ObjectName, its distance and the relative position that is mentioned in the provided data. If you have provided the information, stop telling me more.\n"));

    FString prompt = question + Payload;

    TSharedPtr<FJsonObject> JsonPayload = MakeShareable(new FJsonObject);
    JsonPayload->SetStringField(TEXT("prompt"), prompt);

    FString SerializedPayload;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&SerializedPayload);
    FJsonSerializer::Serialize(JsonPayload.ToSharedRef(), Writer);

    Request->SetURL(TEXT("http://127.0.0.1:1234/v1/completions")); // LM Studio URL
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));

    Request->SetContentAsString(SerializedPayload);
    Request->OnProcessRequestComplete().BindUObject(this, &UHttpHandler_Get::OnResponseReceived);
    if (Request->ProcessRequest())
    {
        UE_LOG(LogTemp, Log, TEXT("Payload sent: %s"), *SerializedPayload);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to send payload: %s"), *SerializedPayload);
    }
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
         
                TSharedPtr<FJsonValue> ChoiceValue = (*ChoicesArray)[0];  
                if (ChoiceValue.IsValid())
                {
                    TSharedPtr<FJsonObject> ChoiceObject = ChoiceValue->AsObject(); 
                    if (ChoiceObject.IsValid())
                    {
                        FString AIResponse;
                        if (ChoiceObject->TryGetStringField(TEXT("text"), AIResponse))
                        {
                            if (AIResponse != "")
                            {
                                UE_LOG(LogTemp, Warning, TEXT("AI response: %s"), *AIResponse);

                                FString FilePath = FPaths::ProjectDir() + TEXT("LLM_Response/LLM_response.txt");
                                //FString FileContent = *AIResponse;
                                UE_LOG(LogTemp, Warning, TEXT("FilePath: %s"), *FilePath);

                                FString FileContent = TrimResponse(*AIResponse);
                                UE_LOG(LogTemp, Log, TEXT("RESPONSE IS: %s"), *FileContent);

                                if (FFileHelper::SaveStringToFile(FileContent, *FilePath))
                                {
                                    UE_LOG(LogTemp, Log, TEXT("File written successfully to: %s"), *FilePath);
                                }
                                else
                                {
                                    UE_LOG(LogTemp, Error, TEXT("Failed to write to file: %s"), *FilePath);
                                }
                            }
                            return;
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
}

FString UHttpHandler_Get::TrimResponse(const FString& InputString)
{
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
