// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/AI/FPSEnemyCharacter.h"
#include "FPS/AI/FPSEnemyAIController.h"
#include "FPS/Weapons/FPSWeapon.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

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

	// AI 전용 기본 무기 지급
	GiveDefaultWeapon();
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

// ========================================
// IFPSWeaponHolder 인터페이스 구현
// ========================================

void AFPSEnemyCharacter::AttachWeaponMeshes(AFPSWeapon* Weapon)
{
	if (!Weapon)
	{
		return;
	}

	// AI는 3인칭 메시만 사용하므로 3인칭 무기 메시만 부착
	if (USkeletalMeshComponent* WeaponThirdPersonMesh = Weapon->GetThirdPersonMesh())
	{
		WeaponThirdPersonMesh->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::KeepRelativeTransform,
			ThirdPersonWeaponSocket
		);
	}

	// 1인칭 메시는 AI가 사용하지 않으므로 숨기기
	if (USkeletalMeshComponent* WeaponFirstPersonMesh = Weapon->GetFirstPersonMesh())
	{
		WeaponFirstPersonMesh->SetVisibility(false);
	}
}

void AFPSEnemyCharacter::PlayFiringMontage(UAnimMontage* Montage)
{
	if (!Montage)
	{
		return;
	}

	// 3인칭 메시에서 발사 몽타주 재생
	if (GetMesh() && GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(Montage);
	}
}

void AFPSEnemyCharacter::AddWeaponRecoil(float Recoil)
{
	// AI는 반동을 받지 않음 (또는 매우 작게)
	// 필요시 AI Controller의 시야 흔들림 등으로 구현 가능
}

void AFPSEnemyCharacter::UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize)
{
	// AI는 HUD가 없으므로 로그 처리 불필요
	// 필요시 AI 전용 UI나 디버그 표시로 대체 가능
}

FVector AFPSEnemyCharacter::GetWeaponTargetLocation()
{
	// AI는 현재 타겟을 향해 조준
	if (AFPSEnemyAIController* AIController = Cast<AFPSEnemyAIController>(GetController()))
	{
		if (APawn* TargetPawn = AIController->GetTargetPawn())
		{
			// 타겟의 중심부를 노림 (캐릭터 높이의 절반 정도)
			FVector TargetLocation = TargetPawn->GetActorLocation();
			if (UCapsuleComponent* TargetCapsule = TargetPawn->FindComponentByClass<UCapsuleComponent>())
			{
				TargetLocation.Z += TargetCapsule->GetScaledCapsuleHalfHeight() * 0.7f;
			}
			return TargetLocation;
		}
	}

	// 타겟이 없으면 부모 클래스의 기본 동작 사용
	return Super::GetWeaponTargetLocation();
}

void AFPSEnemyCharacter::OnWeaponActivated(AFPSWeapon* Weapon)
{
	if (!Weapon)
	{
		return;
	}

	// AI용 애니메이션 인스턴스 클래스 설정
	if (TSubclassOf<UAnimInstance> TPAnimClass = Weapon->GetThirdPersonAnimInstanceClass())
	{
		if (GetMesh())
		{
			GetMesh()->SetAnimInstanceClass(TPAnimClass);
		}
	}
}

void AFPSEnemyCharacter::OnWeaponDeactivated(AFPSWeapon* Weapon)
{
	// TODO: 필요시 기본 애니메이션 인스턴스로 복원
}

void AFPSEnemyCharacter::OnSemiWeaponRefire()
{
	// AI는 연사 속도 제한 없이 계속 발사 (AI Controller에서 제어)
}

void AFPSEnemyCharacter::GiveDefaultWeapon()
{
	if (DefaultWeaponClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("적에게 기본 무기 지급: %s"), *DefaultWeaponClass->GetName());
		// 부모 클래스의 AddWeaponClass 호출
		Super::AddWeaponClass(DefaultWeaponClass);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("적의 기본 무기 클래스가 설정되지 않음!"));
	}
}

