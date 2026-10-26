// Fill out your copyright notice in the Description page of Project Settings.


#include "ZCPlayerCharacter.h"

#include "ZCDeathAnimationComponent.h"

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
#include "ZCWeaponTypes.h"
#include "Engine/DataTable.h"
#include "ZCWeaponBasic.h"
#include "TimerManager.h"
#include "ZCManageKnockback.h"
#include "ZCGameMode.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "ZCPlayerProgressionComponent.h"

// Sets default values
AZCPlayerCharacter::AZCPlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	//上面的可以不写，不影响，下面的要写，会影响绕z轴的旋转
	//意思是是否使用controller的旋转影响角色的旋转
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	GetCharacterMovement()->MaxWalkSpeed = 300.0f;
	DeathAnimationComponent = CreateDefaultSubobject<UZCDeathAnimationComponent>(
		TEXT("DeathAnimationComponent")
	);//创建死亡组件
	MaxHealth = 100.f;
	CurrentHealth = MaxHealth;
}


void AZCPlayerCharacter::AddNewWeapon(const FName& NewWeapon)
{
	Weapons.Add(NewWeapon);
	ApplyNewWeapon();
}

/**
 * 收到 GameMode 通知后，从成长组件重新读取该武器的最终配置。
 * WeaponName：本次强化的武器行名；bRefillAmmo：true 补到新数量，false 不改变剩余子弹。
 * 返回值：无。非当前武器只更新库存，等装备时再读取属性；当前武器直接刷新已有实例。
 */
void AZCPlayerCharacter::RefreshWeaponProperties(const FName& WeaponName, bool bRefillAmmo)
{
	AZCGameMode* GameMode = Cast<AZCGameMode>(UGameplayStatics::GetGameMode(this));
	if (!IsValid(GameMode)) return;
	const UZCPlayerProgressionComponent* Progression = GameMode->GetPlayerProgressionComponent();
	if (!IsValid(Progression)) return;
	const FZCWeaponComputedStats* Computed = Progression->GetWeaponStates().Find(WeaponName);
	if (!Computed) return;

	if (bRefillAmmo && Computed->bUseAmmo)
	{
		// 已装备过的武器覆盖为新配置数量；尚未装备的武器仍在首次装备时建立记录。
		if (TPair<bool, int32>* Ammo = WeaponCurrentAmmo.Find(WeaponName))
		{
			Ammo->Value = Computed->MagazineSize;
		}
	}

	if (!Weapons.IsValidIndex(CurrentWeaponIndex) || Weapons[CurrentWeaponIndex] != WeaponName) return;
	ChangeWeaponProperty(*Computed);
	if (const TPair<bool, int32>* Ammo = WeaponCurrentAmmo.Find(WeaponName))
	{
		// 在库存更新后广播，保证当前武器的弹药界面显示最新值。
		OnWeaponChanged.Broadcast(FText::FromName(WeaponName), Ammo->Key, Ammo->Value);
	}
}

/**
 * 应用 GameMode 转交的生命奖励。
 * NewMaxHealth：奖励表 Value 指定的新上限；返回值：无，更新生命和血条事件。
 * 例如原来 40/100、新上限 150，结果为 40 + (150 - 100) * 0.6 = 70/150。
 */
void AZCPlayerCharacter::ApplyMaxHealthUpgrade(float NewMaxHealth)
{
	if (bIsDead || NewMaxHealth <= 0.0f) return;
	const float OldMaxHealth = MaxHealth;
	MaxHealth = NewMaxHealth;
	// UE Clamp 将回血后的结果限制在 0 到新上限；生命继续使用 float，不丢弃小数。
	CurrentHealth = FMath::Clamp(CurrentHealth + (MaxHealth - OldMaxHealth) * 0.6f, 0.0f, MaxHealth);
	BloodPercent = CurrentHealth / MaxHealth;
	OnHealthChanged.Broadcast(BloodPercent);
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
	//获取本地玩家对象
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	//获得增强输入系统
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
	//将输入资产(IA)绑定到角色的xx函数，当trigger event时触发。
	if (EnhancedInput && FireAction) {
		EnhancedInput->BindAction(
			FireAction,
			ETriggerEvent::Triggered,
			this,
			&AZCPlayerCharacter::FireWeapon
		);
	}

	if (IsValid(EnhancedInput) && IsValid(ChangeWeaponAction)) {
		EnhancedInput->BindAction(
			ChangeWeaponAction,
			ETriggerEvent::Started,
			this,
			&AZCPlayerCharacter::ChangeWeapons
		);
	}

	if (IsValid(EnhancedInput) && IsValid(PauseGameAction)) {
		EnhancedInput->BindAction(
			PauseGameAction,
			ETriggerEvent::Started,
			this,
			&AZCPlayerCharacter::TogglePause
		);
	}
}

void AZCPlayerCharacter::ClosePauseWidget()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!IsValid(PC)) return;

	if (IsValid(PauseMenuWidget)) {
		PauseMenuWidget->RemoveFromParent();
	}
	UGameplayStatics::SetGamePaused(this, false);
	PC->bShowMouseCursor = false;
	UWidgetBlueprintLibrary::SetInputMode_GameOnly(PC, false);
}

float AZCPlayerCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
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
	BloodPercent = CurrentHealth / MaxHealth;
	OnHealthChanged.Broadcast(BloodPercent);

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
		UE_LOG(
			LogTemp,
			Display,
			TEXT("%s died. KillerController: %s"),
			*GetNameSafe(this),
			*GetNameSafe(EventInstigator)
		);
		GetWorldTimerManager().ClearTimer(HitReactionTimerHandle);
		if (AZCGameMode* GameMode = Cast<AZCGameMode>(
			UGameplayStatics::GetGameMode(this)
		)) {
			GameMode->HandlePlayerDied();
		}
		DeathAnimationComponent->PlayDeathAnimation();
	}
	else {
		if (UZCManageKnockback* Knockback = FindComponentByClass<UZCManageKnockback>()) {
			GetWorldTimerManager().SetTimer(
				HitReactionTimerHandle,
				0.35f,
				false
			);
			BP_PlayHitReactionAnimation();
			Knockback->StartKnockback(
				DamageCauser->GetActorForwardVector(),
				KnockbackVelocity,
				0.15f
			);
		}
	}
	return ActualDamage;

}

bool AZCPlayerCharacter::IsDead() {
	return CurrentHealth <= 0;
}

void AZCPlayerCharacter::ChangeWeaponProperty(const FZCWeaponComputedStats& Values)
{
	if (!IsValid(CurrentWeapon)) return;
	CurrentWeapon->UpdateProperty(Values);
}

void AZCPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->RotationRate = FRotator(0.0f, RotateRateY, 0.0f);
	Weapons.Reset();
}

void AZCPlayerCharacter::Move(const FInputActionValue& Value)
{
	if (GetWorldTimerManager().IsTimerActive(HitReactionTimerHandle) || bIsDead) return;

	const FVector2D Input = Value.Get<FVector2D>().GetClampedToMaxSize(1.0f);
	//获取最大长度不超过1的向量
	AddMovementInput(FVector(1.0f, 0.0f, 0.0f), Input.X);//朝x轴方向输入x轴分量的强度
	AddMovementInput(FVector(0.0f, -1.0f, 0.0f), Input.Y);
	//合成强度不超过1
}

void AZCPlayerCharacter::FireWeapon()
{
	if (GetWorldTimerManager().IsTimerActive(HitReactionTimerHandle) || bIsDead) return;

	if (!IsValid(CurrentWeapon)) {
		UE_LOG(LogTemp, Warning, TEXT("Error, Nullptr"));
		return;
	}
	FName CurrentWeaponName = Weapons[CurrentWeaponIndex];
	if (!WeaponCurrentAmmo.Find(CurrentWeaponName)) {
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Weapon not found!")
		);
		return;
	}
	if (WeaponCurrentAmmo[CurrentWeaponName].Key) {
		if (WeaponCurrentAmmo[CurrentWeaponName].Value <= 0) return;
		if (CurrentWeapon->WeaponApply()) {
			WeaponCurrentAmmo[CurrentWeaponName].Value--;
			OnWeaponChanged.Broadcast(
				FText::FromName(Weapons[CurrentWeaponIndex]),
				WeaponCurrentAmmo[CurrentWeaponName].Key,
				WeaponCurrentAmmo[CurrentWeaponName].Value
			);
		}
	}
	else {
		CurrentWeapon->WeaponApply();
	}
	/*
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Current Ammo : %d"),
		WeaponCurrentAmmo[CurrentWeaponName].Value
	);
	//UE_LOG(LogTemp, Warning, TEXT("开始射击"));
	*/
}

void AZCPlayerCharacter::ChangeWeapons(const FInputActionValue& Value)
{
	if (GetWorldTimerManager().IsTimerActive(HitReactionTimerHandle)) return;

	if (bIsDead || Weapons.IsEmpty()) return;
	
	const float Axis = Value.Get<float>();

	int32 Direction = Axis > 0 ? 1 : -1;

	CurrentWeaponIndex = (CurrentWeaponIndex + Direction + Weapons.Num()) 
										% Weapons.Num();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("切换方向是%d，当前武器为%d"),
		Direction,
		CurrentWeaponIndex+1
	);
	ApplyNewWeapon();
}

void AZCPlayerCharacter::TogglePause()
{
	if (bIsDead) return;//没考虑到，死亡后只能产生一个ui

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!IsValid(PC)) return;

	if (UGameplayStatics::IsGamePaused(this)) {
		//if (!UGameplayStatics::SetGamePaused(this, false)) {
		//	return;
		//}

		if (IsValid(PauseMenuWidget)) {
			PauseMenuWidget->RemoveFromParent();
		}
		UGameplayStatics::SetGamePaused(this, false);
		PC->bShowMouseCursor = false;
		UWidgetBlueprintLibrary::SetInputMode_GameOnly(PC, false);
		return;
	}

	if (!IsValid(PauseMenuWidgetClass)) return;
	//if (!IsValid(PauseMenuWidget)) {
		PauseMenuWidget = CreateWidget<UUserWidget>(
			PC,
			PauseMenuWidgetClass
		);
	//}//复用
	if (!IsValid(PauseMenuWidget)) return;
	UGameplayStatics::SetGamePaused(this, true);
	PauseMenuWidget->AddToViewport(0);
	UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(
		PC,
		PauseMenuWidget,
		EMouseLockMode::DoNotLock,
		false,
		false
	);
	PC->bShowMouseCursor = true;
}

void AZCPlayerCharacter::ApplyNewWeapon()
{
	if (!Weapons.IsValidIndex(CurrentWeaponIndex)) {
		UE_LOG(LogTemp, Warning, TEXT("Update Fail"));
		return;
	}

	FName CurrentWeaponName = Weapons[CurrentWeaponIndex];
	AZCGameMode* GameMode = Cast<AZCGameMode>(
		UGameplayStatics::GetGameMode(this)
	);
	checkf(IsValid(GameMode), TEXT("Invalid Game Mode!"));


	const FZCWeaponComputedStats* Row = (GameMode->GetPlayerProgressionComponent()
		->GetWeaponStates()).Find(CurrentWeaponName);

	if (!Row || !Row->VisualClass) {
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("武器行%s查找配置错误"),
			*(CurrentWeaponName.ToString())
		);
		return;
	}
	
	if (!WeaponCurrentAmmo.Contains(CurrentWeaponName)) {
		WeaponCurrentAmmo.Add(CurrentWeaponName, 
			TPair<bool, int32>(Row->bUseAmmo, Row->MagazineSize));
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	AZCWeaponBasic* NewWeapon = GetWorld()->SpawnActor<AZCWeaponBasic>(
		Row->WeaponClass,
		Row->VisualRelativeTransform,
		SpawnParams
	);
	if (!IsValid(NewWeapon)) return;
	if (IsValid(CurrentWeapon)) {
		CurrentWeapon->Destroy();
	}
	CurrentWeapon = NewWeapon;
	ChangeWeaponProperty(*Row);
	BP_ApplyNewWeaponVisual(Row->VisualClass, Row->VisualRelativeTransform);
	OnWeaponChanged.Broadcast(
		FText::FromName(Weapons[CurrentWeaponIndex]),
		WeaponCurrentAmmo[CurrentWeaponName].Key,
		WeaponCurrentAmmo[CurrentWeaponName].Value
	);
}
