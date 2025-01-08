#include "CoreMinimal.h" // Core Unreal Engine functionality
#include "Misc/AutomationTest.h" // Automation testing framework
#include "../projectGameMode.h" // Project-specific GameMode header
#include "../HttpHandler_Get.h" // Project-specific HTTP handler
#include "Misc/Paths.h" // File path utilities
#include "Misc/FileHelper.h" // File helper utilities
#include "GameFramework/GameModeBase.h" // Base GameMode functionality
#include "GameFramework/Actor.h" // Actor class
#include "Tests/AutomationEditorCommon.h" // Editor-specific automation utilities
#include "Kismet/GameplayStatics.h" // Gameplay utility functions
#include "GameStateStoryGen.h" // Custom GameState class for testing
#include "Engine/World.h" // World management utilities
#include "JsonObjectConverter.h" // JSON utilities
#include "HttpModule.h" // HTTP communication module
#include "Interfaces/IHttpRequest.h" // HTTP request interface
#include "Interfaces/IHttpResponse.h" // HTTP response interface

// Mock HTTP Response Class for Testing
class FHttpResponseMock : public IHttpResponse
{
private:
    FString Content; // Holds the mock response content
    int32 ResponseCode; // Simulated HTTP response code

public:
    // Constructor: Simulates a JSON response
    FHttpResponseMock(const TSharedPtr<FJsonObject>& JsonResponse)
    {
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Content); // Serialize JSON to a string
        FJsonSerializer::Serialize(JsonResponse.ToSharedRef(), Writer);

        ResponseCode = 200; // Simulate HTTP 200 OK
    }

    // Override: Return the response content as a string
    virtual const FString& GetContentAsString() const override { return Content; }

    // Override: Return the HTTP response code
    virtual int32 GetResponseCode() const override { return ResponseCode; }

    // Additional IHttpResponse methods can be implemented if necessary
};

// Mock HTTP Request Class for Testing
class MockHttpRequest : public IHttpRequest
{
public:
    // Simulate an HTTP request completion callback
    virtual void OnProcessRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) override
    {
        // Create a mock JSON response
        TSharedPtr<FJsonObject> MockResponse = MakeShareable(new FJsonObject());
        MockResponse->SetStringField(TEXT("content"), TEXT("Mock response")); // Add a mock response field

        TSharedRef<IHttpResponse, ESPMode::ThreadSafe> MockResponseObject = MakeShareable(new FHttpResponseMock(MockResponse));
        Response = MockResponseObject; // Assign the mock response
        bWasSuccessful = true; // Simulate a successful request
    }
};

// Test Implementation
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameStateStoryGenTest, "Project.GameStateStoryGen.UnitTests",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter) // Define the test

// Main test logic
bool FGameStateStoryGenTest::RunTest(const FString& Parameters)
{
    // Create a test world for running the tests
    UWorld* TestWorld = UGameplayStatics::CreateWorld(EWorldType::Game, false, FName("TestWorld"));
    if (!TestWorld)
    {
        AddError(TEXT("Failed to create test world.")); // Log error if the world creation fails
        return false;
    }

    // Spawn a GameStateStoryGen instance in the test world
    AGameStateStoryGen* TestGameState = TestWorld->SpawnActor<AGameStateStoryGen>();
    if (!TestGameState)
    {
        AddError(TEXT("Failed to spawn AGameStateStoryGen.")); // Log error if actor creation fails
        TestWorld->DestroyWorld(false);
        return false;
    }

    // === Test BeginPlay ===
    TestGameState->BeginPlay(); // Trigger BeginPlay
    TestTrue(TEXT("BeginPlay successfully triggered."), TestGameState->HasActorBegunPlay()); // Verify BeginPlay

    // === Test GenerateStartEnvironment ===
    TSharedPtr<FJsonObject> Environment = TestGameState->GenerateStartEnvironment(); // Call custom logic
    TestNotNull(TEXT("GenerateStartEnvironment returns a valid JSON object."), Environment.Get());

    if (Environment.IsValid())
    {
        FString GameTitle;
        TestTrue(TEXT("JSON contains 'game_title' field."), Environment->TryGetStringField(TEXT("game_title"), GameTitle));
        TestEqual(TEXT("Game title is correctly set."), GameTitle, "Woods Adventure"); // Check expected field value
    }

    // === Test Actor Tracking ===
    TestGameState->GetActors(); // Retrieve tracked actors
    int32 TrackedCount = TestGameState->TrackedObjects.Num(); // Count tracked objects
    TestTrue(TEXT("Tracked objects array is populated."), TrackedCount > 0);

    // === Test Relativity Calculations ===
    FVector PlayerPosition = FVector(0, 0, 0); // Define reference position
    FVector TargetPosition = FVector(100, 100, 0); // Define target position
    AActor* TestActor = TestWorld->SpawnActor<AActor>(); // Spawn test actor
    TestActor->SetActorLocation(TargetPosition); // Set actor position

    FString Relativity = TestGameState->GetRelativePosition(0.5, 0.5, 0.0); // Compute relative position
    TestTrue(TEXT("Relative position is calculated correctly."), !Relativity.IsEmpty()); // Verify calculation

    // === Test HTTP Request ===
    MockHttpRequest MockRequest;
    MockRequest.OnProcessRequestComplete(nullptr, nullptr, false); // Simulate request completion

    // Create a mock HTTP request
    FHttpModule* HttpModule = &FHttpModule::Get();
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule->CreateRequest();

    HttpRequest->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            if (bWasSuccessful && Response.IsValid())
            {
                const FString Content = Response->GetContentAsString();
                TestTrue(TEXT("HTTP Response is valid."), !Content.IsEmpty());
                UE_LOG(LogTemp, Log, TEXT("HTTP Response Content: %s"), *Content);

                TSharedPtr<FJsonObject> JsonResponse;
                TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);

                if (FJsonSerializer::Deserialize(Reader, JsonResponse) && JsonResponse.IsValid())
                {
                    FString ResponseContent;
                    TestTrue(TEXT("JSON Response is valid."), JsonResponse->TryGetStringField(TEXT("content"), ResponseContent));
                    TestTrue(TEXT("Response content is not empty."), !ResponseContent.IsEmpty());
                }
                else
                {
                    AddError(TEXT("Failed to parse JSON response.")); // Log error if parsing fails
                }
            }
            else
            {
                AddError(TEXT("HTTP Response failed.")); // Log error if response is invalid
            }
        });

    HttpRequest->SetURL(TEXT("https://api.openai.com/v1/chat/completions")); // Replace with the appropriate URL
    HttpRequest->SetVerb(TEXT("POST")); // HTTP POST method
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8")); // Set request header
    HttpRequest->SetContentAsString(TEXT("{\"key\":\"value\"}")); // Set mock request content

    if (!HttpRequest->ProcessRequest())
    {
        AddError(TEXT("Failed to process HTTP request.")); // Log error if request processing fails
    }

    // Destroy the test world
    TestWorld->DestroyWorld(false);

    return true; // Return test result
}
