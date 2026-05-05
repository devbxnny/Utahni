// Fill out your copyright notice in the Description page of Project Settings.

#include "SwapPlayer/SwapPlayer.h"
#include "PaperFlipbookComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ASwapPlayer::ASwapPlayer()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PartnerCharacter = nullptr;
	bIsCurrentlyControlled = false;

	FollowDistance = 150.0f;
	TeleportDistance = 1000.0f;
	MoveAcceptanceRadius = 75.0f;
	FacingDirection = 0.0f;   // 0 = right, 180 = left
}

// Called when the game starts or when spawned
void ASwapPlayer::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ASwapPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsCurrentlyControlled)
	{
		UpdateFollow();
	}
}

void ASwapPlayer::SetPartner(ASwapPlayer* NewPartner)
{
	PartnerCharacter = NewPartner;
}

void ASwapPlayer::StartFollowing()
{
	bIsCurrentlyControlled = false;

	if (GetCharacterMovement())
	{
		FVector NewVelocity = GetCharacterMovement()->Velocity;
		NewVelocity.X = 0.0f;
		GetCharacterMovement()->Velocity = NewVelocity;
	}
}

void ASwapPlayer::StopFollowing()
{
	bIsCurrentlyControlled = true;

	if (GetCharacterMovement())
	{
		FVector NewVelocity = GetCharacterMovement()->Velocity;
		NewVelocity.X = 0.0f;
		GetCharacterMovement()->Velocity = NewVelocity;
	}
}

void ASwapPlayer::UpdateFollow()
{
	if (!PartnerCharacter)
	{
		return;
	}

	if (PartnerCharacter == this)
	{
		return;
	}

	UCharacterMovementComponent* MyMovement = GetCharacterMovement();
	UCharacterMovementComponent* PartnerMovement = PartnerCharacter->GetCharacterMovement();

	FVector PartnerLocation = PartnerCharacter->GetActorLocation();
	FVector MyLocation = GetActorLocation();

	float XDifference = PartnerLocation.X - MyLocation.X;
	float AbsXDifference = FMath::Abs(XDifference);

	if (GetSprite())
	{
		if (XDifference > 0.0f)
		{
			FacingDirection = 0.0f; // face right
			GetSprite()->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
		}
		else if (XDifference < 0.0f)
		{
			FacingDirection = 180.0f; // face left
			GetSprite()->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
		}
	}

	if (MyMovement && PartnerMovement)
	{
		if ((PartnerMovement->IsFalling() || PartnerMovement->Velocity.Z > 0.0f) && !MyMovement->IsFalling())
		{
			LaunchCharacter(FVector(0.0f, 0.0f, MyMovement->JumpZVelocity), false, true);
		}
	}

	if (AbsXDifference > TeleportDistance)
	{
		FVector NewLocation = GetActorLocation();
		NewLocation.X = PartnerLocation.X - (FMath::Sign(XDifference) * FollowDistance);
		SetActorLocation(NewLocation);

		if (MyMovement)
		{
			FVector NewVelocity = MyMovement->Velocity;
			NewVelocity.X = 0.0f;
			MyMovement->Velocity = NewVelocity;
		}

		return;
	}

	if (AbsXDifference > FollowDistance)
	{
		FVector NewLocation = GetActorLocation();
		float MoveSpeed = 300.0f;
		float Direction = FMath::Sign(XDifference);

		NewLocation.X += Direction * MoveSpeed * GetWorld()->GetDeltaSeconds();
		SetActorLocation(NewLocation);

		if (MyMovement)
		{
			FVector NewVelocity = MyMovement->Velocity;
			NewVelocity.X = Direction * MoveSpeed;
			MyMovement->Velocity = NewVelocity;
		}
	}
	else
	{
		if (MyMovement)
		{
			FVector NewVelocity = MyMovement->Velocity;
			NewVelocity.X = 0.0f;
			MyMovement->Velocity = NewVelocity;
		}
	}
}

// Called to bind functionality to input
void ASwapPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}