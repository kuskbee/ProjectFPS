// Fill out your copyright notice in the Description page of Project Settings.


#include "FPS/CharacterAttributeSet.h"
#include "FPS/GameplayEffect_StaminaRecover.h"
#include "FPS/FPSCharacter.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "Blueprint/UserWidget.h"
#include "UI/DamageNumberWidget.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

// Static 변수 초기화
TSubclassOf<UDamageNumberWidget> UCharacterAttributeSet::DamageNumberWidgetClass = nullptr;

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

	// PreAttributeChange에서 0 ~ Max 범위로 클램핑 (UI 튀는 현상 방지)
	// PostGameplayEffectExecute에서는 추가 로직만 처리 (Shield 우선 소모, Death 등)

	// Health 클램핑: 0 ~ MaxHealth
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	// Stamina 클램핑: 0 ~ MaxStamina
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
	// Shield 클램핑: 0 ~ MaxShield
	else if (Attribute == GetShieldAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxShield());
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

	UE_LOG(LogTemp, VeryVerbose, TEXT("PostGameplayEffectExecute 호출됨! Attribute: %s"), *Data.EvaluatedData.Attribute.GetName());

	// Health 처리 - Shield 우선 소모 + 클램핑 + Death 체크
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		float DamageMagnitude = Data.EvaluatedData.Magnitude;

		// 1. 음수 Magnitude = 데미지 (Health 감소) → Shield 처리 + 공격자 추적
		if (DamageMagnitude < 0.0f)
		{
			// 공격자 추적 (스킬 포인트 보상용)
			AActor* OwnerActor = GetOwningActor();
			if (OwnerActor)
			{
				if (AFPSCharacter* Character = Cast<AFPSCharacter>(OwnerActor))
				{
					// EffectContext에서 Instigator 가져오기
					const FGameplayEffectContextHandle& ContextHandle = Data.EffectSpec.GetEffectContext();
					AActor* InstigatorActor = ContextHandle.GetInstigator();
					if (APawn* InstigatorPawn = Cast<APawn>(InstigatorActor))
					{
						Character->SetLastAttacker(InstigatorPawn);
						UE_LOG(LogTemp, VeryVerbose, TEXT("공격자 추적: %s → %s"), *InstigatorPawn->GetName(), *Character->GetName());
					}
				}
			}

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

		// 2. Health 클램핑 (0 ~ MaxHealth)
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));

		// 3. 데미지 숫자 위젯 표시 (데미지를 받았을 때만)
		if (DamageMagnitude < 0.0f)
		{
			SpawnDamageNumberWidget(Data);
		}

		if (GetHealth() <= 0.0f)
		{
			// 4. Death 처리 (Shield 계산 후 최종 체크!)
			AActor* OwnerActor = GetOwningActor();
			if (OwnerActor)
			{
				if (AFPSCharacter* Character = Cast<AFPSCharacter>(OwnerActor))
				{
					// bIsAlive 플래그 체크로 중복 Death 방지
					if (Character->bIsAlive)
					{
						Character->OnPlayerDeath();
						UE_LOG(LogTemp, Log, TEXT("PostGameplayEffectExecute: Health 0 도달 → Death 처리 (bIsAlive: true -> false)"));
					}
				}
			}
		}
	}

	// Stamina 클램핑 (0 ~ MaxStamina)
	if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		float MaxStaminaValue = GetMaxStamina();

		// 클램핑
		SetStamina(FMath::Clamp(GetStamina(), 0.f, GetMaxStamina()));

		// MaxStamina 도달 시 StaminaRecover Effect 제거
		if (GetStamina() >= MaxStaminaValue)
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

	// Mana 클램핑 (0 ~ MaxMana)
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
	}

	// Shield 클램핑 (0 ~ MaxShield)
	if (Data.EvaluatedData.Attribute == GetShieldAttribute())
	{
		SetShield(FMath::Clamp(GetShield(), 0.f, GetMaxShield()));
	}
}

void UCharacterAttributeSet::SpawnDamageNumberWidget(const FGameplayEffectModCallbackData& Data)
{
	AActor* OwnerActor = GetOwningActor();
	if (!OwnerActor)
	{
		return;
	}

	// 월드와 PlayerController 가져오기
	UWorld* World = OwnerActor->GetWorld();
	if (!World)
	{
		return;
	}

	// 데미지 값 계산 (음수 → 양수로 변환)
	float DamageAmount = FMath::Abs(Data.EvaluatedData.Magnitude);

	// 크리티컬 여부 가져오기 (SetByCaller 방식)
	bool bIsCritical = false;
	if (Data.EffectSpec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.IsCritical"))) > 0.5f)
	{
		bIsCritical = true;
	}

	// Static 변수에서 위젯 클래스 가져오기 (FPSCharacter BeginPlay에서 설정됨)
	if (!DamageNumberWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("DamageNumberWidget 클래스를 찾을 수 없습니다!"));
		return;
	}

	// 위젯 생성
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}

	UDamageNumberWidget* DamageWidget = CreateWidget<UDamageNumberWidget>(PC, DamageNumberWidgetClass);
	if (!DamageWidget)
	{
		return;
	}

	// 데미지 값 설정
	DamageWidget->SetDamageNumber(DamageAmount, bIsCritical);

	// 캐릭터 머리 위에 위젯 배치
	FVector WorldLocation = OwnerActor->GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);  // 머리 위 100cm
	FVector2D ScreenPosition;
	if (PC->ProjectWorldLocationToScreen(WorldLocation, ScreenPosition))
	{
		// 뷰포트에 추가
		DamageWidget->AddToViewport(100);  // ZOrder 100 (최상단)

		// 위치 설정 (Canvas Panel에 추가하기 위해서는 SetPositionInViewport 사용)
		DamageWidget->SetPositionInViewport(ScreenPosition);

		UE_LOG(LogTemp, Log, TEXT("데미지 숫자 위젯 생성: %.0f (크리티컬: %s) at (%.0f, %.0f)"),
			DamageAmount, bIsCritical ? TEXT("예") : TEXT("아니오"), ScreenPosition.X, ScreenPosition.Y);
	}
}
