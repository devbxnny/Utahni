#include "BasePaperBat.h"
#include "Kismet/GameplayStatics.h"
#include "PaperFlipbookComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ABasePaperBat::ABasePaperBat()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABasePaperBat::BeginPlay()
{
	Super::BeginPlay();

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->GravityScale = 0.0f;
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		GetCharacterMovement()->Velocity = FVector::ZeroVector;
	}

	HomeLocation = GetActorLocation();
	BaseHoverZ = GetActorLocation().Z;

	ResetAttackHitTracking();
	ResetFootstepSoundState();
	DisableAttackHitbox();
	UpdateAnimation();
}

void ABasePaperBat::Tick(float DeltaTime)
{
	ABasePaperCharacter::Tick(DeltaTime);

	if (IsDeadState())
	{
		ResetFootstepSoundState();
		ChangeState(EBatMoveState::Returning);
		DisableAttackHitbox();
		ResetAttackHitTracking();
		return;
	}

	if (IsHurt())
	{
		ResetFootstepSoundState();
		UpdateAnimation();
		return;
	}

	RunningTime += DeltaTime;
	UpdateAttackHitboxTransform();

	if (BatState != EBatMoveState::Swooping)
	{
		CheckPlayerDistance();
	}

	switch (BatState)
	{
	case EBatMoveState::Idle:
		UpdateIdleMovement(DeltaTime);
		break;

	case EBatMoveState::Swooping:
		UpdateSwoopMovement(DeltaTime);
		break;

	case EBatMoveState::Returning:
		UpdateReturnMovement(DeltaTime);
		break;

	default:
		break;
	}
}

bool ABasePaperBat::IsAttacking() const
{
	return BatState == EBatMoveState::Swooping;
}

int32 ABasePaperBat::GetAttackIndex() const
{
	return BatState == EBatMoveState::Swooping ? CurrentSwoopAttackIndex : 0;
}

bool ABasePaperBat::IsHurt() const
{
	if (BatState == EBatMoveState::Swooping)
	{
		return false;
	}

	return Super::IsHurt();
}

void ABasePaperBat::OnCharacterDamaged(float DamageTaken, AActor* DamageCauser)
{
	if (BatState == EBatMoveState::Swooping)
	{
		return;
	}

	ResetFootstepSoundState();
	Super::OnCharacterDamaged(DamageTaken, DamageCauser);

	ResetAttackHitTracking();
	DisableAttackHitbox();
	ChangeState(EBatMoveState::Returning);
}

void ABasePaperBat::OnCharacterDied(AActor* DamageCauser, AController* InstigatedBy)
{
	ResetFootstepSoundState();
	Super::OnCharacterDied(DamageCauser, InstigatedBy);

	ResetAttackHitTracking();
	DisableAttackHitbox();
	UpdateAnimation();
}

void ABasePaperBat::CheckPlayerDistance()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (!PlayerPawn)
	{
		TargetPlayer = nullptr;
		ResetAttackHitTracking();
		DisableAttackHitbox();

		if (BatState != EBatMoveState::Returning)
		{
			ChangeState(EBatMoveState::Returning);
		}

		return;
	}

	const float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());

	if (BatState == EBatMoveState::Idle && DistanceToPlayer <= DetectRange)
	{
		TargetPlayer = PlayerPawn;
		PlayerRef = PlayerPawn;
		ChangeState(EBatMoveState::Swooping);

		CurrentSwoopAttackIndex = FMath::RandBool() ? 2 : 3;

		SwoopStartLocation = GetActorLocation();

		const FVector PlayerLocation = PlayerPawn->GetActorLocation();
		const FVector ForwardDir = (PlayerLocation - SwoopStartLocation).GetSafeNormal();

		SwoopTargetLocation = PlayerLocation + (ForwardDir * 100.0f);
		SwoopTargetLocation.Z = BaseHoverZ + SwoopDepthOffset;

		SwoopProgress = 0.0f;

		ResetAttackHitTracking();
		DisableAttackHitbox();
		PlayAttackSoundForIndex(CurrentSwoopAttackIndex);
	}
	else if (BatState == EBatMoveState::Returning && DistanceToPlayer > LoseInterestRange)
	{
		TargetPlayer = nullptr;
		ResetAttackHitTracking();
		DisableAttackHitbox();
	}
}

void ABasePaperBat::UpdateIdleMovement(float DeltaTime)
{
	DisableAttackHitbox();

	const FVector CurrentLocation = GetActorLocation();

	const float CircleX = FMath::Cos(RunningTime * IdleMoveSpeed) * IdleRadius;
	const float BobZ = FMath::Sin(RunningTime * HoverBobSpeed) * HoverBobAmplitude;

	FVector DesiredLocation = HomeLocation;
	DesiredLocation.X += CircleX;
	DesiredLocation.Z = BaseHoverZ + BobZ;

	const FVector NewLocation = FMath::VInterpTo(CurrentLocation, DesiredLocation, DeltaTime, 2.0f);
	SetActorLocation(NewLocation);

	FaceMovementDirection(DesiredLocation - CurrentLocation);

	const float IdleMoveMagnitude = (DesiredLocation - CurrentLocation).Size();
	TryPlayFootstepSound(DeltaTime, true, IdleMoveMagnitude);
}

void ABasePaperBat::UpdateSwoopMovement(float DeltaTime)
{
	ResetFootstepSoundState();

	SwoopProgress += DeltaTime / SwoopDuration;
	SwoopProgress = FMath::Clamp(SwoopProgress, 0.0f, 1.0f);

	FVector NewLocation = FMath::Lerp(SwoopStartLocation, SwoopTargetLocation, SwoopProgress);

	const float ArcOffset = FMath::Sin(SwoopProgress * PI) * SwoopArcHeight;
	NewLocation.Z -= ArcOffset;

	const FVector MoveDirection = NewLocation - GetActorLocation();
	SetActorLocation(NewLocation);
	FaceMovementDirection(MoveDirection);

	if (SwoopProgress >= SwoopHitboxStartProgress && SwoopProgress < SwoopHitboxEndProgress)
	{
		EnableAttackHitbox();
	}
	else
	{
		DisableAttackHitbox();
	}

	if (SwoopProgress >= 1.0f)
	{
		ResetAttackHitTracking();
		DisableAttackHitbox();
		ChangeState(EBatMoveState::Returning);
		HomeLocation.X = GetActorLocation().X;
	}
}

void ABasePaperBat::UpdateReturnMovement(float DeltaTime)
{
	ResetFootstepSoundState();
	DisableAttackHitbox();

	const FVector CurrentLocation = GetActorLocation();

	const float BobZ = FMath::Sin(RunningTime * HoverBobSpeed) * HoverBobAmplitude;

	FVector ReturnTarget = CurrentLocation;
	ReturnTarget.Z = BaseHoverZ + BobZ;

	const FVector NewLocation = FMath::VInterpTo(CurrentLocation, ReturnTarget, DeltaTime, 3.0f);
	const FVector MoveDirection = NewLocation - CurrentLocation;

	SetActorLocation(NewLocation);
	FaceMovementDirection(MoveDirection);

	if (FMath::Abs(NewLocation.Z - ReturnTarget.Z) <= 10.0f)
	{
		ChangeState(EBatMoveState::Idle);
	}
}

void ABasePaperBat::FaceMovementDirection(const FVector& Direction)
{
	if (Direction.X < 0.0f)
	{
		SetActorRotation(FRotator(0.0f, 180.0f, 0.0f));
	}
	else if (Direction.X > 0.0f)
	{
		SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
	}
}

void ABasePaperBat::UpdateAnimation()
{
	if (!GetSprite())
	{
		return;
	}

	UPaperFlipbook* DesiredFlipbook = nullptr;

	if (IsDeadState())
	{
		DesiredFlipbook = DeathFlipbook;
	}
	else if (Super::IsHurt() && HitFlipbook)
	{
		DesiredFlipbook = HitFlipbook;
	}
	else
	{
		switch (BatState)
		{
		case EBatMoveState::Idle:
			DesiredFlipbook = IdleFlipbook;
			break;

		case EBatMoveState::Swooping:
			DesiredFlipbook = (CurrentSwoopAttackIndex == 2) ? AttackFlipbook2 : AttackFlipbook3;
			break;

		case EBatMoveState::Returning:
			DesiredFlipbook = IdleFlipbook;
			break;

		default:
			break;
		}
	}

	if (DesiredFlipbook && GetSprite()->GetFlipbook() != DesiredFlipbook)
	{
		GetSprite()->SetFlipbook(DesiredFlipbook);
	}
}

void ABasePaperBat::ChangeState(EBatMoveState NewState)
{
	if (BatState == NewState)
	{
		return;
	}

	BatState = NewState;
	UpdateAnimation();
}