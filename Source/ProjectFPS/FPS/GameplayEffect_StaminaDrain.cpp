// GameplayEffect_StaminaDrain.cpp

#include "GameplayEffect_StaminaDrain.h"
#include "CharacterAttributeSet.h"

UGameplayEffect_StaminaDrain::UGameplayEffect_StaminaDrain()
{
	// 1. Duration Policy: 무한 지속 (Ability가 종료될 때까지)
	DurationPolicy = EGameplayEffectDurationType::Infinite;

	// 2. Periodic 설정: 0.1초마다 실행 (10FPS)
	Period.Value = 0.1f;
	bExecutePeriodicEffectOnApplication = true;  // 즉시 첫 번째 소모 적용

	// 3. Stamina Modifier 추가 (0.1초당 -5 감소 → 초당 -50)
	FGameplayModifierInfo StaminaModifier;
	StaminaModifier.Attribute = UCharacterAttributeSet::GetStaminaAttribute();
	StaminaModifier.ModifierOp = EGameplayModOp::Additive;  // 가산 방식
	StaminaModifier.ModifierMagnitude = FScalableFloat(-5.0f);  // -5 감소

	Modifiers.Add(StaminaModifier);

	// 4. 스택 설정: 중복 적용 방지
	StackingType = EGameplayEffectStackingType::AggregateBySource;
	StackLimitCount = 1;

	UE_LOG(LogTemp, Log, TEXT("GameplayEffect_StaminaDrain 생성: 0.1초당 -5 스태미나 소모 (초당 -50)"));
}
