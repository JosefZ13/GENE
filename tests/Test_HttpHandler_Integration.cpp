// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "HttpHandler_Get.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

/**
 * Test för att verifiera HTTP-förfrågan och svarshantering.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHttpHandlerIntegrationTest, "HttpHandler.Integration.HttpRequestResponse", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FHttpHandlerIntegrationTest::RunTest(const FString& Parameters)
{
    UHttpHandler_Get* HttpHandler = NewObject<UHttpHandler_Get>();
    if (!HttpHandler)
    {
        AddError("Failed to create UHttpHandler_Get instance.");
        return false;
    }

    FString Payload = TEXT("{ \"test\": \"value\" }");
    FString Question = TEXT("What is this test?");
    FString Context = TEXT("IntegrationTest");

    // Startar HTTP-förfrågan
    HttpHandler->httpSendReq(Payload, Question, Context);

    // Kontrollera att filen sparas korrekt
    FString SavedFilePath = FPaths::ProjectDir() + TEXT("LLM_Response/Response.txt");
    FString FileContent;

    if (FFileHelper::LoadFileToString(FileContent, *SavedFilePath))
    {
        TestTrue("Response file exists and is not empty", !FileContent.IsEmpty());
    }
    else
    {
        AddError("Response file does not exist or could not be read.");
        return false;
    }

    return true;
}

/**
 * Test för att verifiera TrimResponse och sparandet av ett trim-meddelande.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHttpHandlerTrimAndSaveTest, "HttpHandler.Integration.TrimAndSave", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FHttpHandlerTrimAndSaveTest::RunTest(const FString& Parameters)
{
    UHttpHandler_Get* HttpHandler = NewObject<UHttpHandler_Get>();
    if (!HttpHandler)
    {
        AddError("Failed to create UHttpHandler_Get instance.");
        return false;
    }

    FString Response = TEXT("   \n { \"message\": \"success\" }   \r\n");
    FString TrimmedResponse = HttpHandler->TrimResponse(Response);

    FString SavedFilePath = FPaths::ProjectDir() + TEXT("LLM_Response/TrimmedResponse.txt");
    if (FFileHelper::SaveStringToFile(TrimmedResponse, *SavedFilePath))
    {
        FString FileContent;
        if (FFileHelper::LoadFileToString(FileContent, *SavedFilePath))
        {
            TestEqual("Trimmed response saved correctly", FileContent, TEXT("{ \"message\": \"success\" }"));
        }
        else
        {
            AddError("Failed to read the trimmed response file.");
            return false;
        }
    }
    else
    {
        AddError("Failed to save trimmed response.");
        return false;
    }

    return true;
}
