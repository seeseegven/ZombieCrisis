// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZCPlayerCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class UZCDeathAnimationComponent;
struct FInputActionValue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnHealthChangedSignature,
	float,
	BloodPercent
);

UCLASS()
class ZOMBIECRISIS_API AZCPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AZCPlayerCharacter();

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChangedSignature OnHealthChanged;

protected:

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ZC|Input")
	TObjectPtr<UInputMappingContext> PlayerMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ZC|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ZC|Input")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, 
		Category = "ZC|Weapon|Pistol", meta = (ClampMin = "0.0"))
	int32 PistolDamage = 20;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "ZC|Weapon|Pistol", meta = (ClampMin = "0.01"))
	float PistolShotPerSecond = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "ZC|Weapon|Pistol", meta = (ClampMin = "0.0"))
	float PistolRange = 1500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "ZC|Weapon|Pistol")
	float PistolTraceHeight = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "BasicSettings|HP")
	float MaxHealth = 100.0f;

	float CurrentHealth;

	UPROPERTY(BlueprintReadOnly)
	float BloodPercent;

	UPROPERTY(VisibleAnywhere, Category = "ZC|Death")
	TObjectPtr<UZCDeathAnimationComponent> DeathAnimationComponent;

	virtual float TakeDamage(
		float DamageAmount,
		const FDamageEvent& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser
	) override;

private:
	void Move(const FInputActionValue& Value);
	void FirePistol();
	bool IsDead();
	bool bIsDead = false;
	float NextPistolFireTime = 0.0f;
};
