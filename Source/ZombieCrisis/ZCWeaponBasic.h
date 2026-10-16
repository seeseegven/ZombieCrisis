// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZCWeaponBasic.generated.h"

struct FZCWeaponDefinitionRow;
class UNiagaraSystem;

UCLASS(Abstract)
class ZOMBIECRISIS_API AZCWeaponBasic : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AZCWeaponBasic();
	virtual void UpdateProperty(const FZCWeaponDefinitionRow& Value) {};
	virtual void WeaponApply() {};
protected:
	// Called when the game starts or when spawned
	float ShotInterval;
	int32 WeaponCapacity;
	float NextPistolFireTime = 0.0f;
	UPROPERTY()//uobject类型的加宏交给虚幻的反射系统管理生命周期
	TObjectPtr<UNiagaraSystem> WeaponEffectSystem = nullptr;
public:	
	// Called every frame


};
