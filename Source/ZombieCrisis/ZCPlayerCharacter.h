// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZCPlayerCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class UZCDeathAnimationComponent;
struct FInputActionValue;
struct FZCWeaponDefinitionRow;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnHealthChangedSignature,
	float,
	BloodPercent
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnWeaponChangedSignature,
	FText,
	NewWeaponName
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

	UPROPERTY(BlueprintAssignable, Category = "ZC|Weapon")
	FOnWeaponChangedSignature OnWeaponChanged;

protected:
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ZC|Input")
	TObjectPtr<UInputMappingContext> PlayerMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ZC|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ZC|Input")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditDefaultsOnly, Category = "ZC|Input")
	TObjectPtr<UInputAction> ChangeWeaponAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, 
		Category = "ZC|Weapon", meta = (ClampMin = "0.0"))
	int32 WeaponDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "ZC|Weapon", meta = (ClampMin = "0.01"))
	float WeaponShotPerSecond;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "ZC|Weapon", meta = (ClampMin = "0.0"))
	float DamageRange;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "ZC|Weapon")
	float DamageTraceHeight = 50.0f;

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
	UPROPERTY(EditDefaultsOnly, Category = "ZC|Speed")
	float RotateRateY = 180.f;

	UFUNCTION(BlueprintImplementableEvent, Category = "ZC|Weapon")
	void BP_ApplyNewWeaponVisual(TSubclassOf<AActor> VisualClass,
		const FTransform& RelativeTransform);

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "ZC|Weapon")
	TObjectPtr<UDataTable> WeaponDefinitionTable;

	UPROPERTY(EditAnywhere, Category = "ZC|Weapon")
	TArray<FName> Weapons;

private:
	void Move(const FInputActionValue& Value);
	void FirePistol();
	void ChangeWeapons(const FInputActionValue & Value);
	void ApplyNewWeapon();
	bool IsDead();
	void ChangeWeaponProperty(const FZCWeaponDefinitionRow& Values);
	bool bIsDead = false;
	float NextPistolFireTime = 0.0f;
	int32 CurrentWeaponIndex = 0;
};
