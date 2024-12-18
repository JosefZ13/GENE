#include "gtest/gtest.h"
#include "gamemode_playerrelativity.h"
#include "Engine/World.h"
#include "MockWorld.h" 

class AprojectGameModeTests : public ::testing::Test {
protected:
    AprojectGameMode* GameMode;

    virtual void SetUp() override {
        GameMode = new AprojectGameMode();
    }

    virtual void TearDown() override {
        delete GameMode;
    }
};

TEST_F(AprojectGameModeTests, AddTrackedActorTest) {
    AActor* MockActor = new AActor();
    GameMode->AddTrackedActor(MockActor, TEXT("TestActor"), false);

    ASSERT_EQ(GameMode->TrackedObjects.Num(), 1);
    ASSERT_EQ(GameMode->TrackedObjects[0].Actor, MockActor);
    ASSERT_EQ(GameMode->TrackedObjects[0].ActorName, TEXT("TestActor"));
    ASSERT_FALSE(GameMode->TrackedObjects[0].IsPlayer);

    delete MockActor;
}
/* Tests functionality of adding actors to the tracking system and ensures that actors are correctly added with the appropriate attributes, such as name and player status.

Verifies the actor is stored in the TrackedObjects array.

Ensures attributes like ActorName and IsPlayer are correctly assigned.*/

TEST_F(AprojectGameModeTests, GeneratePayloadTest) {
    AActor* MockActor = new AActor();
    FString Payload = GameMode->GeneratePayload(MockActor, 100.0f, TEXT("Front-Left"));

    ASSERT_EQ(Payload, TEXT("{\"TargetObject\":\"Default__Actor\",\"Distance\":100.00,\"RelativePosition\":\"Front-Left\"}"));

    delete MockActor;
}
/*Validates the JSON string generation for actor data and ensures that the payload contains accurate and formatted information 
about an actor's name, distance, and relative position. Confirms the format of the JSON payload. Verifies the inclusion of correct
distance and position information*/

TEST_F(AprojectGameModeTests, GetPlayerRelativityTest_InvalidActor) {
    FString Result = GameMode->GetPlayerRelativityTestable(nullptr);
    ASSERT_EQ(Result, TEXT("Invalid actor or no tracked objects."));
}
/*Returns an error message for invalid actors or an empty TrackedObjects array. 

For valid actors, verifies the JSON output includes correct relative position and distance. */

TEST_F(AprojectGameModeTests, GetPlayerRelativityTest_ValidActor) {
    AActor* PlayerActor = new AActor();
    PlayerActor->SetActorLocation(FVector(0.0f, 0.0f, 0.0f));
    GameMode->AddTrackedActor(PlayerActor, TEXT("Player"), true);

    AActor* TargetActor = new AActor();
    TargetActor->SetActorLocation(FVector(100.0f, 100.0f, 0.0f));

    FString Result = GameMode->GetPlayerRelativityTestable(TargetActor);

    ASSERT_TRUE(Result.Contains(TEXT("\"Distance\":141.42"))); // Check approximate distance
    ASSERT_TRUE(Result.Contains(TEXT("\"RelativePosition\":\"Actor is in front-right of the player.\"")));

    delete PlayerActor;
    delete TargetActor;
}

TEST_F(AprojectGameModeTests, PerformTracking_UpdatesPosition) {
    AActor* MockActor = new AActor();
    MockActor->SetActorLocation(FVector(100, 0, 0));
    GameMode->AddTrackedActor(MockActor, TEXT("TestActor"), false);

    FVector NewPosition(200, 0, 0);
    MockActor->SetActorLocation(NewPosition);
    GameMode->PerformTracking();

    ASSERT_EQ(GameMode->TrackedObjects[0].PreviousPosition, NewPosition);

    delete MockActor;
}
/*Verifies that the actor's previous position is updated when movement is detected. 

Confirms the system detects position changes correctly. */

TEST_F(AprojectGameModeTests, DetermineQuadrantTest) {
    FVector VectorNE(1.0f, 1.0f, 0.0f);
    ASSERT_EQ(GameMode->DetermineQuadrant(VectorNE), TEXT("North-East"));

    FVector VectorNW(-1.0f, 1.0f, 0.0f);
    ASSERT_EQ(GameMode->DetermineQuadrant(VectorNW), TEXT("North-West"));

    FVector VectorSW(-1.0f, -1.0f, 0.0f);
    ASSERT_EQ(GameMode->DetermineQuadrant(VectorSW), TEXT("South-West"));

    FVector VectorSE(1.0f, -1.0f, 0.0f);
    ASSERT_EQ(GameMode->DetermineQuadrant(VectorSE), TEXT("South-East"));
}
/*Checks that input vectors are classified into the correct quadrant based on their X and Y values. */

TEST_F(AprojectGameModeTests, CalculateAngleTest) {
    FVector Vector45(1.0f, 1.0f, 0.0f);
    ASSERT_NEAR(GameMode->CalculateAngle(Vector45), 45.0f, 0.001f);

    FVector Vector135(-1.0f, 1.0f, 0.0f);
    ASSERT_NEAR(GameMode->CalculateAngle(Vector135), 135.0f, 0.001f);

    FVector VectorMinus135(-1.0f, -1.0f, 0.0f);
    ASSERT_NEAR(GameMode->CalculateAngle(VectorMinus135), -135.0f, 0.001f);

    FVector VectorMinus45(1.0f, -1.0f, 0.0f);
    ASSERT_NEAR(GameMode->CalculateAngle(VectorMinus45), -45.0f, 0.001f);
}

TEST_F(AprojectGameModeTests, HttpSendReqTest) {
    /*Mocking the HttpHandler or World system would be needed to properly test HTTP functionality.
    Ensure that when an actor moves, the correct JSON payload is generated and sent*/
    AActor* MockActor = new AActor();
    MockActor->SetActorLocation(FVector(300, 200, 0));
    GameMode->AddTrackedActor(MockActor, TEXT("TestActor"), false);

    FVector NewPosition(400, 200, 0);
    MockActor->SetActorLocation(NewPosition);

    FString ExpectedPayload = GameMode->GetPlayerRelativityTestable(MockActor);
    GameMode->PerformTracking();

    // Validate Payload Content (using hypothetical mock validation)
    ASSERT_TRUE(ExpectedPayload.Contains(TEXT("\"TargetObject\":\"Default__Actor\"")));
    ASSERT_TRUE(ExpectedPayload.Contains(TEXT("\"Distance\":100.00")));
    ASSERT_TRUE(ExpectedPayload.Contains(TEXT("\"RelativePosition\":\"Actor is in front-right of the player.\"")));

    delete MockActor;
}
