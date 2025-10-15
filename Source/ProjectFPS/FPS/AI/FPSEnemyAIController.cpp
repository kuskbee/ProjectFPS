// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/AI/FPSEnemyAIController.h"
#include "FPS/AI/FPSEnemyCharacter.h"
#include "FPS/Weapons/FPSWeapon.h"
#include "FPS/Components/WeaponSlotComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "NavigationSystem.h"
#include "AITypes.h"
#include "Navigation/PathFollowingComponent.h"

AFPSEnemyAIController::AFPSEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// AI 기본 설정
	bWantsPlayerState = false;
	bSkipExtraLOSChecks = false;
	
	// AI 이동을 위한 설정
	bAllowStrafe = false;
	bSetControlRotationFromPawnOrientation = false;
	
	// AI 애니메이션을 위한 설정
	bWantsPlayerState = false;
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
		UE_LOG(LogTemp, Log, TEXT("AI Controller 빙의 완료"));
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

	// 라인 트레이스로 장애물 체크 (벽, 바닥 등 모든 Static 물체 감지)
	FHitResult HitResult;
	FVector Start = ControlledEnemy->GetActorLocation();
	FVector End = TargetPawn->GetActorLocation();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(ControlledEnemy);

	// ECC_Camera: WorldStatic, WorldDynamic 모두 차단
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Camera,  // WorldStatic(벽) + WorldDynamic 모두 감지
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

	// ⭐ 시야에 있는지 확인 (벽 뚫고 공격 방지!)
	if (!CanSeePlayer())
	{
		SetAIState(EAIState::Chase);
		return;
	}

	// 공격 실행
	StartAttacking();
}

// 행동 실행 함수들
void AFPSEnemyAIController::StartChasing()
{
	if (!TargetPawn || !ControlledEnemy)
		return;

	FVector TargetLocation = TargetPawn->GetActorLocation();
	FVector MyLocation = ControlledEnemy->GetActorLocation();
	float Distance = FVector::Dist(MyLocation, TargetLocation);


	// FAIMoveRequest 방식으로 이동
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalLocation(TargetLocation);
	MoveRequest.SetAcceptanceRadius(100.0f);
	MoveRequest.SetCanStrafe(false);
	MoveRequest.SetAllowPartialPath(true);
	
	MoveTo(MoveRequest);
}

void AFPSEnemyAIController::StartAttacking()
{
	if (!TargetPawn || !ControlledEnemy)
	{
		return;
	}

	// 플레이어 방향으로 회전
	FVector LookDirection = (TargetPawn->GetActorLocation() - ControlledEnemy->GetActorLocation()).GetSafeNormal();
	FRotator TargetRotation = LookDirection.Rotation();
	ControlledEnemy->SetActorRotation(TargetRotation);

	// 무기 발사 시도
	if (CanFireWeapon())
	{
		FireWeapon();
	}
}

void AFPSEnemyAIController::StopAttacking()
{
	// 발사 타이머 정리
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimer);
	}
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

// ========================================
// 무기 발사 함수들
// ========================================

void AFPSEnemyAIController::FireWeapon()
{
	if (!ControlledEnemy)
	{
		return;
	}

	// 현재 무기가 있는지 확인
	UWeaponSlotComponent* WSC = ControlledEnemy->GetWeaponSlotComponent();
	if (!WSC)
	{
		UE_LOG(LogTemp, Warning, TEXT("적이 무기 슬롯 컴포넌트를 가지고 있지 않음!"));
		return;
	}
	AFPSWeapon* CurrentWeapon = WSC->GetCurrentWeaponActor();
	if (!CurrentWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("적이 무기를 가지고 있지 않음!"));
		return;
	}

	// 무기 발사
	CurrentWeapon->StartFiring();

	// 마지막 발사 시간 업데이트
	LastFireTime = GetWorld()->GetTimeSeconds();

	// 디버그 메시지
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("적이 발사!"));
	}

	// 연사를 위해 다음 발사 예약 (단발 무기의 경우)
	// 자동 무기는 FPSWeapon에서 자체적으로 연사 처리
	if (GetWorld() && FireRate > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			FireTimer,
			[this]()
			{
				if (ControlledEnemy)
				{
					UWeaponSlotComponent* _WSC = ControlledEnemy->GetWeaponSlotComponent();
					if (!_WSC)
					{
						UE_LOG(LogTemp, Warning, TEXT("적이 무기 슬롯 컴포넌트를 가지고 있지 않음!"));
						return;
					}
					AFPSWeapon* _CurrentWeapon = _WSC->GetCurrentWeaponActor();
					if (!_CurrentWeapon)
					{
						UE_LOG(LogTemp, Warning, TEXT("적이 무기를 가지고 있지 않음!"));
						return;
					}

					_CurrentWeapon->StopFiring();
				}
			},
			0.1f,  // 짧게 발사하고 중단
			false
		);
	}
}

bool AFPSEnemyAIController::CanFireWeapon() const
{
	if (!ControlledEnemy)
	{
		return false;
	}

	// 무기가 있는지 확인
	UWeaponSlotComponent* WSC = ControlledEnemy->GetWeaponSlotComponent();
	if (!WSC)
	{
		return false;
	}
	AFPSWeapon* CurrentWeapon = WSC->GetCurrentWeaponActor();
	if (!CurrentWeapon)
	{
		return false;
	}

	// 발사 간격 체크
	if (GetWorld())
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime - LastFireTime < FireRate)
		{
			return false;
		}
	}

	return true;
}

