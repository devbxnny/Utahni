// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUps/HealthPickUp.h"

// Sets default values
AHealthPickUp::AHealthPickUp()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HealthAmount = 25.0f;

}

// Called when the game starts or when spawned
void AHealthPickUp::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AHealthPickUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHealthPickUp::ApplyPickupEffect_Implementation(AActor* PickingUpActor)
{
	if (!PickingUpActor)
	{
		return;
	}
}