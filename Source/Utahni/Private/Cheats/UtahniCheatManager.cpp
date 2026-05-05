// Fill out your copyright notice in the Description page of Project Settings.

#include "Cheats/UtahniCheatManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameModes/Code_SideScrollerGameModeBase.h"
#include "Kismet/GameplayStatics.h"

void UUtahniCheatManager::SpawnBat()
{
	SpawnActorInFront(BatEnemyClass, FText::FromString("Bat Spawned"));
}

void UUtahniCheatManager::SpawnSlime()
{
	SpawnActorInFront(SlimeEnemyClass, FText::FromString("Slime Spawned"));
}

void UUtahniCheatManager::SpawnGoblin()
{
	SpawnActorInFront(GoblinEnemyClass, FText::FromString("Goblin Spawned"));
}

void UUtahniCheatManager::SpawnHuntress()
{
	SpawnActorInFront(HuntressEnemyClass, FText::FromString("Huntress Spawned"));
}

void UUtahniCheatManager::SpawnMushroom()
{
	SpawnActorInFront(MushroomEnemyClass, FText::FromString("Mushroom Spawned"));
}

void UUtahniCheatManager::SpawnWarrior()
{
	SpawnActorInFront(WarriorEnemyClass, FText::FromString("Warrior Spawned"));
}

void UUtahniCheatManager::SpawnHealth()
{
	SpawnActorInFront(HealthPickupClass, FText::FromString("Health Pickup Spawned"));
}

void UUtahniCheatManager::SpawnDamage()
{
	SpawnActorInFront(DamagePickupClass, FText::FromString("Damage Pickup Spawned"));
}

void UUtahniCheatManager::SpawnKey()
{
	SpawnActorInFront(KeyPickupClass, FText::FromString("Key Spawned"));
}

void UUtahniCheatManager::SpawnDmgAbility()
{
	SpawnActorInFront(DmgAbilityClass, FText::FromString("Damage Ability Spawned"));
}

void UUtahniCheatManager::NextLevel()
{
	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	ACode_SideScrollerGameModeBase* GameMode = Cast<ACode_SideScrollerGameModeBase>(UGameplayStatics::GetGameMode(World));

	if (!GameMode)
	{
		return;
	}

	GameMode->SetHasCollectedKey(true);

	while (true)
	{
		GameMode->UnregisterEnemy();

		if (GameMode->HasCollectedKey())
		{
			if (GameMode)
			{
				break;
			}
		}
	}

	GameMode->CheckWinCondition();
	NotifyCheatActivated(FText::FromString("Next Level Cheat Activated"));
}

void UUtahniCheatManager::GoToBossLevel()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	NotifyCheatActivated(FText::FromString("Boss Level Cheat Activated"));

	static const FName BossLevelName(TEXT("NVHUYNH_BossArena"));
	UGameplayStatics::OpenLevel(World, BossLevelName);
}

void UUtahniCheatManager::ShowFPS()
{
	if (!GetWorld() || !GEngine)
	{
		return;
	}

	bFPSVisible = !bFPSVisible;

	if (bFPSVisible)
	{
		GEngine->Exec(GetWorld(), TEXT("stat fps"));
		NotifyCheatActivated(FText::FromString("FPS Display Enabled"));
	}
	else
	{
		GEngine->Exec(GetWorld(), TEXT("stat none"));
		NotifyCheatActivated(FText::FromString("FPS Display Disabled"));
	}
}

void UUtahniCheatManager::ShowAIStates()
{
	if (!GetWorld() || !GEngine)
	{
		return;
	}

	bAIStatesVisible = !bAIStatesVisible;

	if (bAIStatesVisible)
	{
		GEngine->Exec(GetWorld(), TEXT("showdebug ai"));

		GEngine->AddOnScreenDebugMessage(
			-1,
			3.0f,
			FColor::Green,
			TEXT("AI State Debug ON")
		);

		NotifyCheatActivated(FText::FromString("AI State Debug Enabled"));
	}
	else
	{
		GEngine->Exec(GetWorld(), TEXT("showdebug none"));

		GEngine->AddOnScreenDebugMessage(
			-1,
			3.0f,
			FColor::Red,
			TEXT("AI State Debug OFF")
		);

		NotifyCheatActivated(FText::FromString("AI State Debug Disabled"));
	}
}

void UUtahniCheatManager::SpawnActorInFront(TSubclassOf<AActor> ActorClass, const FText& CheatMessage)
{
	if (!ActorClass)
	{
		NotifyCheatActivated(FText::FromString("Cheat spawn class is not assigned."));
		return;
	}

	APlayerController* PlayerController = GetOuterAPlayerController();

	if (!PlayerController)
	{
		return;
	}

	APawn* PlayerPawn = PlayerController->GetPawn();

	if (!PlayerPawn)
	{
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	FVector SpawnLocation = PlayerPawn->GetActorLocation() + PlayerPawn->GetActorForwardVector() * 300.0f;
	SpawnLocation.Z += 50.0f;

	FRotator SpawnRotation = PlayerPawn->GetActorRotation();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = PlayerPawn;
	SpawnParameters.Instigator = PlayerPawn;

	AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClass, SpawnLocation, SpawnRotation, SpawnParameters);

	if (SpawnedActor)
	{
		NotifyCheatActivated(CheatMessage);
	}
}

void UUtahniCheatManager::NotifyCheatActivated(const FText& CheatMessage)
{
	OnCheatActivated.Broadcast(CheatMessage);
}