// Fill out your copyright notice in the Description page of Project Settings.


#include "ZCZombieAIController.h"
#include "ZCZombieCharacter.h"
#include "ZCPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "TimerManager.h"
#include "ZCManageKnockback.h"

void AZCZombieAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AZCZombieCharacter* Zombie = Cast<AZCZombieCharacter>(InPawn);
	if (!IsValid(Zombie)) {
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Non-Zombie pawn.")
		);
		return;
	}

	UE_LOG(
		LogTemp,
		Display,
		TEXT("%s is controlling %s"),
		*GetNameSafe(this),
		*GetNameSafe(Zombie)
	);

	GetWorld()->GetTimerManager().SetTimer(
		ChaseTimerHandle,
		this,
		&AZCZombieAIController::UpdateChase,
		0.25f,
		true
	);
}

void AZCZombieAIController::OnUnPossess()
{
	GetWorldTimerManager().ClearTimer(ChaseTimerHandle);
	Super::OnUnPossess();
}

void AZCZombieAIController::UpdateChase()
{
	if (!IsValid(GetPawn())) return;

	AZCPlayerCharacter* Player = Cast<AZCPlayerCharacter>(
		UGameplayStatics::GetPlayerCharacter(this, 0)
	);
	if (!IsValid(Player)) return;

	if (GetMoveStatus() != EPathFollowingStatus::Idle) return;

	UZCManageKnockback* Knockback = GetPawn()->FindComponentByClass<UZCManageKnockback>();

	if (IsValid(Knockback) && Knockback->GetKnockbackStatus()) return;

	EPathFollowingRequestResult::Type Result = MoveToActor(
		Player,
		5.f,
		true,
		true,
		false,
		nullptr,
		false
	);

	if (Result == EPathFollowingRequestResult::RequestSuccessful) {
		UE_LOG(
			LogTemp,
			Display,
			TEXT("%s started chasing %s"),
			*GetNameSafe(GetPawn()),
			*GetNameSafe(Player)
		);
	}
}
