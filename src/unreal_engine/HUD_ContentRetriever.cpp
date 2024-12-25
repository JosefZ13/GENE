// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_ContentRetreiver.h"
#include "Dom/JsonObject.h"
#include "Components/TextBlock.h"
#include "Containers/Queue.h"
#include "Components/Border.h"
#include "Serialization/JsonReader.h"

float TimeSinceLastRead_1 = 0.0f;
float TimeSinceLastRead_2 = 0.0f;

void UHUD_ContentRetreiver::NativeConstruct()
{
    Super::NativeConstruct();

    GameStateTexts = { GameStateText_0, GameStateText_1, GameStateText_2 };

    if (GameStateTexts.Contains(nullptr))
    {
        UE_LOG(LogTemp, Error, TEXT("One or more GameStateText widgets are not properly bound."));
    }

    if (GameStateBorder_0) GameStateBorder_0->SetVisibility(ESlateVisibility::Collapsed);
    if (GameStateBorder_1) GameStateBorder_1->SetVisibility(ESlateVisibility::Collapsed);
    if (GameStateBorder_2) GameStateBorder_2->SetVisibility(ESlateVisibility::Collapsed);
    if (WholeGameStateBorder) WholeGameStateBorder->SetVisibility(ESlateVisibility::Collapsed);
}


void UHUD_ContentRetreiver::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{

    Super::NativeTick(MyGeometry, InDeltaTime);
    TimeSinceLastRead_1 += InDeltaTime;
    TimeSinceLastRead_2 += InDeltaTime;


    if (TimeSinceLastRead_1 >= 4.0f)
    {
        FString FilePath = FPaths::ProjectDir() + TEXT("LLM_Response/Actor_LLM_response.txt");
        FString NewResponse;

        if (FFileHelper::LoadFileToString(NewResponse, *FilePath))
        {
            UE_LOG(LogTemp, Log, TEXT("NEWResponse: %s"), *NewResponse);
            FString CleanedResponse = CleanResponse(NewResponse);

            UE_LOG(LogTemp, Log, TEXT("Cleared content: %s"), *CleanedResponse);
            if (CleanedResponse != prevResponse)
            {
                {
                    ProcessNewResponse(CleanedResponse);
                    prevResponse = CleanedResponse;
                    UpdateBorderVisibility(); 
                    PlayAnimation(FadeOutAnimation);
                }
            }
            else if (!CleanedResponse.IsEmpty() && CleanedResponse == prevResponse)
            {
                UE_LOG(LogTemp, Log, TEXT("No new response. Skipping update."));
            }
        }
        TimeSinceLastRead_1 = 0.0f;
    }
    if (TimeSinceLastRead_2 >= 4.0f)
    {
        FString FilePath = FPaths::ProjectDir() + TEXT("LLM_Response/Whole_LLM_response.txt");
        FString NewWholeResponse;

        if (FFileHelper::LoadFileToString(NewWholeResponse, *FilePath))
        {

            FString CleanedWholeResponse = CleanResponse(NewWholeResponse);

            if (CleanedWholeResponse != prevWholeResponse)
            {
                {
                    WholeGameStateText->SetText(FText::FromString(CleanedWholeResponse));
                    prevWholeResponse = CleanedWholeResponse;
                    PlayAnimation(FadeOutAnimation);
                    //UpdateBorderVisibility();
                }
            }
            else if (!CleanedWholeResponse.IsEmpty() && CleanedWholeResponse == prevWholeResponse)
            {
                UE_LOG(LogTemp, Log, TEXT("No new response. Skipping update."));
            }
        }
        TimeSinceLastRead_2 = 20.0f;
    }
}


void UHUD_ContentRetreiver::ProcessNewResponse(const FString& NewResponse)
{
    ResponseQueue.Enqueue(NewResponse);

    AsyncTask(ENamedThreads::GameThread, [this]()
        {
            UpdateGameStateText();
        });
}


void UHUD_ContentRetreiver::UpdateGameStateText()
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
void UHUD_ContentRetreiver::UpdateBorderVisibility()
{
    TArray<UTextBlock*> TextBlocks = { GameStateText_0, GameStateText_1, GameStateText_2 };
    TArray<UBorder*> Borders = { GameStateBorder_0, GameStateBorder_1, GameStateBorder_2 };

    for (int32 i = 0; i < TextBlocks.Num(); i++)
    {
        if (TextBlocks[i] && Borders[i])
        {
            const FString TextContent = TextBlocks[i]->GetText().ToString();
            if (TextContent.IsEmpty())
            {
                Borders[i]->SetVisibility(ESlateVisibility::Collapsed);
            }
            else
            {
                Borders[i]->SetVisibility(ESlateVisibility::Visible);
            }
        }
    }
}

void UHUD_ContentRetreiver::UpdateWholeGameStateBorderVisibility()
{
    if (WholeGameStateText && WholeGameStateBorder)
    {
        const FString WholeTextContent = WholeGameStateText->GetText().ToString();

        if (WholeTextContent.IsEmpty())
        {
            WholeGameStateBorder->SetVisibility(ESlateVisibility::Collapsed);
        }
        else
        {
            WholeGameStateBorder->SetVisibility(ESlateVisibility::Visible);
        }
    }
}

FString UHUD_ContentRetreiver::CleanResponse(const FString& RawResponse)
{
    
    const FString StartMarker = "Here are the details:";
    const FString EndMarker = "Write a short scene";

    
    int32 StartIndex = RawResponse.Find(StartMarker);
    if (StartIndex != INDEX_NONE)
    {
        StartIndex += StartMarker.Len(); 
    }
    else
    {
        StartIndex = 0; 
    }

    
    int32 EndIndex = RawResponse.Find(EndMarker, ESearchCase::IgnoreCase, ESearchDir::FromStart, StartIndex);
    if (EndIndex == INDEX_NONE)
    {
        EndIndex = RawResponse.Len(); 
    }

    return RawResponse.Mid(StartIndex, EndIndex - StartIndex).TrimStartAndEnd();
}

