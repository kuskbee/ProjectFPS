// GameplayEffect_Cooldown.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffect_Cooldown.generated.h"

/**
 * 쿨다운 GameplayEffect
 * - Duration 기반 쿨다운 시스템
 * - Tag: Cooldown.ActiveSkill
 * - 사용 예: 60초 쿨다운
 */
UCLASS()
class PROJECTFPS_API UGameplayEffect_Cooldown : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_Cooldown();
};
