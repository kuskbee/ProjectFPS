// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/UI/WeaponHUD.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Engine/Engine.h"

void UWeaponHUD::NativeConstruct()
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

void UWeaponHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // 크로스헤어 확산 부드럽게 보간
    if (FMath::Abs(CurrentCrosshairSpread - TargetCrosshairSpread) > 0.1f)
    {
        CurrentCrosshairSpread = FMath::FInterpTo(CurrentCrosshairSpread, TargetCrosshairSpread, InDeltaTime, CrosshairInterpSpeed);

        // 크로스헤어 위치 업데이트 (Blueprint에서 RenderTransform 사용)
        // 여기서는 확산 값만 업데이트, 실제 위치는 Blueprint에서 바인딩
    }
}

void UWeaponHUD::UpdateAmmoDisplay(int32 CurrentAmmo, int32 MaxAmmo)
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

void UWeaponHUD::UpdateWeaponName(const FString &WeaponName)
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

void UWeaponHUD::UpdateWeaponSlots(const FString &PrimaryWeapon, const FString &SecondaryWeapon, int32 ActiveSlot)
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

    UE_LOG(LogTemp, VeryVerbose, TEXT("무기 슬롯 업데이트: P=%s, S=%s, Active=%d"),
           *PrimaryWeapon, *SecondaryWeapon, ActiveSlot);
}

void UWeaponHUD::RefreshWeaponHUD()
{
    // 외부에서 호출하여 전체 HUD 갱신
    // WeaponSlotComponent에서 현재 상태를 가져와서 업데이트
    UE_LOG(LogTemp, Log, TEXT("WeaponHUD 전체 갱신 요청"));

    // TODO: WeaponSlotComponent에서 현재 상태 정보를 가져와서 업데이트
    // 지금은 기본 구조만 만들어둠
}

// === 체력/스태미나 업데이트 구현 ===
void UWeaponHUD::UpdateHealthBar(float CurrentHealth, float MaxHealth)
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

    UE_LOG(LogTemp, VeryVerbose, TEXT("체력 바 업데이트: %.0f / %.0f (%.1f%%)"), CurrentHealth, MaxHealth, HealthPercent * 100.0f);
}

void UWeaponHUD::UpdateStaminaBar(float CurrentStamina, float MaxStamina)
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

    // 스태미나 바 색상 (기본 청록색)
    StaminaBar->SetFillColorAndOpacity(FLinearColor(0.0f, 0.8f, 0.8f)); // Cyan

    UE_LOG(LogTemp, VeryVerbose, TEXT("스태미나 바 업데이트: %.0f / %.0f (%.1f%%)"), CurrentStamina, MaxStamina, StaminaPercent * 100.0f);
}

// === 크로스헤어 업데이트 구현 ===
void UWeaponHUD::SetCrosshairSpread(float Spread)
{
    TargetCrosshairSpread = FMath::Clamp(Spread, BaseCrosshairSpread, BaseCrosshairSpread * 3.0f);
    UE_LOG(LogTemp, VeryVerbose, TEXT("크로스헤어 확산 설정: %.1f"), TargetCrosshairSpread);
}