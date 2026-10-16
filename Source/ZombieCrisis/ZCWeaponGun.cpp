// Fill out your copyright notice in the Description page of Project Settings.


#include "ZCWeaponGun.h"
#include "ZCWeaponTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

void AZCWeaponGun::WeaponApply()
{
	UWorld* World = GetWorld();
	if (!World) return;

	AActor* WeaponOwner = GetOwner();
	if (!IsValid(WeaponOwner)) return;
	AController* PC = WeaponOwner->GetInstigatorController();

	const float CurrentTime = World->GetTimeSeconds();

	if (CurrentTime < NextPistolFireTime) return;
	NextPistolFireTime = CurrentTime + ShotInterval;

	TArray<USceneComponent*> SceneComponents;
	WeaponOwner->GetComponents<USceneComponent>(SceneComponents, true);
	//获取玩家身上的组件，SceneComponents变量接收结果，true表示递归寻找子actor的组件
	USceneComponent* MuzzlePoint = nullptr;
	for (USceneComponent* Component : SceneComponents) {
		if (IsValid(Component) && Component->GetFName() == TEXT("MuzzlePoint")) {
			MuzzlePoint = Component;
			break;
		}
	}
	if (!IsValid(MuzzlePoint)) return;

	FVector AimDirection = WeaponOwner->GetActorForwardVector();
	AimDirection.Z = 0.0f;
	if (!AimDirection.Normalize()) return;
	//BP_PlayMuzzleFlash();
	FVector TraceStart = MuzzlePoint->GetComponentLocation();
	FVector TraceEnd = TraceStart + AimDirection * DamageRange;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);//忽略自己
	QueryParams.AddIgnoredActor(WeaponOwner);

	if (IsValid(MuzzleFlashSystem)) {
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			MuzzleFlashSystem,
			MuzzlePoint->GetComponentLocation(),
			MuzzlePoint->GetComponentRotation()
		);
	}

	float EachAngle = -StartDegreeOffset;//每次偏移多少度，初值为开始的偏移角度
	FVector LineDirection = AimDirection.RotateAngleAxis(EachAngle, FVector::UpVector);
	for (int32 Index = 0; Index < TraceCount; Index++) {
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

		if (IsValid(WeaponEffectSystem)) {
			UNiagaraComponent* Tracer = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World,
				WeaponEffectSystem,
				MuzzlePoint->GetComponentLocation(),
				MuzzlePoint->GetComponentRotation(),
				FVector(1.0, 1.0, 1.0f),
				true,
				false,
				ENCPoolMethod::AutoRelease,
				false
			);

			if (IsValid(Tracer)) {
				TraceEnd = TraceStart + LineDirection * DamageRange;
				FVector VisualEnd = bWeaponHit ? HitResult.ImpactPoint : TraceEnd;
				Tracer->SetVariableVec3(FName(TEXT("User.TracerEnd")), VisualEnd);
				Tracer->Activate(true);
				EachAngle += BulletDegrees;
				LineDirection = AimDirection.RotateAngleAxis(EachAngle, FVector::UpVector);
			}
			if (bWeaponHit && HitResult.GetActor()) {

				UGameplayStatics::ApplyPointDamage(
					HitResult.GetActor(),
					WeaponDamage,
					AimDirection,
					HitResult,
					PC,
					this,
					UDamageType::StaticClass()
				);
			}
		}
	}
	/*
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
	*/
	
}

void AZCWeaponGun::UpdateProperty(const FZCWeaponDefinitionRow& Value)
{
	WeaponDamage = Value.BaseDamage;
	ShotInterval = FMath::Max(0.02f, 1.0 / Value.BaseFireRate);
	WeaponCapacity = Value.BaseMagazineSize;
	DamageRange = Value.BaseRange;
	BulletDegrees = Value.BaseSpreadDegrees;
	NextPistolFireTime = 0.0f;
	MuzzleFlashSystem = Value.MuzzleFlashSystem;
	WeaponEffectSystem = Value.WeaponEffectSystem;
	TraceCount = Value.BaseTraceCount;
	StartDegreeOffset = Value.StartDegreeOffset;
}
