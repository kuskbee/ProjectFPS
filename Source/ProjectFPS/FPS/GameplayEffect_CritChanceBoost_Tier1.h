// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffect_CritChanceBoost_Tier1.generated.h"

/**
 * Critical Chance 1단계 스킬 습득 시 CritChance를 증가시키는 GameplayEffect
 * - CritChance +0.05 (5% 영구 증가)
 */
UCLASS()
class PROJECTFPS_API UGameplayEffect_CritChanceBoost_Tier1 : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_CritChanceBoost_Tier1();
};