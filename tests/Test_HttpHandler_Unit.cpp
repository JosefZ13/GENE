// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "HttpHandler_Get.h"
#include "JsonObjectConverter.h"

/**
 * Test för att kontrollera TrimResponse-funktionen i UHttpHandler_Get.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHttpHandlerTrimResponseTest, "HttpHandler.Unit.TrimResponse", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FHttpHandlerTrimResponseTest::RunTest(const FString& Parameters)
{
    UHttpHandler_Get* HttpHandler = NewObject<UHttpHandler_Get>();
    if (!HttpHandler)
    {
        AddError("Failed to create UHttpHandler_Get instance.");
        return false;
    }

    FString Response = TEXT("   \n { \"key\": \"value\" }   \r\n");
    FString TrimmedResponse = HttpHandler->TrimResponse(Response);

    TestEqual("TrimResponse removes whitespace and newlines", TrimmedResponse, TEXT("{ \"key\": \"value\" }"));

    return true;
}

/**
 * Test för att verifiera korrekt JSON-konvertering.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHttpHandlerJsonParsingTest, "HttpHandler.Unit.JsonParsing", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FHttpHandlerJsonParsingTest::RunTest(const FString& Parameters)
{
    UHttpHandler_Get* HttpHandler = NewObject<UHttpHandler_Get>();
    if (!HttpHandler)
    {
        AddError("Failed to create UHttpHandler_Get instance.");
        return false;
    }

    FString JsonString = TEXT("{ \"key\": \"value\" }");
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        FString KeyValue;
        JsonObject->TryGetStringField("key", KeyValue);

        TestEqual("Parsed key value", KeyValue, TEXT("value"));
    }
    else
    {
        AddError("Failed to parse JSON string.");
        return false;
    }

    return true;
}
