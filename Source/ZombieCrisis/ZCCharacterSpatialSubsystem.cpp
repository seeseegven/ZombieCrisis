// Fill out your copyright notice in the Description page of Project Settings.


#include "ZCCharacterSpatialSubsystem.h"

void UZCCharacterSpatialSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FBox2D RootBounds(
		FVector2D(-10000.f, -10000.f),
		FVector2D(10000.f, 10000.f)
	);

	CharacterQuadTree = MakeUnique<TQuadTree<TWeakObjectPtr<ACharacter>, 4>>(
		RootBounds,
		100.f
	);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Tree initialized. World: %s"),
		*GetNameSafe(GetWorld())
	);
}

void UZCCharacterSpatialSubsystem::Deinitialize()
{
	CharacterQuadTree.Reset();
	Super::Deinitialize();
}

bool UZCCharacterSpatialSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game
		|| WorldType == EWorldType::PIE;
}
