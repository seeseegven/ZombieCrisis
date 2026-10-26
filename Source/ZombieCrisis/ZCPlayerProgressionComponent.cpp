// Fill out your copyright notice in the Description page of Project Settings.


#include "ZCPlayerProgressionComponent.h"
#include "Engine/DataTable.h"
#include "ZCGameMode.h"

// Sets default values for this component's properties
UZCPlayerProgressionComponent::UZCPlayerProgressionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UZCPlayerProgressionComponent::InitializeProgression()
{
	PlayerLevel = 0;
	InitializeComputedWeapons();
}

/**
 * 将定义表的基础配置复制为本局使用的 Computed 配置。
 * 参数：无，数据来自组件的 WeaponDefinitions。
 * 返回值：无，结果按武器行名写入 WeaponStates。
 * 每行都从默认值开始，仅复制适用字段，避免上一把武器的数据串到下一把。
 */
void UZCPlayerProgressionComponent::InitializeComputedWeapons()
{
	// 这是本局初始化入口，重新调用会覆盖已经获得的成长结果。
	// 后续处理升级奖励时，应直接修改记录，不能再次调用这个函数。
	WeaponStates.Reset();

	if (!IsValid(WeaponDefinitions)) {
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Progression: WeaponDefinitions is not assigned.")
		);
		return;
	}

	// UE GetRowStruct() 取得表的行结构；StaticStruct() 取得项目结构的反射类型。
	// 防止误填奖励表后，只凭相同的行名建立出错误的武器配置。
	if (WeaponDefinitions->GetRowStruct() != FZCWeaponDefinitionRow::StaticStruct())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Progression: WeaponDefinitions must use FZCWeaponDefinitionRow."));
		return;
	}

	const TArray<FName> WeaponNames = WeaponDefinitions->GetRowNames();
	for (const FName& WeaponName : WeaponNames) {
		// UE FindRow() 按当前行名读取配置；日志上下文用于定位这次初始化。
		const FZCWeaponDefinitionRow* Definition =
			WeaponDefinitions->FindRow<FZCWeaponDefinitionRow>(
				WeaponName, TEXT("InitializeComputedWeapons"));
		if (!Definition)
		{
			continue;
		}

		// 每把武器单独创建一份默认结果：不适用数值为零、资源为空。
		FZCWeaponComputedStats Computed;
		Computed.DisplayName = Definition->DisplayName;
		Computed.VisualClass = Definition->VisualClass;
		Computed.VisualRelativeTransform = Definition->VisualRelativeTransform;
		Computed.WeaponClass = Definition->WeaponClass;
		Computed.WeaponType = Definition->WeaponType;
		Computed.UseMode = Definition->UseMode;
		Computed.FireRate = Definition->BaseFireRate;
		Computed.bUseAmmo = Definition->bUseAmmo;

		// 是否消耗数量由显式开关决定，不能把数量零当成无限弹药。
		if (Computed.bUseAmmo)
		{
			Computed.MagazineSize = Definition->BaseMagazineSize;
		}

		switch (Definition->WeaponType)
		{
		case EZCWeaponType::Gun:
			Computed.Damage = Definition->BaseDamage;
			Computed.Range = Definition->BaseRange;
			Computed.TraceCount = Definition->BaseTraceCount;
			Computed.bCanPassFalseWall = Definition->bCanPassFalseWall;
			Computed.MuzzleFlashSystem = Definition->MuzzleFlashSystem;
			Computed.WeaponEffectSystem = Definition->WeaponEffectSystem;

			// 单弹道不使用起始偏角和弹道间隔；多弹道才复制这两个参数。
			if (Computed.TraceCount >= 2)
			{
				Computed.StartDegreeOffset = Definition->StartDegreeOffset;
				Computed.SpreadDegrees = Definition->BaseSpreadDegrees;
			}
			break;

		case EZCWeaponType::Explosive:
			Computed.Damage = Definition->BaseDamage;
			Computed.ExplosionRadius = Definition->ExplosiveConfig.BaseExplosionRadius;
			Computed.TriggerType = Definition->ExplosiveConfig.TriggerType;
			Computed.WorldActorClass = Definition->WorldActorClass;
			Computed.PlacementOffset = Definition->PlacementOffset;
			// 只是复制初速度配置，不在这里执行投掷；实际是否使用由场景实体类决定。
			Computed.ThrowVelocity = Definition->ThrowVelocity;

			switch (Computed.TriggerType)
			{
			case EZCExplosiveTriggerType::Timed:
				Computed.FuseDelay = Definition->ExplosiveConfig.FuseDelay;
				break;
			case EZCExplosiveTriggerType::Overlap:
				Computed.OverlapTriggerRadius = Definition->ExplosiveConfig.OverlapTriggerRadius;
				break;
			case EZCExplosiveTriggerType::Damage:
				Computed.MaxHealth = Definition->BaseMaxHealth;
				break;
			}
			break;

		case EZCWeaponType::Wall:
			Computed.WorldActorClass = Definition->WorldActorClass;
			Computed.PlacementOffset = Definition->PlacementOffset;
			Computed.MaxHealth = Definition->BaseMaxHealth;
			break;
		}

		// Map 的键就是定义表行名，后续可以用 TotalWeapons 中的 FName 直接 Find。
		WeaponStates.Add(WeaponName, Computed);
	}
	UE_LOG(
		LogTemp,
		Display,
		TEXT("Weapon computed data initialized. Count: %d"),
		WeaponStates.Num()
	);
}


void UZCPlayerProgressionComponent::ApplyNewLevelRewards()
{
	PlayerLevel++;
	checkf(IsValid(LevelRewards), TEXT("Invalid Rewards Table!"));

	const FName RowName(*FString::FromInt(PlayerLevel));
	FZCLevelRewardRow* Reward = LevelRewards->FindRow<FZCLevelRewardRow>(
		RowName,
		TEXT("New Rewards applied!"),
		false
	);
	if (!Reward) return;

	auto Type = Reward->UpgradeType;
	AZCGameMode* GameMode = Cast<AZCGameMode>(GetOwner());
	checkf(IsValid(GameMode), TEXT("Invalid GameMode Parent!"));
	switch (Type)
	{
	case EZCPlayerUpgradeType::None:
		break;
	case EZCPlayerUpgradeType::MaxHealthUp:
		GameMode->UpgradePlayerMaxHealth(Reward->Value);
		break;
	case EZCPlayerUpgradeType::Unlock: {
		GameMode->UnlockWeapon();
		break;
	}
	case EZCPlayerUpgradeType::DamageMultiplier:
	case EZCPlayerUpgradeType::FireRateMultiplier:
	case EZCPlayerUpgradeType::AmmoMultiplier:
	case EZCPlayerUpgradeType::RangeMultiplier:
	case EZCPlayerUpgradeType::SpreadMultiplier:
	case EZCPlayerUpgradeType::ExplosionRadiusMultiplier:
	{
		// GameMode 负责总数组和索引，组件只修改自己保存的最终属性。
		const FName WeaponName = GameMode->GetNextUpgradeWeapon(Type);
		FZCWeaponComputedStats* Computed = WeaponStates.Find(WeaponName);
		if (!Computed) break;

		// Value 是本次乘数；循环回同一武器时，在已经强化的数值上继续相乘。
		switch (Type)
		{
		case EZCPlayerUpgradeType::DamageMultiplier: Computed->Damage *= Reward->Value; break;
		case EZCPlayerUpgradeType::FireRateMultiplier: Computed->FireRate *= Reward->Value; break;
		case EZCPlayerUpgradeType::AmmoMultiplier:
			// 数量仍是整数；UE RoundToInt 将倍率计算后的结果四舍五入。
			Computed->MagazineSize = FMath::RoundToInt(Computed->MagazineSize * Reward->Value);
			break;
		case EZCPlayerUpgradeType::RangeMultiplier: Computed->Range *= Reward->Value; break;
		case EZCPlayerUpgradeType::SpreadMultiplier:
			Computed->SpreadDegrees *= Reward->Value;
			// 同比例调整起始偏角，保持原来弹道排列的相对形状。
			Computed->StartDegreeOffset *= Reward->Value;
			break;
		case EZCPlayerUpgradeType::ExplosionRadiusMultiplier:
			Computed->ExplosionRadius *= Reward->Value;
			break;
		default: break;
		}

		// 每次成功更新后都通知；只有弹量奖励会让玩家补满对应武器。
		GameMode->NotifyWeaponPropertiesChanged(WeaponName, Type == EZCPlayerUpgradeType::AmmoMultiplier);
		break;
	}
	default:
		break;
	}
}

const TMap<FName, FZCWeaponComputedStats>& UZCPlayerProgressionComponent::GetWeaponStates() const
{
	return WeaponStates;
}

// Called when the game starts
void UZCPlayerProgressionComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeProgression();
	// ...
	
}

