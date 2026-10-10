// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UZCFloatingScore.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class ZOMBIECRISIS_API UUZCFloatingScore : public UUserWidget
{
	GENERATED_BODY()
public:
	FVector InitialWorldLocation = FVector::ZeroVector;

	void StartFlight(const FVector2D& InStartScreenPosition);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ScoreText = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Time", meta = (ClampMin = "0.02"))
	float FlyDuration = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Time", meta = (ClampMin = "0.02"))
	float WaitDuration = 0.25f;

	float TotalDuration;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	FVector2D StartScreenPosition = FVector2D::ZeroVector;
	float ElapsedTime = 0.f;
	float FlightDuration = 1.f;
	bool bIsFlying = false;

};
