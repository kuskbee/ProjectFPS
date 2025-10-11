// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Components/EWeaponSlot.h"
#include "ItemDragDropOperation.generated.h"

class UBaseItemData;

/** 드래그 시작 위치 타입 */
UENUM(BlueprintType)
enum class EItemDragSource : uint8
{
	Grid UMETA(DisplayName = "Grid"),              // 인벤토리 그리드에서
	WeaponSlot UMETA(DisplayName = "Weapon Slot"), // 무기 슬롯에서
	World UMETA(DisplayName = "World")             // 월드 픽업 아이템
};

/**
 * 드래그 중인 아이템 정보를 담는 DragDropOperation
 * UMG 드래그 앤 드롭 시스템에서 사용
 */
UCLASS()
class PROJECTFPS_API UItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	/** 드래그 중인 아이템 데이터 */
	UPROPERTY(BlueprintReadWrite, Category = "Drag Drop")
	TObjectPtr<UBaseItemData> DraggedItem;

	/** 원래 위치 (취소 시 복구용) */
	UPROPERTY(BlueprintReadWrite, Category = "Drag Drop")
	int32 OriginGridX = -1;

	UPROPERTY(BlueprintReadWrite, Category = "Drag Drop")
	int32 OriginGridY = -1;

	/** 드래그 시작 위치 타입 */
	UPROPERTY(BlueprintReadWrite, Category = "Drag Drop")
	EItemDragSource DragSource = EItemDragSource::Grid;

	/** 원래 무기 슬롯 (WeaponSlot에서 드래그한 경우) */
	UPROPERTY(BlueprintReadWrite, Category = "Drag Drop")
	EWeaponSlot OriginWeaponSlot = EWeaponSlot::None;
};
