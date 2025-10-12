// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffect_InstantHeal.generated.h"

/**
 * 즉시 체력 회복 GameplayEffect
 * SetByCaller 방식으로 회복량을 동적으로 설정 (Data.HealAmount)
 * 포션 등 소모품에서 사용
 */
UCLASS()
class PROJECTFPS_API UGameplayEffect_InstantHeal : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_InstantHeal();
};
