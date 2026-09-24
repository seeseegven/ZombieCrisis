// Fill out your copyright notice in the Description page of Project Settings.


#include "ZCGameMode.h"
#include "ZCGameState.h"

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
	UE_LOG(
		LogTemp,
		Display,
		TEXT("Enemy killed: %s | Killer: %s | Total score: %d"),
		*GetNameSafe(KilledEnemy),
		*GetNameSafe(KillerController),
		ZCGameState->GetCurrentScore()
	);
}
