#include "Misc/AutomationTest.h"
#include "../HUD_ContentRetreiver.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHUDContentRetriever_NativeConstructTest, "Project.HUD.NativeConstructTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHUDContentRetriever_NativeConstructTest::RunTest(const FString& Parameters)
{
    AddInfo("Creating HUD_ContentRetreiver widget...");
    UHUD_ContentRetreiver* HUDWidget = NewObject<UHUD_ContentRetreiver>();
    HUDWidget->AddToRoot();

    AddInfo("Mocking GameStateBorder...");
    UBorder* MockBorder = NewObject<UBorder>();
    HUDWidget->GameStateBorder = MockBorder;

    AddInfo("Setting MockBorder visibility to Visible...");
    MockBorder->SetVisibility(ESlateVisibility::Visible);

    AddInfo("Validating pre-state: MockBorder visibility should be Visible...");
    TestTrue("MockBorder should initially be visible", MockBorder->GetVisibility() == ESlateVisibility::Visible);

    AddInfo("Calling NativeConstruct...");
    HUDWidget->NativeConstruct();

    AddInfo("Validating that GameStateBorder is set to collapsed...");
    TestTrue("GameStateBorder should start as collapsed", MockBorder->GetVisibility() == ESlateVisibility::Collapsed);

    HUDWidget->RemoveFromRoot();
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHUDContentRetriever_NativeTickTest, "Project.HUD.NativeTickTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHUDContentRetriever_NativeTickTest::RunTest(const FString& Parameters)
{
    AddInfo("Creating HUD_ContentRetreiver widget...");
    UHUD_ContentRetreiver* HUDWidget = NewObject<UHUD_ContentRetreiver>();
    HUDWidget->AddToRoot();

    AddInfo("Mocking components (GameStateText, GameStateBorder)...");
    UTextBlock* MockTextBlock = NewObject<UTextBlock>();
    UBorder* MockBorder = NewObject<UBorder>();
    HUDWidget->GameStateText = MockTextBlock;
    HUDWidget->GameStateBorder = MockBorder;

    TestNotNull("GameStateText should not be null", HUDWidget->GameStateText);

    AddInfo("Resetting prevResponse...");
    HUDWidget->prevResponse = TEXT("");

    AddInfo("Setting up mock file...");
    FString DirectoryPath = FPaths::ProjectDir() + TEXT("LLM_Response");
    FString MockFilePath = DirectoryPath + TEXT("/LLM_response.txt");
    FString MockResponse = TEXT("First Response");

    // Ensure the directory exists
    if (!IFileManager::Get().DirectoryExists(*DirectoryPath))
    {
        IFileManager::Get().MakeDirectory(*DirectoryPath);
    }

    // Write the mock file
    bool bFileWritten = FFileHelper::SaveStringToFile(MockResponse, *MockFilePath);
    if (!bFileWritten)
    {
        AddError(FString::Printf(TEXT("Failed to write mock file: %s"), *MockFilePath));
        return false; // Stop the test if the file cannot be written
    }

    AddInfo(FString::Printf(TEXT("Mock file written to: %s"), *MockFilePath));

    // Attempt to read the mock file
    FString FileContent;
    bool bFileRead = FFileHelper::LoadFileToString(FileContent, *MockFilePath);
    if (!bFileRead)
    {
        AddError(FString::Printf(TEXT("Failed to read mock file: %s"), *MockFilePath));
        return false; // Stop the test if the file cannot be read
    }

    AddInfo(FString::Printf(TEXT("File content read: %s"), *FileContent));
    TestEqual("Mock file content should match the expected response", FileContent, MockResponse);

    AddInfo("Simulating 1.1 seconds in one tick...");
    HUDWidget->NativeTick(FGeometry(), 1.1f);

    AddInfo(FString::Printf(TEXT("Validating GameStateText. Expected: '%s', Actual: '%s'"), *MockResponse, *MockTextBlock->GetText().ToString()));
    TestEqual("GameStateText should update with the response from file", MockTextBlock->GetText().ToString(), MockResponse);

    HUDWidget->RemoveFromRoot();
    IFileManager::Get().Delete(*MockFilePath); // Clean up
    return true;
}




// Test for UpdateBorderVisibility
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHUDContentRetriever_UpdateBorderVisibilityTest, "Project.HUD.UpdateBorderVisibilityTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHUDContentRetriever_UpdateBorderVisibilityTest::RunTest(const FString& Parameters)
{
    AddInfo("Creating HUD_ContentRetreiver widget...");
    UHUD_ContentRetreiver* HUDWidget = NewObject<UHUD_ContentRetreiver>();
    HUDWidget->AddToRoot();

    AddInfo("Mocking components (GameStateText, GameStateBorder)...");
    UTextBlock* MockTextBlock = NewObject<UTextBlock>();
    UBorder* MockBorder = NewObject<UBorder>();
    HUDWidget->GameStateText = MockTextBlock;
    HUDWidget->GameStateBorder = MockBorder;

    AddInfo("Testing empty text scenario...");
    MockTextBlock->SetText(FText::FromString(TEXT("")));
    HUDWidget->UpdateBorderVisibility();
    TestTrue("GameStateBorder should collapse when GameStateText is empty", MockBorder->GetVisibility() == ESlateVisibility::Collapsed);

    AddInfo("Testing non-empty text scenario...");
    MockTextBlock->SetText(FText::FromString(TEXT("Some Text")));
    HUDWidget->UpdateBorderVisibility();
    TestTrue("GameStateBorder should be visible when GameStateText is non-empty", MockBorder->GetVisibility() == ESlateVisibility::Visible);

    HUDWidget->RemoveFromRoot();
    return true;
}
