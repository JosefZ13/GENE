#include "CoreMinimal.h" // Inkluderar kärnbibliotek för Unreal Engine.
#include "Misc/AutomationTest.h" // Inkluderar funktionalitet för att skriva automatiserade tester.
#include "../HUD_ContentRetreiver.h" // Inkluderar headerfilen för UHUD_ContentRetriever-klassen.
#include "Components/TextBlock.h" // Inkluderar komponent för textblock.
#include "Components/Border.h" // Inkluderar komponent för border (ramar).

// Testklass för att verifiera ProcessNewResponse-funktionaliteten i UHUD_ContentRetriever.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestHUDContentRetriever_ProcessNewResponse, "project.HUDContentRetriever.ProcessNewResponse", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TestHUDContentRetriever_ProcessNewResponse::RunTest(const FString& Parameters) {
    // Skapar en instans av UHUD_ContentRetriever.
    UHUD_ContentRetriever* HUD = NewObject<UHUD_ContentRetriever>();

    // Skickar en ny respons till HUD för bearbetning.
    FString NewResponse = "Testrespons";
    HUD->ProcessNewResponse(NewResponse);

    // Kontrollerar om responsen korrekt har lagts till i kön.
    FString RetrievedResponse;
    bool Success = HUD->ResponseQueue.Dequeue(RetrievedResponse);
    TestTrue(TEXT("Response should be added to the queue"), Success && RetrievedResponse == NewResponse);

    return true; // Returnerar att testet lyckades.
}

// Testklass för att verifiera UpdateGameStateText-funktionaliteten i UHUD_ContentRetriever.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestHUDContentRetriever_UpdateGameStateText, "project.HUDContentRetriever.UpdateGameStateText", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TestHUDContentRetriever_UpdateGameStateText::RunTest(const FString& Parameters) {
    // Skapar en instans av UHUD_ContentRetriever.
    UHUD_ContentRetriever* HUD = NewObject<UHUD_ContentRetriever>();

    // Skapar två textblock för att representera spelstatustexter.
    HUD->GameStateTexts = { NewObject<UTextBlock>(), NewObject<UTextBlock>() };

    // Lägger till två svar i kön.
    FString Response1 = "Response 1";
    FString Response2 = "Response 2";
    HUD->ResponseQueue.Enqueue(Response1);
    HUD->ResponseQueue.Enqueue(Response2);

    // Uppdaterar textblock med svar från kön.
    HUD->UpdateGameStateText();

    // Verifierar att textblocken uppdaterades korrekt.
    TestEqual(TEXT("First text block should be updated with the latest response"), HUD->GameStateTexts[0]->GetText().ToString(), Response2);
    TestEqual(TEXT("Second text block should be updated with the previous response"), HUD->GameStateTexts[1]->GetText().ToString(), Response1);

    // Uppdaterar igen för att verifiera att textblocken fortsätter att uppdateras.
    HUD->UpdateGameStateText();
    TestEqual(TEXT("First text block should update with the previous response"), HUD->GameStateTexts[0]->GetText().ToString(), Response1);
    TestTrue(TEXT("Queue should be empty after processing all responses"), HUD->ResponseQueue.IsEmpty());

    return true; // Returnerar att testet lyckades.
}

// Testklass för att verifiera beteendet när kön är tom vid UpdateGameStateText-anrop.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestHUDContentRetriever_UpdateGameStateText_EmptyQueue, "project.HUDContentRetriever.UpdateGameStateText.EmptyQueue", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TestHUDContentRetriever_UpdateGameStateText_EmptyQueue::RunTest(const FString& Parameters) {
    // Skapar en instans av UHUD_ContentRetriever.
    UHUD_ContentRetriever* HUD = NewObject<UHUD_ContentRetriever>();

    // Skapar två textblock för att representera spelstatustexter.
    HUD->GameStateTexts = { NewObject<UTextBlock>(), NewObject<UTextBlock>() };

    // Uppdaterar textblock utan att det finns några svar i kön.
    HUD->UpdateGameStateText();

    // Verifierar att textblocken inte ändrades.
    TestEqual(TEXT("First text block should remain unchanged"), HUD->GameStateTexts[0]->GetText().ToString(), "");
    TestEqual(TEXT("Second text block should remain unchanged"), HUD->GameStateTexts[1]->GetText().ToString(), "");

    return true; // Returnerar att testet lyckades.
}

// Testklass för att verifiera UpdateBorderVisibility-funktionaliteten i UHUD_ContentRetriever.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestHUDContentRetriever_UpdateBorderVisibility, "project.HUDContentRetriever.UpdateBorderVisibility", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TestHUDContentRetriever_UpdateBorderVisibility::RunTest(const FString& Parameters) {
    // Skapar en instans av UHUD_ContentRetriever.
    UHUD_ContentRetriever* HUD = NewObject<UHUD_ContentRetriever>();

    // Skapar två textblock och tilldelar text till ett av dem.
    HUD->GameStateTexts = { NewObject<UTextBlock>(), NewObject<UTextBlock>() };
    HUD->GameStateTexts[0]->SetText(FText::FromString("Non-empty"));
    HUD->GameStateTexts[1]->SetText(FText::FromString(""));

    // Skapar två mock-borders och associerar dem med textblocken.
    UBorder* MockBorder0 = NewObject<UBorder>();
    UBorder* MockBorder1 = NewObject<UBorder>();
    HUD->GameStateBorder_0 = MockBorder0;
    HUD->GameStateBorder_1 = MockBorder1;

    // Uppdaterar border-synlighet baserat på textblockens innehåll.
    HUD->UpdateBorderVisibility();

    // Verifierar att borders har korrekt synlighet.
    TestEqual(TEXT("Border for first text block should be visible"), HUD->GameStateBorder_0->GetVisibility(), ESlateVisibility::Visible);
    TestEqual(TEXT("Border for second text block should be collapsed"), HUD->GameStateBorder_1->GetVisibility(), ESlateVisibility::Collapsed);

    return true; // Returnerar att testet lyckades.
}
