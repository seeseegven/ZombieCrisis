// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZCWeaponTypes.h"
#include "ZCWeaponProgressionComponent.generated.h"

class UDataTable;
//                   在zombie crisis分类下可以找到，蓝图内可以添加组件
UCLASS( ClassGroup=(ZombieCrisis), meta=(BlueprintSpawnableComponent) )
class ZOMBIECRISIS_API UZCWeaponProgressionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UZCWeaponProgressionComponent();

	UFUNCTION(BlueprintCallable, Category = "ZC|Progression")
	void InitializeProgression();

	UFUNCTION(BlueprintCallable, Category = "ZC|Progression")
	void SetPlayerLevel(int32 NewLevel);
	//                     纯函数，只读取数据
	UFUNCTION(BlueprintPure, Category = "ZC|Progression")
	int32 GetPlayerLevel() const {
		return PlayerLevel;
	}

	UFUNCTION(BlueprintPure, Category = "ZC|Weapon")
	bool
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	//                        仅修改默认值，不能改实例对象
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ZC|Data")
	TObjectPtr<UDataTable> WeaponDefinitions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ZC|Data")
	TObjectPtr<UDataTable> LevelRewards;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, 
		Category = "ZC|Progression", meta = (ClampMin = "1"))//最小值为1
	int32 StartingLevel = 1;
	
private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, 
		Category = "ZC|Progression", meta = (AllowPrivateAccess="true"))
	int32 PlayerLevel = 1;                    //              私有变量，但是可以通过反射系统读取
		
};
