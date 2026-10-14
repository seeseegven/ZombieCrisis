// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ZCZombieAIController.generated.h"

/**
 * 
 */
UCLASS()
class ZOMBIECRISIS_API AZCZombieAIController : public AAIController
{
	GENERATED_BODY()
protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

private:
	void UpdateChase();

	FTimerHandle ChaseTimerHandle;
};
