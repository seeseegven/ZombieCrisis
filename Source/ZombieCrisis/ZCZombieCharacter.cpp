// Fill out your copyright notice in the Description page of Project Settings.


#include "ZCZombieCharacter.h"

// Sets default values
AZCZombieCharacter::AZCZombieCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

bool AZCZombieCharacter::IsDead()
{
	return CurrentHealth <= 0;
}

float AZCZombieCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
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
		UE_LOG(
			LogTemp,
			Display,
			TEXT("%s died. KillerController: %s"),
			*GetNameSafe(this),
			*GetNameSafe(EventInstigator)
		);
		Destroy();
	}
	return ActualDamage;
}

// Called when the game starts or when spawned
void AZCZombieCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
}


