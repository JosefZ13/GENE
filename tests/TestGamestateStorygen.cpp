#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "../projectGameMode.h"
#include "../HttpHandler_Get.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Actor.h"
#include "Tests/AutomationEditorCommon.h"
#include "Kismet/GameplayStatics.h"
#include "GameStateStoryGen.h"
#include "Engine/World.h"
#include "JsonObjectConverter.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

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
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Content);
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
        MockResponse->SetStringField(TEXT("content"), TEXT("Mock response"));

        TSharedRef<IHttpResponse, ESPMode::ThreadSafe> MockResponseObject = MakeShareable(new FHttpResponseMock(MockResponse));
        Response = MockResponseObject;
        bWasSuccessful = true; // Simulate successful request
    }
};

// Test Implementation
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameStateStoryGenTest, "Project.GameStateStoryGen.UnitTests",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGameStateStoryGenTest::RunTest(const FString& Parameters)
{
    // Create a test world for running the tests
    UWorld* TestWorld = UGameplayStatics::CreateWorld(EWorldType::Game, false, FName("TestWorld"));
    if (!TestWorld)
    {
        AddError(TEXT("Failed to create test world."));
        return false;
    }

    // Spawn a GameStateStoryGen instance in the test world
    AGameStateStoryGen* TestGameState = TestWorld->SpawnActor<AGameStateStoryGen>();
    if (!TestGameState)
    {
        AddError(TEXT("Failed to spawn AGameStateStoryGen."));
        TestWorld->DestroyWorld(false);
        return false;
    }

    // === Test BeginPlay ===
    TestGameState->BeginPlay();
    TestTrue(TEXT("BeginPlay successfully triggered."), TestGameState->HasActorBegunPlay());

    // === Test GenerateStartEnvironment ===
    TSharedPtr<FJsonObject> Environment = TestGameState->GenerateStartEnvironment();
    TestNotNull(TEXT("GenerateStartEnvironment returns a valid JSON object."), Environment.Get());

    if (Environment.IsValid())
    {
        FString GameTitle;
        TestTrue(TEXT("JSON contains 'game_title' field."), Environment->TryGetStringField(TEXT("game_title"), GameTitle));
        TestEqual(TEXT("Game title is correctly set."), GameTitle, "Woods Adventure");
    }

    // === Test Actor Tracking ===
    TestGameState->GetActors();
    int32 TrackedCount = TestGameState->TrackedObjects.Num();
    TestTrue(TEXT("Tracked objects array is populated."), TrackedCount > 0);

    // === Test Relativity Calculations ===
    FVector PlayerPosition = FVector(0, 0, 0);
    FVector TargetPosition = FVector(100, 100, 0);
    AActor* TestActor = TestWorld->SpawnActor<AActor>();
    TestActor->SetActorLocation(TargetPosition);

    FString Relativity = TestGameState->GetRelativePosition(0.5, 0.5, 0.0);
    TestTrue(TEXT("Relative position is calculated correctly."), !Relativity.IsEmpty());

    // === Test HTTP Request ===
    MockHttpRequest MockRequest;
    MockRequest.OnProcessRequestComplete(nullptr, nullptr, false);

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
                    AddError(TEXT("Failed to parse JSON response."));
                }
            }
            else
            {
                AddError(TEXT("HTTP Response failed."));
            }
        });

    HttpRequest->SetURL(TEXT("https://api.openai.com/v1/chat/completions")); // Replace with the appropriate URL
    HttpRequest->SetVerb(TEXT("POST"));
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
    HttpRequest->SetContentAsString(TEXT("{\"key\":\"value\"}"));

    if (!HttpRequest->ProcessRequest())
    {
        AddError(TEXT("Failed to process HTTP request."));
    }

    // Destroy the test world
    TestWorld->DestroyWorld(false);

    return true;
}
