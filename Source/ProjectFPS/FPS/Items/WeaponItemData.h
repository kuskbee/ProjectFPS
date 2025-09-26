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

public:
	/** 무기 클래스가 유효한지 확인 */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsValidWeapon() const;

	/** 무기 DPS 계산 */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	float CalculateDPS() const { return BaseDamage * FireRate; }
};