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
	float BulletDegrees;
	float DamageRange;
private:
	virtual void WeaponApply() override;
	virtual void UpdateProperty(const FZCWeaponDefinitionRow& Value) override;
};
