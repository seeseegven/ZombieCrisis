#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "ZCDeathAnimationComponent.generated.h"

class USceneComponent;

UCLASS(ClassGroup = (ZombieCrisis))
class ZOMBIECRISIS_API UZCDeathAnimationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UZCDeathAnimationComponent();

	void PlayDeathAnimation();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

private:
	USceneComponent* FindVisualRoot() const;
	void DisableOwnerGameplay();
	void DestroyOwner();

	UPROPERTY(EditDefaultsOnly, Category = "ZC|Death", meta = (ClampMin = "0.01"))
	float FallDuration = 0.55f;

	UPROPERTY(EditDefaultsOnly, Category = "ZC|Death")
	float BackwardFallPitch = 90.0f;

	UPROPERTY(EditDefaultsOnly, Category = "ZC|Death", meta = (ClampMin = "0.0"))
	float DestroyDelay = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "ZC|Death")
	FName VisualRootName = TEXT("VisualRoot");

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> VisualRoot = nullptr;

	FRotator StartRelativeRotation = FRotator::ZeroRotator;
	FRotator TargetRelativeRotation = FRotator::ZeroRotator;
	FTimerHandle DestroyTimerHandle;
	float ElapsedFallTime = 0.0f;
	bool bIsPlaying = false;
};
