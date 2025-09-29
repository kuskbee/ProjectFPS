// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/UI/WeaponHUD.h"
#include "Components/TextBlock.h"
#include "Engine/Engine.h"

void UWeaponHUD::NativeConstruct()
{
    Super::NativeConstruct();

    // 초기 상태 설정
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

    UE_LOG(LogTemp, Log, TEXT("WeaponHUD 초기화 완료"));
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