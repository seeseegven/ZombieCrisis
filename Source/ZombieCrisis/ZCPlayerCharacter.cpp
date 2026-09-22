// Fill out your copyright notice in the Description page of Project Settings.


#include "ZCPlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"//中间管理者，硬件和character之间的桥梁
#include "InputActionValue.h"//传入的输入值
#include "DrawDebugHelpers.h"//画调试射线
#include "Engine/World.h"//获取游戏时间和执行射线检测
#include "GameFramework/DamageType.h"//ue基础伤害类型
#include "Kismet/GameplayStatics.h"//applyPointDamage
#include "Logging/LogMacros.h"

// Sets default values
AZCPlayerCharacter::AZCPlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;

	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	GetCharacterMovement()->MaxWalkSpeed = 300.0f;
	MaxHealth = 100.f;
	CurrentHealth = MaxHealth;
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

	if (EnhancedInput && FireAction) {
		EnhancedInput->BindAction(
			FireAction,
			ETriggerEvent::Triggered,
			this,
			&AZCPlayerCharacter::FirePistol
		);
	}
}

float AZCPlayerCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
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
	BloodPercent = CurrentHealth / MaxHealth;
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

bool AZCPlayerCharacter::IsDead() {
	return CurrentHealth <= 0;
}

void AZCPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>().GetClampedToMaxSize(1.0f);

	AddMovementInput(FVector(1.0f, 0.0f, 0.0f), Input.X);
	AddMovementInput(FVector(0.0f, -1.0f, 0.0f), Input.Y);
}

void AZCPlayerCharacter::FirePistol()
{
	UE_LOG(LogTemp, Warning, TEXT("收到射击输入"));
	UWorld* World = GetWorld();

	if (!World) return;

	const float CurrentTime = World->GetTimeSeconds();

	if (CurrentTime < NextPistolFireTime) return;
	const float SafeShotPerSecond = FMath::Max(0.01f, PistolShotPerSecond);
	const float FireInterval = 1.f / SafeShotPerSecond;
	NextPistolFireTime = CurrentTime + FireInterval;
	FVector AimDirection = GetActorForwardVector();
	AimDirection.Z = 0.0f;
	if (!AimDirection.Normalize()) return;
	FVector TraceStart = GetActorLocation();
	FVector TraceEnd = TraceStart + AimDirection * PistolRange;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);//忽略自己

	FHitResult HitResult;
	const bool bWeaponHit = World->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	const FVector DebugEnd = bWeaponHit ? HitResult.ImpactPoint : TraceEnd;
	const FColor DebugColor = bWeaponHit ? FColor::Green : FColor::Red;
	DrawDebugLine(
		World,
		TraceStart,
		TraceEnd,
		DebugColor,
		false,
		1.0f,
		0,
		1.f
	);
	
	if (!bWeaponHit || !HitResult.GetActor()) return;
	UE_LOG(
		LogTemp,
		Display,
		TEXT("击中了: %s"),
		*GetNameSafe(HitResult.GetActor())
	);

	UGameplayStatics::ApplyPointDamage(
		HitResult.GetActor(),
		PistolDamage,
		AimDirection,
		HitResult,
		GetController(),
		this,
		UDamageType::StaticClass()
	);
}

