// Fill out your copyright notice in the Description page of Project Settings.
#include "projectGameMode.h"
//#include "HttpHandler_Get.h"
#include "projectCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/Actor.h"
#include "EngineUtils.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"


void AprojectGameMode::BeginPlay()
{
    PrimaryActorTick.bCanEverTick = true;
    numberOfTicks = 0;
    TickForGetWorld();
}

AprojectGameMode::AprojectGameMode()
    : Super() // super() is part of C++ constructor initialzation, specifically for calling the constructor of the parent (or base) class.
    // in this scenario Super() refers to AGameModeBase
{
    // set default pawn class to our Blueprinted character
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
    DefaultPawnClass = PlayerPawnClassFinder.Class;

}

void AprojectGameMode::TickForGetWorld()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle,                       // Timer Handle
            this,                              // Object that owns the function
            &AprojectGameMode::wrapper,    // Function to call
            5.0f,                              // Delay (5 seconds)
            true                               // Loop? (true = repeat every 5 seconds)
        );
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("GetWorld is null. Timer not set."));
    }
}

void AprojectGameMode::wrapper()
{
    numberOfTicks++;
    newJSON = GetWorldDataAsJson();
   // UE_LOG(LogTemp, Log, TEXT("NEWJSON AS: %s"), *newJSON);
    bool is_changed = CompareJsonStrings();
    UpdatePrevJson();

    if (is_changed == true)
    {
        UE_LOG(LogTemp, Log, TEXT("World state has changed"));
        HandleWorldDataChange();
    }
}

FString AprojectGameMode::GetWorldDataAsJson()
{
    /*
       Root JSON Object - creates the root object to hold all data
       Actors Data - Iterates through actors, creating a JSON object for each actor and adding it to an arrya
       Player data - Creates json object for the player and adds it to the root object
       Serialize to string - converts the json object into a string using UE's serializer

   */
    TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject());
    TArray<TSharedPtr<FJsonValue>> ActorsArray;
    TArray<TSharedPtr<FJsonValue>> LightingArray;
    TArray<TSharedPtr<FJsonValue>> GameplayArray;

    // Actor Data
    UWorld* World = GetWorld();

    if (World)
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;

            /*
                FJsonObject - represents a JSON object in ue. can be used to create or parse json objects
                TSharedPtr<FJsonObject> - A shared pointer used for handling FJsonObject instances
            */

            // new json object for each actor
            TSharedPtr<FJsonObject> ActorObject = MakeShareable(new FJsonObject());

            // basic variables
            ActorObject->SetStringField(TEXT("Name"), Actor->GetName());
            ActorObject->SetStringField(TEXT("Class"), Actor->GetClass()->GetName());

            // location nested object (easier for LLM to understand than using string):
            TSharedPtr<FJsonObject> LocationObject = MakeShareable(new FJsonObject());
            FVector ActorLocation = Actor->GetActorLocation();
            LocationObject->SetNumberField(TEXT("X"), ActorLocation.X);
            LocationObject->SetNumberField(TEXT("Y"), ActorLocation.Y);
            LocationObject->SetNumberField(TEXT("Z"), ActorLocation.Z);
            ActorObject->SetObjectField(TEXT("Location"), LocationObject);

            TArray<TSharedPtr<FJsonValue>> TagsArray;

            // categorize / tags
            if (Actor->GetClass()->GetName() == TEXT("SkyLight"))
            {
                TagsArray.Add(MakeShareable(new FJsonValueString(TEXT("Environment"))));
                TagsArray.Add(MakeShareable(new FJsonValueString(TEXT("Lighting"))));
                LightingArray.Add(MakeShareable(new FJsonValueObject(ActorObject)));
            }
            else if (Actor->GetClass()->GetName() == TEXT("PlayerStart"))
            {
                TagsArray.Add(MakeShareable(new FJsonValueString(TEXT("Gameplay"))));
                TagsArray.Add(MakeShareable(new FJsonValueString(TEXT("Spawn"))));
                ActorObject->SetStringField(TEXT("Role"), TEXT("Spawn Point"));
                GameplayArray.Add(MakeShareable(new FJsonValueObject(ActorObject)));
            }
            else
            {
                TagsArray.Add(MakeShareable(new FJsonValueString(TEXT("Actor"))));
                ActorsArray.Add(MakeShareable(new FJsonValueObject(ActorObject)));
            }
            ActorObject->SetArrayField(TEXT("Tags"), TagsArray);
        }
        // add to rootobject
        RootObject->SetArrayField(TEXT("Actors"), ActorsArray);
        RootObject->SetArrayField(TEXT("Lighting"), LightingArray);
        RootObject->SetArrayField(TEXT("Gameplay"), GameplayArray);
    }

    // Player Data
    TSharedPtr<FJsonObject> PlayerObject = MakeShareable(new FJsonObject());
    APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
    if (PlayerController)
    {
        APawn* PlayerPawn = PlayerController->GetPawn();
        if (PlayerPawn)
        {
            // Create a JSON object for the player
            PlayerObject->SetStringField(TEXT("Name"), PlayerPawn->GetName());
            PlayerObject->SetStringField(TEXT("Class"), PlayerPawn->GetClass()->GetName());

            // nested location object
            TSharedPtr<FJsonObject> PlayerLocationObject = MakeShareable(new FJsonObject());
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            PlayerLocationObject->SetNumberField(TEXT("X"), PlayerLocation.X);
            PlayerLocationObject->SetNumberField(TEXT("Y"), PlayerLocation.Y);
            PlayerLocationObject->SetNumberField(TEXT("Z"), PlayerLocation.Z);
            PlayerObject->SetObjectField(TEXT("Location"), PlayerLocationObject);

            // tags and player states ( not sure if state is nesseccary)
            TArray<TSharedPtr<FJsonValue>> PlayerTags;
            PlayerTags.Add(MakeShareable(new FJsonValueString(TEXT("Player"))));
            PlayerTags.Add(MakeShareable(new FJsonValueString(TEXT("Controllable"))));
            PlayerObject->SetArrayField(TEXT("Tags"), PlayerTags);

            TSharedPtr<FJsonObject> PlayerStateObject = MakeShareable(new FJsonObject());
            PlayerStateObject->SetNumberField(TEXT("Health"), 100); // Example value 
            PlayerStateObject->SetStringField(TEXT("Status"), TEXT("Idle")); // Example state 
            PlayerObject->SetObjectField(TEXT("State"), PlayerStateObject);
        }
    }
    RootObject->SetObjectField(TEXT("Player"), PlayerObject);

    FString JSONString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JSONString);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

    return *JSONString;
}


bool AprojectGameMode::CompareJsonStrings() {
    return newJSON == prevJson;
}

void AprojectGameMode::UpdatePrevJson() {
    prevJson = newJSON;

    if (prevJson == newJSON)
    {
        UE_LOG(LogTemp, Log, TEXT("Successfully updated old json with new json"));
    }
}

void AprojectGameMode::HandleWorldDataChange() {

    // Deserialize newjson
    TSharedPtr<FJsonObject> DeNewObject; 
    FString JSONString = *newJSON; // your JSON string here
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JSONString);
    if (FJsonSerializer::Deserialize(Reader, DeNewObject))
    {
        // Successful deserialization
        UE_LOG(LogTemp, Log, TEXT("Deserialization successful!"));
    }
    else
    {
        // Deserialization failed
        UE_LOG(LogTemp, Error, TEXT("Deserialization failed!"));
    }
    // Deserialize prevjson
    TSharedPtr<FJsonObject> DePrevObject;
    FString JSONString2 = *prevJson; // your JSON string here
    TSharedRef<TJsonReader<>> Reader2 = TJsonReaderFactory<>::Create(JSONString2);
    if (FJsonSerializer::Deserialize(Reader2, DePrevObject))
    {
        // Successful deserialization
        UE_LOG(LogTemp, Log, TEXT("Deserialization successful!"));
    }
    else
    {
        // Deserialization failed
        UE_LOG(LogTemp, Error, TEXT("Deserialization failed!"));
    }

     TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject());
     RootObject->SetStringField(TEXT("question prompt"), TEXT("What is the difference between the previous created json object and the newly created json object that is included in this nested json-object? If there is no difference between them, say that there is no difference."));
     RootObject->SetObjectField(TEXT("previous created json object"), DePrevObject);
     RootObject->SetObjectField(TEXT("newly created json object"), DeNewObject); 

     FString PayloadJson;
     TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&PayloadJson);
     FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

     UE_LOG(LogTemp, Log, TEXT("NEW: %s"), *PayloadJson);
     
    if (UWorld* World = GetWorld())
    {
        HttpHandler = CreateWidget<UHttpHandler_Get>(World, UHttpHandler_Get::StaticClass());
        if (HttpHandler && !PayloadJson.IsEmpty())
        {
            HttpHandler->AddToViewport();

            HttpHandler->httpSendReq(&PayloadJson);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("HttpHandler is null or PayloadJson is empty!"));
        }
    }
}
