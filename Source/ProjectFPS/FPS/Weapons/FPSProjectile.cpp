// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "TimerManager.h"

AFPSProjectile::AFPSProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// Collision Component 생성 (Root)
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	CollisionComponent->InitSphereRadius(15.0f);

	// Collision 설정: 모두 Ignore, Pawn만 Overlap
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	CollisionComponent->SetGenerateOverlapEvents(true);

	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AFPSProjectile::OnHit);
	SetRootComponent(CollisionComponent);

	// Projectile Mesh 생성
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(CollisionComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Projectile Movement Component 생성
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 3000.0f;
	ProjectileMovement->MaxSpeed = 3000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
}

void AFPSProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 7초 후 자동 파괴 타이머 설정
	GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &AFPSProjectile::DestroyProjectile, 7.0f, false);
}

void AFPSProjectile::OnHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 자기 자신이나 Instigator는 무시
	if (OtherActor == this || OtherActor == GetInstigator())
	{
		return;
	}

	// 데미지 적용
	bool bApply = ApplyDamageToTarget(OtherActor);

	if (bApply)
	{
		// 이펙트/사운드 재생
		PlayHitEffects(SweepResult.ImpactPoint);

		// Blueprint 이벤트 호출
		OnProjectileHit(SweepResult);

		// 발사체 파괴
		Destroy();
	}
}

bool AFPSProjectile::ApplyDamageToTarget(AActor* Target)
{
	if (!Target || !DamageEffectClass)
	{
		return false;
	}

	// AbilitySystemComponent 가져오기
	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Target);
	if (!ASI)
	{
		return false;
	}

	UAbilitySystemComponent* TargetASC = ASI->GetAbilitySystemComponent();
	if (!TargetASC)
	{
		return false;
	}

	// GameplayEffect로 데미지 적용
	FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
	ContextHandle.AddInstigator(GetInstigator(), this);

	FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, ContextHandle);
	if (SpecHandle.IsValid())
	{
		// Damage 값을 GameplayEffect Magnitude로 설정
		SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), -Damage);

		// 크리티컬 여부를 SetByCaller로 전달 (0.0 = 일반, 1.0 = 크리티컬)
		SpecHandle.Data->SetSetByCallerMagnitude(
			FGameplayTag::RequestGameplayTag(FName("Data.IsCritical")),
			bIsCriticalHit ? 1.0f : 0.0f
		);

		TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

		UE_LOG(LogTemp, Log, TEXT("발사체 데미지 적용: %.0f to %s (크리티컬: %s)"),
			Damage, *Target->GetName(), bIsCriticalHit ? TEXT("예") : TEXT("아니오"));

		return true;
	}

	return false;
}

void AFPSProjectile::PlayHitEffects(const FVector& HitLocation)
{
	// 파티클 이펙트 재생
	if (HitParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, HitLocation);
	}

	// 사운드 재생
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, HitLocation);
	}
}

void AFPSProjectile::DestroyProjectile()
{
	Destroy();
}