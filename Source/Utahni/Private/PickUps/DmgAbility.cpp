// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUps/DmgAbility.h"
#include "BasePaperPlayer.h"
#include "Components/BoxComponent.h"
#include "PaperSpriteComponent.h"

// Sets default values
ADmgAbility::ADmgAbility()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	RootComponent = Collision;

	Collision->SetBoxExtent(FVector(40.0f, 20.0f, 40.0f));
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Collision->SetGenerateOverlapEvents(true);

	Sprite = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Sprite"));
	Sprite->SetupAttachment(RootComponent);
	Sprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void ADmgAbility::BeginPlay()
{
	Super::BeginPlay();

	if (Collision)
	{
		Collision->OnComponentBeginOverlap.AddDynamic(this, &ADmgAbility::HandleOverlap);
	}
}

// Called every frame
void ADmgAbility::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADmgAbility::HandleOverlap(
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

	Player->Activate100DamageBoost();
	Destroy();
}