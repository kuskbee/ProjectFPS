// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "WeaponHUD.generated.h"

/**
 * 무기 HUD - 탄약 및 무기 정보 표시
 * C++에서 로직 구현, Blueprint에서 비주얼 디자인
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTFPS_API UWeaponHUD : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	// Blueprint에서 바인딩할 UI 컴포넌트들
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

public:
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

protected:
	/** 텍스트 색상 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Style")
	FLinearColor DefaultTextColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Style")
	FLinearColor ActiveSlotColor = FLinearColor::Yellow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Style")
	FLinearColor InactiveSlotColor = FLinearColor::Gray;
};