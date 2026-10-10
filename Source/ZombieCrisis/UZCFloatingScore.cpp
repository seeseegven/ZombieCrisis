// Fill out your copyright notice in the Description page of Project Settings.


#include "UZCFloatingScore.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/TextBlock.h"

void UUZCFloatingScore::StartFlight(const FVector2D& InStartScreenPosition)
{
	StartScreenPosition = InStartScreenPosition;
	ElapsedTime = 0.f;
	FlightDuration = 1.5f;
	bIsFlying = true;

	SetPositionInViewport(StartScreenPosition, false);
}

void UUZCFloatingScore::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (!bIsFlying) return;

	ElapsedTime += InDeltaTime;
	if (ElapsedTime <= WaitDuration) return;
	
	TotalDuration = FlyDuration + WaitDuration;
	const float Alpha = FMath::Clamp((ElapsedTime - WaitDuration) / FlyDuration, 0.f, 1.f);

	const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportWidgetGeometry(this).GetLocalSize();
	const FVector2D TargetScreenPosition{ ViewportSize.X * 0.5,-ViewportSize.Y };
	const FVector2D CurrentPosition = FMath::Lerp(StartScreenPosition, TargetScreenPosition, Alpha);


	SetPositionInViewport(CurrentPosition, false);
	if (Alpha >= 1.f) {
		bIsFlying = false;
		RemoveFromParent();
	}
}
