#include "Actors/BaseGoblinBombProjectile.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PaperFlipbook.h"
#include "PaperFlipbookComponent.h"
#include "PaperSprite.h"
#include "PaperSpriteComponent.h"

ABaseGoblinBombProjectile::ABaseGoblinBombProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);

	CollisionComponent->InitSphereRadius(16.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CollisionComponent->SetNotifyRigidBodyCollision(true);
	CollisionComponent->SetGenerateOverlapEvents(false);
	CollisionComponent->SetMobility(EComponentMobility::Movable);

	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
	SpriteComponent->SetupAttachment(RootComponent);
	SpriteComponent->SetMobility(EComponentMobility::Movable);

	FlipbookComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("FlipbookComponent"));
	FlipbookComponent->SetupAttachment(RootComponent);
	FlipbookComponent->SetMobility(EComponentMobility::Movable);

	ExplosionHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("ExplosionHitbox"));
	ExplosionHitbox->SetupAttachment(RootComponent);
	ExplosionHitbox->SetBoxExtent(ExplosionHitboxExtent);
	ExplosionHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ExplosionHitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
	ExplosionHitbox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ExplosionHitbox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	ExplosionHitbox->SetGenerateOverlapEvents(true);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 700.0f;
	ProjectileMovement->MaxSpeed = 1200.0f;
	ProjectileMovement->ProjectileGravityScale = 1.0f;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->bAutoActivate = false;

	bHasImpacted = false;
}

void ABaseGoblinBombProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionComponent->OnComponentHit.AddDynamic(this, &ABaseGoblinBombProjectile::OnProjectileHit);
	ExplosionHitbox->OnComponentBeginOverlap.AddDynamic(this, &ABaseGoblinBombProjectile::HandleExplosionHitboxBeginOverlap);

	UpdateExplosionHitboxTransform();
	ExplosionHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (SpriteComponent && TravelSprite)
	{
		SpriteComponent->SetSprite(TravelSprite);
		SpriteComponent->SetVisibility(true);
	}
	else if (SpriteComponent)
	{
		SpriteComponent->SetVisibility(false);
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

void ABaseGoblinBombProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseGoblinBombProjectile::LaunchProjectile(const FVector& LaunchVelocity)
{
	if (CollisionComponent && GetOwner())
	{
		CollisionComponent->IgnoreActorWhenMoving(GetOwner(), true);
	}

	if (CollisionComponent && GetInstigator())
	{
		CollisionComponent->IgnoreActorWhenMoving(GetInstigator(), true);
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = LaunchVelocity;
		ProjectileMovement->Activate(true);
	}
}

void ABaseGoblinBombProjectile::OnProjectileHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit
)
{
	if (bHasImpacted || !OtherActor)
	{
		return;
	}

	if (OtherActor == this || OtherActor == GetOwner() || OtherActor == GetInstigator())
	{
		return;
	}

	StartImpact();
}

void ABaseGoblinBombProjectile::HandleExplosionHitboxBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	ApplyExplosionDamageToActor(OtherActor);
}

void ABaseGoblinBombProjectile::StartImpact()
{
	if (bHasImpacted)
	{
		return;
	}

	bHasImpacted = true;
	HitActorsThisExplosion.Reset();

	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}

	if (CollisionComponent)
	{
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

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

	GetWorldTimerManager().ClearTimer(ExplosionHitboxStartTimerHandle);
	GetWorldTimerManager().ClearTimer(ExplosionHitboxEndTimerHandle);

	const float HitboxStartTime = FMath::Max(0.0f, ExplosionHitboxStartTime);
	const float HitboxEndTime = FMath::Max(HitboxStartTime, ExplosionHitboxEndTime);

	if (HitboxStartTime <= KINDA_SMALL_NUMBER)
	{
		EnableExplosionHitbox();
	}
	else
	{
		GetWorldTimerManager().SetTimer(
			ExplosionHitboxStartTimerHandle,
			this,
			&ABaseGoblinBombProjectile::EnableExplosionHitbox,
			HitboxStartTime,
			false
		);
	}

	if (HitboxEndTime > KINDA_SMALL_NUMBER)
	{
		GetWorldTimerManager().SetTimer(
			ExplosionHitboxEndTimerHandle,
			this,
			&ABaseGoblinBombProjectile::DisableExplosionHitbox,
			HitboxEndTime,
			false
		);
	}

	SetLifeSpan(ImpactDestroyDelay);
}

void ABaseGoblinBombProjectile::EnableExplosionHitbox()
{
	if (!ExplosionHitbox || !bHasImpacted)
	{
		return;
	}

	UpdateExplosionHitboxTransform();
	ExplosionHitbox->SetBoxExtent(ExplosionHitboxExtent);
	ExplosionHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ExplosionHitbox->UpdateOverlaps();

	TArray<AActor*> OverlappingActors;
	ExplosionHitbox->GetOverlappingActors(OverlappingActors);

	for (AActor* OverlappingActor : OverlappingActors)
	{
		ApplyExplosionDamageToActor(OverlappingActor);
	}
}

void ABaseGoblinBombProjectile::DisableExplosionHitbox()
{
	if (ExplosionHitbox)
	{
		ExplosionHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ABaseGoblinBombProjectile::ApplyExplosionDamageToActor(AActor* OtherActor)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner() || OtherActor == GetInstigator())
	{
		return;
	}

	const TWeakObjectPtr<AActor> OtherActorPtr(OtherActor);
	if (HitActorsThisExplosion.Contains(OtherActorPtr))
	{
		return;
	}

	HitActorsThisExplosion.Add(OtherActorPtr);

	if (ExplosionDamage <= 0.0f)
	{
		return;
	}

	const TSubclassOf<UDamageType> ActualDamageType =
		DamageTypeClass ? DamageTypeClass : TSubclassOf<UDamageType>(UDamageType::StaticClass());

	UE_LOG(LogTemp, Warning, TEXT("Bomb explosion damaged: %s"), *GetNameSafe(OtherActor));

	UGameplayStatics::ApplyDamage(
		OtherActor,
		ExplosionDamage,
		GetInstigatorController(),
		this,
		ActualDamageType
	);
}

void ABaseGoblinBombProjectile::UpdateExplosionHitboxTransform()
{
	if (!ExplosionHitbox)
	{
		return;
	}

	ExplosionHitbox->SetRelativeLocation(ExplosionHitboxLocalOffset);
	ExplosionHitbox->SetBoxExtent(ExplosionHitboxExtent);
}