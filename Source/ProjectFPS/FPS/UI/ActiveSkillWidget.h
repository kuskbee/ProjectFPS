// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpec.h"
#include "ActiveSkillWidget.generated.h"

class UImage;
class UTextBlock;
class UOverlay;
class UMaterialInstanceDynamic;
class UAbilitySystemComponent;

/**
 * 액티브 스킬 UI (Q키)
 * - 현재 장착된 액티브 스킬 아이콘 표시
 * - 쿨다운 시각화 (Circular Progress)
 * - Q키 바인딩 텍스트
 */
UCLASS()
class PROJECTFPS_API UActiveSkillWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** 액티브 스킬 변경 시 UI 업데이트 - SkillData 직접 받기 */
	UFUNCTION(BlueprintCallable, Category = "Active Skill")
	void UpdateActiveSkill(class UBaseSkillData* NewSkillData);

	/** 쿨다운 UI 업데이트 */
	void UpdateCooldown();

protected:
	/** 스킬 아이콘 이미지 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> SkillIconImage;

	/** 쿨다운 오버레이 (어두운 효과) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CooldownOverlay;

	/** 남은 쿨다운 시간 텍스트 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CooldownText;

	/** Q키 바인딩 텍스트 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> KeyBindingText;

	/** 빈 슬롯 텍스트 (스킬 없을 때) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> EmptySlotText;

	/** Ability System Component 캐싱 */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	/** 현재 액티브 스킬 데이터 */
	UPROPERTY()
	TObjectPtr<class UBaseSkillData> CurrentActiveSkillData;

	/** 쿨다운 원형 머티리얼 인스턴스 (선택 사항) */
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CooldownMaterialInstance;

private:
	/** AbilitySpec 찾기 헬퍼 */
	FGameplayAbilitySpec* FindAbilitySpecByTag(const FGameplayTag& SkillTag);

	/** 빈 슬롯 표시 */
	void ShowEmptySlot();

	/** 스킬 아이콘 표시 */
	void ShowSkillIcon();
};
