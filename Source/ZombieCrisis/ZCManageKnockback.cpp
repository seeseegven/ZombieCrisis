// Fill out your copyright notice in the Description page of Project Settings.


#include "ZCManageKnockback.h"
#include "Components/CapsuleComponent.h"              // UCapsuleComponent
#include "Engine/World.h"                              // UWorld、GetTimerManager
#include "GameFramework/Character.h"                   // ACharacter
#include "GameFramework/CharacterMovementComponent.h" // UCharacterMovementComponent
#include "TimerManager.h"                              // SetTimer、ClearTimer
#include "AIController.h"

// Sets default values for this component's properties
UZCManageKnockback::UZCManageKnockback()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UZCManageKnockback::StartKnockback(const FVector& Direction, float BackSpeed, float BackDuration)
{
	UWorld* World = GetWorld();
	ACharacter* Owner = Cast<ACharacter>(GetOwner());

	if (!IsValid(World) || !IsValid(Owner) || BackSpeed <= 0.f || BackDuration <= 0.f) return;

	if (Direction.IsNearlyZero()) return;

	KnockbackDirection = Direction;
	KnockbackSpeed = BackSpeed;
	bKnockbackActive = true;

	AAIController* Controller = Cast<AAIController>(Owner->GetController());
	if (IsValid(Controller)) {
		Controller->StopMovement();
	}

	Owner->SetActorRotation((- KnockbackDirection).Rotation());
	Owner->LaunchCharacter(KnockbackDirection * BackSpeed, true, true);

	World->GetTimerManager().SetTimer(
		KnockbackTimerHandle,
		this,
		&UZCManageKnockback::StopKnockback,//到时运行
		BackDuration,
		false//不循环
	);
}

void UZCManageKnockback::StopKnockback()
{
	if (!bKnockbackActive) return;

	bKnockbackActive = false;

	UWorld* World = GetWorld();
	if (IsValid(World)) {
		World->GetTimerManager().ClearTimer(KnockbackTimerHandle);
	}
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!IsValid(Owner)) return;

	UCharacterMovementComponent* Movement = Owner->GetCharacterMovement();
	if (!IsValid(Movement)) return;

	Movement->ClearAccumulatedForces();
	Movement->StopMovementKeepPathing();
}

bool UZCManageKnockback::GetKnockbackStatus() const
{
	return bKnockbackActive;
}


// Called when the game starts
void UZCManageKnockback::BeginPlay()
{
	Super::BeginPlay();

	// ...
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!IsValid(Owner)) return;

	UCapsuleComponent* Capsule = Owner->GetCapsuleComponent();
	if (!IsValid(Capsule)) return;

	Capsule->OnComponentHit.AddUniqueDynamic(
		this,
		&UZCManageKnockback::HandleCapsuleHit
	);
}

void UZCManageKnockback::HandleCapsuleHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!bKnockbackActive || !Hit.bBlockingHit) return;//bBlockingHit是阻挡结果

	const FVector BeKnockedSurface = -Hit.ImpactNormal.GetSafeNormal2D();
	if (BeKnockedSurface.IsNearlyZero()) return;

	float DirectionAgreement = FVector::DotProduct(KnockbackDirection, BeKnockedSurface);

	if (DirectionAgreement >= 0.5f) {
		
		if (IsValid(OtherActor)&&Cast<ACharacter>(OtherActor)) {
			UZCManageKnockback* OtherKnockback = OtherActor->FindComponentByClass<UZCManageKnockback>();
			if (IsValid(OtherKnockback)) {
				OtherKnockback->StartKnockback(
					BeKnockedSurface,
					//300.f,
					KnockbackSpeed,
					0.35f
				);
				StopKnockback();
			}
		}
	}
}


