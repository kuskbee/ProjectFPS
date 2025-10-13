// SkillItemWidget.cpp

#include "UI/SkillItemWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/SkillComponent.h"
#include "Skills/BaseSkillData.h"
#include "UI/SkillTreeWidget.h"
#include "FPSPlayerCharacter.h"
#include "PlayerAttributeSet.h"
#include "AbilitySystemComponent.h"

void USkillItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Learn 버튼 바인딩
	if (LearnButton)
	{
		LearnButton->OnClicked.AddDynamic(this, &USkillItemWidget::OnLearnButtonClicked);
	}
}

void USkillItemWidget::SetSkillData(UBaseSkillData* InSkillData, USkillComponent* InSkillComponent)
{
	SkillData = InSkillData;
	SkillComponent = InSkillComponent;

	if (!SkillData)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetSkillData: SkillData가 nullptr입니다."));
		return;
	}

	// 스킬 이름 표시
	if (SkillNameText)
	{
		SkillNameText->SetText(SkillData->SkillName);
	}

	// 필요 스킬 포인트 표시
	if (SkillCostText)
	{
		SkillCostText->SetText(FText::Format(
			FText::FromString(TEXT("Cost: {0}")),
			FText::AsNumber(SkillData->RequiredSkillPoints)
		));
	}

	if (SkillIcon && SkillData->SkillIcon)
	{
		SkillIcon->SetBrushFromTexture(SkillData->SkillIcon);
	}


	// 스킬 상태 업데이트
	UpdateSkillState();
}

void USkillItemWidget::UpdateSkillState()
{
	if (!SkillData || !SkillComponent || !SkillStatusText || !LearnButton || !OutlineBorder)
	{
		return;
	}

	// 이미 습득한 스킬인지 체크
	bool bIsLearned = SkillComponent->IsSkillLearned(SkillData->SkillID);

	if (bIsLearned)
	{
		SkillStatusText->SetText(FText::FromString(TEXT("[Learned]")));
		LearnButton->SetIsEnabled(false);

		OutlineBorder->SetBrushColor(LearnedColor);
		return;
	}

	// 습득 가능 여부 체크
	bool bCanLearn = CanLearnSkill();

	if (bCanLearn)
	{
		SkillStatusText->SetText(FText::FromString(TEXT("[Available]")));
		LearnButton->SetIsEnabled(true);
		OutlineBorder->SetBrushColor(AvailableColor);
	}
	else
	{
		SkillStatusText->SetText(FText::FromString(TEXT("[Locked]")));
		LearnButton->SetIsEnabled(false);
		OutlineBorder->SetBrushColor(LockedColor);
	}
}

void USkillItemWidget::OnLearnButtonClicked()
{
	if (!SkillComponent || !SkillData)
	{
		return;
	}

	// 스킬 습득 시도
	if (SkillComponent->LearnSkill(SkillData->SkillID))
	{
		UE_LOG(LogTemp, Log, TEXT("스킬 습득 성공: %s"), *SkillData->SkillName.ToString());

		// 부모 SkillTreeWidget의 RefreshSkillList 호출하여 전체 UI 갱신
		if (USkillTreeWidget* SkillTreeWidget = Cast<USkillTreeWidget>(ParentSkillTreeWidget))
		{
			SkillTreeWidget->RefreshSkillList();
			UE_LOG(LogTemp, Log, TEXT("스킬트리 UI 갱신 완료"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("스킬 습득 실패: %s"), *SkillData->SkillName.ToString());
	}
}

bool USkillItemWidget::CanLearnSkill() const
{
	if (!SkillComponent || !SkillData)
	{
		return false;
	}

	// SkillComponent의 CanLearnSkill 함수 사용
	return SkillComponent->CanLearnSkill(SkillData->SkillID);
}
