// SkillTreeWidget.cpp

#include "UI/SkillTreeWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/SkillComponent.h"
#include "UI/SkillItemWidget.h"
#include "Skills/BaseSkillData.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "PlayerAttributeSet.h"

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
	if (!SkillComponent || !SkillTreeCanvas || !SkillItemWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("RefreshSkillList: SkillTreeCanvas 또는 SkillItemWidgetClass가 nullptr입니다."));
		return;
	}

	// 기존 스킬 목록 제거
	SkillTreeCanvas->ClearChildren();

	// 모든 스킬 데이터 가져오기
	const TArray<TObjectPtr<UBaseSkillData>>& AllSkills = SkillComponent->GetAllSkills();

	// 각 스킬에 대해 SkillItemWidget 생성 및 위치 지정
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

			// 부모 위젯 참조 설정 (UI 갱신용)
			SkillItemWidget->SetParentSkillTreeWidget(this);

			// Canvas에 추가
			UCanvasPanelSlot* CanvasSlot = SkillTreeCanvas->AddChildToCanvas(SkillItemWidget);
			if (CanvasSlot)
			{
				// BaseSkillData에 저장된 위치로 설정
				CanvasSlot->SetPosition(FVector2D(SkillData->TreePositionX, SkillData->TreePositionY));

				// SetAutoSize(true)로 설정하여 Widget의 DesiredSize 사용
				CanvasSlot->SetAutoSize(true);

				// Anchor는 좌상단 기준
				CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
			}
		}
	}

	// 스킬 포인트 표시 업데이트
	if (AActor* Owner = SkillComponent->GetOwner())
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner))
		{
			if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
			{
				// PlayerAttributeSet에서 현재 스킬 포인트 가져오기
				const UAttributeSet* AttributeSet = ASC->GetAttributeSet(UPlayerAttributeSet::StaticClass());
				if (const UPlayerAttributeSet* PlayerAttrSet = Cast<UPlayerAttributeSet>(AttributeSet))
				{
					int32 CurrentSkillPoints = static_cast<int32>(PlayerAttrSet->GetSkillPoint());
					UpdateSkillPointDisplay(CurrentSkillPoints);
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("RefreshSkillList: %d개의 스킬을 트리 구조로 표시"), AllSkills.Num());
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
