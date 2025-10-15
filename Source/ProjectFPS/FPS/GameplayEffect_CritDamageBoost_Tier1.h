// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffect_CritDamageBoost_Tier1.generated.h"

/**
 * Critical Damage 1단계 스킬 습득 시 CritDamage를 증가시키는 GameplayEffect
 * - CritDamage +0.5 (기본 1.5 → 2.0, 즉 200% 크리티컬 데미지)
 */
UCLASS()
class PROJECTFPS_API UGameplayEffect_CritDamageBoost_Tier1 : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_CritDamageBoost_Tier1();
};
