// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/GameplayEffect_MaxStaminaBoost.h"
#include "FPS/CharacterAttributeSet.h"

UGameplayEffect_MaxStaminaBoost::UGameplayEffect_MaxStaminaBoost()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// MaxStamina +50
	FGameplayModifierInfo MaxStaminaModifier;
	MaxStaminaModifier.Attribute = UCharacterAttributeSet::GetMaxStaminaAttribute();
	MaxStaminaModifier.ModifierOp = EGameplayModOp::Additive;
	MaxStaminaModifier.ModifierMagnitude = FScalableFloat(50.0f);
	Modifiers.Add(MaxStaminaModifier);

	// Stamina도 함께 회복 (+50)
	FGameplayModifierInfo StaminaModifier;
	StaminaModifier.Attribute = UCharacterAttributeSet::GetStaminaAttribute();
	StaminaModifier.ModifierOp = EGameplayModOp::Additive;
	StaminaModifier.ModifierMagnitude = FScalableFloat(50.0f);
	Modifiers.Add(StaminaModifier);
}