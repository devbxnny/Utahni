#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "BasePaperCharacter.generated.h"

class AController;
class UHealthComponent;
class UPaperFlipbook;
class USoundBase;

enum class EPaperAnimState : uint8
{
	None,
	Idle,
	Run,
	Jump,
	Fall,
	Shield,
	ShieldBlockSuccess,
	Roll,
	WallSlide,
	Attack1,
	Attack2,
	Attack3,
	Hurt,
	Death
};

UCLASS()
class UTAHNI_API ABasePaperCharacter : public APaperCharacter
{
	GENERATED_BODY()

public:
	ABasePaperCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintPure, Category = "Stats")
	UHealthComponent* GetHealthComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Respawn")
	virtual void RespawnAtTransform(const FTransform& RespawnTransform);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHealthComponent> HealthComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UPaperFlipbook> IdleFlipbook = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UPaperFlipbook> RunFlipbook = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UPaperFlipbook> JumpFlipbook = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UPaperFlipbook> FallFlipbook = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UPaperFlipbook> ShieldFlipbook = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UPaperFlipbook> ShieldBlockSuccessFlipbook = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UPaperFlipbook> RollFlipbook = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UPaperFlipbook> WallSlideFlipbook = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UPaperFlipbook> Attack1Flipbook = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UPaperFlipbook> Attack2Flipbook = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UPaperFlipbook> Attack3Flipbook = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UPaperFlipbook> HurtFlipbook = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UPaperFlipbook> DeathFlipbook = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> Attack1Sound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> Attack2Sound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> Attack3Sound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> JumpSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> FootstepSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> HurtSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> DeathSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ClampMin = "0.01"))
	float FootstepInterval = 0.30f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float RunSpeedThreshold = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float FacingThreshold = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float HurtDuration = 0.20f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float InvincibilityDuration = 0.50f;

	virtual bool IsRolling() const;
	virtual bool IsWallSliding() const;
	virtual bool IsAttacking() const;
	virtual int32 GetAttackIndex() const;
	virtual bool IsShielding() const;
	virtual bool IsShieldBlockSuccess() const;
	virtual bool IsHurt() const;
	virtual bool IsDeadState() const;
	virtual void OnCharacterRespawned();
	virtual bool CanBlockDamageFrom(AActor* DamageCauser) const;

	void UpdateFacing();
	void UpdateAnimation();
	void SetFacingDirection(bool bNewFacingRight);
	bool IsFacingRight() const;

	virtual void OnCharacterDamaged(float DamageTaken, AActor* DamageCauser);
	virtual void OnCharacterDied(AActor* DamageCauser, AController* InstigatedBy);
	virtual void OnShieldBlocked(float BlockedDamage, AActor* DamageCauser, AController* InstigatedBy);

	void ResetAnimationState();
	void PlayCharacterSound(USoundBase* SoundToPlay) const;
	USoundBase* GetAttackSoundForIndex(int32 AttackIndex) const;
	void PlayAttackSoundForIndex(int32 AttackIndex) const;
	void PlayJumpSound() const;
	void TryPlayFootstepSound(float DeltaSeconds, bool bIsMovingOnGround, float HorizontalSpeed);
	void ResetFootstepSoundState();

private:
	UFUNCTION()
	void HandleHealthChanged(UHealthComponent* InHealthComponent, float NewHealth, float InMaxHealth, float HealthDelta, AActor* DamageCauser);

	UFUNCTION()
	void HandleDeath(UHealthComponent* InHealthComponent, AActor* DamageCauser, AController* InstigatedBy);

	void SetAnimation(UPaperFlipbook* NewFlipbook, EPaperAnimState NewState, bool bLooping);
	void ClearHurtState();
	void ClearInvincibility();

	bool bFacingRight = true;
	bool bIsHurt = false;
	bool bIsDead = false;
	FVector InitialSpriteScale = FVector::OneVector;

	TObjectPtr<UPaperFlipbook> CurrentFlipbook = nullptr;
	EPaperAnimState CurrentAnimState = EPaperAnimState::None;

	float FootstepSoundTimer = 0.0f;

	FTimerHandle HurtTimerHandle;
	FTimerHandle InvincibilityTimerHandle;
};
