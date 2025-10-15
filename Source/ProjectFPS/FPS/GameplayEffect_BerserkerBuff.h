// GameplayEffect_BerserkerBuff.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffect_BerserkerBuff.generated.h"

/**
 * 버서커 스킬 버프 GameplayEffect
 *
 * GAS 학습 포인트:
 * 1. Duration Policy: HasDuration (10초 동안 지속)
 * 2. Modifier:
 *    - 공격속도 증가: FireRate Multiply 설정 (값은 Blueprint에서 조정)
 *    - 이동속도 증가: MaxWalkSpeed Multiply 설정 (값은 Blueprint에서 조정)
 * 3. 일반적으로 공격속도 +50% = FireRate를 0.67배로 감소 (연사 간격 감소)
 * 4. 이동속도 +30% = MaxWalkSpeed를 1.3배로 증가
 *
 * 사용 예시:
 * - Berserker Ability 활성화 시 적용
 * - Ability 종료 시 자동 제거
 */
UCLASS()
class PROJECTFPS_API UGameplayEffect_BerserkerBuff : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGameplayEffect_BerserkerBuff();
};
