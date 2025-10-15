// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EWeaponSlot.h"
#include "WeaponSlotItemWidget.generated.h"

class UImage;
class UTextBlock;
class UWeaponItemData;
class UItemDragDropOperation;
class UWeaponSlotComponent;
class UInventoryComponent;
class UInventoryWidget;

/**
 * 무기 슬롯 UI 위젯 (Primary/Secondary)
 * - 드래그 앤 드롭 가능
 * - 현재 장착된 무기 아이콘 표시
 */
UCLASS()
class PROJECTFPS_API UWeaponSlotItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 무기 슬롯 초기화
	void SetWeaponSlot(EWeaponSlot InSlotType);

	// 컴포넌트 참조 설정
	void SetComponents(UWeaponSlotComponent* InWeaponSlotComponent, UInventoryComponent* InInventoryComponent);

	// 무기 아이콘 설정
	void SetWeaponIcon(UWeaponItemData* WeaponData);

	// 무기 아이콘 제거
	void ClearWeaponIcon();

	// 슬롯 타입 가져오기
	EWeaponSlot GetWeaponSlot() const { return SlotType; }

protected:
	virtual void NativeConstruct() override;

	// 드래그 시작
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	// 드롭 처리
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	// UI 위젯들 (Blueprint에서 바인딩)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> WeaponIconImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SlotNameText;

	// 슬롯 타입 (Primary/Secondary)
	EWeaponSlot SlotType = EWeaponSlot::None;

	// 현재 표시 중인 무기 데이터
	UPROPERTY()
	TObjectPtr<UWeaponItemData> CurrentWeaponData;

	// 빈 슬롯 배경 텍스처 (Blueprint에서 설정)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Slot", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> EmptySlotTexture;

	// 컴포넌트 참조
	UPROPERTY()
	TObjectPtr<UWeaponSlotComponent> WeaponSlotComponent;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> InventoryComponent;
};
