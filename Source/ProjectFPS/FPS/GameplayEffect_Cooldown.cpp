// GameplayEffect_Cooldown.cpp

#include "GameplayEffect_Cooldown.h"

UGameplayEffect_Cooldown::UGameplayEffect_Cooldown()
{
	// Duration 기반 쿨다운 (SetByCaller로 동적 설정)
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	// SetByCaller로 쿨다운 시간 설정
	FSetByCallerFloat CooldownDuration;
	CooldownDuration.DataTag = FGameplayTag::RequestGameplayTag(FName("Data.Cooldown"));
	DurationMagnitude = FGameplayEffectModifierMagnitude(CooldownDuration);

	// Cooldown Tag는 Ability에서 동적으로 추가
	// (SpecHandle.Data->GetDynamicGrantedTags().AppendTags(CooldownTags) 방식)

	// 스택 정책: 쿨다운은 스택 안됨
	StackingType = EGameplayEffectStackingType::None;
}
