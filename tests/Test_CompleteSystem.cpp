// Additional Tests for projectGameMode and HttpHandler
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "projectGameMode.h"
#include "HttpHandler_Get.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

// Test GetActors Functionality
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameModeGetActorsTest, "Project.GameMode.GetActors", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGameModeGetActorsTest::RunTest(const FString& Parameters)
{
    AprojectGameMode* GameMode = NewObject<AprojectGameMode>();
    TestNotNull("GameMode should not be null", GameMode);

    UWorld* World = GEditor->GetEditorWorldContext().World();
    GameMode->SetWorld(World);

    GameMode->GetActors();
    TestTrue("TrackedObjects array should not be empty", GameMode->TrackedObjects.Num() > 0);

    return true;
}

// Test PerformTracking
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameModePerformTrackingTest, "Project.GameMode.PerformTracking", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGameModePerformTrackingTest::RunTest(const FString& Parameters)
{
    AprojectGameMode* GameMode = NewObject<AprojectGameMode>();
    TestNotNull("GameMode should not be null", GameMode);

    UWorld* World = GEditor->GetEditorWorldContext().World();
    GameMode->SetWorld(World);

    GameMode->GetActors();
    GameMode->PerformTracking();

    TestTrue("TrackedObjects should have updated positions", GameMode->TrackedObjects[0].PreviousPosition != FVector::ZeroVector);

    return true;
}

// Test GetPlayerRelativity
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameModePlayerRelativityTest, "Project.GameMode.GetPlayerRelativity", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGameModePlayerRelativityTest::RunTest(const FString& Parameters)
{
    AprojectGameMode* GameMode = NewObject<AprojectGameMode>();
    TestNotNull("GameMode should not be null", GameMode);

    UWorld* World = GEditor->GetEditorWorldContext().World();
    GameMode->SetWorld(World);

    AActor* TestActor = World->SpawnActor<AActor>();
    TestActor->SetActorLocation(FVector(100.0f, 100.0f, 0.0f));

    FString Relativity = GameMode->GetPlayerRelativity(TestActor);
    TestTrue("Relativity string should not be empty", !Relativity.IsEmpty());

    return true;
}

// Test DetermineQuadrant
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameModeDetermineQuadrantTest, "Project.GameMode.DetermineQuadrant", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGameModeDetermineQuadrantTest::RunTest(const FString& Parameters)
{
    AprojectGameMode* GameMode = NewObject<AprojectGameMode>();
    TestNotNull("GameMode should not be null", GameMode);

    FVector TestVector(100.0f, 100.0f, 0.0f);
    FString Quadrant = GameMode->DetermineQuadrant(TestVector);
    TestEqual("Quadrant should be North-East", Quadrant, TEXT("North-East"));

    TestVector = FVector(-100.0f, 100.0f, 0.0f);
    Quadrant = GameMode->DetermineQuadrant(TestVector);
    TestEqual("Quadrant should be North-West", Quadrant, TEXT("North-West"));

    return true;
}

// Test OnResponseReceived
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHttpHandlerResponseTest, "Project.HttpHandler.OnResponseReceived", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHttpHandlerResponseTest::RunTest(const FString& Parameters)
{
    UHttpHandler_Get* HttpHandler = NewObject<UHttpHandler_Get>();
    TestNotNull("HttpHandler should not be null", HttpHandler);

    FHttpResponsePtr MockResponse;
    HttpHandler->OnResponseReceived(nullptr, MockResponse, false);
    TestTrue("GameStateText should display failure message", HttpHandler->GameStateText->GetText().ToString().Contains(TEXT("Failed")));

    return true;
}
