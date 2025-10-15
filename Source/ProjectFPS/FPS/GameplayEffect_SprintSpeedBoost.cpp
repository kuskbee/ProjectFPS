// GameplayEffect_SprintSpeedBoost.cpp

#include "GameplayEffect_SprintSpeedBoost.h"
#include "PlayerAttributeSet.h"

UGameplayEffect_SprintSpeedBoost::UGameplayEffect_SprintSpeedBoost()
{
	// Duration Policy: Infinite (Sprint Ability에서 수동 제거)
	DurationPolicy = EGameplayEffectDurationType::Infinite;

	// Modifier: MoveSpeedMultiplier (SetByCaller 방식)
	FGameplayModifierInfo MoveSpeedModifier;
	MoveSpeedModifier.Attribute = UPlayerAttributeSet::GetMoveSpeedMultiplierAttribute();
	MoveSpeedModifier.ModifierOp = EGameplayModOp::Additive;

	// SetByCaller로 값 받기 (Data.SprintSpeed 태그)
	FSetByCallerFloat SetByCallerData;
	SetByCallerData.DataTag = FGameplayTag::RequestGameplayTag(FName("Data.SprintSpeed"));
	MoveSpeedModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerData);

	Modifiers.Add(MoveSpeedModifier);

	UE_LOG(LogTemp, Log, TEXT("GameplayEffect_SprintSpeedBoost 생성자 호출 (SetByCaller 방식)"));
}
