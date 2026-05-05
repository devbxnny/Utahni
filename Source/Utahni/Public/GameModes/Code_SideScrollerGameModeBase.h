//----- Code_SideScrollerGameModeBase.h START -----
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Code_SideScrollerGameModeBase.generated.h"

UCLASS()
class UTAHNI_API ACode_SideScrollerGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACode_SideScrollerGameModeBase();

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
	bool bHasTriggeredWin = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game")
	float WinDelay = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game")
	TArray<FName> LevelOrder;

	void HandleWin();
	FName GetNextLevelName() const;

	FTimerHandle WinTimerHandle;
};

//----- Code_SideScrollerGameModeBase.h END -----