// Fill out your copyright notice in the Description page of Project Settings.


#include "FPS/CharacterAttributeSet.h"
#include "FPS/GameplayEffect_StaminaRecover.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UCharacterAttributeSet::UCharacterAttributeSet()
{
	// Default values for Attributes
	Health.SetBaseValue(100.0f);
	Health.SetCurrentValue(100.0f);
	MaxHealth.SetBaseValue(100.0f);
	MaxHealth.SetCurrentValue(100.0f);

	Mana.SetBaseValue(100.0f);
	Mana.SetCurrentValue(100.0f);
	MaxMana.SetBaseValue(100.0f);
	MaxMana.SetCurrentValue(100.0f);

	Stamina.SetBaseValue(100.0f);
	Stamina.SetCurrentValue(100.0f);
	MaxStamina.SetBaseValue(100.0f);
	MaxStamina.SetCurrentValue(100.0f);

	Shield.SetBaseValue(0.0f);
	Shield.SetCurrentValue(0.0f);
	MaxShield.SetBaseValue(0.0f);
	MaxShield.SetCurrentValue(0.0f);
}

void UCharacterAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UCharacterAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCharacterAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCharacterAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCharacterAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCharacterAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCharacterAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCharacterAttributeSet, Shield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCharacterAttributeSet, MaxShield, COND_None, REPNOTIFY_Always);
}

void UCharacterAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCharacterAttributeSet, Health, OldHealth);
}

void UCharacterAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCharacterAttributeSet, MaxHealth, OldMaxHealth);
}

void UCharacterAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCharacterAttributeSet, Mana, OldMana);
}

void UCharacterAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCharacterAttributeSet, MaxMana, OldMaxMana);
}

void UCharacterAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCharacterAttributeSet, Stamina, OldStamina);
}

void UCharacterAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCharacterAttributeSet, MaxStamina, OldMaxStamina);
}

void UCharacterAttributeSet::OnRep_Shield(const FGameplayAttributeData& OldShield)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCharacterAttributeSet, Shield, OldShield);
}

void UCharacterAttributeSet::OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCharacterAttributeSet, MaxShield, OldMaxShield);
}

void UCharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Health 클램핑: 0 ~ MaxHealth
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	// Stamina 클램핑: 0 ~ MaxStamina
	else if (Attribute == GetStaminaAttribute())
	{
		float OldValue = GetStamina();
		UE_LOG(LogTemp, VeryVerbose, TEXT("PreAttributeChange 스태미나 클램핑 전: %.1f -> %.1f (+%.1f)"), OldValue, NewValue, NewValue - OldValue);
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
		UE_LOG(LogTemp, VeryVerbose, TEXT("PreAttributeChange 스태미나 클램핑 후: %.1f -> %.1f (+%.1f)"), OldValue, NewValue, NewValue - OldValue);

		// 스태미나 변경 로그 (증가/감소 구분)
		/*if (NewValue > OldValue)
		{
			UE_LOG(LogTemp, Log, TEXT("스태미나 회복: %.1f -> %.1f (+%.1f)"), OldValue, NewValue, NewValue - OldValue);
		}
		else if (NewValue < OldValue)
		{
			UE_LOG(LogTemp, Log, TEXT("스태미나 소모: %.1f -> %.1f (%.1f)"), OldValue, NewValue, NewValue - OldValue);
		}*/
	}
	// Mana 클램핑: 0 ~ MaxMana
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMana());
	}
}

void UCharacterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	UE_LOG(LogTemp, Warning, TEXT("PostGameplayEffectExecute 호출됨! Attribute: %s"), *Data.EvaluatedData.Attribute.GetName());

	// Health 데미지 처리 - Shield 우선 소모
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		float DamageMagnitude = Data.EvaluatedData.Magnitude;

		// 음수 Magnitude = 데미지 (Health 감소)
		if (DamageMagnitude < 0.0f)
		{
			float AbsoluteDamage = -DamageMagnitude;
			float CurrentShield = GetShield();

			if (CurrentShield > 0.0f)
			{
				// Shield가 흡수할 수 있는 데미지 계산
				float ShieldDamage = FMath::Min(CurrentShield, AbsoluteDamage);
				SetShield(CurrentShield - ShieldDamage);

				// 이미 Health에서 전체 데미지가 차감된 상태이므로,
				// Shield가 막은 만큼 Health를 다시 복구
				SetHealth(GetHealth() + ShieldDamage);

				UE_LOG(LogTemp, Log, TEXT("Shield 데미지 처리: Shield %.1f → %.1f (흡수: %.1f), Health 복구: +%.1f"),
					CurrentShield, GetShield(), ShieldDamage, ShieldDamage);
			}
		}
	}

	// 스태미나가 MaxStamina에 도달하면 StaminaRecover Effect 제거
	if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		float CurrentStamina = GetStamina();
		float MaxStaminaValue = GetMaxStamina();

		if (CurrentStamina >= MaxStaminaValue)
		{
			UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
			if (ASC)
			{
				// GameplayEffect_StaminaRecover 클래스로 Effect 찾기
				FGameplayEffectQuery Query;
				Query.CustomMatchDelegate.BindLambda([](const FActiveGameplayEffect& ActiveGE)
					{
						// 자식 클래스 까지 포함해서 찾도록 함.
						const UGameplayEffect* Def = ActiveGE.Spec.Def;
						return Def && Def->GetClass()->IsChildOf(UGameplayEffect_StaminaRecover::StaticClass());
					});

				TArray<FActiveGameplayEffectHandle> ActiveEffects = ASC->GetActiveEffects(Query);

				for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
				{
					ASC->RemoveActiveGameplayEffect(Handle);
					UE_LOG(LogTemp, VeryVerbose, TEXT("PostGameplayEffectExecute: 스태미나 최대치 도달, StaminaRecover Effect 제거"));
				}
			}
		}
	}
}
