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
	// 이미 토스트가 표시 중이면 기존 위젯 제거
	if (CurrentToastWidget && ToastContainer)
	{
		ToastContainer->RemoveChild(CurrentToastWidget);
		CurrentToastWidget = nullptr;
	}

	// 새 토스트 위젯 생성
	if (ToastMessageWidgetClass && ToastContainer)
	{
		CurrentToastWidget = CreateWidget<UToastMessageWidget>(this, ToastMessageWidgetClass);
		if (CurrentToastWidget)
		{
			// 컨테이너에 추가
			ToastContainer->AddChild(CurrentToastWidget);

			// 완료 델리게이트 바인딩
			CurrentToastWidget->OnToastFinished.BindUObject(this, &UToastManagerWidget::OnToastFinished);

			// 메시지 표시
			CurrentToastWidget->ShowMessage(Message, DisplayDuration);
		}
	}
}

void UToastManagerWidget::OnToastFinished()
{
	// 토스트 위젯 제거
	if (CurrentToastWidget && ToastContainer)
	{
		ToastContainer->RemoveChild(CurrentToastWidget);
		CurrentToastWidget = nullptr;
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
