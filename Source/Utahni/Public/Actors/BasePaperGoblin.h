#pragma once

#include "CoreMinimal.h"
#include "BasePaperEnemy.h"
#include "Actors/BaseGoblinBombProjectile.h"
#include "BasePaperGoblin.generated.h"

class AController;

UENUM(BlueprintType)
enum class EGoblinState : uint8
{
	Idle         UMETA(DisplayName = "Idle"),
	Chasing      UMETA(DisplayName = "Chasing"),
	KnifeAttack  UMETA(DisplayName = "Knife Attack"),
	DodgeAttack  UMETA(DisplayName = "Dodge Attack"),
	BombAttack   UMETA(DisplayName = "Bomb Attack"),
	Hit          UMETA(DisplayName = "Hit"),
	Dead         UMETA(DisplayName = "Dead")
};

UCLASS()
class UTAHNI_API ABasePaperGoblin : public ABasePaperEnemy
{
	GENERATED_BODY()

public:
	ABasePaperGoblin();

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
	void UpdateKnifeAttack(float DeltaSeconds);
	void UpdateDodgeAttack(float DeltaSeconds);
	void UpdateBombAttack(float DeltaSeconds);

	void FinishAttack();
	void ChangeState(EGoblinState NewState);
	void SpawnBombProjectile();
	void TryChooseMeleeAttack();
	void FacePlayer();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Goblin|State")
	EGoblinState CurrentState = EGoblinState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goblin|Combat")
	float KnifeRange = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goblin|Combat")
	float BombRange = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goblin|Combat", meta = (ClampMin = "0.01"))
	float KnifeDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goblin|Combat", meta = (ClampMin = "0.0"))
	float KnifeHitboxStartTime = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goblin|Combat", meta = (ClampMin = "0.0"))
	float KnifeHitboxEndTime = 0.22f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goblin|Combat", meta = (ClampMin = "0.01"))
	float BombDuration = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goblin|Combat", meta = (ClampMin = "0.0"))
	float KnifeCooldown = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goblin|Combat", meta = (ClampMin = "0.0"))
	float BombCooldown = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goblin|Combat", meta = (ClampMin = "0.0"))
	float BombSpawnTime = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goblin|Combat", meta = (ClampMin = "0.01"))
	float DodgeDuration = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goblin|Combat", meta = (ClampMin = "0.0"))
	float DodgeHitboxStartTime = 0.06f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goblin|Combat", meta = (ClampMin = "0.0"))
	float DodgeHitboxEndTime = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goblin|Combat", meta = (ClampMin = "0.0"))
	float DodgeCooldown = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goblin|Projectile")
	TSubclassOf<ABaseGoblinBombProjectile> BombProjectileClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Goblin|Combat")
	bool bBombSpawned = false;

	float AttackTimer = 0.0f;
	float LastKnifeTime = -100.0f;
	float LastBombTime = -100.0f;
	float LastDodgeTime = -100.0f;
};