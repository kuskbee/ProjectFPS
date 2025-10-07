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

	/** 쿨다운 시간 (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shield Barrier")
	float CooldownDuration = 60.0f;

	/** 쿨다운 GameplayEffect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shield Barrier")
	TSubclassOf<UGameplayEffect> CooldownEffect;

	/** 쿨다운 Tag (UI 표시용) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shield Barrier")
	FGameplayTagContainer CooldownTags;

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
};
