// Fill out your copyright notice in the Description page of Project Settings.


#include "ZCWeaponGun.h"
#include "ZCWeaponTypes.h"
#include "Kismet/GameplayStatics.h"

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
	FVector AimDirection = WeaponOwner->GetActorForwardVector();
	AimDirection.Z = 0.0f;
	if (!AimDirection.Normalize()) return;
	//BP_PlayMuzzleFlash();
	FVector TraceStart = WeaponOwner->GetActorLocation();
	FVector TraceEnd = TraceStart + AimDirection * DamageRange;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);//ºöÂÔ×Ô¼º
	QueryParams.AddIgnoredActor(WeaponOwner);

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
		TEXT("has hitten: %s"),
		*GetNameSafe(HitResult.GetActor())
	);

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

void AZCWeaponGun::UpdateProperty(const FZCWeaponDefinitionRow& Value)
{
	WeaponDamage = Value.BaseDamage;
	ShotInterval = FMath::Max(0.02f, 1.0 / Value.BaseFireRate);
	WeaponCapacity = Value.BaseMagazineSize;
	DamageRange = Value.BaseRange;
	BulletDegrees = Value.BaseSpreadDegrees;
	NextPistolFireTime = 0.0f;
}
