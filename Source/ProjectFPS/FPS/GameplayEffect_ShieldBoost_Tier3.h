// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffect_ShieldBoost_Tier3.generated.h"

/**
 * Shield Tier 3: MaxShield +150, Shield +150
 */
UCLASS()
class PROJECTFPS_API UGameplayEffect_ShieldBoost_Tier3 : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_ShieldBoost_Tier3();
};
