// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSWeaponHolder.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"
#include "FPS/Interfaces/Pickupable.h"
#include "FPSWeapon.generated.h"

class IFPSWeaponHolder;
class AProjectile;
class USkeletalMeshComponent;
class UAnimMontage;
class UAnimInstance;
class UGameplayAbility;
class UPickupTriggerComponent;

/**
 * GAS 통합 FPS 무기를 위한 기본 클래스
 * 1인칭 및 3인칭 시점 메시 제공
 * GameplayAbilities를 통한 탄약 및 발사 로직 처리
 * FPSWeaponHolder 인터페이스를 통해 무기 소유자와 상호작용
 */
UCLASS(abstract, BlueprintType, Blueprintable)
class PROJECTFPS_API AFPSWeapon : public AActor, public IPickupable
{
	GENERATED_BODY()

	/** 1인칭 시점 메시 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;

	/** 3인칭 시점 메시 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> ThirdPersonMesh;

	/** 픽업 감지용 트리거 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPickupTriggerComponent> PickupTrigger;

protected:

	/** 무기 소유자에 대한 캐스트 포인터 */
	IFPSWeaponHolder* WeaponOwner;

	/** 이 무기 발사 시 사용할 GameplayAbility */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Abilities")
	TSubclassOf<UGameplayAbility> FireAbility;

	/** 이 무기의 게임플레이 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GameplayTags")
	FGameplayTagContainer WeaponTags;

	/** 이 무기가 발사할 발사체 타입 (BP_Projectile 참조용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ammo")
	TSubclassOf<AActor> ProjectileClass;

	/** 이 무기의 모든 데이터 (마스터 데이터 + 런타임 상태) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Data")
	TObjectPtr<class UWeaponItemData> WeaponItemData;

	/** 이 무기 발사 시 재생할 애니메이션 몽타주 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UAnimMontage> FiringMontage;

	/** 이 무기 리로드 시 재생할 애니메이션 몽타주 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UAnimMontage> ReloadMontage;

	/** 이 무기가 활성화될 때 1인칭 캐릭터 메시에 설정할 AnimInstance 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation")
	TSubclassOf<UAnimInstance> FirstPersonAnimInstanceClass;

	/** 이 무기가 활성화될 때 3인칭 캐릭터 메시에 설정할 AnimInstance 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation")
	TSubclassOf<UAnimInstance> ThirdPersonAnimInstanceClass;

	// 조준 관련 데이터는 WeaponItemData에서 관리 (AccuracySpread, RecoilStrength)

	/** 발사체가 생성될 1인칭 총구 소켓 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Aim")
	FName MuzzleSocketName = FName("Muzzle");

	/** 총구 앞쪽 탄약이 생성될 거리 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Aim", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
	float MuzzleOffset = 10.0f;

	// 연사 관련 데이터는 WeaponItemData에서 관리 (bIsAutomatic, FireRate)

	/** 마지막 발사 시점, 반자동 연사 속도 제한에 사용 */
	float TimeOfLastShot = 0.0f;

	/** true인 경우, 무기가 현재 발사 중 */
	bool bIsFiring = false;

	/** 자동 연사 처리용 타이머 */
	FTimerHandle RefireTimer;

	/** AI 인식 시스템 상호작용을 위한 소유자 폰 포인터 캐스트 */
	TObjectPtr<APawn> PawnOwner;

	/** AI 인식 시스템 상호작용을 위한 발사 소음 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Perception", meta = (ClampMin = 0, ClampMax = 100))
	float ShotLoudness = 1.0f;

	/** 발사 AI 인식 소음의 최대 범위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Perception", meta = (ClampMin = 0, ClampMax = 100000, Units = "cm"))
	float ShotNoiseRange = 3000.0f;

	/** 이 무기 발사로 생성되는 소음에 적용할 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Perception")
	FName ShotNoiseTag = FName("Shot");


public:

	/** 생성자 */
	AFPSWeapon();

protected:

	/** 게임플레이 초기화 */
	virtual void BeginPlay() override;

	/** 게임플레이 정리 */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

protected:

	/** 무기 소유자가 파괴될 때 호출됨 */
	UFUNCTION()
	void OnOwnerDestroyed(AActor* DestroyedActor);

public:

	/** 이 무기를 활성화하고 발사 준비를 함 */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void ActivateWeapon();

	/** 이 무기를 비활성화함 */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void DeactivateWeapon();

	/** 이 무기 발사 시작 (FPSCharacter에서 호출) */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void StartFiring();

	/** 이 무기 발사 중지 */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void StopFiring();

	/** 무기 발사 (GameplayAbility에서 호출) */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	virtual void Fire();

	/** 목표 위치를 향해 발사체 발사 (Fire()에서 호출) */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	virtual void FireProjectile(const FVector& TargetLocation);

	/** 이 무기가 발사하는 발사체의 생성 트랜스폼 계산 */
	UFUNCTION(BlueprintPure, Category="Weapon")
	FTransform CalculateProjectileSpawnTransform(const FVector& TargetLocation) const;

protected:

	/** 반자동 무기 발사 시 연사 속도 시간이 경과했을 때 호출됨 */
	void FireCooldownExpired();

	/** 크리티컬 데미지 계산 (CritChance/CritDamage 기반) */
	/** 최종 데미지 계산 (크리티컬 포함) */
	float CalculateFinalDamage(bool& bOutIsCritical) const;

	/** AttackSpeedMultiplier를 반영한 실제 연사 간격 반환 */
	float GetCurrentRefireRate() const;

public:

	/** 1인칭 메시 반환 */
	UFUNCTION(BlueprintPure, Category="Weapon")
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; };

	/** 3인칭 메시 반환 */
	UFUNCTION(BlueprintPure, Category="Weapon")
	USkeletalMeshComponent* GetThirdPersonMesh() const { return ThirdPersonMesh; };

	/** 1인칭 애님 인스턴스 클래스 반환 */
	UFUNCTION(BlueprintPure, Category="Weapon")
	TSubclassOf<UAnimInstance> GetFirstPersonAnimInstanceClass() const;

	/** 3인칭 애님 인스턴스 클래스 반환 */
	UFUNCTION(BlueprintPure, Category="Weapon")
	TSubclassOf<UAnimInstance> GetThirdPersonAnimInstanceClass() const;

	/** 탄창 크기 반환 */
	UFUNCTION(BlueprintPure, Category="Weapon")
	int32 GetMagazineSize() const;

	/** 현재 탄약 수 반환 */
	UFUNCTION(BlueprintPure, Category="Weapon")
	int32 GetBulletCount() const;

	/** 발사 어빌리티 클래스 반환 */
	UFUNCTION(BlueprintPure, Category="Weapon")
	TSubclassOf<UGameplayAbility> GetFireAbility() const { return FireAbility; }

	/** 무기 태그 반환 */
	UFUNCTION(BlueprintPure, Category="Weapon")
	FGameplayTagContainer GetWeaponTags() const { return WeaponTags; }

	/** 이 무기의 발사체 클래스 반환 */
	UFUNCTION(BlueprintPure, Category="Weapon")
	TSubclassOf<AActor> GetProjectileClass() const { return ProjectileClass; }

	/** 리로드 애니메이션 몽타주 반환 */
	UFUNCTION(BlueprintPure, Category="Weapon")
	UAnimMontage* GetReloadMontage() const { return ReloadMontage; }

	/** 현재 탄약 설정 (WeaponItemData에 위임) */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void SetCurrentAmmo(int32 NewAmmo);

	/** 탄약 소모 (WeaponItemData에 위임) */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	bool ConsumeAmmo(int32 AmmoToConsume = 1);

	/** WeaponItemData 설정 (모든 데이터를 이 객체로 위임) */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void SetWeaponItemData(class UWeaponItemData* ItemData);

	/** WeaponItemData 반환 */
	UFUNCTION(BlueprintPure, Category="Weapon")
	class UWeaponItemData* GetWeaponItemData() const { return WeaponItemData; }

	// ========================================
	// IPickupable 인터페이스 구현
	// ========================================

	/** 이 무기를 픽업할 수 있는지 확인 */
	virtual bool CanBePickedUp(AFPSCharacter* Character) override;

	/** 무기 픽업 시 호출 */
	virtual bool OnPickedUp(AFPSCharacter* Character) override;

	/** 픽업 시 표시할 무기 이름 */
	virtual FString GetPickupDisplayName() const override;

	/** 아이템이 월드에 드롭된 상태인지 확인 */
	virtual bool IsDropped() const override;

	/** 아이템의 드롭 상태 설정 */
	virtual void SetDropped(bool bNewDropped) override;

	// ========================================
	// 편의 함수
	// ========================================

	void SetWeaponOwner(AActor* WeaponHolder);

protected:
	/** 무기가 월드에 드롭된 상태인지 (픽업 가능한지) */
	UPROPERTY(BlueprintReadOnly, Category="Pickup")
	bool bIsDropped = false;
};