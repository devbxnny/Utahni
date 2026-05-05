#include "UI/GameOverWidget.h"
#include "Components/Button.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "BasePaperPlayer.h"
#include "Components/Widget.h"
#include "Blueprint/UserWidget.h"

void UGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Resume != nullptr)
	{
		Resume->OnClicked.Clear();
		Resume->OnClicked.AddDynamic(this, &UGameOverWidget::HandleResumeClicked);
	}

	if (MainMenu != nullptr)
	{
		MainMenu->OnClicked.Clear();
		MainMenu->OnClicked.AddDynamic(this, &UGameOverWidget::HandleMainMenuClicked);
	}

	if (CheatsButton != nullptr)
	{
		CheatsButton->OnClicked.Clear();
		CheatsButton->OnClicked.AddDynamic(this, &UGameOverWidget::HandleCheatsClicked);
	}

	if (BackFromCheatsButton != nullptr)
	{
		BackFromCheatsButton->OnClicked.Clear();
		BackFromCheatsButton->OnClicked.AddDynamic(this, &UGameOverWidget::HandleBackFromCheatsClicked);
	}

	if (CheatsPanel != nullptr)
	{
		CheatsPanel->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (SettingsButton != nullptr)
	{
		SettingsButton->OnClicked.Clear();
		SettingsButton->OnClicked.AddDynamic(this, &UGameOverWidget::HandleSettingsClicked);
	}
}

void UGameOverWidget::SetMainMenuLevelName(const FName& InLevelName)
{
	MainMenuLevelName = InLevelName;
}

void UGameOverWidget::HandleResumeClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Resume clicked"));

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Resume failed: PlayerController is null"));
		return;
	}

	ABasePaperPlayer* Player = Cast<ABasePaperPlayer>(PlayerController->GetPawn());
	if (Player == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Resume failed: controlled pawn is not ABasePaperPlayer"));
		return;
	}

	Player->ResumeGame();
}

void UGameOverWidget::HandleMainMenuClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("MainMenu clicked"));

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("MainMenu failed: PlayerController is null"));
		return;
	}

	ABasePaperPlayer* Player = Cast<ABasePaperPlayer>(PlayerController->GetPawn());
	if (Player == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("MainMenu failed: controlled pawn is not ABasePaperPlayer"));
		return;
	}

	Player->OpenMainMenu();
}

void UGameOverWidget::HandleCheatsClicked()
{
	if (CheatsPanel != nullptr)
	{
		CheatsPanel->SetVisibility(ESlateVisibility::Visible);
	}
}

void UGameOverWidget::HandleBackFromCheatsClicked()
{
	if (CheatsPanel != nullptr)
	{
		CheatsPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UGameOverWidget::HandleSettingsClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Settings clicked"));

	if (SettingsWidgetClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Settings failed: SettingsWidgetClass is not assigned"));
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Settings failed: PlayerController is null"));
		return;
	}

	UUserWidget* SettingsWidget = CreateWidget<UUserWidget>(PlayerController, SettingsWidgetClass);
	if (SettingsWidget == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Settings failed: could not create settings widget"));
		return;
	}

	SettingsWidget->AddToViewport(20);
	//set input mode game ui (per jesse)

	PlayerController->bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(SettingsWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
}