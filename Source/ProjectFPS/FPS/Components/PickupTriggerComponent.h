// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "PickupTriggerComponent.generated.h"

class AFPSCharacter;
class IPickupable;

/**
 * IPickupable 객체에 부착하여 플레이어와의 Overlap을 감지하고 픽업을 처리하는 컴포넌트
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTFPS_API UPickupTriggerComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	explicit UPickupTriggerComponent(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;

	/** 픽업 시도 쿨다운 (연속 픽업 방지) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup Settings", meta = (ClampMin = 0.1, ClampMax = 5.0))
	float PickupCooldown = 0.5f;

	/** 마지막 픽업 시도 시간 */
	float LastPickupAttemptTime = 0.0f;

	/** 픽업 성공 시 화면에 메시지를 표시할지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup Settings")
	bool bShowPickupMessage = true;

public:
	/** Overlap 시작 시 호출 */
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** 픽업 처리 */
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	bool TryPickup(AFPSCharacter* Character);

	/** 픽업 가능한지 확인 */
	UFUNCTION(BlueprintPure, Category = "Pickup")
	bool CanPickup(AFPSCharacter* Character) const;

	/** 픽업 범위 설정 */
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void SetPickupRange(float NewRadius);

protected:
	/** 소유자가 IPickupable을 구현하는지 확인하고 반환 */
	IPickupable* GetPickupableOwner() const;
};