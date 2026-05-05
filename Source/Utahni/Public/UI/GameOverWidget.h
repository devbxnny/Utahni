#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameOverWidget.generated.h"

class UTextBlock;
class UButton;
class UWidgetAnimation;
class UWidget;
class UUserWidget;

UCLASS()
class UTAHNI_API UGameOverWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetMainMenuLevelName(const FName& InLevelName);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Resume = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MainMenu = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CheatsButton = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BackFromCheatsButton = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> CheatsPanel = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SettingsButton = nullptr;
	
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	TSubclassOf<UUserWidget> SettingsWidgetClass;


private:
	UFUNCTION()
	void HandleResumeClicked();

	UFUNCTION()
	void HandleMainMenuClicked();

	UPROPERTY()
	FName MainMenuLevelName = NAME_None;

	UFUNCTION()
	void HandleCheatsClicked();

	UFUNCTION()
	void HandleBackFromCheatsClicked();

	UFUNCTION()
	void HandleSettingsClicked();
};