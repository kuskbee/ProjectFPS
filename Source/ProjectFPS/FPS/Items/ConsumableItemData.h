// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseItemData.h"
#include "ConsumableItemData.generated.h"

/**
 * 소모품 아이템 데이터 (포션, 회복 아이템 등)
 */
UCLASS(BlueprintType)
class PROJECTFPS_API UConsumableItemData : public UBaseItemData
{
	GENERATED_BODY()

public:
	UConsumableItemData();

	// ==================== 소모품 속성 ====================

	/** 사용 시 적용할 GameplayEffect (체력 회복, 버프 등) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable")
	TSubclassOf<class UGameplayEffect> ConsumableEffect;

	/** 효과 크기 (회복량, 버프 강도 등) - SetByCaller로 전달 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable", meta = (ClampMin = 0.0f))
	float EffectMagnitude = 50.0f;

	/** 즉시 소모 여부 (true: 픽업 시 즉시 사용, false: 인벤토리에 저장) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable")
	bool bConsumeOnPickup = false;

	// MaxStackSize는 BaseItemData에 이미 정의되어 있음 (상속받음)
};
