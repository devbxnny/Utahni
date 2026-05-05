#pragma once

#include "CoreMinimal.h"
#include "BasePaperEnemy.h"
#include "BaseMushroomProjectile.h"
#include "BasePaperMushroom.generated.h"

class AController;

UENUM(BlueprintType)
enum class EMushroomState : uint8
{
	Idle      UMETA(DisplayName = "Idle"),
	Chasing   UMETA(DisplayName = "Chasing"),
	Punching  UMETA(DisplayName = "Punching"),
	Chomping  UMETA(DisplayName = "Chomping"),
	Bursting  UMETA(DisplayName = "Bursting"),
	Hit       UMETA(DisplayName = "Hit"),
	Dead      UMETA(DisplayName = "Dead")
};

UCLASS()
class UTAHNI_API ABasePaperMushroom : public ABasePaperEnemy
{
	GENERATED_BODY()

public:
	ABasePaperMushroom();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	virtual bool IsAttacking() const override;
	virtual int32 GetAttackIndex() const override;
	virtual bool IsHurt() const override;
	virtual void OnCharacterDamaged(float DamageTaken, AActor* DamageCauser) override;
	virtual void OnCharacterDied(AActor* DamageCauser, AController* InstigatedBy) override;

	void UpdateState(float DeltaSeconds);
	void UpdateIdle(float DeltaSeconds);
	void UpdateChase(float DeltaSeconds);
	void UpdatePunch(float DeltaSeconds);
	void UpdateChomp(float DeltaSeconds);
	void UpdateBurst(float DeltaSeconds);

	void TryChooseAttack();
	void FinishAttack();
	void FacePlayer();
	void ChangeState(EMushroomState NewState);
	void SpawnBurstProjectile();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mushroom|State")
	EMushroomState CurrentState = EMushroomState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mushroom|Combat")
	float MeleeRange = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mushroom|Combat")
	float BurstRange = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mushroom|Combat", meta = (ClampMin = "0.01"))
	float PunchDuration = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mushroom|Combat", meta = (ClampMin = "0.0"))
	float PunchHitboxStartTime = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mushroom|Combat", meta = (ClampMin = "0.0"))
	float PunchHitboxEndTime = 0.24f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mushroom|Combat", meta = (ClampMin = "0.01"))
	float ChompDuration = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mushroom|Combat", meta = (ClampMin = "0.0"))
	float ChompHitboxStartTime = 0.10f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mushroom|Combat", meta = (ClampMin = "0.0"))
	float ChompHitboxEndTime = 0.30f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mushroom|Combat", meta = (ClampMin = "0.01"))
	float BurstDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mushroom|Combat", meta = (ClampMin = "0.0"))
	float PunchCooldown = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mushroom|Combat", meta = (ClampMin = "0.0"))
	float ChompCooldown = 1.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mushroom|Combat", meta = (ClampMin = "0.0"))
	float BurstCooldown = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mushroom|Combat", meta = (ClampMin = "0.0"))
	float ProjectileSpawnTime = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mushroom|Projectile")
	TSubclassOf<ABaseMushroomProjectile> ProjectileClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mushroom|Combat")
	bool bProjectileSpawned = false;

	float AttackTimer = 0.0f;
	float LastPunchTime = -100.0f;
	float LastChompTime = -100.0f;
	float LastBurstTime = -100.0f;
};