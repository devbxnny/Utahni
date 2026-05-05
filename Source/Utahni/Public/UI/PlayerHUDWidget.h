#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

class ABasePaperPlayer;
class UImage;
class UWidget;
class UTextBlock;
class UWidgetAnimation;
class UUtahniCheatManager;

UCLASS()
class UTAHNI_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "HUD")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintCallable, Category = "HUD")
	float GetCurrentHealth() const;

	UFUNCTION(BlueprintCallable, Category = "HUD")
	float GetMaxHealth() const;

protected:
	UPROPERTY(meta = (BindWidget))
	UWidget* CheatNotificationBox = nullptr;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CheatNotificationText = nullptr;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* CheatNotificationFadeOut = nullptr;

	UPROPERTY(meta = (BindWidget))
	UWidget* ControlsPanel = nullptr;

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	ABasePaperPlayer* GetPlayerCharacter() const;

	UPROPERTY(meta = (BindWidget))
	UImage* health_red = nullptr;

private:
	float FullHealthBarWidth = 0.0f;

	FTimerHandle HideControlsTimerHandle;
	FTimerHandle CheatNotificationFadeTimerHandle;
	FTimerHandle CheatNotificationHideTimerHandle;

	void HideControlsPanel();

	UFUNCTION()
	void ShowCheatNotification(FText CheatMessage);

	void StartCheatNotificationFade();
	void HideCheatNotification();

	void BindToCheatManager();
};