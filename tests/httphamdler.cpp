#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "../HttpHandler_Get.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestHttpHandler_TrimResponse_WithNewlines, "MyProject.HttpHandler.TrimResponse.WithNewlines", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TestHttpHandler_TrimResponse_WithNewlines::RunTest(const FString& Parameters) {
    UHttpHandler_Get* HttpHandler = NewObject<UHttpHandler_Get>();
    FString Input = "\n\nTeststräng\n\n";
    FString Expected = "Teststräng";
    FString Result = HttpHandler->TrimResponse(Input);
    TestEqual(TEXT("Should trim leading and trailing newlines"), Result, Expected);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestHttpHandler_TrimResponse_NoNewlines, "MyProject.HttpHandler.TrimResponse.NoNewlines", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TestHttpHandler_TrimResponse_NoNewlines::RunTest(const FString& Parameters) {
    UHttpHandler_Get* HttpHandler = NewObject<UHttpHandler_Get>();
    FString Input = "Teststräng";
    FString Expected = "Teststräng";
    FString Result = HttpHandler->TrimResponse(Input);
    TestEqual(TEXT("Should return the string as-is when no newlines"), Result, Expected);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestHttpHandler_TrimResponse_Empty, "MyProject.HttpHandler.TrimResponse.Empty", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TestHttpHandler_TrimResponse_Empty::RunTest(const FString& Parameters) {
    UHttpHandler_Get* HttpHandler = NewObject<UHttpHandler_Get>();
    FString Input = "";
    FString Expected = "";
    FString Result = HttpHandler->TrimResponse(Input);
    TestEqual(TEXT("Should return an empty string when input is empty"), Result, Expected);
    return true;
}
