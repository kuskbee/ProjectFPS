// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/EWeaponSlot.h"
#include "UObject/WeakInterfacePtr.h"
#include "WeaponSlotComponent.generated.h"

class UWeaponItemData;
class AFPSWeapon;
class IFPSWeaponHolder;

/**
 * 무기 슬롯 관리 컴포넌트 (캐싱 방식)
 * - 각 슬롯에 무기를 스폰하고 숨김/표시로 전환
 * - 무기 상태(탄약 등) 유지됨
 * - 인벤토리 이동 및 월드 드롭 지원
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTFPS_API UWeaponSlotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponSlotComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 각 슬롯의 무기 아이템 데이터 */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon Slots")
	TArray<TObjectPtr<UWeaponItemData>> WeaponSlots;

	/** 각 슬롯에 스폰된 무기 액터들 (캐싱됨) */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon Slots")
	TArray<TObjectPtr<AFPSWeapon>> SpawnedWeapons;

	/** 현재 활성화된 슬롯 인덱스 */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon Slots")
	int32 ActiveSlotIndex = 0;

	/** WeaponHolder 인터페이스 캐시 (원시 포인터 사용) */
	IFPSWeaponHolder* WeaponHolder;

public:
	// ========================================
	// 무기 장착/해제 시스템
	// ========================================

	/** 빈 슬롯에 무기 장착 (슬롯이 차있으면 실패) */
	UFUNCTION(BlueprintCallable, Category = "Weapon Slots")
	bool EquipWeaponToSlot(EWeaponSlot SlotType, UWeaponItemData* WeaponItem);

	/** 이미 스폰된 무기를 빈 슬롯에 장착 (픽업용) */
	UFUNCTION(BlueprintCallable, Category = "Weapon Slots")
	bool EquipExistingWeaponToSlot(EWeaponSlot SlotType, AFPSWeapon* ExistingWeapon);

	/** 슬롯에서 무기 해제 (아이템 데이터 반환, 무기 액터는 정리) */
	UFUNCTION(BlueprintCallable, Category = "Weapon Slots")
	UWeaponItemData* UnequipWeaponFromSlot(EWeaponSlot SlotType);

	/** 슬롯의 무기를 월드에 드롭 */
	UFUNCTION(BlueprintCallable, Category = "Weapon Slots")
	void DropWeaponFromSlot(EWeaponSlot SlotType);

	/** 현재 활성 슬롯의 무기를 월드에 드롭 */
	UFUNCTION(BlueprintCallable, Category = "Weapon Slots")
	void DropCurrentWeapon();

	// ========================================
	// 슬롯 전환 시스템
	// ========================================

	/** 특정 슬롯으로 전환 */
	UFUNCTION(BlueprintCallable, Category = "Weapon Slots")
	bool SwitchToSlot(EWeaponSlot SlotType);

	/** 다음 슬롯으로 전환 (Primary → Secondary → Primary) */
	UFUNCTION(BlueprintCallable, Category = "Weapon Slots")
	void SwitchToNextSlot();

	/** 숫자키로 슬롯 전환 (1=Primary, 2=Secondary) */
	UFUNCTION(BlueprintCallable, Category = "Weapon Slots")
	bool SwitchToSlotByNumber(int32 SlotNumber);

	/** 두 슬롯 간 무기 교환 */
	UFUNCTION(BlueprintCallable, Category = "Weapon Slots")
	bool SwapWeaponSlots(EWeaponSlot SlotA, EWeaponSlot SlotB);

	// ========================================
	// 정보 조회 시스템
	// ========================================

	/** 현재 활성 슬롯의 무기 아이템 반환 */
	UFUNCTION(BlueprintPure, Category = "Weapon Slots")
	UWeaponItemData* GetActiveWeaponItem() const;

	/** 현재 활성 슬롯 반환 */
	UFUNCTION(BlueprintPure, Category = "Weapon Slots")
	EWeaponSlot GetActiveSlot() const { return static_cast<EWeaponSlot>(ActiveSlotIndex); }

	/** 현재 활성화된 무기 액터 반환 */
	UFUNCTION(BlueprintPure, Category = "Weapon Slots")
	AFPSWeapon* GetCurrentWeaponActor() const;

	/** 특정 슬롯의 무기 아이템 반환 */
	UFUNCTION(BlueprintPure, Category = "Weapon Slots")
	UWeaponItemData* GetWeaponInSlot(EWeaponSlot SlotType) const;

	/** 특정 슬롯이 비어있는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Weapon Slots")
	bool IsSlotEmpty(EWeaponSlot SlotType) const;

	/** 모든 슬롯이 비어있는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Weapon Slots")
	bool AreAllSlotsEmpty() const;

	// ========================================
	// 편의 함수들
	// ========================================

	/** 현재 무기를 보이기/숨기기 **/
	void ShowCurrentWeapon(bool bMakeVisible);

	/** WeaponHolder의 HUD 업데이트 (무기 상태 변경 시 호출) */
	void UpdateWeaponHUD();

protected:
	// ========================================
	// 내부 구현 함수들
	// ========================================

	/** WeaponHolder 인터페이스 초기화 */
	void InitializeWeaponHolder();

	/** 무기 스폰 및 초기화 */
	AFPSWeapon* SpawnWeaponActor(UWeaponItemData* WeaponItem);

	/** 특정 슬롯의 무기를 detach (Drop이나 Switch용) */
	void DetachWeaponFromSlot(int32 SlotIndex, bool bMakeVisible = false);

	/** 무기 보이기/숨기기 **/
	void ShowWeapon(AFPSWeapon* Weapon, bool bMakeVisible);

	/** 특정 슬롯의 무기 완전 정리 (Unequip - 인벤토리 복귀용) */
	void ClearWeaponInSlot(int32 SlotIndex);

	/** 모든 무기 정리 */
	void ClearAllWeapons();

	/** 무기를 월드에 드롭하는 실제 구현 */
	void DropWeaponToWorld(UWeaponItemData* WeaponItem, const FVector& DropLocation);

	/** 슬롯 인덱스 유효성 검사 */
	bool IsValidSlotIndex(int32 SlotIndex) const;

	/** EWeaponSlot을 배열 인덱스로 변환 */
	int32 SlotTypeToIndex(EWeaponSlot SlotType) const { return static_cast<int32>(SlotType); }

public:
	// ========================================
	// 델리게이트들
	// ========================================

	/** 슬롯 변경 시 호출되는 델리게이트 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponSlotChanged, EWeaponSlot, OldSlot, EWeaponSlot, NewSlot);
	UPROPERTY(BlueprintAssignable, Category = "Weapon Slots")
	FOnWeaponSlotChanged OnWeaponSlotChanged;

	/** 무기 장착 시 호출되는 델리게이트 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWeaponEquipped, EWeaponSlot, SlotType, UWeaponItemData*, WeaponItem, AFPSWeapon*, WeaponActor);
	UPROPERTY(BlueprintAssignable, Category = "Weapon Slots")
	FOnWeaponEquipped OnWeaponEquipped;

	/** 무기 해제 시 호출되는 델리게이트 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponUnequipped, EWeaponSlot, SlotType, UWeaponItemData*, WeaponItem);
	UPROPERTY(BlueprintAssignable, Category = "Weapon Slots")
	FOnWeaponUnequipped OnWeaponUnequipped;

	/** 무기 드롭 시 호출되는 델리게이트 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWeaponDropped, EWeaponSlot, SlotType, UWeaponItemData*, WeaponItem, FVector, DropLocation);
	UPROPERTY(BlueprintAssignable, Category = "Weapon Slots")
	FOnWeaponDropped OnWeaponDropped;
};