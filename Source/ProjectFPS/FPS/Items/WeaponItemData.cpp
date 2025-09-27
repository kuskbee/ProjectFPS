// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/Items/WeaponItemData.h"
#include "FPS/Weapons/FPSWeapon.h"

UWeaponItemData::UWeaponItemData()
{
	// 기본값 설정
	ItemType = EItemType::Weapon;
	MaxStackSize = 1; // 무기는 스택 불가

	// 무기 기본 스탯
	BaseDamage = 10;
	FireRate = 1.0f;
	MagazineSize = 30;
	AccuracySpread = 2.0f;
	RecoilStrength = 1.0f;
	WeaponRange = 5000.0f;
	bIsAutomatic = false;

	// 런타임 상태 초기화
	CurrentAmmo = MagazineSize; // 처음엔 탄약 가득
	Durability = 100.0f;
}

bool UWeaponItemData::IsValidWeapon() const
{
	return WeaponClass != nullptr;
}

bool UWeaponItemData::ConsumeAmmo(int32 AmmoToConsume)
{
	if (CurrentAmmo >= AmmoToConsume)
	{
		CurrentAmmo -= AmmoToConsume;
		return true;
	}
	return false;
}
