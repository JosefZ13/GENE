// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "projectGameMode.h"
#include "MockWorld.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

/**
 * Test för att verifiera BeginPlay och interaktion med filer.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProjectGameModeBeginPlayTest, "ProjectGameMode.Integration.BeginPlay", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FProjectGameModeBeginPlayTest::RunTest(const FString& Parameters)
{
    AprojectGameMode* GameMode = NewObject<AprojectGameMode>();
    if (!GameMode)
    {
        AddError("Failed to create AprojectGameMode instance.");
        return false;
    }

    GameMode->SetWorld(NewObject<UMockWorld>());
    GameMode->BeginPlay();

    FString FilePath = FPaths::ProjectDir() + TEXT("LLM_Response/LLM_response.txt");
    FString FileContent;
    if (FFileHelper::LoadFileToString(FileContent, *FilePath))
    {
        TestTrue("File content is cleared after BeginPlay", FileContent.IsEmpty());
    }
    else
    {
        AddError("Failed to load or clear LLM_response.txt.");
        return false;
    }

    return true;
}

/**
 * Test för att verifiera TrackActors och JSON-hantering.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProjectGameModeTrackingTest, "ProjectGameMode.Integration.TrackActors", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FProjectGameModeTrackingTest::RunTest(const FString& Parameters)
{
    AprojectGameMode* GameMode = NewObject<AprojectGameMode>();
    if (!GameMode)
    {
        AddError("Failed to create AprojectGameMode instance.");
        return false;
    }

    GameMode->SetWorld(NewObject<UMockWorld>());
    GameMode->GetActors();

    TestTrue("Actors are being tracked", GameMode->TrackedObjects.Num() > 0);

    GameMode->PerformTracking();
    TestNotEqual("Generated JSON should not be empty after tracking", GameMode->newJSON, TEXT(""));

    return true;
}
