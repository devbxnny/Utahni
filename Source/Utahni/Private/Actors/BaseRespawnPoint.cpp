
#include "Actors/BaseRespawnPoint.h"
#include "BasePaperPlayer.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "PaperSprite.h"
#include "PaperSpriteComponent.h"

ABaseRespawnPoint::ABaseRespawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(SceneRoot);
	TriggerBox->SetBoxExtent(FVector(64.0f, 64.0f, 64.0f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	TriggerBox->SetGenerateOverlapEvents(true);

	RespawnLocation = CreateDefaultSubobject<USceneComponent>(TEXT("RespawnLocation"));
	RespawnLocation->SetupAttachment(SceneRoot);


	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
	SpriteComponent->SetupAttachment(SceneRoot);
	SpriteComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpriteComponent->SetGenerateOverlapEvents(false);
}

void ABaseRespawnPoint::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ABaseRespawnPoint::HandleTriggerBeginOverlap);
}

void ABaseRespawnPoint::ApplySpriteSettings()
{
	if (!SpriteComponent)
	{
		return;
	}

	SpriteComponent->SetRelativeLocation(SpriteOffset);
	SpriteComponent->SetSprite(CheckpointSprite);
	SpriteComponent->SetVisibility(CheckpointSprite != nullptr);
}

void ABaseRespawnPoint::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	ABasePaperPlayer* Player = Cast<ABasePaperPlayer>(OtherActor);
	if (!Player)
	{
		return;
	}

	Player->SetRespawnTransform(RespawnLocation->GetComponentTransform());
}