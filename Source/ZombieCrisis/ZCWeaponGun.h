// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZCWeaponBasic.h"
#include "ZCWeaponGun.generated.h"



/**
 * 
 */
UCLASS()
class ZOMBIECRISIS_API AZCWeaponGun : public AZCWeaponBasic
{
	GENERATED_BODY()
public:

protected:
	int32 WeaponDamage;
	int32 TraceCount;
	float BulletDegrees;
	float DamageRange;
	float StartDegreeOffset;
	UPROPERTY()
	TObjectPtr<UNiagaraSystem> MuzzleFlashSystem = nullptr;
private:
	virtual bool WeaponApply() override;
	virtual void UpdateProperty(const FZCWeaponComputedStats& Value) override;
	void DrawTraceEffect();
};
