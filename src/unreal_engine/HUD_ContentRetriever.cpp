// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_ContentRetreiver.h"
#include "LLMResponseRetriever.h"
#include "Dom/JsonObject.h"
#include "Components/TextBlock.h"
#include "Serialization/JsonReader.h"

float TimeSinceLastRead = 0.0f;

void UHUD_ContentRetreiver::NativeConstruct()
{
    Super::NativeConstruct();

    UE_LOG(LogTemp, Warning, TEXT("UHUD_ContentRetreiver: NativeConstruct called (BeginPlay equivalent)."));

}


void UHUD_ContentRetreiver::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{

    Super::NativeTick(MyGeometry, InDeltaTime);
    TimeSinceLastRead += InDeltaTime;


    if (TimeSinceLastRead >= 3.0f)
    {
        ResponseFileRead(); 
        TimeSinceLastRead = 0.0f; 
    }
}

void UHUD_ContentRetreiver::ResponseFileRead()
{
    FString FilePath = FPaths::ProjectDir() + TEXT("LLM_Response/LLM_response.txt");
    UE_LOG(LogTemp, Log, TEXT("FILEPATH:\n%s"), *FilePath);

    if (FFileHelper::LoadFileToString(LLM_Response, *FilePath))
    {
        UE_LOG(LogTemp, Log, TEXT("TEST !:\n"));
        UE_LOG(LogTemp, Log, TEXT("File read successfully! Content:\n%s"), *LLM_Response);
        if (GameStateText)
        {
            AsyncTask(ENamedThreads::GameThread, [this, ResponseCopy = LLM_Response]()
                {
                    GameStateText->SetText(FText::FromString(LLM_Response));
                    UE_LOG(LogTemp, Warning, TEXT("GameStateText updated successfully."));
                });
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("GameStateText is null."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to read file: %s"), *FilePath);
    }
}
