#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "GameFramework/Actor.h"
#include "AprojectGameMode.h"
#include "Engine/World.h"

class AprojectGameModeTests : public FAutomationTestBase
{
public:
    AprojectGameModeTests(const FString& InName)
        : FAutomationTestBase(InName) {}

    virtual bool RunTest(const FString& Parameters) override
    {
        AprojectGameMode* GameMode = NewObject<AprojectGameMode>();

        // Test AddTrackedActorTest
        {
            AActor* MockActor = NewObject<AActor>();
            GameMode->AddTrackedActor(MockActor, TEXT("TestActor"), false);
            
            TestEqual(TEXT("TrackedObjects should contain 1 actor."), GameMode->TrackedObjects.Num(), 1);
            TestEqual(TEXT("Actor name should match."), GameMode->TrackedObjects[0].ActorName, TEXT("TestActor"));
            TestFalse(TEXT("Actor should not be a player."), GameMode->TrackedObjects[0].IsPlayer);
        }

        // Test GeneratePayloadTest
        {
            AActor* MockActor = NewObject<AActor>();
            FString Payload = GameMode->GeneratePayload(MockActor, 100.0f, TEXT("Front-Left"));
            
            TestEqual(TEXT("Payload should match the expected format."), Payload, TEXT("{\"TargetObject\":\"Default__Actor\",\"Distance\":100.00,\"RelativePosition\":\"Front-Left\"}"));
        }

        // Test GetPlayerRelativityTest_InvalidActor
        {
            FString Result = GameMode->GetPlayerRelativityTestable(nullptr);
            TestEqual(TEXT("Result should indicate invalid actor."), Result, TEXT("Invalid actor or no tracked objects."));
        }

        // Test GetPlayerRelativityTest_ValidActor
        {
            AActor* PlayerActor = NewObject<AActor>();
            PlayerActor->SetActorLocation(FVector(0.0f, 0.0f, 0.0f));
            GameMode->AddTrackedActor(PlayerActor, TEXT("Player"), true);

            AActor* TargetActor = NewObject<AActor>();
            TargetActor->SetActorLocation(FVector(100.0f, 100.0f, 0.0f));

            FString Result = GameMode->GetPlayerRelativityTestable(TargetActor);

            TestTrue(TEXT("Result should contain distance."), Result.Contains(TEXT("\"Distance\":141.42")));
            TestTrue(TEXT("Result should contain relative position."), Result.Contains(TEXT("\"RelativePosition\":\"Actor is in front-right of the player.\"")));
        }

        // Test PerformTracking_UpdatesPosition
        {
            AActor* MockActor = NewObject<AActor>();
            MockActor->SetActorLocation(FVector(100, 0, 0));
            GameMode->AddTrackedActor(MockActor, TEXT("TestActor"), false);

            FVector NewPosition(200, 0, 0);
            MockActor->SetActorLocation(NewPosition);
            GameMode->PerformTracking();

            TestEqual(TEXT("Tracked actor position should be updated."), GameMode->TrackedObjects[0].PreviousPosition, NewPosition);
        }

        // Test DetermineQuadrantTest
        {
            FVector VectorNE(1.0f, 1.0f, 0.0f);
            TestEqual(TEXT("Vector NE should be in North-East."), GameMode->DetermineQuadrant(VectorNE), TEXT("North-East"));

            FVector VectorNW(-1.0f, 1.0f, 0.0f);
            TestEqual(TEXT("Vector NW should be in North-West."), GameMode->DetermineQuadrant(VectorNW), TEXT("North-West"));

            FVector VectorSW(-1.0f, -1.0f, 0.0f);
            TestEqual(TEXT("Vector SW should be in South-West."), GameMode->DetermineQuadrant(VectorSW), TEXT("South-West"));

            FVector VectorSE(1.0f, -1.0f, 0.0f);
            TestEqual(TEXT("Vector SE should be in South-East."), GameMode->DetermineQuadrant(VectorSE), TEXT("South-East"));
        }

        // Test CalculateAngleTest
        {
            FVector Vector45(1.0f, 1.0f, 0.0f);
            TestNear(TEXT("Angle of vector (1, 1) should be ~45."), GameMode->CalculateAngle(Vector45), 45.0f, 0.001f);

            FVector Vector135(-1.0f, 1.0f, 0.0f);
            TestNear(TEXT("Angle of vector (-1, 1) should be ~135."), GameMode->CalculateAngle(Vector135), 135.0f, 0.001f);

            FVector VectorMinus135(-1.0f, -1.0f, 0.0f);
            TestNear(TEXT("Angle of vector (-1, -1) should be ~-135."), GameMode->CalculateAngle(VectorMinus135), -135.0f, 0.001f);

            FVector VectorMinus45(1.0f, -1.0f, 0.0f);
            TestNear(TEXT("Angle of vector (1, -1) should be ~-45."), GameMode->CalculateAngle(VectorMinus45), -45.0f, 0.001f);
        }

        // Test HttpSendReqTest (requires HTTP mocking or system integration)
        // This is a placeholder since mocking HTTP requests in Unreal Engine can be more involved.
        {
            AActor* MockActor = NewObject<AActor>();
            MockActor->SetActorLocation(FVector(300, 200, 0));
            GameMode->AddTrackedActor(MockActor, TEXT("TestActor"), false);

            FVector NewPosition(400, 200, 0);
            MockActor->SetActorLocation(NewPosition);

            FString ExpectedPayload = GameMode->GetPlayerRelativityTestable(MockActor);
            GameMode->PerformTracking();

            // Validate Payload Content (assuming some HTTP mocking system is in place)
            TestTrue(TEXT("Payload should contain target object."), ExpectedPayload.Contains(TEXT("\"TargetObject\":\"Default__Actor\"")));
            TestTrue(TEXT("Payload should contain distance."), ExpectedPayload.Contains(TEXT("\"Distance\":100.00")));
            TestTrue(TEXT("Payload should contain relative position."), ExpectedPayload.Contains(TEXT("\"RelativePosition\":\"Actor is in front-right of the player.\"")));
        }

        return true;
    }
};

// Register the test
IMPLEMENT_SIMPLE_AUTOMATION_TEST(AprojectGameModeTests, "GameModeTests.AprojectGameMode", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
