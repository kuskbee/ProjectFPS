// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayEffect_InstantHeal.h"
#include "CharacterAttributeSet.h"
#include "GameplayTags.h"

UGameplayEffect_InstantHeal::UGameplayEffect_InstantHeal()
{
	// Instant Duration (즉시 적용)
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// Modifier 1: Health 회복 (SetByCaller 방식)
	FGameplayModifierInfo HealthModifier;
	HealthModifier.Attribute = UCharacterAttributeSet::GetHealthAttribute();
	HealthModifier.ModifierOp = EGameplayModOp::Additive;

	// SetByCaller 설정 (Data.HealAmount 태그로 동적 전달)
	FSetByCallerFloat SetByCallerData;
	SetByCallerData.DataTag = FGameplayTag::RequestGameplayTag(FName("Data.HealAmount"));
	HealthModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerData);

	Modifiers.Add(HealthModifier);
}
