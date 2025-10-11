// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponSpawner.h"
#include "FPS/Items/WeaponItemData.h"
#include "FPS/Weapons/FPSWeapon.h"
#include "FPS/Components/PickupTriggerComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

AWeaponSpawner::AWeaponSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	// 루트 컴포넌트 생성
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// 스포너 표시용 메시 생성
	SpawnerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpawnerMesh"));
	SpawnerMesh->SetupAttachment(RootComponent);
	SpawnerMesh->SetCollisionProfileName(TEXT("NoCollision"));

	// 기본값 설정
	SpawnOffset = FVector(0, 0, 100);
	bAutoSpawnOnBeginPlay = true;
}

void AWeaponSpawner::BeginPlay()
{
	Super::BeginPlay();

	// 자동 스폰이 활성화되어 있으면 무기 스폰
	if (bAutoSpawnOnBeginPlay)
	{
		SpawnWeapon();
	}
}

AFPSWeapon* AWeaponSpawner::SpawnWeapon()
{
	if (!WeaponToSpawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("WeaponSpawner: WeaponToSpawn이 설정되지 않았습니다! %s"), *GetName());
		return nullptr;
	}

	// 클래스 기본 객체에서 유효성 검사
	UWeaponItemData* DefaultWeaponData = WeaponToSpawn.GetDefaultObject();
	if (!DefaultWeaponData || !DefaultWeaponData->IsValidWeapon())
	{
		UE_LOG(LogTemp, Warning, TEXT("WeaponSpawner: 잘못된 WeaponItemData 클래스입니다!"));
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("WeaponSpawner: World가 null입니다"));
		return nullptr;
	}

	// 이미 스폰된 무기가 있으면 제거
	if (SpawnedWeapon && IsValid(SpawnedWeapon))
	{
		UE_LOG(LogTemp, Log, TEXT("WeaponSpawner: 기존 무기 제거 중..."));
		SpawnedWeapon->Destroy();
		SpawnedWeapon = nullptr;
	}

	// 스폰 위치 계산
	FVector SpawnLocation = GetActorLocation() + SpawnOffset;
	FRotator SpawnRotation = GetActorRotation();

	// 무기 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// WeaponClass 가져오기
	if (!DefaultWeaponData->WeaponClass)
	{
		UE_LOG(LogTemp, Error, TEXT("WeaponSpawner: WeaponClass가 설정되지 않았습니다"));
		return nullptr;
	}

	AFPSWeapon* NewWeapon = World->SpawnActor<AFPSWeapon>(
		DefaultWeaponData->WeaponClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (!NewWeapon)
	{
		UE_LOG(LogTemp, Error, TEXT("WeaponSpawner: 무기 스폰 실패"));
		return nullptr;
	}

	// WeaponItemData 인스턴스 생성 및 설정
	UWeaponItemData* WeaponDataInstance = NewObject<UWeaponItemData>(NewWeapon, WeaponToSpawn);
	NewWeapon->SetWeaponItemData(WeaponDataInstance);

	// 픽업 트리거 활성화 (무기가 월드에 드롭된 상태)
	if (UPickupTriggerComponent* PickupTrigger = NewWeapon->FindComponentByClass<UPickupTriggerComponent>())
	{
		PickupTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		UE_LOG(LogTemp, Log, TEXT("WeaponSpawner: 픽업 트리거 활성화"));
	}

	// 무기를 픽업 가능한 상태로 설정 (보이게 하고 콜리전 활성화)
	NewWeapon->SetActorHiddenInGame(false);
	NewWeapon->SetActorEnableCollision(true);

	// 무기를 드롭된 상태로 설정 (픽업 가능하게)
	NewWeapon->SetDropped(true);

	SpawnedWeapon = NewWeapon;

	UE_LOG(LogTemp, Log, TEXT("WeaponSpawner: 무기 스폰 완료 - %s at %s"),
		*WeaponDataInstance->GetItemName(), *SpawnLocation.ToString());

	// 성공 메시지 표시
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
			FString::Printf(TEXT("무기 스폰: %s"), *WeaponDataInstance->GetItemName()));
	}

	return NewWeapon;
}

void AWeaponSpawner::SetWeaponData(TSubclassOf<UWeaponItemData> NewWeaponDataClass)
{
	WeaponToSpawn = NewWeaponDataClass;

	if (WeaponToSpawn)
	{
		UWeaponItemData* DefaultData = WeaponToSpawn.GetDefaultObject();
		UE_LOG(LogTemp, Log, TEXT("WeaponSpawner: WeaponData 클래스 설정됨 - %s"),
			DefaultData ? *DefaultData->GetItemName() : TEXT("Unknown"));
	}
}