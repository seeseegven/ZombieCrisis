// Fill out your copyright notice in the Description page of Project Settings.


#include "ZCMenuPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ZCGameMode.h"
#include "Blueprint/WidgetBlueprintLibrary.h"


void AZCMenuPlayerController::StartGame()
{
	UWidgetBlueprintLibrary::SetInputMode_GameOnly(this, true);
	UGameplayStatics::OpenLevel(
		this,
		FName(TEXT("/Game/ZombieCrisis/Maps/L_ZC_PVETest"))
	);
	
}

void AZCMenuPlayerController::ExitGame()
{
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}

void AZCMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsValid(MainMenuWidgetClass)) return;

	MainMenuWidget = CreateWidget<UUserWidget>(
		this,
		MainMenuWidgetClass
	);

	if (!IsValid(MainMenuWidget)) return;

	MainMenuWidget->AddToViewport(0);
	bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	SetInputMode(InputMode);
}
