// GameplayEffect_StaminaRecover.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffect_StaminaRecover.generated.h"

/**
 * 질주 종료 후 스태미나를 점진적으로 회복시키는 GameplayEffect
 *
 * GAS 학습 포인트:
 * 1. Duration Policy: Infinite (무한 지속, Sprint 시작 시 제거됨)
 * 2. Periodic: Period = 0.1초마다 실행 (10FPS)
 * 3. Modifier: Stamina를 +3씩 증가 (초당 30 회복)
 * 4. PreAttributeChange에서 자동으로 MaxStamina 초과 방지
 *
 * 사용 예시:
 * - Sprint Ability 종료 시 적용
 * - Sprint Ability 시작 시 제거
 */
UCLASS()
class PROJECTFPS_API UGameplayEffect_StaminaRecover : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_StaminaRecover();
};
