// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseRespawnPoint.generated.h"

class UBoxComponent;
class UPaperSprite;
class UPaperSpriteComponent;
class USceneComponent;

UCLASS()
class UTAHNI_API ABaseRespawnPoint : public AActor
{
	GENERATED_BODY()

public:
	ABaseRespawnPoint();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Respawn")
	TObjectPtr<USceneComponent> SceneRoot = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Respawn")
	TObjectPtr<UBoxComponent> TriggerBox = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Respawn")
	TObjectPtr<USceneComponent> RespawnLocation = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Respawn")
	TObjectPtr<UPaperSpriteComponent> SpriteComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Respawn")
	TObjectPtr<UPaperSprite> CheckpointSprite = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Respawn")
	FVector SpriteOffset = FVector::ZeroVector;

	void ApplySpriteSettings();

	UFUNCTION()
	void HandleTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};