// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUps/MushroomPickUp.h"
#include "BasePaperPlayer.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AMushroomPickUp::AMushroomPickUp()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	RootComponent = SphereCollision;

	SphereCollision->InitSphereRadius(60.0f);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereCollision->SetGenerateOverlapEvents(true);

	MushroomMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MushroomMesh"));
	MushroomMesh->SetupAttachment(RootComponent);
	MushroomMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void AMushroomPickUp::BeginPlay()
{
	Super::BeginPlay();

	if (SphereCollision)
	{
		SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AMushroomPickUp::OnOverlapBegin);
	}
}

// Called every frame
void AMushroomPickUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMushroomPickUp::OnOverlapBegin(
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

	Player->ActivateMushroom3DForm();
	Destroy();
}