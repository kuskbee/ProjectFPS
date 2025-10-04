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

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	virtual void OnRep_SkillPoint(const FGameplayAttributeData& OldSkillPoint);

	// PreAttributeChange: Attribute 값 변경 전 클램핑 처리
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// PostGameplayEffectExecute: GameplayEffect 적용 후 추가 로직
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
};
