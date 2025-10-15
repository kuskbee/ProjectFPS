// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/** 무기 슬롯 타입 */
UENUM(BlueprintType)
enum class EWeaponSlot : uint8
{
	None = 255,
	Primary = 0,
	Secondary = 1,
	Max = 2
};
