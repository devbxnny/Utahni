#pragma once

#include "CoreMinimal.h"
#include "BasePaperEnemy.h"
#include "PaperFlipbook.h"
#include "BasePaperBat.generated.h"

UENUM(BlueprintType)
enum class EBatMoveState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Swooping UMETA(DisplayName = "Swooping"),
	Returning UMETA(DisplayName = "Returning")
};

UCLASS()
class UTAHNI_API ABasePaperBat : public ABasePaperEnemy
{
	GENERATED_BODY()

public:
	ABasePaperBat();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual bool IsAttacking() const override;
	virtual int32 GetAttackIndex() const override;
	virtual bool IsHurt() const override;
	virtual void OnCharacterDamaged(float DamageTaken, AActor* DamageCauser) override;
	virtual void OnCharacterDied(AActor* DamageCauser, AController* InstigatedBy) override;

	void UpdateIdleMovement(float DeltaTime);
	void UpdateSwoopMovement(float DeltaTime);
	void UpdateReturnMovement(float DeltaTime);
	void CheckPlayerDistance();
	void FaceMovementDirection(const FVector& Direction);
	void ChangeState(EBatMoveState NewState);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bat")
	EBatMoveState BatState = EBatMoveState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bat")
	float BaseHoverZ = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bat|Detection")
	float DetectRange = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bat|Detection")
	float LoseInterestRange = 700.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bat|Idle")
	float IdleRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bat|Idle")
	float IdleMoveSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bat|Idle")
	float HoverHeight = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bat|Idle")
	float HoverBobAmplitude = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bat|Idle")
	float HoverBobSpeed = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bat|Swoop")
	float SwoopSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bat|Swoop")
	float SwoopDepthOffset = -40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bat|Return")
	float ReturnSpeed = 220.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bat")
	FVector HomeLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bat")
	AActor* TargetPlayer = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bat|Animation")
	UPaperFlipbook* AttackFlipbook2 = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bat|Animation")
	UPaperFlipbook* AttackFlipbook3 = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bat|Animation")
	UPaperFlipbook* HitFlipbook = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bat|Swoop")
	FVector SwoopStartLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bat|Swoop")
	FVector SwoopTargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bat|Swoop")
	float SwoopProgress = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bat|Swoop")
	float SwoopDuration = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bat|Swoop")
	float SwoopArcHeight = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bat|Swoop", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SwoopHitboxStartProgress = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bat|Swoop", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SwoopHitboxEndProgress = 0.75f;

	void UpdateAnimation();

	float RunningTime = 0.0f;

private:
	int32 CurrentSwoopAttackIndex = 2;
};
