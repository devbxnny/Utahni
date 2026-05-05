// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HintNPC.generated.h"

class UBoxComponent;
class USceneComponent;
class UPaperSpriteComponent;
class ACharacter;
class APlayerController;
class UHintDialogWidget;

UCLASS()
class UTAHNI_API AHintNPC : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AHintNPC();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC")
	UPaperSpriteComponent* NPCSprite;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC")
	UBoxComponent* InteractionBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	FString HintMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	float HintDisplayTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	bool bTriggerOnlyOnce;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC")
	bool bHasTriggered;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	TSubclassOf<UHintDialogWidget> HintDialogWidgetClass;

	UPROPERTY()
	UHintDialogWidget* ActiveHintDialogWidget;

	UPROPERTY()
	ACharacter* CachedCharacter;

	UPROPERTY()
	APlayerController* CachedPlayerController;

	UFUNCTION()
	void OnInteractionBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	void RestorePlayerControl();

	FTimerHandle RestoreControlTimer;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};