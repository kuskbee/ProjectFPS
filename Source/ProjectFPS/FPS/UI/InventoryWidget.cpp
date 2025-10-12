// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Styling/SlateBrush.h"
#include "Framework/Application/SlateApplication.h"
#include "../Components/InventoryComponent.h"
#include "../Components/WeaponSlotComponent.h"
#include "../Items/BaseItemData.h"
#include "../Items/WeaponItemData.h"
#include "../FPSPlayerCharacter.h"
#include "ItemDragDropOperation.h"
#include "InventoryItemWidget.h"
#include "WeaponSlotItemWidget.h"

UInventoryWidget::UInventoryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);  // 드래그 앤 드롭을 위한 포커스 필요
}

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Close 버튼 바인딩
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UInventoryWidget::OnCloseButtonClicked);
	}

	// 그리드 배경 테두리와 델리게이트는 InitializeInventory()에서 생성됨

	if (GridPanel)
	{
		GridPanel->SetSlotPadding(FMargin(2.f)); // 셀 사이 2px
		// 기본 셀 크기 제어(옵션)
		GridPanel->SetMinDesiredSlotWidth(64.f);
		GridPanel->SetMinDesiredSlotHeight(64.f);
	}
	
}

void UInventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 첫 프레임에 GridPanel의 실제 셀 크기 측정
	if (!bSlotSizeMeasured && GridPanel && GridPanel->GetChildrenCount() > 0)
	{
		MeasureActualSlotSize();
	}

	// 드래그 중인 경우 하이라이트 업데이트
	if (CurrentDragOperation)
	{
		// Slate의 FSlateApplication을 통해 절대 마우스 좌표 가져오기 (픽셀 단위)
		if (FSlateApplication::IsInitialized())
		{
			FVector2D ScreenMousePosition = FSlateApplication::Get().GetCursorPos();
			UpdateDragHighlight(ScreenMousePosition);
		}
	}
}

void UInventoryWidget::InitializeInventory(UInventoryComponent* InInventoryComponent, UWeaponSlotComponent* InWeaponSlotComponent)
{
	InventoryComponent = InInventoryComponent;
	WeaponSlotComponent = InWeaponSlotComponent;

	if (InventoryComponent)
	{
		// 그리드 배경 테두리 생성 (InventoryComponent가 준비된 시점)
		CreateGridBackground();

		// 델리게이트 바인딩
		InventoryComponent->OnInventoryChanged.AddDynamic(this, &UInventoryWidget::OnInventoryChanged);

		// 인벤토리 내용 표시
		RefreshInventory();
	}

	// 무기 슬롯 초기화
	if (PrimaryWeaponSlot)
	{
		PrimaryWeaponSlot->SetWeaponSlot(EWeaponSlot::Primary);
		PrimaryWeaponSlot->SetComponents(WeaponSlotComponent, InventoryComponent);
	}
	if (SecondaryWeaponSlot)
	{
		SecondaryWeaponSlot->SetWeaponSlot(EWeaponSlot::Secondary);
		SecondaryWeaponSlot->SetComponents(WeaponSlotComponent, InventoryComponent);
	}

	// 무기 슬롯 새로고침
	RefreshWeaponSlots();
}

// ========================================
// 델리게이트 핸들러
// ========================================

void UInventoryWidget::OnInventoryChanged()
{
	RefreshInventory();
}

// ========================================
// Canvas 렌더링
// ========================================

void UInventoryWidget::RefreshInventory()
{
	if (!InventoryComponent || !ItemCanvas)
	{
		return;
	}

	// 기존 아이템 이미지 모두 제거
	ClearItemCanvas();

	// 모든 아이템 순회하며 이미지 생성
	const int32 GridWidth = InventoryComponent->GetGridWidth();
	const int32 GridHeight = InventoryComponent->GetGridHeight();

	for (int32 Y = 0; Y < GridHeight; ++Y)
	{
		for (int32 X = 0; X < GridWidth; ++X)
		{
			UBaseItemData* ItemData = InventoryComponent->GetItemAt(X, Y);
			if (ItemData)
			{
				// Origin인 경우에만 이미지 생성 (중복 방지)
				int32 OriginX, OriginY;
				if (InventoryComponent->FindItemOrigin(X, Y, OriginX, OriginY) && OriginX == X && OriginY == Y)
				{
					AddItemImageToCanvas(ItemData, X, Y);
				}
			}
		}
	}
}

void UInventoryWidget::AddItemImageToCanvas(UBaseItemData* ItemData, int32 GridX, int32 GridY)
{
	if (!ItemData || !ItemCanvas || !InventoryComponent)
	{
		return;
	}

	// ItemWidgetClass가 설정되지 않은 경우 기본 이미지로 대체
	if (!ItemWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddItemImageToCanvas: ItemWidgetClass가 설정되지 않았습니다. 기본 Image 사용"));

		// 기존 방식: UImage 사용 (드래그 불가능)
		UImage* ItemImage = NewObject<UImage>(this);
		if (!ItemImage || !ItemData->ItemIcon)
		{
			return;
		}

		ItemImage->SetBrushFromTexture(ItemData->ItemIcon);
		UCanvasPanelSlot* CanvasSlot = ItemCanvas->AddChildToCanvas(ItemImage);
		if (!CanvasSlot)
		{
			return;
		}

		FIntPoint GridSize = ItemData->GetGridSize();

		// 아이템 크기/위치 계산 (GridPanel 실제 크기 사용)
		// UniformGridPanel의 SlotPadding은 모든 셀 외부에 균등하게 적용됨
		float ImageWidth = (ActualSlotWidth * GridSize.X) + (ActualSlotPaddingWidth * (GridSize.X - 1));
		float ImageHeight = (ActualSlotHeight * GridSize.Y) + (ActualSlotPaddingHeight * (GridSize.Y - 1));
		float PosX = (ActualSlotWidth + ActualSlotPaddingWidth) * GridX + ActualSlotPaddingWidth;
		float PosY = (ActualSlotHeight + ActualSlotPaddingHeight) * GridY + ActualSlotPaddingHeight;

		CanvasSlot->SetPosition(FVector2D(PosX, PosY));
		CanvasSlot->SetSize(FVector2D(ImageWidth, ImageHeight));
		CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
		return;
	}

	// InventoryItemWidget 생성 (드래그 가능)
	UInventoryItemWidget* ItemWidget = CreateWidget<UInventoryItemWidget>(GetOwningPlayer(), ItemWidgetClass);
	if (!ItemWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("AddItemImageToCanvas: ItemWidget 생성 실패"));
		return;
	}

	// 아이템 데이터 설정 (스택 개수 포함)
	int32 StackCount = InventoryComponent->GetItemStackCount(GridX, GridY);
	ItemWidget->SetItemData(ItemData, GridX, GridY, StackCount);

	// Canvas에 추가
	UCanvasPanelSlot* CanvasSlot = ItemCanvas->AddChildToCanvas(ItemWidget);
	if (!CanvasSlot)
	{
		return;
	}

	// 위치 및 크기 계산
	// GridPanel의 실제 셀 크기 사용 (ActualSlotWidth/Height)
	FIntPoint GridSize = ItemData->GetGridSize();

	// 아이템 크기: (실제 셀 크기 * 아이템 칸 수) + (실제 간격 * (칸 수 - 1))
	// UniformGridPanel의 SlotPadding은 모든 셀 외부에 균등하게 적용됨
	float ImageWidth = (ActualSlotWidth * GridSize.X) + (ActualSlotPaddingWidth * (GridSize.X - 1));
	float ImageHeight = (ActualSlotHeight * GridSize.Y) + (ActualSlotPaddingHeight * (GridSize.Y - 1));

	// 아이템 위치: (실제 셀 크기 + 실제 간격) * 좌표 + 왼쪽/위쪽 여백
	float PosX = (ActualSlotWidth + ActualSlotPaddingWidth) * GridX + ActualSlotPaddingWidth;
	float PosY = (ActualSlotHeight + ActualSlotPaddingHeight) * GridY + ActualSlotPaddingHeight;

	// Canvas Slot 설정
	CanvasSlot->SetPosition(FVector2D(PosX, PosY));
	CanvasSlot->SetSize(FVector2D(ImageWidth, ImageHeight));
	CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));  // 절대 좌표
}

void UInventoryWidget::ClearItemCanvas()
{
	if (ItemCanvas)
	{
		ItemCanvas->ClearChildren();
	}
}

// ========================================
// 드래그 앤 드롭
// ========================================

bool UInventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UItemDragDropOperation* DragOp = Cast<UItemDragDropOperation>(InOperation);
	if (!DragOp)
	{
		return false;
	}

	// 마우스 위치에 아이템 배치 시도
	FVector2D MousePosition = InDragDropEvent.GetScreenSpacePosition();
	bool bSuccess = TryPlaceItemAtMouse(InGeometry, MousePosition, DragOp);

	FinishDragDropEvent();

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("인벤토리 아이템 배치 성공"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("인벤토리 아이템 배치 실패 - 원래 위치로 복구됨"));
	}

	return bSuccess;
}

void UInventoryWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	CurrentDragOperation = Cast<UItemDragDropOperation>(InOperation);
}

void UInventoryWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	HideDragHighlight();
	CurrentDragOperation = nullptr;
}

void UInventoryWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	UItemDragDropOperation* DragOp = Cast<UItemDragDropOperation>(InOperation);
	if (!DragOp || !InventoryComponent)
	{
		return;
	}

	// 인벤토리 밖으로 드래그했거나 우클릭으로 취소한 경우
	// 원래 위치로 복구 (Grid에서 드래그한 경우만)
	if (DragOp->DragSource == EItemDragSource::Grid)
	{
		bool bRestored = InventoryComponent->PlaceItemAt(
			DragOp->DraggedItem,
			DragOp->OriginGridX,
			DragOp->OriginGridY
		);

		if (bRestored)
		{
			UE_LOG(LogTemp, Log, TEXT("드래그 취소 - 아이템을 원래 위치(%d, %d)로 복구했습니다"),
				DragOp->OriginGridX, DragOp->OriginGridY);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("드래그 취소 - 원래 위치로 복구 실패!"));
		}
	}

	HideDragHighlight();
	CurrentDragOperation = nullptr;
}

// ========================================
// 드래그 시각 피드백
// ========================================

void UInventoryWidget::UpdateDragHighlight(const FVector2D& MousePosition)
{
	if (!CurrentDragOperation || !InventoryComponent)
	{
		return;
	}

	// 1. 먼저 무기 슬롯 위에 있는지 체크 (절대 좌표 기반 AABB)
	if (PrimaryWeaponSlot)
	{
		const FGeometry& G = PrimaryWeaponSlot->GetCachedGeometry();

		// 누적 레이아웃 변환이 반영된 '절대 좌표계'의 사각형
		const FSlateRect Rect = G.GetRenderBoundingRect();

		if (Rect.ContainsPoint(MousePosition))
		{
			//UE_LOG(LogTemp, Log, TEXT("UpdateDragHighlight: Primary 무기 슬롯 영역! Mouse(%f,%f) Rect(%f,%f,%f,%f"),
			//	MousePosition.X, MousePosition.Y, Rect.Left, Rect.Top, Rect.Right, Rect.Bottom);
			UpdateWeaponSlotHighlight(MousePosition, PrimaryWeaponSlot);
			return;
		}
	}

	if (SecondaryWeaponSlot)
	{
		const FGeometry& G = SecondaryWeaponSlot->GetCachedGeometry();

		// 누적 레이아웃 변환이 반영된 '절대 좌표계'의 사각형
		const FSlateRect Rect = G.GetRenderBoundingRect();

		if (Rect.ContainsPoint(MousePosition))
		{
			//UE_LOG(LogTemp, Log, TEXT("UpdateDragHighlight: Secondary 무기 슬롯 영역! Mouse(%f,%f) Rect(%f,%f,%f,%f"),
			//	MousePosition.X, MousePosition.Y, Rect.Left, Rect.Top, Rect.Right, Rect.Bottom);
			UpdateWeaponSlotHighlight(MousePosition, SecondaryWeaponSlot);
			return;
		}
	}

	// 2. 그리드 영역에서 하이라이트 표시
	// 마우스 위치에서 그리드 좌표 계산
	FIntPoint GridPos = GetGridPosFromMouse(MousePosition);

	if (GridPos.X < 0 || GridPos.Y < 0)
	{
		UE_LOG(LogTemp, Log, TEXT("UpdateDragHighlight: 마우스 좌표가 영역 밖!"));
		return;
	}

	// 배치 가능 여부 체크
	bool bCanPlace = InventoryComponent->CanPlaceItemAt(CurrentDragOperation->DraggedItem, GridPos.X, GridPos.Y);

	// 하이라이트 이미지 생성 (Blueprint에서 추가 가능)
	if (DragHighlightImage)
	{
		// 초록(배치 가능) / 빨강(배치 불가능) 색상 변경
		FLinearColor HighlightColor = bCanPlace ? FLinearColor::Green : FLinearColor::Red;
		HighlightColor.A = 0.5f;  // 반투명
		DragHighlightImage->SetColorAndOpacity(HighlightColor);

		// 위치 및 크기 설정
		if (UCanvasPanelSlot* HighlightSlot = Cast<UCanvasPanelSlot>(DragHighlightImage->Slot))
		{
			FIntPoint ItemSize = CurrentDragOperation->DraggedItem->GetGridSize();

			UWidget* HoverCell = GridPanel->GetChildAt(GridPos.X + GridPos.Y * InventoryComponent->GetGridWidth());
			if (!HoverCell)
			{
				UE_LOG(LogTemp, Log, TEXT("UpdateDragHighlight: GridPanel GetChildAt 실패! (%d, %d)"), GridPos.X, GridPos.Y);
				return;
			}
			const FGeometry& G = HoverCell->GetCachedGeometry();
			FVector2D HighlightSize = G.GetLocalSize();
			FVector2D SlotScreenPosition = G.GetAbsolutePosition();

			// InventoryWidget의 Geometry를 기준으로 로컬 좌표로 변환
			FVector2D LocalPosition = GetCachedGeometry().AbsoluteToLocal(SlotScreenPosition);

			// 실제 셀 크기와 실제 패딩으로 좌표 계산
			float Width = (HighlightSize.X * ItemSize.X) + (ActualSlotPaddingWidth * (ItemSize.X + 1));
			float Height = (HighlightSize.Y * ItemSize.Y) + (ActualSlotPaddingHeight * (ItemSize.Y + 1));
			float PosX = LocalPosition.X;
			float PosY = LocalPosition.Y;

			HighlightSlot->SetPosition(FVector2D(PosX, PosY));
			HighlightSlot->SetSize(FVector2D(Width, Height));
			HighlightSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));  // 절대 좌표
		}

		DragHighlightImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UInventoryWidget::UpdateWeaponSlotHighlight(const FVector2D& MousePosition, UWeaponSlotItemWidget* SlotWidget)
{
	if (!DragHighlightImage || !SlotWidget || !CurrentDragOperation || !WeaponSlotComponent)
	{
		return;
	}

	// WeaponItemData만 무기 슬롯에 장착 가능
	UWeaponItemData* WeaponData = Cast<UWeaponItemData>(CurrentDragOperation->DraggedItem);
	EWeaponSlot HoverSlot = SlotWidget->GetWeaponSlot();
	bool bCanEquip = (WeaponData != nullptr) && WeaponSlotComponent->IsSlotEmpty(HoverSlot);

	// 초록(장착 가능) / 빨강(장착 불가능) 색상 변경
	FLinearColor HighlightColor = bCanEquip ? FLinearColor::Green : FLinearColor::Red;
	HighlightColor.A = 0.5f;  // 반투명
	DragHighlightImage->SetColorAndOpacity(HighlightColor);

	// 무기 슬롯의 Geometry를 기반으로 하이라이트 위치 및 크기 설정
	const FGeometry& SlotGeometry = SlotWidget->GetCachedGeometry();
	FVector2D HighlightSize = SlotGeometry.GetLocalSize();
	FVector2D SlotScreenPosition = SlotGeometry.GetAbsolutePosition();

	// InventoryWidget의 Geometry를 기준으로 로컬 좌표로 변환
	FVector2D LocalPosition = GetCachedGeometry().AbsoluteToLocal(SlotScreenPosition);

	if (UCanvasPanelSlot* HighlightSlot = Cast<UCanvasPanelSlot>(DragHighlightImage->Slot))
	{
		HighlightSlot->SetPosition(LocalPosition);
		HighlightSlot->SetSize(HighlightSize);
		HighlightSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));  // 절대 좌표
	}

	DragHighlightImage->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UInventoryWidget::HideDragHighlight()
{
	if (DragHighlightImage)
	{
		DragHighlightImage->SetVisibility(ESlateVisibility::Hidden);
	}
}

FIntPoint UInventoryWidget::GetGridPosFromMouse(const FVector2D& MousePosition) const
{
	UE_LOG(LogTemp, Log, TEXT("GetGridPosFromMouse Mouse(%f, %f)"), MousePosition.X, MousePosition.Y);

	if (!GridPanel || !InventoryComponent) return FIntPoint(-1,-1);

	// 그리드 패널의 Geometry 확보
	const FGeometry& GridGeo = GridPanel->GetCachedGeometry();

	// 마우스가 그리드 영역 위에 있는지
	if (!GridGeo.IsUnderLocation(MousePosition)) return FIntPoint(-1, -1);

	// 절대 → 로컬
	FVector2D LocalPosition = GridGeo.AbsoluteToLocal(MousePosition);

	// 그리드 좌표 계산 (ActualSlotWidth/Height + ActualSlotPadding 간격 고려)
	int32 GridX = FMath::FloorToInt(LocalPosition.X / (ActualSlotWidth + ActualSlotPaddingWidth));
	int32 GridY = FMath::FloorToInt(LocalPosition.Y / (ActualSlotHeight + ActualSlotPaddingHeight));

	// 범위 클램핑
	if (InventoryComponent)
	{
		GridX = FMath::Clamp(GridX, 0, InventoryComponent->GetGridWidth() - 1);
		GridY = FMath::Clamp(GridY, 0, InventoryComponent->GetGridHeight() - 1);
	}

	UE_LOG(LogTemp, Log, TEXT("GetGridPosFromMouse Grid(%d, %d)"), GridX, GridY);
	return FIntPoint(GridX, GridY);
}

bool UInventoryWidget::TryPlaceItemAtMouse(const FGeometry& MyGeometry, const FVector2D& MousePosition, UItemDragDropOperation* DragOp)
{
	if (!InventoryComponent || !DragOp || !DragOp->DraggedItem)
	{
		return false;
	}

	// 마우스 위치에서 그리드 좌표 계산
	FIntPoint GridPos = GetGridPosFromMouse(MousePosition);

	// 배치 가능 여부 먼저 체크
	bool bCanPlace = false;

	if (DragOp->DragSource == EItemDragSource::Grid)
	{
		// Grid → Grid: 원래 위치 제외하고 배치 가능한지 체크
		InventoryComponent->RemoveItemAt(DragOp->OriginGridX, DragOp->OriginGridY);
		bCanPlace = InventoryComponent->CanPlaceItemAt(DragOp->DraggedItem, GridPos.X, GridPos.Y);
	}
	else if (DragOp->DragSource == EItemDragSource::WeaponSlot)
	{
		// WeaponSlot → Grid: 그냥 배치 가능한지만 체크
		bCanPlace = InventoryComponent->CanPlaceItemAt(DragOp->DraggedItem, GridPos.X, GridPos.Y);
	}

	// 배치 불가능하면 원래 위치 복구 (Grid의 경우만)
	if (!bCanPlace)
	{
		if (DragOp->DragSource == EItemDragSource::Grid)
		{
			InventoryComponent->PlaceItemAt(DragOp->DraggedItem, DragOp->OriginGridX, DragOp->OriginGridY);
		}
		// WeaponSlot의 경우 아무것도 안함 (장착 상태 유지)
		return false;
	}

	// 배치 가능하면 실제 배치
	bool bSuccess = InventoryComponent->PlaceItemAt(DragOp->DraggedItem, GridPos.X, GridPos.Y);

	// 성공 시 원래 위치 처리
	if (bSuccess && DragOp->DragSource == EItemDragSource::WeaponSlot && WeaponSlotComponent)
	{
		// 무기 슬롯에서 해제
		WeaponSlotComponent->UnequipWeaponFromSlot(DragOp->OriginWeaponSlot);
		RefreshWeaponSlots();
		UE_LOG(LogTemp, Log, TEXT("무기 슬롯 → 그리드 배치 성공"));
	}

	return bSuccess;
}

// ========================================
// 그리드 배경 생성
// ========================================

void UInventoryWidget::CreateGridBackground()
{
	if (!GridPanel)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGridBackground: GridPanel이 nullptr입니다! BindWidget 확인 필요"));
		return;
	}

	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGridBackground: InventoryComponent가 nullptr입니다!"));
		return;
	}

	// 그리드 크기
	const int32 GridWidth = InventoryComponent->GetGridWidth();
	const int32 GridHeight = InventoryComponent->GetGridHeight();

	UE_LOG(LogTemp, Warning, TEXT("CreateGridBackground 시작: %dx%d 그리드 생성"), GridWidth, GridHeight);

	// 8x6 = 48개의 Border 생성
	for (int32 Y = 0; Y < GridHeight; ++Y)
	{
		for (int32 X = 0; X < GridWidth; ++X)
		{
			// Border 위젯 생성 (그리드 한 칸)
			UBorder* GridCellBorder = NewObject<UBorder>(this);
			if (!GridCellBorder)
			{
				UE_LOG(LogTemp, Error, TEXT("GridCellBorder 생성 실패: (%d, %d)"), X, Y);
				continue;
			}

			// Border 배경색 설정 (어두운 회색, 약간 투명)
			GridCellBorder->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.5f));

			// Border 패딩 설정 (테두리처럼 보이게)
			GridCellBorder->SetPadding(FMargin(10.0f));

			// UniformGridPanel에 추가
			UUniformGridSlot* GridSlot = GridPanel->AddChildToUniformGrid(GridCellBorder, Y, X);
			if (GridSlot)
			{
				GridSlot->SetHorizontalAlignment(HAlign_Fill);
				GridSlot->SetVerticalAlignment(VAlign_Fill);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("GridSlot 생성 실패: (%d, %d)"), X, Y);
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("CreateGridBackground 완료: %dx%d = %d개 Border 생성"), GridWidth, GridHeight, GridWidth * GridHeight);
}

void UInventoryWidget::MeasureActualSlotSize()
{
	if (!GridPanel || !InventoryComponent)
	{
		return;
	}

	const int32 ExpectedChildCount = InventoryComponent->GetGridWidth() * InventoryComponent->GetGridHeight();
	if (GridPanel->GetChildrenCount() != ExpectedChildCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("MeasureActualSlotSize: 자식 개수 불일치 (현재: %d, 예상: %d)"),
			GridPanel->GetChildrenCount(), ExpectedChildCount);
		return;
	}

	// GridPanel 전체 크기 가져오기
	FVector2D GridPanelSize = GridPanel->GetCachedGeometry().GetLocalSize();

	// 크기가 유효하지 않으면 다음 프레임에 다시 시도
	if (GridPanelSize.X <= 0.0f || GridPanelSize.Y <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("MeasureActualSlotSize: GridPanel 크기가 아직 유효하지 않음 (%.2f x %.2f)"),
			GridPanelSize.X, GridPanelSize.Y);
		return;
	}

	// 첫 번째 셀의 크기 가져오기
	UWidget* FirstCell = GridPanel->GetChildAt(0);
	if (!FirstCell)
	{
		UE_LOG(LogTemp, Error, TEXT("MeasureActualSlotSize: 첫 번째 셀을 가져올 수 없음"));
		return;
	}

	FVector2D CellSize = FirstCell->GetCachedGeometry().GetLocalSize();

	// Geometry가 아직 유효하지 않으면 다음 프레임에 다시 시도
	if (CellSize.X <= 0.0f || CellSize.Y <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("MeasureActualSlotSize: 셀 크기가 아직 유효하지 않음 (%.2f x %.2f)"),
			CellSize.X, CellSize.Y);
		return;
	}

	const int32 GridWidth = InventoryComponent->GetGridWidth();
	const int32 GridHeight = InventoryComponent->GetGridHeight();

	// 실제 셀 크기
	ActualSlotWidth = CellSize.X;
	ActualSlotHeight = CellSize.Y;

	// 패딩 역계산: GridPanelSize = (CellSize * Count) + (Padding * (Count + 1))
	// => Padding = (GridPanelSize - CellSize * Count) / (Count + 1)
	// UniformGridPanel의 SlotPadding은 왼쪽/오른쪽 끝에도 적용되므로 (Count + 1)개
	ActualSlotPaddingWidth = (GridPanelSize.X - ActualSlotWidth * GridWidth) / (GridWidth + 1);
	ActualSlotPaddingHeight = (GridPanelSize.Y - ActualSlotHeight * GridHeight) / (GridHeight + 1);

	// 유효성 검증
	if (ActualSlotWidth <= 0.0f || ActualSlotHeight <= 0.0f ||
		ActualSlotPaddingWidth < 0.0f || ActualSlotPaddingHeight < 0.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("MeasureActualSlotSize: 계산된 값이 잘못됨 (셀: %.2f x %.2f, 패딩: %.2f x %.2f)"),
			ActualSlotWidth, ActualSlotHeight, ActualSlotPaddingWidth, ActualSlotPaddingHeight);
		return;
	}

	bSlotSizeMeasured = true;

	UE_LOG(LogTemp, Warning, TEXT("MeasureActualSlotSize: GridPanel 크기 = %.2f x %.2f"),
		GridPanelSize.X, GridPanelSize.Y);
	UE_LOG(LogTemp, Warning, TEXT("MeasureActualSlotSize: 계산된 셀 크기 = %.2f x %.2f (기본값: %.2f)"),
		ActualSlotWidth, ActualSlotHeight, SlotSize);
	UE_LOG(LogTemp, Warning, TEXT("MeasureActualSlotSize: 계산된 패딩 = %.2f x %.2f (기본값: %.2f)"),
		ActualSlotPaddingWidth, ActualSlotPaddingHeight, SlotPadding);

	// 셀 크기가 측정되었으므로 인벤토리 다시 그리기
	RefreshInventory();
}

void UInventoryWidget::OnCloseButtonClicked()
{
	// PlayerController 가져오기
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		// Controller 없으면 그냥 UI만 닫기
		RemoveFromParent();
		return;
	}

	// I키 입력 시뮬레이션 (ToggleInventory 재호출)
	// 이렇게 하면 PlayerCharacter의 상태 정리 로직이 자동으로 실행됨
	if (AFPSPlayerCharacter* PlayerCharacter = Cast<AFPSPlayerCharacter>(PC->GetPawn()))
	{
		// ToggleInventory() 호출로 상태 정리
		PlayerCharacter->ToggleInventory(FInputActionValue());
	}
	else
	{
		// FPSPlayerCharacter가 아니면 수동으로 정리
		RemoveFromParent();
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());
	}
}

// ========================================
// 무기 슬롯 UI 갱신
// ========================================

void UInventoryWidget::RefreshWeaponSlots()
{
	UE_LOG(LogTemp, Log, TEXT("RefreshWeaponSlots:"));
	if (!WeaponSlotComponent)
	{
		return;
	}

	// Primary 슬롯 갱신
	if (PrimaryWeaponSlot)
	{
		UWeaponItemData* PrimaryWeapon = WeaponSlotComponent->GetWeaponInSlot(EWeaponSlot::Primary);
		if (PrimaryWeapon)
		{
			UE_LOG(LogTemp, Log, TEXT("RefreshWeaponSlots: PrimaryWeaponSlot SetWeaponIcon"));
			PrimaryWeaponSlot->SetWeaponIcon(PrimaryWeapon);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("RefreshWeaponSlots: PrimaryWeaponSlot ClearWeaponIcon"));
			PrimaryWeaponSlot->ClearWeaponIcon();
		}
	}

	// Secondary 슬롯 갱신
	if (SecondaryWeaponSlot)
	{
		UWeaponItemData* SecondaryWeapon = WeaponSlotComponent->GetWeaponInSlot(EWeaponSlot::Secondary);
		if (SecondaryWeapon)
		{
			UE_LOG(LogTemp, Log, TEXT("RefreshWeaponSlots: SecondaryWeapon SetWeaponIcon"));
			SecondaryWeaponSlot->SetWeaponIcon(SecondaryWeapon);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("RefreshWeaponSlots: SecondaryWeapon ClearWeaponIcon"));
			SecondaryWeaponSlot->ClearWeaponIcon();
		}
	}
}

void UInventoryWidget::FinishDragDropEvent()
{
	// 하이라이트 숨기기
	HideDragHighlight();
	CurrentDragOperation = nullptr;
}

