// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

class UCanvasPanel;
class UUniformGridPanel;
class UImage;
class UInventoryComponent;
class UBaseItemData;
class UItemDragDropOperation;
class UInventoryItemWidget;
class UButton;

/**
 * 디아블로2 스타일 인벤토리 UI
 * - Canvas 기반 가변 크기 아이템 렌더링
 * - 드래그 앤 드롭 시각 피드백
 */
UCLASS()
class PROJECTFPS_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UInventoryWidget(const FObjectInitializer& ObjectInitializer);

	// 초기화
	void InitializeInventory(UInventoryComponent* InInventoryComponent);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 드래그 앤 드롭
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	// 인벤토리 컴포넌트 참조
	UPROPERTY()
	TObjectPtr<UInventoryComponent> InventoryComponent;

	// UI 위젯들 (Blueprint에서 바인딩)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> GridPanel;  // 배경 그리드

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> ItemCanvas;  // 아이템 이미지들

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

	// 그리드 설정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	float SlotSize = 64.0f;  // 한 칸의 기본 크기 (64x64 픽셀, 초기값)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	float SlotPadding = 2.0f;  // 칸 간격

	// 런타임에 측정된 실제 셀 크기 (GridPanel이 Fill로 확장되면 달라질 수 있음)
	float ActualSlotWidth = 64.0f;
	float ActualSlotHeight = 64.0f;
	float ActualSlotPaddingWidth = 2.0f;   // 런타임에 측정된 실제 가로 패딩
	float ActualSlotPaddingHeight = 2.0f;  // 런타임에 측정된 실제 세로 패딩
	bool bSlotSizeMeasured = false;  // 셀 크기 측정 완료 여부

	// InventoryItemWidget 클래스 (Blueprint에서 설정)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UInventoryItemWidget> ItemWidgetClass;

	// 드래그 하이라이트
	UPROPERTY()
	TObjectPtr<UImage> DragHighlightImage;  // 초록/빨강 테두리

	UPROPERTY()
	TObjectPtr<UItemDragDropOperation> CurrentDragOperation;  // 현재 드래그 중인 Operation

	// 델리게이트 핸들러
	UFUNCTION()
	void OnInventoryChanged();

	// 그리드 배경 생성
	void CreateGridBackground();

	// 그리드 셀 크기 측정 (런타임)
	void MeasureActualSlotSize();

	// Canvas 렌더링
	void RefreshInventory();
	void AddItemImageToCanvas(UBaseItemData* ItemData, int32 GridX, int32 GridY);
	void ClearItemCanvas();

	// 드래그 시각 피드백
	void UpdateDragHighlight(const FGeometry& MyGeometry, const FVector2D& MousePosition);
	void HideDragHighlight();
	FIntPoint GetGridPosFromMouse(const FGeometry& MyGeometry, const FVector2D& MousePosition) const;

	// 아이템 배치 시도
	bool TryPlaceItemAtMouse(const FGeometry& MyGeometry, const FVector2D& MousePosition, UItemDragDropOperation* DragOp);

	// Close 버튼 클릭 핸들러
	UFUNCTION()
	void OnCloseButtonClicked();
};
