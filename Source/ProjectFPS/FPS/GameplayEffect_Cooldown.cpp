// GameplayEffect_Cooldown.cpp

#include "GameplayEffect_Cooldown.h"
#include "GameplayEffectComponents/AssetTagsGameplayEffectComponent.h"

UGameplayEffect_Cooldown::UGameplayEffect_Cooldown()
{
	// Duration 기반 쿨다운 (SetByCaller로 동적 설정)
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	// SetByCaller로 쿨다운 시간 설정
	FSetByCallerFloat CooldownDuration;
	CooldownDuration.DataTag = FGameplayTag::RequestGameplayTag(FName("Data.Cooldown"));
	DurationMagnitude = FGameplayEffectModifierMagnitude(CooldownDuration);

	// ⭐ Cooldown.ActiveSkill 태그 추가 (GetCooldownTags()가 이걸 반환함!)
	// UE 5.6+ 새로운 API: UAssetTagsGameplayEffectComponent 사용
	UAssetTagsGameplayEffectComponent* AssetTagsComp = CreateDefaultSubobject<UAssetTagsGameplayEffectComponent>(TEXT("AssetTagsComponent"));

	FInheritedTagContainer AssetTags;
	AssetTags.Added.AddTag(FGameplayTag::RequestGameplayTag(FName("Cooldown.ActiveSkill")));

	AssetTagsComp->SetAndApplyAssetTagChanges(AssetTags);

	// ⭐ GEComponents에 등록 (Ensure 에러 수정!)
	GEComponents.Add(AssetTagsComp);

	// 스택 정책: 쿨다운은 스택 안됨
	StackingType = EGameplayEffectStackingType::None;
}
