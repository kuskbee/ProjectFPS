// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPS/Items/BaseItemData.h"
#include "WeaponItemData.generated.h"

class AFPSWeapon;

/**
 * 무기 아이템 데이터 클래스
 * FPSWeapon 스폰 시 초기화에 사용되는 마스터 데이터
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTFPS_API UWeaponItemData : public UBaseItemData
{
	GENERATED_BODY()

public:
	UWeaponItemData();

	/** 실제 스폰할 무기 액터 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AFPSWeapon> WeaponClass;

	// ========================================
	// 무기 메시 (BaseItemData::WorldSkeletalMesh = ThirdPersonMesh)
	// ========================================

	/** 1인칭 시점 무기 메시 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Mesh")
	TObjectPtr<USkeletalMesh> FirstPersonMesh;

	// 3인칭 메시는 BaseItemData::WorldSkeletalMesh 사용

	/** 무기 데미지 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats", meta = (ClampMin = 0))
	int32 BaseDamage = 10;

	/** 발사 속도 (초당 발사 횟수) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats", meta = (ClampMin = 0.1, ClampMax = 10.0))
	float FireRate = 1.0f;

	/** 탄창 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats", meta = (ClampMin = 1, ClampMax = 100))
	int32 MagazineSize = 30;

	/** 무기 정확도 (조준 시 분산각, 도 단위) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats", meta = (ClampMin = 0.0, ClampMax = 90.0))
	float AccuracySpread = 2.0f;

	/** 무기 반동 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats", meta = (ClampMin = 0.0, ClampMax = 10.0))
	float RecoilStrength = 1.0f;

	/** 무기 사거리 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats", meta = (ClampMin = 100, ClampMax = 10000))
	float WeaponRange = 5000.0f;

	/** 자동 사격 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	bool bIsAutomatic = false;

	// ========================================
	// 크로스헤어 설정
	// ========================================

	/** 크로스헤어 기본 확산 (무기별로 다름) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshair", meta = (ClampMin = 5.0, ClampMax = 50.0))
	float CrosshairBaseSpread = 5.0f;

	/** 발사 시 크로스헤어 확산 증가량 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crosshair", meta = (ClampMin = 0.0, ClampMax = 20.0))
	float CrosshairRecoilSpread = 15.0f;

	// ========================================
	// 런타임 상태 데이터 (게임 중 변경되는 값들)
	// ========================================

	/** 현재 탄약 수 */
	UPROPERTY(BlueprintReadWrite, Category = "Runtime State")
	int32 CurrentAmmo = 0;

	/** 무기 내구도 (0.0 ~ 100.0) */
	UPROPERTY(BlueprintReadWrite, Category = "Runtime State", meta = (ClampMin = 0.0, ClampMax = 100.0))
	float Durability = 100.0f;

public:
#if WITH_EDITOR
	/** 에디터에서 프로퍼티 변경 시 호출 (MagazineSize 변경 시 CurrentAmmo 동기화) */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	/** 무기 클래스가 유효한지 확인 */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsValidWeapon() const;

	/** 무기 DPS 계산 */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	float CalculateDPS() const { return BaseDamage * FireRate; }

	/** 현재 탄약을 최대로 채우기 */
	UFUNCTION(BlueprintCallable, Category = "Runtime State")
	void RefillAmmo() { CurrentAmmo = MagazineSize; }

	/** 탄약 소모 (성공 시 true 반환) */
	UFUNCTION(BlueprintCallable, Category = "Runtime State")
	bool ConsumeAmmo(int32 AmmoToConsume = 1);

	/** 탄약이 비어있는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Runtime State")
	bool IsAmmoEmpty() const { return CurrentAmmo <= 0; }

	/** 탄약이 가득 찬지 확인 */
	UFUNCTION(BlueprintPure, Category = "Runtime State")
	bool IsAmmoFull() const { return CurrentAmmo >= MagazineSize; }

	/** 발사 간격 계산 (FireRate -> RefireRate 변환) */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetRefireRate() const { return FireRate > 0.0f ? 1.0f / FireRate : 1.0f; }

	/** 1인칭 메시 반환 */
	UFUNCTION(BlueprintPure, Category = "Weapon Mesh")
	USkeletalMesh* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** 3인칭 메시 반환 (BaseItemData::WorldSkeletalMesh) */
	UFUNCTION(BlueprintPure, Category = "Weapon Mesh")
	USkeletalMesh* GetThirdPersonMesh() const { return WorldSkeletalMesh; }
};