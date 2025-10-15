// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryItemWidget.generated.h"

class UImage;
class UTextBlock;
class UBaseItemData;

/**
 * 인벤토리 그리드에 표시되는 개별 아이템 위젯
 * - 드래그 시작 지점
 * - 아이템 이미지 표시
 */
UCLASS()
class PROJECTFPS_API UInventoryItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UInventoryItemWidget(const FObjectInitializer& ObjectInitializer);

	// 아이템 데이터 설정 (스택 개수 포함)
	void SetItemData(UBaseItemData* InItemData, int32 InGridX, int32 InGridY, int32 InStackCount = 1);

protected:
	// 드래그 시작 감지
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	// 우클릭 드래그 취소
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// 소모품 사용
	void UseConsumableItem();

private:
	// 아이템 이미지 (Blueprint에서 바인딩)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemImage;

	// 스택 개수 텍스트 (Blueprint에서 바인딩, Optional)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StackCountText;

	// 아이템 데이터
	UPROPERTY()
	TObjectPtr<UBaseItemData> ItemData;

	// 스택 개수
	int32 StackCount = 1;

	// 그리드 좌표 (원래 위치 저장용)
	int32 GridX = -1;
	int32 GridY = -1;
};
