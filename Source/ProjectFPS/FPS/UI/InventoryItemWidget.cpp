// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryItemWidget.h"
#include "Components/Image.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "../Items/BaseItemData.h"
#include "ItemDragDropOperation.h"

UInventoryItemWidget::UInventoryItemWidget(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UInventoryItemWidget::SetItemData(UBaseItemData *InItemData, int32 InGridX, int32 InGridY)
{
	ItemData = InItemData;
	GridX = InGridX;
	GridY = InGridY;

	// 아이템 이미지 설정
	if (ItemImage && ItemData && ItemData->ItemIcon)
	{
		ItemImage->SetBrushFromTexture(ItemData->ItemIcon);
	}
}

FReply UInventoryItemWidget::NativeOnMouseButtonDown(const FGeometry &InGeometry, const FPointerEvent &InMouseEvent)
{
	// 좌클릭: 드래그 시작 감지
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		// DetectDrag 시작
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	// 우클릭: 컨텍스트 메뉴 (추후 구현)
	if (InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		UE_LOG(LogTemp, Log, TEXT("우클릭: 컨텍스트 메뉴 (추후 구현)"));
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void UInventoryItemWidget::NativeOnDragDetected(const FGeometry &InGeometry, const FPointerEvent &InMouseEvent, UDragDropOperation *&OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("NativeOnDragDetected: ItemData가 없습니다"));
		return;
	}

	// DragDropOperation 생성
	UItemDragDropOperation *DragOp = NewObject<UItemDragDropOperation>();
	if (!DragOp)
	{
		UE_LOG(LogTemp, Error, TEXT("NativeOnDragDetected: DragDropOperation 생성 실패"));
		return;
	}

	// ⭐ DragOp 데이터 설정
	DragOp->DraggedItem = ItemData;
	DragOp->OriginGridX = GridX;
	DragOp->OriginGridY = GridY;
	DragOp->DragSource = EItemDragSource::Grid; // 인벤토리 그리드에서 드래그

	// 드래그 비주얼 설정 (아이템 이미지를 따라다니게)
	DragOp->DefaultDragVisual = this;
	DragOp->Pivot = EDragPivot::MouseDown;

	OutOperation = DragOp;

	UE_LOG(LogTemp, Log, TEXT("드래그 시작: %s (%d, %d)"),
		   *ItemData->GetItemName(), GridX, GridY);
}

FReply UInventoryItemWidget::NativeOnMouseButtonUp(const FGeometry &InGeometry, const FPointerEvent &InMouseEvent)
{
	// 우클릭으로 드래그 취소
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		// 현재 드래그 중인 경우 취소
		UDragDropOperation *CurrentDragOp = UWidgetBlueprintLibrary::GetDragDroppingContent();
		if (CurrentDragOp)
		{
			// 드래그 취소
			UWidgetBlueprintLibrary::CancelDragDrop();
			UE_LOG(LogTemp, Log, TEXT("우클릭으로 드래그 취소"));
			return FReply::Handled();
		}
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}
