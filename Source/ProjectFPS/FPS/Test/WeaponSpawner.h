// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "WeaponSpawner.generated.h"

class UWeaponItemData;
class AFPSWeapon;

/**
 * 테스트용 무기 스포너
 * 월드에 배치하면 지정된 WeaponItemData로 무기를 스폰하여 픽업 가능하게 만듦
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTFPS_API AWeaponSpawner : public AActor
{
	GENERATED_BODY()

public:
	AWeaponSpawner();

protected:
	virtual void BeginPlay() override;

	/** 스포너 표시용 메시 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SpawnerMesh;

	/** 스폰할 무기 데이터 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Spawner")
	TSubclassOf<UWeaponItemData> WeaponToSpawn;

	/** 스폰 위치 오프셋 (스포너 위쪽으로) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Spawner")
	FVector SpawnOffset = FVector(0, 0, 100);

	/** 게임 시작 시 자동으로 스폰할지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Spawner")
	bool bAutoSpawnOnBeginPlay = true;

	/** 스폰된 무기 참조 */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon Spawner")
	TObjectPtr<AFPSWeapon> SpawnedWeapon;

public:
	/** 무기 스폰 */
	UFUNCTION(BlueprintCallable, Category = "Weapon Spawner")
	AFPSWeapon* SpawnWeapon();

	/** 무기 데이터 클래스 설정 */
	UFUNCTION(BlueprintCallable, Category = "Weapon Spawner")
	void SetWeaponData(TSubclassOf<UWeaponItemData> NewWeaponDataClass);

	/** 스폰된 무기 반환 */
	UFUNCTION(BlueprintPure, Category = "Weapon Spawner")
	AFPSWeapon* GetSpawnedWeapon() const { return SpawnedWeapon; }
};