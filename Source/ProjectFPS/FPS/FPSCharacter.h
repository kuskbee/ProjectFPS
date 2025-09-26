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

	// Health 속성 변경 시 호출
	virtual void OnHealthChanged(const FOnAttributeChangeData& Data);

	UFUNCTION(Server, Reliable)
	void ServerNotifyPlayerDeath();
	void ServerNotifyPlayerDeath_Implementation();

	// 플레이어 사망 처리
	virtual void OnPlayerDeath();

public:
	// 플레이어 리스폰 처리
	UFUNCTION(BlueprintCallable, Category="Player")
	virtual void OnPlayerRespawn();
	// 매 프레임마다 호출
	virtual void Tick(float DeltaTime) override;

	// 입력과 기능을 바인딩하기 위해 호출
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 입력 액션 핸들러 함수들
	void FireAbilityPressed(const FInputActionValue& Value);
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void SwitchWeaponSlot(const FInputActionValue& Value);
	void SwitchToPrimaryWeapon(const FInputActionValue& Value);
	void SwitchToSecondaryWeapon(const FInputActionValue& Value);

	// IFPSWeaponHolder 인터페이스 구현
	virtual void AttachWeaponMeshes(AFPSWeapon* Weapon) override;
	virtual void PlayFiringMontage(UAnimMontage* Montage) override;
	virtual void AddWeaponRecoil(float Recoil) override;
	virtual void UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize) override;
	virtual FVector GetWeaponTargetLocation() override;
	virtual void AddWeaponClass(const TSubclassOf<AFPSWeapon>& WeaponClass) override;
	virtual void OnWeaponActivated(AFPSWeapon* Weapon) override;
	virtual void OnWeaponDeactivated(AFPSWeapon* Weapon) override;
	virtual void OnSemiWeaponRefire() override;

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

	// 플레이어 생존 상태 (Blueprint에서 발사체 충돌 체크용)
	UPROPERTY(BlueprintReadOnly, Category="Player State")
	bool bIsAlive = true;

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

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> FireAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayEffect> HealEffect;

	// 무기 슬롯 관리 함수들
	UFUNCTION(BlueprintCallable, Category="Weapons")
	class UWeaponSlotComponent* GetWeaponSlotComponent() const { return WeaponSlotComponent; }

	/** 테스트용: 기본 무기 자동 지급 */
	UFUNCTION(BlueprintCallable, Category="Weapons")
	virtual void GiveDefaultWeapon();

	/** 기본으로 지급할 무기 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapons")
	TSubclassOf<AFPSWeapon> DefaultWeaponClass;

	// 입력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> MouseLookMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SwitchWeaponAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> PrimaryWeaponAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SecondaryWeaponAction;
};