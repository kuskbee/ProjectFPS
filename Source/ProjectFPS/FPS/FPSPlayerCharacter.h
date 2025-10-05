// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPS/FPSCharacter.h"
#include "FPSPlayerCharacter.generated.h"

class UPlayerAttributeSet;
class UPlayerHUD;
class USkillComponent;

/**
 * 플레이어 전용 캐릭터 클래스
 * AFPSCharacter를 상속받아 플레이어만 사용하는 기능들 추가
 * (PlayerAttributeSet, HUD, 플레이어 입력 등)
 */
UCLASS()
class PROJECTFPS_API AFPSPlayerCharacter : public AFPSCharacter
{
	GENERATED_BODY()

public:
	AFPSPlayerCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Attribute 델리게이트 override
	virtual void OnHealthChanged(const FOnAttributeChangeData& Data) override;
	virtual void OnStaminaChanged(const FOnAttributeChangeData& Data) override;
	virtual void OnShieldChanged(const FOnAttributeChangeData& Data) override;
	virtual void OnSkillPointChanged(const FOnAttributeChangeData& Data);

protected:
	// Player 전용 AttributeSet
	UPROPERTY()
	TObjectPtr<UPlayerAttributeSet> PlayerAttributeSet;

	// 스킬 시스템
	/** 스킬 관리 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkillComponent> SkillComponent;

	// UI 관련
	/** PlayerHUD 위젯 클래스 (Blueprint에서 설정) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPlayerHUD> PlayerHUDClass;

	/** 현재 활성화된 PlayerHUD 인스턴스 */
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UPlayerHUD> PlayerHUDWidget;

public:
	// 플레이어 입력 액션들
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> MouseLookMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SwitchWeaponAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> PrimaryWeaponAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SecondaryWeaponAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> PickupAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ReloadAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;

	// 테스트용 입력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Debug")
	TObjectPtr<UInputAction> TestSkillAction;

	// 플레이어 입력 핸들러들
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void FireAbilityPressed(const FInputActionValue& Value);
	void SwitchWeaponSlot(const FInputActionValue& Value);
	void SwitchToPrimaryWeapon(const FInputActionValue& Value);
	void SwitchToSecondaryWeapon(const FInputActionValue& Value);
	void TryPickupItem(const FInputActionValue& Value);
	void ReloadPressed(const FInputActionValue& Value);
	void Sprint(const FInputActionValue& Value);

	// 테스트용 함수
	void TestAcquireSkill(const FInputActionValue& Value);

	// IFPSWeaponHolder 인터페이스 오버라이드 (HUD 업데이트용)
	virtual void UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize) override;
	virtual void UpdateCrosshairFiringSpread(float Spread) override;
	virtual void UpdateCrosshairMovementSpread(float Spread) override;
	virtual void OnWeaponActivated(AFPSWeapon* Weapon) override;
	virtual void OnWeaponDeactivated(AFPSWeapon* Weapon) override;
};
