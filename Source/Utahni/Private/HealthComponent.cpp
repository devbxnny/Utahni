// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "GameFramework/Actor.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	bIsDead = CurrentHealth <= 0.0f;
	bCanTakeDamage = !bIsDead;

	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleOwnerTakeAnyDamage);
	}
}

float UHealthComponent::GetCurrentHealth() const
{
	return CurrentHealth;
}

float UHealthComponent::GetMaxHealth() const
{
	return MaxHealth;
}

float UHealthComponent::GetHealthPercent() const
{
	return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

bool UHealthComponent::IsDead() const
{
	return bIsDead;
}

bool UHealthComponent::CanTakeDamage() const
{
	return bCanTakeDamage && !bIsDead;
}

void UHealthComponent::SetCanTakeDamage(bool bNewCanTakeDamage)
{
	bCanTakeDamage = bNewCanTakeDamage && !bIsDead;
}

float UHealthComponent::Heal(float HealAmount)
{
	if (HealAmount <= 0.0f || bIsDead)
	{
		return 0.0f;
	}

	return ApplyHealthDelta(FMath::Abs(HealAmount), nullptr, nullptr);
}

float UHealthComponent::ApplyDamage(float DamageAmount, AController* InstigatedBy, AActor* DamageCauser)
{
	if (DamageAmount <= 0.0f || bIsDead || !CanTakeDamage())
	{
		return 0.0f;
	}

	return -ApplyHealthDelta(-FMath::Abs(DamageAmount), InstigatedBy, DamageCauser);
}

void UHealthComponent::ResetHealth()
{
	const float OldHealth = CurrentHealth;

	CurrentHealth = MaxHealth;
	bIsDead = false;
	bCanTakeDamage = true;

	const float AppliedDelta = CurrentHealth - OldHealth;
	if (!FMath::IsNearlyZero(AppliedDelta))
	{
		OnHealthChanged.Broadcast(this, CurrentHealth, MaxHealth, AppliedDelta, nullptr);
	}
}

void UHealthComponent::RefillToFull()
{
	const float OldHealth = CurrentHealth;

	CurrentHealth = MaxHealth;

	const float AppliedDelta = CurrentHealth - OldHealth;
	if (!FMath::IsNearlyZero(AppliedDelta))
	{
		OnHealthChanged.Broadcast(this, CurrentHealth, MaxHealth, AppliedDelta, nullptr);
	}
}

void UHealthComponent::HandleOwnerTakeAnyDamage(
	AActor* DamagedActor,
	float Damage,
	const UDamageType* DamageType,
	AController* InstigatedBy,
	AActor* DamageCauser
)
{
	if (!DamagedActor || Damage <= 0.0f || bIsDead || !CanTakeDamage())
	{
		return;
	}

	ApplyHealthDelta(-Damage, InstigatedBy, DamageCauser);
}

float UHealthComponent::ApplyHealthDelta(float DeltaAmount, AController* InstigatedBy, AActor* DamageCauser)
{
	const float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth + DeltaAmount, 0.0f, MaxHealth);

	const float AppliedDelta = CurrentHealth - OldHealth;
	if (FMath::IsNearlyZero(AppliedDelta))
	{
		return 0.0f;
	}

	OnHealthChanged.Broadcast(this, CurrentHealth, MaxHealth, AppliedDelta, DamageCauser);

	if (!bIsDead && CurrentHealth <= 0.0f)
	{
		bIsDead = true;
		bCanTakeDamage = false;
		OnDeath.Broadcast(this, DamageCauser, InstigatedBy);

		if (bAutoDestroyOwnerOnDeath)
		{
			if (AActor* Owner = GetOwner())
			{
				Owner->Destroy();
			}
		}
	}

	return AppliedDelta;
}