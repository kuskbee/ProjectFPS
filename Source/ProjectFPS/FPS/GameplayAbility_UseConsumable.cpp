// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbility_UseConsumable.h"
#include "Items/ConsumableItemData.h"
#include "Components/InventoryComponent.h"
#include "FPSPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameplayTags.h"
#include "CharacterAttributeSet.h"
#include "GameplayEffect_InstantHeal.h"

UGameplayAbility_UseConsumable::UGameplayAbility_UseConsumable()
{
	// InstancedPerActor: 액터당 하나의 인스턴스 (상태 유지 가능)
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// Tag 설정
	FGameplayTagContainer Tags;
	Tags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.UseConsumable")));
	SetAssetTags(Tags);
}

void UGameplayAbility_UseConsumable::SetConsumableData(UConsumableItemData* InConsumableData, int32 InGridX, int32 InGridY)
{
	ConsumableData = InConsumableData;
	GridX = InGridX;
	GridY = InGridY;
}

bool UGameplayAbility_UseConsumable::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                         const FGameplayAbilityActorInfo* ActorInfo,
                                                         const FGameplayTagContainer* SourceTags,
                                                         const FGameplayTagContainer* TargetTags,
                                                         FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// ConsumableData 체크
	if (!ConsumableData || !ConsumableData->ConsumableEffect)
	{
		UE_LOG(LogTemp, Warning, TEXT("CanActivateAbility: ConsumableData 또는 ConsumableEffect가 없습니다."));
		return false;
	}

	// 회복 포션인 경우 체력이 꽉 찼는지 체크
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (ASC)
	{
		// ConsumableEffect가 GameplayEffect_InstantHeal인지 체크 (IsA 관계)
		if (ConsumableData->ConsumableEffect && ConsumableData->ConsumableEffect->IsChildOf(UGameplayEffect_InstantHeal::StaticClass()))
		{
			// Health와 MaxHealth 가져오기
			const float CurrentHealth = ASC->GetNumericAttribute(UCharacterAttributeSet::GetHealthAttribute());
			const float MaxHealth = ASC->GetNumericAttribute(UCharacterAttributeSet::GetMaxHealthAttribute());

			// Health가 이미 최대치면 사용 불가
			if (CurrentHealth >= MaxHealth)
			{
				UE_LOG(LogTemp, Warning, TEXT("CanActivateAbility: 체력이 가득 차서 포션을 사용할 수 없습니다."));
				return false;
			}
		}
	}

	return true;
}

void UGameplayAbility_UseConsumable::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                      const FGameplayAbilityActorInfo* ActorInfo,
                                                      const FGameplayAbilityActivationInfo ActivationInfo,
                                                      const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Owner 확인
	AFPSPlayerCharacter* PlayerCharacter = Cast<AFPSPlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("ActivateAbility: PlayerCharacter를 찾을 수 없습니다."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UInventoryComponent* InventoryComp = PlayerCharacter->GetInventoryComponent();
	if (!InventoryComp)
	{
		UE_LOG(LogTemp, Error, TEXT("ActivateAbility: InventoryComponent를 찾을 수 없습니다."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. GameplayEffect 적용
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (ASC && ConsumableData && ConsumableData->ConsumableEffect)
	{
		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
			ConsumableData->ConsumableEffect,
			1.0f,
			ContextHandle
		);

		if (SpecHandle.IsValid())
		{
			// SetByCaller로 회복량 전달
			SpecHandle.Data->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.HealAmount")),
				ConsumableData->EffectMagnitude
			);

			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

			UE_LOG(LogTemp, Log, TEXT("포션 사용: %s (회복량: %.1f)"),
			       *ConsumableData->GetItemName(),
			       ConsumableData->EffectMagnitude);
		}
	}

	// 2. 인벤토리에서 스택 감소 (1개 사용)
	if (GridX >= 0 && GridY >= 0)
	{
		InventoryComp->DecreaseStackAt(GridX, GridY, 1);
		UE_LOG(LogTemp, Log, TEXT("인벤토리에서 포션 1개 사용: (%d, %d)"), GridX, GridY);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
