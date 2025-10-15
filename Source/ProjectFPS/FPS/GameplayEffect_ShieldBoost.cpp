// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/GameplayEffect_ShieldBoost.h"
#include "FPS/CharacterAttributeSet.h"

UGameplayEffect_ShieldBoost::UGameplayEffect_ShieldBoost()
{
	// Duration: Instant (즉시 적용 후 종료)
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// Modifier 1: MaxShield +50 (영구 증가)
	FGameplayModifierInfo MaxShieldModifier;
	MaxShieldModifier.Attribute = UCharacterAttributeSet::GetMaxShieldAttribute();
	MaxShieldModifier.ModifierOp = EGameplayModOp::Additive;
	MaxShieldModifier.ModifierMagnitude = FScalableFloat(50.0f);
	Modifiers.Add(MaxShieldModifier);

	// Modifier 2: Shield +50 (즉시 회복)
	FGameplayModifierInfo ShieldModifier;
	ShieldModifier.Attribute = UCharacterAttributeSet::GetShieldAttribute();
	ShieldModifier.ModifierOp = EGameplayModOp::Additive;
	ShieldModifier.ModifierMagnitude = FScalableFloat(50.0f);
	Modifiers.Add(ShieldModifier);
}
