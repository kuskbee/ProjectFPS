// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffect_MaxStaminaBoost.generated.h"

/**
 * MaxStamina +50 증가
 */
UCLASS()
class PROJECTFPS_API UGameplayEffect_MaxStaminaBoost : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_MaxStaminaBoost();
};
