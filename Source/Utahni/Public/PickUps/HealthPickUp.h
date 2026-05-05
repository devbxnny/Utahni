// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickUps/BasePickUp.h"
#include "HealthPickUp.generated.h"

UCLASS()
class UTAHNI_API AHealthPickUp : public ABasePickUp
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AHealthPickUp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	float HealthAmount;

	virtual void ApplyPickupEffect_Implementation(AActor* PickingUpActor) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};