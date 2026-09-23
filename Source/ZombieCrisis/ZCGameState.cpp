// Fill out your copyright notice in the Description page of Project Settings.


#include "ZCGameState.h"

int32 AZCGameState::GetCurrentScore() const
{
    return CurrentScore;
}

void AZCGameState::AddScore(int32 Amount)
{
    if (Amount < 0) return;

    CurrentScore += Amount;
    OnScoreChange.Broadcast(CurrentScore);
}
