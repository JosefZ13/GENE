// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 *	
 */
class PROJECT_API LLMResponseRetriever
{
public:
	LLMResponseRetriever();
	~LLMResponseRetriever();
	void ResponseFileRead();

	FString LLM_Response;

};
