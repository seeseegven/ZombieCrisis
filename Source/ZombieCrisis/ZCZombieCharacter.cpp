// Fill out your copyright notice in the Description page of Project Settings.


#include "ZCZombieCharacter.h"
#include "ZCDeathAnimationComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "TimerManager.h"
#include "ZCPlayerCharacter.h"
#include "ZCGameMode.h"
#include "UZCFloatingScore.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "GameFramework/PlayerController.h"

// Sets default values
AZCZombieCharacter::AZCZombieCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	DeathAnimationComponent = CreateDefaultSubobject<UZCDeathAnimationComponent>(
		TEXT("DeathAnimationComponent")
	);

}

void AZCZombieCharacter::HandleElimination()
{
	if (!IsValid(ScoreWidgetClass)) return;

	UUZCFloatingScore* ScoreUI = CreateWidget<UUZCFloatingScore>(GetWorld(), ScoreWidgetClass);
	if (!IsValid(ScoreUI)) return;
	ScoreUI->InitialWorldLocation = GetActorLocation() + FVector(0.f, 0.f, 120.f);
	FVector2D ScreenLocation;
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;
	if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
		PC,
		ScoreUI->InitialWorldLocation,
		ScreenLocation,
		false
	)) {
		return;
	}
	ScoreUI->AddToViewport();
	//ScoreUI->SetDesiredSizeInViewport(FVector2D(20.f, 40.f));
	//ScoreUI->SetAlignmentInViewport(FVector2D(0.5f, 1.f));
	ScoreUI->StartFlight(ScreenLocation);
}

bool AZCZombieCharacter::IsDead()
{
	return CurrentHealth <= 0;
}

float AZCZombieCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead) {
		return 0.0f;
	}

	const float ActualDamage = Super::TakeDamage(
		DamageAmount,
		DamageEvent,
		EventInstigator,
		DamageCauser
	);

	if (ActualDamage <= 0.f) {
		return 0.f;
	}
	CurrentHealth =
		//FMath::Clamp(
		CurrentHealth - ActualDamage;
		//0.0f,
		//MaxHealth
	//);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("%s took %.1f damage. Health: %.1f / %.1f"),
		*GetNameSafe(this),
		ActualDamage,
		CurrentHealth,
		MaxHealth
	);

	if (IsDead()) {
		bIsDead = true;
		bIsAttacking = false;
		bIsAnimationStarted = false;
		bIsTargetInAttackRange = false;
		AttackTarget = nullptr;
		UE_LOG(
			LogTemp,
			Display,
			TEXT("%s died. KillerController: %s"),
			*GetNameSafe(this),
			*GetNameSafe(EventInstigator)
		);
		AZCGameMode* ZCGameMode = Cast<AZCGameMode>(UGameplayStatics::GetGameMode(this));
		if (IsValid(ZCGameMode)) ZCGameMode->HandleEnemyKilled(this, EventInstigator);
		HandleElimination();
		DeathAnimationComponent->PlayDeathAnimation();
	}
	return ActualDamage;
}

void AZCZombieCharacter::StartAttack(AActor* NewTarget)
{
	if (!IsValid(NewTarget)) return;

	AZCPlayerCharacter*Player = Cast<AZCPlayerCharacter>(NewTarget);
	AttackTarget = Player;
	bIsTargetInAttackRange = true;

	if (bIsAttacking) return;

	bIsAttacking = true;
	bIsAnimationStarted = true;
	BP_PlayAttackAnimation();
}

void AZCZombieCharacter::StopAttack(AActor* LeavingTarget)
{
	if (LeavingTarget != AttackTarget) return;

	bIsAttacking = false;
	bIsTargetInAttackRange = false;
	AttackTarget = nullptr;
}

void AZCZombieCharacter::AttackHitCheck()
{
	if (!IsValid(AttackTarget)) return;

	if (!bIsAttacking || !bIsTargetInAttackRange) {
		AttackTarget = nullptr;
		return;
	}

	UGameplayStatics::ApplyDamage(
		AttackTarget,
		AttackDamage,
		GetController(),
		this,
		UDamageType::StaticClass() 
	);

}

void AZCZombieCharacter::AttackFinished()
{
	bIsAttacking = false;
	bIsAnimationStarted = false;

	if (!bIsTargetInAttackRange || !IsValid(AttackTarget)) {
		AttackTarget = nullptr;
		return;
	}

	GetWorldTimerManager().SetTimerForNextTick(
		this,
		&AZCZombieCharacter::RestartAttackIfStillInRange
	);
}

void AZCZombieCharacter::RestartAttackIfStillInRange()
{
	if (!bIsTargetInAttackRange || !IsValid(AttackTarget)) {
		AttackTarget = nullptr;
		return;
	}

	StartAttack(AttackTarget);
}
// Called when the game starts or when spawned
void AZCZombieCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
}

