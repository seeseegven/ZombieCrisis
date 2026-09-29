// Fill out your copyright notice in the Description page of Project Settings.


#include "UZCFloatingScore.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UUZCFloatingScore::StartFlight(const FVector2D& InStartScreenPosition)
{
	StartScreenPosition = InStartScreenPosition;
	ElapsedTime = 0.f;
	bIsFlying = true;

	SetPositionInViewport(StartScreenPosition, false);
}

void UUZCFloatingScore::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (!bIsFlying) return;

	ElapsedTime += InDeltaTime;
	const float Alpha = FMath::Clamp(ElapsedTime / FlightDuration, 0.f, 1.f);

	const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportWidgetGeometry(this).GetLocalSize();
	const FVector2D TargetScreenPosition{ ViewportSize.X * 0.5,-ViewportSize.Y };
	const FVector2D CurrentPosition = FMath::Lerp(StartScreenPosition, TargetScreenPosition, Alpha);

	SetPositionInViewport(CurrentPosition, false);
	if (Alpha >= 0.1f) {
		bIsFlying = false;
		RemoveFromParent();
	}
}
