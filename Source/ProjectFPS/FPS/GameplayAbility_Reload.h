// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbility_Reload.generated.h"

class UAnimMontage;
class AFPSWeapon;
class UWeaponItemData;

/**
 * GAS 기반 리로드 GameplayAbility
 * 현재 무기의 탄약을 보충하고 리로드 애니메이션을 재생
 * AnimNotify_RefillAmmo와 연동하여 정확한 타이밍에 탄약 보충
 */
UCLASS()
class PROJECTFPS_API UGameplayAbility_Reload : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_Reload();

protected:
	/** 리로드 가능 여부 확인 */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

	/** 리로드 어빌리티 활성화 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** 리로드 어빌리티 종료 */
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** 몽타주 재생 완료 시 호출 */
	UFUNCTION()
	void OnMontageCompleted();

	/** 몽타주 재생 취소 시 호출 */
	UFUNCTION()
	void OnMontageCancelled();

	/** 몽타주 재생 중단 시 호출 */
	UFUNCTION()
	void OnMontageInterrupted();

private:
	/** 현재 재생 중인 몽타주 태스크 */
	UPROPERTY()
	class UAbilityTask_PlayMontageAndWait* MontageTask;

	/** 리로드 중인지 여부 */
	bool bIsReloading = false;

	/** 현재 리로드 중인 무기 */
	UPROPERTY()
	TObjectPtr<AFPSWeapon> ReloadingWeapon;
};