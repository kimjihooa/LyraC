// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"

/**
 * 
 */
struct FCharacterGameplayTags
{
public:
	static void InitializeNativeGameplayTags();
	static const FCharacterGameplayTags& Get() { return GameplayTags; };

	FGameplayTag State_Combat_Firing;

protected:
	void RegisterAllTags(UGameplayTagsManager& Manager);
	static FCharacterGameplayTags GameplayTags;
};