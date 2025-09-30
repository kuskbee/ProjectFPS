// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSWeapon.h"
#include "FPSWeaponHolder.h"
#include "FPS/Items/WeaponItemData.h"
#include "FPS/Components/PickupTriggerComponent.h"
#include "FPS/FPSCharacter.h"
#include "FPS/Components/WeaponSlotComponent.h"
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

	// 픽업 트리거 컴포넌트 생성
	PickupTrigger = CreateDefaultSubobject<UPickupTriggerComponent>(TEXT("PickupTrigger"));
	PickupTrigger->SetupAttachment(RootComponent);
	PickupTrigger->SetPickupRange(150.0f);
	// 기본적으로 픽업 트리거 비활성화 (무기가 스폰될 때는 보통 장착 상태)
	PickupTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 기본값 초기화 (WeaponItemData에 없는 것들만)
	MuzzleOffset = 10.0f;
	ShotLoudness = 1.0f;
	ShotNoiseRange = 3000.0f;
	ShotNoiseTag = FName("Shot");
	MuzzleSocketName = FName("Muzzle");
	TimeOfLastShot = 0.0f;
	bIsFiring = false;
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

	// WeaponItemData 유효성 검사
	if (!WeaponItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("AFPSWeapon::BeginPlay: WeaponItemData가 설정되지 않았습니다! %s"), *GetName());
	}

	// HUD 업데이트
	if (WeaponOwner && WeaponItemData)
	{
		WeaponOwner->UpdateWeaponHUD(WeaponItemData->CurrentAmmo, WeaponItemData->MagazineSize);
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
	if (WeaponItemData)
	{
		WeaponOwner->UpdateWeaponHUD(WeaponItemData->CurrentAmmo, WeaponItemData->MagazineSize);
	}
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

	// WeaponItemData 확인
	if (!WeaponItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartFiring: WeaponItemData가 null입니다"));
		return;
	}

	// 리로드 중인지 확인 (AbilitySystemComponent를 통해)
	if (PawnOwner->Implements<UAbilitySystemInterface>())
	{
		if (UAbilitySystemComponent* ASC = Cast<IAbilitySystemInterface>(PawnOwner)->GetAbilitySystemComponent())
		{
			// 모든 활성화 가능한 어빌리티를 확인
			const TArray<FGameplayAbilitySpec>& Abilities = ASC->GetActivatableAbilities();
			for (const FGameplayAbilitySpec& Spec : Abilities)
			{
				// 활성화된 어빌리티이고 리로드 태그를 가지고 있는지 확인
				if (Spec.IsActive() && Spec.Ability)
				{
					const FGameplayTagContainer& AssetTags = Spec.Ability->GetAssetTags();
					if (AssetTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Ability.Reload"))))
					{
						UE_LOG(LogTemp, Warning, TEXT("StartFiring: 리로드 중이므로 발사 불가"));
						return;
					}
				}
			}
		}
	}

	// 탄약이 있는지 확인
	if (WeaponItemData->IsAmmoEmpty())
	{
		// TODO: 빈 무기 사운드/애니메이션 재생
		UE_LOG(LogTemp, Warning, TEXT("StartFiring: 탄약이 없음"));
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
			if (ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Ability.Fire")))))
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
	if (!WeaponOwner || !bIsFiring || !WeaponItemData)
	{
		return;
	}

	// 탄약이 있는지 확인
	if (!WeaponItemData->ConsumeAmmo())
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
	if (WeaponItemData->RecoilStrength > 0.0f)
	{
		WeaponOwner->AddWeaponRecoil(WeaponItemData->RecoilStrength);
	}

	// HUD 업데이트
	WeaponOwner->UpdateWeaponHUD(WeaponItemData->CurrentAmmo, WeaponItemData->MagazineSize);

	// 자동 무기의 연사 처리
	if (WeaponItemData->bIsAutomatic && bIsFiring)
	{
		// 다음 발사 예약
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AFPSWeapon::Fire, WeaponItemData->GetRefireRate(), false);
	}
	else
	{
		// 반자동 무기의 경우, 쿨다운 알림 예약
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AFPSWeapon::FireCooldownExpired, WeaponItemData->GetRefireRate(), false);
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
	if (WeaponItemData && WeaponItemData->AccuracySpread > 0.0f)
	{
		FRotator VarianceRotation = FRotator(
			FMath::RandRange(-WeaponItemData->AccuracySpread, WeaponItemData->AccuracySpread),  // Pitch
			FMath::RandRange(-WeaponItemData->AccuracySpread, WeaponItemData->AccuracySpread),  // Yaw
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

int32 AFPSWeapon::GetMagazineSize() const
{
	return WeaponItemData ? WeaponItemData->MagazineSize : 0;
}

int32 AFPSWeapon::GetBulletCount() const
{
	return WeaponItemData ? WeaponItemData->CurrentAmmo : 0;
}

void AFPSWeapon::SetCurrentAmmo(int32 NewAmmo)
{
	if (!WeaponItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetCurrentAmmo: WeaponItemData가 null입니다"));
		return;
	}

	WeaponItemData->CurrentAmmo = FMath::Clamp(NewAmmo, 0, WeaponItemData->MagazineSize);

	// HUD 업데이트
	if (WeaponOwner)
	{
		WeaponOwner->UpdateWeaponHUD(WeaponItemData->CurrentAmmo, WeaponItemData->MagazineSize);
	}
}

bool AFPSWeapon::ConsumeAmmo(int32 AmmoToConsume)
{
	if (!WeaponItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("ConsumeAmmo: WeaponItemData가 null입니다"));
		return false;
	}

	return WeaponItemData->ConsumeAmmo(AmmoToConsume);
}


void AFPSWeapon::SetWeaponItemData(UWeaponItemData* ItemData)
{
	if (!ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetWeaponItemData: ItemData가 null입니다!"));
		return;
	}

	WeaponItemData = ItemData;

	// HUD 업데이트
	if (WeaponOwner)
	{
		WeaponOwner->UpdateWeaponHUD(WeaponItemData->CurrentAmmo, WeaponItemData->MagazineSize);
	}

	UE_LOG(LogTemp, Log, TEXT("WeaponItemData 설정됨: %s"), *ItemData->GetItemName());
}

// ========================================
// IPickupable 인터페이스 구현
// ========================================

bool AFPSWeapon::CanBePickedUp(AFPSCharacter* Character)
{
	if (!Character)
	{
		return false;
	}

	// WeaponSlotComponent 확인
	UWeaponSlotComponent* WeaponSlotComp = Character->GetWeaponSlotComponent();
	if (!WeaponSlotComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("CanBePickedUp: WeaponSlotComponent를 찾을 수 없습니다"));
		return false;
	}

	// 빈 슬롯이 있는지 확인 (모든 슬롯이 차있지 않으면 픽업 가능)
	return WeaponSlotComp->IsSlotEmpty(EWeaponSlot::Primary) ||
		   WeaponSlotComp->IsSlotEmpty(EWeaponSlot::Secondary);
}

bool AFPSWeapon::OnPickedUp(AFPSCharacter* Character)
{
	if (!Character || !CanBePickedUp(Character))
	{
		UE_LOG(LogTemp, Warning, TEXT("OnPickedUp: 장착할 수 없는 상태입니다!"));
		return false;
	}

	// WeaponSlotComponent 가져오기
	UWeaponSlotComponent* WeaponSlotComp = Character->GetWeaponSlotComponent();
	if (!WeaponSlotComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnPickedUp: WeaponSlotComponent를 찾을 수 없습니다"));
		return false;
	}

	// 빈 슬롯 찾기 (Primary 우선, 없으면 Secondary)
	EWeaponSlot TargetSlot = EWeaponSlot::Primary;
	if (!WeaponSlotComp->IsSlotEmpty(EWeaponSlot::Primary))
	{
		if (!WeaponSlotComp->IsSlotEmpty(EWeaponSlot::Secondary))
		{
			//:TODO: 슬롯이 꽉찬 경우 인벤토리 이동 (추후 인벤토리 작업 후 처리)
			UE_LOG(LogTemp, Warning, TEXT("OnPickedUp: 모든 슬롯이 차있습니다"));
			return false;
		}
		TargetSlot = EWeaponSlot::Secondary;
	}

	// 무기 장착 시도
	bool bSuccess = WeaponSlotComp->EquipExistingWeaponToSlot(TargetSlot, this);
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("무기 픽업 성공: %s를 %s 슬롯에 장착"),
			*GetPickupDisplayName(),
			TargetSlot == EWeaponSlot::Primary ? TEXT("Primary") : TEXT("Secondary"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("무기 픽업 실패: %s"), *GetPickupDisplayName());
	}

	return bSuccess;
}

FString AFPSWeapon::GetPickupDisplayName() const
{
	// TODO: WeaponItemData에서 이름 가져오기
	return FString::Printf(TEXT("무기 (%s)"), *GetClass()->GetName());
}

bool AFPSWeapon::IsDropped() const
{
	return bIsDropped;
}

void AFPSWeapon::SetDropped(bool bNewDropped)
{
	bIsDropped = bNewDropped;

	// 픽업 트리거 활성화/비활성화
	if (PickupTrigger)
	{
		if (bIsDropped)
		{
			PickupTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			UE_LOG(LogTemp, Log, TEXT("무기 %s: 드롭 상태로 변경, 픽업 트리거 활성화"), *GetName());
		}
		else
		{
			PickupTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			UE_LOG(LogTemp, Log, TEXT("무기 %s: 장착 상태로 변경, 픽업 트리거 비활성화"), *GetName());
		}
	}
}

void AFPSWeapon::SetWeaponOwner(AActor* WeaponHolder)
{
	if (!WeaponHolder)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetWeaponOwner의 Owner가 null입니다!"));
		return;
	}

	IFPSWeaponHolder* Holder = Cast<IFPSWeaponHolder>(WeaponHolder);
	if (!Holder)
	{
		UE_LOG(LogTemp, Warning, TEXT("Owner는 IFPSWeaponHolder로 구현되지 않았습니다."));
		return;
	}

	SetOwner(WeaponHolder);
	SetInstigator(Cast<APawn>(WeaponHolder));
	WeaponOwner = Holder;
	PawnOwner = Cast<APawn>(WeaponHolder);
}
