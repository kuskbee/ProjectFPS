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

	/** 쿨다운 적용 오버라이드 - SetByCaller로 동적 시간 전달 */
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;

	/** 활성화 가능 여부 체크 - 쿨다운 중이면 막음 */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	/** 버서커 지속 시간 (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Berserker")
	float BerserkerDuration = 10.0f;

	/** 버서커 버프 GameplayEffect (공격속도/이동속도 증가) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Berserker")
	TSubclassOf<UGameplayEffect> BerserkerBuffEffect;

	/** 쿨다운 시간 (초) - SetByCaller로 전달 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cooldown")
	float CooldownDuration = 30.0f;

	// ⭐ 표준 GAS 쿨다운 시스템 사용 (GetCooldownTags() 자동 지원)
	// CooldownGameplayEffectClass는 UGameplayAbility 부모 클래스에서 상속받음
	// Blueprint에서 "Cooldown" 카테고리에 GE_Cooldown 할당

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

	/** 쿨다운 체크 헬퍼 함수 (코드 중복 제거) */
	bool IsOnCooldown(const FGameplayAbilityActorInfo* ActorInfo) const;
};
