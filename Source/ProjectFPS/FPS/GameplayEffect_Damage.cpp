// Fill out your copyright notice in the Description page of Project Settings.


#include "FPS/GameplayEffect_Damage.h"
#include "FPS/CharacterAttributeSet.h"

UGameplayEffect_Damage::UGameplayEffect_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant; // instant : 즉시 적용되고 사라짐.

	// "Health" 속성을 수정하는 Modifier 정의
	FGameplayModifierInfo ModifierInfo;
	ModifierInfo.Attribute = UCharacterAttributeSet::GetHealthAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;
	const float DamageAmount = -10.0f; //:TODO: 수정할 수있는 값으로 빼기
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(DamageAmount));

	Modifiers.Add(ModifierInfo);
}