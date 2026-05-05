#include "UI/PlayerHUDWidget.h"
#include "BasePaperPlayer.h"
#include "HealthComponent.h"
#include "GameFramework/Pawn.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Widget.h"
#include "TimerManager.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"
#include "Cheats/UtahniCheatManager.h"
#include "GameFramework/PlayerController.h"

void UPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (health_red != nullptr)
	{
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(health_red->Slot))
		{
			FullHealthBarWidth = CanvasSlot->GetSize().X;
		}
	}

	if (ControlsPanel != nullptr)
	{
		ControlsPanel->SetVisibility(ESlateVisibility::Visible);

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				HideControlsTimerHandle,
				this,
				&UPlayerHUDWidget::HideControlsPanel,
				5.0f,
				false
			);
		}
	}
	if (CheatNotificationBox != nullptr)
	{
		CheatNotificationBox->SetVisibility(ESlateVisibility::Collapsed);
		CheatNotificationBox->SetRenderOpacity(1.0f);
	}

	BindToCheatManager();
}

void UPlayerHUDWidget::HideControlsPanel()
{
	if (ControlsPanel != nullptr)
	{
		ControlsPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPlayerHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (health_red == nullptr)
	{
		return;
	}

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(health_red->Slot);
	if (CanvasSlot == nullptr)
	{
		return;
	}

	const float HealthPercent = GetHealthPercent();
	const float NewWidth = FullHealthBarWidth * HealthPercent;

	FVector2D CurrentSize = CanvasSlot->GetSize();
	CanvasSlot->SetSize(FVector2D(NewWidth, CurrentSize.Y));
}

ABasePaperPlayer* UPlayerHUDWidget::GetPlayerCharacter() const
{
	APawn* OwningPawn = GetOwningPlayerPawn();
	if (OwningPawn == nullptr)
	{
		return nullptr;
	}

	return Cast<ABasePaperPlayer>(OwningPawn);
}

float UPlayerHUDWidget::GetHealthPercent() const
{
	ABasePaperPlayer* PlayerCharacter = GetPlayerCharacter();
	if (PlayerCharacter == nullptr)
	{
		return 0.0f;
	}

	UHealthComponent* HealthComponent = PlayerCharacter->GetHealthComponent();
	if (HealthComponent == nullptr)
	{
		return 0.0f;
	}

	return HealthComponent->GetHealthPercent();
}

float UPlayerHUDWidget::GetCurrentHealth() const
{
	ABasePaperPlayer* PlayerCharacter = GetPlayerCharacter();
	if (PlayerCharacter == nullptr)
	{
		return 0.0f;
	}

	UHealthComponent* HealthComponent = PlayerCharacter->GetHealthComponent();
	if (HealthComponent == nullptr)
	{
		return 0.0f;
	}

	return HealthComponent->GetCurrentHealth();
}

float UPlayerHUDWidget::GetMaxHealth() const
{
	ABasePaperPlayer* PlayerCharacter = GetPlayerCharacter();
	if (PlayerCharacter == nullptr)
	{
		return 0.0f;
	}

	UHealthComponent* HealthComponent = PlayerCharacter->GetHealthComponent();
	if (HealthComponent == nullptr)
	{
		return 0.0f;
	}

	return HealthComponent->GetMaxHealth();
}

void UPlayerHUDWidget::BindToCheatManager()
{
	APlayerController* PlayerController = GetOwningPlayer();

	if (PlayerController == nullptr)
	{
		return;
	}

	UUtahniCheatManager* UtahniCheatManager = Cast<UUtahniCheatManager>(PlayerController->CheatManager);

	if (UtahniCheatManager == nullptr)
	{
		return;
	}

	UtahniCheatManager->OnCheatActivated.RemoveDynamic(this, &UPlayerHUDWidget::ShowCheatNotification);
	UtahniCheatManager->OnCheatActivated.AddDynamic(this, &UPlayerHUDWidget::ShowCheatNotification);
}

void UPlayerHUDWidget::ShowCheatNotification(FText CheatMessage)
{
	if (CheatNotificationText != nullptr)
	{
		CheatNotificationText->SetText(CheatMessage);
	}

	if (CheatNotificationBox != nullptr)
	{
		CheatNotificationBox->SetRenderOpacity(1.0f);
		CheatNotificationBox->SetVisibility(ESlateVisibility::Visible);
	}

	if (CheatNotificationFadeOut != nullptr)
	{
		StopAnimation(CheatNotificationFadeOut);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CheatNotificationFadeTimerHandle);
		World->GetTimerManager().ClearTimer(CheatNotificationHideTimerHandle);

		World->GetTimerManager().SetTimer(
			CheatNotificationFadeTimerHandle,
			this,
			&UPlayerHUDWidget::StartCheatNotificationFade,
			2.0f,
			false
		);

		World->GetTimerManager().SetTimer(
			CheatNotificationHideTimerHandle,
			this,
			&UPlayerHUDWidget::HideCheatNotification,
			3.0f,
			false
		);
	}
}

void UPlayerHUDWidget::StartCheatNotificationFade()
{
	if (CheatNotificationFadeOut != nullptr)
	{
		PlayAnimation(CheatNotificationFadeOut);
	}
	else
	{
		HideCheatNotification();
	}
}

void UPlayerHUDWidget::HideCheatNotification()
{
	if (CheatNotificationBox != nullptr)
	{
		CheatNotificationBox->SetVisibility(ESlateVisibility::Collapsed);
		CheatNotificationBox->SetRenderOpacity(1.0f);
	}
}