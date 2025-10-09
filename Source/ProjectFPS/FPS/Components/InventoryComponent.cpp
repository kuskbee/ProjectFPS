// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/Components/InventoryComponent.h"
#include "FPS/Items/BaseItemData.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// 기본 그리드 크기
	GridWidth = 8;
	GridHeight = 6;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// 인벤토리 초기화
	InitializeInventory();
}

void UInventoryComponent::InitializeInventory()
{
	// 그리드 슬롯 배열 생성 (8x6 = 48칸)
	const int32 TotalSlots = GridWidth * GridHeight;
	GridSlots.Empty(TotalSlots);
	GridSlots.SetNum(TotalSlots);

	UE_LOG(LogTemp, Log, TEXT("InventoryComponent 초기화 완료: %dx%d = %d 슬롯"), GridWidth, GridHeight, TotalSlots);
}

// ==================== 핵심 함수 구현 ====================

bool UInventoryComponent::CanPlaceItemAt(UBaseItemData* Item, int32 GridX, int32 GridY) const
{
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("CanPlaceItemAt: Item이 null입니다."));
		return false;
	}

	// 그리드 범위 체크
	if (GridX < 0 || GridY < 0 ||
		GridX + Item->GridWidth > GridWidth ||
		GridY + Item->GridHeight > GridHeight)
	{
		// 그리드 범위 밖
		return false;
	}

	// 아이템이 차지할 모든 칸이 비어있는지 체크
	for (int32 Y = 0; Y < Item->GridHeight; ++Y)
	{
		for (int32 X = 0; X < Item->GridWidth; ++X)
		{
			int32 CheckX = GridX + X;
			int32 CheckY = GridY + Y;
			int32 SlotIndex = GetSlotIndex(CheckX, CheckY);

			if (GridSlots[SlotIndex].bIsOccupied)
			{
				// 이미 아이템이 있음
				return false;
			}
		}
	}

	return true;
}

bool UInventoryComponent::PlaceItemAt(UBaseItemData* Item, int32 GridX, int32 GridY)
{
	// 배치 가능 여부 체크
	if (!CanPlaceItemAt(Item, GridX, GridY))
	{
		return false;
	}

	// 아이템이 차지할 모든 칸을 "Occupied" 상태로 변경
	for (int32 Y = 0; Y < Item->GridHeight; ++Y)
	{
		for (int32 X = 0; X < Item->GridWidth; ++X)
		{
			int32 SlotIndex = GetSlotIndex(GridX + X, GridY + Y);
			GridSlots[SlotIndex].bIsOccupied = true;
			GridSlots[SlotIndex].OriginPos = FIntPoint(GridX, GridY);  // 모든 칸에 Origin 좌표 저장

			// 시작점(Origin)에만 ItemData 저장
			if (X == 0 && Y == 0)
			{
				GridSlots[SlotIndex].ItemData = Item;
				GridSlots[SlotIndex].bIsOrigin = true;
			}
			else
			{
				GridSlots[SlotIndex].bIsOrigin = false;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("아이템 배치 성공: %s at (%d, %d), Size: %dx%d"),
		*Item->GetItemName(), GridX, GridY, Item->GridWidth, Item->GridHeight);

	// 인벤토리 변경 이벤트 발생
	OnInventoryChanged.Broadcast();

	return true;
}

bool UInventoryComponent::RemoveItemAt(int32 GridX, int32 GridY)
{
	// 범위 체크
	if (GridX < 0 || GridX >= GridWidth || GridY < 0 || GridY >= GridHeight)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItemAt: 잘못된 좌표 (%d, %d)"), GridX, GridY);
		return false;
	}

	// Origin 좌표 찾기
	int32 OriginX, OriginY;
	if (!FindItemOrigin(GridX, GridY, OriginX, OriginY))
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItemAt: Origin을 찾을 수 없습니다. (%d, %d)"), GridX, GridY);
		return false;
	}

	int32 OriginIndex = GetSlotIndex(OriginX, OriginY);
	UBaseItemData* Item = GridSlots[OriginIndex].ItemData;

	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItemAt: ItemData가 없습니다. (%d, %d)"), OriginX, OriginY);
		return false;
	}

	// 아이템이 차지하는 모든 칸 초기화
	for (int32 Y = 0; Y < Item->GridHeight; ++Y)
	{
		for (int32 X = 0; X < Item->GridWidth; ++X)
		{
			int32 SlotIndex = GetSlotIndex(OriginX + X, OriginY + Y);
			GridSlots[SlotIndex].ItemData = nullptr;
			GridSlots[SlotIndex].bIsOccupied = false;
			GridSlots[SlotIndex].bIsOrigin = false;
			GridSlots[SlotIndex].OriginPos = FIntPoint(-1, -1);  // Origin 좌표 초기화
		}
	}

	UE_LOG(LogTemp, Log, TEXT("아이템 제거 성공: %s at (%d, %d)"), *Item->GetItemName(), OriginX, OriginY);

	// 인벤토리 변경 이벤트 발생
	OnInventoryChanged.Broadcast();

	return true;
}

bool UInventoryComponent::AutoPlaceItem(UBaseItemData* Item, int32& OutX, int32& OutY)
{
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("AutoPlaceItem: Item이 null입니다."));
		return false;
	}

	// 왼쪽 위부터 순차적으로 빈 공간 탐색
	for (int32 Y = 0; Y < GridHeight; ++Y)
	{
		for (int32 X = 0; X < GridWidth; ++X)
		{
			if (CanPlaceItemAt(Item, X, Y))
			{
				if (PlaceItemAt(Item, X, Y))
				{
					OutX = X;
					OutY = Y;
					UE_LOG(LogTemp, Log, TEXT("AutoPlaceItem 성공: %s at (%d, %d)"), *Item->GetItemName(), X, Y);
					return true;
				}
			}
		}
	}

	// 빈 공간 없음
	UE_LOG(LogTemp, Warning, TEXT("AutoPlaceItem 실패: 인벤토리 공간 부족! (%s)"), *Item->GetItemName());
	return false;
}

bool UInventoryComponent::MoveItem(int32 FromX, int32 FromY, int32 ToX, int32 ToY)
{
	// Origin 좌표 찾기
	int32 OriginX, OriginY;
	if (!FindItemOrigin(FromX, FromY, OriginX, OriginY))
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveItem: From 위치에 아이템이 없습니다. (%d, %d)"), FromX, FromY);
		return false;
	}

	int32 OriginIndex = GetSlotIndex(OriginX, OriginY);
	UBaseItemData* Item = GridSlots[OriginIndex].ItemData;

	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveItem: ItemData가 없습니다. (%d, %d)"), OriginX, OriginY);
		return false;
	}

	// 새 위치에 배치 가능한지 체크
	if (!CanPlaceItemAt(Item, ToX, ToY))
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveItem: To 위치에 배치할 수 없습니다. (%d, %d)"), ToX, ToY);
		return false;
	}

	// 기존 위치 제거
	for (int32 Y = 0; Y < Item->GridHeight; ++Y)
	{
		for (int32 X = 0; X < Item->GridWidth; ++X)
		{
			int32 SlotIndex = GetSlotIndex(OriginX + X, OriginY + Y);
			GridSlots[SlotIndex].ItemData = nullptr;
			GridSlots[SlotIndex].bIsOccupied = false;
			GridSlots[SlotIndex].bIsOrigin = false;
			GridSlots[SlotIndex].OriginPos = FIntPoint(-1, -1);  // Origin 좌표 초기화
		}
	}

	// 새 위치에 배치
	PlaceItemAt(Item, ToX, ToY);

	UE_LOG(LogTemp, Log, TEXT("아이템 이동 성공: %s (%d, %d) -> (%d, %d)"),
		*Item->GetItemName(), OriginX, OriginY, ToX, ToY);

	return true;
}

// ==================== 유틸리티 함수 구현 ====================

UBaseItemData* UInventoryComponent::GetItemAt(int32 GridX, int32 GridY) const
{
	// 범위 체크
	if (GridX < 0 || GridX >= GridWidth || GridY < 0 || GridY >= GridHeight)
	{
		return nullptr;
	}

	// Origin 좌표 찾기
	int32 OriginX, OriginY;
	if (!FindItemOrigin(GridX, GridY, OriginX, OriginY))
	{
		return nullptr;
	}

	int32 OriginIndex = GetSlotIndex(OriginX, OriginY);
	return GridSlots[OriginIndex].ItemData;
}

bool UInventoryComponent::FindItemOrigin(int32 GridX, int32 GridY, int32& OutOriginX, int32& OutOriginY) const
{
	// 범위 체크
	if (GridX < 0 || GridX >= GridWidth || GridY < 0 || GridY >= GridHeight)
	{
		return false;
	}

	int32 SlotIndex = GetSlotIndex(GridX, GridY);

	// 슬롯이 비어있으면 실패
	if (!GridSlots[SlotIndex].bIsOccupied)
	{
		return false;
	}

	// OriginPos를 그대로 반환 (O(1) 탐색!)
	FIntPoint OriginPos = GridSlots[SlotIndex].OriginPos;

	// 유효한 Origin인지 확인
	if (OriginPos.X < 0 || OriginPos.Y < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FindItemOrigin: 유효하지 않은 OriginPos (%d, %d) at (%d, %d)"),
			OriginPos.X, OriginPos.Y, GridX, GridY);
		return false;
	}

	OutOriginX = OriginPos.X;
	OutOriginY = OriginPos.Y;
	return true;
}
