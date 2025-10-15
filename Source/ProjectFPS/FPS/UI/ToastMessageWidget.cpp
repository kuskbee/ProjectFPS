// Copyright Epic Games, Inc. All Rights Reserved.

#include "ToastMessageWidget.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

void UToastMessageWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 초기 상태: 투명
	if (ToastBorder)
	{
		ToastBorder->SetRenderOpacity(0.0f);
	}
}

void UToastMessageWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (CurrentState == EToastState::Finished)
	{
		return;
	}

	StateTimer += InDeltaTime;

	switch (CurrentState)
	{
		case EToastState::FadeIn:
		{
			// 페이드인 진행률 (0.0 → 1.0)
			float Alpha = FMath::Clamp(StateTimer / FadeInDuration, 0.0f, 1.0f);
			if (ToastBorder)
			{
				ToastBorder->SetRenderOpacity(Alpha);
			}

			// 페이드인 완료
			if (StateTimer >= FadeInDuration)
			{
				CurrentState = EToastState::Display;
				StateTimer = 0.0f;
			}
			break;
		}

		case EToastState::Display:
		{
			// DisplayDuration이 0이면 무한 표시 (수동으로 닫을 때까지)
			if (DisplayDuration > 0.0f && StateTimer >= DisplayDuration)
			{
				CurrentState = EToastState::FadeOut;
				StateTimer = 0.0f;
			}
			break;
		}

		case EToastState::FadeOut:
		{
			// 페이드아웃 진행률 (1.0 → 0.0)
			float Alpha = 1.0f - FMath::Clamp(StateTimer / FadeOutDuration, 0.0f, 1.0f);
			if (ToastBorder)
			{
				ToastBorder->SetRenderOpacity(Alpha);
			}

			// 페이드아웃 완료
			if (StateTimer >= FadeOutDuration)
			{
				CurrentState = EToastState::Finished;
				StateTimer = 0.0f;

				// 델리게이트 호출 (자기 자신의 포인터 전달)
				if (OnToastFinished.IsBound())
				{
					OnToastFinished.Execute(this);
				}
			}
			break;
		}

		default:
			break;
	}
}

void UToastMessageWidget::ShowMessage(const FString& Message, float InDisplayDuration)
{
	if (MessageText)
	{
		MessageText->SetText(FText::FromString(Message));
	}

	DisplayDuration = InDisplayDuration;
	CurrentState = EToastState::FadeIn;
	StateTimer = 0.0f;
}

void UToastMessageWidget::HideToast()
{
	// 현재 상태와 관계없이 즉시 페이드아웃 시작
	if (CurrentState != EToastState::Finished && CurrentState != EToastState::FadeOut)
	{
		CurrentState = EToastState::FadeOut;
		StateTimer = 0.0f;
	}
}
