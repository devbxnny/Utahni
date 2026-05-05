#pragma once

#include "CoreMinimal.h"
#include "BasePaperCharacter.h"
#include "BasePaperEnemy.generated.h"

class AController;
class UBoxComponent;
class UPrimitiveComponent;

UCLASS()
class UTAHNI_API ABasePaperEnemy : public ABasePaperCharacter
{
	GENERATED_BODY()

public:
	ABasePaperEnemy();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	virtual bool IsAttacking() const override;
	virtual int32 GetAttackIndex() const override;
	virtual void OnCharacterDamaged(float DamageTaken, AActor* DamageCauser) override;
	virtual void OnCharacterDied(AActor* DamageCauser, AController* InstigatedBy) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float MoveSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float DetectionRange = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float AttackRange = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float Damage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy", meta = (ClampMin = "0.01"))
	float AttackCooldown = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy", meta = (ClampMin = "0.01"))
	float MinimumAttackDuration = 0.20f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy", meta = (ClampMin = "0.0"))
	float DeathDestroyDelay = 1.0f;

	UPROPERTY()
	TObjectPtr<AActor> PlayerRef = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UBoxComponent> AttackHitbox = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float AttackHitboxForwardOffset = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FVector AttackHitboxBoxExtent = FVector(40.0f, 20.0f, 40.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FVector AttackHitboxLocalOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float AttackHitboxStartTime = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float AttackHitboxEndTime = 0.16f;

	void HandleMovement();
	void FacePlayer();
	void AttackPlayer();
	void EndAttack();

	void EnableAttackHitbox();
	void DisableAttackHitbox();
	void UpdateAttackHitboxTransform();
	bool IsEnemyFacingRight() const;

	void ResetAttackHitTracking();

	UFUNCTION()
	void HandleAttackHitboxBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

private:
	bool bEnemyAttacking = false;
	bool bAttackHitboxActive = false;
	float LastAttackTime = -999.0f;

	TSet<TWeakObjectPtr<AActor>> HitActorsThisAttack;

	FTimerHandle AttackTimerHandle;
	FTimerHandle AttackHitboxStartTimerHandle;
	FTimerHandle AttackHitboxEndTimerHandle;
};
