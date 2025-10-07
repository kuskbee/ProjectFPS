// ShieldBarrierActor.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShieldBarrierActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class AFPSProjectile;

/**
 * 방어막 구체 Actor
 * - 외부 공격만 차단 (Owner는 통과)
 * - HP 기반 내구도 시스템
 * - HP 소진 시 자동 파괴
 */
UCLASS()
class PROJECTFPS_API AShieldBarrierActor : public AActor
{
	GENERATED_BODY()

public:
	AShieldBarrierActor();

	virtual void Tick(float DeltaTime) override;

	/** 방어막 HP 설정 */
	UFUNCTION(BlueprintCallable, Category = "Shield Barrier")
	void SetBarrierHealth(float InHealth);

	/** 현재 방어막 HP 반환 */
	UFUNCTION(BlueprintPure, Category = "Shield Barrier")
	float GetCurrentHealth() const { return CurrentHealth; }

	/** 최대 방어막 HP 반환 */
	UFUNCTION(BlueprintPure, Category = "Shield Barrier")
	float GetMaxHealth() const { return MaxHealth; }

protected:
	virtual void BeginPlay() override;

	/** 충돌 이벤트 핸들러 */
	UFUNCTION()
	void OnBarrierHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

	/** 발사체가 방어막에 들어왔을 때 */
	UFUNCTION()
	void OnProjectileBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	/** 방어막 메시 (반투명 구체) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> BarrierMesh;

	/** 충돌 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> CollisionSphere;

	/** 현재 방어막 HP */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shield Barrier", meta = (AllowPrivateAccess = "true"))
	float CurrentHealth;

	/** 최대 방어막 HP */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shield Barrier", meta = (AllowPrivateAccess = "true"))
	float MaxHealth = 200.0f;

	/** 방어막 구체 반지름 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shield Barrier", meta = (AllowPrivateAccess = "true"))
	float BarrierRadius = 150.0f;

	/** 대미지 처리 (내부에서 발사된 발사체는 무시) */
	void ProcessDamage(AActor* DamageCauser, float Damage);

	/** Owner에서 발사된 발사체인지 체크 */
	bool IsProjectileFromOwner(AActor* ProjectileActor) const;
};
