// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/GameplayAbility_Reload.h"
#include "FPS/Weapons/FPSWeapon.h"
#include "FPS/Weapons/FPSWeaponHolder.h"
#include "FPS/Components/WeaponSlotComponent.h"
#include "FPS/Items/WeaponItemData.h"
#include "FPS/FPSCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"

UGameplayAbility_Reload::UGameplayAbility_Reload()
{
	// 어빌리티 기본 설정
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// 리로드는 일반적으로 블로킹 어빌리티
	bRetriggerInstancedAbility = false;
}

bool UGameplayAbility_Reload::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// 이미 리로드 중이면 활성화 불가
	if (bIsReloading)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameplayAbility_Reload: 이미 리로드 중입니다 (bIsReloading = true)"));
		return false;
	}

	// WeaponHolder 인터페이스 구현 확인
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return false;
	}

	IFPSWeaponHolder* WeaponHolder = Cast<IFPSWeaponHolder>(ActorInfo->AvatarActor.Get());
	if (!WeaponHolder)
	{
		return false;
	}

	// WeaponSlotComponent 확인
	UWeaponSlotComponent* WeaponSlotComp = ActorInfo->AvatarActor->FindComponentByClass<UWeaponSlotComponent>();
	if (!WeaponSlotComp)
	{
		return false;
	}

	// 현재 활성화된 무기 확인
	AFPSWeapon* CurrentWeapon = WeaponSlotComp->GetCurrentWeaponActor();
	if (!CurrentWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameplayAbility_Reload: 현재 활성화된 무기가 없습니다"));
		return false;
	}

	// 무기에 리로드 몽타주가 있는지 확인
	UAnimMontage* ReloadMontage = CurrentWeapon->GetReloadMontage();
	if (!ReloadMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameplayAbility_Reload: 무기에 리로드 몽타주가 설정되지 않았습니다"));
		return false;
	}

	// 무기 아이템 데이터 확인
	UWeaponItemData* WeaponData = WeaponSlotComp->GetActiveWeaponItem();
	if (!WeaponData)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameplayAbility_Reload: 무기 아이템 데이터가 없습니다"));
		return false;
	}

	// 이미 탄약이 가득 찬 경우 리로드 불가
	if (WeaponData->IsAmmoFull())
	{
		UE_LOG(LogTemp, Log, TEXT("GameplayAbility_Reload: 탄약이 이미 가득 참 (%d/%d)"),
			WeaponData->CurrentAmmo, WeaponData->MagazineSize);
		return false;
	}

	return true;
}

void UGameplayAbility_Reload::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bIsReloading = true;

	// WeaponSlotComponent에서 현재 무기 가져오기
	UWeaponSlotComponent* WeaponSlotComp = ActorInfo->AvatarActor->FindComponentByClass<UWeaponSlotComponent>();
	if (!WeaponSlotComp)
	{
		UE_LOG(LogTemp, Error, TEXT("GameplayAbility_Reload: WeaponSlotComponent를 찾을 수 없습니다"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ReloadingWeapon = WeaponSlotComp->GetCurrentWeaponActor();
	if (!ReloadingWeapon)
	{
		UE_LOG(LogTemp, Error, TEXT("GameplayAbility_Reload: 현재 활성화된 무기가 없습니다"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 리로드 몽타주 가져오기
	UAnimMontage* ReloadMontage = ReloadingWeapon->GetReloadMontage();
	if (!ReloadMontage)
	{
		UE_LOG(LogTemp, Error, TEXT("GameplayAbility_Reload: 리로드 몽타주가 설정되지 않았습니다 - 무기: %s"),
			*ReloadingWeapon->GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("GameplayAbility_Reload: 리로드 몽타주 확인됨 - %s"),
		*ReloadMontage->GetName());

	// WeaponHolder 인터페이스를 통해 애니메이션 재생
	IFPSWeaponHolder* WeaponHolder = Cast<IFPSWeaponHolder>(ActorInfo->AvatarActor.Get());
	if (!WeaponHolder)
	{
		UE_LOG(LogTemp, Error, TEXT("GameplayAbility_Reload: AvatarActor가 IFPSWeaponHolder를 구현하지 않습니다"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// WeaponHolder를 통해 1인칭/3인칭 메시에서 모두 애니메이션 재생
	WeaponHolder->PlayReloadMontage(ReloadMontage);

	// 표준 AbilityTask 사용 (3인칭 메시 기반으로 타이밍 계산)
	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		FName("ReloadMontage"),
		ReloadMontage,
		1.0f, // 재생 속도
		NAME_None, // 시작 섹션
		true, // 스톱되면 어빌리티 종료
		1.0f, // 애니메이션 변경 시 블렌드 시간
		0.0f // 시작 시 블렌드 시간
	);

	if (!MontageTask)
	{
		UE_LOG(LogTemp, Error, TEXT("GameplayAbility_Reload: 몽타주 태스크 생성 실패"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 몽타주 이벤트 바인딩
	MontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_Reload::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_Reload::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_Reload::OnMontageInterrupted);

	// 몽타주 재생 시작
	MontageTask->ReadyForActivation();

	UE_LOG(LogTemp, Log, TEXT("GameplayAbility_Reload: 리로드 시작 - %s"),
		*ReloadingWeapon->GetName());
}

void UGameplayAbility_Reload::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UE_LOG(LogTemp, Warning, TEXT("GameplayAbility_Reload: EndAbility 호출됨 - 취소: %s, 이전 bIsReloading: %s"),
		bWasCancelled ? TEXT("true") : TEXT("false"),
		bIsReloading ? TEXT("true") : TEXT("false"));

	// 상태 초기화
	bIsReloading = false;
	ReloadingWeapon = nullptr;

	// 몽타주 태스크 정리
	if (MontageTask)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameplayAbility_Reload: MontageTask 정리 중"));
		MontageTask->EndTask();
		MontageTask = nullptr;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GameplayAbility_Reload: MontageTask가 이미 null"));
	}

	UE_LOG(LogTemp, Warning, TEXT("GameplayAbility_Reload: 리로드 종료 완료 - bIsReloading을 false로 설정"));

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_Reload::OnMontageCompleted()
{
	UE_LOG(LogTemp, Log, TEXT("GameplayAbility_Reload: 몽타주 재생 완료"));

	// 정상적으로 리로드 완료
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGameplayAbility_Reload::OnMontageCancelled()
{
	UE_LOG(LogTemp, Log, TEXT("GameplayAbility_Reload: 몽타주 재생 취소"));

	// 리로드 취소됨
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGameplayAbility_Reload::OnMontageInterrupted()
{
	UE_LOG(LogTemp, Log, TEXT("GameplayAbility_Reload: 몽타주 재생 중단"));

	// 리로드 중단됨
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}