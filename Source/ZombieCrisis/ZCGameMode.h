// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ZCGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ZOMBIECRISIS_API AZCGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	AZCGameMode();
	void HandleEnemyKilled(AActor* KilledEnemy, AController* KillerController);
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Score")
	int32 ZombieKilledScore = 100;
};
