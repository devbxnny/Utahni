// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BasePickUp.generated.h"

class USceneComponent;
class USphereComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;
struct FHitResult;

UCLASS()
class UTAHNI_API ABasePickUp : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABasePickUp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PickupMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	bool bDestroyOnPickup;

	UFUNCTION()
	void OnPickupOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION(BlueprintNativeEvent)
	void ApplyPickupEffect(AActor* PickingUpActor);
	virtual void ApplyPickupEffect_Implementation(AActor* PickingUpActor);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};