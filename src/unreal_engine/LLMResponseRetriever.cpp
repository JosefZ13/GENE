// Fill out your copyright notice in the Description page of Project Settings.


#include "LLMResponseRetriever.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"

LLMResponseRetriever::LLMResponseRetriever()
{
}

LLMResponseRetriever::~LLMResponseRetriever()
{
}

void LLMResponseRetriever::ResponseFileRead()
{
	FString FilePath = FPaths::ProjectDir() + TEXT("LLM_Response/LLM_response.txt");
    

    if (FFileHelper::LoadFileToString(LLM_Response, *FilePath))
    {
        UE_LOG(LogTemp, Log, TEXT("File read successfully! Content:\n%s"), *LLM_Response);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to read file: %s"), *FilePath);
    }

    if (FFileHelper::SaveStringToFile(TEXT(""), *FilePath))
    {
        UE_LOG(LogTemp, Log, TEXT("File content cleared successfully: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to clear file content: %s"), *FilePath);
    }
}
