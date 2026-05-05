// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/Code_SideScrollerGameMode.h"
#include "Engine/Engine.h"

ACode_SideScrollerGameMode::ACode_SideScrollerGameMode()
{
}

void ACode_SideScrollerGameMode::RegisterEnemy()
{
	AliveEnemyCount++;
}

void ACode_SideScrollerGameMode::UnregisterEnemy()
{
	AliveEnemyCount = FMath::Max(0, AliveEnemyCount - 1);
	CheckWinCondition();
}

void ACode_SideScrollerGameMode::SetHasCollectedKey(bool bNewHasCollectedKey)
{
	bHasCollectedKey = bNewHasCollectedKey;
}

bool ACode_SideScrollerGameMode::HasCollectedKey() const
{
	return bHasCollectedKey;
}

void ACode_SideScrollerGameMode::CheckWinCondition()
{
	if (AliveEnemyCount == 0 && bHasCollectedKey)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				5.0f,
				FColor::Green,
				TEXT("You Win!")
			);
		}
	}
}