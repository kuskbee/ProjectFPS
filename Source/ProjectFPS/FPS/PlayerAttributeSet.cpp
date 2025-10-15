// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/PlayerAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UPlayerAttributeSet::UPlayerAttributeSet()
{
	// Default values for Player Attributes
	// 테스트용: 시작 시 스킬 포인트 5개 지급
	SkillPoint.SetBaseValue(5.0f);
	SkillPoint.SetCurrentValue(5.0f);

	// Critical 초기값 설정
	CritChance.SetBaseValue(0.05f);      // 기본 크리티컬 확률 5%
	CritChance.SetCurrentValue(0.05f);

	CritDamage.SetBaseValue(1.5f);       // 기본 크리티컬 데미지 150%
	CritDamage.SetCurrentValue(1.5f);

	AttackSpeedMultiplier.SetBaseValue(1.0f);  // 기본 공격속도 배율 100%
	AttackSpeedMultiplier.SetCurrentValue(1.0f);

	MoveSpeedMultiplier.SetBaseValue(1.0f);    // 기본 이동속도 배율 100%
	MoveSpeedMultiplier.SetCurrentValue(1.0f);
}

void UPlayerAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// SkillPoint 리플리케이션 설정
	DOREPLIFETIME_CONDITION_NOTIFY(UPlayerAttributeSet, SkillPoint, COND_None, REPNOTIFY_Always);

	// Critical Attributes 리플리케이션 설정
	DOREPLIFETIME_CONDITION_NOTIFY(UPlayerAttributeSet, CritChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPlayerAttributeSet, CritDamage, COND_None, REPNOTIFY_Always);

	// AttackSpeedMultiplier 리플리케이션 설정
	DOREPLIFETIME_CONDITION_NOTIFY(UPlayerAttributeSet, AttackSpeedMultiplier, COND_None, REPNOTIFY_Always);

	// MoveSpeedMultiplier 리플리케이션 설정
	DOREPLIFETIME_CONDITION_NOTIFY(UPlayerAttributeSet, MoveSpeedMultiplier, COND_None, REPNOTIFY_Always);
}

void UPlayerAttributeSet::OnRep_SkillPoint(const FGameplayAttributeData& OldSkillPoint)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPlayerAttributeSet, SkillPoint, OldSkillPoint);
}

void UPlayerAttributeSet::OnRep_CritChance(const FGameplayAttributeData& OldCritChance)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPlayerAttributeSet, CritChance, OldCritChance);
}

void UPlayerAttributeSet::OnRep_CritDamage(const FGameplayAttributeData& OldCritDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPlayerAttributeSet, CritDamage, OldCritDamage);
}

void UPlayerAttributeSet::OnRep_AttackSpeedMultiplier(const FGameplayAttributeData& OldAttackSpeedMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPlayerAttributeSet, AttackSpeedMultiplier, OldAttackSpeedMultiplier);
}

void UPlayerAttributeSet::OnRep_MoveSpeedMultiplier(const FGameplayAttributeData& OldMoveSpeedMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPlayerAttributeSet, MoveSpeedMultiplier, OldMoveSpeedMultiplier);
}

void UPlayerAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// SkillPoint는 0 미만으로 떨어지지 않도록 클램핑
	if (Attribute == GetSkillPointAttribute())
	{
		NewValue = FMath::Max(0.0f, NewValue);
	}

	// CritChance는 0.0 ~ 1.0 사이로 클램핑
	if (Attribute == GetCritChanceAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, 1.0f);
	}

	// CritDamage는 1.0 이상으로 클램핑 (100% 이상)
	if (Attribute == GetCritDamageAttribute())
	{
		NewValue = FMath::Max(1.0f, NewValue);
	}

	// AttackSpeedMultiplier는 0.1 ~ 3.0 사이로 클램핑
	if (Attribute == GetAttackSpeedMultiplierAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.1f, 3.0f);
	}

	// MoveSpeedMultiplier는 0.1 ~ 3.0 사이로 클램핑
	if (Attribute == GetMoveSpeedMultiplierAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.1f, 3.0f);
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

	// CritChance 변경 시 로그 출력
	if (Data.EvaluatedData.Attribute == GetCritChanceAttribute())
	{
		UE_LOG(LogTemp, Log, TEXT("CritChance 변경: %.2f%%"), GetCritChance() * 100.0f);
	}

	// CritDamage 변경 시 로그 출력
	if (Data.EvaluatedData.Attribute == GetCritDamageAttribute())
	{
		UE_LOG(LogTemp, Log, TEXT("CritDamage 변경: %.0f%%"), GetCritDamage() * 100.0f);
	}

	// AttackSpeedMultiplier 변경 시 로그 출력
	if (Data.EvaluatedData.Attribute == GetAttackSpeedMultiplierAttribute())
	{
		UE_LOG(LogTemp, Warning, TEXT("⚡ AttackSpeedMultiplier 변경: %.2fx"), GetAttackSpeedMultiplier());
	}
}
