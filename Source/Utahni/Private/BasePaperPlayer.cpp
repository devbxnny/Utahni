#include "BasePaperPlayer.h"
#include "HealthComponent.h"
#include "Engine/Engine.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PaperFlipbook.h"
#include "TimerManager.h"
#include "UI/GameOverWidget.h"
#include "Blueprint/UserWidget.h"

ABasePaperPlayer::ABasePaperPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = SpringArmLength;
	SpringArm->SetRelativeRotation(SpringArmRotation);
	SpringArm->SocketOffset = CameraSocketOffset;
	SpringArm->bDoCollisionTest = false;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	Camera->ProjectionMode = ECameraProjectionMode::Perspective;
	Camera->FieldOfView = CameraFieldOfView;

	AttackHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackHitbox"));
	AttackHitbox->SetupAttachment(RootComponent);
	AttackHitbox->SetBoxExtent(AttackHitboxBoxExtent);
	AttackHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttackHitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
	AttackHitbox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AttackHitbox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	AttackHitbox->SetGenerateOverlapEvents(true);
	AttackHitbox->OnComponentBeginOverlap.AddDynamic(this, &ABasePaperPlayer::HandleAttackHitboxBeginOverlap);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->GravityScale = 2.5f;
		Movement->AirControl = 0.8f;
		Movement->JumpZVelocity = 650.0f;
		Movement->MaxWalkSpeed = 600.0f;
		Movement->bConstrainToPlane = true;
		Movement->SetPlaneConstraintAxisSetting(EPlaneConstraintAxisSetting::Y);
		Movement->bSnapToPlaneAtStart = true;

		CachedGroundFriction = Movement->GroundFriction;
		CachedBrakingFrictionFactor = Movement->BrakingFrictionFactor;
		CachedBrakingDecelerationWalking = Movement->BrakingDecelerationWalking;
		DefaultWalkSpeed = Movement->MaxWalkSpeed;
	}

	JumpMaxCount = 1;
	JumpMaxHoldTime = 0.12f;

	PartnerCharacter = nullptr;
	bIsCurrentlyControlled = false;
	FollowDistance = 150.0f;
	TeleportDistance = 1000.0f;
	MoveAcceptanceRadius = 75.0f;
	FacingDirection = 0.0f;
	bJumpSoundPlayedThisAirTime = false;
}

void ABasePaperPlayer::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyCameraSettings();
}

void ABasePaperPlayer::BeginPlay()
{
	Super::BeginPlay();

	InitializeSwapPartners();
	ApplyCameraSettings();
	UpdateAttackMoveSpeed();
	CurrentRespawnTransform = GetActorTransform();

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController != nullptr)
	{
		PlayerController->bShowMouseCursor = false;
		PlayerController->SetInputMode(FInputModeGameOnly());
	}

	if (PlayerController != nullptr && PlayerHUDWidgetClass != nullptr)
	{
		if (PlayerHUDWidget == nullptr)
		{
			PlayerHUDWidget = CreateWidget<UUserWidget>(PlayerController, PlayerHUDWidgetClass);
		}

		if (PlayerHUDWidget != nullptr && !PlayerHUDWidget->IsInViewport())
		{
			PlayerHUDWidget->AddToViewport();
		}
	}

	UGameplayStatics::SetGamePaused(this, false);
	bPauseMenuOpen = false;
}

void ABasePaperPlayer::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	bJumpSoundPlayedThisAirTime = false;
	PlayCharacterSound(LandSound);
}

void ABasePaperPlayer::SetRespawnTransform(const FTransform& NewRespawnTransform)
{
	CurrentRespawnTransform = NewRespawnTransform;
}

void ABasePaperPlayer::Tick(float DeltaSeconds)
{
	CheckFallDeath();

	if (!bIsCurrentlyControlled)
	{
		UpdateFollow();
	}

	UpdateShieldState();
	UpdateAttackHitboxTransform();
	UpdateWallSlide();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		TryPlayFootstepSound(
			DeltaSeconds,
			Movement->IsMovingOnGround(),
			FMath::Abs(GetVelocity().X)
		);
	}

	Super::Tick(DeltaSeconds);
}

void ABasePaperPlayer::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	StopFollowing();
}

void ABasePaperPlayer::UnPossessed()
{
	Super::UnPossessed();
	StartFollowing();
}

void ABasePaperPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ABasePaperPlayer::MoveRight);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ABasePaperPlayer::StartJump);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ABasePaperPlayer::StopJumpInput);
	PlayerInputComponent->BindAction(TEXT("Roll"), IE_Pressed, this, &ABasePaperPlayer::StartRoll);
	PlayerInputComponent->BindAction(TEXT("Attack"), IE_Pressed, this, &ABasePaperPlayer::StartAttack);
	PlayerInputComponent->BindAction(TEXT("Shield"), IE_Pressed, this, &ABasePaperPlayer::StartShield);
	PlayerInputComponent->BindAction(TEXT("Shield"), IE_Released, this, &ABasePaperPlayer::StopShield);
	PlayerInputComponent->BindAction(TEXT("SwitchCharacter"), IE_Pressed, this, &ABasePaperPlayer::SwitchCharacters);
	PlayerInputComponent->BindAction(TEXT("Pause"), IE_Pressed, this, &ABasePaperPlayer::HandlePausePressed);
}

void ABasePaperPlayer::HandlePausePressed()
{
	if (IsDeadState())
	{
		return;
	}

	TogglePauseMenu();
}

void ABasePaperPlayer::TogglePauseMenu()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController == nullptr)
	{
		return;
	}

	if (bPauseMenuOpen)
	{
		ResumeGame();
		return;
	}

	if (PauseMenuWidgetClass == nullptr)
	{
		return;
	}

	if (PauseMenuWidget == nullptr)
	{
		PauseMenuWidget = CreateWidget<UUserWidget>(PlayerController, PauseMenuWidgetClass);
	}

	if (PauseMenuWidget == nullptr)
	{
		return;
	}

	if (!PauseMenuWidget->IsInViewport())
	{
		PauseMenuWidget->AddToViewport();
	}

	UGameplayStatics::SetGamePaused(this, true);

	PlayerController->bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(PauseMenuWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);

	bPauseMenuOpen = true;
}

void ABasePaperPlayer::ResumeGame()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController == nullptr)
	{
		return;
	}

	if (PauseMenuWidget != nullptr)
	{
		PauseMenuWidget->RemoveFromParent();
	}

	UGameplayStatics::SetGamePaused(this, false);

	PlayerController->bShowMouseCursor = false;
	PlayerController->SetInputMode(FInputModeGameOnly());

	bPauseMenuOpen = false;
}

void ABasePaperPlayer::OpenMainMenu()
{
	UGameplayStatics::SetGamePaused(this, false);
	UGameplayStatics::OpenLevel(this, MainMenuLevelName);
}

void ABasePaperPlayer::Activate100DamageBoost()
{
	b100DamageBoostActive = true;
}

bool ABasePaperPlayer::IsRolling() const
{
	return bIsRolling;
}

bool ABasePaperPlayer::IsWallSliding() const
{
	return bIsWallSliding;
}

bool ABasePaperPlayer::IsAttacking() const
{
	return bIsAttacking;
}

int32 ABasePaperPlayer::GetAttackIndex() const
{
	return CurrentAttackIndex;
}

bool ABasePaperPlayer::IsShielding() const
{
	return bIsShieldingState;
}

bool ABasePaperPlayer::IsShieldBlockSuccess() const
{
	return bShieldBlockSuccessState;
}

bool ABasePaperPlayer::CanBlockDamageFrom(AActor* DamageCauser) const
{
	if (!bIsShieldingState || bShieldOnCooldown || !DamageCauser)
	{
		return false;
	}

	const float DeltaX = DamageCauser->GetActorLocation().X - GetActorLocation().X;

	if (FMath::IsNearlyZero(DeltaX))
	{
		return true;
	}

	return IsFacingRight() ? (DeltaX >= 0.0f) : (DeltaX <= 0.0f);
}

void ABasePaperPlayer::OnCharacterDamaged(float DamageTaken, AActor* DamageCauser)
{
	bIsWallSliding = false;
	bWantsToShield = false;
	bIsShieldingState = false;
	bShieldBlockSuccessState = false;
	GetWorldTimerManager().ClearTimer(ShieldBlockSuccessTimerHandle);
	ResetFootstepSoundState();

	if (bIsRolling)
	{
		EndRoll();
	}

	CancelAttackState();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}
}

void ABasePaperPlayer::OnCharacterDied(AActor* DamageCauser, AController* InstigatedBy)
{
	bIsWallSliding = false;
	bWantsToShield = false;
	bIsShieldingState = false;
	bShieldBlockSuccessState = false;
	bShieldOnCooldown = false;

	GetWorldTimerManager().ClearTimer(ShieldBlockSuccessTimerHandle);
	GetWorldTimerManager().ClearTimer(ShieldCooldownTimerHandle);
	ResetFootstepSoundState();

	if (bIsRolling)
	{
		EndRoll();
	}

	CancelAttackState();
	DisableAttackHitbox();

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController != nullptr)
	{
		DisableInput(PlayerController);
		PlayerController->bShowMouseCursor = false;
		PlayerController->SetInputMode(FInputModeUIOnly());
	}

	ShowDeathScreenAndRespawn();

	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ABasePaperPlayer::RespawnToCheckpoint, 3.0f, false);
}

void ABasePaperPlayer::OnShieldBlocked(float BlockedDamage, AActor* DamageCauser, AController* InstigatedBy)
{
	if (IsDeadState() || bShieldOnCooldown)
	{
		return;
	}

	bWantsToShield = false;
	bIsShieldingState = false;
	bShieldBlockSuccessState = true;
	bShieldOnCooldown = true;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		FVector NewVelocity = Movement->Velocity;
		NewVelocity.X = 0.0f;
		Movement->Velocity = NewVelocity;
	}

	PlayCharacterSound(ShieldBlockSuccessSound);

	ResetAnimationState();
	UpdateAnimation();

	const float BlockDuration = ShieldBlockSuccessFlipbook
		? FMath::Max(ShieldBlockSuccessFlipbook->GetTotalDuration(), 0.01f)
		: ShieldBlockSuccessDuration;

	GetWorldTimerManager().ClearTimer(ShieldBlockSuccessTimerHandle);
	GetWorldTimerManager().SetTimer(
		ShieldBlockSuccessTimerHandle,
		this,
		&ABasePaperPlayer::EndShieldBlockSuccess,
		BlockDuration,
		false
	);

	GetWorldTimerManager().ClearTimer(ShieldCooldownTimerHandle);
	GetWorldTimerManager().SetTimer(
		ShieldCooldownTimerHandle,
		this,
		&ABasePaperPlayer::EndShieldCooldown,
		ShieldCooldownDuration,
		false
	);
}

void ABasePaperPlayer::RespawnToCheckpoint()
{
	RespawnAtTransform(CurrentRespawnTransform);
}

void ABasePaperPlayer::OnCharacterRespawned()
{
	Super::OnCharacterRespawned();

	bIsWallSliding = false;
	bIsRolling = false;
	bWantsToShield = false;
	bIsShieldingState = false;
	bShieldBlockSuccessState = false;
	bShieldOnCooldown = false;
	bJumpSoundPlayedThisAirTime = false;

	GetWorldTimerManager().ClearTimer(ShieldBlockSuccessTimerHandle);
	GetWorldTimerManager().ClearTimer(ShieldCooldownTimerHandle);

	CancelAttackState();
	DisableAttackHitbox();
	UpdateAttackMoveSpeed();
	LastMoveInput = 0.0f;
	bHasTriggeredFallDeath = false;
	ResetFootstepSoundState();
	PlayCharacterSound(RespawnSound);

	if (DeathScreenWidget != nullptr)
	{
		DeathScreenWidget->RemoveFromParent();
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController != nullptr)
	{
		EnableInput(PlayerController);
		PlayerController->bShowMouseCursor = false;
		PlayerController->SetInputMode(FInputModeGameOnly());
	}
}

void ABasePaperPlayer::MoveRight(float Value)
{
	if (!bIsCurrentlyControlled)
	{
		LastMoveInput = 0.0f;
		return;
	}

	if (IsDeadState() || IsHurt() || bIsShieldingState || bShieldBlockSuccessState)
	{
		LastMoveInput = 0.0f;

		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			FVector NewVelocity = Movement->Velocity;
			NewVelocity.X = 0.0f;
			Movement->Velocity = NewVelocity;
		}

		return;
	}

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	LastMoveInput = Value;

	if (FMath::IsNearlyZero(Value))
	{
		if (bIsAttacking && Movement && Movement->IsMovingOnGround())
		{
			FVector NewVelocity = Movement->Velocity;
			NewVelocity.X = 0.0f;
			Movement->Velocity = NewVelocity;
		}
		return;
	}

	SetFacingDirection(Value > 0.0f);
	FacingDirection = Value > 0.0f ? 0.0f : 180.0f;

	if (bIsRolling)
	{
		return;
	}

	float InputScale = Value;

	if (bIsAttacking && Movement && Movement->IsMovingOnGround() && DefaultWalkSpeed > 0.0f)
	{
		const float GroundAttackMoveMultiplier = FMath::Clamp(AttackMoveSpeed / DefaultWalkSpeed, 0.0f, 1.0f);
		InputScale *= GroundAttackMoveMultiplier;
	}

	AddMovementInput(FVector(1.0f, 0.0f, 0.0f), InputScale);
}

void ABasePaperPlayer::StartJump()
{
	if (!bIsCurrentlyControlled)
	{
		return;
	}

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Movement)
	{
		return;
	}

	if (IsDeadState() || IsHurt() || bIsRolling || bIsAttacking || bIsShieldingState || bShieldBlockSuccessState)
	{
		return;
	}

	if (bIsWallSliding)
	{
		bIsWallSliding = false;

		const float JumpDirection = IsFacingRight() ? -1.0f : 1.0f;
		SetFacingDirection(JumpDirection > 0.0f);
		FacingDirection = JumpDirection > 0.0f ? 0.0f : 180.0f;
		LaunchCharacter(FVector(JumpDirection * WallJumpHorizontalSpeed, 0.0f, WallJumpVerticalSpeed), true, true);

		if (!bJumpSoundPlayedThisAirTime)
		{
			PlayJumpSound();
			bJumpSoundPlayedThisAirTime = true;
		}

		return;
	}

	if (!Movement->IsMovingOnGround() || !CanJump())
	{
		return;
	}

	Jump();

	if (!bJumpSoundPlayedThisAirTime)
	{
		PlayJumpSound();
		bJumpSoundPlayedThisAirTime = true;
	}
}

void ABasePaperPlayer::StopJumpInput()
{
	StopJumping();
}

bool ABasePaperPlayer::CanStartRoll() const
{
	const UCharacterMovementComponent* Movement = GetCharacterMovement();
	return bIsCurrentlyControlled && Movement && Movement->IsMovingOnGround() && !IsDeadState() && !IsHurt() && !bIsRolling && !bIsAttacking && !bIsShieldingState && !bShieldBlockSuccessState;
}

void ABasePaperPlayer::StartRoll()
{
	if (!CanStartRoll())
	{
		return;
	}

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Movement)
	{
		return;
	}

	bIsRolling = true;
	bIsWallSliding = false;

	RollDirection = FMath::IsNearlyZero(LastMoveInput) ? (IsFacingRight() ? 1.0f : -1.0f) : FMath::Sign(LastMoveInput);
	SetFacingDirection(RollDirection > 0.0f);
	FacingDirection = RollDirection > 0.0f ? 0.0f : 180.0f;

	CachedGroundFriction = Movement->GroundFriction;
	CachedBrakingFrictionFactor = Movement->BrakingFrictionFactor;
	CachedBrakingDecelerationWalking = Movement->BrakingDecelerationWalking;

	Movement->GroundFriction = 0.0f;
	Movement->BrakingFrictionFactor = 0.0f;
	Movement->BrakingDecelerationWalking = 0.0f;

	LaunchCharacter(FVector(RollDirection * RollLaunchSpeed, 0.0f, 0.0f), true, false);
	PlayCharacterSound(RollSound);

	GetWorldTimerManager().ClearTimer(RollTimerHandle);
	GetWorldTimerManager().SetTimer(RollTimerHandle, this, &ABasePaperPlayer::EndRoll, RollDuration, false);
}

void ABasePaperPlayer::EndRoll()
{
	bIsRolling = false;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->GroundFriction = CachedGroundFriction;
		Movement->BrakingFrictionFactor = CachedBrakingFrictionFactor;
		Movement->BrakingDecelerationWalking = CachedBrakingDecelerationWalking;
	}
}

bool ABasePaperPlayer::CanStartAttack() const
{
	const UCharacterMovementComponent* Movement = GetCharacterMovement();
	return bIsCurrentlyControlled
		&& Movement
		&& !IsDeadState()
		&& !IsHurt()
		&& !bIsRolling
		&& !bIsWallSliding
		&& !bIsShieldingState
		&& !bShieldBlockSuccessState;
}

void ABasePaperPlayer::StartAttack()
{
	if (!CanStartAttack())
	{
		return;
	}

	if (bIsAttacking)
	{
		const int32 RemainingComboSlots = 3 - (CurrentAttackIndex + PendingAttackInputs);
		if (RemainingComboSlots > 0)
		{
			++PendingAttackInputs;
		}
		return;
	}

	const bool bCanContinueCombo =
		GetWorldTimerManager().IsTimerActive(ComboResetTimerHandle) &&
		CurrentAttackIndex > 0 &&
		CurrentAttackIndex < 3;

	GetWorldTimerManager().ClearTimer(ComboResetTimerHandle);

	BeginAttack(bCanContinueCombo ? CurrentAttackIndex + 1 : 1);
}

void ABasePaperPlayer::BeginAttack(int32 AttackIndex)
{
	UPaperFlipbook* AttackFlipbook = GetAttackFlipbookForIndex(AttackIndex);
	if (!AttackFlipbook)
	{
		return;
	}

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Movement)
	{
		return;
	}

	bIsAttacking = true;
	CurrentAttackIndex = AttackIndex;

	HitActorsThisAttack.Reset();
	DisableAttackHitbox();
	UpdateAttackHitboxTransform();

	if (Movement->IsMovingOnGround() && FMath::IsNearlyZero(LastMoveInput))
	{
		FVector NewVelocity = Movement->Velocity;
		NewVelocity.X = 0.0f;
		Movement->Velocity = NewVelocity;
	}

	PlayAttackSoundForIndex(AttackIndex);

	const float AttackDuration = FMath::Max(AttackFlipbook->GetTotalDuration(), 0.01f);
	const float HitStartTime = FMath::Clamp(GetAttackHitStartTimeForIndex(AttackIndex), 0.0f, AttackDuration);
	const float HitEndTime = FMath::Clamp(GetAttackHitEndTimeForIndex(AttackIndex), HitStartTime, AttackDuration);

	GetWorldTimerManager().ClearTimer(AttackTimerHandle);
	GetWorldTimerManager().ClearTimer(AttackHitboxStartTimerHandle);
	GetWorldTimerManager().ClearTimer(AttackHitboxEndTimerHandle);

	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &ABasePaperPlayer::FinishAttack, AttackDuration, false);
	GetWorldTimerManager().SetTimer(AttackHitboxStartTimerHandle, this, &ABasePaperPlayer::EnableAttackHitbox, HitStartTime, false);
	GetWorldTimerManager().SetTimer(AttackHitboxEndTimerHandle, this, &ABasePaperPlayer::DisableAttackHitbox, HitEndTime, false);
}

void ABasePaperPlayer::FinishAttack()
{
	bIsAttacking = false;
	DisableAttackHitbox();

	GetWorldTimerManager().ClearTimer(AttackTimerHandle);
	GetWorldTimerManager().ClearTimer(AttackHitboxStartTimerHandle);
	GetWorldTimerManager().ClearTimer(AttackHitboxEndTimerHandle);

	if (PendingAttackInputs > 0 && CurrentAttackIndex < 3)
	{
		--PendingAttackInputs;
		BeginAttack(CurrentAttackIndex + 1);
		return;
	}

	PendingAttackInputs = 0;
	GetWorldTimerManager().ClearTimer(ComboResetTimerHandle);
	GetWorldTimerManager().SetTimer(ComboResetTimerHandle, this, &ABasePaperPlayer::ResetAttackCombo, ComboResetTime, false);
}

void ABasePaperPlayer::ResetAttackCombo()
{
	if (bIsAttacking)
	{
		return;
	}

	CurrentAttackIndex = 0;
	PendingAttackInputs = 0;
}

void ABasePaperPlayer::StartShield()
{
	if (!bIsCurrentlyControlled)
	{
		return;
	}

	if (!CanStartShield())
	{
		return;
	}

	const bool bWasShielding = bIsShieldingState;

	bWantsToShield = true;
	bIsShieldingState = true;
	bShieldBlockSuccessState = false;

	if (!bWasShielding)
	{
		PlayCharacterSound(ShieldStartSound);
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		FVector NewVelocity = Movement->Velocity;
		NewVelocity.X = 0.0f;
		Movement->Velocity = NewVelocity;
	}
}

void ABasePaperPlayer::StopShield()
{
	bWantsToShield = false;

	if (!bShieldBlockSuccessState)
	{
		bIsShieldingState = false;
	}
}

void ABasePaperPlayer::UpdateShieldState()
{
	if (!bIsCurrentlyControlled)
	{
		bWantsToShield = false;
		bIsShieldingState = false;
		bShieldBlockSuccessState = false;
		return;
	}

	if (bShieldOnCooldown)
	{
		bWantsToShield = false;

		if (!bShieldBlockSuccessState)
		{
			bIsShieldingState = false;
		}

		return;
	}

	if (!CanStartShield())
	{
		if (!bShieldBlockSuccessState)
		{
			bIsShieldingState = false;
		}
		return;
	}

	if (bWantsToShield && !bShieldBlockSuccessState)
	{
		bIsShieldingState = true;
	}
	else if (!bWantsToShield && !bShieldBlockSuccessState)
	{
		bIsShieldingState = false;
	}

	if (bIsShieldingState)
	{
		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			FVector NewVelocity = Movement->Velocity;
			NewVelocity.X = 0.0f;
			Movement->Velocity = NewVelocity;
		}
	}
}

void ABasePaperPlayer::EndShieldBlockSuccess()
{
	bShieldBlockSuccessState = false;
	bIsShieldingState = false;
}

void ABasePaperPlayer::EndShieldCooldown()
{
	bShieldOnCooldown = false;
}

void ABasePaperPlayer::EnableAttackHitbox()
{
	if (!AttackHitbox || !bIsAttacking || IsDeadState())
	{
		return;
	}

	bAttackHitboxActive = true;
	HitActorsThisAttack.Reset();
	UpdateAttackHitboxTransform();
	AttackHitbox->SetBoxExtent(AttackHitboxBoxExtent);
	AttackHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void ABasePaperPlayer::DisableAttackHitbox()
{
	bAttackHitboxActive = false;

	if (AttackHitbox)
	{
		AttackHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	HitActorsThisAttack.Reset();
}

void ABasePaperPlayer::UpdateAttackHitboxTransform()
{
	if (!AttackHitbox)
	{
		return;
	}

	const float DirectionSign = IsFacingRight() ? 1.0f : -1.0f;
	const FVector LocalOffset(
		AttackHitboxLocalOffset.X + (AttackHitboxForwardOffset * DirectionSign),
		AttackHitboxLocalOffset.Y,
		AttackHitboxLocalOffset.Z
	);

	AttackHitbox->SetRelativeLocation(LocalOffset);
	AttackHitbox->SetBoxExtent(AttackHitboxBoxExtent);
}

void ABasePaperPlayer::CancelAttackState()
{
	bIsAttacking = false;
	bAttackHitboxActive = false;
	CurrentAttackIndex = 0;
	PendingAttackInputs = 0;
	UpdateAttackMoveSpeed();

	DisableAttackHitbox();

	GetWorldTimerManager().ClearTimer(AttackTimerHandle);
	GetWorldTimerManager().ClearTimer(AttackHitboxStartTimerHandle);
	GetWorldTimerManager().ClearTimer(AttackHitboxEndTimerHandle);
	GetWorldTimerManager().ClearTimer(ComboResetTimerHandle);
}

void ABasePaperPlayer::SetPartner(ABasePaperPlayer* NewPartner)
{
	PartnerCharacter = NewPartner;
}

void ABasePaperPlayer::StartFollowing()
{
	bIsCurrentlyControlled = false;
	ResetFootstepSoundState();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		FVector NewVelocity = Movement->Velocity;
		NewVelocity.X = 0.0f;
		Movement->Velocity = NewVelocity;

		Movement->MaxWalkSpeed = DefaultWalkSpeed;
		Movement->bRunPhysicsWithNoController = true;
		Movement->bCheatFlying = false;

		if (Movement->MovementMode == MOVE_Flying)
		{
			Movement->SetMovementMode(MOVE_Falling);
		}
	}
}

void ABasePaperPlayer::StopFollowing()
{
	bIsCurrentlyControlled = true;
	ResetFootstepSoundState();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		FVector NewVelocity = Movement->Velocity;
		NewVelocity.X = 0.0f;
		Movement->Velocity = NewVelocity;
	}
}

void ABasePaperPlayer::InitializeSwapPartners()
{
	if (PartnerCharacter)
	{
		return;
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABasePaperPlayer::StaticClass(), FoundActors);

	if (FoundActors.Num() < 2)
	{
		return;
	}

	for (AActor* Actor : FoundActors)
	{
		ABasePaperPlayer* OtherPlayer = Cast<ABasePaperPlayer>(Actor);
		if (OtherPlayer && OtherPlayer != this)
		{
			PartnerCharacter = OtherPlayer;

			if (!OtherPlayer->PartnerCharacter)
			{
				OtherPlayer->PartnerCharacter = this;
			}
			break;
		}
	}
}

void ABasePaperPlayer::SwitchCharacters()
{
	if (!bIsCurrentlyControlled)
	{
		return;
	}

	if (!PartnerCharacter || PartnerCharacter == this || PartnerCharacter->IsDeadState())
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}

	UCharacterMovementComponent* CurrentMovement = GetCharacterMovement();
	UCharacterMovementComponent* PartnerMovement = PartnerCharacter->GetCharacterMovement();

	if (!CurrentMovement || !PartnerMovement)
	{
		return;
	}

	if (!CurrentMovement->IsMovingOnGround() || CurrentMovement->IsFalling())
	{
		return;
	}

	if (!PartnerMovement->IsMovingOnGround() || PartnerMovement->IsFalling())
	{
		return;
	}

	FVector CurrentVelocity = CurrentMovement->Velocity;
	CurrentVelocity.X = 0.0f;
	CurrentMovement->Velocity = CurrentVelocity;

	CurrentMovement->MaxWalkSpeed = DefaultWalkSpeed;
	CurrentMovement->StopMovementImmediately();

	FVector PartnerVelocity = PartnerMovement->Velocity;
	PartnerVelocity.X = 0.0f;
	PartnerMovement->Velocity = PartnerVelocity;
	PartnerMovement->MaxWalkSpeed = PartnerCharacter->DefaultWalkSpeed;

	PlayCharacterSound(SwitchCharacterSound);

	PlayerController->Possess(PartnerCharacter);

	if (FacingDirection == 180.0f)
	{
		PartnerCharacter->SetFacingDirection(false);
		PartnerCharacter->FacingDirection = 180.0f;
	}
	else
	{
		PartnerCharacter->SetFacingDirection(true);
		PartnerCharacter->FacingDirection = 0.0f;
	}

	CurrentMovement->bCheatFlying = false;

	if (CurrentMovement->MovementMode == MOVE_Flying)
	{
		CurrentMovement->SetMovementMode(MOVE_Falling);
	}
}

void ABasePaperPlayer::ApplyCameraSettings()
{
	if (SpringArm)
	{
		SpringArm->TargetArmLength = SpringArmLength;
		SpringArm->SetRelativeRotation(SpringArmRotation);
		SpringArm->SocketOffset = CameraSocketOffset;
	}

	if (Camera)
	{
		Camera->ProjectionMode = ECameraProjectionMode::Perspective;
		Camera->FieldOfView = CameraFieldOfView;
	}
}

void ABasePaperPlayer::HandleAttackHitboxBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!bAttackHitboxActive || !OtherActor || OtherActor == this || IsDeadState())
	{
		return;
	}

	if (OtherActor->IsA(ABasePaperPlayer::StaticClass()))
	{
		return;
	}

	const TWeakObjectPtr<AActor> OtherActorPtr(OtherActor);
	if (HitActorsThisAttack.Contains(OtherActorPtr))
	{
		return;
	}

	HitActorsThisAttack.Add(OtherActorPtr);

	const float Damage = GetAttackDamageForIndex(CurrentAttackIndex);
	if (Damage <= 0.0f)
	{
		return;
	}

	UGameplayStatics::ApplyDamage(OtherActor, Damage, GetController(), this, UDamageType::StaticClass());
}

void ABasePaperPlayer::UpdateFollow()
{
	if (!PartnerCharacter)
	{
		return;
	}

	if (PartnerCharacter == this || PartnerCharacter->IsDeadState() || IsDeadState())
	{
		return;
	}

	UCharacterMovementComponent* MyMovement = GetCharacterMovement();
	UCharacterMovementComponent* PartnerMovement = PartnerCharacter->GetCharacterMovement();

	if (!MyMovement || !PartnerMovement)
	{
		return;
	}

	const FVector PartnerLocation = PartnerCharacter->GetActorLocation();
	const FVector MyLocation = GetActorLocation();

	const float XDifference = PartnerLocation.X - MyLocation.X;
	const float AbsXDifference = FMath::Abs(XDifference);

	if (AbsXDifference > FollowDistance)
	{
		const float Direction = FMath::Sign(XDifference);
		const float FollowMoveSpeed = PartnerMovement->Velocity.Size2D() > 10.0f ? PartnerMovement->Velocity.Size2D() : DefaultWalkSpeed;

		MyMovement->MaxWalkSpeed = FollowMoveSpeed;

		FVector NewVelocity = MyMovement->Velocity;
		NewVelocity.X = Direction * FollowMoveSpeed;
		MyMovement->Velocity = NewVelocity;

		LastMoveInput = Direction;

		if (Direction > 0.0f)
		{
			FacingDirection = 0.0f;
			SetFacingDirection(true);
		}
		else if (Direction < 0.0f)
		{
			FacingDirection = 180.0f;
			SetFacingDirection(false);
		}
	}
	else
	{
		FVector NewVelocity = MyMovement->Velocity;
		NewVelocity.X = 0.0f;
		MyMovement->Velocity = NewVelocity;
		MyMovement->MaxWalkSpeed = DefaultWalkSpeed;
		LastMoveInput = 0.0f;
	}

	if ((PartnerMovement->IsFalling() || PartnerMovement->Velocity.Z > 100.0f) &&
		!MyMovement->IsFalling() &&
		MyMovement->IsMovingOnGround() &&
		AbsXDifference < FollowDistance * 2.5f)
	{
		Jump();

		if (!bJumpSoundPlayedThisAirTime)
		{
			PlayJumpSound();
			bJumpSoundPlayedThisAirTime = true;
		}
	}
}

void ABasePaperPlayer::UpdateWallSlide()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Movement || bIsRolling || bIsAttacking || bIsShieldingState || bShieldBlockSuccessState || IsDeadState() || IsHurt())
	{
		bIsWallSliding = false;
		return;
	}

	const bool bDescending = Movement->IsFalling() && Movement->Velocity.Z < 0.0f;
	const bool bHasHorizontalIntent = !FMath::IsNearlyZero(LastMoveInput, 0.1f);

	if (!bDescending || !bHasHorizontalIntent)
	{
		bIsWallSliding = false;
		return;
	}

	const float CheckDirection = FMath::Sign(LastMoveInput);
	const bool bTouchingWall = IsTouchingWallInDirection(CheckDirection);

	bIsWallSliding = bTouchingWall;

	if (!bIsWallSliding)
	{
		return;
	}

	SetFacingDirection(CheckDirection > 0.0f);
	FacingDirection = CheckDirection > 0.0f ? 0.0f : 180.0f;

	FVector Velocity = Movement->Velocity;
	Velocity.Z = FMath::Max(Velocity.Z, -WallSlideMaxFallSpeed);
	Movement->Velocity = Velocity;
}

bool ABasePaperPlayer::IsTouchingWallInDirection(float Direction) const
{
	if (FMath::IsNearlyZero(Direction))
	{
		return false;
	}

	const UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (!Capsule || !GetWorld())
	{
		return false;
	}

	const float TraceDistance = Capsule->GetScaledCapsuleRadius() + WallCheckDistance;
	const FVector Start = GetActorLocation();
	const FVector End = Start + FVector(Direction * TraceDistance, 0.0f, 0.0f);

	FHitResult Hit;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WallSlideTrace), false, this);

	return GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams);
}

UPaperFlipbook* ABasePaperPlayer::GetAttackFlipbookForIndex(int32 AttackIndex) const
{
	switch (AttackIndex)
	{
	case 1:
		return Attack1Flipbook;

	case 2:
		return Attack2Flipbook;

	case 3:
		return Attack3Flipbook;

	default:
		return nullptr;
	}
}

float ABasePaperPlayer::GetAttackDamageForIndex(int32 AttackIndex) const
{
	if (b100DamageBoostActive)
	{
		return BoostedAttackDamage;
	}

	switch (AttackIndex)
	{
	case 1:
		return Attack1Damage;

	case 2:
		return Attack2Damage;

	case 3:
		return Attack3Damage;

	default:
		return 0.0f;
	}
}

float ABasePaperPlayer::GetAttackHitStartTimeForIndex(int32 AttackIndex) const
{
	switch (AttackIndex)
	{
	case 1:
		return Attack1HitStartTime;

	case 2:
		return Attack2HitStartTime;

	case 3:
		return Attack3HitStartTime;

	default:
		return 0.0f;
	}
}

float ABasePaperPlayer::GetAttackHitEndTimeForIndex(int32 AttackIndex) const
{
	switch (AttackIndex)
	{
	case 1:
		return Attack1HitEndTime;

	case 2:
		return Attack2HitEndTime;

	case 3:
		return Attack3HitEndTime;

	default:
		return 0.0f;
	}
}

bool ABasePaperPlayer::CanStartShield() const
{
	const UCharacterMovementComponent* Movement = GetCharacterMovement();
	return bIsCurrentlyControlled
		&& Movement
		&& !IsDeadState()
		&& !IsHurt()
		&& !bIsRolling
		&& !bIsAttacking
		&& !bIsWallSliding
		&& !bShieldBlockSuccessState
		&& !bShieldOnCooldown;
}

void ABasePaperPlayer::UpdateAttackMoveSpeed()
{
}

void ABasePaperPlayer::CheckFallDeath()
{
	if (!bIsCurrentlyControlled)
	{
		return;
	}

	if (bHasTriggeredFallDeath || IsDeadState())
	{
		return;
	}

	if (GetActorLocation().Z > FallDeathZ)
	{
		return;
	}

	bHasTriggeredFallDeath = true;
	UGameplayStatics::ApplyDamage(this, 999999.0f, GetController(), this, UDamageType::StaticClass());
}

void ABasePaperPlayer::ShowDeathScreenAndRespawn()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController == nullptr)
	{
		return;
	}

	if (DeathScreenWidgetClass == nullptr)
	{
		return;
	}

	if (DeathScreenWidget == nullptr)
	{
		DeathScreenWidget = CreateWidget<UUserWidget>(PlayerController, DeathScreenWidgetClass);
	}

	if (DeathScreenWidget != nullptr && !DeathScreenWidget->IsInViewport())
	{
		DeathScreenWidget->AddToViewport(100);
	}
}

float ABasePaperPlayer::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser
)
{
	if (bGodModeEnabled)
	{
		return 0.0f;
	}

	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

bool ABasePaperPlayer::IsGodModeEnabled() const
{
	return bGodModeEnabled;
}

void ABasePaperPlayer::ToggleGodMode()
{
	bGodModeEnabled = !bGodModeEnabled;

	if (!bGodModeEnabled && HealthComponent)
	{
		HealthComponent->SetCanTakeDamage(true);
	}

	if (GEngine)
	{
		const FColor MessageColor = bGodModeEnabled ? FColor::Green : FColor::Red;
		const TCHAR* MessageText = bGodModeEnabled ? TEXT("God Mode ON") : TEXT("God Mode OFF");
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, MessageColor, MessageText);
	}
}

void ABasePaperPlayer::RefillHealthToFull()
{
	if (!HealthComponent)
	{
		return;
	}

	HealthComponent->RefillToFull();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Health Refilled"));
	}
}

bool ABasePaperPlayer::IsAttackHitboxActive() const
{
	return bAttackHitboxActive;
}
