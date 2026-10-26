// Fill out your copyright notice in the Description page of Project Settings.


#include "ZCGameMode.h"
#include "ZCGameState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "ZCZombieSpawner.h"
#include "ZCPlayerProgressionComponent.h"
#include "ZCPlayerCharacter.h"

AZCGameMode::AZCGameMode()
{
	GameStateClass = AZCGameState::StaticClass();
	PlayerProgression = CreateDefaultSubobject<UZCPlayerProgressionComponent>(
		FName("WeaponProgression")
	);
}

void AZCGameMode::HandleEnemyKilled(AActor* KilledEnemy, AController* KillerController)
{
	if (!IsValid(KilledEnemy)) return;

	AZCGameState* ZCGameState = Cast<AZCGameState>(GameState);

	if (!IsValid(ZCGameState)) {
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("转换失败，无法添加分数")
			);
		return;
	}
	ZCGameState->AddScore(ZombieKilledScore);
	CurrentAliveZombie--;
	if (CurrentAliveZombie <= 0) {
		if (IsValid(PlayerProgression)) {
			int32 CurrentLevel = PlayerProgression->GetPlayerLevel();
			PlayerProgression->ApplyNewLevelRewards();
			UE_LOG(
				LogTemp,
				Display,
				TEXT("Wave cleared. Player level: %d"),
				PlayerProgression->GetPlayerLevel()
			);
		}
		Wave++;
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Current wave ended.")
		);

		GetWorldTimerManager().SetTimer(
			WaveTimerHandle,
			this,
			&AZCGameMode::SpawningEachWave,
			5.0f,
			false
			);
	}
	UE_LOG(
		LogTemp,
		Display,
		TEXT("Enemy killed: %s | Killer: %s | Total score: %d"),
		*GetNameSafe(KilledEnemy),
		*GetNameSafe(KillerController),
		ZCGameState->GetCurrentScore()
	);
}

void AZCGameMode::HandlePlayerDied()
{
	AZCGameState* CurrentGameState = Cast<AZCGameState>(GameState);

	if (!IsValid(CurrentGameState)) {
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Error. Invalid GameState")
		);
		return;
	}

	BP_DisplayFinalScore(CurrentGameState->GetCurrentScore());
}

void AZCGameMode::StartPlay()
{
	Super::StartPlay();

	Wave = 1;
	checkf(
		IsValid(PlayerProgression),
		TEXT("Invalid Component!")
	);
	PlayerProgression->ApplyNewLevelRewards();
	ZombieSpawners.Reset();

	TArray<AActor*> SpawnersInMap;

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AZCZombieSpawner::StaticClass(),
		SpawnersInMap
	);
	
	for (AActor* Spawner : SpawnersInMap) {
		AZCZombieSpawner* CheckedSpawner = Cast<AZCZombieSpawner>(Spawner);
		if (IsValid(CheckedSpawner)) {
			ZombieSpawners.Add(CheckedSpawner);
		}
	}

	SpawningEachWave();
}

void AZCGameMode::HandleRestartLevel()
{
	FString LevelName = UGameplayStatics::GetCurrentLevelName(this, true);

	if (LevelName.IsEmpty()) {
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Current Level is empty.")
		);
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!IsValid(PC)) {
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Player is not valid.")
		);
		return;
	}

	UGameplayStatics::SetGamePaused(this, false);
	UWidgetBlueprintLibrary::SetInputMode_GameOnly(PC, true);
	UWidgetLayoutLibrary::RemoveAllWidgets(this);

	UGameplayStatics::OpenLevel(this, FName(LevelName), true);
}

void AZCGameMode::HandleReturnMainMenu()
{
	UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(
		UGameplayStatics::GetPlayerController(this, 0),
		nullptr,
		EMouseLockMode::DoNotLock,
		true
	);
	UGameplayStatics::OpenLevel(
		this,
		"/Game/ZombieCrisis/Maps/L_ZC_MainMenu"
	);
}

void AZCGameMode::UnlockWeapon()
{
	AZCPlayerCharacter* Player = Cast<AZCPlayerCharacter>(
		UGameplayStatics::GetPlayerCharacter(this, 0)
	);
	if (IsValid(Player)) {
		if (UnlockIndex < Weapons.Num()) {
			Player->AddNewWeapon(Weapons[UnlockIndex]);
			UnlockIndex++;
		}
	}
}

/**
 * 为组件选择本次强化的武器，仍由 GameMode 保存各类强化的独立进度。
 * Type：奖励表中的升级类型，用来选择索引和判断哪些属性适用。
 * 返回武器定义行名；没有适用武器时返回 NAME_None，且不推进索引。
 * 解锁先后由奖励表保证，这里不额外检查武器是否已经解锁。
 */
FName AZCGameMode::GetNextUpgradeWeapon(EZCPlayerUpgradeType Type)
{
	if (Weapons.IsEmpty() || !IsValid(PlayerProgression)) return NAME_None;

	int32* UpgradeIndex = nullptr;
	switch (Type)
	{
	case EZCPlayerUpgradeType::DamageMultiplier: UpgradeIndex = &DamageMuliplierIndex; break;
	case EZCPlayerUpgradeType::FireRateMultiplier: UpgradeIndex = &FireRateIndex; break;
	case EZCPlayerUpgradeType::AmmoMultiplier: UpgradeIndex = &AmmoCapacityIndex; break;
	case EZCPlayerUpgradeType::RangeMultiplier: UpgradeIndex = &DamageRangeIndex; break;
	case EZCPlayerUpgradeType::SpreadMultiplier: UpgradeIndex = &SpreadMultiplierIndex; break;
	case EZCPlayerUpgradeType::ExplosionRadiusMultiplier: UpgradeIndex = &ExplosionRadiusIndex; break;
	default: return NAME_None;
	}

	const auto& ComputedWeapons = PlayerProgression->GetWeaponStates();
	// 最多查看总数组一遍；例如当前没有爆炸物时，不会无限查找爆炸范围强化目标。
	for (int32 Offset = 0; Offset < Weapons.Num(); ++Offset)
	{
		const int32 CandidateIndex = (*UpgradeIndex + Offset) % Weapons.Num();
		const FName WeaponName = Weapons[CandidateIndex];
		const FZCWeaponComputedStats* Computed = ComputedWeapons.Find(WeaponName);
		if (!Computed) continue;

		bool bApplicable = false;
		switch (Type)
		{
		case EZCPlayerUpgradeType::DamageMultiplier:
			bApplicable = Computed->WeaponType != EZCWeaponType::Wall;
			break;
		case EZCPlayerUpgradeType::FireRateMultiplier:
			// 枪械、投掷和放置都可以使用 FireRate 作为使用频率。
			bApplicable = true;
			break;
		case EZCPlayerUpgradeType::AmmoMultiplier:
			bApplicable = Computed->bUseAmmo;
			break;
		case EZCPlayerUpgradeType::RangeMultiplier:
			bApplicable = Computed->WeaponType == EZCWeaponType::Gun;
			break;
		case EZCPlayerUpgradeType::SpreadMultiplier:
			bApplicable = Computed->WeaponType == EZCWeaponType::Gun && Computed->TraceCount >= 2;
			break;
		case EZCPlayerUpgradeType::ExplosionRadiusMultiplier:
			bApplicable = Computed->WeaponType == EZCWeaponType::Explosive;
			break;
		default: break;
		}
		if (!bApplicable) continue;

		// 返回末项后，下次从 0 开始；每种奖励只推进自己的索引。
		*UpgradeIndex = (CandidateIndex + 1) % Weapons.Num();
		return WeaponName;
	}
	return NAME_None;
}

/**
 * 将组件的属性变更通知转交给当前玩家，组件不直接访问玩家 Actor。
 * WeaponName：本次修改的武器行名；bRefillAmmo：true 补到新配置数量，false 保留剩余弹药。
 * 返回值：无；玩家自行重新读取 Computed 并决定是否刷新当前装备。
 */
void AZCGameMode::NotifyWeaponPropertiesChanged(const FName& WeaponName, bool bRefillAmmo)
{
	AZCPlayerCharacter* Player = Cast<AZCPlayerCharacter>(
		UGameplayStatics::GetPlayerCharacter(this, 0));
	if (IsValid(Player)) Player->RefreshWeaponProperties(WeaponName, bRefillAmmo);
}

/**
 * 应用玩家生命奖励：NewMaxHealth 来自本级 Reward.Value，表示新的生命上限。
 * 返回值：无；具体生命计算与血条通知由拥有这些状态的玩家完成。
 */
void AZCGameMode::UpgradePlayerMaxHealth(float NewMaxHealth)
{
	AZCPlayerCharacter* Player = Cast<AZCPlayerCharacter>(
		UGameplayStatics::GetPlayerCharacter(this, 0));
	if (IsValid(Player)) Player->ApplyMaxHealthUpgrade(NewMaxHealth);
}

const TObjectPtr<UZCPlayerProgressionComponent> AZCGameMode::GetPlayerProgressionComponent() const
{
	return PlayerProgression;
}

int32 AZCGameMode::ManageSpawningCount()
{
	return Wave*2;
}

void AZCGameMode::SpawningEachWave()
{
	for (AZCZombieSpawner* Spawner : ZombieSpawners) {
		CurrentAliveZombie += ManageSpawningCount();
		Spawner->StartSpawning(ManageSpawningCount());
		UE_LOG(
			LogTemp,
			Display,
			TEXT("Start Wave Wave")
		);
	}
	BP_DisplayCurrentWave(Wave);
}


