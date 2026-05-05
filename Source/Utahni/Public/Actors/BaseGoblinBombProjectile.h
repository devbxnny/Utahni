#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseGoblinBombProjectile.generated.h"

class UBoxComponent;
class UDamageType;
class UPrimitiveComponent;
class UProjectileMovementComponent;
class UPaperFlipbook;
class UPaperFlipbookComponent;
class UPaperSprite;
class UPaperSpriteComponent;
class USphereComponent;

UCLASS()
class UTAHNI_API ABaseGoblinBombProjectile : public AActor
{
	GENERATED_BODY()

public:
	ABaseGoblinBombProjectile();

	virtual void Tick(float DeltaTime) override;

	void LaunchProjectile(const FVector& LaunchVelocity);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnProjectileHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

	UFUNCTION()
	void HandleExplosionHitboxBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	void StartImpact();
	void EnableExplosionHitbox();
	void DisableExplosionHitbox();
	void ApplyExplosionDamageToActor(AActor* OtherActor);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Components")
	TObjectPtr<USphereComponent> CollisionComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Components")
	TObjectPtr<UPaperSpriteComponent> SpriteComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Components")
	TObjectPtr<UPaperFlipbookComponent> FlipbookComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Components")
	TObjectPtr<UBoxComponent> ExplosionHitbox = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TObjectPtr<UPaperSprite> TravelSprite = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TObjectPtr<UPaperFlipbook> ImpactFlipbook = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Explosion", meta = (ClampMin = "0.0"))
	float ExplosionDamage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Explosion")
	TSubclassOf<UDamageType> DamageTypeClass = UDamageType::StaticClass();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Explosion")
	FVector ExplosionHitboxExtent = FVector(70.0f, 30.0f, 70.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Explosion")
	FVector ExplosionHitboxLocalOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Explosion", meta = (ClampMin = "0.0"))
	float ExplosionHitboxStartTime = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Explosion", meta = (ClampMin = "0.0"))
	float ExplosionHitboxEndTime = 1.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float ImpactDestroyDelay = 2.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	bool bHasImpacted = false;

private:
	void UpdateExplosionHitboxTransform();

	TSet<TWeakObjectPtr<AActor>> HitActorsThisExplosion;

	FTimerHandle ExplosionHitboxStartTimerHandle;
	FTimerHandle ExplosionHitboxEndTimerHandle;
};