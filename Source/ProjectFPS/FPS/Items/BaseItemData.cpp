// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/Items/BaseItemData.h"

UBaseItemData::UBaseItemData()
{
	// 기본값 설정
	ItemName = TEXT("Unknown Item");
	ItemDescription = TEXT("No description available.");
	ItemType = EItemType::None;
	MaxStackSize = 1;
	Rarity = 0;
	ItemValue = 0;
}