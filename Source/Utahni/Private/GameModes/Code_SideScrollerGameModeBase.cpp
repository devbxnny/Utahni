//----- Code_SideScrollerGameModeBase.cpp START -----

#include "GameModes/Code_SideScrollerGameModeBase.h"
#include "Cheats/UtahniCheatPlayerController.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "JSAdams/GameInstances/GI_UtahniGameInstanceCode.h"

ACode_SideScrollerGameModeBase::ACode_SideScrollerGameModeBase()
{
	PlayerControllerClass = AUtahniCheatPlayerController::StaticClass();

	LevelOrder =
	{
		TEXT("NVHuynh_GrassMap"),
		TEXT("TestMap"),
		TEXT("Dungeon"),
		TEXT("NVHUYNH_BossArena")
	};
}

void ACode_SideScrollerGameModeBase::RegisterEnemy()
{
	AliveEnemyCount++;
}

void ACode_SideScrollerGameModeBase::UnregisterEnemy()
{
	AliveEnemyCount = FMath::Max(0, AliveEnemyCount - 1);
	CheckWinCondition();
}

void ACode_SideScrollerGameModeBase::SetHasCollectedKey(bool bNewHasCollectedKey)
{
	bHasCollectedKey = bNewHasCollectedKey;
}

bool ACode_SideScrollerGameModeBase::HasCollectedKey() const
{
	return bHasCollectedKey;
}

void ACode_SideScrollerGameModeBase::CheckWinCondition()
{
	if (bHasTriggeredWin)
	{
		return;
	}

	if (AliveEnemyCount == 0 && bHasCollectedKey)
	{
		bHasTriggeredWin = true;

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, WinDelay, FColor::Yellow, TEXT("YOU WIN"));
		}

		GetWorldTimerManager().SetTimer(WinTimerHandle, this, &ACode_SideScrollerGameModeBase::HandleWin, WinDelay, false);
	}
}

FName ACode_SideScrollerGameModeBase::GetNextLevelName() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return NAME_None;
	}

	const FName CurrentLevelName = FName(*UGameplayStatics::GetCurrentLevelName(World, true));
	if (GEngine)
{
	GEngine->AddOnScreenDebugMessage(
		-1,
		5.0f,
		FColor::Orange,
		FString::Printf(TEXT("CURRENT MAP IS: %s"), *CurrentLevelName.ToString())
	);
}
	for (int32 Index = 0; Index < LevelOrder.Num(); ++Index)
	{
		if (LevelOrder[Index] == CurrentLevelName)
		{
			const int32 NextIndex = Index + 1;
			return LevelOrder.IsValidIndex(NextIndex) ? LevelOrder[NextIndex] : NAME_None;
		}
	}

	return NAME_None;
}

void ACode_SideScrollerGameModeBase::HandleWin()
{
	const FName NextLevel = GetNextLevelName();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.0f,
			FColor::Cyan,
			FString::Printf(TEXT("NEXT MAP IS: %s"), *NextLevel.ToString())
		);
	}

	if (NextLevel != NAME_None)
	{
		if (UGI_UtahniGameInstanceCode* UtahniGI = Cast<UGI_UtahniGameInstanceCode>(GetGameInstance()))
		{
			UtahniGI->SaveCurrentLevel(NextLevel.ToString());
		}

		UGameplayStatics::OpenLevel(this, NextLevel);
		return;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("FINAL LEVEL COMPLETE"));
	}
}

//----- Code_SideScrollerGameModeBase.cpp END -----