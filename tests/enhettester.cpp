#include "Misc/AutomationTest.h"
#include "HttpHandler_Get.h"
#include "HUD_ContentRetreiver.h"
#include "projectGameMode.h"
#include "Engine/World.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "HAL/PlatformProcess.h"

/*HttpHandler_Get
Verifierar att systemet korrekt kan hämta och bearbeta data från en HTTP-källa.
- Tester: 
  - Kontroll av korrekt borttagning av onödiga tecken (som newline) i svaren.
  - Säkerställer att dataformatet är korrekt behandlat.
HUD_ContentRetriever Säkerställer att spelets HUD (Heads-Up Display) visar information korrekt baserat på inkommande data.
- Tester
  - Kontroll av uppdatering av textfält med rätt information.
  - Kontroll av att gränssnittets element, som synlighet och gränser, uppdateras korrekt.
projectGameMode Testar spellogik som styr interaktionen mellan spelaren och omgivningen.
- Tester:
  - Kontroll av att vektorer serialiseras korrekt till JSON-format.
  - Validerar funktioner för att beräkna en aktörs relativa position i förhållande till spelaren.
  - Kontroll av anpassade frågeresponslogiker (t.ex. generera berättelser eller beskriva positioner baserat på data)
  Testet returnerar true och markerar testet som lyckas eller false och markerar testet som Misslyckat.*/

// Hjälpfunktion för att vänta med villkor
bool WaitForCondition(UWorld* World, TFunction<bool()> Condition, float Timeout = 5.0f) {
    float StartTime = World->GetTimeSeconds();
    while (World->GetTimeSeconds() - StartTime < Timeout) {
        if (Condition()) return true;
        FPlatformProcess::Sleep(0.1f);
        World->Tick(LEVELTICK_All, 0.1f);
    }
    return false;
}


// Enhetstester för HttpHandler_Get.h

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestTrimResponse_WithNewlines, "MyProject.HttpHandler.TrimResponse.WithNewlines", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TestTrimResponse_WithNewlines::RunTest(const FString& Parameters) {
    UHttpHandler_Get* HttpHandler = NewObject<UHttpHandler_Get>();

    FString Input = "\n\nTeststräng\n\n";
    FString Expected = "Teststräng";
    FString Result = HttpHandler->TrimResponse(Input);

    TestEqual(TEXT("Should trim leading and trailing newlines"), Result, Expected);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestTrimResponse_NoNewlines, "MyProject.HttpHandler.TrimResponse.NoNewlines", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TestTrimResponse_NoNewlines::RunTest(const FString& Parameters) {
    UHttpHandler_Get* HttpHandler = NewObject<UHttpHandler_Get>();

    FString Input = "Teststräng";
    FString Expected = "Teststräng";
    FString Result = HttpHandler->TrimResponse(Input);

    TestEqual(TEXT("Should return the string as-is when no newlines"), Result, Expected);
    return true;
}


// Enhetstester för HUD_ContentRetriever.h


IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestProcessNewResponse, "MyProject.HUDContentRetriever.ProcessNewResponse", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TestProcessNewResponse::RunTest(const FString& Parameters) {
    UHUD_ContentRetriever* HUD = NewObject<UHUD_ContentRetriever>();
    FString NewResponse = "{\"event\":\"ActorMovement\",\"data\":{\"actorName\":\"Cube\",\"distance\":10.0,\"relativePosition\":\"In front\"}}";

    HUD->ProcessNewResponse(NewResponse);
    FString RetrievedResponse;
    bool Success = HUD->ResponseQueue.Dequeue(RetrievedResponse);

    TestTrue(TEXT("Response should be added to the queue"), Success && RetrievedResponse == NewResponse);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestUpdateGameStateText_WithTwoResponses, "MyProject.HUDContentRetriever.UpdateGameStateText.WithTwoResponses", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TestUpdateGameStateText_WithTwoResponses::RunTest(const FString& Parameters) {
    UHUD_ContentRetriever* HUD = NewObject<UHUD_ContentRetriever>();
    HUD->GameStateTexts = { NewObject<UTextBlock>(), NewObject<UTextBlock>() };

    FString Response1 = "Response 1";
    FString Response2 = "Response 2";
    HUD->ResponseQueue.Enqueue(Response1);
    HUD->ResponseQueue.Enqueue(Response2);

    HUD->UpdateGameStateText();
    TestEqual(TEXT("First text block should be updated with the latest response"), HUD->GameStateTexts[0]->GetText().ToString(), Response2);
    TestEqual(TEXT("Second text block should be updated with the previous response"), HUD->GameStateTexts[1]->GetText().ToString(), Response1);

    HUD->UpdateGameStateText();
    TestEqual(TEXT("First text block should update with the previous response"), HUD->GameStateTexts[0]->GetText().ToString(), Response1);
    TestTrue(TEXT("Queue should be empty after processing all responses"), HUD->ResponseQueue.IsEmpty());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestUpdateBorderVisibility, "MyProject.HUDContentRetriever.UpdateBorderVisibility", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TestUpdateBorderVisibility::RunTest(const FString& Parameters) {
    UHUD_ContentRetriever* HUD = NewObject<UHUD_ContentRetriever>();
    HUD->GameStateTexts = { NewObject<UTextBlock>(), NewObject<UTextBlock>() };
    HUD->GameStateTexts[0]->SetText(FText::FromString("Non-empty"));
    HUD->GameStateTexts[1]->SetText(FText::FromString(""));

    HUD->UpdateBorderVisibility();
    TestEqual(TEXT("Border for first text block should be visible"), HUD->GameStateBorder_0->GetVisibility(), ESlateVisibility::Visible);
    TestEqual(TEXT("Border for second text block should be collapsed"), HUD->GameStateBorder_1->GetVisibility(), ESlateVisibility::Collapsed);

    // Enhetstester för gamemode_playerrelativity.h


    IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestSerializeVector, "MyProject.GameMode.SerializeVector", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
        bool TestSerializeVector::RunTest(const FString & Parameters) {
        AprojectGameMode* GameMode = NewObject<AprojectGameMode>();
        FVector Vector(1.0f, 2.0f, 3.0f);
        TSharedPtr<FJsonObject> JsonObject = GameMode->SerializeVector(Vector);

        TestEqual(TEXT("X value should be correct"), JsonObject->GetNumberField("x"), 1.0f);
        TestEqual(TEXT("Y value should be correct"), JsonObject->GetNumberField("y"), 2.0f);
        TestEqual(TEXT("Z value should be correct"), JsonObject->GetNumberField("z"), 3.0f);
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestGetRelativePosition_InFront, "MyProject.GameMode.GetRelativePosition.InFront", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
        bool TestGetRelativePosition_InFront::RunTest(const FString & Parameters) {
        AprojectGameMode* GameMode = NewObject<AprojectGameMode>();

        FString Result = GameMode->GetRelativePosition(1.0f, 0.0f, 0.0f);
        TestEqual(TEXT("Actor should be directly in front"), Result, "Actor is directly in front of the player.");

        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestGetRelativePosition_BehindRightAbove, "MyProject.GameMode.GetRelativePosition.BehindRightAbove", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
        bool TestGetRelativePosition_BehindRightAbove::RunTest(const FString & Parameters) {
        AprojectGameMode* GameMode = NewObject<AprojectGameMode>();

        FString Result = GameMode->GetRelativePosition(-0.5f, 0.5f, 0.5f);
        TestEqual(TEXT("Actor should be behind-right-above"), Result, "Actor is behind-right-above the player.");

        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestGetRelativePosition_Below, "MyProject.GameMode.GetRelativePosition.Below", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
        bool TestGetRelativePosition_Below::RunTest(const FString & Parameters) {
        AprojectGameMode* GameMode = NewObject<AprojectGameMode>();

        FString Result = GameMode->GetRelativePosition(0.0f, 0.0f, -1.0f);
        TestEqual(TEXT("Actor should be directly below"), Result, "Actor is directly below the player.");

        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestGetRelativePosition_Above, "MyProject.GameMode.GetRelativePosition.Above", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
        bool TestGetRelativePosition_Above::RunTest(const FString & Parameters) {
        AprojectGameMode* GameMode = NewObject<AprojectGameMode>();
        FString Result = GameMode->GetRelativePosition(0.0f, 0.0f, 1.0f);
        TestEqual(TEXT("Actor should be directly above"), Result, "Actor is directly above the player.");
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestGetRelativePosition_Left, "MyProject.GameMode.GetRelativePosition.Left", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
        bool TestGetRelativePosition_Left::RunTest(const FString & Parameters) {
        AprojectGameMode* GameMode = NewObject<AprojectGameMode>();
        FString Result = GameMode->GetRelativePosition(0.0f, -1.0f, 0.0f);
        TestEqual(TEXT("Actor should be directly to the left"), Result, "Actor is directly to the left of the player.");
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestQuestionPrompt_ActorMovement, "MyProject.GameMode.QuestionPrompt.ActorMovement", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
        bool TestQuestionPrompt_ActorMovement::RunTest(const FString & Parameters) {
        AprojectGameMode* GameMode = NewObject<AprojectGameMode>();

        FString Result = GameMode->question_prompt("ActorMovement");
        TestEqual(TEXT("ActorMovement prompt should match"), Result, "Tell the ObjectName, its distance and the relative position that is mentioned in the provided data. Stop when you have said it. Do not answer with json like format. Answer in regular text. \n");

        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestQuestionPrompt_StoryWholeJson, "MyProject.GameMode.QuestionPrompt.StoryWholeJson", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
        bool TestQuestionPrompt_StoryWholeJson::RunTest(const FString & Parameters) {
        AprojectGameMode* GameMode = NewObject<AprojectGameMode>();

        FString Result = GameMode->question_prompt("StoryWholeJson");
        TestEqual(TEXT("StoryWholeJson prompt should match"), Result, "Write a short story using the following details without explaining your process.\n");

        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestQuestionPrompt_Story, "MyProject.GameMode.QuestionPrompt.Story", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
        bool TestQuestionPrompt_Story::RunTest(const FString & Parameters) {
        AprojectGameMode* GameMode = NewObject<AprojectGameMode>();

        FString Result = GameMode->question_prompt("Story");
        TestEqual(TEXT("Story prompt should match"), Result, "Generate a story based on the provided data. Stop when you have said it. Do not answer with json like format. Answer in regular text.\n");

        return true;
    }
