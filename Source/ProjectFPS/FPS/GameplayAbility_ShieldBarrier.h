// GameplayAbility_ShieldBarrier.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbility_ShieldBarrier.generated.h"

class UGameplayEffect;

/**
 * 방어막 스킬 (Shield 브랜치 Tier 5)
 * - Q키로 활성화
 * - 반투명 구체 비주얼 생성
 * - 방어막이 대미지 흡수
 * - 지속 시간 또는 HP 소진 시 자동 해제
 */
UCLASS()
class PROJECTFPS_API UGameplayAbility_ShieldBarrier : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_ShieldBarrier();

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
	/** 방어막 지속 시간 (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shield Barrier")
	float BarrierDuration = 20.0f;

	/** 방어막 HP (대미지 흡수량) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shield Barrier")
	float BarrierHealth = 200.0f;

	/** 방어막 구체 Actor 클래스 (Blueprint에서 설정) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shield Barrier")
	TSubclassOf<AActor> BarrierActorClass;

	/** 쿨다운 시간 (초) - SetByCaller로 전달 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cooldown")
	float CooldownDuration = 60.0f;

	// 표준 GAS 쿨다운 시스템 사용 (GetCooldownTags() 자동 지원)
	// CooldownGameplayEffectClass는 UGameplayAbility 부모 클래스에서 상속받음
	// Blueprint에서 "Cooldown" 카테고리에 GE_Cooldown 할당

private:
	/** 현재 활성화된 방어막 구체 */
	UPROPERTY()
	TObjectPtr<AActor> ActiveBarrierActor;

	/** 현재 방어막 HP */
	float CurrentBarrierHealth;

	/** 방어막 구체 생성 */
	void SpawnBarrierVisual();

	/** 방어막 구체 제거 */
	void RemoveBarrierVisual();

	/** 타이머 콜백: 방어막 종료 */
	void OnBarrierExpired();

	/** 쿨다운 체크 헬퍼 함수 (코드 중복 제거) */
	bool IsOnCooldown(const FGameplayAbilityActorInfo* ActorInfo) const;
};
