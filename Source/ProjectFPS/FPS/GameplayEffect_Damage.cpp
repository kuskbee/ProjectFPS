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

	// SetByCaller 방식으로 데미지 값 설정 (발사체에서 동적으로 설정)
	FSetByCallerFloat SetByCallerMagnitude;
	SetByCallerMagnitude.DataTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerMagnitude);

	Modifiers.Add(ModifierInfo);
}