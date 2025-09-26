// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSWeapon.h"
#include "FPSWeaponHolder.h"
#include "FPS/Items/WeaponItemData.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

AFPSWeapon::AFPSWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	// 루트 컴포넌트 생성
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// 1인칭 메시 생성
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMesh->SetupAttachment(RootComponent);
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	FirstPersonMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);
	FirstPersonMesh->bOnlyOwnerSee = true;

	// 3인칭 메시 생성
	ThirdPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThirdPersonMesh"));
	ThirdPersonMesh->SetupAttachment(RootComponent);
	ThirdPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	ThirdPersonMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::WorldSpaceRepresentation);
	ThirdPersonMesh->bOwnerNoSee = true;

	// 기본값 초기화
	MagazineSize = 10;
	CurrentBullets = 0;
	RefireRate = 0.5f;
	bFullAuto = false;
	AimVariance = 0.0f;
	FiringRecoil = 0.0f;
	MuzzleOffset = 10.0f;
	ShotLoudness = 1.0f;
	ShotNoiseRange = 3000.0f;
	ShotNoiseTag = FName("Shot");
	MuzzleSocketName = FName("Muzzle");
}

void AFPSWeapon::BeginPlay()
{
	Super::BeginPlay();

	// 소유자의 파괴 델리게이트에 구독
	if (GetOwner())
	{
		GetOwner()->OnDestroyed.AddDynamic(this, &AFPSWeapon::OnOwnerDestroyed);
	}

	// 무기 소유자 캐스트
	WeaponOwner = Cast<IFPSWeaponHolder>(GetOwner());
	PawnOwner = Cast<APawn>(GetOwner());

	// 첫 번째 탄창 채우기
	CurrentBullets = MagazineSize;

	// HUD 업데이트
	if (WeaponOwner)
	{
		WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);
	}
}

void AFPSWeapon::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	// 활성 타이머 모두 정리
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(RefireTimer);
	}

	Super::EndPlay(EndPlayReason);
}

void AFPSWeapon::OnOwnerDestroyed(AActor* DestroyedActor)
{
	// 소유자가 파괴됨, 정리 작업
	UE_LOG(LogTemp, Warning, TEXT("무기 소유자 파괴됨 - 무기도 파괴: %s"), *GetName());

	WeaponOwner = nullptr;
	PawnOwner = nullptr;

	// 발사 중지
	StopFiring();

	// 무기도 같이 파괴 (약간의 딜레이를 주어 안전하게 처리)
	if (GetWorld())
	{
		FTimerHandle DestroyTimer;
		GetWorld()->GetTimerManager().SetTimer(DestroyTimer, [this]()
		{
			if (IsValid(this))
			{
				Destroy();
			}
		}, 0.1f, false);
	}
}

void AFPSWeapon::ActivateWeapon()
{
	if (!WeaponOwner)
	{
		return;
	}

	// 무기 메시를 소유자에게 부착
	WeaponOwner->AttachWeaponMeshes(this);

	// 무기가 활성화되었음을 소유자에게 알림
	WeaponOwner->OnWeaponActivated(this);

	// HUD 업데이트
	WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);
}

void AFPSWeapon::DeactivateWeapon()
{
	if (!WeaponOwner)
	{
		return;
	}

	// 현재 발사 중이면 발사 중지
	StopFiring();

	// 무기가 비활성화되었음을 소유자에게 알림
	WeaponOwner->OnWeaponDeactivated(this);
}

void AFPSWeapon::StartFiring()
{
	if (!WeaponOwner || !PawnOwner)
	{
		return;
	}

	// 탄약이 있는지 확인
	if (CurrentBullets <= 0)
	{
		// TODO: 빈 무기 사운드/애니메이션 재생
		return;
	}

	// 발사 플래그 설정
	bIsFiring = true;

	// GAS를 통해 발사 어빌리티 활성화 시도
	if (FireAbility && PawnOwner->Implements<UAbilitySystemInterface>())
	{
		if (UAbilitySystemComponent* ASC = Cast<IAbilitySystemInterface>(PawnOwner)->GetAbilitySystemComponent())
		{
			// 발사 어빌리티 활성화 시도
			if (ASC->TryActivateAbilityByClass(FireAbility))
			{
				// 어빌리티 활성화 성공, 어빌리티가 Fire() 호출 처리
				return;
			}
		}
	}

	// 대체: GAS를 사용할 수 없으면 Fire() 직접 호출
	Fire();
}

void AFPSWeapon::StopFiring()
{
	// 발사 플래그 해제
	bIsFiring = false;

	// 연사 타이머 정리
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(RefireTimer);
	}
}

void AFPSWeapon::Fire()
{
	if (!WeaponOwner || !bIsFiring)
	{
		return;
	}

	// 탄약이 있는지 확인
	if (!ConsumeAmmo())
	{
		StopFiring();
		return;
	}

	// 마지막 발사 시간 업데이트
	TimeOfLastShot = GetWorld()->GetTimeSeconds();

	// 목표를 향해 발사체 발사
	FireProjectile(WeaponOwner->GetWeaponTargetLocation());

	// 발사 몽타주 재생
	if (FiringMontage)
	{
		WeaponOwner->PlayFiringMontage(FiringMontage);
	}

	// 반동 적용
	if (FiringRecoil > 0.0f)
	{
		WeaponOwner->AddWeaponRecoil(FiringRecoil);
	}

	// HUD 업데이트
	WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);

	// 자동 무기의 연사 처리
	if (bFullAuto && bIsFiring)
	{
		// 다음 발사 예약
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AFPSWeapon::Fire, RefireRate, false);
	}
	else
	{
		// 반자동 무기의 경우, 쿨다운 알림 예약
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AFPSWeapon::FireCooldownExpired, RefireRate, false);
	}
}

void AFPSWeapon::FireCooldownExpired()
{
	// 무기가 다시 발사할 준비가 되었음을 소유자에게 알림
	if (WeaponOwner)
	{
		WeaponOwner->OnSemiWeaponRefire();
	}
}

void AFPSWeapon::FireProjectile(const FVector& TargetLocation)
{
	if (!ProjectileClass || !GetWorld())
	{
		return;
	}

	// 발사체 트랜스폼 가져오기
	FTransform ProjectileTransform = CalculateProjectileSpawnTransform(TargetLocation);

	// 발사체 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::OverrideRootScale;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = PawnOwner;

	AActor* Projectile = GetWorld()->SpawnActor<AActor>(ProjectileClass, ProjectileTransform, SpawnParams);

	// TODO: AI 인식 시스템용 소음 생성
	// if (PawnOwner && ShotLoudness > 0.0f)
	// {
	//     PawnOwner->MakeNoise(ShotLoudness, PawnOwner, GetActorLocation(), ShotNoiseRange, ShotNoiseTag);
	// }
}

FTransform AFPSWeapon::CalculateProjectileSpawnTransform(const FVector& TargetLocation) const
{
	FVector SpawnLocation;
	FRotator SpawnRotation;

	// 머즐 소켓 위치 가져오기 시도
	if (FirstPersonMesh && FirstPersonMesh->DoesSocketExist(MuzzleSocketName))
	{
		SpawnLocation = FirstPersonMesh->GetSocketLocation(MuzzleSocketName);

		// 머즐에서 목표까지의 방향 계산
		FVector Direction = (TargetLocation - SpawnLocation).GetSafeNormal();
		SpawnRotation = Direction.Rotation();
	}
	else
	{
		// 액터 위치와 회전으로 대체
		SpawnLocation = GetActorLocation();
		SpawnRotation = GetActorRotation();

		// 머즐 오프셋 추가
		SpawnLocation += SpawnRotation.Vector() * MuzzleOffset;
	}

	// 지정된 경우 조준 분산 적용
	if (AimVariance > 0.0f)
	{
		FRotator VarianceRotation = FRotator(
			FMath::RandRange(-AimVariance, AimVariance),  // Pitch
			FMath::RandRange(-AimVariance, AimVariance),  // Yaw
			0.0f                                          // Roll
		);
		SpawnRotation += VarianceRotation;
	}

	return FTransform(SpawnRotation, SpawnLocation);
}

TSubclassOf<UAnimInstance> AFPSWeapon::GetFirstPersonAnimInstanceClass() const
{
	return FirstPersonAnimInstanceClass;
}

TSubclassOf<UAnimInstance> AFPSWeapon::GetThirdPersonAnimInstanceClass() const
{
	return ThirdPersonAnimInstanceClass;
}

void AFPSWeapon::SetCurrentAmmo(int32 NewAmmo)
{
	CurrentBullets = FMath::Clamp(NewAmmo, 0, MagazineSize);

	// HUD 업데이트
	if (WeaponOwner)
	{
		WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);
	}
}

bool AFPSWeapon::ConsumeAmmo(int32 AmmoToConsume)
{
	if (CurrentBullets >= AmmoToConsume)
	{
		CurrentBullets -= AmmoToConsume;
		return true;
	}

	return false;
}

void AFPSWeapon::InitializeFromItemData(UWeaponItemData* ItemData)
{
	if (!ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitializeFromItemData: ItemData가 null입니다!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("무기를 ItemData로 초기화: %s"), *ItemData->GetItemName());

	// WeaponItemData의 값들을 FPSWeapon에 적용
	MagazineSize = ItemData->MagazineSize;
	RefireRate = 1.0f / ItemData->FireRate; // FireRate(초당 발사횟수) -> RefireRate(발사간격)로 변환
	AimVariance = ItemData->AccuracySpread;
	FiringRecoil = ItemData->RecoilStrength;
	bFullAuto = ItemData->bIsAutomatic;

	// 현재 탄약도 탄창 크기에 맞춰 설정
	CurrentBullets = MagazineSize;

	// TODO: WeaponRange 등 추가 스탯들도 적용

	UE_LOG(LogTemp, Log, TEXT("무기 초기화 완료 - 탄창:%d, 연사속도:%.2f, 정확도:%.2f"),
		MagazineSize, RefireRate, AimVariance);
}