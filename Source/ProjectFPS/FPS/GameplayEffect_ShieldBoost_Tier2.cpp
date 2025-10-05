// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/GameplayEffect_ShieldBoost_Tier2.h"
#include "FPS/CharacterAttributeSet.h"

UGameplayEffect_ShieldBoost_Tier2::UGameplayEffect_ShieldBoost_Tier2()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// MaxShield +100
	FGameplayModifierInfo MaxShieldModifier;
	MaxShieldModifier.Attribute = UCharacterAttributeSet::GetMaxShieldAttribute();
	MaxShieldModifier.ModifierOp = EGameplayModOp::Additive;
	MaxShieldModifier.ModifierMagnitude = FScalableFloat(100.0f);
	Modifiers.Add(MaxShieldModifier);

	// Shield +100 (즉시 회복)
	FGameplayModifierInfo ShieldModifier;
	ShieldModifier.Attribute = UCharacterAttributeSet::GetShieldAttribute();
	ShieldModifier.ModifierOp = EGameplayModOp::Additive;
	ShieldModifier.ModifierMagnitude = FScalableFloat(100.0f);
	Modifiers.Add(ShieldModifier);
}
