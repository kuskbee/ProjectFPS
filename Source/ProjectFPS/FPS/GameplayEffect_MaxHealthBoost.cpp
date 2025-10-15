// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/GameplayEffect_MaxHealthBoost.h"
#include "FPS/CharacterAttributeSet.h"

UGameplayEffect_MaxHealthBoost::UGameplayEffect_MaxHealthBoost()
{
	// Duration: Instant (즉시 적용 후 종료)
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// Modifier: MaxHealth +50 (영구 증가)
	FGameplayModifierInfo MaxHealthModifier;
	MaxHealthModifier.Attribute = UCharacterAttributeSet::GetMaxHealthAttribute();
	MaxHealthModifier.ModifierOp = EGameplayModOp::Additive;
	MaxHealthModifier.ModifierMagnitude = FScalableFloat(50.0f);
	Modifiers.Add(MaxHealthModifier);

	// Health도 함께 회복 (+50)
	FGameplayModifierInfo HealthModifier;
	HealthModifier.Attribute = UCharacterAttributeSet::GetHealthAttribute();
	HealthModifier.ModifierOp = EGameplayModOp::Additive;
	HealthModifier.ModifierMagnitude = FScalableFloat(50.0f);
	Modifiers.Add(HealthModifier);
}