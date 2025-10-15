// DamageNumberWidget.cpp

#include "UI/DamageNumberWidget.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"

void UDamageNumberWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 시작 위치 저장
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
	{
		InitialPosition = CanvasSlot->GetPosition();
	}
}

void UDamageNumberWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	ElapsedTime += InDeltaTime;

	// 페이드 아웃 (0 → 1 → 0)
	float Alpha = 1.0f - (ElapsedTime / AnimationDuration);
	if (Alpha < 0.0f)
	{
		// 애니메이션 종료 → 위젯 제거
		RemoveFromParent();
		return;
	}

	// 위로 떠오르는 효과
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
	{
		FVector2D NewPosition = InitialPosition + FVector2D(0.0f, -RiseSpeed * ElapsedTime);
		CanvasSlot->SetPosition(NewPosition);
	}

	// 텍스트 투명도 적용
	if (DamageText)
	{
		FLinearColor CurrentColor = DamageText->GetColorAndOpacity().GetSpecifiedColor();
		CurrentColor.A = Alpha;
		DamageText->SetColorAndOpacity(FSlateColor(CurrentColor));
	}
}

void UDamageNumberWidget::SetDamageNumber(float Damage, bool bIsCritical)
{
	bCritical = bIsCritical;

	if (!DamageText)
	{
		return;
	}

	// 데미지 텍스트 설정
	FString DamageString = FString::Printf(TEXT("%.0f"), FMath::Abs(Damage));
	if (bIsCritical)
	{
		DamageString += TEXT("!");  // 크리티컬은 느낌표 추가
	}
	DamageText->SetText(FText::FromString(DamageString));

	// 크리티컬 스타일
	if (bIsCritical)
	{
		// 주황색
		DamageText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.5f, 0.0f, 1.0f)));

		// 폰트 크기 1.5배
		FSlateFontInfo FontInfo = DamageText->GetFont();
		FontInfo.Size = 36;  // 크리티컬: 36
		DamageText->SetFont(FontInfo);
	}
	else
	{
		// 일반: 흰색
		DamageText->SetColorAndOpacity(FSlateColor(FLinearColor::White));

		// 기본 크기
		FSlateFontInfo FontInfo = DamageText->GetFont();
		FontInfo.Size = 24;  // 일반: 24
		DamageText->SetFont(FontInfo);
	}
}
