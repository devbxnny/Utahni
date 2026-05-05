// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUps/KeyPickUp.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "BasePaperPlayer.h"
#include "GameModes/Code_SideScrollerGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "PaperSpriteComponent.h"

// Sets default values
AKeyPickUp::AKeyPickUp()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	KeyMesh = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("KeyMesh"));
	KeyMesh->SetupAttachment(SceneRoot);
	KeyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetupAttachment(SceneRoot);
	CollisionSphere->SetSphereRadius(80.0f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

// Called when the game starts or when spawned
void AKeyPickUp::BeginPlay()
{
	Super::BeginPlay();

	if (CollisionSphere)
	{
		CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AKeyPickUp::HandleOverlapBegin);
	}

}

// Called every frame
void AKeyPickUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AKeyPickUp::HandleOverlapBegin(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	ABasePaperPlayer* Player = Cast<ABasePaperPlayer>(OtherActor);
	if (!Player)
	{
		return;
	}

	ACode_SideScrollerGameModeBase* GameMode = Cast<ACode_SideScrollerGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->SetHasCollectedKey(true);
		GameMode->CheckWinCondition();
	}

	Destroy();
}