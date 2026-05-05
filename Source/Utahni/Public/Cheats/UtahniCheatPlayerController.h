// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UtahniCheatPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class UTAHNI_API AUtahniCheatPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	AUtahniCheatPlayerController();

protected:

	virtual void BeginPlay() override;
};
