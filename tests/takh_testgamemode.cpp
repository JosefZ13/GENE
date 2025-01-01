#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "../projectGameMode.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "GameFramework/GameModeBase.h"
#include "Tests/AutomationEditorCommon.h"


/*VIKTIGT : 1_Lägg UWorld* CurrentWorld = nullptr; 

void SetWorld(UWorld* InWorld); 
2_Lägg den här funktion utanför alla andra funktioner.  på projectgamemode.cpp : 
void AprojectGameMode::SetWorld(UWorld* InWorld) // Definiera SetWorld funktionen här
{
	CurrentWorld = InWorld;
}
3_lägg  PrivateIncludePathModuleNames.AddRange(new string[] { "SessionServices", "AutomationWorker" }); 
    }      till project.Build.cs */



// BeginPlayTest: Verifies the behavior of the BeginPlay function in GameMode.

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBeginPlayTest, "project.GameMode.BeginPlayTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FBeginPlayTest::RunTest(const FString& Parameters)
{
    // Create a test world.
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    if (!World)
    {
        AddError("Failed to create a test world.");
        return false;
    }

    // Spawn an instance of AprojectGameMode in the world.
    AprojectGameMode* GameMode = World->SpawnActor<AprojectGameMode>();
    if (!GameMode)
    {
        AddError("Failed to spawn GameMode.");
        return false;
    }

    // Define file paths for test data.
    FString FilePath1 = FPaths::ProjectDir() + TEXT("LLM_Response/Actor_LLM_response.txt");
    FString FilePath2 = FPaths::ProjectDir() + TEXT("LLM_Response/Whole_LLM_response.txt");

    // Check if the files exist.
    TestTrue("Actor_LLM_response.txt file exists after BeginPlay", FPaths::FileExists(FilePath1));
    TestTrue("Whole_LLM_response.txt file exists after BeginPlay", FPaths::FileExists(FilePath2));

    // Check if the files are empty.
    FString FileContent;
    TestTrue("Actor_LLM_response.txt content is cleared", FFileHelper::LoadFileToString(FileContent, *FilePath1) && FileContent.IsEmpty());
    TestTrue("Whole_LLM_response.txt content is cleared", FFileHelper::LoadFileToString(FileContent, *FilePath2) && FileContent.IsEmpty());

    // Check if PrimaryActorTick is set correctly.
    TestTrue("PrimaryActorTick.bCanEverTick is set to true", GameMode->PrimaryActorTick.bCanEverTick);
    AddInfo("Ensure critical functions (WholeWorldJson, GetActors, TickForGetWorld) are executed...");

    return true;
}

// TestGameMode_SerializeVector: Verifies vector serialization in GameMode.

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestGameMode_SerializeVector, "project.GameMode.SerializeVector", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TestGameMode_SerializeVector::RunTest(const FString& Parameters)
{
    // Create an instance of GameMode.
    AprojectGameMode* GameMode = NewObject<AprojectGameMode>();

    // Create a vector and serialize it to JSON.
    FVector Vector(1.0f, 2.0f, 3.0f);
    TSharedPtr<FJsonObject> JsonObject = GameMode->SerializeVector(Vector);

    // Check if the values in JSON match the vector.
    TestEqual(TEXT("X value should be correct"), JsonObject->GetNumberField("x"), 1.0);
    TestEqual(TEXT("Y value should be correct"), JsonObject->GetNumberField("y"), 2.0);
    TestEqual(TEXT("Z value should be correct"), JsonObject->GetNumberField("z"), 3.0);

    return true;
}

// TestGameMode_GetRelativePosition_InFront: Verifies relative position in front of the player.

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestGameMode_GetRelativePosition_InFront, "project.GameMode.GetRelativePosition.InFront", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TestGameMode_GetRelativePosition_InFront::RunTest(const FString& Parameters)
{
    // Create GameMode and a mock world.
    AprojectGameMode* GameMode = NewObject<AprojectGameMode>();
    UWorld* MockWorld = NewObject<UWorld>();
    GameMode->SetWorld(MockWorld);

    // Check the relative position.
    FString Result = GameMode->GetRelativePosition(1.0f, 0.0f, 0.0f);
    TestEqual(TEXT("Actor should be directly in front"), Result, "Actor is directly in front of the player.");

    return true;
}

// TestGameMode_GetRelativePosition_BehindRightAbove: Verifies relative position behind, right, and above the player.

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestGameMode_GetRelativePosition_BehindRightAbove, "project.GameMode.GetRelativePosition.BehindRightAbove", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TestGameMode_GetRelativePosition_BehindRightAbove::RunTest(const FString& Parameters)
{
    AprojectGameMode* GameMode = NewObject<AprojectGameMode>();
    UWorld* MockWorld = NewObject<UWorld>();
    GameMode->SetWorld(MockWorld);

    FString Result = GameMode->GetRelativePosition(-0.5f, 0.5f, 0.5f);
    TestEqual(TEXT("Actor should be behind-right-above"), Result, "Actor is behind-right-above the player.");

    return true;
}

// TestGameMode_GetRelativePosition_Below: Verifies relative position below the player.

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestGameMode_GetRelativePosition_Below, "project.GameMode.GetRelativePosition.Below", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TestGameMode_GetRelativePosition_Below::RunTest(const FString& Parameters)
{
    AprojectGameMode* GameMode = NewObject<AprojectGameMode>();
    UWorld* MockWorld = NewObject<UWorld>();
    GameMode->SetWorld(MockWorld);

    FString Result = GameMode->GetRelativePosition(0.0f, 0.0f, -1.0f);
    TestEqual(TEXT("Actor should be directly below"), Result, "Actor is directly below the player.");

    return true;
}

// TestGameMode_QuestionPrompt_ActorMovement: Verifies the response for the ActorMovement question.

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestGameMode_QuestionPrompt_ActorMovement, "project.GameMode.QuestionPrompt.ActorMovement", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TestGameMode_QuestionPrompt_ActorMovement::RunTest(const FString& Parameters)
{
    AprojectGameMode* GameMode = NewObject<AprojectGameMode>();
    FString Result = GameMode->question_prompt("ActorMovement");
    TestEqual(TEXT("ActorMovement prompt should match"), Result, "Tell the ObjectName, its distance and the relative position that is mentioned in the provided data. Stop when you have said it. Do not answer with json like format. Answer in regular text. \n");

    return true;
}

// TestGameMode_QuestionPrompt_StoryWholeJson: Verifies the response for the StoryWholeJson question.

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestGameMode_QuestionPrompt_StoryWholeJson, "project.GameMode.QuestionPrompt.StoryWholeJson", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TestGameMode_QuestionPrompt_StoryWholeJson::RunTest(const FString& Parameters)
{
    AprojectGameMode* GameMode = NewObject<AprojectGameMode>();
    FString Result = GameMode->question_prompt("StoryWholeJson");
    TestEqual(TEXT("StoryWholeJson prompt should match"), Result, "Write a short story using the following details without explaining your process.\n");

    return true;
}

// TestGameMode_QuestionPrompt_Story: Verifies the response for the Story question.

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestGameMode_QuestionPrompt_Story, "project.GameMode.QuestionPrompt.Story", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TestGameMode_QuestionPrompt_Story::RunTest(const FString& Parameters)
{
    AprojectGameMode* GameMode = NewObject<AprojectGameMode>();
    FString Result = GameMode->question_prompt("Story");
    TestEqual(TEXT("Story prompt should match"), Result, "Generate a coherent and engaging story based on the given parameters without explaining the steps.");

    return true;
}
