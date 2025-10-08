// GameplayAbility_Berserker.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbility_Berserker.generated.h"

class UGameplayEffect;

/**
 * 버서커 스킬 (Crit 브랜치 Tier 5)
 * - Q키로 활성화
 * - 10초간 공격속도 +50%, 이동속도 +30%
 * - 붉은 오오라 이펙트 (선택)
 * - 쿨다운 30초
 */
UCLASS()
class PROJECTFPS_API UGameplayAbility_Berserker : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_Berserker();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** 버서커 지속 시간 (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Berserker")
	float BerserkerDuration = 10.0f;

	/** 버서커 버프 GameplayEffect (공격속도/이동속도 증가) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Berserker")
	TSubclassOf<UGameplayEffect> BerserkerBuffEffect;

	/** 쿨다운 시간 (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Berserker")
	float CooldownDuration = 30.0f;

	/** 쿨다운 GameplayEffect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Berserker")
	TSubclassOf<UGameplayEffect> CooldownEffect;

	/** 쿨다운 Tag (UI 표시용) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Berserker")
	FGameplayTagContainer CooldownTags;

	/** 버서커 오오라 Actor 클래스 (선택, Blueprint에서 설정) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Berserker")
	TSubclassOf<AActor> AuraActorClass;

private:
	/** 현재 적용된 버프 Effect Handle */
	FActiveGameplayEffectHandle ActiveBuffHandle;

	/** 현재 활성화된 오오라 Actor */
	UPROPERTY()
	TObjectPtr<AActor> ActiveAuraActor;

	/** 버프 적용 */
	void ApplyBerserkerBuff();

	/** 버프 제거 */
	void RemoveBerserkerBuff();

	/** 오오라 생성 (선택) */
	void SpawnAuraVisual();

	/** 오오라 제거 */
	void RemoveAuraVisual();

	/** 타이머 콜백: 버서커 종료 */
	void OnBerserkerExpired();
};
