// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/Components/SkillComponent.h"
#include "FPS/Skills/BaseSkillData.h"
#include "FPS/PlayerAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

USkillComponent::USkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USkillComponent::BeginPlay()
{
	Super::BeginPlay();

	// SkillDataArray → SkillDataMap 변환 (런타임 최적화)
	for (UBaseSkillData* SkillData : SkillDataArray)
	{
		if (SkillData && SkillData->SkillID.IsValid())
		{
			SkillDataMap.Add(SkillData->SkillID, SkillData);
			UE_LOG(LogTemp, Log, TEXT("스킬 등록: %s"), *SkillData->SkillID.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("유효하지 않은 스킬 데이터 발견"));
		}
	}

	UE_LOG(LogTemp, Log, TEXT("SkillComponent 초기화 완료: %d개 스킬 등록"), SkillDataMap.Num());

	// AbilitySystemComponent 캐싱
	if (AActor* Owner = GetOwner())
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner))
		{
			CachedASC = ASI->GetAbilitySystemComponent();
		}
	}
}

ESkillAcquireResult USkillComponent::TryAcquireSkill(const FGameplayTag& SkillID)
{
	// 1. 스킬 데이터 찾기
	UBaseSkillData* SkillData = FindSkillData(SkillID);
	if (!SkillData)
	{
		UE_LOG(LogTemp, Warning, TEXT("스킬을 찾을 수 없음: %s"), *SkillID.ToString());
		return ESkillAcquireResult::InvalidSkill;
	}

	// 2. 이미 습득했는지 확인
	if (HasSkill(SkillID))
	{
		UE_LOG(LogTemp, Warning, TEXT("이미 습득한 스킬: %s"), *SkillData->SkillName.ToString());
		return ESkillAcquireResult::AlreadyAcquired;
	}

	// 3. 습득 가능 여부 확인 (선행 스킬, 스킬 포인트)
	if (!CanAcquireSkill(SkillID))
	{
		UE_LOG(LogTemp, Warning, TEXT("스킬 습득 조건 미충족: %s"), *SkillData->SkillName.ToString());
		return ESkillAcquireResult::PrerequisiteNotMet;
	}

	// 4. 스킬 포인트 확인 및 소모
	if (!CachedASC.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("AbilitySystemComponent를 찾을 수 없음"));
		return ESkillAcquireResult::InvalidSkill;
	}

	const UPlayerAttributeSet* PlayerAttrSet = CachedASC->GetSet<UPlayerAttributeSet>();
	if (!PlayerAttrSet)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerAttributeSet를 찾을 수 없음"));
		return ESkillAcquireResult::InvalidSkill;
	}

	float CurrentSkillPoints = PlayerAttrSet->GetSkillPoint();
	if (CurrentSkillPoints < SkillData->RequiredSkillPoints)
	{
		UE_LOG(LogTemp, Warning, TEXT("스킬 포인트 부족: 필요 %d, 보유 %.0f"),
			SkillData->RequiredSkillPoints, CurrentSkillPoints);
		return ESkillAcquireResult::InsufficientPoints;
	}

	// 5. 스킬 포인트 소모 (GameplayEffect로 처리해야 하지만 지금은 직접 설정)
	// TODO: GameplayEffect로 SkillPoint 감소 처리
	UPlayerAttributeSet* MutableAttrSet = const_cast<UPlayerAttributeSet*>(PlayerAttrSet);
	MutableAttrSet->SetSkillPoint(CurrentSkillPoints - SkillData->RequiredSkillPoints);

	// 6. 스킬 효과 적용
	ApplySkillEffects(SkillData);

	// 7. 습득한 스킬 목록에 추가
	AcquiredSkills.Add(SkillID);

	UE_LOG(LogTemp, Log, TEXT("스킬 습득 성공: %s (남은 포인트: %.0f)"),
		*SkillData->SkillName.ToString(), MutableAttrSet->GetSkillPoint());

	return ESkillAcquireResult::Success;
}

bool USkillComponent::HasSkill(const FGameplayTag& SkillID) const
{
	return AcquiredSkills.Contains(SkillID);
}

bool USkillComponent::CanAcquireSkill(const FGameplayTag& SkillID) const
{
	UBaseSkillData* SkillData = FindSkillData(SkillID);
	if (!SkillData)
	{
		return false;
	}

	// 1. 선행 스킬 확인 (OR 조건: 하나라도 습득했으면 통과)
	if (SkillData->PrerequisiteSkills.Num() > 0)
	{
		bool bHasAnyPrerequisite = false;
		for (const FGameplayTag& PrereqSkill : SkillData->PrerequisiteSkills)
		{
			if (HasSkill(PrereqSkill))
			{
				bHasAnyPrerequisite = true;
				break;
			}
		}

		if (!bHasAnyPrerequisite)
		{
			UE_LOG(LogTemp, VeryVerbose, TEXT("선행 스킬 조건 미충족"));
			return false;
		}
	}

	// 2. 상호배타적 스킬 확인 (이미 습득한 스킬과 충돌하는지 체크)
	for (const FGameplayTag& ExclusiveSkill : SkillData->MutuallyExclusiveSkills)
	{
		if (HasSkill(ExclusiveSkill))
		{
			UE_LOG(LogTemp, Warning, TEXT("상호배타적 스킬 이미 습득됨: %s"), *ExclusiveSkill.ToString());
			return false;
		}
	}

	// 3. 역방향 체크: 이미 습득한 스킬 중에 이 스킬을 배타적으로 막는 스킬이 있는지
	for (const FGameplayTag& AcquiredSkillID : AcquiredSkills)
	{
		UBaseSkillData* AcquiredSkillData = FindSkillData(AcquiredSkillID);
		if (AcquiredSkillData && AcquiredSkillData->MutuallyExclusiveSkills.Contains(SkillID))
		{
			UE_LOG(LogTemp, Warning, TEXT("이미 습득한 스킬이 이 스킬을 막음: %s"), *AcquiredSkillID.ToString());
			return false;
		}
	}

	// 스킬 포인트 확인
	if (CachedASC.IsValid())
	{
		const UPlayerAttributeSet* PlayerAttrSet = CachedASC->GetSet<UPlayerAttributeSet>();
		if (PlayerAttrSet)
		{
			float CurrentSkillPoints = PlayerAttrSet->GetSkillPoint();
			if (CurrentSkillPoints < SkillData->RequiredSkillPoints)
			{
				return false;
			}
		}
	}

	return true;
}

UBaseSkillData* USkillComponent::FindSkillData(const FGameplayTag& SkillID) const
{
	// TMap에서 O(1) 검색
	if (const TObjectPtr<UBaseSkillData>* FoundSkill = SkillDataMap.Find(SkillID))
	{
		return FoundSkill->Get();
	}

	return nullptr;
}

void USkillComponent::ApplySkillEffects(UBaseSkillData* SkillData)
{
	if (!SkillData || !CachedASC.IsValid())
	{
		return;
	}

	// GameplayEffect 적용
	for (TSubclassOf<UGameplayEffect> EffectClass : SkillData->SkillEffects)
	{
		if (EffectClass)
		{
			FGameplayEffectContextHandle ContextHandle = CachedASC->MakeEffectContext();
			ContextHandle.AddSourceObject(SkillData);

			FGameplayEffectSpecHandle SpecHandle = CachedASC->MakeOutgoingSpec(EffectClass, 1.0f, ContextHandle);
			if (SpecHandle.IsValid())
			{
				CachedASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				UE_LOG(LogTemp, Log, TEXT("GameplayEffect 적용: %s"), *EffectClass->GetName());
			}
		}
	}

	// GameplayAbility 부여
	for (TSubclassOf<UGameplayAbility> AbilityClass : SkillData->SkillAbilities)
	{
		if (AbilityClass)
		{
			FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, GetOwner());
			CachedASC->GiveAbility(AbilitySpec);
			UE_LOG(LogTemp, Log, TEXT("GameplayAbility 부여: %s"), *AbilityClass->GetName());

			// 액티브 스킬인 경우 AbilityTag 저장 (Q키용)
			if (SkillData->SkillType == ESkillType::Active && AbilityClass.GetDefaultObject())
			{
				const UGameplayAbility* AbilityCDO = AbilityClass.GetDefaultObject();
				if (AbilityCDO->AbilityTags.Num() > 0)
				{
					// 첫 번째 태그를 액티브 스킬 태그로 저장
					ActiveSkillAbilityTag = AbilityCDO->AbilityTags.First();
					UE_LOG(LogTemp, Log, TEXT("액티브 스킬 태그 저장: %s"), *ActiveSkillAbilityTag.ToString());
				}
			}
		}
	}
}

bool USkillComponent::LearnSkill(const FGameplayTag& SkillID)
{
	ESkillAcquireResult Result = TryAcquireSkill(SkillID);
	return Result == ESkillAcquireResult::Success;
}
