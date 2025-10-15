// Fill out your copyright notice in the Description page of Project Settings.

#include "ActiveSkillWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "FPS/Skills/BaseSkillData.h"

void UActiveSkillWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// AbilitySystemComponent 가져오기
	APlayerController* PC = GetOwningPlayer();
	if (PC && PC->GetPawn())
	{
		AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PC->GetPawn());
	}

	// Q키 텍스트 설정
	if (KeyBindingText)
	{
		KeyBindingText->SetText(FText::FromString(TEXT("Q")));
	}

	// CooldownOverlay에 머티리얼 설정 (Blueprint에서 설정한 머티리얼 사용)
	if (CooldownOverlay && CooldownOverlay->GetDynamicMaterial())
	{
		CooldownMaterialInstance = CooldownOverlay->GetDynamicMaterial();
		UE_LOG(LogTemp, Log, TEXT("ActiveSkillWidget: 쿨다운 머티리얼 인스턴스 생성 완료"));
	}

	// 초기 상태: 빈 슬롯
	ShowEmptySlot();
}

void UActiveSkillWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 액티브 스킬이 있으면 쿨다운 업데이트
	if (CurrentActiveSkillData)
	{
		UpdateCooldown();
	}
}

void UActiveSkillWidget::UpdateActiveSkill(UBaseSkillData* NewSkillData)
{
	CurrentActiveSkillData = NewSkillData;

	if (CurrentActiveSkillData)
	{
		ShowSkillIcon();
	}
	else
	{
		ShowEmptySlot();
	}
}

void UActiveSkillWidget::UpdateCooldown()
{
	// 단계 1: 기본 체크
	if (!AbilitySystemComponent)
	{
		static bool bWarned = false;
		if (!bWarned)
		{
			UE_LOG(LogTemp, Warning, TEXT("UpdateCooldown: AbilitySystemComponent 없음!"));
			bWarned = true;
		}
		return;
	}

	if (!CurrentActiveSkillData)
	{
		// VeryVerbose: 매 Tick마다 로그 방지
		return;
	}

	static int32 DebugCounter = 0;
	if (DebugCounter % 60 == 0) // 약 1초마다 로그
	{
		UE_LOG(LogTemp, Log, TEXT("UpdateCooldown: 쿨다운 체크 시작 - %s"), *CurrentActiveSkillData->SkillName.ToString());
	}
	DebugCounter++;

	// 단계 2: SkillAbilities 체크
	if (CurrentActiveSkillData->SkillAbilities.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateCooldown: SkillAbilities 배열이 비어있음!"));
		return;
	}

	TSubclassOf<UGameplayAbility> AbilityClass = CurrentActiveSkillData->SkillAbilities[0];
	if (!AbilityClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateCooldown: AbilityClass가 nullptr!"));
		return;
	}

	const UGameplayAbility* AbilityCDO = AbilityClass.GetDefaultObject();
	if (!AbilityCDO)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateCooldown: AbilityCDO를 가져올 수 없음!"));
		return;
	}

	// 단계 3: AssetTags 체크
	const FGameplayTagContainer& AssetTags = AbilityCDO->GetAssetTags();
	if (AssetTags.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateCooldown: AssetTags가 비어있음! Ability에 태그 설정 필요"));
		return;
	}

	FGameplayTag AbilityTag = AssetTags.First();
	if (DebugCounter % 60 == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("UpdateCooldown: Ability 태그 - %s"), *AbilityTag.ToString());
	}

	// 단계 4: AbilitySpec 찾기
	FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecByTag(AbilityTag);
	if (!AbilitySpec)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateCooldown: AbilitySpec을 찾을 수 없음! (태그: %s)"), *AbilityTag.ToString());

		// 쿨다운 없음 (정상)
		if (CooldownOverlay)
		{
			CooldownOverlay->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (CooldownText)
		{
			CooldownText->SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}

	if (!AbilitySpec->Ability)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateCooldown: AbilitySpec->Ability가 nullptr!"));
		return;
	}

	// ⭐ 새로운 방식: ASC에서 모든 Active GameplayEffect를 순회하며 쿨다운 찾기
	// GetCooldownTags() 대신 직접 "Cooldown.ActiveSkill" 태그를 가진 Effect를 찾음!
	FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(FName("Cooldown.ActiveSkill"));

	// ASC에서 Cooldown.ActiveSkill 태그를 가진 Effect 쿼리
	FGameplayEffectQuery Query;
	Query.EffectTagQuery = FGameplayTagQuery::MakeQuery_MatchAnyTags(FGameplayTagContainer(CooldownTag));

	TArray<FActiveGameplayEffectHandle> ActiveCooldowns = AbilitySystemComponent->GetActiveEffects(Query);

	if (DebugCounter % 60 == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("UpdateCooldown: Cooldown.ActiveSkill 태그를 가진 Effect 개수: %d"), ActiveCooldowns.Num());
	}

	if (ActiveCooldowns.Num() == 0)
	{
		// 쿨다운 없음 (정상 - 스킬 사용 가능 상태)
		if (CooldownOverlay)
		{
			CooldownOverlay->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (CooldownText)
		{
			CooldownText->SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ActiveSkillWidget: 쿨다운 감지! 활성 쿨다운 개수: %d"), ActiveCooldowns.Num());

	// 가장 긴 쿨다운 시간 가져오기
	float LongestCooldownTimeRemaining = 0.0f;
	float LongestCooldownDuration = 0.0f;

	for (const FActiveGameplayEffectHandle& Handle : ActiveCooldowns)
	{
		const FActiveGameplayEffect* ActiveGE = AbilitySystemComponent->GetActiveGameplayEffect(Handle);
		if (ActiveGE)
		{
			float TimeRemaining = ActiveGE->GetTimeRemaining(AbilitySystemComponent->GetWorld()->GetTimeSeconds());
			float Duration = ActiveGE->GetDuration();

			if (TimeRemaining > LongestCooldownTimeRemaining)
			{
				LongestCooldownTimeRemaining = TimeRemaining;
				LongestCooldownDuration = Duration;
			}
		}
	}

	// 쿨다운 UI 표시
	if (LongestCooldownTimeRemaining > 0.0f)
	{
		UE_LOG(LogTemp, Log, TEXT("ActiveSkillWidget: 쿨다운 시간 - %.1f / %.1f초"),
			LongestCooldownTimeRemaining, LongestCooldownDuration);

		if (CooldownOverlay)
		{
			CooldownOverlay->SetVisibility(ESlateVisibility::HitTestInvisible);

			// 쿨다운 진행도 계산 (0.0 ~ 1.0)
			float CooldownProgress = (LongestCooldownTimeRemaining / LongestCooldownDuration);

			// 머티리얼 파라미터 설정 (선택 사항)
			if (CooldownMaterialInstance)
			{
				CooldownMaterialInstance->SetScalarParameterValue(FName("Progress"), CooldownProgress);
			}

			// Opacity로 간단하게 표시
			CooldownOverlay->SetOpacity(0.7f);
		}

		if (CooldownText)
		{
			CooldownText->SetVisibility(ESlateVisibility::HitTestInvisible);

			// 소수점 한 자리로 표시 (예: "5.3")
			FString CooldownString = FString::Printf(TEXT("%.1f"), LongestCooldownTimeRemaining);
			CooldownText->SetText(FText::FromString(CooldownString));

			UE_LOG(LogTemp, Log, TEXT("ActiveSkillWidget: 쿨다운 텍스트 설정 완료 - %s"), *CooldownString);
		}
	}
	else
	{
		// 쿨다운 종료
		if (CooldownOverlay)
		{
			CooldownOverlay->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (CooldownText)
		{
			CooldownText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

FGameplayAbilitySpec* UActiveSkillWidget::FindAbilitySpecByTag(const FGameplayTag& SkillTag)
{
	if (!AbilitySystemComponent || !SkillTag.IsValid())
	{
		return nullptr;
	}

	// 부여된 모든 Ability 탐색
	for (FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (Spec.Ability)
		{
			const FGameplayTagContainer& AssetTags = Spec.Ability->GetAssetTags();
			if (AssetTags.HasTag(SkillTag))
			{
				return &Spec;
			}
		}
	}

	return nullptr;
}

void UActiveSkillWidget::ShowEmptySlot()
{
	if (SkillIconImage)
	{
		SkillIconImage->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (CooldownOverlay)
	{
		CooldownOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (CooldownText)
	{
		CooldownText->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (EmptySlotText)
	{
		EmptySlotText->SetVisibility(ESlateVisibility::HitTestInvisible);
		EmptySlotText->SetText(FText::FromString(TEXT("No Skill")));
	}
}

void UActiveSkillWidget::ShowSkillIcon()
{
	if (EmptySlotText)
	{
		EmptySlotText->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (SkillIconImage && CurrentActiveSkillData)
	{
		SkillIconImage->SetVisibility(ESlateVisibility::HitTestInvisible);

		// SkillData에서 아이콘 직접 가져오기 (단일 데이터 소스!)
		if (CurrentActiveSkillData->SkillIcon)
		{
			SkillIconImage->SetBrushFromTexture(CurrentActiveSkillData->SkillIcon);
			UE_LOG(LogTemp, Log, TEXT("ActiveSkillWidget: 스킬 아이콘 표시 - %s"),
				*CurrentActiveSkillData->SkillName.ToString());
		}
		else
		{
			// 기본 아이콘 또는 경고 표시
			UE_LOG(LogTemp, Warning, TEXT("ActiveSkillWidget: 스킬 아이콘이 없음 - %s"),
				*CurrentActiveSkillData->SkillName.ToString());
		}
	}
}
