// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_ContentRetreiver.h"
#include "LLMResponseRetriever.h"
#include "Dom/JsonObject.h"
#include "Components/TextBlock.h"
#include "Containers/Queue.h"
#include "Serialization/JsonReader.h"

float TimeSinceLastRead = 0.0f;

void UHUD_ContentRetreiver::NativeConstruct()
{
    Super::NativeConstruct();

    GameStateTexts = { GameStateText_0, GameStateText_1, GameStateText_2 };

    if (GameStateTexts.Contains(nullptr))
    {
        UE_LOG(LogTemp, Error, TEXT("One or more GameStateText widgets are not properly bound."));
    }

}


void UHUD_ContentRetreiver::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{

    Super::NativeTick(MyGeometry, InDeltaTime);
    TimeSinceLastRead += InDeltaTime;


    if (TimeSinceLastRead >= 4.0f)
    {
        FString FilePath = FPaths::ProjectDir() + TEXT("LLM_Response/LLM_response.txt");
        FString NewResponse;

        if (FFileHelper::LoadFileToString(NewResponse, *FilePath))
        {
            if (NewResponse != prevResponse)
            {
                {
                    ProcessNewResponse(NewResponse);
                    prevResponse = NewResponse;
                }
            }
            else if (!NewResponse.IsEmpty() && NewResponse == prevResponse)
            {
                UE_LOG(LogTemp, Log, TEXT("No new response. Skipping update."));
            }
        }
        TimeSinceLastRead = 0.0f;
    }
}

void UHUD_ContentRetreiver::ProcessNewResponse(const FString& NewResponse)
{
    ResponseQueue.Enqueue(NewResponse);

    AsyncTask(ENamedThreads::GameThread, [this]()
        {
            UpdateTextBlocks();
        });
}


void UHUD_ContentRetreiver::UpdateTextBlocks()
{
    if (ResponseQueue.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Response queue is empty. No updates to apply."));
        return;
    }

    FString RetrievedResponse;
    if (ResponseQueue.Dequeue(RetrievedResponse))
    {

        UE_LOG(LogTemp, Log, TEXT("Dequeued response: %s"), *RetrievedResponse);
        for (int i = GameStateTexts.Num() - 1; i > 0; --i)
        {
            if (GameStateTexts[i] && GameStateTexts[i - 1])
            {
                GameStateTexts[i]->SetText(GameStateTexts[i - 1]->GetText());
            }
        }

        if (GameStateTexts[0])
        {
            GameStateTexts[0]->SetText(FText::FromString(RetrievedResponse));
        }

        UE_LOG(LogTemp, Log, TEXT("GameStateText widgets updated successfully."));
    }
}
/*
void UHUD_ContentRetreiver::ResponseFileRead()
{
    FString FilePath = FPaths::ProjectDir() + TEXT("LLM_Response/LLM_response.txt");
    UE_LOG(LogTemp, Log, TEXT("FILEPATH:\n%s"), *FilePath);


    if (FFileHelper::LoadFileToString(newResponse, *FilePath))
    {
        if (newResponse != prevResponse)
        {
            UE_LOG(LogTemp, Log, TEXT("TEST !:\n"));
            UE_LOG(LogTemp, Log, TEXT("File read successfully! Content:\n%s"), *newResponse);

            ResponseQueue.Enqueue(newResponse);

            if (GameStateText_2 && GameStateText_1 && GameStateText_0)
            {
                AsyncTask(ENamedThreads::GameThread, [this, ResponseCopy = newResponse]()
                    {
                        FString Retrieved;
                        if (ResponseQueue.Dequeue(Retrieved))
                        {
                            
                            if (GameStateText_1 && GameStateText_0)
                            {
                                GameStateText_0->SetText(GameStateText_1->GetText());
                            }

                            if (GameStateText_2 && GameStateText_1)
                            {
                                GameStateText_1->SetText(GameStateText_2->GetText());
                            }

                            if (GameStateText_2)
                            {
                                GameStateText_2->SetText(FText::FromString(Retrieved));
                            }
                            else
                            {
                                UE_LOG(LogTemp, Warning, TEXT("Queue is empty."));
                            }
                        }
                        else
                        {
                            UE_LOG(LogTemp, Warning, TEXT("Queue is empty."));
                        }
                        UE_LOG(LogTemp, Warning, TEXT("GameStateText updated successfully."));
                    });
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("GameStateText is null."));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to read file: %s"), *FilePath);
    }
}

