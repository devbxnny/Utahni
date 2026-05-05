#include "BasePaperCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PaperFlipbook.h"
#include "PaperFlipbookComponent.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

ABasePaperCharacter::ABasePaperCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void ABasePaperCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (UPaperFlipbookComponent* SpriteComponent = GetSprite())
	{
		InitialSpriteScale = SpriteComponent->GetRelativeScale3D();
		bFacingRight = InitialSpriteScale.X >= 0.0f;
	}

	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &ABasePaperCharacter::HandleHealthChanged);
		HealthComponent->OnDeath.AddDynamic(this, &ABasePaperCharacter::HandleDeath);
	}

	UpdateFacing();
	UpdateAnimation();
}

UHealthComponent* ABasePaperCharacter::GetHealthComponent() const
{
	return HealthComponent;
}

void ABasePaperCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateFacing();
	UpdateAnimation();
}

float ABasePaperCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageAmount <= 0.0f || bIsDead)
	{
		return 0.0f;
	}

	if (IsShielding() && CanBlockDamageFrom(DamageCauser))
	{
		OnShieldBlocked(DamageAmount, DamageCauser, EventInstigator);
		return 0.0f;
	}

	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

bool ABasePaperCharacter::CanBlockDamageFrom(AActor* DamageCauser) const
{
	return true;
}

void ABasePaperCharacter::RespawnAtTransform(const FTransform& RespawnTransform)
{
	GetWorldTimerManager().ClearTimer(HurtTimerHandle);
	GetWorldTimerManager().ClearTimer(InvincibilityTimerHandle);

	bIsDead = false;
	bIsHurt = false;
	FootstepSoundTimer = 0.0f;

	if (HealthComponent)
	{
		HealthComponent->ResetHealth();
	}

	ResetAnimationState();

	SetActorLocationAndRotation(
		RespawnTransform.GetLocation(),
		RespawnTransform.GetRotation().Rotator(),
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->SetMovementMode(MOVE_Walking);
	}

	OnCharacterRespawned();
	UpdateFacing();
	UpdateAnimation();
}

bool ABasePaperCharacter::IsRolling() const
{
	return false;
}

bool ABasePaperCharacter::IsWallSliding() const
{
	return false;
}

bool ABasePaperCharacter::IsAttacking() const
{
	return false;
}

int32 ABasePaperCharacter::GetAttackIndex() const
{
	return 0;
}

bool ABasePaperCharacter::IsShielding() const
{
	return false;
}

bool ABasePaperCharacter::IsShieldBlockSuccess() const
{
	return false;
}

bool ABasePaperCharacter::IsHurt() const
{
	return bIsHurt;
}

bool ABasePaperCharacter::IsDeadState() const
{
	return bIsDead;
}

void ABasePaperCharacter::OnCharacterRespawned()
{
}

bool ABasePaperCharacter::IsFacingRight() const
{
	return bFacingRight;
}

void ABasePaperCharacter::OnCharacterDamaged(float DamageTaken, AActor* DamageCauser)
{
}

void ABasePaperCharacter::OnCharacterDied(AActor* DamageCauser, AController* InstigatedBy)
{
}

void ABasePaperCharacter::OnShieldBlocked(float BlockedDamage, AActor* DamageCauser, AController* InstigatedBy)
{
}

void ABasePaperCharacter::HandleHealthChanged(
	UHealthComponent* InHealthComponent,
	float NewHealth,
	float InMaxHealth,
	float HealthDelta,
	AActor* DamageCauser
)
{
	if (HealthDelta >= 0.0f || bIsDead)
	{
		return;
	}

	bIsHurt = true;
	ResetFootstepSoundState();

	GetWorldTimerManager().ClearTimer(HurtTimerHandle);
	if (HurtDuration > 0.0f)
	{
		GetWorldTimerManager().SetTimer(HurtTimerHandle, this, &ABasePaperCharacter::ClearHurtState, HurtDuration, false);
	}
	else
	{
		ClearHurtState();
	}

	if (HealthComponent)
	{
		HealthComponent->SetCanTakeDamage(false);
	}

	GetWorldTimerManager().ClearTimer(InvincibilityTimerHandle);
	if (InvincibilityDuration > 0.0f)
	{
		GetWorldTimerManager().SetTimer(InvincibilityTimerHandle, this, &ABasePaperCharacter::ClearInvincibility, InvincibilityDuration, false);
	}
	else
	{
		ClearInvincibility();
	}

	if (NewHealth > 0.0f)
	{
		PlayCharacterSound(HurtSound);
	}

	OnCharacterDamaged(-HealthDelta, DamageCauser);
}

void ABasePaperCharacter::HandleDeath(UHealthComponent* InHealthComponent, AActor* DamageCauser, AController* InstigatedBy)
{
	bIsDead = true;
	bIsHurt = false;
	ResetFootstepSoundState();

	GetWorldTimerManager().ClearTimer(HurtTimerHandle);
	GetWorldTimerManager().ClearTimer(InvincibilityTimerHandle);

	if (HealthComponent)
	{
		HealthComponent->SetCanTakeDamage(false);
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	PlayCharacterSound(DeathSound);
	OnCharacterDied(DamageCauser, InstigatedBy);
}

void ABasePaperCharacter::ClearHurtState()
{
	if (!bIsDead)
	{
		bIsHurt = false;
	}
}

void ABasePaperCharacter::ClearInvincibility()
{
	if (HealthComponent && !bIsDead)
	{
		HealthComponent->SetCanTakeDamage(true);
	}
}

void ABasePaperCharacter::SetFacingDirection(bool bNewFacingRight)
{
	UPaperFlipbookComponent* SpriteComponent = GetSprite();
	if (!SpriteComponent || bFacingRight == bNewFacingRight || bIsDead)
	{
		return;
	}

	bFacingRight = bNewFacingRight;

	FVector NewScale = InitialSpriteScale;
	NewScale.X = FMath::Abs(InitialSpriteScale.X) * (bFacingRight ? 1.0f : -1.0f);
	SpriteComponent->SetRelativeScale3D(NewScale);
}

void ABasePaperCharacter::UpdateFacing()
{
	UPaperFlipbookComponent* SpriteComponent = GetSprite();
	if (!SpriteComponent || bIsDead || bIsHurt || IsShielding() || IsShieldBlockSuccess())
	{
		return;
	}

	const float HorizontalVelocity = GetVelocity().X;
	if (FMath::Abs(HorizontalVelocity) < FacingThreshold)
	{
		return;
	}

	SetFacingDirection(HorizontalVelocity > 0.0f);
}

void ABasePaperCharacter::UpdateAnimation()
{
	UPaperFlipbookComponent* SpriteComponent = GetSprite();
	UCharacterMovementComponent* Movement = GetCharacterMovement();

	if (!SpriteComponent || !Movement)
	{
		return;
	}

	if (IsDeadState() && DeathFlipbook)
	{
		SetAnimation(DeathFlipbook, EPaperAnimState::Death, false);
		return;
	}

	if (IsHurt() && HurtFlipbook)
	{
		SetAnimation(HurtFlipbook, EPaperAnimState::Hurt, false);
		return;
	}

	if (IsShieldBlockSuccess() && ShieldBlockSuccessFlipbook)
	{
		SetAnimation(ShieldBlockSuccessFlipbook, EPaperAnimState::ShieldBlockSuccess, false);
		return;
	}

	if (IsShielding() && ShieldFlipbook)
	{
		SetAnimation(ShieldFlipbook, EPaperAnimState::Shield, true);
		return;
	}

	if (IsRolling() && RollFlipbook)
	{
		SetAnimation(RollFlipbook, EPaperAnimState::Roll, false);
		return;
	}

	if (IsAttacking())
	{
		switch (GetAttackIndex())
		{
		case 1:
			if (Attack1Flipbook)
			{
				SetAnimation(Attack1Flipbook, EPaperAnimState::Attack1, false);
				return;
			}
			break;

		case 2:
			if (Attack2Flipbook)
			{
				SetAnimation(Attack2Flipbook, EPaperAnimState::Attack2, false);
				return;
			}
			break;

		case 3:
			if (Attack3Flipbook)
			{
				SetAnimation(Attack3Flipbook, EPaperAnimState::Attack3, false);
				return;
			}
			break;

		default:
			break;
		}
	}

	if (IsWallSliding() && WallSlideFlipbook)
	{
		SetAnimation(WallSlideFlipbook, EPaperAnimState::WallSlide, true);
		return;
	}

	if (Movement->IsFalling())
	{
		const float VerticalVelocity = GetVelocity().Z;

		if (VerticalVelocity >= 0.0f)
		{
			SetAnimation(JumpFlipbook, EPaperAnimState::Jump, false);
		}
		else
		{
			SetAnimation(FallFlipbook, EPaperAnimState::Fall, false);
		}

		return;
	}

	const float HorizontalSpeed = FMath::Abs(GetVelocity().X);

	if (HorizontalSpeed > RunSpeedThreshold)
	{
		SetAnimation(RunFlipbook, EPaperAnimState::Run, true);
	}
	else
	{
		SetAnimation(IdleFlipbook, EPaperAnimState::Idle, true);
	}
}

void ABasePaperCharacter::SetAnimation(UPaperFlipbook* NewFlipbook, EPaperAnimState NewState, bool bLooping)
{
	UPaperFlipbookComponent* SpriteComponent = GetSprite();
	if (!SpriteComponent || !NewFlipbook)
	{
		return;
	}

	if (CurrentAnimState == NewState && CurrentFlipbook == NewFlipbook)
	{
		return;
	}

	CurrentAnimState = NewState;
	CurrentFlipbook = NewFlipbook;

	SpriteComponent->SetLooping(bLooping);
	SpriteComponent->SetFlipbook(NewFlipbook);
	SpriteComponent->PlayFromStart();
}

void ABasePaperCharacter::ResetAnimationState()
{
	CurrentFlipbook = nullptr;
	CurrentAnimState = EPaperAnimState::None;
}

void ABasePaperCharacter::PlayCharacterSound(USoundBase* SoundToPlay) const
{
	if (!SoundToPlay || !GetRootComponent())
	{
		return;
	}

	UGameplayStatics::SpawnSoundAttached(SoundToPlay, GetRootComponent());
}

USoundBase* ABasePaperCharacter::GetAttackSoundForIndex(int32 AttackIndex) const
{
	switch (AttackIndex)
	{
	case 1:
		return Attack1Sound;

	case 2:
		return Attack2Sound;

	case 3:
		return Attack3Sound;

	default:
		return nullptr;
	}
}

void ABasePaperCharacter::PlayAttackSoundForIndex(int32 AttackIndex) const
{
	PlayCharacterSound(GetAttackSoundForIndex(AttackIndex));
}

void ABasePaperCharacter::PlayJumpSound() const
{
	PlayCharacterSound(JumpSound);
}

void ABasePaperCharacter::TryPlayFootstepSound(float DeltaSeconds, bool bIsMovingOnGround, float HorizontalSpeed)
{
	if (!FootstepSound || FootstepInterval <= 0.0f)
	{
		return;
	}

	if (!bIsMovingOnGround || HorizontalSpeed <= RunSpeedThreshold)
	{
		ResetFootstepSoundState();
		return;
	}

	FootstepSoundTimer -= DeltaSeconds;
	if (FootstepSoundTimer > 0.0f)
	{
		return;
	}

	PlayCharacterSound(FootstepSound);
	FootstepSoundTimer = FootstepInterval;
}

void ABasePaperCharacter::ResetFootstepSoundState()
{
	FootstepSoundTimer = 0.0f;
}