// GameplayEffect_SprintSpeedBoost.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffect_SprintSpeedBoost.generated.h"

/**
 * Sprint 시 이동속도 증가 GameplayEffect
 *
 * GAS 학습 포인트:
 * 1. Duration Policy: Infinite (Sprint가 끝날 때까지 지속)
 * 2. Modifier: MoveSpeedMultiplier +1.0 (1.0 → 2.0, 즉 2배 증가)
 * 3. Sprint Ability에서 적용/제거
 *
 * 장점:
 * - Berserker와 Sprint가 동시에 활성화되어도 충돌 없음
 * - MoveSpeedMultiplier가 자동으로 곱셈 적용
 * - Effect 제거 시 자동으로 이전 값으로 복구
 */
UCLASS()
class PROJECTFPS_API UGameplayEffect_SprintSpeedBoost : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_SprintSpeedBoost();
};
