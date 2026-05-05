// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class AController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(
	FOnHealthChangedSignature,
	UHealthComponent*, HealthComponent,
	float, NewHealth,
	float, MaxHealth,
	float, HealthDelta,
	AActor*, DamageCauser
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnDeathSignature,
	UHealthComponent*, HealthComponent,
	AActor*, DamageCauser,
	AController*, InstigatedBy
);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UTAHNI_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentHealth() const;

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const;

	UFUNCTION(BlueprintPure, Category = "Health")
	bool CanTakeDamage() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetCanTakeDamage(bool bNewCanTakeDamage);

	UFUNCTION(BlueprintCallable, Category = "Health")
	float Heal(float HealAmount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	float ApplyDamage(float DamageAmount, AController* InstigatedBy = nullptr, AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ResetHealth();

	UFUNCTION(BlueprintCallable, Category = "Health")
	void RefillToFull();

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDeathSignature OnDeath;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	bool bAutoDestroyOwnerOnDeath = false;

private:
	UFUNCTION()
	void HandleOwnerTakeAnyDamage(
		AActor* DamagedActor,
		float Damage,
		const class UDamageType* DamageType,
		AController* InstigatedBy,
		AActor* DamageCauser
	);

	float ApplyHealthDelta(float DeltaAmount, AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	float CurrentHealth = 0.0f;

	bool bIsDead = false;
	bool bCanTakeDamage = true;
};