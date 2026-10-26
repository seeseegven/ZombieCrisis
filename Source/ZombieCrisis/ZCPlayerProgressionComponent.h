// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZCWeaponTypes.h"
#include "ZCPlayerProgressionComponent.generated.h"

class UDataTable;
//                   在zombie crisis分类下可以找到，蓝图内可以添加组件
UCLASS( ClassGroup=(ZombieCrisis), meta=(BlueprintSpawnableComponent) )
class ZOMBIECRISIS_API UZCPlayerProgressionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UZCPlayerProgressionComponent();

	UFUNCTION(BlueprintCallable, Category = "ZC|Progression")
	void InitializeProgression();

	//                     纯函数，只读取数据
	UFUNCTION(BlueprintPure, Category = "ZC|Progression")
	int32 GetPlayerLevel() const {
		return PlayerLevel;
	}

	void ApplyNewLevelRewards();

	const TMap<FName, FZCWeaponComputedStats>& GetWeaponStates() const;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	//                        仅修改默认值，不能改实例对象
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ZC|Data")
	TObjectPtr<UDataTable> WeaponDefinitions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ZC|Data")
	TObjectPtr<UDataTable> LevelRewards;
	
private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, 
		Category = "ZC|Progression", meta = (AllowPrivateAccess="true"))
	int32 PlayerLevel = 0;                    //              私有变量，但是可以通过反射系统读取

	// 按定义表行名保存本局最终配置；后续奖励直接更新对应 Computed 数值。
	UPROPERTY(VisibleInstanceOnly, Category = "ZC|Progression")
	TMap<FName, FZCWeaponComputedStats> WeaponStates;

	/**
	 * 开局将武器定义表转换成本局最终配置，只复制每类武器适用的字段。
	 * 参数：无，读取组件配置的 WeaponDefinitions。
	 * 返回值：无，清空并重新填充 WeaponStates；仅在开局或明确重置本局时调用。
	 */
	void InitializeComputedWeapons();
};
