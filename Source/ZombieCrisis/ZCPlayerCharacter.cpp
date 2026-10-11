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
		DeathAnimationComponent->PlayDeathAnimation();
	}
	else {
		GetWorldTimerManager().SetTimer(
			HitReactionTimerHandle,
			0.35f,
			false
		);
		//受击更新时间句柄，用于阻止输入，僵尸的后面加了攻击再加。
		BP_PlayHitReactionAnimation();
		FVector DamageCauserDirection = DamageCauser->GetActorForwardVector();
		KnockbackVelocityEachAxis = DamageCauserDirection * KnockbackVelocity;
		LaunchCharacter(KnockbackVelocityEachAxis, true, true);
	}
	return ActualDamage;

}

bool AZCPlayerCharacter::IsDead() {
	return CurrentHealth <= 0;
}

void AZCPlayerCharacter::ChangeWeaponProperty(const FZCWeaponDefinitionRow& Values)
{
	if (!IsValid(CurrentWeapon)) return;
	CurrentWeapon->UpdateProperty(Values);
}

void AZCPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->RotationRate = FRotator(0.0f, RotateRateY, 0.0f);
	ApplyNewWeapon();
}

void AZCPlayerCharacter::Move(const FInputActionValue& Value)
{
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
	CurrentWeapon->WeaponApply();
	UE_LOG(LogTemp, Warning, TEXT("开始射击"));
	
}

void AZCPlayerCharacter::ChangeWeapons(const FInputActionValue& Value)
{
	if (GetWorldTimerManager().IsTimerActive(HitReactionTimerHandle)) return;

	if (bIsDead) return;
	
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
	OnWeaponChanged.Broadcast(FText::FromName(Weapons[CurrentWeaponIndex]));
	ApplyNewWeapon();
}

void AZCPlayerCharacter::ApplyNewWeapon()
{
	if (!IsValid(WeaponDefinitionTable) || !Weapons.IsValidIndex(CurrentWeaponIndex)) {
		UE_LOG(LogTemp, Warning, TEXT("Update Fail"));
		return;
	}

	FName CurrentWeaponName = Weapons[CurrentWeaponIndex];
	const FZCWeaponDefinitionRow* Row = WeaponDefinitionTable->FindRow<FZCWeaponDefinitionRow>(
		CurrentWeaponName, TEXT("ApplyCurrentWeaponVisual")
	);

	if (!Row || !Row->VisualClass) {
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("武器行%s查找配置错误"),
			*(CurrentWeaponName.ToString())
		);
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	CurrentWeapon = GetWorld()->SpawnActor<AZCWeaponBasic>(
		Row->WeaponClass,
		Row->VisualRelativeTransform,
		SpawnParams
	);
	ChangeWeaponProperty(*Row);
	BP_ApplyNewWeaponVisual(Row->VisualClass, Row->VisualRelativeTransform);
}
