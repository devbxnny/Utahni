// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "UtahniCheatManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCheatActivatedSignature, FText, CheatMessage);
/**
 *
 */
UCLASS()

class UTAHNI_API UUtahniCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, Category = "Cheat Spawning")
	TSubclassOf<AActor> BatEnemyClass;

	UPROPERTY(EditDefaultsOnly, Category = "Cheat Spawning")
	TSubclassOf<AActor> SlimeEnemyClass;

	UPROPERTY(EditDefaultsOnly, Category = "Cheat Spawning")
	TSubclassOf<AActor> GoblinEnemyClass;

	UPROPERTY(EditDefaultsOnly, Category = "Cheat Spawning")
	TSubclassOf<AActor> HuntressEnemyClass;

	UPROPERTY(EditDefaultsOnly, Category = "Cheat Spawning")
	TSubclassOf<AActor> MushroomEnemyClass;

	UPROPERTY(EditDefaultsOnly, Category = "Cheat Spawning")
	TSubclassOf<AActor> WarriorEnemyClass;

	UPROPERTY(EditDefaultsOnly, Category = "Cheat Spawning")
	TSubclassOf<AActor> HealthPickupClass;

	UPROPERTY(EditDefaultsOnly, Category = "Cheat Spawning")
	TSubclassOf<AActor> DamagePickupClass;

	UPROPERTY(EditDefaultsOnly, Category = "Cheat Spawning")
	TSubclassOf<AActor> KeyPickupClass;

	UPROPERTY(EditDefaultsOnly, Category = "Cheat Spawning")
	TSubclassOf<AActor> DmgAbilityClass;

	UFUNCTION(exec)
	void SpawnBat();

	UFUNCTION(exec)
	void SpawnSlime();

	UFUNCTION(exec)
	void SpawnGoblin();

	UFUNCTION(exec)
	void SpawnHuntress();

	UFUNCTION(exec)
	void SpawnMushroom();

	UFUNCTION(exec)
	void SpawnWarrior();

	UFUNCTION(exec)
	void SpawnHealth();

	UFUNCTION(exec)
	void SpawnDamage();

	UFUNCTION(exec)
	void SpawnKey();

	UFUNCTION(exec)
	void SpawnDmgAbility();

	UFUNCTION(exec)
	void NextLevel();

	UFUNCTION(exec)
	void GoToBossLevel();

	UFUNCTION(exec)
	void ShowFPS();

	UFUNCTION(exec)
	void ShowAIStates();

	UPROPERTY(BlueprintAssignable, Category = "Cheats")
	FCheatActivatedSignature OnCheatActivated;

private:

	bool bFPSVisible = false;
	bool bAIStatesVisible = false;

	void SpawnActorInFront(TSubclassOf<AActor> ActorClass, const FText& CheatMessage);
	void NotifyCheatActivated(const FText& CheatMessage);
};