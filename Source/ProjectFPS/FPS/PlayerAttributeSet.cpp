// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/PlayerAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UPlayerAttributeSet::UPlayerAttributeSet()
{
	// Default values for Player Attributes
	SkillPoint.SetBaseValue(0.0f);
	SkillPoint.SetCurrentValue(0.0f);
}

void UPlayerAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// SkillPoint 리플리케이션 설정
	DOREPLIFETIME_CONDITION_NOTIFY(UPlayerAttributeSet, SkillPoint, COND_None, REPNOTIFY_Always);
}

void UPlayerAttributeSet::OnRep_SkillPoint(const FGameplayAttributeData& OldSkillPoint)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPlayerAttributeSet, SkillPoint, OldSkillPoint);
}

void UPlayerAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// SkillPoint는 0 미만으로 떨어지지 않도록 클램핑
	if (Attribute == GetSkillPointAttribute())
	{
		NewValue = FMath::Max(0.0f, NewValue);
	}
}

void UPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// SkillPoint 변경 시 로그 출력
	if (Data.EvaluatedData.Attribute == GetSkillPointAttribute())
	{
		UE_LOG(LogTemp, Log, TEXT("SkillPoint 변경: %.0f"), GetSkillPoint());
	}
}
