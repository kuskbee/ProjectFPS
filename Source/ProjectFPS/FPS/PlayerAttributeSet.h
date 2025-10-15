// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "PlayerAttributeSet.generated.h"

// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 플레이어 전용 Attribute Set
 * 스킬 포인트, 경험치 등 플레이어만 사용하는 속성들
 */
UCLASS()
class PROJECTFPS_API UPlayerAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPlayerAttributeSet();

	// Skill Point (스킬 포인트)
	UPROPERTY(BlueprintReadOnly, Category = "Player Attributes", ReplicatedUsing = OnRep_SkillPoint)
	FGameplayAttributeData SkillPoint;
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, SkillPoint);

	// Critical Chance (크리티컬 확률, 0.0 ~ 1.0)
	UPROPERTY(BlueprintReadOnly, Category = "Player Attributes", ReplicatedUsing = OnRep_CritChance)
	FGameplayAttributeData CritChance;
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, CritChance);

	// Critical Damage (크리티컬 데미지 배율, 1.5 = 150%)
	UPROPERTY(BlueprintReadOnly, Category = "Player Attributes", ReplicatedUsing = OnRep_CritDamage)
	FGameplayAttributeData CritDamage;
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, CritDamage);

	// Attack Speed Multiplier (공격속도 배율, 1.0 = 기본, 1.5 = 50% 증가)
	UPROPERTY(BlueprintReadOnly, Category = "Player Attributes", ReplicatedUsing = OnRep_AttackSpeedMultiplier)
	FGameplayAttributeData AttackSpeedMultiplier;
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, AttackSpeedMultiplier);

	// Move Speed Multiplier (이동속도 배율, 1.0 = 기본, 1.3 = 30% 증가)
	UPROPERTY(BlueprintReadOnly, Category = "Player Attributes", ReplicatedUsing = OnRep_MoveSpeedMultiplier)
	FGameplayAttributeData MoveSpeedMultiplier;
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, MoveSpeedMultiplier);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	virtual void OnRep_SkillPoint(const FGameplayAttributeData& OldSkillPoint);

	UFUNCTION()
	virtual void OnRep_CritChance(const FGameplayAttributeData& OldCritChance);

	UFUNCTION()
	virtual void OnRep_CritDamage(const FGameplayAttributeData& OldCritDamage);

	UFUNCTION()
	virtual void OnRep_AttackSpeedMultiplier(const FGameplayAttributeData& OldAttackSpeedMultiplier);

	UFUNCTION()
	virtual void OnRep_MoveSpeedMultiplier(const FGameplayAttributeData& OldMoveSpeedMultiplier);

	// PreAttributeChange: Attribute 값 변경 전 클램핑 처리
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// PostGameplayEffectExecute: GameplayEffect 적용 후 추가 로직
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
};
