// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/HintNPC.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "PaperSpriteComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Dialog/HintDialogWidget.h"

// Sets default values
AHintNPC::AHintNPC()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	NPCSprite = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("NPCSprite"));
	NPCSprite->SetupAttachment(SceneRoot);

	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	InteractionBox->SetupAttachment(SceneRoot);
	InteractionBox->SetBoxExtent(FVector(60.0f, 60.0f, 100.0f));
	InteractionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	HintMessage = TEXT("the key is hidden near the cave");
	HintDisplayTime = 3.0f;
	bTriggerOnlyOnce = true;
	bHasTriggered = false;
	ActiveHintDialogWidget = nullptr;
	CachedCharacter = nullptr;
	CachedPlayerController = nullptr;
}

// Called when the game starts or when spawned
void AHintNPC::BeginPlay()
{
	Super::BeginPlay();

	InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &AHintNPC::OnInteractionBoxBeginOverlap);

}

// Called every frame
void AHintNPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHintNPC::OnInteractionBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (bTriggerOnlyOnce && bHasTriggered)
	{
		return;
	}

	ACharacter* OverlapCharacter = Cast<ACharacter>(OtherActor);
	if (!OverlapCharacter)
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(OverlapCharacter->GetController());
	if (!PlayerController)
	{
		return;
	}

	CachedCharacter = OverlapCharacter;
	CachedPlayerController = PlayerController;
	bHasTriggered = true;

	if (CachedCharacter->GetCharacterMovement())
	{
		CachedCharacter->GetCharacterMovement()->StopMovementImmediately();
		CachedCharacter->GetCharacterMovement()->DisableMovement();
	}

	CachedPlayerController->DisableInput(CachedPlayerController);

	if (HintDialogWidgetClass && CachedPlayerController)
	{
		ActiveHintDialogWidget = CreateWidget<UHintDialogWidget>(CachedPlayerController, HintDialogWidgetClass);
		if (ActiveHintDialogWidget)
		{
			ActiveHintDialogWidget->AddToViewport();
			ActiveHintDialogWidget->SetHintMessage(HintMessage);
		}
	}

	GetWorldTimerManager().ClearTimer(RestoreControlTimer);
	GetWorldTimerManager().SetTimer(RestoreControlTimer, this, &AHintNPC::RestorePlayerControl, HintDisplayTime, false);
}

void AHintNPC::RestorePlayerControl()
{
	if (ActiveHintDialogWidget)
	{
		ActiveHintDialogWidget->RemoveFromParent();
		ActiveHintDialogWidget = nullptr;
	}

	if (CachedCharacter && CachedCharacter->GetCharacterMovement())
	{
		CachedCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	if (CachedPlayerController)
	{
		CachedPlayerController->EnableInput(CachedPlayerController);
	}
}