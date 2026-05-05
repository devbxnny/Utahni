// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Code_SideScrollerGameMode.generated.h"

/**
 *
 */
UCLASS()
class UTAHNI_API ACode_SideScrollerGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ACode_SideScrollerGameMode();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void RegisterEnemy();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void UnregisterEnemy();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void SetHasCollectedKey(bool bNewHasCollectedKey);

	UFUNCTION(BlueprintPure, Category = "Game")
	bool HasCollectedKey() const;

	UFUNCTION(BlueprintCallable, Category = "Game")
	void CheckWinCondition();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
	int32 AliveEnemyCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
	bool bHasCollectedKey = false;
};