// Copyright Epic Games, Inc. All Rights Reserved.

#include "ToastManagerWidget.h"
#include "ToastMessageWidget.h"
#include "Components/VerticalBox.h"

void UToastManagerWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UToastManagerWidget::ShowToast(const FString& Message, float DisplayDuration)
{
	// 이미 토스트가 표시 중이면 기존 위젯을 페이드아웃 시작
	if (CurrentToastWidget)
	{
		CurrentToastWidget->HideToast();
		// 제거는 OnToastFinished에서 자동 처리됨
	}

	// 새 토스트 위젯 생성
	if (ToastMessageWidgetClass && ToastContainer)
	{
		UToastMessageWidget* NewToast = CreateWidget<UToastMessageWidget>(this, ToastMessageWidgetClass);
		if (NewToast)
		{
			// 컨테이너에 추가 (기존 토스트 아래에 추가됨)
			ToastContainer->AddChild(NewToast);

			// 완료 델리게이트 바인딩
			NewToast->OnToastFinished.BindUObject(this, &UToastManagerWidget::OnToastFinished);

			// 메시지 표시
			NewToast->ShowMessage(Message, DisplayDuration);

			// 현재 토스트 업데이트
			CurrentToastWidget = NewToast;
		}
	}
}

void UToastManagerWidget::OnToastFinished(UToastMessageWidget* FinishedWidget)
{
	// 완료된 위젯만 제거 (CurrentToastWidget이 아닐 수도 있음!)
	if (FinishedWidget && ToastContainer)
	{
		ToastContainer->RemoveChild(FinishedWidget);

		// 완료된 위젯이 현재 위젯이라면 초기화
		if (CurrentToastWidget == FinishedWidget)
		{
			CurrentToastWidget = nullptr;
		}
	}
}

void UToastManagerWidget::HideToast()
{
	// 현재 토스트 위젯에 숨김 명령
	if (CurrentToastWidget)
	{
		CurrentToastWidget->HideToast();
	}
}
