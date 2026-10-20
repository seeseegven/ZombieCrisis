// Fill out your copyright notice in the Description page of Project Settings.


#include "ZCGameMode.h"
#include "ZCGameState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "ZCZombieSpawner.h"

AZCGameMode::AZCGameMode()
{
	GameStateClass = AZCGameState::StaticClass();
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

}
