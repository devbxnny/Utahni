// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SwapPlayerController.generated.h"

class ASwapPlayer;

UCLASS()
class UTAHNI_API ASwapPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

protected:
	virtual void SetupInputComponent() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swap")
	ASwapPlayer* CharacterA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swap")
	ASwapPlayer* CharacterB;

	UFUNCTION()
	void SwitchCharacters();

	void InitializeCharacters();
};