

//----- GI_UtahniGameInstanceCode.h START -----


#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GI_UtahniGameInstanceCode.generated.h"
/**
 * 
 */

class USG_UtahniSettingsCode;




UCLASS()
class UTAHNI_API UGI_UtahniGameInstanceCode : public UGameInstance
{
	GENERATED_BODY()
public:
	virtual void Init() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Save Data")
	float MasterVolume = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Save Data")
	float MusicVolume = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Save Data")
	float SFXVolume = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Save Data")
	FString CurrentLevelName = TEXT("NVHuynh_GrassMap");

	UFUNCTION(BlueprintCallable, Category = "Save Data")
	void LoadSettings();

	UFUNCTION(BlueprintCallable, Category = "Save Data")
	void SaveSettings();

	UFUNCTION(BlueprintCallable, Category = "Save Data")
	void SaveCurrentLevel(const FString& LevelName);

	UFUNCTION(BlueprintCallable, Category = "Save Data")
	void OpenSavedOrDefaultLevel();

private:
	const FString SaveSlotName = TEXT("SettingsSlot");
	const int32 UserIndex = 0;
	
};


//----- GI_UtahniGameInstanceCode.h END -----


