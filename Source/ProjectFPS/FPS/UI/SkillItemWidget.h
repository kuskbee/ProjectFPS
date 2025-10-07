// SkillItemWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "SkillItemWidget.generated.h"

class UBaseSkillData;
class USkillComponent;
class UTextBlock;
class UButton;
class UImage;

/**
 * 개별 스킬 아이템 위젯
 * - 스킬 이름, 필요 포인트 표시
 * - 습득 가능/불가능 상태 표시
 * - 습득 버튼
 */
UCLASS()
class PROJECTFPS_API USkillItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// 스킬 데이터로 위젯 초기화
	UFUNCTION(BlueprintCallable, Category = "SkillTree")
	void SetSkillData(UBaseSkillData* InSkillData, USkillComponent* InSkillComponent);

	// 스킬 상태 업데이트 (습득 가능 여부)
	UFUNCTION(BlueprintCallable, Category = "SkillTree")
	void UpdateSkillState();

protected:
	// UI 컴포넌트들 (Blueprint에서 바인딩)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SkillNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SkillCostText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SkillStatusText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LearnButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> SkillIcon;

	// 스킬 데이터 참조
	UPROPERTY(BlueprintReadOnly, Category = "SkillTree")
	TObjectPtr<UBaseSkillData> SkillData;

	// SkillComponent 참조
	UPROPERTY(BlueprintReadOnly, Category = "SkillTree")
	TObjectPtr<USkillComponent> SkillComponent;

private:
	// Learn 버튼 클릭 핸들러
	UFUNCTION()
	void OnLearnButtonClicked();

	// 스킬 습득 가능 여부 체크
	bool CanLearnSkill() const;
};
