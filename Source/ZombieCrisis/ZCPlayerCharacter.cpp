// Fill out your copyright notice in the Description page of Project Settings.


#include "ZCPlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"//中间管理者，硬件和character之间的桥梁
#include "InputActionValue.h"//传入的输入值

// Sets default values
AZCPlayerCharacter::AZCPlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;

	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	GetCharacterMovement()->MaxWalkSpeed = 300.0f;
}


// Called to bind functionality to input
void AZCPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//super代表父类，等价与ACharacter::Set...

	APlayerController* PC = Cast<APlayerController>(GetController());
	//GetController()返回的是AController，所以要cast
	//主要是判断是player还是AIController
	if (!PC) return;

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer) return;

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

	if (InputSubsystem && PlayerMappingContext) {
		InputSubsystem->AddMappingContext(PlayerMappingContext, 0);
	}

	UEnhancedInputComponent* EnhancedInput =
		Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (EnhancedInput && MoveAction) {
		EnhancedInput->BindAction(
			MoveAction, //响应moveaction
			ETriggerEvent::Triggered,//持续按着
			this,
			&AZCPlayerCharacter::Move);
	}
}

void AZCPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>().GetClampedToMaxSize(1.0f);

	AddMovementInput(FVector(1.0f, 0.0f, 0.0f), Input.X);
	AddMovementInput(FVector(0.0f, -1.0f, 0.0f), Input.Y);
}

