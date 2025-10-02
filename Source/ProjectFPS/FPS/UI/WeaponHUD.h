// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "WeaponHUD.generated.h"

/**
 * 플레이어 HUD - 체력, 스태미나, 탄약, 크로스헤어 표시
 * C++에서 로직 구현, Blueprint에서 비주얼 디자인
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTFPS_API UWeaponHUD : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// === 체력/스태미나 관련 ===
	/** 체력 프로그레스 바 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	/** 스태미나 프로그레스 바 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaBar;

	/** 체력 텍스트 (예: "100 / 100") */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HealthText;

	/** 스태미나 텍스트 (예: "100 / 100") */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StaminaText;

	// === 무기 관련 ===
	/** 탄약 수 표시 텍스트 (예: "30 / 30") */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AmmoText;

	/** 무기 이름 표시 텍스트 (예: "RIFLE") */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WeaponNameText;

	/** Primary 슬롯 상태 텍스트 (예: "[1] RIFLE") */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PrimarySlotText;

	/** Secondary 슬롯 상태 텍스트 (예: "[2] PISTOL") */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SecondarySlotText;

	// === 크로스헤어 관련 ===
	/** 크로스헤어 이미지 (중앙 점) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> CrosshairCenter;

	/** 크로스헤어 상단 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> CrosshairTop;

	/** 크로스헤어 하단 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> CrosshairBottom;

	/** 크로스헤어 좌측 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> CrosshairLeft;

	/** 크로스헤어 우측 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> CrosshairRight;

public:
	// === 체력/스태미나 업데이트 ===
	/** 체력 바 업데이트 */
	UFUNCTION(BlueprintCallable, Category = "Player HUD")
	void UpdateHealthBar(float CurrentHealth, float MaxHealth);

	/** 스태미나 바 업데이트 */
	UFUNCTION(BlueprintCallable, Category = "Player HUD")
	void UpdateStaminaBar(float CurrentStamina, float MaxStamina);

	// === 무기 정보 업데이트 ===
	/** 탄약 정보 업데이트 */
	UFUNCTION(BlueprintCallable, Category = "Weapon HUD")
	void UpdateAmmoDisplay(int32 CurrentAmmo, int32 MaxAmmo);

	/** 현재 무기 이름 업데이트 */
	UFUNCTION(BlueprintCallable, Category = "Weapon HUD")
	void UpdateWeaponName(const FString& WeaponName);

	/** 무기 슬롯 상태 업데이트 */
	UFUNCTION(BlueprintCallable, Category = "Weapon HUD")
	void UpdateWeaponSlots(const FString& PrimaryWeapon, const FString& SecondaryWeapon, int32 ActiveSlot);

	/** HUD 전체 갱신 (WeaponSlotComponent에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "Weapon HUD")
	void RefreshWeaponHUD();

	// === 크로스헤어 업데이트 ===
	/** 크로스헤어 확산 설정 (발사 반동, 이동 시) */
	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void SetCrosshairSpread(float Spread);

protected:
	// === UI 스타일 설정 ===
	/** 텍스트 색상 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Style")
	FLinearColor DefaultTextColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Style")
	FLinearColor ActiveSlotColor = FLinearColor::Yellow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Style")
	FLinearColor InactiveSlotColor = FLinearColor::Gray;

	// === 크로스헤어 설정 ===
	/** 크로스헤어 기본 간격 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair")
	float BaseCrosshairSpread = 10.0f;

	/** 현재 크로스헤어 확산 */
	UPROPERTY(BlueprintReadOnly, Category = "Crosshair")
	float CurrentCrosshairSpread = 10.0f;

	/** 크로스헤어 확산 보간 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair")
	float CrosshairInterpSpeed = 10.0f;

	/** 목표 크로스헤어 확산 */
	float TargetCrosshairSpread = 10.0f;
};