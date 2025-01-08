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
    /**
     * # NativeTick
     *
     * ## Brief
     * Overrides the native tick function to periodically update the HUD based on the contents of a file and handle fading animations.
     *
     * ## Parameters
     * - `MyGeometry`: Geometry information of the widget.
     * - `InDeltaTime`: Time elapsed since the last frame, used for timing logic.
     *
     * ## Details
     * This function performs the following tasks every tick:
     * 1. Reads the content of `LLM_response.txt` every second:
     *    - Checks if the content has changed since the last read.
     *    - Updates the `GameStateText` widget with the new content if it differs from the previous response.
     *    - Calls `UpdateBorderVisibility()` to adjust the HUD's border visibility.
     * 2. Plays a fade-out animation if the fade timer exceeds 10 seconds.
     *
     * ## Notes
     * - The file path is hardcoded to `LLM_Response/LLM_response.txt` in the project directory.
     *
     * ## See Also
     * - UpdateBorderVisibility()
     */

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
    /**
     * # UpdateBorderVisibility
     *
     * ## Brief
     * Adjusts the visibility of the `GameStateBorder` based on the content of `GameStateText`.
     *
     * ## Details
     * - Sets the `GameStateBorder` visibility to `Collapsed` if `GameStateText` is empty.
     * - Sets the `GameStateBorder` visibility to `Visible` otherwise.
     */
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


