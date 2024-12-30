// Inkluderar grundläggande funktionalitet i Unreal Engine
#include "CoreMinimal.h"

// Inkluderar funktionalitet för att skapa automatiserade tester
#include "Misc/AutomationTest.h"

// Inkluderar definitionen av klassen UHttpHandler_Get som testas
#include "../HttpHandler_Get.h"

// Definierar ett automatiserat test för att testa TrimResponse med radbrytningar
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestHttpHandler_TrimResponse_WithNewlines, "project.HttpHandler.TrimResponse.WithNewlines", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

// Implementerar testet för TrimResponse när input innehåller radbrytningar
bool TestHttpHandler_TrimResponse_WithNewlines::RunTest(const FString& Parameters) {
    // Skapar en ny instans av UHttpHandler_Get
    UHttpHandler_Get* HttpHandler = NewObject<UHttpHandler_Get>();
    
    // Definierar en inputsträng med ledande och avslutande radbrytningar
    FString Input = "\n\nTeststräng\n\n";
    
    // Förväntat resultat efter att TrimResponse anropas
    FString Expected = "Teststräng";
    
    // Anropar TrimResponse för att bearbeta input
    FString Result = HttpHandler->TrimResponse(Input);
    
    // Kontrollerar om resultatet matchar det förväntade värdet
    TestEqual(TEXT("Should trim leading and trailing newlines"), Result, Expected);
    
    // Returnerar true för att indikera att testet har körts klart
    return true;
}

// Definierar ett automatiserat test för att testa TrimResponse utan radbrytningar
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestHttpHandler_TrimResponse_NoNewlines, "project.HttpHandler.TrimResponse.NoNewlines", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

// Implementerar testet för TrimResponse när input inte innehåller radbrytningar
bool TestHttpHandler_TrimResponse_NoNewlines::RunTest(const FString& Parameters) {
    // Skapar en ny instans av UHttpHandler_Get
    UHttpHandler_Get* HttpHandler = NewObject<UHttpHandler_Get>();
    
    // Definierar en inputsträng utan radbrytningar
    FString Input = "Teststräng";
    
    // Förväntat resultat: samma som input eftersom inga radbrytningar finns
    FString Expected = "Teststräng";
    
    // Anropar TrimResponse för att bearbeta input
    FString Result = HttpHandler->TrimResponse(Input);
    
    // Kontrollerar om resultatet matchar det förväntade värdet
    TestEqual(TEXT("Should return the string as-is when no newlines"), Result, Expected);
    
    // Returnerar true för att indikera att testet har körts klart
    return true;
}

// Definierar ett automatiserat test för att testa TrimResponse med en tom sträng
IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestHttpHandler_TrimResponse_Empty, "project.HttpHandler.TrimResponse.Empty", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

// Implementerar testet för TrimResponse när input är en tom sträng
bool TestHttpHandler_TrimResponse_Empty::RunTest(const FString& Parameters) {
    // Skapar en ny instans av UHttpHandler_Get
    UHttpHandler_Get* HttpHandler = NewObject<UHttpHandler_Get>();
    
    // Definierar en tom inputsträng
    FString Input = "";
    
    // Förväntat resultat: en tom sträng
    FString Expected = "";
    
    // Anropar TrimResponse för att bearbeta input
    FString Result = HttpHandler->TrimResponse(Input);
    
    // Kontrollerar om resultatet matchar det förväntade värdet
    TestEqual(TEXT("Should return an empty string when input is empty"), Result, Expected);
    
    // Returnerar true för att indikera att testet har körts klart
    return true;
}
