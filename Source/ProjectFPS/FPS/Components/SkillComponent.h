// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "SkillComponent.generated.h"

class UBaseSkillData;
class UAbilitySystemComponent;

/**
 * 스킬 습득 결과
 */
UENUM(BlueprintType)
enum class ESkillAcquireResult : uint8
{
	Success              UMETA(DisplayName = "Success"),
	AlreadyAcquired      UMETA(DisplayName = "Already Acquired"),
	InsufficientPoints   UMETA(DisplayName = "Insufficient Skill Points"),
	PrerequisiteNotMet   UMETA(DisplayName = "Prerequisite Not Met"),
	InvalidSkill         UMETA(DisplayName = "Invalid Skill")
};

/**
 * 스킬 습득/관리 컴포넌트
 * AFPSPlayerCharacter에 부착되어 스킬트리 시스템 관리
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTFPS_API USkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USkillComponent();

protected:
	virtual void BeginPlay() override;

public:
	/** 습득한 스킬 목록 (SkillID 저장) */
	UPROPERTY(BlueprintReadOnly, Category = "Skills")
	TSet<FGameplayTag> AcquiredSkills;

	/** 전체 스킬 데이터 (에디터에서 설정 - Array 방식) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Tree")
	TArray<TObjectPtr<UBaseSkillData>> SkillDataArray;

	/** 스킬 습득 시도 */
	UFUNCTION(BlueprintCallable, Category = "Skills")
	ESkillAcquireResult TryAcquireSkill(const FGameplayTag& SkillID);

	/** 스킬 습득 여부 확인 */
	UFUNCTION(BlueprintPure, Category = "Skills")
	bool HasSkill(const FGameplayTag& SkillID) const;

	/** 스킬 습득 가능 여부 확인 (선행 스킬, 포인트 등) */
	UFUNCTION(BlueprintPure, Category = "Skills")
	bool CanAcquireSkill(const FGameplayTag& SkillID) const;

	/** SkillID로 스킬 데이터 찾기 */
	UFUNCTION(BlueprintPure, Category = "Skills")
	UBaseSkillData* FindSkillData(const FGameplayTag& SkillID) const;

	/** 전체 스킬 목록 가져오기 (UI용) */
	// C++ 전용 함수 (UFUNCTION 제거 - TObjectPtr 사용으로 인해)
	const TArray<TObjectPtr<UBaseSkillData>>& GetAllSkills() const { return SkillDataArray; }

	/** LearnSkill - K키 테스트용 래퍼 함수 */
	UFUNCTION(BlueprintCallable, Category = "Skills")
	bool LearnSkill(const FGameplayTag& SkillID);

	/** 스킬 습득 가능 여부 (별칭) */
	UFUNCTION(BlueprintPure, Category = "Skills")
	bool CanLearnSkill(const FGameplayTag& SkillID) const { return CanAcquireSkill(SkillID); }

	/** 스킬 습득 여부 (별칭) */
	UFUNCTION(BlueprintPure, Category = "Skills")
	bool IsSkillLearned(const FGameplayTag& SkillID) const { return HasSkill(SkillID); }

	/** 습득한 액티브 스킬의 AbilityTag 반환 (Q키용) */
	UFUNCTION(BlueprintPure, Category = "Skills")
	FGameplayTag GetActiveSkillAbilityTag() const { return ActiveSkillAbilityTag; }

private:
	/** 스킬 효과 적용 (GameplayEffect + Ability) */
	void ApplySkillEffects(UBaseSkillData* SkillData);

	/** 런타임 캐시 (빠른 검색용 - Map 방식) */
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UBaseSkillData>> SkillDataMap;

	/** Owner의 AbilitySystemComponent 캐싱 */
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;

	/** 습득한 액티브 스킬의 AbilityTag (Q키로 활성화) */
	FGameplayTag ActiveSkillAbilityTag;
};
