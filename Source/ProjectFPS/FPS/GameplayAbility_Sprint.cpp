// GameplayAbility_Sprint.cpp

#include "GameplayAbility_Sprint.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "CharacterAttributeSet.h"

UGameplayAbility_Sprint::UGameplayAbility_Sprint()
{
	// Ability 기본 설정
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// Ability 태그 설정 (UE 5.6+ 방식)
	FGameplayTagContainer Tags;
	Tags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Sprint")));
	SetAssetTags(Tags);
}

void UGameplayAbility_Sprint::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character || !Character->GetCharacterMovement())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. 기본 이동 속도 저장
	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	OriginalMaxWalkSpeed = MovementComp->MaxWalkSpeed;

	// 2. 이동 속도 증가
	MovementComp->MaxWalkSpeed = OriginalMaxWalkSpeed * SprintSpeedMultiplier;

	// 3. 스태미나 회복 Effect 제거 (Sprint 중에는 회복 안됨)
	if (ActiveStaminaRecoverHandle.IsValid() && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveStaminaRecoverHandle);
		ActiveStaminaRecoverHandle.Invalidate();
		UE_LOG(LogTemp, Log, TEXT("Sprint 시작: 스태미나 회복 중단"));
	}

	// 4. 스태미나 소모 GameplayEffect 적용 (Periodic)
	if (StaminaDrainEffect)
	{
		FGameplayEffectContextHandle ContextHandle = ActorInfo->AbilitySystemComponent->MakeEffectContext();
		ContextHandle.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(
			StaminaDrainEffect,
			GetAbilityLevel(),
			ContextHandle
		);

		if (SpecHandle.IsValid())
		{
			ActiveStaminaDrainHandle = ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(
				*SpecHandle.Data.Get()
			);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Sprint 시작: 이동속도 %.0f -> %.0f"),
		OriginalMaxWalkSpeed, MovementComp->MaxWalkSpeed);
}

void UGameplayAbility_Sprint::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (Character && Character->GetCharacterMovement())
	{
		// 1. 이동 속도 복구
		Character->GetCharacterMovement()->MaxWalkSpeed = OriginalMaxWalkSpeed;
		UE_LOG(LogTemp, Log, TEXT("Sprint 종료: 이동속도 복구 %.0f"), OriginalMaxWalkSpeed);
	}

	// 2. 스태미나 소모 Effect 제거
	if (ActiveStaminaDrainHandle.IsValid() && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveStaminaDrainHandle);
		ActiveStaminaDrainHandle.Invalidate();
	}

	// 3. 스태미나 회복 Effect 적용 (Periodic, Infinite)
	if (StaminaRecoverEffect && ActorInfo->AbilitySystemComponent.IsValid())
	{
		FGameplayEffectContextHandle ContextHandle = ActorInfo->AbilitySystemComponent->MakeEffectContext();
		ContextHandle.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(
			StaminaRecoverEffect,
			GetAbilityLevel(),
			ContextHandle
		);

		if (SpecHandle.IsValid())
		{
			// 핸들 저장 (다음 Sprint 시작 시 제거하기 위해)
			ActiveStaminaRecoverHandle = ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			UE_LOG(LogTemp, Log, TEXT("Sprint 종료: 스태미나 회복 시작"));
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UGameplayAbility_Sprint::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// 스태미나가 0 이하면 질주 불가
	if (ActorInfo->AbilitySystemComponent.IsValid())
	{
		const UCharacterAttributeSet* AttributeSet = ActorInfo->AbilitySystemComponent->GetSet<UCharacterAttributeSet>();
		if (AttributeSet && AttributeSet->GetStamina() <= 0.0f)
		{
			UE_LOG(LogTemp, Warning, TEXT("Sprint 불가: 스태미나 부족 (%.1f)"), AttributeSet->GetStamina());
			return false;
		}
	}

	return true;
}
