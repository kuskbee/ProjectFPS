// DamageNumberWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DamageNumberWidget.generated.h"

/**
 * 데미지 숫자 표시 위젯
 * - 일반 데미지: 흰색, 기본 크기
 * - 크리티컬: 주황색, 큰 크기, 느낌표(!)
 * - 위로 떠오르며 페이드 아웃
 */
UCLASS()
class PROJECTFPS_API UDamageNumberWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 데미지 숫자 설정 */
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void SetDamageNumber(float Damage, bool bIsCritical);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** 데미지 텍스트 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> DamageText;

	/** 애니메이션 타이머 */
	float ElapsedTime = 0.0f;

	/** 애니메이션 지속 시간 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float AnimationDuration = 1.5f;

	/** 위로 올라가는 속도 (픽셀/초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float RiseSpeed = 50.0f;

	/** 크리티컬 여부 */
	bool bCritical = false;

	/** 시작 위치 저장 */
	FVector2D InitialPosition;
};
