// SkillTreeWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillTreeWidget.generated.h"

class USkillComponent;
class UCanvasPanel;
class UTextBlock;
class UButton;

/**
 * 스킬트리 UI 메인 위젯
 * - 스킬 목록 표시 (리스트 방식)
 * - 스킬 포인트 표시
 * - Tab키로 토글
 */
UCLASS()
class PROJECTFPS_API USkillTreeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// SkillComponent 설정 및 UI 초기화
	UFUNCTION(BlueprintCallable, Category = "SkillTree")
	void InitializeSkillTree(USkillComponent* InSkillComponent);

	// 스킬 목록 새로고침
	UFUNCTION(BlueprintCallable, Category = "SkillTree")
	void RefreshSkillList();

	// 스킬 포인트 표시 업데이트
	UFUNCTION(BlueprintCallable, Category = "SkillTree")
	void UpdateSkillPointDisplay(int32 CurrentPoints);

protected:
	// SkillComponent 참조
	UPROPERTY(BlueprintReadOnly, Category = "SkillTree")
	TObjectPtr<USkillComponent> SkillComponent;

	// UI 컴포넌트들 (Blueprint에서 바인딩)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> SkillTreeCanvas;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SkillPointText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

	// 스킬 아이템 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SkillTree")
	TSubclassOf<UUserWidget> SkillItemWidgetClass;

private:
	// Close 버튼 클릭 핸들러
	UFUNCTION()
	void OnCloseButtonClicked();
};
