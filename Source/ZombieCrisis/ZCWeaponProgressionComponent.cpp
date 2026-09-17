// Fill out your copyright notice in the Description page of Project Settings.


#include "ZCWeaponProgressionComponent.h"

// Sets default values for this component's properties
UZCWeaponProgressionComponent::UZCWeaponProgressionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UZCWeaponProgressionComponent::InitializeProgression()
{
	PlayerLevel = FMath::Max(1, StartingLevel);
}

void UZCWeaponProgressionComponent::SetPlayerLevel(int32 NewLevel)
{
	PlayerLevel = FMath::Max(1, NewLevel);
}


// Called when the game starts
void UZCWeaponProgressionComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeProgression();
	// ...
	
}

