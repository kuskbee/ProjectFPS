// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interfaces/Pickupable.h"
#include "PickupItemActor.generated.h"

class UStaticMeshComponent;
class USkeletalMeshComponent;
class UPickupTriggerComponent;
class UBaseItemData;

/**
 * 월드에 떨어진 픽업 가능한 아이템 Actor
 * - 포션, 탄약, 장비 등 모든 아이템에 사용 가능
 * - IPickupable 인터페이스 구현
 */
UCLASS()
class PROJECTFPS_API APickupItemActor : public AActor, public IPickupable
{
	GENERATED_BODY()

public:
	APickupItemActor();

protected:
	virtual void BeginPlay() override;

public:
	// IPickupable 인터페이스 구현
	virtual bool CanBePickedUp(AFPSCharacter* Character) override;
	virtual bool OnPickedUp(AFPSCharacter* Character) override;
	virtual FString GetPickupDisplayName() const override;
	virtual bool IsDropped() const override { return bIsDropped; }
	virtual void SetDropped(bool bNewDropped) override { bIsDropped = bNewDropped; }

	// 아이템 데이터 설정
	void SetItemData(UBaseItemData* InItemData);

protected:
	// 컴포넌트들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPickupTriggerComponent> PickupTrigger;

	// 아이템 데이터
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UBaseItemData> ItemData;

	// 픽업 상태
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	bool bIsDropped = true;
};
