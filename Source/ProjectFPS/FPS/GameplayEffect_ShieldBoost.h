// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffect_ShieldBoost.generated.h"

/**
 * Shield 스킬 습득 시 MaxShield와 Shield를 증가시키는 GameplayEffect
 * - MaxShield +50 (영구 증가)
 * - Shield +50 (즉시 회복)
 */
UCLASS()
class PROJECTFPS_API UGameplayEffect_ShieldBoost : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_ShieldBoost();
};
