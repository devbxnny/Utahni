// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUps/DamagePickUp.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ADamagePickUp::ADamagePickUp()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DamageAmount = 10.0f;
	bDestroyOnPickup = false;

}

// Called when the game starts or when spawned
void ADamagePickUp::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ADamagePickUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADamagePickUp::ApplyPickupEffect_Implementation(AActor* PickingUpActor)
{
	if (!PickingUpActor)
	{
		return;
	}

	UGameplayStatics::ApplyDamage(PickingUpActor, DamageAmount, nullptr, this, UDamageType::StaticClass());
}