// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterGameplayTags.h"

FCharacterGameplayTags FCharacterGameplayTags::GameplayTags;

void FCharacterGameplayTags::InitializeNativeGameplayTags()
{
	GameplayTags.RegisterAllTags(UGameplayTagsManager::Get());
}

void FCharacterGameplayTags::RegisterAllTags(UGameplayTagsManager& Manager)
{
	State_Combat_Firing = Manager.AddNativeGameplayTag(FName("State.Combat.Firing"));
}
