// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ZCGameMode.generated.h"

/**
 * 
 */

class AZCZombieSpawner;
class UZCPlayerProgressionComponent;

UCLASS()
class ZOMBIECRISIS_API AZCGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	AZCGameMode();
	void HandleEnemyKilled(AActor* KilledEnemy, AController* KillerController);
	void HandlePlayerDied();

	virtual void StartPlay() override;

	UFUNCTION(BlueprintImplementableEvent)
	void BP_DisplayFinalScore(int32 FinalScore);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_DisplayCurrentWave(int32 CurrentWave);

	UFUNCTION(BlueprintCallable)
	void HandleRestartLevel();

	UFUNCTION(BlueprintCallable)
	void HandleReturnMainMenu();

	void UnlockWeapon();
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Score")
	int32 ZombieKilledScore = 100;

	UPROPERTY()
	TArray<TObjectPtr<AZCZombieSpawner>> ZombieSpawners;

	UPROPERTY(EditAnywhere, Category = "ZC|Weapon")
	TArray<FName> Weapons;//全局配置的武器，升级什么的都用该数组和对应指针。

	int32 CurrentAliveZombie = 0;//场上还剩余的僵尸，全死完进入下一波
	int32 Wave = 0;//当前波次
	int32 ManageSpawningCount();//管理每波每个生成点产生的僵尸数量
	FTimerHandle WaveTimerHandle;//波次之前间隔的计时器

	void SpawningEachWave();
private:
	int32 UnlockIndex = 0;
	int32 DamageMuliplierIndex = 0;
	int32 FireRateIndex = 0;
	int32 AmmoCapacityIndex = 0;
	int32 DamageRangeIndex = 0;
	int32 SpreadMultiplierIndex = 0;

	//运行时的玩家升级增益
	UPROPERTY(VisibleAnywhere, Category = "ZC|Progression")
	TObjectPtr<UZCPlayerProgressionComponent> PlayerProgression = nullptr;


};
