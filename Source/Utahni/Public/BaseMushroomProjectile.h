#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseMushroomProjectile.generated.h"

class USphereComponent;
class UPaperSpriteComponent;
class UPaperFlipbookComponent;
class UProjectileMovementComponent;
class UPaperFlipbook;
class UPaperSprite;
class UPrimitiveComponent;

UCLASS()
class UTAHNI_API ABaseMushroomProjectile : public AActor
{
	GENERATED_BODY()

public:
	ABaseMushroomProjectile();

	virtual void Tick(float DeltaTime) override;

	void LaunchProjectile(const FVector& LaunchVelocity);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnProjectileOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	void StartImpact();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Components")
	USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Components")
	UPaperSpriteComponent* SpriteComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Components")
	UPaperFlipbookComponent* FlipbookComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Components")
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	UPaperSprite* TravelSprite = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	UPaperFlipbook* ImpactFlipbook = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float ImpactDestroyDelay = 0.8f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	bool bHasImpacted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Damage")
	float Damage = 10.0f;
};