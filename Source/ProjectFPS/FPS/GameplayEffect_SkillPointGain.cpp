// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayEffect_SkillPointGain.h"
#include "PlayerAttributeSet.h"

UGameplayEffect_SkillPointGain::UGameplayEffect_SkillPointGain()
{
	// Instant Duration (즉시 적용)
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// Modifier: SkillPoint Additive (SetByCaller 방식)
	FGameplayModifierInfo SkillPointModifier;
	SkillPointModifier.Attribute = UPlayerAttributeSet::GetSkillPointAttribute();
	SkillPointModifier.ModifierOp = EGameplayModOp::Additive;

	// SetByCaller 설정 (Data.SkillPointGain 태그로 동적 전달)
	FSetByCallerFloat SetByCallerData;
	SetByCallerData.DataTag = FGameplayTag::RequestGameplayTag(FName("Data.SkillPointGain"));
	SkillPointModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerData);

	Modifiers.Add(SkillPointModifier);
}
