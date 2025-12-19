// Fill out your copyright notice in the Description page of Project Settings.


#include "MainAssetManager.h"

void UMainAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	FCharacterGameplayTags::InitializeNativeGameplayTags();
}
