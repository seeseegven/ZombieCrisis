// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZCZombieCharacter.generated.h"

class UZCDeathAnimationComponent;
class UUZCFloatingScore;
class UDataTable;

UCLASS()
class ZOMBIECRISIS_API AZCZombieCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AZCZombieCharacter();
	void HandleElimination();
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BasicSettings|HP")
	float MaxHealth = 100.f;
	float CurrentHealth=0.f;
	UPROPERTY(EditDefaultsOnly, Category = "ScoreWidget")
	TSubclassOf<UUZCFloatingScore> ScoreWidgetClass;
	UPROPERTY(VisibleAnywhere, Category = "ZC|Death")
	TObjectPtr<UZCDeathAnimationComponent> DeathAnimationComponent;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	bool IsDead();
	virtual float TakeDamage(
		float DamageAmount,
		const FDamageEvent& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser
	) override;

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void StartAttack(AActor* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void StopAttack(AActor* LeavingTarget);

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void AttackHitCheck();

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void AttackFinished();

	void RestartAttackIfStillInRange();

	UFUNCTION(BlueprintImplementableEvent)//仅声明，实现由蓝图来
	void BP_PlayAttackAnimation();

	UPROPERTY()
	TObjectPtr<AActor> AttackTarget = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	float AttackDamage = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bIsAnimationStarted = false;
	bool bIsAttacking = false;
	bool bIsTargetInAttackRange = false;
	bool bIsDead = false;
	
};
