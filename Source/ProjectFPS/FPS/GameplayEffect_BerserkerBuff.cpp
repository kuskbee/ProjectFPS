// GameplayEffect_BerserkerBuff.cpp

#include "GameplayEffect_BerserkerBuff.h"
#include "PlayerAttributeSet.h"
#include "CharacterAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"

UGameplayEffect_BerserkerBuff::UGameplayEffect_BerserkerBuff()
{
	// Duration Policy: 10초간 지속
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FScalableFloat(10.0f);

	// Modifier 1: AttackSpeedMultiplier +50% (1.0 → 1.5)
	FGameplayModifierInfo AttackSpeedModifier;
	AttackSpeedModifier.Attribute = UPlayerAttributeSet::GetAttackSpeedMultiplierAttribute();
	AttackSpeedModifier.ModifierOp = EGameplayModOp::Additive;
	AttackSpeedModifier.ModifierMagnitude = FScalableFloat(0.5f);  // +0.5 (즉, 1.0 + 0.5 = 1.5)
	Modifiers.Add(AttackSpeedModifier);

	// Modifier 2: MoveSpeedMultiplier +30% (1.0 → 1.3)
	FGameplayModifierInfo MoveSpeedModifier;
	MoveSpeedModifier.Attribute = UPlayerAttributeSet::GetMoveSpeedMultiplierAttribute();
	MoveSpeedModifier.ModifierOp = EGameplayModOp::Additive;
	MoveSpeedModifier.ModifierMagnitude = FScalableFloat(0.3f);  // +0.3 (즉, 1.0 + 0.3 = 1.3)
	Modifiers.Add(MoveSpeedModifier);

	UE_LOG(LogTemp, Log, TEXT("GameplayEffect_BerserkerBuff 생성자 호출 (공격속도 +50%%, 이동속도 +30%%)"));
}
