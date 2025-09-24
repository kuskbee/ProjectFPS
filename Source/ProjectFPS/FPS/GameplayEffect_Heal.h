// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffect_Heal.generated.h"

/**
 * 체력 회복을 위한 GameplayEffect
 * OnPlayerRespawn에서 사용하여 체력을 완전 회복시킴
 */
UCLASS()
class PROJECTFPS_API UGameplayEffect_Heal : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_Heal();
};