#include "CoreMinimal.h" // Inkluderar kärnbibliotek för Unreal Engine.
#include "Misc/AutomationTest.h" // För automatiseringstestfunktioner.
#include "../projectGameMode.h" // Header för den specifika GameMode-klassen.
#include "Misc/Paths.h" // För att hantera fil- och katalogvägar.
#include "Misc/FileHelper.h" // För att hantera filer (läsa och skriva).
#include "GameFramework/GameModeBase.h" // Bas GameMode-klass i Unreal Engine.
#include "Tests/AutomationEditorCommon.h" // För vanliga funktioner inom automationstester.

/**
 * Testklass för att verifiera beteendet hos BeginPlay-funktionen i GameMode.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBeginPlayTest, "project.GameMode.BeginPlayTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FBeginPlayTest::RunTest(const FString& Parameters)
{
    // Skapar en testvärld.
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    if (!World)
    {
        AddError("Failed to create a test world."); // Lägger till ett fel om världen inte kunde skapas.
        return false;
    }

    // Skapar och spawna en instans av AprojectGameMode i världen.
    AprojectGameMode* GameMode = World->SpawnActor<AprojectGameMode>();
    if (!GameMode)
    {
        AddError("Failed to spawn GameMode."); // Lägger till ett fel om GameMode inte kunde skapas.
        return false;
    }

    // Definierar filvägar för testdata.
    FString FilePath1 = FPaths::ProjectDir() + TEXT("LLM_Response/Actor_LLM_response.txt");
    FString FilePath2 = FPaths::ProjectDir() + TEXT("LLM_Response/Whole_LLM_response.txt");

    // Kontrollerar om filerna existerar.
    TestTrue("Actor_LLM_response.txt file exists after BeginPlay", FPaths::FileExists(FilePath1));
    TestTrue("Whole_LLM_response.txt file exists after BeginPlay", FPaths::FileExists(FilePath2));

    // Kontrollerar om filerna är tomma.
    FString FileContent;
    TestTrue("Actor_LLM_response.txt content is cleared", FFileHelper::LoadFileToString(FileContent, *FilePath1) && FileContent.IsEmpty());
    TestTrue("Whole_LLM_response.txt content is cleared", FFileHelper::LoadFileToString(FileContent, *FilePath2) && FileContent.IsEmpty());

    // Kontrollerar om PrimaryActorTick är korrekt inställd.
    TestTrue("PrimaryActorTick.bCanEverTick is set to true", GameMode->PrimaryActorTick.bCanEverTick);
    AddInfo("Ensure critical functions (WholeWorldJson, GetActors, TickForGetWorld) are executed...");

    return true; // Returnerar att testet lyckades.
}

/**
 * Testklass för att verifiera vektorserialisering i GameMode.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestGameMode_SerializeVector, "project.GameMode.SerializeVector", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TestGameMode_SerializeVector::RunTest(const FString& Parameters)
{
    // Skapar en instans av GameMode.
    AprojectGameMode* GameMode = NewObject<AprojectGameMode>();
    
    // Skapar en vektor och serialiserar den till JSON.
    FVector Vector(1.0f, 2.0f, 3.0f);
    TSharedPtr<FJsonObject> JsonObject = GameMode->SerializeVector(Vector);

    // Kontrollerar att värdena i JSON stämmer överens med vektorn.
    TestEqual(TEXT("X value should be correct"), JsonObject->GetNumberField("x"), 1.0f);
    TestEqual(TEXT("Y value should be correct"), JsonObject->GetNumberField("y"), 2.0f);
    TestEqual(TEXT("Z value should be correct"), JsonObject->GetNumberField("z"), 3.0f);

    return true; // Testet lyckades.
}

// Fortsätt på samma sätt för övriga tester med detaljerade kommentarer:

/**
 * Testklass för att verifiera relativ position framför spelaren.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestGameMode_GetRelativePosition_InFront, "project.GameMode.GetRelativePosition.InFront", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TestGameMode_GetRelativePosition_InFront::RunTest(const FString& Parameters)
{
    // Skapar GameMode och en mockvärld.
    AprojectGameMode* GameMode = NewObject<AprojectGameMode>();
    UWorld* MockWorld = NewObject<UWorld>();
    GameMode->SetWorld(MockWorld);

    // Kontrollerar den relativa positionen.
    FString Result = GameMode->GetRelativePosition(1.0f, 0.0f, 0.0f);
    TestEqual(TEXT("Actor should be directly in front"), Result, "Actor is directly in front of the player.");

    return true; // Testet lyckades.
}

/**
 * Testklass för att verifiera relativ position bakom, höger och ovanför spelaren.
 */
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

/**
 * Testklass för att verifiera relativ position under spelaren.
 */
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

/**
 * Testklass för att verifiera svar för ActorMovement-frågan.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestGameMode_QuestionPrompt_ActorMovement, "project.GameMode.QuestionPrompt.ActorMovement", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TestGameMode_QuestionPrompt_ActorMovement::RunTest(const FString& Parameters)
{
    AprojectGameMode* GameMode = NewObject<AprojectGameMode>();
    FString Result = GameMode->question_prompt("ActorMovement");
    TestEqual(TEXT("ActorMovement prompt should match"), Result, "Tell the ObjectName, its distance and the relative position that is mentioned in the provided data. Stop when you have said it. Do not answer with json like format. Answer in regular text. \n");

    return true;
}

/**
 * Testklass för att verifiera svar för StoryWholeJson-frågan.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestGameMode_QuestionPrompt_StoryWholeJson, "project.GameMode.QuestionPrompt.StoryWholeJson", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TestGameMode_QuestionPrompt_StoryWholeJson::RunTest(const FString& Parameters)
{
    AprojectGameMode* GameMode = NewObject<AprojectGameMode>();
    FString Result = GameMode->question_prompt("StoryWholeJson");
    TestEqual(TEXT("StoryWholeJson prompt should match"), Result, "Write a short story using the following details without explaining your process.\n");

    return true;
}

/**
 * Testklass för att verifiera svar för Story-frågan.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestGameMode_QuestionPrompt_Story, "project.GameMode.QuestionPrompt.Story", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TestGameMode_QuestionPrompt_Story::RunTest(const FString& Parameters)
{
    AprojectGameMode* GameMode = NewObject<AprojectGameMode>();
    FString Result = GameMode->question_prompt("Story");
    TestEqual(TEXT("Story prompt should match"), Result, "Generate a coherent and engaging story based on the given parameters without explaining the steps.");

    return true;
}
