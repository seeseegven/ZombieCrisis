// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ZCGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnScoreChangedSignature,
	int32,
	CurrentScore
);

/**
 * 
 */
UCLASS()
class ZOMBIECRISIS_API AZCGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	void AddScore(int32 Amount);
	int32 GetCurrentScore() const;
protected:
	UPROPERTY(BlueprintAssignable, Category = "Score")
	FOnScoreChangedSignature OnScoreChange;

	
	
private:
	int32 CurrentScore = 0;
};
