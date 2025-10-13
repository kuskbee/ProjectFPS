// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "InputActionValue.h"
#include "GameplayEffectTypes.h" // Added for FOnAttributeChangeData
#include "FPS/Weapons/FPSWeaponHolder.h"
#include "FPSCharacter.generated.h"

class USkeletalMeshComponent;
class UAbilitySystemComponent;
class UCharacterAttributeSet;
class UGameplayAbility;
class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;
class AFPSWeapon;
class UAnimMontage;
class UGameplayEffect;
class UWeaponSlotComponent;

UCLASS()
class PROJECTFPS_API AFPSCharacter : public ACharacter, public IAbilitySystemInterface, public IFPSWeaponHolder
{
	GENERATED_BODY()

public:
	// 캐릭터 기본값 설정
	AFPSCharacter();
	
	// IAbilitySystemInterface 구현을 위한 필수 함수
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	// 게임 시작 시 또는 스폰될 때 호출
	virtual void BeginPlay() override;

	// Health 속성 변경 시 호출 (자식 클래스에서 override)
	virtual void OnHealthChanged(const FOnAttributeChangeData& Data) {}

	// Stamina 속성 변경 시 호출 (자식 클래스에서 override)
	virtual void OnStaminaChanged(const FOnAttributeChangeData& Data) {}

	// Shield 속성 변경 시 호출 (자식 클래스에서 override)
	virtual void OnShieldChanged(const FOnAttributeChangeData& Data) {}

	UFUNCTION(Server, Reliable)
	void ServerNotifyPlayerDeath();
	void ServerNotifyPlayerDeath_Implementation();

public:

	// 플레이어 사망 처리
	virtual void OnPlayerDeath();

	// 플레이어 리스폰 처리
	UFUNCTION(BlueprintCallable, Category="Player")
	virtual void OnPlayerRespawn();
	// 매 프레임마다 호출
	virtual void Tick(float DeltaTime) override;

	// IFPSWeaponHolder 인터페이스 구현
	virtual void AttachWeaponMeshes(AFPSWeapon* Weapon) override;
	virtual void PlayFiringMontage(UAnimMontage* Montage) override;
	virtual void PlayReloadMontage(UAnimMontage* Montage) override;
	virtual void AddWeaponRecoil(float Recoil) override;
	virtual void UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize) override;
	virtual FVector GetWeaponTargetLocation() override;
	virtual void AddWeaponClass(const TSubclassOf<AFPSWeapon>& WeaponClass) override;
	virtual void OnWeaponActivated(AFPSWeapon* Weapon) override {}
	virtual void OnWeaponDeactivated(AFPSWeapon* Weapon) override {}
	virtual void OnSemiWeaponRefire() override {}
	virtual void UpdateCrosshairFiringSpread(float Spread) override {}
	virtual void UpdateCrosshairMovementSpread(float Spread) override {}

	// 1인칭 메시 접근자
	UFUNCTION(BlueprintCallable, Category = "Character")
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UCharacterAttributeSet> AttributeSet;

	// 무기 슬롯 관리 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWeaponSlotComponent> WeaponSlotComponent;

	// 1인칭 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

	FTimerHandle RespawnTimerHandle;

public:
	// 플레이어/적 공통 생존 상태 플래그 (Blueprint에서도 접근 가능)
	UPROPERTY(BlueprintReadOnly, Category="Player State")
	bool bIsAlive = true;

protected:

	// 무기 소켓 이름들

	/** 1인칭 메시 무기 소켓 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapons")
	FName FirstPersonWeaponSocket = FName("HandGrip_R");

	/** 3인칭 메시 무기 소켓 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapons")
	FName ThirdPersonWeaponSocket = FName("HandGrip_R");

	/** 조준 트레이스에 사용할 최대 거리 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Aim", meta = (ClampMin = 0, ClampMax = 100000, Units = "cm"))
	float MaxAimDistance = 10000.0f;

	// 기본 애니메이션 인스턴스 클래스들 (무기 비활성화 시 복원용)
	TSubclassOf<UAnimInstance> DefaultFirstPersonAnimClass;
	TSubclassOf<UAnimInstance> DefaultThirdPersonAnimClass;

	/** 마지막으로 데미지를 준 공격자 (스킬 포인트 보상용) */
	UPROPERTY()
	TWeakObjectPtr<APawn> LastAttacker;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayEffect> HealEffect;

	/** 마지막 공격자 설정 (GameplayEffect에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetLastAttacker(APawn* Attacker) { LastAttacker = Attacker; }

	/** 마지막 공격자 가져오기 */
	UFUNCTION(BlueprintPure, Category = "Combat")
	APawn* GetLastAttacker() const { return LastAttacker.Get(); }

	// 무기 슬롯 관리 함수들
	UFUNCTION(BlueprintCallable, Category="Weapons")
	class UWeaponSlotComponent* GetWeaponSlotComponent() const { return WeaponSlotComponent; }

	/** 게임 시작 시 자동으로 부여할 어빌리티들 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;
};