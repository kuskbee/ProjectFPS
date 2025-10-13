// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryItemWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "../Items/BaseItemData.h"
#include "../Items/ConsumableItemData.h"
#include "ItemDragDropOperation.h"
#include "../FPSPlayerCharacter.h"
#include "../GameplayAbility_UseConsumable.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayTags.h"
#include "ToastManagerWidget.h"
#include "../CharacterAttributeSet.h"
#include "../GameplayEffect_InstantHeal.h"

UInventoryItemWidget::UInventoryItemWidget(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UInventoryItemWidget::SetItemData(UBaseItemData *InItemData, int32 InGridX, int32 InGridY, int32 InStackCount)
{
	ItemData = InItemData;
	GridX = InGridX;
	GridY = InGridY;
	StackCount = InStackCount;

	// 아이템 이미지 설정
	if (ItemImage && ItemData && ItemData->ItemIcon)
	{
		ItemImage->SetBrushFromTexture(ItemData->ItemIcon);
	}

	// 스택 개수 텍스트 설정 (스택 가능한 아이템만)
	if (StackCountText)
	{
		if (ItemData && ItemData->IsStackable() && StackCount > 1)
		{
			// "x10" 형태로 표시
			FString StackText = FString::Printf(TEXT("x%d"), StackCount);
			StackCountText->SetText(FText::FromString(StackText));
			StackCountText->SetVisibility(ESlateVisibility::HitTestInvisible);

			UE_LOG(LogTemp, Log, TEXT("스택 텍스트 설정: %s (StackCount: %d)"), *StackText, StackCount);
		}
		else
		{
			// 스택 불가능하거나 1개면 숨김
			StackCountText->SetVisibility(ESlateVisibility::Collapsed);

			if (ItemData)
			{
				UE_LOG(LogTemp, Log, TEXT("스택 텍스트 숨김: %s (Stackable: %s, Count: %d)"),
					*ItemData->GetItemName(),
					ItemData->IsStackable() ? TEXT("Yes") : TEXT("No"),
					StackCount);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("StackCountText가 null입니다. Blueprint에서 바인딩 확인 필요"));
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

	// 우클릭: 소모품 사용
	if (InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		UseConsumableItem();
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

	//// 드래그 비주얼 설정 (아이템 이미지를 따라다니게)
	//DragOp->DefaultDragVisual = this;
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

void UInventoryItemWidget::UseConsumableItem()
{
	// 소모품인지 체크
	UConsumableItemData* ConsumableData = Cast<UConsumableItemData>(ItemData);
	if (!ConsumableData)
	{
		UE_LOG(LogTemp, Warning, TEXT("UseConsumableItem: 소모품이 아닙니다."));
		return;
	}

	// Owner 확인
	APawn* OwnerPawn = GetOwningPlayerPawn();
	AFPSPlayerCharacter* PlayerCharacter = Cast<AFPSPlayerCharacter>(OwnerPawn);
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("UseConsumableItem: PlayerCharacter를 찾을 수 없습니다."));
		return;
	}

	UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("UseConsumableItem: AbilitySystemComponent를 찾을 수 없습니다."));
		return;
	}

	// ConsumableEffect가 있는지 체크
	if (!ConsumableData->ConsumableEffect)
	{
		UE_LOG(LogTemp, Warning, TEXT("UseConsumableItem: ConsumableEffect가 설정되지 않았습니다."));
		return;
	}

	// "Ability.UseConsumable" 태그를 가진 Ability 찾기
	FGameplayTag UseConsumableTag = FGameplayTag::RequestGameplayTag(FName("Ability.UseConsumable"));
	FGameplayAbilitySpec* FoundSpec = nullptr;

	for (FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.Ability && Spec.Ability->GetAssetTags().HasTag(UseConsumableTag))
		{
			FoundSpec = &Spec;
			break;
		}
	}

	if (!FoundSpec)
	{
		UE_LOG(LogTemp, Error, TEXT("UseConsumableItem: UseConsumable Ability를 찾을 수 없습니다. DefaultAbilities에 추가했는지 확인하세요."));
		return;
	}

	// Ability 인스턴스 가져오기 (InstancedPerActor이므로 인스턴스가 존재함)
	UGameplayAbility_UseConsumable* UseConsumableAbility = Cast<UGameplayAbility_UseConsumable>(FoundSpec->GetPrimaryInstance());
	if (!UseConsumableAbility)
	{
		UE_LOG(LogTemp, Error, TEXT("UseConsumableItem: Ability 인스턴스를 가져올 수 없습니다."));
		return;
	}

	// 소모품 데이터 설정
	UseConsumableAbility->SetConsumableData(ConsumableData, GridX, GridY);

	// Tag 기반 활성화
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(UseConsumableTag);

	bool bSuccess = ASC->TryActivateAbilitiesByTag(TagContainer);
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("포션 사용: %s"), *ConsumableData->GetItemName());
	}
	else
	{
		// Ability 활성화 실패 (CanActivateAbility에서 false 반환)
		// 체력이 꽉 찬 경우 Toast 메시지 표시
		UToastManagerWidget* ToastManager = PlayerCharacter->ToastManagerWidget;
		if (ToastManager)
		{
			// ConsumableEffect가 GameplayEffect_InstantHeal인지 체크 (IsA 관계)
			if (ConsumableData->ConsumableEffect && ConsumableData->ConsumableEffect->IsChildOf(UGameplayEffect_InstantHeal::StaticClass()))
			{
				const float CurrentHealth = ASC->GetNumericAttribute(UCharacterAttributeSet::GetHealthAttribute());
				const float MaxHealth = ASC->GetNumericAttribute(UCharacterAttributeSet::GetMaxHealthAttribute());

				if (CurrentHealth >= MaxHealth)
				{
					ToastManager->ShowToast(TEXT("체력이 가득 차서 포션을 사용할 수 없습니다"), 2.0f);
				}
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("UseConsumableItem: Ability 활성화 실패"));
	}
}
