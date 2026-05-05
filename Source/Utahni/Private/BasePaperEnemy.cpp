#include "BasePaperEnemy.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PaperFlipbook.h"
#include "PaperFlipbookComponent.h"
#include "TimerManager.h"
#include "GameModes/Code_SideScrollerGameModeBase.h"

ABasePaperEnemy::ABasePaperEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = MoveSpeed;
		Movement->bConstrainToPlane = true;
		Movement->SetPlaneConstraintAxisSetting(EPlaneConstraintAxisSetting::Y);
		Movement->bSnapToPlaneAtStart = true;
	}

	AttackHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackHitbox"));
	AttackHitbox->SetupAttachment(RootComponent);
	AttackHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttackHitbox->SetCollisionObjectType(ECC_WorldDynamic);
	AttackHitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
	AttackHitbox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AttackHitbox->SetGenerateOverlapEvents(true);
	AttackHitbox->SetBoxExtent(AttackHitboxBoxExtent);
}

void ABasePaperEnemy::BeginPlay()
{
	Super::BeginPlay();

	PlayerRef = UGameplayStatics::GetPlayerPawn(this, 0);

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = MoveSpeed;
	}

	if (AttackHitbox)
	{
		AttackHitbox->OnComponentBeginOverlap.AddDynamic(this, &ABasePaperEnemy::HandleAttackHitboxBeginOverlap);
		UpdateAttackHitboxTransform();
		DisableAttackHitbox();
	}

	ACode_SideScrollerGameModeBase* GameMode = Cast<ACode_SideScrollerGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->RegisterEnemy();
	}
}

void ABasePaperEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IsDeadState() || !PlayerRef)
	{
		return;
	}

	UpdateAttackHitboxTransform();
	FacePlayer();
	HandleMovement();
}

bool ABasePaperEnemy::IsAttacking() const
{
	return bEnemyAttacking;
}

int32 ABasePaperEnemy::GetAttackIndex() const
{
	return bEnemyAttacking ? 1 : 0;
}

void ABasePaperEnemy::OnCharacterDamaged(float DamageTaken, AActor* DamageCauser)
{
	bEnemyAttacking = false;
	bAttackHitboxActive = false;

	GetWorldTimerManager().ClearTimer(AttackTimerHandle);
	GetWorldTimerManager().ClearTimer(AttackHitboxStartTimerHandle);
	GetWorldTimerManager().ClearTimer(AttackHitboxEndTimerHandle);

	ResetAttackHitTracking();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}
}

void ABasePaperEnemy::OnCharacterDied(AActor* DamageCauser, AController* InstigatedBy)
{
	bEnemyAttacking = false;
	bAttackHitboxActive = false;

	GetWorldTimerManager().ClearTimer(AttackTimerHandle);
	GetWorldTimerManager().ClearTimer(AttackHitboxStartTimerHandle);
	GetWorldTimerManager().ClearTimer(AttackHitboxEndTimerHandle);

	ResetAttackHitTracking();

	ACode_SideScrollerGameModeBase* GameMode = Cast<ACode_SideScrollerGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->UnregisterEnemy();
	}

	if (DeathDestroyDelay > 0.0f)
	{
		const float DeathTime = DeathFlipbook
			? FMath::Max(DeathFlipbook->GetTotalDuration(), DeathDestroyDelay)
			: DeathDestroyDelay;

		SetLifeSpan(DeathTime);
	}
}

void ABasePaperEnemy::HandleMovement()
{
	if (!PlayerRef || IsHurt() || bEnemyAttacking)
	{
		return;
	}

	const FVector EnemyLocation = GetActorLocation();
	const FVector PlayerLocation = PlayerRef->GetActorLocation();

	const float DeltaX = PlayerLocation.X - EnemyLocation.X;
	const float DistanceToPlayer = FMath::Abs(DeltaX);

	if (DistanceToPlayer <= AttackRange)
	{
		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}

		AttackPlayer();
		return;
	}

	if (DistanceToPlayer <= DetectionRange)
	{
		const float MoveDirection = FMath::Sign(DeltaX);
		AddMovementInput(FVector(1.0f, 0.0f, 0.0f), MoveDirection);
		return;
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}
}

void ABasePaperEnemy::FacePlayer()
{
	if (!PlayerRef || IsDeadState() || IsHurt())
	{
		return;
	}

	const float DeltaX = PlayerRef->GetActorLocation().X - GetActorLocation().X;
	if (!FMath::IsNearlyZero(DeltaX))
	{
		SetFacingDirection(DeltaX > 0.0f);
	}
}

void ABasePaperEnemy::AttackPlayer()
{
	if (!PlayerRef || bEnemyAttacking || IsDeadState() || IsHurt())
	{
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAttackTime < AttackCooldown)
	{
		return;
	}

	LastAttackTime = CurrentTime;
	bEnemyAttacking = true;
	bAttackHitboxActive = false;

	ResetAttackHitTracking();

	const float AttackDuration = Attack1Flipbook
		? FMath::Max(Attack1Flipbook->GetTotalDuration(), MinimumAttackDuration)
		: MinimumAttackDuration;

	GetWorldTimerManager().ClearTimer(AttackHitboxStartTimerHandle);
	GetWorldTimerManager().ClearTimer(AttackHitboxEndTimerHandle);
	GetWorldTimerManager().ClearTimer(AttackTimerHandle);

	GetWorldTimerManager().SetTimer(
		AttackHitboxStartTimerHandle,
		this,
		&ABasePaperEnemy::EnableAttackHitbox,
		AttackHitboxStartTime,
		false
	);

	GetWorldTimerManager().SetTimer(
		AttackHitboxEndTimerHandle,
		this,
		&ABasePaperEnemy::DisableAttackHitbox,
		AttackHitboxEndTime,
		false
	);

	GetWorldTimerManager().SetTimer(
		AttackTimerHandle,
		this,
		&ABasePaperEnemy::EndAttack,
		AttackDuration,
		false
	);
}

void ABasePaperEnemy::EndAttack()
{
	bEnemyAttacking = false;
	bAttackHitboxActive = false;
	ResetAttackHitTracking();
}

void ABasePaperEnemy::EnableAttackHitbox()
{
	if (!AttackHitbox || IsDeadState())
	{
		return;
	}

	bAttackHitboxActive = true;
	UpdateAttackHitboxTransform();
	AttackHitbox->SetBoxExtent(AttackHitboxBoxExtent);
	AttackHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void ABasePaperEnemy::DisableAttackHitbox()
{
	if (!AttackHitbox)
	{
		return;
	}

	bAttackHitboxActive = false;
	AttackHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABasePaperEnemy::ResetAttackHitTracking()
{
	HitActorsThisAttack.Empty();
	DisableAttackHitbox();
}

bool ABasePaperEnemy::IsEnemyFacingRight() const
{
	const UPaperFlipbookComponent* SpriteComponent = GetSprite();
	if (!SpriteComponent)
	{
		return true;
	}

	return SpriteComponent->GetRelativeScale3D().X >= 0.0f;
}

void ABasePaperEnemy::UpdateAttackHitboxTransform()
{
	if (!AttackHitbox)
	{
		return;
	}

	const float FacingSign = IsEnemyFacingRight() ? 1.0f : -1.0f;

	FVector NewOffset = AttackHitboxLocalOffset;
	NewOffset.X += AttackHitboxForwardOffset * FacingSign;

	AttackHitbox->SetRelativeLocation(NewOffset);
	AttackHitbox->SetBoxExtent(AttackHitboxBoxExtent);
}

void ABasePaperEnemy::HandleAttackHitboxBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!bAttackHitboxActive || !OtherActor || OtherActor == this)
	{
		return;
	}

	if (OtherActor != PlayerRef)
	{
		return;
	}

	if (HitActorsThisAttack.Contains(OtherActor))
	{
		return;
	}

	HitActorsThisAttack.Add(OtherActor);

	UGameplayStatics::ApplyDamage(
		OtherActor,
		Damage,
		GetController(),
		this,
		UDamageType::StaticClass()
	);
}
