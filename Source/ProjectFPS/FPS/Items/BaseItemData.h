// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BaseItemData.generated.h"

/** 아이템 타입 열거형 */
UENUM(BlueprintType)
enum class EItemType : uint8
{
	None = 0,
	Weapon = 1,
	Consumable = 2,
	Equipment = 3,
	Ammo = 4,
	Misc = 5
};

/**
 * 모든 아이템의 기본 데이터 클래스
 * 다양한 아이템 타입의 기본이 되는 클래스
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTFPS_API UBaseItemData : public UDataAsset
{
	GENERATED_BODY()

public:
	UBaseItemData();

	/** 아이템 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info")
	FString ItemName = TEXT("Unknown Item");

	/** 아이템 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info", meta = (MultiLine = true))
	FString ItemDescription = TEXT("No description available.");

	/** 아이템 아이콘 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info")
	TObjectPtr<UTexture2D> ItemIcon;

	/** 아이템 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info")
	EItemType ItemType = EItemType::None;

	/** 최대 스택 크기 (1 = 스택 불가) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info", meta = (ClampMin = 1, ClampMax = 999))
	int32 MaxStackSize = 1;

	/** 현재 스택 개수 (런타임 정보) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Runtime")
	int32 CurrentStackSize = 1;

	/** 아이템 희귀도 (나중에 색상 등에 활용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info")
	int32 Rarity = 0;

	/** 아이템 가치 (상점 시스템용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info", meta = (ClampMin = 0))
	int32 ItemValue = 0;

	// ==================== 인벤토리 시스템 ====================

	/** 인벤토리 그리드 가로 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = 1, ClampMax = 8))
	int32 GridWidth = 1;

	/** 인벤토리 그리드 세로 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = 1, ClampMax = 6))
	int32 GridHeight = 1;

	// ==================== 월드 메시 (드롭/픽업용) ====================

	/** 월드에 드롭될 때 사용할 SkeletalMesh (무기 등) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Mesh")
	TObjectPtr<USkeletalMesh> WorldSkeletalMesh;

	/** 월드에 드롭될 때 사용할 StaticMesh (포션, 장비 등) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Mesh")
	TObjectPtr<UStaticMesh> WorldStaticMesh;

public:
	/** 아이템 타입 반환 */
	UFUNCTION(BlueprintPure, Category = "Item")
	EItemType GetItemType() const { return ItemType; }

	/** 스택 가능 여부 반환 */
	UFUNCTION(BlueprintPure, Category = "Item")
	bool IsStackable() const { return MaxStackSize > 1; }

	/** 아이템 이름 반환 */
	UFUNCTION(BlueprintPure, Category = "Item")
	FString GetItemName() const { return ItemName; }

	/** 인벤토리 그리드 크기 반환 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	FIntPoint GetGridSize() const { return FIntPoint(GridWidth, GridHeight); }

	/** 월드 메시 반환 (SkeletalMesh 우선, 없으면 StaticMesh) */
	UFUNCTION(BlueprintPure, Category = "World Mesh")
	UObject* GetWorldMesh() const
	{
		if (WorldSkeletalMesh) return WorldSkeletalMesh;
		return WorldStaticMesh;
	}
};