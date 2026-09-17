// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZCPlayerCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class ZOMBIECRISIS_API AZCPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AZCPlayerCharacter();

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
	float PistolShotPerSecond = 4.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "ZC|Weapon|Pistol", meta = (ClampMin = "0.0"))
	float PistolRange = 1500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "ZC|Weapon|Pistol")
	float PistolTraceHeight = 50.0f;

private:
	void Move(const FInputActionValue& Value);
	void FirePistol();
	float NextPistolFireTime = 0.0f;
};
