// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GenericQuadTree.h"
#include "ZCCharacterSpatialSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class ZOMBIECRISIS_API UZCCharacterSpatialSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	virtual bool DoesSupportWorldType(
		const EWorldType::Type WorldType
	) const override;

private:
	TUniquePtr<TQuadTree<TWeakObjectPtr<ACharacter>, 4>> CharacterQuadTree;
};
