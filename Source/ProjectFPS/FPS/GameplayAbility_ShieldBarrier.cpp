// GameplayAbility_ShieldBarrier.cpp

#include "GameplayAbility_ShieldBarrier.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

UGameplayAbility_ShieldBarrier::UGameplayAbility_ShieldBarrier()
{
	// Ability 기본 설정
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// Tag 설정 (SetAssetTags 사용)
	FGameplayTagContainer Tags;
	Tags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.ShieldBarrier")));
	SetAssetTags(Tags);
}

void UGameplayAbility_ShieldBarrier::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// ⭐ CommitAbility() 대신 CommitCheck()만 호출 (쿨다운 자동 적용 방지)
	if (!CommitCheck(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Cost 수동 적용 (있다면)
	CommitExecute(Handle, ActorInfo, ActivationInfo);

	// ⭐ 쿨다운 수동 적용 (EndAbility()에서 제거되지 않도록!)
	ApplyCooldown(Handle, ActorInfo, ActivationInfo);

	// 방어막 HP 초기화
	CurrentBarrierHealth = BarrierHealth;

	// 방어막 구체 생성
	SpawnBarrierVisual();

	// 지속 시간 타이머 설정
	FTimerHandle BarrierTimerHandle;
	if (AActor* OwnerActor = ActorInfo->OwnerActor.Get())
	{
		OwnerActor->GetWorldTimerManager().SetTimer(
			BarrierTimerHandle,
			this,
			&UGameplayAbility_ShieldBarrier::OnBarrierExpired,
			BarrierDuration,
			false
		);
	}

	UE_LOG(LogTemp, Log, TEXT("방어막 스킬 활성화 (HP: %.0f, 지속시간: %.1f초, 쿨다운: %.1f초)"), BarrierHealth, BarrierDuration, CooldownDuration);
}

void UGameplayAbility_ShieldBarrier::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	if (CooldownGE && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		// ⭐ ApplyGameplayEffectToOwner 대신 직접 구현 (SetByCaller 추가)
		// HasAuthorityOrPredictionKey 체크 포함
		if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
		{
			FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(
				Handle, ActorInfo, ActivationInfo, CooldownGE->GetClass(), GetAbilityLevel(Handle, ActorInfo));

			if (SpecHandle.IsValid())
			{
				// ⭐ SetByCaller로 쿨다운 시간 전달!
				SpecHandle.Data->SetSetByCallerMagnitude(
					FGameplayTag::RequestGameplayTag(FName("Data.Cooldown")),
					CooldownDuration
				);

				// Cooldown.ActiveSkill 태그는 GameplayEffect_Cooldown 생성자에서 자동 추가됨!

				// 쿨다운 Effect 적용
				ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);

				UE_LOG(LogTemp, Log, TEXT("방어막 쿨다운 적용: %.0f초"), CooldownDuration);
			}
		}
	}
}

bool UGameplayAbility_ShieldBarrier::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	// 부모 클래스 체크 (기본 조건들)
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// ⭐ 쿨다운 체크 (헬퍼 함수로 리팩토링)
	if (IsOnCooldown(ActorInfo))
	{
		UE_LOG(LogTemp, Warning, TEXT("방어막 스킬 쿨다운 중! 사용 불가"));
		return false;
	}

	return true;
}

bool UGameplayAbility_ShieldBarrier::IsOnCooldown(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	// ASC에서 "Cooldown.ActiveSkill" 태그를 가진 Effect 쿼리
	FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(FName("Cooldown.ActiveSkill"));
	FGameplayEffectQuery Query;
	Query.EffectTagQuery = FGameplayTagQuery::MakeQuery_MatchAnyTags(FGameplayTagContainer(CooldownTag));

	TArray<FActiveGameplayEffectHandle> ActiveCooldowns = ActorInfo->AbilitySystemComponent->GetActiveEffects(Query);

	return ActiveCooldowns.Num() > 0;
}

void UGameplayAbility_ShieldBarrier::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	// 방어막 구체 제거
	RemoveBarrierVisual();

	UE_LOG(LogTemp, Log, TEXT("방어막 스킬 종료"));

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_ShieldBarrier::SpawnBarrierVisual()
{
	if (!BarrierActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("BarrierActorClass가 설정되지 않음"));
		return;
	}

	AActor* OwnerActor = GetAvatarActorFromActorInfo();
	if (!OwnerActor)
	{
		return;
	}

	// 방어막 구체 생성
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerActor;
	SpawnParams.Instigator = Cast<APawn>(OwnerActor);

	ActiveBarrierActor = GetWorld()->SpawnActor<AActor>(
		BarrierActorClass,
		OwnerActor->GetActorLocation(),
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (ActiveBarrierActor)
	{
		// 캐릭터에 부착
		ActiveBarrierActor->AttachToActor(OwnerActor, FAttachmentTransformRules::SnapToTargetIncludingScale);
		UE_LOG(LogTemp, Log, TEXT("방어막 구체 생성 완료"));
	}
}

void UGameplayAbility_ShieldBarrier::RemoveBarrierVisual()
{
	if (ActiveBarrierActor)
	{
		ActiveBarrierActor->Destroy();
		ActiveBarrierActor = nullptr;
		UE_LOG(LogTemp, Log, TEXT("방어막 구체 제거"));
	}
}

void UGameplayAbility_ShieldBarrier::OnBarrierExpired()
{
	UE_LOG(LogTemp, Log, TEXT("방어막 지속 시간 만료"));

	// Ability 종료
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
