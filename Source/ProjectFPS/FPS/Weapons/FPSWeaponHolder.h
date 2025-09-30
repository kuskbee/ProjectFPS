// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FPSWeaponHolder.generated.h"

class AFPSWeapon;
class UAnimMontage;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UFPSWeaponHolder : public UInterface
{
	GENERATED_BODY()
};

/**
 * FPS 무기 홀더 클래스를 위한 공통 인터페이스
 * ShooterWeaponHolder와 유사하지만 GAS 통합을 위해 FPS 프로젝트에 맞게 조정됨
 */
class PROJECTFPS_API IFPSWeaponHolder
{
	GENERATED_BODY()

public:

	/** 무기의 메시들을 소유자에게 부착 */
	virtual void AttachWeaponMeshes(AFPSWeapon* Weapon) = 0;

	/** 무기 발사 몽타주 재생 */
	virtual void PlayFiringMontage(UAnimMontage* Montage) = 0;

	/** 무기 리로드 몽타주 재생 */
	virtual void PlayReloadMontage(UAnimMontage* Montage) = 0;

	/** 소유자에게 무기 반동 적용 */
	virtual void AddWeaponRecoil(float Recoil) = 0;

	/** 현재 탄약 수로 무기 HUD 업데이트 */
	virtual void UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize) = 0;

	/** 무기의 조준 위치 계산 및 반환 */
	virtual FVector GetWeaponTargetLocation() = 0;

	/** 이 클래스의 무기를 소유자에게 지급 */
	virtual void AddWeaponClass(const TSubclassOf<AFPSWeapon>& WeaponClass) = 0;

	/** 전달된 무기 활성화 */
	virtual void OnWeaponActivated(AFPSWeapon* Weapon) = 0;

	/** 전달된 무기 비활성화 */
	virtual void OnWeaponDeactivated(AFPSWeapon* Weapon) = 0;

	/** 반자동 무기의 쿨다운이 만료되어 다시 발사할 준비가 되었음을 소유자에게 알림 */
	virtual void OnSemiWeaponRefire() = 0;
};