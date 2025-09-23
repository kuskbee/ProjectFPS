// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbility_FireProjectile.generated.h"

/**
 * 무기 발사를 위한 기본 GameplayAbility
 * 각 무기가 이 클래스를 상속받아 고유한 발사 로직 구현
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTFPS_API UGameplayAbility_FireProjectile : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_FireProjectile();

protected:
	/** 발사할 발사체 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<AActor> ProjectileClass;

	/** 발사 시 재생할 사운드 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	class USoundBase* FireSound;

	/** 발사 시 재생할 파티클 이펙트 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	class UParticleSystem* MuzzleFlash;

	/** 머즐 플래시 재생을 위한 소켓 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	FName MuzzleSocketName = FName("Muzzle");

public:
	/** Ability 활성화 시 호출 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** 실제 발사 로직 (Blueprint에서 오버라이드 가능) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Firing")
	void BlueprintFireProjectile();

	/** C++에서 발사체 스폰 */
	UFUNCTION(BlueprintCallable, Category = "Firing")
	void SpawnProjectile(const FVector& TargetLocation);

	/** 현재 장착된 무기 가져오기 */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	class AFPSWeapon* GetCurrentWeapon() const;

	/** 발사 가능 여부 확인 */
	UFUNCTION(BlueprintPure, Category = "Firing")
	bool CanFire() const;
};
