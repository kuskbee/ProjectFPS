// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WeaponSlotItemWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/WeaponSlotComponent.h"
#include "Components/InventoryComponent.h"
#include "Items/WeaponItemData.h"
#include "UI/ItemDragDropOperation.h"
#include "UI/InventoryWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void UWeaponSlotItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UWeaponSlotItemWidget::SetWeaponSlot(EWeaponSlot InSlotType)
{
	SlotType = InSlotType;

	// 슬롯 이름 텍스트 설정
	if (SlotNameText)
	{
		FString SlotName = (SlotType == EWeaponSlot::Primary) ? TEXT("Primary") : TEXT("Secondary");
		SlotNameText->SetText(FText::FromString(SlotName));
	}
}

void UWeaponSlotItemWidget::SetComponents(UWeaponSlotComponent* InWeaponSlotComponent, UInventoryComponent* InInventoryComponent)
{
	WeaponSlotComponent = InWeaponSlotComponent;
	InventoryComponent = InInventoryComponent;
}

void UWeaponSlotItemWidget::SetWeaponIcon(UWeaponItemData* WeaponData)
{
	CurrentWeaponData = WeaponData;

	if (WeaponIconImage && WeaponData && WeaponData->ItemIcon)
	{
		WeaponIconImage->SetBrushFromTexture(WeaponData->ItemIcon);
		WeaponIconImage->SetVisibility(ESlateVisibility::Visible);
	}
}

void UWeaponSlotItemWidget::ClearWeaponIcon()
{
	CurrentWeaponData = nullptr;

	if (WeaponIconImage)
	{
		// 빈 슬롯 배경 텍스처가 있으면 표시, 없으면 숨김
		if (EmptySlotTexture)
		{
			WeaponIconImage->SetBrushFromTexture(EmptySlotTexture);
			WeaponIconImage->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			WeaponIconImage->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

// ========================================
// 드래그 앤 드롭
// ========================================

FReply UWeaponSlotItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 무기가 있을 때만 드래그 시작 가능
	if (CurrentWeaponData && InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	return FReply::Unhandled();
}

void UWeaponSlotItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!CurrentWeaponData)
	{
		UE_LOG(LogTemp, Warning, TEXT("NativeOnDragDetected: CurrentWeaponData가 없습니다"));
		return;
	}

	// DragDropOperation 생성
	UItemDragDropOperation* DragOp = NewObject<UItemDragDropOperation>();
	if (!DragOp)
	{
		UE_LOG(LogTemp, Error, TEXT("NativeOnDragDetected: DragDropOperation 생성 실패"));
		return;
	}

	DragOp->DraggedItem = CurrentWeaponData;
	DragOp->DragSource = EItemDragSource::WeaponSlot;
	DragOp->OriginWeaponSlot = SlotType;  // 원래 슬롯 저장

	OutOperation = DragOp;

	UE_LOG(LogTemp, Log, TEXT("무기 슬롯에서 드래그 시작: %s"), *CurrentWeaponData->GetItemName());
}

bool UWeaponSlotItemWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UItemDragDropOperation* DragOp = Cast<UItemDragDropOperation>(InOperation);
	if (!DragOp || !DragOp->DraggedItem)
	{
		return false;
	}

	// WeaponItemData만 허용
	UWeaponItemData* WeaponData = Cast<UWeaponItemData>(DragOp->DraggedItem);
	if (!WeaponData)
	{
		UE_LOG(LogTemp, Warning, TEXT("무기 슬롯에는 무기만 배치할 수 있습니다"));
		return false;
	}

	// 같은 슬롯에 드롭하면 취소
	if (DragOp->DragSource == EItemDragSource::WeaponSlot && DragOp->OriginWeaponSlot == SlotType)
	{
		UE_LOG(LogTemp, Log, TEXT("같은 슬롯에 드롭 - 취소"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("무기 슬롯 OnDrop: %s → %s 슬롯 (From: %d)"),
		*WeaponData->GetItemName(),
		(SlotType == EWeaponSlot::Primary) ? TEXT("Primary") : TEXT("Secondary"),
		(int32)DragOp->DragSource);

	// 컴포넌트가 없으면 실패
	if (!WeaponSlotComponent || !InventoryComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("WeaponSlotComponent 또는 InventoryComponent가 없습니다!"));
		return false;
	}

	// 타겟 슬롯에 이미 무기가 있는지 체크
	UWeaponItemData* ExistingWeapon = WeaponSlotComponent->GetWeaponInSlot(SlotType);
	bool bTargetSlotHasWeapon = (ExistingWeapon != nullptr);

	bool bSuccess = false;

	// 무기 슬롯 → 무기 슬롯 (교환)
	if (DragOp->DragSource == EItemDragSource::WeaponSlot && bTargetSlotHasWeapon)
	{
		// WeaponSlotComponent의 SwapWeaponSlots 함수 사용
		bSuccess = WeaponSlotComponent->SwapWeaponSlots(DragOp->OriginWeaponSlot, SlotType);

		if (bSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("무기 슬롯 <교환> 성공"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("무기 슬롯 <교환> 실패"));
		}
	}
	else
	{
		// 일반 장착 (그리드 → 무기 슬롯, 또는 빈 슬롯으로 이동)

		// 원래 위치에서 제거
		if (DragOp->DragSource == EItemDragSource::Grid)
		{
			InventoryComponent->RemoveItemAt(DragOp->OriginGridX, DragOp->OriginGridY);
		}
		else if (DragOp->DragSource == EItemDragSource::WeaponSlot)
		{
			WeaponSlotComponent->UnequipWeaponFromSlot(DragOp->OriginWeaponSlot);
		}

		// 무기 장착
		bSuccess = WeaponSlotComponent->EquipWeaponToSlot(SlotType, WeaponData);

		if (bSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("무기 슬롯 <장착> 성공"));
		}
		else
		{
			// 실패 시 원래 위치 복구
			if (DragOp->DragSource == EItemDragSource::Grid)
			{
				InventoryComponent->PlaceItemAt(WeaponData, DragOp->OriginGridX, DragOp->OriginGridY);
			}
			else if (DragOp->DragSource == EItemDragSource::WeaponSlot)
			{
				WeaponSlotComponent->EquipWeaponToSlot(DragOp->OriginWeaponSlot, WeaponData);
			}
			UE_LOG(LogTemp, Warning, TEXT("무기 슬롯 <장착> 실패 - 원래 위치 복구"));
		}
	}

	// 부모 InventoryWidget 찾아서 아이콘 갱신 / 드래그 이벤트 종료
	UInventoryWidget* ParentInventory = GetTypedOuter<UInventoryWidget>();
	if (ParentInventory)
	{
		ParentInventory->RefreshWeaponSlots();
		ParentInventory->FinishDragDropEvent();
	}

	return bSuccess;
}
