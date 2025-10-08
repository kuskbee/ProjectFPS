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
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 방어막 HP 초기화
	CurrentBarrierHealth = BarrierHealth;

	// 방어막 구체 생성
	SpawnBarrierVisual();

	// 쿨다운 적용
	if (CooldownEffect && ActorInfo->AbilitySystemComponent.IsValid())
	{
		FGameplayEffectContextHandle ContextHandle = ActorInfo->AbilitySystemComponent->MakeEffectContext();
		ContextHandle.AddInstigator(ActorInfo->OwnerActor.Get(), ActorInfo->AvatarActor.Get());

		FGameplayEffectSpecHandle SpecHandle = ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(
			CooldownEffect, 1.0f, ContextHandle);

		if (SpecHandle.IsValid())
		{
			// SetByCaller로 쿨다운 시간 설정
			SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Cooldown")), CooldownDuration);

			// 동적 Cooldown Tag 추가
			SpecHandle.Data->DynamicGrantedTags.AppendTags(CooldownTags);

			// Effect 적용
			ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			UE_LOG(LogTemp, Log, TEXT("쿨다운 적용: %.0f초"), CooldownDuration);
		}
	}

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

	UE_LOG(LogTemp, Log, TEXT("방어막 스킬 활성화 (HP: %.0f, 지속시간: %.1f초)"), BarrierHealth, BarrierDuration);
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
