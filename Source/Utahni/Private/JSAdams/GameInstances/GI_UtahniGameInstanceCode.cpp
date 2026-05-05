//----- GI_UtahniGameInstanceCode.cpp START ----- 

#include "JSAdams/GameInstances/GI_UtahniGameInstanceCode.h"
#include "JSAdams/SaveGame/SG_UtahniSettingsCode.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

void UGI_UtahniGameInstanceCode::Init()
{
	Super::Init();

	LoadSettings();
}

void UGI_UtahniGameInstanceCode::LoadSettings()
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex))
	{
		USaveGame* LoadedGame = UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex);
		USG_UtahniSettingsCode* LoadedSettings = Cast<USG_UtahniSettingsCode>(LoadedGame);

		if (LoadedSettings)
		{
			MasterVolume = LoadedSettings->MasterVolume;
			MusicVolume = LoadedSettings->MusicVolume;
			SFXVolume = LoadedSettings->SFXVolume;
			CurrentLevelName = LoadedSettings->CurrentLevelName;

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					5.0f,
					FColor::Green,
					FString::Printf(TEXT("C++ LOADED LEVEL: %s"), *CurrentLevelName)
				);
			}

			return;
		}
	}

	MasterVolume = 1.0f;
	MusicVolume = 1.0f;
	SFXVolume = 1.0f;
	CurrentLevelName = TEXT("NVHuynh_GrassMap");

	SaveSettings();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.0f,
			FColor::Yellow,
			TEXT("C++ CREATED DEFAULT SAVE")
		);
	}
}

void UGI_UtahniGameInstanceCode::SaveSettings()
{
	USG_UtahniSettingsCode* SaveObject = Cast<USG_UtahniSettingsCode>(
		UGameplayStatics::CreateSaveGameObject(USG_UtahniSettingsCode::StaticClass())
	);

	if (!SaveObject)
	{
		return;
	}

	SaveObject->MasterVolume = MasterVolume;
	SaveObject->MusicVolume = MusicVolume;
	SaveObject->SFXVolume = SFXVolume;
	SaveObject->CurrentLevelName = CurrentLevelName;

	const bool bSaved = UGameplayStatics::SaveGameToSlot(SaveObject, SaveSlotName, UserIndex);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.0f,
			bSaved ? FColor::Cyan : FColor::Red,
			FString::Printf(TEXT("C++ SAVE LEVEL: %s"), *CurrentLevelName)
		);
	}
}

void UGI_UtahniGameInstanceCode::SaveCurrentLevel(const FString& LevelName)
{
	CurrentLevelName = LevelName;
	SaveSettings();
}

void UGI_UtahniGameInstanceCode::OpenSavedOrDefaultLevel()
{
	LoadSettings();

	FString LevelToOpen = CurrentLevelName;

	if (LevelToOpen.IsEmpty())
	{
		LevelToOpen = TEXT("NVHuynh_GrassMap");
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.0f,
			FColor::Orange,
			FString::Printf(TEXT("C++ OPENING LEVEL: %s"), *LevelToOpen)
		);
	}

	UGameplayStatics::OpenLevel(this, FName(*LevelToOpen));
}

//----- GI_UtahniGameInstanceCode.cpp END -----


