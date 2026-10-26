#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "ZCWeaponBasic.h"
#include "ZCWeaponTypes.generated.h"

class UNiagaraSystem;

//武器类型
UENUM()
enum class EZCWeaponType : uint8 {
	Gun		UMETA(DisplayName = "枪械"),
	Explosive UMETA(DisplayName = "爆炸物"),
	Wall      UMETA(DisplayName = "障碍物")
};

//攻击方式(单发，连发）
UENUM()
enum class EZCWeaponUseMode : uint8
{
	SinglePress UMETA(DisplayName = "按一次使用一次"),
	RepeatWhileHeld UMETA(DisplayName = "按住持续使用")
};

//爆炸物引爆方式
UENUM()
enum class EZCExplosiveTriggerType : uint8
{
	Timed   UMETA(DisplayName = "延迟引爆"),
	Overlap UMETA(DisplayName = "重叠引爆"),
	Damage  UMETA(DisplayName = "受击引爆")
};
/**
 * 一条等级奖励会对武器产生哪种影响。
 *
 * Multiplier 类型保存本次奖励的乘数，直接作用于 Computed 的当前最终值。
 * 同一武器先后获得两次 2.0 时，最终是基础值的四倍；不另存累计倍率。
 */
UENUM(BlueprintType)
enum class EZCPlayerUpgradeType : uint8
{
	None UMETA(DisplayName = "啥也木有"),
	MaxHealthUp UMETA(DisplayName = "升级后的血量"),
	Unlock UMETA(DisplayName = "解锁武器"),
	DamageMultiplier UMETA(DisplayName = "伤害倍率"),
	FireRateMultiplier UMETA(DisplayName = "射速倍率"),
	AmmoMultiplier UMETA(DisplayName = "弹容量倍率"),
	RangeMultiplier UMETA(DisplayName = "射程倍率"),
	SpreadMultiplier UMETA(DisplayName = "散射范围倍率"),
	ExplosionRadiusMultiplier UMETA(DisplayName = "爆炸半径倍率")
};


/**
 * 爆炸物专属配置。
 * 手雷、地雷和油桶共用这组结构，通过 TriggerType 区分引爆条件。
 * 基础伤害沿用武器定义行中的 BaseDamage。
 */
USTRUCT()
struct FZCExplosiveConfig
{
	GENERATED_BODY()

	// 爆炸物何时引爆：手雷用 Timed，地雷用 Overlap，油桶用 Damage。
	UPROPERTY(EditAnywhere, Category = "Explosive")
	EZCExplosiveTriggerType TriggerType = EZCExplosiveTriggerType::Timed;

	// 爆炸伤害的基础作用半径，单位为厘米。
	// 后续“爆炸范围强化”以这个基础值计算，不借用枪械射程。
	UPROPERTY(EditAnywhere, Category = "Explosive", meta = (ClampMin = "0.0"))
	float BaseExplosionRadius = 300.0f;

	// 延迟引爆等待的秒数，仅 Timed 模式使用。
	// 地雷和油桶不会因为结构中保存了这个值就自动开始倒计时。
	UPROPERTY(EditAnywhere, Category = "Explosive", meta = (ClampMin = "0.0"))
	float FuseDelay = 2.0f;

	// 重叠触发检测范围的半径，单位为厘米，仅 Overlap 模式使用。
	// 它表示“多近会触发”，与“爆炸后伤害多远”是两个独立范围。
	UPROPERTY(EditAnywhere, Category = "Explosive", meta = (ClampMin = "0.0"))
	float OverlapTriggerRadius = 80.0f;
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bUseAmmo = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = "1"))
	int32 BaseTraceCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AZCWeaponBasic> WeaponClass;

	UPROPERTY(EditAnywhere, Category = "Weapon|FX")
	TObjectPtr<UNiagaraSystem> MuzzleFlashSystem = nullptr;

	UPROPERTY(EditAnywhere, Category = "Weapon|FX")
	TObjectPtr<UNiagaraSystem> WeaponEffectSystem = nullptr;

	// 旧武器行默认仍属于枪械。
	UPROPERTY(EditAnywhere, Category = "Weapon|Common")
	EZCWeaponType WeaponType = EZCWeaponType::Gun;

	// 旧枪械默认沿用按住持续使用的方式。
	// 新建炸弹、假墙行时，需要明确设置成 SinglePress。
	UPROPERTY(EditAnywhere, Category = "Weapon|Common")
	EZCWeaponUseMode UseMode = EZCWeaponUseMode::RepeatWhileHeld;

	// 使用武器后，实际生成在世界中的实体类。
	// 爆炸物可以填写手雷、地雷、油桶的场景类；假墙填写墙体场景类。
	// 这个实体独立于当前装备的逻辑武器，切枪后仍然可以存在。
	UPROPERTY(EditAnywhere, Category = "Weapon|Placement")
	TSubclassOf<AActor> WorldActorClass;

	// 只有爆炸物类别会读取这组配置。
	UPROPERTY(EditAnywhere, Category = "Weapon|Explosive")
	FZCExplosiveConfig ExplosiveConfig;

	// true：这把枪允许穿过假墙；false：受到假墙阻挡。
	// 后续射击代码根据它选择对应的查询通道。
	UPROPERTY(EditAnywhere, Category = "Weapon|Gun")
	bool bCanPassFalseWall = false;

	// 投放到世界中的实体生命上限：假墙和受击引爆的爆炸物使用。
	// 每个生成出来的 Actor 根据这个上限建立自己的当前生命，配置表不保存剩余生命。
	UPROPERTY(EditAnywhere, Category = "Weapon|WorldActor", meta = (ClampMin = "0.0"))
	float BaseMaxHealth = 100.0f;

	// 投放起点相对玩家的局部位置偏移，单位为厘米。
	// X 向前、Y 向右、Z 向上；实际世界落点由以后投放时计算。
	// 它与手持外观的 VisualRelativeTransform 分别配置。
	UPROPERTY(EditAnywhere, Category = "Weapon|Placement")
	FVector PlacementOffset = FVector::ZeroVector;

	// 投掷型实体的局部初速度，单位为厘米/秒，X 向前、Y 向右、Z 向上。
	// 手雷类以后读取它来发起投掷；不投掷的地雷、油桶等保持为零。
	UPROPERTY(EditAnywhere, Category = "Weapon|Placement")
	FVector ThrowVelocity = FVector::ZeroVector;
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
	EZCPlayerUpgradeType UpgradeType = EZCPlayerUpgradeType::Unlock;

	/** Multiplier：本次乘数；MaxHealthUp：新生命上限；Unlock/None：忽略该值。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression", meta = (ClampMin = "0.0"))
	float Value = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FText Description;
};

/**
 * 本局某把武器的最终配置。
 * 开局从定义表复制适用字段，后续升级直接更新这里的数值。
 * 由成长组件按武器行名保存；不适用的数值保持为零，由 WeaponType 等配置决定是否读取。
 * 这里只保存投放实体的生命上限，不保存任何已生成 Actor 的当前生命。
 */
USTRUCT(BlueprintType)
struct FZCWeaponComputedStats
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	FText DisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AActor> VisualClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bUnlocked = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	float Damage = 0.0f;

	// 每秒允许使用的次数，供开火、投掷和放置逻辑计算冷却。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	float FireRate = 0.0f;

	// 初始可用数量；玩家当前剩余数量继续由玩家自己的记录保存。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 MagazineSize = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	float Range = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	float SpreadDegrees = 0.0f;

	// 计算后的爆炸伤害半径，单位为厘米。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	float ExplosionRadius = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Weapon|Common")
	EZCWeaponType WeaponType = EZCWeaponType::Gun;

	UPROPERTY(VisibleAnywhere, Category = "Weapon|Common")
	EZCWeaponUseMode UseMode = EZCWeaponUseMode::RepeatWhileHeld;

	UPROPERTY(VisibleAnywhere, Category = "Weapon|Common")
	TSubclassOf<AZCWeaponBasic> WeaponClass;

	UPROPERTY(VisibleAnywhere, Category = "Weapon|Common")
	FTransform VisualRelativeTransform = FTransform::Identity;

	// false 表示不按数量拦截或消耗；此时 MagazineSize 保持为零。
	UPROPERTY(VisibleAnywhere, Category = "Weapon|Common")
	bool bUseAmmo = false;

	// 枪械每次使用的射线条数；非枪械保持为零。
	UPROPERTY(VisibleAnywhere, Category = "Weapon|Gun")
	int32 TraceCount = 0;

	// 多弹道枪械的起始偏角，单位为度；单弹道时不使用。
	UPROPERTY(VisibleAnywhere, Category = "Weapon|Gun")
	float StartDegreeOffset = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Weapon|Gun")
	bool bCanPassFalseWall = false;

	UPROPERTY(VisibleAnywhere, Category = "Weapon|FX")
	TObjectPtr<UNiagaraSystem> MuzzleFlashSystem = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Weapon|FX")
	TObjectPtr<UNiagaraSystem> WeaponEffectSystem = nullptr;

	// 使用后实际留在世界中的实体类，仅爆炸物和假墙使用。
	UPROPERTY(VisibleAnywhere, Category = "Weapon|Placement")
	TSubclassOf<AActor> WorldActorClass;

	UPROPERTY(VisibleAnywhere, Category = "Weapon|Placement")
	FVector PlacementOffset = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, Category = "Weapon|Placement")
	FVector ThrowVelocity = FVector::ZeroVector;

	// 仅 WeaponType 为 Explosive 时读取；其他类别忽略这个枚举的默认值。
	UPROPERTY(VisibleAnywhere, Category = "Weapon|Explosive")
	EZCExplosiveTriggerType TriggerType = EZCExplosiveTriggerType::Timed;

	// 仅延迟引爆使用，单位为秒。
	UPROPERTY(VisibleAnywhere, Category = "Weapon|Explosive")
	float FuseDelay = 0.0f;

	// 仅重叠引爆使用，单位为厘米。
	UPROPERTY(VisibleAnywhere, Category = "Weapon|Explosive")
	float OverlapTriggerRadius = 0.0f;

	// 投放假墙和受击引爆物的初始生命上限；各实体剩余生命保存在自身 Actor 中。
	UPROPERTY(VisibleAnywhere, Category = "Weapon|WorldActor")
	float MaxHealth = 0.0f;
};
