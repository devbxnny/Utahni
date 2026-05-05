// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SG_UtahniSettingsCode.generated.h"

/**
 * 
 */
UCLASS()
class UTAHNI_API USG_UtahniSettingsCode : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Audio")
	float MasterVolume = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Audio")
	float MusicVolume = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Audio")
	float SFXVolume = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Progress")
	FString CurrentLevelName = TEXT("NVHuynh_GrassMap");
};
