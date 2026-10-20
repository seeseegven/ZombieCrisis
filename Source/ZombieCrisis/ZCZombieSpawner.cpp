// Fill out your copyright notice in the Description page of Project Settings.


#include "ZCZombieSpawner.h"
#include "Components/BoxComponent.h"
#include "ZCZombieCharacter.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"

// Sets default values
AZCZombieSpawner::AZCZombieSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SpawnPoint = CreateDefaultSubobject<USceneComponent>(
		TEXT("SpawnPoint")
	);
	SetRootComponent(SpawnPoint);
	ReleaseTrigger = CreateDefaultSubobject<UBoxComponent>(
		TEXT("ReleaseTrigger")
	);
	ReleaseTrigger->SetupAttachment(SpawnPoint);
	

	ReleaseTrigger->SetBoxExtent(
		FVector(60.f, 150.f, 160.f),
		false
	);

	ReleaseTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);//仅查询
	ReleaseTrigger->SetGenerateOverlapEvents(true);
	ReleaseTrigger->SetCanEverAffectNavigation(false);
}

void AZCZombieSpawner::StartSpawning(int32 NewCount)
{
	StopSpawning();

	Count = FMath::Max(0, NewCount);
	if (Count > 0) {
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Total zombie: %d"),
			Count
		);
		SpawnZombie();
		
	}
}

void AZCZombieSpawner::StopSpawning()
{
	Count = 0;

	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	WaitingZombie = nullptr;
}

// Called when the game starts or when spawned
void AZCZombieSpawner::BeginPlay()
{
	Super::BeginPlay();

	AZCZombieCharacter* ZombieDefaults = ZombieClass.GetDefaultObject();
	if (IsValid(ZombieDefaults)) {
		UCapsuleComponent* CapsuleComp = ZombieDefaults->GetCapsuleComponent();
		if (IsValid(CapsuleComp)) {
			float CapsuleDiameter = CapsuleComp->GetUnscaledCapsuleRadius() * 2.f;

			ReleaseTrigger->SetRelativeLocation(FVector(CapsuleDiameter, 0.f, 0.f));
		}
	}
	else {
		ReleaseTrigger->SetRelativeLocation(FVector(100.f, 0.f, 0.f));
	}

	ReleaseTrigger->OnComponentBeginOverlap.AddUniqueDynamic(
		this,
		&AZCZombieSpawner::HandleReleaseTriggerOverlap
	);

	//SpawnZombie();
}

void AZCZombieSpawner::SpawnZombie()
{
	if (Count <= 0 || IsValid(WaitingZombie)) return;

	UWorld* World = GetWorld();
	if (!IsValid(World) || !IsValid(ZombieClass) || !IsValid(SpawnPoint)) {
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Condition is not enough. Can't spawn.")
		);
		return;
	}

	FVector SpawnLocation = SpawnPoint->GetComponentLocation();
	FRotator SpawnRotation = SpawnPoint->GetComponentRotation();
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;

	float BornOffset = ReleaseTrigger->GetScaledBoxExtent().Y;
	SpawnLocation += SpawnPoint->GetRightVector() * FMath::FRandRange(-BornOffset, BornOffset);

	AZCZombieCharacter* SpawnedZombie = World->SpawnActor<AZCZombieCharacter>(
		ZombieClass.Get(),
		SpawnLocation,
		SpawnRotation,
		SpawnParameters
	);

	if (!IsValid(SpawnedZombie)) {
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Can't spawn")
		);
		GetWorldTimerManager().SetTimer(
			SpawnTimerHandle,
			this,
			&AZCZombieSpawner::SpawnZombie,
			0.2f,
			false
		);
		return;
	}

	WaitingZombie = SpawnedZombie;
	--Count;

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Spawned a zombie. Location: %f,%f,%f"),
		SpawnedZombie->GetActorLocation().X,
		SpawnedZombie->GetActorLocation().Y,
		SpawnedZombie->GetActorLocation().Z
	);
}

void AZCZombieSpawner::HandleReleaseTriggerOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComponent, 
	int32 OtherBodyIndex, bool bFromSweep, 
	const FHitResult& SweepResult)
{
	if (!IsValid(WaitingZombie)) return;

	if (OtherActor != WaitingZombie.Get()) return;

	if (OtherComponent != WaitingZombie->GetCapsuleComponent()) return;

	WaitingZombie = nullptr;
	if (Count > 0) {
		GetWorldTimerManager().SetTimer(
			SpawnTimerHandle,
			this,
			&AZCZombieSpawner::SpawnZombie,
			0.2f,
			false
		);
	}
}

void AZCZombieSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	if (IsValid(ReleaseTrigger)) {
		ReleaseTrigger->OnComponentBeginOverlap.RemoveDynamic(
			this,
			&AZCZombieSpawner::HandleReleaseTriggerOverlap
		);
	}
	WaitingZombie = nullptr;
	Super::EndPlay(EndPlayReason);
}


