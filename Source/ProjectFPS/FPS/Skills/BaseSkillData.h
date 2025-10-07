// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "BaseSkillData.generated.h"

class UGameplayEffect;
class UGameplayAbility;
class UTexture2D;

/**
 * 스킬 타입 (패시브 vs 액티브)
 */
UENUM(BlueprintType)
enum class ESkillType : uint8
{
	Passive UMETA(DisplayName = "Passive"),   // 습득 시 자동 적용
	Active  UMETA(DisplayName = "Active")     // 사용자가 활성화 필요
};

/**
 * 스킬 기본 데이터 클래스
 * DataAsset 방식으로 스킬 정보 저장
 */
UCLASS(BlueprintType)
class PROJECTFPS_API UBaseSkillData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 스킬 고유 ID (GameplayTag 방식) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Info")
	FGameplayTag SkillID;

	/** 스킬 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Info")
	FText SkillName;

	/** 스킬 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Info", meta = (MultiLine = true))
	FText SkillDescription;

	/** 스킬 아이콘 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Info")
	TObjectPtr<UTexture2D> SkillIcon;

	/** 스킬 타입 (패시브/액티브) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Info")
	ESkillType SkillType = ESkillType::Passive;

	/** 스킬 티어 (1~5) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Info", meta = (ClampMin = 1, ClampMax = 5))
	int32 SkillTier = 1;

	/** 트리 UI 위치 - X 좌표 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Tree UI")
	float TreePositionX = 0.0f;

	/** 트리 UI 위치 - Y 좌표 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Tree UI")
	float TreePositionY = 0.0f;


	/** 필요 스킬 포인트 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Requirements")
	int32 RequiredSkillPoints = 1;

	/** 선행 스킬 (이 스킬을 배우려면 필요한 스킬들) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Requirements")
	TArray<FGameplayTag> PrerequisiteSkills;

	/** 스킬 습득 시 적용할 GameplayEffect */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Effects")
	TArray<TSubclassOf<UGameplayEffect>> SkillEffects;

	/** 스킬 습득 시 부여할 GameplayAbility (액티브 스킬용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Effects")
	TArray<TSubclassOf<UGameplayAbility>> SkillAbilities;
};
