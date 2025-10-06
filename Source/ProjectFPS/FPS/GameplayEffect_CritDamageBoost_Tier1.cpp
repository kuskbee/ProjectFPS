// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/GameplayEffect_CritDamageBoost_Tier1.h"
#include "FPS/PlayerAttributeSet.h"

UGameplayEffect_CritDamageBoost_Tier1::UGameplayEffect_CritDamageBoost_Tier1()
{
	// Duration: Instant (즉시 적용 후 종료)
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// Modifier: CritDamage +0.5 (1.5 → 2.0, 즉 200% 크리티컬 데미지)
	FGameplayModifierInfo CritDamageModifier;
	CritDamageModifier.Attribute = UPlayerAttributeSet::GetCritDamageAttribute();
	CritDamageModifier.ModifierOp = EGameplayModOp::Additive;
	CritDamageModifier.ModifierMagnitude = FScalableFloat(0.5f);
	Modifiers.Add(CritDamageModifier);
}
