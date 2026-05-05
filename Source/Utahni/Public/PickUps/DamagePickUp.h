// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickUps/BasePickUp.h"
#include "DamagePickUp.generated.h"

UCLASS()
class UTAHNI_API ADamagePickUp : public ABasePickUp
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADamagePickUp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	float DamageAmount;

	virtual void ApplyPickupEffect_Implementation(AActor* PickingUpActor) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};