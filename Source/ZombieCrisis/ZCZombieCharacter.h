// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZCZombieCharacter.generated.h"

UCLASS()
class ZOMBIECRISIS_API AZCZombieCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AZCZombieCharacter();
	bool IsDead();
	virtual float TakeDamage(
		float DamageAmount,
		const FDamageEvent& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser
	) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BasicSettings|HP")
	float MaxHealth = 100.f;
	float CurrentHealth=0.f;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
