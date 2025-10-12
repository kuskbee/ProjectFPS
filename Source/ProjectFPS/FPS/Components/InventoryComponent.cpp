// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/Components/InventoryComponent.h"
#include "FPS/Items/BaseItemData.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// 기본 그리드 크기
	GridWidth = 8;
	GridHeight = 6;

	// 생성자에서 그리드 슬롯 초기화 (BeginPlay 이전에 사용될 수 있음)
	const int32 TotalSlots = GridWidth * GridHeight;
	GridSlots.Empty(TotalSlots);
	GridSlots.SetNum(TotalSlots);
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
	// 기본값 스택 개수 1로 호출
	return PlaceItemAt(Item, GridX, GridY, 1);
}

bool UInventoryComponent::PlaceItemAt(UBaseItemData* Item, int32 GridX, int32 GridY, int32 StackCount)
{
	// 배치 가능 여부 체크
	if (!CanPlaceItemAt(Item, GridX, GridY))
	{
		return false;
	}

	// DataAsset 원본을 보호하기 위해 복제본 생성 (PIE에서 저장 방지)
	UBaseItemData* ItemCopy = DuplicateObject(Item, this);
	ItemCopy->ClearFlags(RF_Public | RF_Standalone);
	ItemCopy->SetFlags(RF_Transient);  // PIE 종료 시 자동 삭제

	// 아이템이 차지할 모든 칸을 "Occupied" 상태로 변경
	for (int32 Y = 0; Y < Item->GridHeight; ++Y)
	{
		for (int32 X = 0; X < Item->GridWidth; ++X)
		{
			int32 SlotIndex = GetSlotIndex(GridX + X, GridY + Y);
			GridSlots[SlotIndex].bIsOccupied = true;
			GridSlots[SlotIndex].OriginPos = FIntPoint(GridX, GridY);  // 모든 칸에 Origin 좌표 저장

			// 시작점(Origin)에만 ItemData 복제본 저장
			if (X == 0 && Y == 0)
			{
				GridSlots[SlotIndex].ItemData = ItemCopy;
				GridSlots[SlotIndex].bIsOrigin = true;
				// BaseItemData 복제본의 CurrentStackSize 설정
				ItemCopy->CurrentStackSize = StackCount;
			}
			else
			{
				GridSlots[SlotIndex].bIsOrigin = false;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("아이템 배치 성공: %s x%d at (%d, %d), Size: %dx%d"),
		*Item->GetItemName(), StackCount, GridX, GridY, Item->GridWidth, Item->GridHeight);

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

bool UInventoryComponent::AutoPlaceItem(UBaseItemData* Item, int32& OutX, int32& OutY, int32 StackCount)
{
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("AutoPlaceItem: Item이 null입니다."));
		return false;
	}

	// 1. 스택 가능한 아이템인 경우 기존 스택 찾기
	if (Item->IsStackable())
	{
		for (int32 Y = 0; Y < GridHeight; ++Y)
		{
			for (int32 X = 0; X < GridWidth; ++X)
			{
				int32 SlotIndex = GetSlotIndex(X, Y);
				FInventorySlot& Slot = GridSlots[SlotIndex];

				// Origin 슬롯이고 같은 종류의 아이템이며 스택 여유가 있는 경우
				// ItemID로 비교 (같은 DataAsset에서 나온 아이템들은 같은 ItemID를 가짐)
				if (Slot.bIsOrigin && Slot.ItemData &&
					Slot.ItemData->ItemID != NAME_None &&
					Slot.ItemData->ItemID == Item->ItemID &&
					Slot.ItemData->CurrentStackSize + StackCount <= Item->MaxStackSize)
				{
					// 기존 스택에 추가
					Slot.ItemData->CurrentStackSize += StackCount;
					OutX = X;
					OutY = Y;
					UE_LOG(LogTemp, Log, TEXT("AutoPlaceItem 스택 추가: %s x%d (총 %d개) at (%d, %d)"),
						*Item->GetItemName(), StackCount, Slot.ItemData->CurrentStackSize, X, Y);

					// 인벤토리 변경 이벤트
					OnInventoryChanged.Broadcast();
					return true;
				}
			}
		}
	}

	// 2. 기존 스택이 없거나 스택 불가능한 경우 새 슬롯에 배치
	for (int32 Y = 0; Y < GridHeight; ++Y)
	{
		for (int32 X = 0; X < GridWidth; ++X)
		{
			if (CanPlaceItemAt(Item, X, Y))
			{
				if (PlaceItemAt(Item, X, Y, StackCount))
				{
					OutX = X;
					OutY = Y;
					UE_LOG(LogTemp, Log, TEXT("AutoPlaceItem 새 슬롯: %s x%d at (%d, %d)"),
						*Item->GetItemName(), StackCount, X, Y);
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
	PlaceItemAt(Item, ToX, ToY, Item->CurrentStackSize);

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

int32 UInventoryComponent::GetItemStackCount(int32 GridX, int32 GridY) const
{
	// Origin 찾기
	int32 OriginX, OriginY;
	if (!FindItemOrigin(GridX, GridY, OriginX, OriginY))
	{
		return 0;  // 아이템이 없음
	}

	// Origin 슬롯의 ItemData->CurrentStackSize 반환
	int32 OriginIndex = GetSlotIndex(OriginX, OriginY);
	const FInventorySlot& Slot = GridSlots[OriginIndex];

	if (Slot.ItemData)
	{
		return Slot.ItemData->CurrentStackSize;
	}

	return 0;
}

bool UInventoryComponent::DecreaseStackAt(int32 GridX, int32 GridY, int32 Amount)
{
	// Origin 찾기
	int32 OriginX, OriginY;
	if (!FindItemOrigin(GridX, GridY, OriginX, OriginY))
	{
		UE_LOG(LogTemp, Warning, TEXT("DecreaseStackAt: Origin을 찾을 수 없습니다. (%d, %d)"), GridX, GridY);
		return false;
	}

	int32 OriginIndex = GetSlotIndex(OriginX, OriginY);
	FInventorySlot& Slot = GridSlots[OriginIndex];

	if (!Slot.ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("DecreaseStackAt: ItemData가 없습니다."));
		return false;
	}

	// 스택 개수 체크
	if (Slot.ItemData->CurrentStackSize < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("DecreaseStackAt: 스택 개수 부족. 현재: %d, 요청: %d"),
			Slot.ItemData->CurrentStackSize, Amount);
		return false;
	}

	// 스택 감소
	Slot.ItemData->CurrentStackSize -= Amount;

	// 스택이 0이 되면 아이템 제거
	if (Slot.ItemData->CurrentStackSize <= 0)
	{
		UE_LOG(LogTemp, Log, TEXT("DecreaseStackAt: 스택이 0이 되어 아이템 제거"));
		return RemoveItemAt(OriginX, OriginY);
	}

	UE_LOG(LogTemp, Log, TEXT("DecreaseStackAt: %d개 감소, 남은 스택: %d"), Amount, Slot.ItemData->CurrentStackSize);

	// 인벤토리 변경 이벤트
	OnInventoryChanged.Broadcast();

	return true;
}
