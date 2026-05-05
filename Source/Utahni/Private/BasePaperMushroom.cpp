#include "BasePaperMushroom.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PaperFlipbookComponent.h"

ABasePaperMushroom::ABasePaperMushroom()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABasePaperMushroom::BeginPlay()
{
	Super::BeginPlay();

	PlayerRef = UGameplayStatics::GetPlayerPawn(this, 0);

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = MoveSpeed;
		Movement->bConstrainToPlane = true;
		Movement->SetPlaneConstraintAxisSetting(EPlaneConstraintAxisSetting::Y);
		Movement->bSnapToPlaneAtStart = true;
	}

	ChangeState(EMushroomState::Idle);
}

void ABasePaperMushroom::Tick(float DeltaSeconds)
{
	ABasePaperCharacter::Tick(DeltaSeconds);

	if (IsDeadState())
	{
		ResetFootstepSoundState();
		ChangeState(EMushroomState::Dead);
		return;
	}

	if (IsHurt())
	{
		ResetFootstepSoundState();
		ChangeState(EMushroomState::Hit);
		return;
	}

	UpdateAttackHitboxTransform();
	UpdateState(DeltaSeconds);
}

bool ABasePaperMushroom::IsAttacking() const
{
	return CurrentState == EMushroomState::Punching
		|| CurrentState == EMushroomState::Chomping
		|| CurrentState == EMushroomState::Bursting;
}

int32 ABasePaperMushroom::GetAttackIndex() const
{
	switch (CurrentState)
	{
	case EMushroomState::Punching:
		return 1;

	case EMushroomState::Chomping:
		return 2;

	case EMushroomState::Bursting:
		return 3;

	default:
		return 0;
	}
}

bool ABasePaperMushroom::IsHurt() const
{
	if (CurrentState == EMushroomState::Punching
		|| CurrentState == EMushroomState::Chomping
		|| CurrentState == EMushroomState::Bursting)
	{
		return false;
	}

	return Super::IsHurt();
}

void ABasePaperMushroom::OnCharacterDamaged(float DamageTaken, AActor* DamageCauser)
{
	if (CurrentState == EMushroomState::Punching
		|| CurrentState == EMushroomState::Chomping
		|| CurrentState == EMushroomState::Bursting)
	{
		return;
	}

	ResetFootstepSoundState();
	Super::OnCharacterDamaged(DamageTaken, DamageCauser);

	AttackTimer = 0.0f;
	bProjectileSpawned = false;
	ResetAttackHitTracking();
	ChangeState(EMushroomState::Hit);

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}
}

void ABasePaperMushroom::OnCharacterDied(AActor* DamageCauser, AController* InstigatedBy)
{
	ResetFootstepSoundState();
	Super::OnCharacterDied(DamageCauser, InstigatedBy);

	AttackTimer = 0.0f;
	bProjectileSpawned = false;
	ResetAttackHitTracking();
	ChangeState(EMushroomState::Dead);
}

void ABasePaperMushroom::UpdateState(float DeltaSeconds)
{
	PlayerRef = UGameplayStatics::GetPlayerPawn(this, 0);

	if (!PlayerRef)
	{
		ChangeState(EMushroomState::Idle);
		UpdateIdle(DeltaSeconds);
		return;
	}

	if (CurrentState == EMushroomState::Hit)
	{
		ChangeState(EMushroomState::Idle);
	}

	switch (CurrentState)
	{
	case EMushroomState::Punching:
		UpdatePunch(DeltaSeconds);
		return;

	case EMushroomState::Chomping:
		UpdateChomp(DeltaSeconds);
		return;

	case EMushroomState::Bursting:
		UpdateBurst(DeltaSeconds);
		return;

	default:
		break;
	}

	const float DistanceX = FMath::Abs(PlayerRef->GetActorLocation().X - GetActorLocation().X);

	if (DistanceX > DetectionRange)
	{
		ChangeState(EMushroomState::Idle);
		UpdateIdle(DeltaSeconds);
		return;
	}

	if (DistanceX <= MeleeRange)
	{
		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}

		TryChooseAttack();
		return;
	}

	if (DistanceX <= BurstRange)
	{
		const float CurrentTime = GetWorld()->GetTimeSeconds();

		if ((CurrentTime - LastBurstTime) >= BurstCooldown)
		{
			if (UCharacterMovementComponent* Movement = GetCharacterMovement())
			{
				Movement->StopMovementImmediately();
			}

			ResetAttackHitTracking();
			ResetFootstepSoundState();
			bProjectileSpawned = false;
			AttackTimer = 0.0f;
			LastBurstTime = CurrentTime;
			FacePlayer();
			ChangeState(EMushroomState::Bursting);
			PlayAttackSoundForIndex(3);
		}
		else
		{
			ChangeState(EMushroomState::Chasing);
			UpdateChase(DeltaSeconds);
		}

		return;
	}

	ChangeState(EMushroomState::Chasing);
	UpdateChase(DeltaSeconds);
}

void ABasePaperMushroom::UpdateIdle(float DeltaSeconds)
{
	DisableAttackHitbox();
	ResetFootstepSoundState();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}
}

void ABasePaperMushroom::UpdateChase(float DeltaSeconds)
{
	if (!PlayerRef)
	{
		ResetFootstepSoundState();
		return;
	}

	DisableAttackHitbox();
	FacePlayer();

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Movement)
	{
		ResetFootstepSoundState();
		return;
	}

	const float DeltaX = PlayerRef->GetActorLocation().X - GetActorLocation().X;
	const float MoveDirection = FMath::Sign(DeltaX);

	if (FMath::IsNearlyZero(MoveDirection))
	{
		ResetFootstepSoundState();
		return;
	}

	AddMovementInput(FVector(1.0f, 0.0f, 0.0f), MoveDirection);
	TryPlayFootstepSound(DeltaSeconds, Movement->IsMovingOnGround(), MoveSpeed);
}

void ABasePaperMushroom::TryChooseAttack()
{
	if (!PlayerRef)
	{
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const bool bCanPunch = (CurrentTime - LastPunchTime) >= PunchCooldown;
	const bool bCanChomp = (CurrentTime - LastChompTime) >= ChompCooldown;

	if (!bCanPunch && !bCanChomp)
	{
		ChangeState(EMushroomState::Idle);
		UpdateIdle(0.0f);
		return;
	}

	ResetAttackHitTracking();
	ResetFootstepSoundState();
	bProjectileSpawned = false;
	AttackTimer = 0.0f;
	FacePlayer();

	if (bCanPunch && bCanChomp)
	{
		const int32 Choice = FMath::RandRange(0, 1);

		if (Choice == 0)
		{
			LastPunchTime = CurrentTime;
			ChangeState(EMushroomState::Punching);
			PlayAttackSoundForIndex(1);
		}
		else
		{
			LastChompTime = CurrentTime;
			ChangeState(EMushroomState::Chomping);
			PlayAttackSoundForIndex(2);
		}
	}
	else if (bCanPunch)
	{
		LastPunchTime = CurrentTime;
		ChangeState(EMushroomState::Punching);
		PlayAttackSoundForIndex(1);
	}
	else
	{
		LastChompTime = CurrentTime;
		ChangeState(EMushroomState::Chomping);
		PlayAttackSoundForIndex(2);
	}
}

void ABasePaperMushroom::UpdatePunch(float DeltaSeconds)
{
	AttackTimer += DeltaSeconds;
	FacePlayer();
	ResetFootstepSoundState();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}

	if (AttackTimer >= PunchHitboxStartTime && AttackTimer < PunchHitboxEndTime)
	{
		EnableAttackHitbox();
	}
	else
	{
		DisableAttackHitbox();
	}

	if (AttackTimer >= PunchDuration)
	{
		FinishAttack();
	}
}

void ABasePaperMushroom::UpdateChomp(float DeltaSeconds)
{
	AttackTimer += DeltaSeconds;
	FacePlayer();
	ResetFootstepSoundState();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}

	if (AttackTimer >= ChompHitboxStartTime && AttackTimer < ChompHitboxEndTime)
	{
		EnableAttackHitbox();
	}
	else
	{
		DisableAttackHitbox();
	}

	if (AttackTimer >= ChompDuration)
	{
		FinishAttack();
	}
}

void ABasePaperMushroom::UpdateBurst(float DeltaSeconds)
{
	AttackTimer += DeltaSeconds;
	FacePlayer();
	DisableAttackHitbox();
	ResetFootstepSoundState();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}

	if (!bProjectileSpawned && AttackTimer >= ProjectileSpawnTime)
	{
		SpawnBurstProjectile();
		bProjectileSpawned = true;
	}

	if (AttackTimer >= BurstDuration)
	{
		FinishAttack();
	}
}

void ABasePaperMushroom::FinishAttack()
{
	ResetAttackHitTracking();
	ResetFootstepSoundState();
	bProjectileSpawned = false;
	AttackTimer = 0.0f;
	ChangeState(EMushroomState::Idle);
}

void ABasePaperMushroom::FacePlayer()
{
	if (!PlayerRef || IsDeadState() || IsHurt())
	{
		return;
	}

	const float DirectionX = PlayerRef->GetActorLocation().X - GetActorLocation().X;
	if (!FMath::IsNearlyZero(DirectionX))
	{
		SetFacingDirection(DirectionX > 0.0f);
	}
}

void ABasePaperMushroom::SpawnBurstProjectile()
{
	if (!ProjectileClass || !PlayerRef || !GetWorld())
	{
		return;
	}

	FVector SpawnLocation = GetActorLocation();
	SpawnLocation.Z += 95.0f;

	const float FacingSign = (GetSprite() && GetSprite()->GetRelativeScale3D().X >= 0.0f) ? 1.0f : -1.0f;
	SpawnLocation.X += FacingSign * 8.0f;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();

	ABaseMushroomProjectile* Projectile = GetWorld()->SpawnActor<ABaseMushroomProjectile>(
		ProjectileClass,
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (!Projectile)
	{
		return;
	}

	FVector TargetLocation = PlayerRef->GetActorLocation();
	TargetLocation.Z += 20.0f;

	if (UCharacterMovementComponent* MoveComp = PlayerRef->FindComponentByClass<UCharacterMovementComponent>())
	{
		const float LeadTime = 0.08f;
		TargetLocation += MoveComp->Velocity * LeadTime;
		TargetLocation.Y = SpawnLocation.Y;
	}

	FVector TossVelocity;
	const bool bFoundVelocity = UGameplayStatics::SuggestProjectileVelocity(
		this,
		TossVelocity,
		SpawnLocation,
		TargetLocation,
		900.0f,
		false,
		0.0f,
		0.0f,
		ESuggestProjVelocityTraceOption::DoNotTrace
	);

	if (!bFoundVelocity)
	{
		const float DeltaX = TargetLocation.X - SpawnLocation.X;
		TossVelocity = FVector(
			FMath::Clamp(DeltaX * 2.4f, -700.0f, 700.0f),
			0.0f,
			420.0f
		);
	}

	TossVelocity.Y = 0.0f;
	Projectile->LaunchProjectile(TossVelocity);
}

void ABasePaperMushroom::ChangeState(EMushroomState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	CurrentState = NewState;
}