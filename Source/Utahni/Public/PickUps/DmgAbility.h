// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DmgAbility.generated.h"

class UBoxComponent;
class UPaperSpriteComponent;

UCLASS()
class UTAHNI_API ADmgAbility : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADmgAbility();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* Collision;

	UPROPERTY(VisibleAnywhere)
	UPaperSpriteComponent* Sprite;

	UFUNCTION()
	void HandleOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};