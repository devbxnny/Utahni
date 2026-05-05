#include "Actors/BasePaperGoblin.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PaperFlipbookComponent.h"

ABasePaperGoblin::ABasePaperGoblin()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABasePaperGoblin::BeginPlay()
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

	ChangeState(EGoblinState::Idle);
}

void ABasePaperGoblin::Tick(float DeltaSeconds)
{
	ABasePaperCharacter::Tick(DeltaSeconds);

	if (IsDeadState())
	{
		ChangeState(EGoblinState::Dead);
		return;
	}

	if (IsHurt())
	{
		ChangeState(EGoblinState::Hit);
		return;
	}

	UpdateState(DeltaSeconds);
}

bool ABasePaperGoblin::IsAttacking() const
{
	return CurrentState == EGoblinState::KnifeAttack
		|| CurrentState == EGoblinState::DodgeAttack
		|| CurrentState == EGoblinState::BombAttack;
}

int32 ABasePaperGoblin::GetAttackIndex() const
{
	switch (CurrentState)
	{
	case EGoblinState::KnifeAttack:
		return 1;

	case EGoblinState::DodgeAttack:
		return 2;

	case EGoblinState::BombAttack:
		return 3;

	default:
		return 0;
	}
}

bool ABasePaperGoblin::IsHurt() const
{
	if (CurrentState == EGoblinState::KnifeAttack
		|| CurrentState == EGoblinState::DodgeAttack
		|| CurrentState == EGoblinState::BombAttack)
	{
		return false;
	}

	return Super::IsHurt();
}

void ABasePaperGoblin::OnCharacterDamaged(float DamageTaken, AActor* DamageCauser)
{
	if (CurrentState == EGoblinState::KnifeAttack
		|| CurrentState == EGoblinState::DodgeAttack
		|| CurrentState == EGoblinState::BombAttack)
	{
		return;
	}

	Super::OnCharacterDamaged(DamageTaken, DamageCauser);

	AttackTimer = 0.0f;
	bBombSpawned = false;
	ResetAttackHitTracking();
	ChangeState(EGoblinState::Hit);

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}
}

void ABasePaperGoblin::OnCharacterDied(AActor* DamageCauser, AController* InstigatedBy)
{
	Super::OnCharacterDied(DamageCauser, InstigatedBy);

	AttackTimer = 0.0f;
	bBombSpawned = false;
	ResetAttackHitTracking();
	ChangeState(EGoblinState::Dead);
}

void ABasePaperGoblin::UpdateState(float DeltaSeconds)
{
	PlayerRef = UGameplayStatics::GetPlayerPawn(this, 0);

	if (!PlayerRef)
	{
		ChangeState(EGoblinState::Idle);
		UpdateIdle(DeltaSeconds);
		return;
	}

	if (CurrentState == EGoblinState::Hit)
	{
		ChangeState(EGoblinState::Idle);
	}

	switch (CurrentState)
	{
	case EGoblinState::KnifeAttack:
		UpdateKnifeAttack(DeltaSeconds);
		return;

	case EGoblinState::DodgeAttack:
		UpdateDodgeAttack(DeltaSeconds);
		return;

	case EGoblinState::BombAttack:
		UpdateBombAttack(DeltaSeconds);
		return;

	default:
		break;
	}

	const float DistanceX = FMath::Abs(PlayerRef->GetActorLocation().X - GetActorLocation().X);

	if (DistanceX > DetectionRange)
	{
		ChangeState(EGoblinState::Idle);
		UpdateIdle(DeltaSeconds);
		return;
	}

	if (DistanceX <= KnifeRange)
	{
		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}

		TryChooseMeleeAttack();
		return;
	}

	if (DistanceX <= BombRange)
	{
		const float CurrentTime = GetWorld()->GetTimeSeconds();

		if ((CurrentTime - LastBombTime) >= BombCooldown)
		{
			if (UCharacterMovementComponent* Movement = GetCharacterMovement())
			{
				Movement->StopMovementImmediately();
			}

			ResetAttackHitTracking();
			bBombSpawned = false;
			AttackTimer = 0.0f;
			LastBombTime = CurrentTime;
			FacePlayer();
			ChangeState(EGoblinState::BombAttack);
			PlayAttackSoundForIndex(3);
		}
		else
		{
			ChangeState(EGoblinState::Chasing);
			UpdateChase(DeltaSeconds);
		}

		return;
	}

	ChangeState(EGoblinState::Chasing);
	UpdateChase(DeltaSeconds);
}

void ABasePaperGoblin::UpdateIdle(float DeltaSeconds)
{
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}

	DisableAttackHitbox();
}

void ABasePaperGoblin::UpdateChase(float DeltaSeconds)
{
	if (!PlayerRef)
	{
		return;
	}

	DisableAttackHitbox();
	FacePlayer();

	const float DeltaX = PlayerRef->GetActorLocation().X - GetActorLocation().X;
	const float MoveDirection = FMath::Sign(DeltaX);

	if (!FMath::IsNearlyZero(MoveDirection))
	{
		AddMovementInput(FVector(1.0f, 0.0f, 0.0f), MoveDirection);
	}
}

void ABasePaperGoblin::UpdateKnifeAttack(float DeltaSeconds)
{
	AttackTimer += DeltaSeconds;
	FacePlayer();

	if (AttackTimer >= KnifeHitboxStartTime && AttackTimer < KnifeHitboxEndTime)
	{
		EnableAttackHitbox();
	}
	else
	{
		DisableAttackHitbox();
	}

	if (AttackTimer >= KnifeDuration)
	{
		FinishAttack();
	}
}

void ABasePaperGoblin::UpdateDodgeAttack(float DeltaSeconds)
{
	AttackTimer += DeltaSeconds;
	FacePlayer();

	if (AttackTimer >= DodgeHitboxStartTime && AttackTimer < DodgeHitboxEndTime)
	{
		EnableAttackHitbox();
	}
	else
	{
		DisableAttackHitbox();
	}

	if (AttackTimer >= DodgeDuration)
	{
		FinishAttack();
	}
}

void ABasePaperGoblin::UpdateBombAttack(float DeltaSeconds)
{
	AttackTimer += DeltaSeconds;
	FacePlayer();
	DisableAttackHitbox();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}

	if (!bBombSpawned && AttackTimer >= BombSpawnTime)
	{
		SpawnBombProjectile();
		bBombSpawned = true;
	}

	if (AttackTimer >= BombDuration)
	{
		FinishAttack();
	}
}

void ABasePaperGoblin::FinishAttack()
{
	ResetAttackHitTracking();
	bBombSpawned = false;
	AttackTimer = 0.0f;
	ChangeState(EGoblinState::Idle);
}

void ABasePaperGoblin::SpawnBombProjectile()
{
	if (!BombProjectileClass || !PlayerRef || !GetWorld())
	{
		return;
	}

	FVector SpawnLocation = GetActorLocation();
	SpawnLocation.Z += 30.0f;

	const float FacingSign = (GetSprite() && GetSprite()->GetRelativeScale3D().X >= 0.0f) ? 1.0f : -1.0f;
	SpawnLocation.X += FacingSign * 35.0f;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();

	ABaseGoblinBombProjectile* Projectile = GetWorld()->SpawnActor<ABaseGoblinBombProjectile>(
		BombProjectileClass,
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (!Projectile)
	{
		return;
	}

	FVector TargetLocation = PlayerRef->GetActorLocation();
	TargetLocation.Z += 10.0f;

	const FVector ToTarget = TargetLocation - SpawnLocation;

	FVector LaunchVelocity = FVector::ZeroVector;
	LaunchVelocity.X = FMath::Clamp(ToTarget.X * 1.4f, -450.0f, 450.0f);
	LaunchVelocity.Y = 0.0f;
	LaunchVelocity.Z = 260.0f;

	Projectile->LaunchProjectile(LaunchVelocity);
}

void ABasePaperGoblin::TryChooseMeleeAttack()
{
	if (!PlayerRef)
	{
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();

	const bool bCanKnife = (CurrentTime - LastKnifeTime) >= KnifeCooldown;
	const bool bCanDodge = (CurrentTime - LastDodgeTime) >= DodgeCooldown;

	if (!bCanKnife && !bCanDodge)
	{
		ChangeState(EGoblinState::Idle);
		UpdateIdle(0.0f);
		return;
	}

	ResetAttackHitTracking();
	AttackTimer = 0.0f;
	FacePlayer();

	if (bCanKnife && bCanDodge)
	{
		const int32 Choice = FMath::RandRange(0, 1);

		if (Choice == 0)
		{
			LastKnifeTime = CurrentTime;
			ChangeState(EGoblinState::KnifeAttack);
			PlayAttackSoundForIndex(1);
		}
		else
		{
			LastDodgeTime = CurrentTime;
			ChangeState(EGoblinState::DodgeAttack);
			PlayAttackSoundForIndex(2);
		}
	}
	else if (bCanKnife)
	{
		LastKnifeTime = CurrentTime;
		ChangeState(EGoblinState::KnifeAttack);
		PlayAttackSoundForIndex(1);
	}
	else
	{
		LastDodgeTime = CurrentTime;
		ChangeState(EGoblinState::DodgeAttack);
		PlayAttackSoundForIndex(2);
	}
}

void ABasePaperGoblin::FacePlayer()
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

void ABasePaperGoblin::ChangeState(EGoblinState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	CurrentState = NewState;
}