// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/AI/FPSEnemyCharacter.h"
#include "FPS/AI/FPSEnemyAIController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"

AFPSEnemyCharacter::AFPSEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// AI 캐릭터 설정
	bUseControllerRotationYaw = false;
	
	// Character Movement 설정
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	CharMovement->bOrientRotationToMovement = true;
	CharMovement->bUseControllerDesiredRotation = false;
	CharMovement->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	CharMovement->MaxWalkSpeed = 400.0f;
	CharMovement->MaxAcceleration = 1000.0f;
	CharMovement->BrakingDecelerationWalking = 1000.0f;
	
	// AI 애니메이션을 위한 설정
	CharMovement->bRequestedMoveUseAcceleration = true;
	
	// AI Controller 클래스 설정
	AIControllerClass = AFPSEnemyAIController::StaticClass();
	
	// 1인칭 메시와 카메라는 AI에게 필요없으므로 숨기기
	if (FirstPersonMesh)
	{
		FirstPersonMesh->SetVisibility(false);
	}
}

void AFPSEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AFPSEnemyCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	// 디버그 메시지
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 
			3.f, 
			FColor::Orange, 
			FString::Printf(TEXT("적 체력: %f"), Data.NewValue)
		);
	}

	// 체력이 0 이하가 되면 사망 처리
	if (Data.NewValue <= 0.0f && !bIsDead)
	{
		OnPlayerDeath();
	}
}

void AFPSEnemyCharacter::OnPlayerDeath()
{
	if (bIsDead) return;
	
	bIsDead = true;
	
	UE_LOG(LogTemp, Warning, TEXT("적 캐릭터 사망!"));

	// 움직임 정지
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	// AI 정지
	if (GetController())
	{
		GetController()->UnPossess();
	}

	// 충돌 비활성화
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 메시 숨기기 (나중에 래그돌로 변경 가능)
	GetMesh()->SetVisibility(false);

	// 일정 시간 후 파괴
	GetWorld()->GetTimerManager().SetTimer(DeathTimer, this, &AFPSEnemyCharacter::OnDeathDestroy, DeathDestroyDelay, false);
}

void AFPSEnemyCharacter::Die()
{
	// 호환성을 위해 OnPlayerDeath 호출
	OnPlayerDeath();
}

void AFPSEnemyCharacter::OnDeathDestroy()
{
	Destroy();
}

void AFPSEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

