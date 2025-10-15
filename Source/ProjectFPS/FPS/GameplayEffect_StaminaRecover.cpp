// GameplayEffect_StaminaRecover.cpp

#include "GameplayEffect_StaminaRecover.h"
#include "CharacterAttributeSet.h"

UGameplayEffect_StaminaRecover::UGameplayEffect_StaminaRecover()
{
	// 1. Duration Policy: 무한 지속 (Sprint 시작 시 제거됨)
	DurationPolicy = EGameplayEffectDurationType::Infinite;

	// 2. Periodic 설정: 0.1초마다 실행 (10FPS)
	Period.Value = 0.1f;
	bExecutePeriodicEffectOnApplication = true;  // 즉시 첫 번째 회복 적용

	// 3. Stamina Modifier 추가 (0.1초당 +3 회복 → 초당 +30)
	FGameplayModifierInfo StaminaModifier;
	StaminaModifier.Attribute = UCharacterAttributeSet::GetStaminaAttribute();
	StaminaModifier.ModifierOp = EGameplayModOp::Additive;  // 가산 방식
	StaminaModifier.ModifierMagnitude = FScalableFloat(3.0f);  // +3 회복

	Modifiers.Add(StaminaModifier);

	// 4. 스택 설정: 중복 적용 방지
	StackingType = EGameplayEffectStackingType::AggregateBySource;
	StackLimitCount = 1;

	UE_LOG(LogTemp, Log, TEXT("GameplayEffect_StaminaRecover 생성: 0.1초당 +3 스태미나 회복 (초당 +30)"));
}
