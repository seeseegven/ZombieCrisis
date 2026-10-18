// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZCZombieSpawner.generated.h"

class UBoxComponent;
class AZCZombieCharacter;

UCLASS()
class ZOMBIECRISIS_API AZCZombieSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AZCZombieSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void SpawnZombie();

	UFUNCTION()//绑定广播需要靠回调函数，要进入反射系统
	void HandleReleaseTriggerOverlap(
		UPrimitiveComponent* OverlappedComponent,//被重叠的组件
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UPROPERTY(VisibleAnywhere, Category = "Spawn")
	TObjectPtr<USceneComponent> SpawnPoint;

	UPROPERTY(VisibleAnywhere, Category = "Spawn")
	TObjectPtr<UBoxComponent> ReleaseTrigger;

	UPROPERTY(EditInstanceOnly, Category = "Spawn", meta = (ClampMin = "0"))
	int32 Count = 3;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AZCZombieCharacter> ZombieClass;

	UPROPERTY()
	TObjectPtr<AZCZombieCharacter> WaitingZombie = nullptr;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	FTimerHandle SpawnTimerHandle;
};
