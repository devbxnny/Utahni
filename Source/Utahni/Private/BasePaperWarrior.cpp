#include "BasePaperWarrior.h"
#include "BasePaperPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ABasePaperWarrior::ABasePaperWarrior()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABasePaperWarrior::BeginPlay()
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

	ChangeState(EWarriorState::Idle);
}

void ABasePaperWarrior::Tick(float DeltaSeconds)
{
	ABasePaperCharacter::Tick(DeltaSeconds);

	if (IsDeadState())
	{
		ChangeState(EWarriorState::Dead);
		return;
	}

	if (IsHurt())
	{
		ChangeState(EWarriorState::Hit);
		return;
	}

	UpdateAttackHitboxTransform();
	UpdateState(DeltaSeconds);
}

bool ABasePaperWarrior::IsAttacking() const
{
	return CurrentState == EWarriorState::Attack1
		|| CurrentState == EWarriorState::Attack2
		|| CurrentState == EWarriorState::Attack3
		|| CurrentState == EWarriorState::Attack4Jump;
}

int32 ABasePaperWarrior::GetAttackIndex() const
{
	switch (CurrentState)
	{
	case EWarriorState::Attack1:
		return 1;

	case EWarriorState::Attack2:
		return 2;

	case EWarriorState::Attack3:
		return 3;

	case EWarriorState::Attack4Jump:
		return 3;

	default:
		return 0;
	}
}

bool ABasePaperWarrior::IsHurt() const
{
	if (CurrentState == EWarriorState::Attack1
		|| CurrentState == EWarriorState::Attack2
		|| CurrentState == EWarriorState::Attack3
		|| CurrentState == EWarriorState::Attack4Jump)
	{
		return false;
	}

	return Super::IsHurt();
}

void ABasePaperWarrior::OnCharacterDamaged(float DamageTaken, AActor* DamageCauser)
{
	if (CurrentState == EWarriorState::Attack1
		|| CurrentState == EWarriorState::Attack2
		|| CurrentState == EWarriorState::Attack3
		|| CurrentState == EWarriorState::Attack4Jump)
	{
		return;
	}

	ResetFootstepSoundState();
	Super::OnCharacterDamaged(DamageTaken, DamageCauser);
	ChangeState(EWarriorState::Hit);
}

void ABasePaperWarrior::OnCharacterDied(AActor* DamageCauser, AController* InstigatedBy)
{
	ResetFootstepSoundState();
	Super::OnCharacterDied(DamageCauser, InstigatedBy);

	DisableAttackHitbox();
	ActionTimer = 0.0f;
	bJumpAttackLaunched = false;
	ChangeState(EWarriorState::Dead);
}

void ABasePaperWarrior::UpdateState(float DeltaSeconds)
{
	PlayerRef = UGameplayStatics::GetPlayerPawn(this, 0);

	if (!PlayerRef)
	{
		ChangeState(EWarriorState::Idle);
		UpdateIdle();
		return;
	}

	switch (CurrentState)
	{
	case EWarriorState::Attack1:
		UpdateAttack1(DeltaSeconds);
		return;

	case EWarriorState::Attack2:
		UpdateAttack2(DeltaSeconds);
		return;

	case EWarriorState::Attack3:
		UpdateAttack3(DeltaSeconds);
		return;

	case EWarriorState::Attack4Jump:
		UpdateAttack4Jump(DeltaSeconds);
		return;

	case EWarriorState::Dodging:
		UpdateDodge(DeltaSeconds);
		return;

	case EWarriorState::Hit:
		ChangeState(EWarriorState::Idle);
		break;

	default:
		break;
	}

	if (ShouldStartDodge())
	{
		StartDodge();
		return;
	}

	const float DistanceToPlayer = FMath::Abs(PlayerRef->GetActorLocation().X - GetActorLocation().X);

	if (DistanceToPlayer > DetectionRange)
	{
		ChangeState(EWarriorState::Idle);
		UpdateIdle();
		return;
	}

	TryStartAttack();
	if (CurrentState == EWarriorState::Attack1
		|| CurrentState == EWarriorState::Attack2
		|| CurrentState == EWarriorState::Attack3
		|| CurrentState == EWarriorState::Attack4Jump)
	{
		return;
	}

	ChangeState(EWarriorState::Chasing);
	UpdateChase(DeltaSeconds);
}

void ABasePaperWarrior::UpdateIdle()
{
	DisableAttackHitbox();
	ResetFootstepSoundState();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}
}

void ABasePaperWarrior::UpdateChase(float DeltaSeconds)
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

void ABasePaperWarrior::UpdateAttack1(float DeltaSeconds)
{
	ActionTimer += DeltaSeconds;
	FacePlayer();
	ResetFootstepSoundState();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}

	if (ActionTimer >= Attack1HitboxStartTime && ActionTimer < Attack1HitboxEndTime)
	{
		EnableAttackHitbox();
	}
	else
	{
		DisableAttackHitbox();
	}

	if (ActionTimer >= Attack1Duration)
	{
		FinishCurrentAction();
	}
}

void ABasePaperWarrior::UpdateAttack2(float DeltaSeconds)
{
	ActionTimer += DeltaSeconds;
	FacePlayer();
	ResetFootstepSoundState();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}

	if (ActionTimer >= Attack2HitboxStartTime && ActionTimer < Attack2HitboxEndTime)
	{
		EnableAttackHitbox();
	}
	else
	{
		DisableAttackHitbox();
	}

	if (ActionTimer >= Attack2Duration)
	{
		FinishCurrentAction();
	}
}

void ABasePaperWarrior::UpdateAttack3(float DeltaSeconds)
{
	ActionTimer += DeltaSeconds;
	FacePlayer();
	ResetFootstepSoundState();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}

	if (ActionTimer >= Attack3HitboxStartTime && ActionTimer < Attack3HitboxEndTime)
	{
		EnableAttackHitbox();
	}
	else
	{
		DisableAttackHitbox();
	}

	if (ActionTimer >= Attack3Duration)
	{
		FinishCurrentAction();
	}
}

void ABasePaperWarrior::UpdateAttack4Jump(float DeltaSeconds)
{
	ActionTimer += DeltaSeconds;
	ResetFootstepSoundState();

	if (!bJumpAttackLaunched)
	{
		FacePlayer();

		const float ForwardSign = IsEnemyFacingRight() ? 1.0f : -1.0f;
		LaunchCharacter(FVector(ForwardSign * JumpAttackForwardSpeed, 0.0f, JumpAttackUpSpeed), true, true);
		PlayJumpSound();
		bJumpAttackLaunched = true;
	}

	if (ActionTimer >= Attack4JumpHitboxStartTime && ActionTimer < Attack4JumpHitboxEndTime)
	{
		EnableAttackHitbox();
	}
	else
	{
		DisableAttackHitbox();
	}

	if (ActionTimer >= Attack4JumpDuration)
	{
		FinishCurrentAction();
	}
}

void ABasePaperWarrior::UpdateDodge(float DeltaSeconds)
{
	ActionTimer += DeltaSeconds;
	DisableAttackHitbox();
	ResetFootstepSoundState();
	FacePlayer();

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (ActionTimer >= DodgeDuration && Movement && Movement->IsMovingOnGround())
	{
		FinishCurrentAction();
	}
}

void ABasePaperWarrior::TryStartAttack()
{
	if (!PlayerRef || IsDeadState() || Super::IsHurt())
	{
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime < NextAttackAllowedTime)
	{
		return;
	}

	const float DistanceToPlayer = FMath::Abs(PlayerRef->GetActorLocation().X - GetActorLocation().X);

	const bool bCanAttack1 = (CurrentTime - LastAttack1Time) >= Attack1Cooldown;
	const bool bCanAttack2 = (CurrentTime - LastAttack2Time) >= Attack2Cooldown;
	const bool bCanAttack3 = (CurrentTime - LastAttack3Time) >= Attack3Cooldown;
	const bool bCanAttack4Jump = (CurrentTime - LastAttack4JumpTime) >= Attack4JumpCooldown;

	if (DistanceToPlayer <= Attack1Range)
	{
		TArray<int32> AvailableCloseAttacks;

		if (bCanAttack1)
		{
			AvailableCloseAttacks.Add(1);
		}

		if (bCanAttack2)
		{
			AvailableCloseAttacks.Add(2);
		}

		if (bCanAttack3)
		{
			AvailableCloseAttacks.Add(3);
		}

		if (AvailableCloseAttacks.Num() > 0)
		{
			const int32 ChoiceIndex = FMath::RandRange(0, AvailableCloseAttacks.Num() - 1);
			const int32 SelectedAttack = AvailableCloseAttacks[ChoiceIndex];

			switch (SelectedAttack)
			{
			case 1:
				StartAttack1();
				return;

			case 2:
				StartAttack2();
				return;

			case 3:
				StartAttack3();
				return;

			default:
				break;
			}
		}
	}

	if (DistanceToPlayer <= Attack4JumpRange && DistanceToPlayer > Attack2Range && bCanAttack4Jump)
	{
		StartAttack4Jump();
	}
}

bool ABasePaperWarrior::ShouldStartDodge() const
{
	const ABasePaperPlayer* PlayerCharacter = Cast<ABasePaperPlayer>(PlayerRef);
	if (!PlayerCharacter || IsDeadState() || IsAttacking() || CurrentState == EWarriorState::Dodging)
	{
		return false;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if ((CurrentTime - LastDodgeTime) < DodgeCooldown)
	{
		return false;
	}

	const float DistanceToPlayer = FMath::Abs(PlayerRef->GetActorLocation().X - GetActorLocation().X);
	if (DistanceToPlayer > DodgeDetectRange)
	{
		return false;
	}

	return PlayerCharacter->IsAttackHitboxActive();
}

void ABasePaperWarrior::StartAttack1()
{
	FacePlayer();
	ChangeState(EWarriorState::Attack1);
	ConfigureHitboxForCurrentAttack();

	ActionTimer = 0.0f;
	bJumpAttackLaunched = false;
	LastAttack1Time = GetWorld()->GetTimeSeconds();

	ResetAttackHitTracking();
	ResetFootstepSoundState();
	PlayAttackSoundForIndex(1);

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}
}

void ABasePaperWarrior::StartAttack2()
{
	FacePlayer();
	ChangeState(EWarriorState::Attack2);
	ConfigureHitboxForCurrentAttack();

	ActionTimer = 0.0f;
	bJumpAttackLaunched = false;
	LastAttack2Time = GetWorld()->GetTimeSeconds();

	ResetAttackHitTracking();
	ResetFootstepSoundState();
	PlayAttackSoundForIndex(2);

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}
}

void ABasePaperWarrior::StartAttack3()
{
	FacePlayer();
	ChangeState(EWarriorState::Attack3);
	ConfigureHitboxForCurrentAttack();

	ActionTimer = 0.0f;
	bJumpAttackLaunched = false;
	LastAttack3Time = GetWorld()->GetTimeSeconds();

	ResetAttackHitTracking();
	ResetFootstepSoundState();
	PlayAttackSoundForIndex(3);

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}
}

void ABasePaperWarrior::StartAttack4Jump()
{
	FacePlayer();
	ChangeState(EWarriorState::Attack4Jump);
	ConfigureHitboxForCurrentAttack();

	ActionTimer = 0.0f;
	bJumpAttackLaunched = false;
	LastAttack4JumpTime = GetWorld()->GetTimeSeconds();

	ResetAttackHitTracking();
	ResetFootstepSoundState();
	PlayAttackSoundForIndex(3);

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}
}

void ABasePaperWarrior::StartDodge()
{
	if (!PlayerRef)
	{
		return;
	}

	ChangeState(EWarriorState::Dodging);
	ActionTimer = 0.0f;
	bJumpAttackLaunched = false;
	LastDodgeTime = GetWorld()->GetTimeSeconds();

	DisableAttackHitbox();
	ResetFootstepSoundState();
	FacePlayer();

	const float AwayDirection = FMath::Sign(GetActorLocation().X - PlayerRef->GetActorLocation().X);
	const float SafeAwayDirection = FMath::IsNearlyZero(AwayDirection) ? (IsEnemyFacingRight() ? -1.0f : 1.0f) : AwayDirection;

	LaunchCharacter(FVector(SafeAwayDirection * DodgeBackwardSpeed, 0.0f, DodgeJumpSpeed), true, true);
	PlayJumpSound();
}

void ABasePaperWarrior::FinishCurrentAction()
{
	const bool bWasAttack =
		CurrentState == EWarriorState::Attack1 ||
		CurrentState == EWarriorState::Attack2 ||
		CurrentState == EWarriorState::Attack3 ||
		CurrentState == EWarriorState::Attack4Jump;

	DisableAttackHitbox();
	ResetFootstepSoundState();
	ActionTimer = 0.0f;
	bJumpAttackLaunched = false;

	if (bWasAttack)
	{
		NextAttackAllowedTime = GetWorld()->GetTimeSeconds() + AttackRecoveryTime;
	}

	if (IsDeadState())
	{
		ChangeState(EWarriorState::Dead);
		return;
	}

	ChangeState(EWarriorState::Idle);
}

void ABasePaperWarrior::ChangeState(EWarriorState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	CurrentState = NewState;
}

void ABasePaperWarrior::ConfigureHitboxForCurrentAttack()
{
	switch (CurrentState)
	{
	case EWarriorState::Attack1:
		Damage = Attack1DamageAmount;
		AttackHitboxBoxExtent = Attack1HitboxExtent;
		AttackHitboxForwardOffset = Attack1ForwardOffset;
		break;

	case EWarriorState::Attack2:
		Damage = Attack2DamageAmount;
		AttackHitboxBoxExtent = Attack2HitboxExtent;
		AttackHitboxForwardOffset = Attack2ForwardOffset;
		break;

	case EWarriorState::Attack3:
		Damage = Attack3DamageAmount;
		AttackHitboxBoxExtent = Attack3HitboxExtent;
		AttackHitboxForwardOffset = Attack3ForwardOffset;
		break;

	case EWarriorState::Attack4Jump:
		Damage = Attack4JumpDamageAmount;
		AttackHitboxBoxExtent = Attack4JumpHitboxExtent;
		AttackHitboxForwardOffset = Attack4JumpForwardOffset;
		break;

	default:
		break;
	}

	UpdateAttackHitboxTransform();
}