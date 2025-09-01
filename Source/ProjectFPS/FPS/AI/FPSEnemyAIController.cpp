// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/AI/FPSEnemyAIController.h"
#include "FPS/AI/FPSEnemyCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

AFPSEnemyAIController::AFPSEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// AI 기본 설정
	bWantsPlayerState = false;
	bSkipExtraLOSChecks = false;
}

void AFPSEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
	
	// AI 업데이트 타이머 시작
	GetWorld()->GetTimerManager().SetTimer(
		AIUpdateTimer, 
		this, 
		&AFPSEnemyAIController::UpdateAI, 
		TickInterval, 
		true  // 반복
	);
}

void AFPSEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFPSEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	// 내가 조종할 적 캐릭터 캐스팅
	ControlledEnemy = Cast<AFPSEnemyCharacter>(InPawn);
	
	if (ControlledEnemy)
	{
		UE_LOG(LogTemp, Warning, TEXT("AI Controller가 적 캐릭터를 빙의했습니다!"));
	}
}

// AI 메인 업데이트 함수
void AFPSEnemyAIController::UpdateAI()
{
	// 죽었으면 AI 중단
	if (!ControlledEnemy || ControlledEnemy->IsDead())
	{
		return;
	}

	// 플레이어 찾기
	if (!TargetPawn)
	{
		TargetPawn = FindPlayerPawn();
	}

	// 타겟이 없으면 대기 상태
	if (!TargetPawn)
	{
		SetAIState(EAIState::Idle);
		return;
	}

	// 현재 상태에 따라 행동
	switch (CurrentState)
	{
	case EAIState::Idle:
		HandleIdleState();
		break;
	case EAIState::Chase:
		HandleChaseState();
		break;
	case EAIState::Attack:
		HandleAttackState();
		break;
	}
}

// 플레이어 탐지 함수
bool AFPSEnemyAIController::CanSeePlayer()
{
	if (!TargetPawn || !ControlledEnemy)
		return false;

	// 거리 체크
	if (!IsInSightRange())
		return false;

	// 시야각 체크
	FVector ToTarget = TargetPawn->GetActorLocation() - ControlledEnemy->GetActorLocation();
	ToTarget.Normalize();
	
	FVector Forward = ControlledEnemy->GetActorForwardVector();
	float DotProduct = FVector::DotProduct(Forward, ToTarget);
	float AngleRadians = FMath::Acos(DotProduct);
	float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

	// 시야각 안에 있는지 체크
	if (AngleDegrees > SightAngle)
		return false;

	// 라인 트레이스로 장애물 체크
	FHitResult HitResult;
	FVector Start = ControlledEnemy->GetActorLocation();
	FVector End = TargetPawn->GetActorLocation();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(ControlledEnemy);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult, 
		Start, 
		End, 
		ECC_Visibility, 
		QueryParams
	);

	// 플레이어에게 직접 닿으면 보임
	return !bHit || HitResult.GetActor() == TargetPawn;
}

APawn* AFPSEnemyAIController::FindPlayerPawn()
{
	// 첫 번째 플레이어 컨트롤러 찾기
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC && PC->GetPawn())
	{
		return PC->GetPawn();
	}
	return nullptr;
}

// 거리 계산 함수들
float AFPSEnemyAIController::GetDistanceToTarget() const
{
	if (!TargetPawn || !ControlledEnemy)
		return 0.0f;
		
	return FVector::Dist(ControlledEnemy->GetActorLocation(), TargetPawn->GetActorLocation());
}

bool AFPSEnemyAIController::IsInAttackRange() const
{
	return GetDistanceToTarget() <= AttackRange;
}

bool AFPSEnemyAIController::IsInSightRange() const
{
	return GetDistanceToTarget() <= SightRange;
}

// AI 상태별 행동 함수들
void AFPSEnemyAIController::HandleIdleState()
{
	// 플레이어를 볼 수 있으면 추적 시작
	if (CanSeePlayer())
	{
		SetAIState(EAIState::Chase);
	}
}

void AFPSEnemyAIController::HandleChaseState()
{
	if (!TargetPawn)
	{
		SetAIState(EAIState::Idle);
		return;
	}

	// 공격 범위에 도달하면 공격 상태로
	if (IsInAttackRange())
	{
		SetAIState(EAIState::Attack);
		return;
	}

	// 시야에서 벗어났고 너무 멀면 추적 중단
	if (!CanSeePlayer() && GetDistanceToTarget() > SightRange * 1.5f)
	{
		SetAIState(EAIState::Idle);
		return;
	}

	// 플레이어 쪽으로 이동
	StartChasing();
}

void AFPSEnemyAIController::HandleAttackState()
{
	if (!TargetPawn)
	{
		SetAIState(EAIState::Idle);
		return;
	}

	// 공격 범위를 벗어나면 다시 추적
	if (!IsInAttackRange())
	{
		SetAIState(EAIState::Chase);
		return;
	}

	// 공격 실행 (현재는 로그만)
	StartAttacking();
}

// 행동 실행 함수들
void AFPSEnemyAIController::StartChasing()
{
	if (!TargetPawn || !ControlledEnemy)
		return;

	// AI 이동 명령
	MoveToLocation(TargetPawn->GetActorLocation(), 50.0f);
}

void AFPSEnemyAIController::StartAttacking()
{
	// 현재는 로그만 출력 (나중에 무기 시스템 연동)
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 
			0.5f, 
			FColor::Red, 
			TEXT("적이 공격 중!")
		);
	}
	
	// 플레이어 쪽으로 회전
	if (TargetPawn && ControlledEnemy)
	{
		FVector LookDirection = (TargetPawn->GetActorLocation() - ControlledEnemy->GetActorLocation()).GetSafeNormal();
		FRotator TargetRotation = LookDirection.Rotation();
		ControlledEnemy->SetActorRotation(TargetRotation);
	}
}

void AFPSEnemyAIController::StopAttacking()
{
	// 공격 중단 로직 (나중에 무기 시스템 연동)
}

// 상태 변경
void AFPSEnemyAIController::SetAIState(EAIState NewState)
{
	if (CurrentState == NewState)
		return;

	// 상태 변경 로그
	FString StateNames[] = { TEXT("Idle"), TEXT("Patrol"), TEXT("Chase"), TEXT("Attack") };
	UE_LOG(LogTemp, Log, TEXT("AI 상태 변경: %s -> %s"), 
		*StateNames[(int32)CurrentState], 
		*StateNames[(int32)NewState]
	);

	CurrentState = NewState;

	// 상태별 초기화
	switch (NewState)
	{
	case EAIState::Idle:
		StopMovement();
		StopAttacking();
		break;
	case EAIState::Chase:
		StopAttacking();
		break;
	case EAIState::Attack:
		StopMovement();
		break;
	}
}

