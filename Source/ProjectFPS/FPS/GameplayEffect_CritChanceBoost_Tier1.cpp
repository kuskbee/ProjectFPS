// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/GameplayEffect_CritChanceBoost_Tier1.h"
#include "FPS/PlayerAttributeSet.h"

UGameplayEffect_CritChanceBoost_Tier1::UGameplayEffect_CritChanceBoost_Tier1()
{
	// Duration: Instant (즉시 적용 후 종료)
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// Modifier: CritChance +0.05 (5% 영구 증가)
	FGameplayModifierInfo CritChanceModifier;
	CritChanceModifier.Attribute = UPlayerAttributeSet::GetCritChanceAttribute();
	CritChanceModifier.ModifierOp = EGameplayModOp::Additive;
	CritChanceModifier.ModifierMagnitude = FScalableFloat(0.05f);
	Modifiers.Add(CritChanceModifier);
}