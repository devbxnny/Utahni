// Fill out your copyright notice in the Description page of Project Settings.

#include "Cheats/UtahniCheatPlayerController.h"
#include "Cheats/UtahniCheatManager.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/CheatManager.h"

AUtahniCheatPlayerController::AUtahniCheatPlayerController()
{
	static ConstructorHelpers::FClassFinder<UCheatManager> CheatManagerBPClass(TEXT("/Game/Blueprint/BP_UtahniCheatManager"));

	if (CheatManagerBPClass.Succeeded())
	{
		CheatClass = CheatManagerBPClass.Class;
	}
	else
	{
		CheatClass = UUtahniCheatManager::StaticClass();
	}
}

void AUtahniCheatPlayerController::BeginPlay()
{
	Super::BeginPlay();

	EnableCheats();
}