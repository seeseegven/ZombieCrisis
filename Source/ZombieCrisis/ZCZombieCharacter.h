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

	UPROPERTY(EditDefaultsOnly, Category = "ZC|Speed")
	float KnockbackVelocity = 300.f;

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
	void StartAttack(AActor* NewTarget);//开始攻击

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void StopAttack(AActor* LeavingTarget);//停止攻击，即判定失效了

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void AttackHitCheck();//动画播完后判断是否击中

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void AttackFinished();//角色未离开范围，一次攻击结束，会继续攻击

	void RestartAttackIfStillInRange();//如果离开就退出，否则执行Start Attack函数

	UFUNCTION(BlueprintImplementableEvent)//仅声明，实现由蓝图来
	void BP_PlayAttackAnimation();//蓝图实现攻击动画

	UFUNCTION(BlueprintImplementableEvent)
	void BP_PlayHitReactionAnimation();

	UPROPERTY()
	TObjectPtr<AActor> AttackTarget = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	float AttackDamage = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bIsAnimationStarted = false;
	bool bIsAttacking = false;
	bool bIsTargetInAttackRange = false;
	bool bIsDead = false;
private:
	FVector KnockbackVelocityEachAxis;
};
