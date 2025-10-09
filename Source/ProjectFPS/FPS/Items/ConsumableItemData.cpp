// Fill out your copyright notice in the Description page of Project Settings.

#include "ConsumableItemData.h"

UConsumableItemData::UConsumableItemData()
{
	// 기본값 설정
	ItemType = EItemType::Consumable;
	Rarity = 0;  // 0 = Common

	// 포션은 보통 1x1 크기
	GridWidth = 1;
	GridHeight = 1;

	ItemName = TEXT("Potion");
	ItemDescription = TEXT("Restores health or other stats");
}
