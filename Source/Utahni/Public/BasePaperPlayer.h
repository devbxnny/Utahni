#pragma once

#include "CoreMinimal.h"
#include "BasePaperCharacter.h"
#include "Blueprint/UserWidget.h"
#include "BasePaperPlayer.generated.h"

class AController;
class UBoxComponent;
class UCameraComponent;
class UInputComponent;
class UPaperFlipbook;
class USoundBase;
class USpringArmComponent;
class UUserWidget;
class USkeletalMeshComponent;
class UAnimSequence;

UCLASS()
class UTAHNI_API ABasePaperPlayer : public ABasePaperCharacter
{
	GENERATED_BODY()

public:
	ABasePaperPlayer();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void Landed(const FHitResult& Hit) override;

	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void SetRespawnTransform(const FTransform& NewRespawnTransform);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void TogglePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ResumeGame();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void OpenMainMenu();

	UFUNCTION(BlueprintCallable, Category = "Powerups")
	void Activate100DamageBoost();

	UFUNCTION(BlueprintCallable, Category = "Powerups")
	void ActivateMushroom3DForm();

	UFUNCTION(Exec)
	void ToggleGodMode();

	UFUNCTION(Exec)
	void RefillHealthToFull();

	UFUNCTION(BlueprintPure, Category = "Cheats")
	bool IsGodModeEnabled() const;

	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser
	) override;

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAttackHitboxActive() const;

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual bool IsRolling() const override;
	virtual bool IsWallSliding() const override;
	virtual bool IsAttacking() const override;
	virtual int32 GetAttackIndex() const override;
	virtual bool IsShielding() const override;
	virtual bool IsShieldBlockSuccess() const override;
	virtual void OnCharacterDamaged(float DamageTaken, AActor* DamageCauser) override;
	virtual void OnCharacterDied(AActor* DamageCauser, AController* InstigatedBy) override;
	virtual void OnCharacterRespawned() override;
	virtual void OnShieldBlocked(float BlockedDamage, AActor* DamageCauser, AController* InstigatedBy) override;
	virtual bool CanBlockDamageFrom(AActor* DamageCauser) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "0.0"))
	float SpringArmLength = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	FRotator SpringArmRotation = FRotator(0.0f, -90.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	FVector CameraSocketOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "1.0", ClampMax = "170.0"))
	float CameraFieldOfView = 90.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UBoxComponent> AttackHitbox = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> LandSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> RollSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> ShieldStartSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> ShieldBlockSuccessSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> RespawnSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> SwitchCharacterSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Shield", meta = (ClampMin = "0.01"))
	float ShieldBlockSuccessDuration = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Shield", meta = (ClampMin = "0.0"))
	float ShieldCooldownDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Shield", meta = (ClampMin = "0.01"))
	float ShieldMaxHoldDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.01"))
	float RollDuration = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float RollLaunchSpeed = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float WallSlideMaxFallSpeed = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float WallCheckDistance = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float WallJumpHorizontalSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float WallJumpVerticalSpeed = 650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.01"))
	float ComboResetTime = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float AttackMoveSpeed = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float AttackHitboxForwardOffset = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FVector AttackHitboxBoxExtent = FVector(55.0f, 20.0f, 60.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FVector AttackHitboxLocalOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float Attack1Damage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float Attack2Damage = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float Attack3Damage = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powerups")
	bool b100DamageBoostActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powerups", meta = (ClampMin = "0.0"))
	float BoostedAttackDamage = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powerups")
	bool bIsFoxCharacter = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powerups")
	float Mushroom3DFormDuration = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powerups")
	TObjectPtr<UAnimSequence> MushroomIdleAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powerups")
	TObjectPtr<UAnimSequence> MushroomWalkAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float Attack1HitStartTime = 0.04f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float Attack2HitStartTime = 0.04f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float Attack3HitStartTime = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float Attack1HitEndTime = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float Attack2HitEndTime = 0.20f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float Attack3HitEndTime = 0.24f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swap")
	TObjectPtr<ABasePaperPlayer> PartnerCharacter = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swap")
	bool bIsCurrentlyControlled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
	float FollowDistance = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
	float TeleportDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
	float MoveAcceptanceRadius = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swap")
	float FacingDirection = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float FallDeathZ = -3500.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Respawn")
	FTransform CurrentRespawnTransform;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuWidgetClass = nullptr;

	UPROPERTY()
	TObjectPtr<UUserWidget> PauseMenuWidget = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	FName MainMenuLevelName = TEXT("L_MainMenu");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> DeathScreenWidgetClass = nullptr;

	UPROPERTY()
	TObjectPtr<UUserWidget> DeathScreenWidget = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> PlayerHUDWidgetClass = nullptr;

	UPROPERTY()
	TObjectPtr<UUserWidget> PlayerHUDWidget = nullptr;

	void ShowDeathScreenAndRespawn();

	void MoveRight(float Value);
	void StartJump();
	void StopJumpInput();
	void StartRoll();
	void EndRoll();
	void StartAttack();
	void BeginAttack(int32 AttackIndex);
	void FinishAttack();
	void ResetAttackCombo();
	void StartShield();
	void StopShield();
	void UpdateShieldState();
	void EndShieldBlockSuccess();
	void EnableAttackHitbox();
	void DisableAttackHitbox();
	void UpdateAttackHitboxTransform();
	void CancelAttackState();
	void EndShieldCooldown();
	void EndShieldByTimeout();

	void SetPartner(ABasePaperPlayer* NewPartner);
	void StartFollowing();
	void StopFollowing();
	void UpdateFollow();
	void InitializeSwapPartners();
	void SwitchCharacters();
	void ApplyCameraSettings();
	void UpdateAttackMoveSpeed();
	void CheckFallDeath();
	void EndMushroom3DForm();
	void UpdateMushroom3DAnimation();
	USkeletalMeshComponent* GetMushroom3DMesh() const;

	void HandlePausePressed();

	UFUNCTION()
	void HandleAttackHitboxBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	bool CanStartRoll() const;
	bool CanStartAttack() const;
	bool CanStartShield() const;
	void UpdateWallSlide();
	bool IsTouchingWallInDirection(float Direction) const;
	UPaperFlipbook* GetAttackFlipbookForIndex(int32 AttackIndex) const;
	float GetAttackDamageForIndex(int32 AttackIndex) const;
	float GetAttackHitStartTimeForIndex(int32 AttackIndex) const;
	float GetAttackHitEndTimeForIndex(int32 AttackIndex) const;

private:
	void RespawnToCheckpoint();

private:
	bool bIsRolling = false;
	bool bIsWallSliding = false;
	bool bIsAttacking = false;
	bool bAttackHitboxActive = false;
	bool bHasTriggeredFallDeath = false;
	bool bWantsToShield = false;
	bool bIsShieldingState = false;
	bool bShieldBlockSuccessState = false;
	bool bShieldOnCooldown = false;
	bool bGodModeEnabled = false;
	bool bJumpSoundPlayedThisAirTime = false;
	bool bMushroom3DFormActive = false;
	bool bMushroomUsingWalkAnim = false;

	float LastMoveInput = 0.0f;
	float RollDirection = 1.0f;
	float DefaultWalkSpeed = 600.0f;

	int32 CurrentAttackIndex = 0;
	int32 PendingAttackInputs = 0;

	float CachedGroundFriction = 0.0f;
	float CachedBrakingFrictionFactor = 0.0f;
	float CachedBrakingDecelerationWalking = 0.0f;

	TSet<TWeakObjectPtr<AActor>> HitActorsThisAttack;

	FTimerHandle RespawnTimerHandle;
	FTimerHandle RollTimerHandle;
	FTimerHandle AttackTimerHandle;
	FTimerHandle ComboResetTimerHandle;
	FTimerHandle AttackHitboxStartTimerHandle;
	FTimerHandle AttackHitboxEndTimerHandle;
	FTimerHandle ShieldBlockSuccessTimerHandle;
	FTimerHandle ShieldCooldownTimerHandle;
	FTimerHandle ShieldHoldTimerHandle;
	FTimerHandle Mushroom3DFormTimerHandle;

	bool bPauseMenuOpen = false;
};