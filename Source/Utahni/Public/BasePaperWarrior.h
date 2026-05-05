#pragma once

#include "CoreMinimal.h"
#include "BasePaperEnemy.h"
#include "BasePaperWarrior.generated.h"

class AController;

UENUM(BlueprintType)
enum class EWarriorState : uint8
{
	Idle        UMETA(DisplayName = "Idle"),
	Chasing     UMETA(DisplayName = "Chasing"),
	Attack1     UMETA(DisplayName = "Attack 1"),
	Attack2     UMETA(DisplayName = "Attack 2"),
	Attack3     UMETA(DisplayName = "Attack 3"),
	Attack4Jump UMETA(DisplayName = "Attack 4 Jump"),
	Dodging     UMETA(DisplayName = "Dodging"),
	Hit         UMETA(DisplayName = "Hit"),
	Dead        UMETA(DisplayName = "Dead")
};

UCLASS()
class UTAHNI_API ABasePaperWarrior : public ABasePaperEnemy
{
	GENERATED_BODY()

public:
	ABasePaperWarrior();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	virtual bool IsAttacking() const override;
	virtual int32 GetAttackIndex() const override;
	virtual bool IsHurt() const override;
	virtual void OnCharacterDamaged(float DamageTaken, AActor* DamageCauser) override;
	virtual void OnCharacterDied(AActor* DamageCauser, AController* InstigatedBy) override;

	void UpdateState(float DeltaSeconds);
	void UpdateIdle();
	void UpdateChase(float DeltaSeconds);
	void UpdateAttack1(float DeltaSeconds);
	void UpdateAttack2(float DeltaSeconds);
	void UpdateAttack3(float DeltaSeconds);
	void UpdateAttack4Jump(float DeltaSeconds);
	void UpdateDodge(float DeltaSeconds);

	void TryStartAttack();
	bool ShouldStartDodge() const;

	void StartAttack1();
	void StartAttack2();
	void StartAttack3();
	void StartAttack4Jump();
	void StartDodge();
	void FinishCurrentAction();
	void ChangeState(EWarriorState NewState);
	void ConfigureHitboxForCurrentAttack();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Warrior|State")
	EWarriorState CurrentState = EWarriorState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat")
	float Attack1Range = 85.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat")
	float Attack2Range = 130.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat")
	float Attack3Range = 110.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat")
	float Attack4JumpRange = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.01"))
	float Attack1Duration = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.01"))
	float Attack2Duration = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.01"))
	float Attack3Duration = 0.60f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.01"))
	float Attack4JumpDuration = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.0"))
	float Attack1DamageAmount = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.0"))
	float Attack2DamageAmount = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.0"))
	float Attack3DamageAmount = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.0"))
	float Attack4JumpDamageAmount = 24.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.0"))
	float Attack1HitboxStartTime = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.0"))
	float Attack1HitboxEndTime = 0.20f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.0"))
	float Attack2HitboxStartTime = 0.10f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.0"))
	float Attack2HitboxEndTime = 0.26f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.0"))
	float Attack3HitboxStartTime = 0.10f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.0"))
	float Attack3HitboxEndTime = 0.28f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.0"))
	float Attack4JumpHitboxStartTime = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.0"))
	float Attack4JumpHitboxEndTime = 0.38f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.0"))
	float Attack1Cooldown = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.0"))
	float Attack2Cooldown = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.0"))
	float Attack3Cooldown = 1.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.0"))
	float Attack4JumpCooldown = 1.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.0"))
	float AttackRecoveryTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat")
	FVector Attack1HitboxExtent = FVector(48.0f, 20.0f, 42.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat")
	FVector Attack2HitboxExtent = FVector(64.0f, 20.0f, 46.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat")
	FVector Attack3HitboxExtent = FVector(56.0f, 20.0f, 46.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat")
	FVector Attack4JumpHitboxExtent = FVector(58.0f, 20.0f, 52.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.0"))
	float Attack1ForwardOffset = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.0"))
	float Attack2ForwardOffset = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.0"))
	float Attack3ForwardOffset = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Combat", meta = (ClampMin = "0.0"))
	float Attack4JumpForwardOffset = 65.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Dodge", meta = (ClampMin = "0.0"))
	float DodgeDetectRange = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Dodge", meta = (ClampMin = "0.0"))
	float DodgeCooldown = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Dodge", meta = (ClampMin = "0.0"))
	float DodgeBackwardSpeed = 420.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Dodge", meta = (ClampMin = "0.0"))
	float DodgeJumpSpeed = 340.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|Dodge", meta = (ClampMin = "0.01"))
	float DodgeDuration = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|JumpAttack", meta = (ClampMin = "0.0"))
	float JumpAttackForwardSpeed = 540.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warrior|JumpAttack", meta = (ClampMin = "0.0"))
	float JumpAttackUpSpeed = 420.0f;

private:
	float ActionTimer = 0.0f;
	float LastAttack1Time = -100.0f;
	float LastAttack2Time = -100.0f;
	float LastAttack3Time = -100.0f;
	float LastAttack4JumpTime = -100.0f;
	float LastDodgeTime = -100.0f;
	float NextAttackAllowedTime = -100.0f;

	bool bJumpAttackLaunched = false;
};