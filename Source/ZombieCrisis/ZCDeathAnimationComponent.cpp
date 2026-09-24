#include "ZCDeathAnimationComponent.h"

#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

UZCDeathAnimationComponent::UZCDeathAnimationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UZCDeathAnimationComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(false);
}

void UZCDeathAnimationComponent::PlayDeathAnimation()
{
	if (bIsPlaying) {
		return;
	}

	AActor* Owner = GetOwner();//组件拥有者
	if (!IsValid(Owner)) {
		return;
	}

	bIsPlaying = true;
	DisableOwnerGameplay();

	VisualRoot = FindVisualRoot();
	if (IsValid(VisualRoot)) {
		StartRelativeRotation = VisualRoot->GetRelativeRotation();
		TargetRelativeRotation = StartRelativeRotation;
		TargetRelativeRotation.Pitch = FRotator::NormalizeAxis(
			StartRelativeRotation.Pitch + BackwardFallPitch
		);
		ElapsedFallTime = 0.0f;
		SetComponentTickEnabled(true);
	}
	else {
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("%s cannot play death animation: scene component '%s' was not found."),
			*GetNameSafe(Owner),
			*VisualRootName.ToString()
		);
	}

	if (DestroyDelay <= 0.0f) {
		DestroyOwner();
		return;
	}

	Owner->GetWorldTimerManager().SetTimer(
		DestroyTimerHandle,
		this,
		&UZCDeathAnimationComponent::DestroyOwner,
		DestroyDelay,
		false
	);
}

void UZCDeathAnimationComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsPlaying || !IsValid(VisualRoot)) {
		SetComponentTickEnabled(false);
		return;
	}
	//					总时长																非常小的正数，趋近于0，避免除0错误
	const float SafeFallDuration = FMath::Max(FallDuration, KINDA_SMALL_NUMBER);
	ElapsedFallTime = FMath::Min(ElapsedFallTime + DeltaTime, SafeFallDuration);//已经播放的时长
	const float LinearAlpha = ElapsedFallTime / SafeFallDuration;
	const float SmoothAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, LinearAlpha, 2.0f);
	const FQuat NewRotation = FQuat::Slerp(
		StartRelativeRotation.Quaternion(),
		TargetRelativeRotation.Quaternion(),
		SmoothAlpha
	);
	VisualRoot->SetRelativeRotation(NewRotation);
	if (LinearAlpha >= 1.0f) {
		VisualRoot->SetRelativeRotation(TargetRelativeRotation);
		SetComponentTickEnabled(false);
	}
}

USceneComponent* UZCDeathAnimationComponent::FindVisualRoot() const
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner)) {
		return nullptr;
	}

	TArray<USceneComponent*> SceneComponents;
	Owner->GetComponents(SceneComponents);

	for (USceneComponent* SceneComponent : SceneComponents) {
		if (IsValid(SceneComponent) && SceneComponent->GetFName() == TEXT("Scene")) {
			return SceneComponent;
		}
	}

	return nullptr;
}

void UZCDeathAnimationComponent::DisableOwnerGameplay()
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner)) {
		return;
	}

	Owner->SetCanBeDamaged(false);//不能受到伤害
	Owner->SetActorEnableCollision(false);//禁止碰撞

	ACharacter* Character = Cast<ACharacter>(Owner);
	if (!Character) {
		return;
	}

	if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement()) {
		Movement->StopMovementImmediately();
		Movement->DisableMovement();//停止移动
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController())) {
		Character->DisableInput(PlayerController);//禁止输入
	}
}

void UZCDeathAnimationComponent::DestroyOwner()
{
	if (AActor* Owner = GetOwner(); IsValid(Owner)) {
		Owner->Destroy();
	}
}
