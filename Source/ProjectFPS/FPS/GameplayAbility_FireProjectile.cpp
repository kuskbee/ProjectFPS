// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/GameplayAbility_FireProjectile.h"
#include "FPS/FPSCharacter.h"
#include "FPS/Weapons/FPSWeapon.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"

UGameplayAbility_FireProjectile::UGameplayAbility_FireProjectile()
{
	// 기본 설정
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 기본 머즐 소켓 이름
	MuzzleSocketName = FName("Muzzle");
}

void UGameplayAbility_FireProjectile::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 발사 가능한지 확인
	if (!CanFire())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// Blueprint 구현이 있으면 Blueprint 버전 실행
	BlueprintFireProjectile();

	// 기본 C++ 구현: 무기가 있으면 무기의 Fire() 호출
	if (AFPSWeapon* CurrentWeapon = GetCurrentWeapon())
	{
		CurrentWeapon->Fire();
	}

	// Ability 종료
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGameplayAbility_FireProjectile::SpawnProjectile(const FVector& TargetLocation)
{
	if (!GetWorld())
	{
		return;
	}

	// 현재 무기에서 발사체 클래스 가져오기
	TSubclassOf<AActor> ProjectileToSpawn = ProjectileClass;

	if (AFPSWeapon* CurrentWeapon = GetCurrentWeapon())
	{
		// 무기에 설정된 발사체 클래스가 있으면 우선 사용
		if (CurrentWeapon->GetProjectileClass())
		{
			ProjectileToSpawn = CurrentWeapon->GetProjectileClass();
		}
	}

	if (!ProjectileToSpawn)
	{
		return;
	}

	// 발사체 스폰 위치와 회전 계산
	FTransform SpawnTransform;
	if (AFPSWeapon* CurrentWeapon = GetCurrentWeapon())
	{
		SpawnTransform = CurrentWeapon->CalculateProjectileSpawnTransform(TargetLocation);
	}
	else
	{
		// 무기가 없으면 액터 위치에서 발사
		SpawnTransform = FTransform(GetAvatarActorFromActorInfo()->GetActorRotation(),
			GetAvatarActorFromActorInfo()->GetActorLocation());
	}

	// 발사체 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetAvatarActorFromActorInfo();
	SpawnParams.Instigator = Cast<APawn>(GetAvatarActorFromActorInfo());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* Projectile = GetWorld()->SpawnActor<AActor>(ProjectileToSpawn, SpawnTransform, SpawnParams);

	// 사운드 재생
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, SpawnTransform.GetLocation());
	}

	// 머즐 플래시 재생
	if (MuzzleFlash && GetCurrentWeapon())
	{
		if (USkeletalMeshComponent* WeaponMesh = GetCurrentWeapon()->GetFirstPersonMesh())
		{
			UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, WeaponMesh, MuzzleSocketName);
		}
	}
}

AFPSWeapon* UGameplayAbility_FireProjectile::GetCurrentWeapon() const
{
	if (AFPSCharacter* FPSCharacter = Cast<AFPSCharacter>(GetAvatarActorFromActorInfo()))
	{
		return FPSCharacter->GetCurrentWeapon();
	}

	return nullptr;
}

bool UGameplayAbility_FireProjectile::CanFire() const
{
	// 기본 Ability 체크
	if (!CanActivateAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo()))
	{
		return false;
	}

	// 무기가 있는지 확인
	AFPSWeapon* CurrentWeapon = GetCurrentWeapon();
	if (!CurrentWeapon)
	{
		return true; // 무기가 없어도 기본 발사는 가능
	}

	// 탄약이 있는지 확인
	if (CurrentWeapon->GetBulletCount() <= 0)
	{
		return false;
	}

	return true;
}

