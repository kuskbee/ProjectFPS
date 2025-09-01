// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPS/FPSCharacter.h"
#include "FPSEnemyCharacter.generated.h"

/**
 * AI가 조종하는 적 캐릭터 - AFPSCharacter 상속
 */
UCLASS()
class PROJECTFPS_API AFPSEnemyCharacter : public AFPSCharacter
{
	GENERATED_BODY()

public:
	AFPSEnemyCharacter();

protected:
	virtual void BeginPlay() override;

	// 부모 클래스의 OnHealthChanged 오버라이드
	virtual void OnHealthChanged(const FOnAttributeChangeData& Data) override;

	// 부모 클래스의 OnPlayerDeath 오버라이드 (적 전용 사망 처리)
	virtual void OnPlayerDeath() override;

public:	
	virtual void Tick(float DeltaTime) override;

protected:
	// 적 상태
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsDead = false;

	// 사망 후 제거 시간
	UPROPERTY(EditAnywhere, Category = "Death")
	float DeathDestroyDelay = 5.0f;

	FTimerHandle DeathTimer;

public:
	// AI에서 사용할 함수들
	UFUNCTION(BlueprintPure, Category = "AI")
	bool IsDead() const { return bIsDead; }

protected:
	// 사망 처리 (AI 전용)
	void Die();

	UFUNCTION()
	void OnDeathDestroy();
};
