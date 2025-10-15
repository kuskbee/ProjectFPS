// GameplayAbility_Berserker.cpp

#include "GameplayAbility_Berserker.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

UGameplayAbility_Berserker::UGameplayAbility_Berserker()
{
	// Ability 기본 설정
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// Tag 설정 (SetAssetTags 사용)
	FGameplayTagContainer Tags;
	Tags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Berserker")));
	SetAssetTags(Tags);
}

void UGameplayAbility_Berserker::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// ⭐ CommitAbility() 대신 CommitCheck()만 호출 (쿨다운 자동 적용 방지)
	if (!CommitCheck(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Cost 수동 적용 (있다면) & 내부에서 쿨다운 적용
	CommitExecute(Handle, ActorInfo, ActivationInfo);

	// 버서커 버프 적용
	ApplyBerserkerBuff();

	// 오오라 생성 (선택)
	SpawnAuraVisual();

	// 지속 시간 타이머 설정
	FTimerHandle BerserkerTimerHandle;
	if (AActor* OwnerActor = ActorInfo->OwnerActor.Get())
	{
		OwnerActor->GetWorldTimerManager().SetTimer(
			BerserkerTimerHandle,
			this,
			&UGameplayAbility_Berserker::OnBerserkerExpired,
			BerserkerDuration,
			false
		);
	}

	UE_LOG(LogTemp, Warning, TEXT("버서커 스킬 활성화! (지속시간: %.1f초, 쿨다운: %.1f초, 공격속도/이동속도 증가)"), BerserkerDuration, CooldownDuration);
}

void UGameplayAbility_Berserker::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	if (CooldownGE && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
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

				UE_LOG(LogTemp, Log, TEXT("버서커 쿨다운 적용: %.0f초"), CooldownDuration);
			}
		}
	}
}

bool UGameplayAbility_Berserker::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
		UE_LOG(LogTemp, Warning, TEXT("버서커 스킬 쿨다운 중! 사용 불가"));
		return false;
	}

	return true;
}

bool UGameplayAbility_Berserker::IsOnCooldown(const FGameplayAbilityActorInfo* ActorInfo) const
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

void UGameplayAbility_Berserker::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	// 버프 제거
	RemoveBerserkerBuff();

	// 오오라 제거
	RemoveAuraVisual();

	UE_LOG(LogTemp, Log, TEXT("버서커 스킬 종료"));

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_Berserker::ApplyBerserkerBuff()
{
	if (!BerserkerBuffEffect)
	{
		UE_LOG(LogTemp, Warning, TEXT("BerserkerBuffEffect가 설정되지 않음"));
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	// 버프 Effect 적용
	FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
	ContextHandle.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
		BerserkerBuffEffect, 1.0f, ContextHandle);

	if (SpecHandle.IsValid())
	{
		ActiveBuffHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		UE_LOG(LogTemp, Log, TEXT("버서커 버프 적용 완료"));
	}
}

void UGameplayAbility_Berserker::RemoveBerserkerBuff()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC && ActiveBuffHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(ActiveBuffHandle);
		UE_LOG(LogTemp, Log, TEXT("버서커 버프 제거"));
	}
}

void UGameplayAbility_Berserker::SpawnAuraVisual()
{
	if (!AuraActorClass)
	{
		// 오오라는 선택사항이므로 경고 없음
		return;
	}

	AActor* OwnerActor = GetAvatarActorFromActorInfo();
	if (!OwnerActor)
	{
		return;
	}

	// 오오라 Actor 생성
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerActor;
	SpawnParams.Instigator = Cast<APawn>(OwnerActor);

	ActiveAuraActor = GetWorld()->SpawnActor<AActor>(
		AuraActorClass,
		OwnerActor->GetActorLocation(),
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (ActiveAuraActor)
	{
		// 캐릭터에 부착
		ActiveAuraActor->AttachToActor(OwnerActor, FAttachmentTransformRules::SnapToTargetIncludingScale);
		UE_LOG(LogTemp, Log, TEXT("버서커 오오라 생성 완료"));
	}
}

void UGameplayAbility_Berserker::RemoveAuraVisual()
{
	if (ActiveAuraActor)
	{
		ActiveAuraActor->Destroy();
		ActiveAuraActor = nullptr;
		UE_LOG(LogTemp, Log, TEXT("버서커 오오라 제거"));
	}
}

void UGameplayAbility_Berserker::OnBerserkerExpired()
{
	UE_LOG(LogTemp, Log, TEXT("버서커 지속 시간 만료"));

	// Ability 종료
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
