// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbility_UseConsumable.generated.h"

/**
 * 소모품 사용 Ability
 * 인벤토리에서 아이템을 소모하고 GameplayEffect를 적용
 */
UCLASS()
class PROJECTFPS_API UGameplayAbility_UseConsumable : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_UseConsumable();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	                              const FGameplayAbilityActorInfo* ActorInfo,
	                              const FGameplayAbilityActivationInfo ActivationInfo,
	                              const FGameplayEventData* TriggerEventData) override;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	                                const FGameplayAbilityActorInfo* ActorInfo,
	                                const FGameplayTagContainer* SourceTags,
	                                const FGameplayTagContainer* TargetTags,
	                                FGameplayTagContainer* OptionalRelevantTags) const override;

public:
	/** 소모품 데이터와 그리드 좌표를 설정 (활성화 전 호출) */
	UFUNCTION(BlueprintCallable, Category = "Consumable")
	void SetConsumableData(class UConsumableItemData* InConsumableData, int32 InGridX, int32 InGridY);

private:
	/** 사용할 소모품 데이터 */
	UPROPERTY()
	TObjectPtr<class UConsumableItemData> ConsumableData;

	/** 인벤토리 그리드 좌표 (아이템 제거용) */
	int32 GridX = -1;
	int32 GridY = -1;
};
