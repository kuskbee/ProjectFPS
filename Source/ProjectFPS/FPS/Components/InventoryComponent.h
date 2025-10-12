// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class UBaseItemData;

/**
 * 인벤토리 슬롯 구조체
 * 그리드의 각 칸을 나타냄
 */
USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()

	/** 이 슬롯에 저장된 아이템 데이터 (Origin 슬롯에만 저장) */
	UPROPERTY()
	TObjectPtr<UBaseItemData> ItemData = nullptr;

	/** 이 슬롯이 아이템으로 점유되어 있는지 여부 */
	UPROPERTY()
	bool bIsOccupied = false;

	/** 이 슬롯이 아이템의 시작점(Origin)인지 여부 */
	UPROPERTY()
	bool bIsOrigin = false;

	/** 이 슬롯이 속한 아이템의 Origin 좌표 (O(1) 탐색용) */
	UPROPERTY()
	FIntPoint OriginPos = FIntPoint(-1, -1);

	// 기본 생성자
	FInventorySlot()
		: ItemData(nullptr)
		, bIsOccupied(false)
		, bIsOrigin(false)
		, OriginPos(-1, -1)
	{}
};

/**
 * 디아블로2 스타일 그리드 인벤토리 컴포넌트
 * 가변 크기 아이템 배치 지원 (1x1, 2x2, 2x4 등)
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTFPS_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;

public:
	// ==================== 그리드 설정 ====================

	/** 그리드 가로 크기 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	int32 GridWidth = 8;

	/** 그리드 세로 크기 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	int32 GridHeight = 6;

	/** 그리드 슬롯 데이터 (1차원 배열: Index = Y * GridWidth + X) */
	UPROPERTY()
	TArray<FInventorySlot> GridSlots;

	// ==================== 델리게이트 ====================

	/** 인벤토리 변경 시 호출되는 델리게이트 (UI 갱신용) */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;

	// ==================== 핵심 함수들 ====================

	/**
	 * 특정 위치에 아이템 배치 가능한지 체크
	 * @param Item 배치할 아이템
	 * @param GridX 그리드 X 좌표
	 * @param GridY 그리드 Y 좌표
	 * @return 배치 가능하면 true
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool CanPlaceItemAt(UBaseItemData* Item, int32 GridX, int32 GridY) const;

	/**
	 * 특정 위치에 아이템 배치
	 * @param Item 배치할 아이템
	 * @param GridX 그리드 X 좌표
	 * @param GridY 그리드 Y 좌표
	 * @return 배치 성공하면 true
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool PlaceItemAt(UBaseItemData* Item, int32 GridX, int32 GridY);

	/**
	 * 특정 위치에 아이템 배치 (스택 개수 지정)
	 * @param Item 배치할 아이템
	 * @param GridX 그리드 X 좌표
	 * @param GridY 그리드 Y 좌표
	 * @param StackCount 스택 개수
	 * @return 배치 성공하면 true
	 */
	bool PlaceItemAt(UBaseItemData* Item, int32 GridX, int32 GridY, int32 StackCount);

	/**
	 * 특정 위치의 아이템 제거
	 * @param GridX 그리드 X 좌표
	 * @param GridY 그리드 Y 좌표
	 * @return 제거 성공하면 true
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItemAt(int32 GridX, int32 GridY);

	/**
	 * 특정 위치의 스택 감소 (소모품 사용 등)
	 * @param GridX 그리드 X 좌표
	 * @param GridY 그리드 Y 좌표
	 * @param Amount 감소할 개수 (기본값 1)
	 * @return 감소 성공하면 true
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool DecreaseStackAt(int32 GridX, int32 GridY, int32 Amount = 1);

	/**
	 * 아이템을 빈 공간에 자동 배치 (E키 픽업용)
	 * @param Item 배치할 아이템
	 * @param OutX 배치된 X 좌표 (출력)
	 * @param OutY 배치된 Y 좌표 (출력)
	 * @param StackCount 스택 개수 (기본값 1)
	 * @return 배치 성공하면 true, 공간 부족하면 false
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AutoPlaceItem(UBaseItemData* Item, int32& OutX, int32& OutY, int32 StackCount = 1);

	/**
	 * 아이템 이동 (기존 위치 제거 + 새 위치 배치)
	 * @param FromX 기존 X 좌표
	 * @param FromY 기존 Y 좌표
	 * @param ToX 새 X 좌표
	 * @param ToY 새 Y 좌표
	 * @return 이동 성공하면 true
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool MoveItem(int32 FromX, int32 FromY, int32 ToX, int32 ToY);

	// ==================== 유틸리티 함수 ====================

	/**
	 * 2D 그리드 좌표를 1D 배열 인덱스로 변환
	 * @param GridX X 좌표
	 * @param GridY Y 좌표
	 * @return 배열 인덱스
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetSlotIndex(int32 GridX, int32 GridY) const
	{
		return GridY * GridWidth + GridX;
	}

	/**
	 * 특정 위치의 아이템 데이터 반환
	 * @param GridX X 좌표
	 * @param GridY Y 좌표
	 * @return 아이템 데이터 (없으면 nullptr)
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	UBaseItemData* GetItemAt(int32 GridX, int32 GridY) const;

	/**
	 * 그리드 가로 크기 반환
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetGridWidth() const { return GridWidth; }

	/**
	 * 그리드 세로 크기 반환
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetGridHeight() const { return GridHeight; }

	/**
	 * 특정 위치의 아이템 Origin 좌표 찾기
	 * @param GridX X 좌표
	 * @param GridY Y 좌표
	 * @param OutOriginX Origin X 좌표 (출력)
	 * @param OutOriginY Origin Y 좌표 (출력)
	 * @return Origin 찾기 성공하면 true
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool FindItemOrigin(int32 GridX, int32 GridY, int32& OutOriginX, int32& OutOriginY) const;

	/**
	 * 특정 위치 아이템의 스택 개수 가져오기
	 * @param GridX X 좌표
	 * @param GridY Y 좌표
	 * @return 스택 개수 (아이템 없으면 0)
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetItemStackCount(int32 GridX, int32 GridY) const;

	/**
	 * 그리드 슬롯 배열 반환 (UI에서 읽기용)
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	const TArray<FInventorySlot>& GetGridSlots() const { return GridSlots; }

	/**
	 * 인벤토리 초기화 (그리드 슬롯 생성)
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitializeInventory();
};
