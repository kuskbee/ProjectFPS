// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ToastMessageWidget.generated.h"

class UTextBlock;
class UBorder;

/**
 * 단일 토스트 메시지 위젯
 * 페이드인 → 표시 → 페이드아웃 애니메이션
 */
UCLASS()
class PROJECTFPS_API UToastMessageWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 메시지 설정 및 애니메이션 시작
	UFUNCTION(BlueprintCallable, Category = "Toast")
	void ShowMessage(const FString& Message, float DisplayDuration = 2.0f);

	// 메시지 즉시 숨김 (페이드아웃 시작)
	UFUNCTION(BlueprintCallable, Category = "Toast")
	void HideToast();

	// 메시지 제거 완료 델리게이트 (자기 자신의 포인터 전달)
	DECLARE_DELEGATE_OneParam(FOnToastFinished, UToastMessageWidget*);
	FOnToastFinished OnToastFinished;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	// UI 컴포넌트 (Blueprint에서 바인딩)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> ToastBorder;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MessageText;

	// 애니메이션 상태
	enum class EToastState : uint8
	{
		FadeIn,
		Display,
		FadeOut,
		Finished
	};

	EToastState CurrentState = EToastState::Finished;
	float StateTimer = 0.0f;

	// 타이밍 설정
	float FadeInDuration = 0.3f;
	float DisplayDuration = 2.0f;
	float FadeOutDuration = 0.5f;
};
