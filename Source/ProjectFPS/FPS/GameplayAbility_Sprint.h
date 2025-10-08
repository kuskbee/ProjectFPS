// GameplayAbility_Sprint.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbility_Sprint.generated.h"

/**
 * 스태미나를 소모하며 질주하는 Ability
 * Shift 키를 누르는 동안 활성화되며, 이동 속도가 증가합니다.
 *
 * GAS 학습 포인트:
 * 1. Duration GameplayEffect: 지속 시간 동안 효과가 유지됨
 * 2. Periodic GameplayEffect: 주기적으로 스태미나를 감소시킴
 * 3. CanActivateAbility: 스태미나가 0 이하일 때 질주 불가
 * 4. EndAbility: Shift 키를 떼면 질주 종료
 */
UCLASS()
class PROJECTFPS_API UGameplayAbility_Sprint : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_Sprint();

	// Ability 활성화 시 호출
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	// Ability 종료 시 호출
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;

	// Ability 활성화 가능 여부 검사
	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags,
		const FGameplayTagContainer* TargetTags,
		FGameplayTagContainer* OptionalRelevantTags
	) const override;

protected:
	// 질주 시 이동 속도 증가량 (기본값: 1.0, 즉 +100% → 2배)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprint")
	float SprintSpeedBoost = 1.0f;

	// 이동 속도 증가 GameplayEffect (Infinite)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprint")
	TSubclassOf<class UGameplayEffect> SprintSpeedEffect;

	// 스태미나 소모 GameplayEffect (Periodic)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprint")
	TSubclassOf<class UGameplayEffect> StaminaDrainEffect;

	// 스태미나 회복 GameplayEffect (Periodic, Infinite)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprint")
	TSubclassOf<class UGameplayEffect> StaminaRecoverEffect;

private:
	// 적용된 SprintSpeed Effect의 핸들 (Sprint 종료 시 제거용)
	FActiveGameplayEffectHandle ActiveSprintSpeedHandle;

	// 적용된 StaminaDrain Effect의 핸들 (Sprint 종료 시 제거용)
	FActiveGameplayEffectHandle ActiveStaminaDrainHandle;

	// 적용된 StaminaRecover Effect의 핸들 (Sprint 시작 시 제거용)
	FActiveGameplayEffectHandle ActiveStaminaRecoverHandle;
};
