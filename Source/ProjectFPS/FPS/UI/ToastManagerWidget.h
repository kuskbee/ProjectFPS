// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ToastManagerWidget.generated.h"

class UToastMessageWidget;
class UVerticalBox;

/**
 * 토스트 메시지 관리 위젯
 * 여러 메시지를 큐로 관리하고 순차적으로 표시
 */
UCLASS()
class PROJECTFPS_API UToastManagerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 토스트 메시지 표시
	UFUNCTION(BlueprintCallable, Category = "Toast")
	void ShowToast(const FString& Message, float DisplayDuration = 2.0f);

	// 현재 토스트 메시지 숨김
	UFUNCTION(BlueprintCallable, Category = "Toast")
	void HideToast();

protected:
	virtual void NativeConstruct() override;

private:
	// Toast 메시지 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Toast")
	TSubclassOf<UToastMessageWidget> ToastMessageWidgetClass;

	// 토스트 메시지 컨테이너 (Blueprint에서 바인딩)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> ToastContainer;

	// 현재 표시 중인 토스트 위젯
	UPROPERTY()
	TObjectPtr<UToastMessageWidget> CurrentToastWidget;

	// 토스트 완료 시 호출
	void OnToastFinished();
};
