// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPS/FPSCharacter.h"
#include "FPS/Weapons/FPSWeaponHolder.h"
#include "FPSEnemyCharacter.generated.h"

class AFPSWeapon;
class UAnimMontage;
class UWeaponItemData;

/**
 * AI가 조종하는 적 캐릭터 - AFPSCharacter 상속
 * 부모 클래스에서 이미 IFPSWeaponHolder 인터페이스를 구현하고 있음
 */
UCLASS()
class PROJECTFPS_API AFPSEnemyCharacter : public AFPSCharacter
{
	GENERATED_BODY()

public:
	AFPSEnemyCharacter();

protected:
	virtual void BeginPlay() override;

	// 부모 클래스의 OnHealthChanged 오버라이드
	virtual void OnHealthChanged(const FOnAttributeChangeData& Data) override;

	// 부모 클래스의 OnPlayerDeath 오버라이드 (적 전용 사망 처리)
	virtual void OnPlayerDeath() override;

public:	
	virtual void Tick(float DeltaTime) override;

protected:
	// 적 상태
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsDead = false;

	// 사망 후 제거 시간
	UPROPERTY(EditAnywhere, Category = "Death")
	float DeathDestroyDelay = 5.0f;

	FTimerHandle DeathTimer;

	/** AI 전용: 기본 무기 데이터 (WeaponItemData 기반) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Weapon")
	TSubclassOf<UWeaponItemData> DefaultWeaponData;

public:
	// IFPSWeaponHolder 인터페이스 구현
	virtual void AttachWeaponMeshes(AFPSWeapon* Weapon) override;
	virtual void PlayFiringMontage(UAnimMontage* Montage) override;
	virtual void AddWeaponRecoil(float Recoil) override;
	virtual void UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize) override;
	virtual FVector GetWeaponTargetLocation() override;
	virtual void OnWeaponActivated(AFPSWeapon* Weapon) override;
	virtual void OnWeaponDeactivated(AFPSWeapon* Weapon) override;
	virtual void OnSemiWeaponRefire() override;

	// AI에서 사용할 함수들
	UFUNCTION(BlueprintPure, Category = "AI")
	bool IsDead() const { return bIsDead; }

	/** AI 전용: 기본 무기 자동 지급 (부모 클래스 오버라이드) */
	void GiveDefaultWeapon();

protected:
	// 사망 처리 (AI 전용)
	void Die();

	UFUNCTION()
	void OnDeathDestroy();
};
