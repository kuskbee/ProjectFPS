// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/GameplayEffect_Heal.h"
#include "GameplayEffectTypes.h"
#include "FPS/CharacterAttributeSet.h"

UGameplayEffect_Heal::UGameplayEffect_Heal()
{
	// 즉시 적용되는 일회성 효과로 설정
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// Health를 MaxHealth 값으로 설정하는 Modifier 추가
	FGameplayModifierInfo HealthModifier;
	HealthModifier.Attribute = UCharacterAttributeSet::GetHealthAttribute();
	HealthModifier.ModifierOp = EGameplayModOp::Override;

	// MaxHealth Attribute를 참조하는 FAttributeBasedFloat 사용
	FAttributeBasedFloat MaxHealthBasedFloat;
	MaxHealthBasedFloat.BackingAttribute =
		FGameplayEffectAttributeCaptureDefinition(
			UCharacterAttributeSet::GetMaxHealthAttribute(),
			EGameplayEffectAttributeCaptureSource::Target,
			/*bSnapshot*/ false
		);
	MaxHealthBasedFloat.AttributeCalculationType = EAttributeBasedFloatCalculationType::AttributeMagnitude;

	MaxHealthBasedFloat.Coefficient = 1.0f; // MaxHealth 그대로 사용
	MaxHealthBasedFloat.PreMultiplyAdditiveValue = 0.0f;
	MaxHealthBasedFloat.PostMultiplyAdditiveValue = 0.0f;
	MaxHealthBasedFloat.FinalChannel = EGameplayModEvaluationChannel::Channel0;

	HealthModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(MaxHealthBasedFloat);

	Modifiers.Add(HealthModifier);
}