// SkillTreeWidget.cpp

#include "UI/SkillTreeWidget.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/SkillComponent.h"
#include "UI/SkillItemWidget.h"
#include "Skills/BaseSkillData.h"
#include "Blueprint/UserWidget.h"

void USkillTreeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Close 버튼 바인딩
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &USkillTreeWidget::OnCloseButtonClicked);
	}
}

void USkillTreeWidget::InitializeSkillTree(USkillComponent* InSkillComponent)
{
	SkillComponent = InSkillComponent;

	if (!SkillComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitializeSkillTree: SkillComponent가 nullptr입니다."));
		return;
	}

	// 스킬 목록 초기 표시
	RefreshSkillList();
}

void USkillTreeWidget::RefreshSkillList()
{
	if (!SkillComponent || !SkillListBox || !SkillItemWidgetClass)
	{
		return;
	}

	// 기존 스킬 목록 제거
	SkillListBox->ClearChildren();

	// 모든 스킬 데이터 가져오기
	const TArray<TObjectPtr<UBaseSkillData>>& AllSkills = SkillComponent->GetAllSkills();

	// 각 스킬에 대해 SkillItemWidget 생성
	for (UBaseSkillData* SkillData : AllSkills)
	{
		if (!SkillData)
		{
			continue;
		}

		// SkillItemWidget 생성
		USkillItemWidget* SkillItemWidget = CreateWidget<USkillItemWidget>(GetWorld(), SkillItemWidgetClass);
		if (SkillItemWidget)
		{
			// 스킬 데이터 설정
			SkillItemWidget->SetSkillData(SkillData, SkillComponent);

			// VerticalBox에 추가
			SkillListBox->AddChild(SkillItemWidget);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("RefreshSkillList: %d개의 스킬 표시"), AllSkills.Num());
}

void USkillTreeWidget::UpdateSkillPointDisplay(int32 CurrentPoints)
{
	if (SkillPointText)
	{
		SkillPointText->SetText(FText::Format(
			FText::FromString(TEXT("Skill Points: {0}")),
			FText::AsNumber(CurrentPoints)
		));
	}
}

void USkillTreeWidget::OnCloseButtonClicked()
{
	// UI 닫기
	RemoveFromParent();
}
