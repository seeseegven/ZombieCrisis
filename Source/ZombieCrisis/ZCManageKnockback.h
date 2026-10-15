// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/HitResult.h"
#include "TimerManager.h"
#include "ZCManageKnockback.generated.h"

class UPrimitiveComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZOMBIECRISIS_API UZCManageKnockback : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UZCManageKnockback();

	void StartKnockback(const FVector& Direction, float BackSpeed, float BackDuration);
	void StopKnockback();
	bool GetKnockbackStatus() const;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleCapsuleHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

	bool bKnockbackActive = false;
	float KnockbackSpeed = 300.f;
	FVector KnockbackDirection = FVector::ZeroVector;
	FTimerHandle KnockbackTimerHandle;
};
