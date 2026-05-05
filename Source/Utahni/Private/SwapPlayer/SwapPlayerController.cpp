// Fill out your copyright notice in the Description page of Project Settings.

#include "SwapPlayer/SwapPlayerController.h"
#include "SwapPlayer/SwapPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

void ASwapPlayerController::BeginPlay()
{
	Super::BeginPlay();
	InitializeCharacters();
}

void ASwapPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent)
	{
		InputComponent->BindAction("SwitchCharacter", IE_Pressed, this, &ASwapPlayerController::SwitchCharacters);
	}
}

void ASwapPlayerController::InitializeCharacters()
{
	if (!CharacterA || !CharacterB)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASwapPlayer::StaticClass(), FoundActors);
		if (FoundActors.Num() >= 2)
		{
			CharacterA = Cast<ASwapPlayer>(FoundActors[0]);
			CharacterB = Cast<ASwapPlayer>(FoundActors[1]);
		}
	}

	if (!CharacterA || !CharacterB)
	{
		return;
	}

	CharacterA->SetPartner(CharacterB);
	CharacterB->SetPartner(CharacterA);

	Possess(CharacterA);
	CharacterA->StopFollowing();
	CharacterB->StartFollowing();
}

void ASwapPlayerController::SwitchCharacters()
{
	ASwapPlayer* CurrentCharacter = Cast<ASwapPlayer>(GetPawn());
	if (!CurrentCharacter)
	{
		return;
	}

	ASwapPlayer* NewCharacter = CurrentCharacter->PartnerCharacter;
	if (!NewCharacter)
	{
		return;
	}

	float SavedFacingDirection = CurrentCharacter->FacingDirection;

	if (CurrentCharacter->GetCharacterMovement())
	{
		FVector NewVelocity = CurrentCharacter->GetCharacterMovement()->Velocity;
		NewVelocity.X = 0.0f;
		CurrentCharacter->GetCharacterMovement()->Velocity = NewVelocity;
	}

	if (NewCharacter->GetCharacterMovement())
	{
		FVector NewVelocity = NewCharacter->GetCharacterMovement()->Velocity;
		NewVelocity.X = 0.0f;
		NewCharacter->GetCharacterMovement()->Velocity = NewVelocity;
	}

	CurrentCharacter->StartFollowing();
	Possess(NewCharacter);
	NewCharacter->FacingDirection = SavedFacingDirection;
	CurrentCharacter->FacingDirection = SavedFacingDirection;
	NewCharacter->StopFollowing();
}