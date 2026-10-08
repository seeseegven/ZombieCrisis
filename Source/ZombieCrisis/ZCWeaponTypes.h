#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "ZCWeaponBasic.h"
#include "ZCWeaponTypes.generated.h"

class UNiagaraSystem;

/**
 * 一条等级奖励会对武器产生哪种影响。
 *
 * Multiplier 类型保存的是“强化后的目标倍率”，而不是在旧值上继续相乘。
 * 例如双倍弹容量写 2.0，四倍弹容量写 4.0，避免最终错误地叠成八倍。
 */
UENUM(BlueprintType)
enum class EZCWeaponUpgradeType : uint8
{
	Unlock UMETA(DisplayName = "解锁武器"),
	DamageMultiplier UMETA(DisplayName = "伤害倍率"),
	FireRateMultiplier UMETA(DisplayName = "射速倍率"),
	AmmoMultiplier UMETA(DisplayName = "弹容量倍率"),
	RangeMultiplier UMETA(DisplayName = "射程倍率"),
	SpreadMultiplier UMETA(DisplayName = "散射范围倍率")
};


/**
 * 武器不会随玩家等级变化的基础数据。
 * 该结构继承 FTableRowBase，因此可以用来创建武器定义 DataTable。
 */
USTRUCT(BlueprintType)
struct FZCWeaponDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AActor> VisualClass;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	FTransform VisualRelativeTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = "0.0"))
	float BaseDamage = 10.0f;

	/** 每秒可发射的次数。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = "0.0"))
	float BaseFireRate = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = "0"))
	int32 BaseMagazineSize = 12;

	/** 射线或弹丸的基础有效距离，单位为厘米。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = "0.0"))
	float BaseRange = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = "0.0"))
	float BaseSpreadDegrees = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float StartDegreeOffset = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = "1"))
	int32 BaseTraceCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AZCWeaponBasic> WeaponClass;

	UPROPERTY(EditAnywhere, Category = "Weapon|FX")
	TObjectPtr<UNiagaraSystem> MuzzleFlashSystem = nullptr;

	UPROPERTY(EditAnywhere, Category = "Weapon|FX")
	TObjectPtr<UNiagaraSystem> WeaponEffectSystem = nullptr;
};


/**
 * 某个等级会授予的一条武器奖励。
 * 该结构用于创建等级奖励 DataTable。
 */
USTRUCT(BlueprintType)
struct FZCLevelRewardRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression", meta = (ClampMin = "1"))
	int32 RequiredLevel = 1;

	/** 必须与武器定义 DataTable 中的行名一致，例如 Pistol 或 Shotgun。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName WeaponRowName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	EZCWeaponUpgradeType UpgradeType = EZCWeaponUpgradeType::Unlock;

	/** 强化后的目标倍率；Unlock 类型忽略该值。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression", meta = (ClampMin = "0.0"))
	float Value = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FText Description;
};


/**
 * 一局游戏中，某把武器已经获得的解锁与强化状态。
 * 这是运行时数据，不直接存进 DataTable。
 */
USTRUCT(BlueprintType)
struct FZCWeaponRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bUnlocked = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	float DamageMultiplier = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	float FireRateMultiplier = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	float AmmoMultiplier = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	float RangeMultiplier = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	float SpreadMultiplier = 1.0f;
};


/**
 * 基础数据乘以运行时倍率后，提供给射击与蓝图表现逻辑的最终数据。
 */
USTRUCT(BlueprintType)
struct FZCWeaponComputedStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AActor> VisualClass;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	bool bUnlocked = false;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	float Damage = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	float FireRate = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	int32 MagazineSize = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	float Range = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	float SpreadDegrees = 0.0f;
};
