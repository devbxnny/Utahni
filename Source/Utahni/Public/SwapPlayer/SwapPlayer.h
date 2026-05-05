// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "SwapPlayer.generated.h"

UCLASS()
class UTAHNI_API ASwapPlayer : public APaperCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASwapPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swap")
	ASwapPlayer* PartnerCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swap")
	bool bIsCurrentlyControlled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
	float FollowDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
	float TeleportDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
	float MoveAcceptanceRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swap")
	float FacingDirection;

	UFUNCTION(BlueprintCallable)
	void SetPartner(ASwapPlayer* NewPartner);

	void StartFollowing();
	void StopFollowing();
	void UpdateFollow();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};