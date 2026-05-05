#include "BaseMushroomProjectile.h"
#include "Components/SphereComponent.h"
#include "PaperSpriteComponent.h"
#include "PaperFlipbookComponent.h"
#include "PaperSprite.h"
#include "PaperFlipbook.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"



ABaseMushroomProjectile::ABaseMushroomProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);

	CollisionComponent->InitSphereRadius(16.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetMobility(EComponentMobility::Movable);

	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
	SpriteComponent->SetupAttachment(RootComponent);
	SpriteComponent->SetMobility(EComponentMobility::Movable);

	FlipbookComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("FlipbookComponent"));
	FlipbookComponent->SetupAttachment(RootComponent);
	FlipbookComponent->SetMobility(EComponentMobility::Movable);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 600.0f;
	ProjectileMovement->MaxSpeed = 1200.0f;
	ProjectileMovement->ProjectileGravityScale = 1.0f;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->bAutoActivate = false;

	bHasImpacted = false;
}

void ABaseMushroomProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ABaseMushroomProjectile::OnProjectileOverlap);

	if (SpriteComponent && TravelSprite)
	{
		SpriteComponent->SetSprite(TravelSprite);
		SpriteComponent->SetVisibility(true);
	}

	if (FlipbookComponent)
	{
		FlipbookComponent->SetVisibility(false);

		if (ImpactFlipbook)
		{
			FlipbookComponent->SetFlipbook(ImpactFlipbook);
			FlipbookComponent->SetLooping(false);
			FlipbookComponent->Stop();
		}
	}
}

void ABaseMushroomProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseMushroomProjectile::LaunchProjectile(const FVector& LaunchVelocity)
{
	if (CollisionComponent && GetOwner())
	{
		CollisionComponent->IgnoreActorWhenMoving(GetOwner(), true);
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = LaunchVelocity;
		ProjectileMovement->Activate(true);
	}
}

void ABaseMushroomProjectile::OnProjectileOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bHasImpacted || !OtherActor)
	{
		return;
	}

	// ignore self, owner, and instigator
	if (OtherActor == this || OtherActor == GetOwner() || OtherActor == GetInstigator())
	{
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
	{
		return;
	}

	// only explode when hitting the active player
	if (OtherActor != PlayerPawn)
	{
		return;
	}

	UGameplayStatics::ApplyDamage(
		OtherActor,
		Damage,
		GetInstigatorController(),
		this,
		UDamageType::StaticClass()
	);

	StartImpact();
}

void ABaseMushroomProjectile::StartImpact()
{
	if (bHasImpacted)
	{
		return;
	}

	bHasImpacted = true;

	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}

	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (SpriteComponent)
	{
		SpriteComponent->SetVisibility(false);
	}

	if (FlipbookComponent && ImpactFlipbook)
	{
		FlipbookComponent->SetVisibility(true);
		FlipbookComponent->SetFlipbook(ImpactFlipbook);
		FlipbookComponent->SetLooping(false);
		FlipbookComponent->PlayFromStart();
	}

	SetLifeSpan(ImpactDestroyDelay);
}