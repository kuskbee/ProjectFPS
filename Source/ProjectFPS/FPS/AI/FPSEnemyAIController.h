// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "FPSEnemyAIController.generated.h"

// AI 행동 상태
UENUM(BlueprintType)
enum class EAIState : uint8
{
	Idle,      // 대기 상태
	Patrol,    // 순찰 (미구현)
	Chase,     // 추적
	Attack     // 공격
};

class AFPSEnemyCharacter;

/**
 * 적 AI Controller - 플레이어 탐지, 추적, 공격
 */
UCLASS()
class PROJECTFPS_API AFPSEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AFPSEnemyAIController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Pawn을 빙의했을 때 호출
	virtual void OnPossess(APawn* InPawn) override;

protected:
	// AI 설정값들
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Settings")
	float SightRange = 1500.0f; // 시야 범위

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Settings")
	float AttackRange = 600.0f; // 공격 범위

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Settings")
	float SightAngle = 60.0f; // 시야각 (도 단위)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Settings")
	float TickInterval = 0.2f; // AI 업데이트 간격

	// 무기 관련 설정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Settings")
	float FireRate = 1.0f; // 발사 간격 (초)

	// 현재 AI 상태
	UPROPERTY(BlueprintReadOnly, Category = "AI State")
	EAIState CurrentState = EAIState::Idle;

	// 타겟 (플레이어)
	UPROPERTY(BlueprintReadOnly, Category = "AI State")
	TObjectPtr<APawn> TargetPawn;

	// 내가 조종하는 적 캐릭터
	UPROPERTY(BlueprintReadOnly, Category = "AI State")
	TObjectPtr<AFPSEnemyCharacter> ControlledEnemy;

	// AI 업데이트 타이머
	FTimerHandle AIUpdateTimer;

	// 무기 발사 관련
	FTimerHandle FireTimer;
	float LastFireTime = 0.0f;

protected:
	// AI 핵심 함수들
	UFUNCTION()
	void UpdateAI();

	// 플레이어 탐지
	bool CanSeePlayer();
	APawn* FindPlayerPawn();

	// 거리 계산
	float GetDistanceToTarget() const;
	bool IsInAttackRange() const;
	bool IsInSightRange() const;

	// AI 상태별 행동
	void HandleIdleState();
	void HandleChaseState(); 
	void HandleAttackState();

	// 행동 실행
	void StartChasing();
	void StartAttacking();
	void StopAttacking();

	// 상태 변경
	void SetAIState(EAIState NewState);

	// 무기 발사 함수들
	void FireWeapon();
	bool CanFireWeapon() const;

public:
	// Blueprint에서 사용할 수 있는 함수들
	UFUNCTION(BlueprintPure, Category = "AI")
	EAIState GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "AI")
	APawn* GetTargetPawn() const { return TargetPawn; }
};
