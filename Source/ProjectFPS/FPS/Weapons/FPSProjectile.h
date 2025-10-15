// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UParticleSystem;
class USoundBase;

/**
 * FPS 발사체 기본 클래스
 * - 충돌 처리 (OnComponentBeginOverlap)
 * - 데미지 적용 (GameplayEffect 사용)
 * - Blueprint에서 상속하여 메시/이펙트/사운드 추가 가능
 */
UCLASS()
class PROJECTFPS_API AFPSProjectile : public AActor
{
	GENERATED_BODY()

public:
	AFPSProjectile();

protected:
	virtual void BeginPlay() override;

public:
	// ========================================
	// Components
	// ========================================

	/** 충돌 감지용 Sphere 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	/** 발사체 이동 처리 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	/** 발사체 메시 (Blueprint에서 설정) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;

	// ========================================
	// Damage Settings
	// ========================================

	/** 데미지 적용용 GameplayEffect 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TSubclassOf<class UGameplayEffect> DamageEffectClass;

	/** 이 발사체가 적용할 데미지 값 (무기에서 설정) */
	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	float Damage = 10.0f;

	/** 크리티컬 여부 (무기에서 설정) */
	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	bool bIsCriticalHit = false;

	/** 데미지 값 및 크리티컬 설정 함수 (무기에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void SetDamage(float NewDamage, bool bCritical = false)
	{
		Damage = NewDamage;
		bIsCriticalHit = bCritical;
	}

	/** 데미지 값 반환 함수 */
	UFUNCTION(BlueprintPure, Category = "Damage")
	float GetDamage() const { return Damage; }

	/** 크리티컬 여부 반환 함수 */
	UFUNCTION(BlueprintPure, Category = "Damage")
	bool IsCriticalHit() const { return bIsCriticalHit; }

	// ========================================
	// Visual/Audio Effects
	// ========================================

	/** 충돌 시 재생할 파티클 이펙트 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<UParticleSystem> HitParticle;

	/** 충돌 시 재생할 사운드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<USoundBase> HitSound;

	// ========================================
	// Collision Events
	// ========================================

	/** 충돌 시 호출되는 함수 (OnComponentBeginOverlap에 바인딩) */
	UFUNCTION()
	void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** 데미지 적용 함수 */
	bool ApplyDamageToTarget(AActor* Target);

	/** 충돌 후 이펙트/사운드 재생 */
	void PlayHitEffects(const FVector& HitLocation);

	/** 충돌 후 처리 (Blueprint에서 오버라이드 가능) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile")
	void OnProjectileHit(const FHitResult& HitResult);

protected:
	/** 7초 후 자동 파괴 타이머 핸들 */
	FTimerHandle DestroyTimerHandle;

	/** 발사체 자동 파괴 함수 */
	void DestroyProjectile();
};
