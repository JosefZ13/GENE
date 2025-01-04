// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_ContentRetreiver.h"
#include "Dom/JsonObject.h"
#include "Components/TextBlock.h"
#include "Containers/Queue.h"
#include "Components/Border.h"
#include "Serialization/JsonReader.h"

float TimeSinceLastRead_1 = 0.0f;
float TimetoFade = 0.0f;

void UHUD_ContentRetreiver::NativeConstruct()
{
    Super::NativeConstruct();

    if (GameStateBorder) GameStateBorder->SetVisibility(ESlateVisibility::Collapsed);

}


void UHUD_ContentRetreiver::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{

    Super::NativeTick(MyGeometry, InDeltaTime);
    TimeSinceLastRead_1 += InDeltaTime;


    if (TimeSinceLastRead_1 >= 1.0f)
    {
        FString FilePath = FPaths::ProjectDir() + TEXT("LLM_Response/LLM_response.txt");
        FString NewResponse;

        if (FFileHelper::LoadFileToString(NewResponse, *FilePath))
        {
            if (NewResponse != prevResponse)
            {
                //uppdatera HUD 
                GameStateText->SetText(FText::FromString(NewResponse));
                UpdateBorderVisibility();
            }
        }
        TimeSinceLastRead_1 = 0.0f;
    }

    if (TimetoFade >= 10.0f)
    {
        PlayAnimation(FadeOutAnimation);
    }
}



void UHUD_ContentRetreiver::UpdateBorderVisibility()
{
    FString TextContent = GameStateText->GetText().ToString();
    if (TextContent.IsEmpty())
    {
        GameStateBorder->SetVisibility(ESlateVisibility::Collapsed);
    }
    else
    {
        GameStateBorder->SetVisibility(ESlateVisibility::Visible);
    }
}


