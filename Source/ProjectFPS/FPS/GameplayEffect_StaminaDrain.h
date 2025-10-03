// GameplayEffect_StaminaDrain.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffect_StaminaDrain.generated.h"

/**
 * 질주 중 주기적으로 스태미나를 감소시키는 GameplayEffect
 *
 * GAS 학습 포인트:
 * 1. Duration Policy: HasDuration (일정 시간 동안 지속)
 * 2. Periodic: Period = 0.1초마다 실행 (10FPS)
 * 3. Modifier: Stamina를 -5씩 감소 (초당 50 감소)
 * 4. ExecutePeriodicEffectOnApplication: 즉시 첫 번째 소모 적용
 *
 * 사용 예시:
 * - Sprint Ability 활성화 시 적용
 * - Sprint Ability 종료 시 제거
 */
UCLASS()
class PROJECTFPS_API UGameplayEffect_StaminaDrain : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_StaminaDrain();
};
