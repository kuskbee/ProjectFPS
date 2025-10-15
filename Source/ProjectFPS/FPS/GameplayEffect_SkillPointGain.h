// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffect_SkillPointGain.generated.h"

/**
 * UGameplayEffect_SkillPointGain
 *
 * 스킬 포인트를 증가시키는 GameplayEffect (Enemy 처치 보상)
 * - Duration: Instant (즉시 적용)
 * - Modifier: SkillPoint Additive
 * - SetByCaller: "Data.SkillPointGain" 태그로 동적 보상량 설정
 */
UCLASS()
class PROJECTFPS_API UGameplayEffect_SkillPointGain : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_SkillPointGain();
};
