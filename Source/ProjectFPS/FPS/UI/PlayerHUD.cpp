// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/UI/PlayerHUD.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/Spacer.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Engine/Engine.h"

void UPlayerHUD::NativeConstruct()
{
    Super::NativeConstruct();

    // === 체력/스태미나 바 초기화 ===
    if (HealthBar)
    {
        HealthBar->SetPercent(1.0f);
    }

    if (StaminaBar)
    {
        StaminaBar->SetPercent(1.0f);
    }

    if (ShieldBar)
    {
        ShieldBar->SetPercent(1.0f);
    }

    if (HealthText)
    {
        HealthText->SetText(FText::FromString(TEXT("100 / 100")));
    }

    if (StaminaText)
    {
        StaminaText->SetText(FText::FromString(TEXT("100 / 100")));
    }

    // === 무기 정보 초기화 ===
    if (AmmoText)
    {
        AmmoText->SetText(FText::FromString(TEXT("-- / --")));
        AmmoText->SetColorAndOpacity(DefaultTextColor);
    }

    if (WeaponNameText)
    {
        WeaponNameText->SetText(FText::FromString(TEXT("NO WEAPON")));
        WeaponNameText->SetColorAndOpacity(DefaultTextColor);
    }

    if (PrimarySlotText)
    {
        PrimarySlotText->SetText(FText::FromString(TEXT("[1] EMPTY")));
        PrimarySlotText->SetColorAndOpacity(InactiveSlotColor);
    }

    if (SecondarySlotText)
    {
        SecondarySlotText->SetText(FText::FromString(TEXT("[2] EMPTY")));
        SecondarySlotText->SetColorAndOpacity(InactiveSlotColor);
    }

    // === 크로스헤어 초기화 ===
    CurrentCrosshairSpread = BaseCrosshairSpread;
    TargetCrosshairSpread = BaseCrosshairSpread;

    UE_LOG(LogTemp, Log, TEXT("PlayerHUD 초기화 완료 (체력/스태미나/무기/크로스헤어)"));
}

void UPlayerHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // 발사 확산은 시간에 따라 감소
    if (FiringSpreadAmount > 0.0f)
    {
        FiringSpreadAmount = FMath::FInterpTo(FiringSpreadAmount, 0.0f, InDeltaTime, FiringSpreadDecaySpeed);
    }

    // 최종 목표 확산 = 기본 확산 + 발사 확산 + 이동 확산
    TargetCrosshairSpread = BaseCrosshairSpread + FiringSpreadAmount + MovementSpreadAmount;

    // 크로스헤어 확산 부드럽게 보간
    if (FMath::Abs(CurrentCrosshairSpread - TargetCrosshairSpread) > 0.1f)
    {
        CurrentCrosshairSpread = FMath::FInterpTo(CurrentCrosshairSpread, TargetCrosshairSpread, InDeltaTime, CrosshairInterpSpeed);
    }

    // Spacer 크기 변경으로 크로스헤어 확산 표현
    if (SpacerTop && SpacerBottom && SpacerLeft && SpacerRight)
    {
        SpacerTop->SetSize(FVector2D(DefaultSpacerSize, CurrentCrosshairSpread));
        SpacerBottom->SetSize(FVector2D(DefaultSpacerSize, CurrentCrosshairSpread));
        SpacerLeft->SetSize(FVector2D(CurrentCrosshairSpread, DefaultSpacerSize));
        SpacerRight->SetSize(FVector2D(CurrentCrosshairSpread, DefaultSpacerSize));
    }
}

void UPlayerHUD::UpdateAmmoDisplay(int32 CurrentAmmo, int32 MaxAmmo)
{
    if (!AmmoText)
    {
        UE_LOG(LogTemp, Warning, TEXT("AmmoText가 null입니다!"));
        return;
    }

    FString AmmoString = "";

    // 탄약 텍스트 업데이트
    if (MaxAmmo < 0 || CurrentAmmo < 0)
    {
        AmmoString = FString::Printf(TEXT("-- / --"));
        AmmoText->SetText(FText::FromString(AmmoString));
    }
    else
    {
        AmmoString = FString::Printf(TEXT("%d / %d"), CurrentAmmo, MaxAmmo);
        AmmoText->SetText(FText::FromString(AmmoString));
    }
    
    // 탄약이 부족하면 색상 변경
    if (CurrentAmmo <= MaxAmmo * 0.3f) // 30% 이하
    {
        AmmoText->SetColorAndOpacity(FLinearColor::Red);
    }
    else if (CurrentAmmo <= MaxAmmo * 0.6f) // 60% 이하
    {
        AmmoText->SetColorAndOpacity(FLinearColor::Yellow);
    }
    else
    {
        AmmoText->SetColorAndOpacity(DefaultTextColor);
    }

    UE_LOG(LogTemp, VeryVerbose, TEXT("탄약 표시 업데이트: %s"), *AmmoString);
}

void UPlayerHUD::UpdateWeaponName(const FString &WeaponName)
{
    if (!WeaponNameText)
    {
        UE_LOG(LogTemp, Warning, TEXT("WeaponNameText가 null입니다!"));
        return;
    }

    if (WeaponName.IsEmpty())
    {
        WeaponNameText->SetText(FText::FromString(TEXT("NO WEAPON")));
    }
    else
    {
        WeaponNameText->SetText(FText::FromString(WeaponName));
    }

    WeaponNameText->SetColorAndOpacity(DefaultTextColor);

    UE_LOG(LogTemp, VeryVerbose, TEXT("무기 이름 업데이트: %s"), *WeaponName);
}

void UPlayerHUD::UpdateWeaponSlots(const FString &PrimaryWeapon, const FString &SecondaryWeapon, int32 ActiveSlot)
{
    // Primary 슬롯 업데이트
    if (PrimarySlotText)
    {
        FString PrimaryText = PrimaryWeapon.IsEmpty() ? TEXT("[1] EMPTY") : FString::Printf(TEXT("[1] %s"), *PrimaryWeapon);

        PrimarySlotText->SetText(FText::FromString(PrimaryText));

        // 활성 슬롯이면 색상 변경
        if (ActiveSlot == 1)
        {
            PrimarySlotText->SetColorAndOpacity(ActiveSlotColor);
        }
        else
        {
            PrimarySlotText->SetColorAndOpacity(InactiveSlotColor);
        }
    }

    // Secondary 슬롯 업데이트
    if (SecondarySlotText)
    {
        FString SecondaryText = SecondaryWeapon.IsEmpty() ? TEXT("[2] EMPTY") : FString::Printf(TEXT("[2] %s"), *SecondaryWeapon);

        SecondarySlotText->SetText(FText::FromString(SecondaryText));

        // 활성 슬롯이면 색상 변경
        if (ActiveSlot == 2)
        {
            SecondarySlotText->SetColorAndOpacity(ActiveSlotColor);
        }
        else
        {
            SecondarySlotText->SetColorAndOpacity(InactiveSlotColor);
        }
    }

    // 현재 활성무기가 없다면 (BaseCrosshairSpread는 OnWeaponDeactivated에서 0으로 설정됨)

    UE_LOG(LogTemp, VeryVerbose, TEXT("무기 슬롯 업데이트: P=%s, S=%s, Active=%d"),
           *PrimaryWeapon, *SecondaryWeapon, ActiveSlot);
}

void UPlayerHUD::RefreshWeaponHUD()
{
    // 외부에서 호출하여 전체 HUD 갱신
    // WeaponSlotComponent에서 현재 상태를 가져와서 업데이트
    UE_LOG(LogTemp, Log, TEXT("WeaponHUD 전체 갱신 요청"));

    // TODO: WeaponSlotComponent에서 현재 상태 정보를 가져와서 업데이트
    // 지금은 기본 구조만 만들어둠
}

// === 체력/스태미나 업데이트 구현 ===
void UPlayerHUD::UpdateHealthBar(float CurrentHealth, float MaxHealth)
{
    if (!HealthBar)
    {
        UE_LOG(LogTemp, Warning, TEXT("HealthBar가 null입니다!"));
        return;
    }

    // 체력 퍼센트 계산
    float HealthPercent = (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
    HealthBar->SetPercent(HealthPercent);

    // 체력 텍스트 업데이트 (Optional)
    if (HealthText)
    {
        FString HealthString = FString::Printf(TEXT("%.0f / %.0f"), CurrentHealth, MaxHealth);
        HealthText->SetText(FText::FromString(HealthString));
    }

    // 체력이 낮으면 바 색상 변경 (Blueprint에서 설정 가능하도록)
    // 30% 이하면 빨간색, 60% 이하면 노란색
    FLinearColor BarColor = FLinearColor::Green;
    if (HealthPercent <= 0.3f)
    {
        BarColor = FLinearColor::Red;
    }
    else if (HealthPercent <= 0.6f)
    {
        BarColor = FLinearColor::Yellow;
    }

    HealthBar->SetFillColorAndOpacity(BarColor);

    // SizeBox Width 동적 조정 (MaxHealth 기준: 1 MaxHealth = 4px)
    if (HealthBarSizeBox)
    {
        float NewWidth = MaxHealth * 4.0f;  // 100 MaxHealth = 400px
        HealthBarSizeBox->SetWidthOverride(NewWidth);
        UE_LOG(LogTemp, Log, TEXT("HealthBar Width 조정: %.0fpx (MaxHealth=%.0f)"), NewWidth, MaxHealth);
    }

    UE_LOG(LogTemp, VeryVerbose, TEXT("체력 바 업데이트: %.0f / %.0f (%.1f%%)"), CurrentHealth, MaxHealth, HealthPercent * 100.0f);
}

void UPlayerHUD::UpdateStaminaBar(float CurrentStamina, float MaxStamina)
{
    if (!StaminaBar)
    {
        UE_LOG(LogTemp, Warning, TEXT("StaminaBar가 null입니다!"));
        return;
    }

    // 스태미나 퍼센트 계산
    float StaminaPercent = (MaxStamina > 0.0f) ? (CurrentStamina / MaxStamina) : 0.0f;
    StaminaBar->SetPercent(StaminaPercent);

    // 스태미나 텍스트 업데이트 (Optional)
    if (StaminaText)
    {
        FString StaminaString = FString::Printf(TEXT("%.0f / %.0f"), CurrentStamina, MaxStamina);
        StaminaText->SetText(FText::FromString(StaminaString));
    }

    // SizeBox Width 동적 조정 (MaxStamina 기준: 1 MaxStamina = 4px)
    if (StaminaBarSizeBox)
    {
        float NewWidth = MaxStamina * 4.0f;  // 100 MaxStamina = 400px
        StaminaBarSizeBox->SetWidthOverride(NewWidth);
        UE_LOG(LogTemp, Log, TEXT("StaminaBar Width 조정: %.0fpx (MaxStamina=%.0f)"), NewWidth, MaxStamina);
    }

    UE_LOG(LogTemp, Log, TEXT("스태미나 바 업데이트: %.0f / %.0f (%.1f%%)"), CurrentStamina, MaxStamina, StaminaPercent * 100.0f);
}

void UPlayerHUD::UpdateShieldBar(float CurrentShield, float MaxShield)
{
    if (!ShieldBar)
    {
        UE_LOG(LogTemp, Warning, TEXT("ShieldBar가 null입니다!"));
        return;
    }

    // MaxShield가 0이면 Border를 숨김 (스킬 미개방 상태)
    if (MaxShield <= 0.0f)
    {
        if (ShieldBarBorder)
        {
            ShieldBarBorder->SetVisibility(ESlateVisibility::Collapsed);
            UE_LOG(LogTemp, Log, TEXT("쉴드 스킬 미개방 - ShieldBarBorder 숨김"));
        }
        return;
    }

    // MaxShield가 0보다 크면 Border 표시
    if (ShieldBarBorder)
    {
        ShieldBarBorder->SetVisibility(ESlateVisibility::Visible);
        UE_LOG(LogTemp, Warning, TEXT("ShieldBarBorder를 Visible로 설정! CurrentShield=%.0f, MaxShield=%.0f"), CurrentShield, MaxShield);
    }

    // 쉴드 퍼센트 계산
    float ShieldPercent = CurrentShield / MaxShield;
    ShieldBar->SetPercent(ShieldPercent);

    // SizeBox Width 동적 조정 (MaxShield 기준: 1 MaxShield = 2px)
    if (ShieldBarSizeBox)
    {
        float NewWidth = MaxShield * 2.0f;  // 50 MaxShield = 100px
        ShieldBarSizeBox->SetWidthOverride(NewWidth);
        UE_LOG(LogTemp, Log, TEXT("ShieldBar Width 조정: %.0fpx (MaxShield=%.0f)"), NewWidth, MaxShield);
    }

    UE_LOG(LogTemp, Log, TEXT("쉴드 바 업데이트: %.0f / %.0f (%.1f%%)"), CurrentShield, MaxShield, ShieldPercent * 100.0f);
}

// === 크로스헤어 업데이트 구현 ===
void UPlayerHUD::SetCrosshairFiringSpread(float Spread)
{
    // 발사 확산 설정 (현재 값보다 크면 갱신)
    FiringSpreadAmount = FMath::Max(FiringSpreadAmount, Spread);
    UE_LOG(LogTemp, VeryVerbose, TEXT("크로스헤어 발사 확산: %.1f"), FiringSpreadAmount);
}

void UPlayerHUD::SetCrosshairMovementSpread(float Spread)
{
    // 이동 확산 설정
    MovementSpreadAmount = Spread;
    UE_LOG(LogTemp, VeryVerbose, TEXT("크로스헤어 이동 확산: %.1f"), MovementSpreadAmount);
}

void UPlayerHUD::SetBaseCrosshairSpread(float NewBaseSpread)
{
    // 기본 크로스헤어 확산 설정 (무기 교체 시)
    BaseCrosshairSpread = NewBaseSpread;
    UE_LOG(LogTemp, Log, TEXT("크로스헤어 기본 확산 변경: %.1f"), BaseCrosshairSpread);
}