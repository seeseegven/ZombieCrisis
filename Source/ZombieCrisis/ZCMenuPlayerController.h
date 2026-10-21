// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ZCMenuPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ZOMBIECRISIS_API AZCMenuPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void StartGame();

	UFUNCTION(BlueprintCallable)
	void ExitGame();
protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> MainMenuWidget = nullptr;
};
