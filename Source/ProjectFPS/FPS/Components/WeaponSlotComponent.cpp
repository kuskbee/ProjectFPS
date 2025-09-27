// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/Components/WeaponSlotComponent.h"
#include "FPS/Items/WeaponItemData.h"
#include "FPS/Weapons/FPSWeapon.h"
#include "FPS/Weapons/FPSWeaponHolder.h"
#include "FPS/Components/PickupTriggerComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

UWeaponSlotComponent::UWeaponSlotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// 슬롯 배열 초기화 (Primary, Secondary)
	WeaponSlots.SetNum(static_cast<int32>(EWeaponSlot::Max));
	SpawnedWeapons.SetNum(static_cast<int32>(EWeaponSlot::Max));

	// 모든 슬롯을 비어있게 초기화
	for (int32 i = 0; i < static_cast<int32>(EWeaponSlot::Max); ++i)
	{
		WeaponSlots[i] = nullptr;
		SpawnedWeapons[i] = nullptr;
	}

	ActiveSlotIndex = 0;
}

void UWeaponSlotComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeWeaponHolder();
}

void UWeaponSlotComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 모든 무기 정리
	ClearAllWeapons();

	Super::EndPlay(EndPlayReason);
}

void UWeaponSlotComponent::InitializeWeaponHolder()
{
	// 소유자가 IFPSWeaponHolder 인터페이스를 구현하는지 확인
	if (AActor* Owner = GetOwner())
	{
		if (Owner->Implements<UFPSWeaponHolder>())
		{
			WeaponHolder = Cast<IFPSWeaponHolder>(Owner);
			UE_LOG(LogTemp, Log, TEXT("WeaponSlotComponent: WeaponHolder 인터페이스 초기화 완료"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("WeaponSlotComponent: 소유자가 IFPSWeaponHolder 인터페이스를 구현하지 않습니다!"));
		}
	}
}

// ========================================
// 무기 장착/해제 시스템
// ========================================

bool UWeaponSlotComponent::EquipWeaponToSlot(EWeaponSlot SlotType, UWeaponItemData* WeaponItem)
{
	if (!WeaponItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipWeaponToSlot: WeaponItem이 null입니다"));
		return false;
	}

	int32 SlotIndex = SlotTypeToIndex(SlotType);
	if (!IsValidSlotIndex(SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipWeaponToSlot: 잘못된 슬롯 타입입니다"));
		return false;
	}

	// 슬롯이 이미 차있는지 확인
	if (!IsSlotEmpty(SlotType))
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipWeaponToSlot: 슬롯이 이미 차있습니다. 먼저 비워주세요."));
		return false;
	}

	// 무기 스폰
	AFPSWeapon* NewWeapon = SpawnWeaponActor(WeaponItem);
	if (!NewWeapon)
	{
		UE_LOG(LogTemp, Error, TEXT("EquipWeaponToSlot: 무기 스폰에 실패했습니다"));
		return false;
	}

	// 슬롯에 저장
	WeaponSlots[SlotIndex] = WeaponItem;
	SpawnedWeapons[SlotIndex] = NewWeapon;

	// 무기가 장착되었으므로 픽업 트리거 비활성화
	if (UPickupTriggerComponent* PickupTrigger = NewWeapon->FindComponentByClass<UPickupTriggerComponent>())
	{
		PickupTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		UE_LOG(LogTemp, Log, TEXT("무기 장착 - 픽업 트리거 비활성화: %s"), *NewWeapon->GetName());
	}

	// 현재 활성 슬롯이 아니면 숨기기
	if (SlotIndex != ActiveSlotIndex)
	{
		NewWeapon->SetActorHiddenInGame(true);
		NewWeapon->SetActorEnableCollision(false);
	}
	else
	{
		// 현재 활성 슬롯이면 WeaponHolder와 연동
		if (WeaponHolder)
		{
			WeaponHolder->OnWeaponActivated(NewWeapon);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("무기 장착 완료: %s를 %d번 슬롯에"),
		*WeaponItem->GetItemName(), SlotIndex);

	// 델리게이트 호출
	OnWeaponEquipped.Broadcast(SlotType, WeaponItem, NewWeapon);

	return true;
}

UWeaponItemData* UWeaponSlotComponent::UnequipWeaponFromSlot(EWeaponSlot SlotType)
{
	int32 SlotIndex = SlotTypeToIndex(SlotType);
	if (!IsValidSlotIndex(SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("UnequipWeaponFromSlot: 잘못된 슬롯 타입입니다"));
		return nullptr;
	}

	if (IsSlotEmpty(SlotType))
	{
		UE_LOG(LogTemp, Warning, TEXT("UnequipWeaponFromSlot: 슬롯이 이미 비어있습니다"));
		return nullptr;
	}

	// 아이템 데이터 백업
	UWeaponItemData* WeaponItem = WeaponSlots[SlotIndex];

	// 현재 활성 슬롯이면 WeaponHolder에 알림
	if (SlotIndex == ActiveSlotIndex && WeaponHolder)
	{
		WeaponHolder->OnWeaponDeactivated(SpawnedWeapons[SlotIndex]);
	}

	// 무기 정리
	ClearWeaponInSlot(SlotIndex);

	// 활성 슬롯이 비워진 경우 빈 손 상태 유지 (자동 전환하지 않음)
	if (SlotIndex == ActiveSlotIndex)
	{
		UE_LOG(LogTemp, Log, TEXT("활성 슬롯이 비워짐 - 빈 손 상태 유지"));
	}

	UE_LOG(LogTemp, Log, TEXT("무기 해제 완료: %s"), *WeaponItem->GetItemName());

	// 델리게이트 호출
	OnWeaponUnequipped.Broadcast(SlotType, WeaponItem);

	return WeaponItem;
}

void UWeaponSlotComponent::DropWeaponFromSlot(EWeaponSlot SlotType)
{
	int32 SlotIndex = SlotTypeToIndex(SlotType);
	if (!IsValidSlotIndex(SlotIndex) || IsSlotEmpty(SlotType))
	{
		return;
	}

	UWeaponItemData* WeaponItem = WeaponSlots[SlotIndex];
	AFPSWeapon* WeaponActor = SpawnedWeapons[SlotIndex];

	// 드롭 위치 계산 (소유자 앞쪽으로)
	FVector DropLocation = GetOwner()->GetActorLocation() +
		GetOwner()->GetActorForwardVector() * 200.0f;

	// 현재 활성 슬롯이면 WeaponHolder에 비활성화 알림
	if (SlotIndex == ActiveSlotIndex && WeaponHolder && WeaponActor)
	{
		WeaponHolder->OnWeaponDeactivated(WeaponActor);
	}

	// 무기를 detach하고 보이게 설정 (Drop용)
	DetachWeaponFromSlot(SlotIndex, true);

	// 무기 위치를 드롭 위치로 이동
	if (WeaponActor)
	{
		WeaponActor->SetActorLocation(DropLocation);
		WeaponActor->SetActorRotation(FRotator::ZeroRotator);
	}

	// 슬롯에서 제거 (무기 액터는 월드에 남김)
	WeaponSlots[SlotIndex] = nullptr;
	SpawnedWeapons[SlotIndex] = nullptr;

	// 활성 슬롯이 비워진 경우 빈 손 상태 유지
	if (SlotIndex == ActiveSlotIndex)
	{
		UE_LOG(LogTemp, Log, TEXT("활성 슬롯 드롭 - 빈 손 상태 유지"));
	}

	UE_LOG(LogTemp, Log, TEXT("무기 드롭: %s"), *WeaponItem->GetItemName());

	// 델리게이트 호출
	OnWeaponDropped.Broadcast(SlotType, WeaponItem, DropLocation);
}

void UWeaponSlotComponent::DropCurrentWeapon()
{
	if (!AreAllSlotsEmpty())
	{
		DropWeaponFromSlot(GetActiveSlot());
	}
}

// ========================================
// 슬롯 전환 시스템
// ========================================

bool UWeaponSlotComponent::SwitchToSlot(EWeaponSlot SlotType)
{
	int32 NewSlotIndex = SlotTypeToIndex(SlotType);
	if (!IsValidSlotIndex(NewSlotIndex))
	{
		return false;
	}

	// 같은 슬롯이면 무시
	if (NewSlotIndex == ActiveSlotIndex)
	{
		return true;
	}

	EWeaponSlot OldSlot = GetActiveSlot();

	// 현재 무기 숨기기
	if (SpawnedWeapons[ActiveSlotIndex])
	{
		SpawnedWeapons[ActiveSlotIndex]->SetActorHiddenInGame(true);
		SpawnedWeapons[ActiveSlotIndex]->SetActorEnableCollision(false);

		if (WeaponHolder)
		{
			WeaponHolder->OnWeaponDeactivated(SpawnedWeapons[ActiveSlotIndex]);
		}
	}

	// 새 슬롯으로 전환
	ActiveSlotIndex = NewSlotIndex;

	// 새 무기 표시
	if (SpawnedWeapons[ActiveSlotIndex])
	{
		SpawnedWeapons[ActiveSlotIndex]->SetActorHiddenInGame(false);
		SpawnedWeapons[ActiveSlotIndex]->SetActorEnableCollision(true);

		if (WeaponHolder)
		{
			WeaponHolder->OnWeaponActivated(SpawnedWeapons[ActiveSlotIndex]);
		}

		UE_LOG(LogTemp, Log, TEXT("슬롯 전환: %s"),
			*WeaponSlots[ActiveSlotIndex]->GetItemName());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("빈 슬롯으로 전환"));
	}

	// 델리게이트 호출
	OnWeaponSlotChanged.Broadcast(OldSlot, SlotType);

	return true;
}

void UWeaponSlotComponent::SwitchToNextSlot()
{
	int32 NextSlot = (ActiveSlotIndex + 1) % static_cast<int32>(EWeaponSlot::Max);
	SwitchToSlot(static_cast<EWeaponSlot>(NextSlot));
}

bool UWeaponSlotComponent::SwitchToSlotByNumber(int32 SlotNumber)
{
	// 1=Primary, 2=Secondary
	if (SlotNumber == 1)
	{
		return SwitchToSlot(EWeaponSlot::Primary);
	}
	else if (SlotNumber == 2)
	{
		return SwitchToSlot(EWeaponSlot::Secondary);
	}

	return false;
}

// ========================================
// 정보 조회 시스템
// ========================================

UWeaponItemData* UWeaponSlotComponent::GetActiveWeaponItem() const
{
	if (IsValidSlotIndex(ActiveSlotIndex))
	{
		return WeaponSlots[ActiveSlotIndex];
	}
	return nullptr;
}

AFPSWeapon* UWeaponSlotComponent::GetCurrentWeaponActor() const
{
	if (IsValidSlotIndex(ActiveSlotIndex))
	{
		return SpawnedWeapons[ActiveSlotIndex];
	}
	return nullptr;
}

UWeaponItemData* UWeaponSlotComponent::GetWeaponInSlot(EWeaponSlot SlotType) const
{
	int32 SlotIndex = SlotTypeToIndex(SlotType);
	if (IsValidSlotIndex(SlotIndex))
	{
		return WeaponSlots[SlotIndex];
	}
	return nullptr;
}

bool UWeaponSlotComponent::IsSlotEmpty(EWeaponSlot SlotType) const
{
	int32 SlotIndex = SlotTypeToIndex(SlotType);
	if (IsValidSlotIndex(SlotIndex))
	{
		return WeaponSlots[SlotIndex] == nullptr;
	}
	return true;
}

bool UWeaponSlotComponent::AreAllSlotsEmpty() const
{
	for (const auto& WeaponItem : WeaponSlots)
	{
		if (WeaponItem != nullptr)
		{
			return false;
		}
	}
	return true;
}

void UWeaponSlotComponent::ShowCurrentWeapon(bool bMakeVisible)
{
	AFPSWeapon* CurrentWeapon = GetCurrentWeaponActor();
	if (!CurrentWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("무기를 가지고 있지 않음!"));
		return;
	}

	ShowWeapon(CurrentWeapon, bMakeVisible);
}

// ========================================
// 내부 구현 함수들
// ========================================

AFPSWeapon* UWeaponSlotComponent::SpawnWeaponActor(UWeaponItemData* WeaponItem)
{
	if (!WeaponItem || !WeaponItem->IsValidWeapon())
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnWeaponActor: 잘못된 WeaponItem입니다"));
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnWeaponActor: World가 null입니다"));
		return nullptr;
	}

	// 무기 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AFPSWeapon* NewWeapon = World->SpawnActor<AFPSWeapon>(
		WeaponItem->WeaponClass,
		GetOwner()->GetActorLocation(),
		GetOwner()->GetActorRotation(),
		SpawnParams
	);

	if (NewWeapon)
	{
		// WeaponItemData 설정
		NewWeapon->SetWeaponItemData(WeaponItem);

		// WeaponHolder와 무기 연결
		if (WeaponHolder)
		{
			WeaponHolder->AttachWeaponMeshes(NewWeapon);
		}

		UE_LOG(LogTemp, Log, TEXT("무기 스폰 완료: %s"), *WeaponItem->GetItemName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("무기 스폰 실패: %s"), *WeaponItem->GetItemName());
	}

	return NewWeapon;
}

void UWeaponSlotComponent::DetachWeaponFromSlot(int32 SlotIndex, bool bMakeVisible)
{
	if (!IsValidSlotIndex(SlotIndex) || !SpawnedWeapons[SlotIndex])
	{
		return;
	}

	AFPSWeapon* Weapon = SpawnedWeapons[SlotIndex];

	// 무기 메시들을 detach
	if (USkeletalMeshComponent* FPMesh = Weapon->GetFirstPersonMesh())
	{
		FPMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}
	if (USkeletalMeshComponent* TPMesh = Weapon->GetThirdPersonMesh())
	{
		TPMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}

	// 무기 비활성화
	Weapon->DeactivateWeapon();

	// Drop인 경우 보이게, Switch인 경우 숨기게
	ShowWeapon(Weapon, bMakeVisible);

	// Drop인 경우 픽업 트리거 활성화
	if (bMakeVisible)
	{
		if (UPickupTriggerComponent* PickupTrigger = Weapon->FindComponentByClass<UPickupTriggerComponent>())
		{
			PickupTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			UE_LOG(LogTemp, Log, TEXT("무기 드롭 - 픽업 트리거 활성화: %s"), *Weapon->GetName());
		}
	}
}

void UWeaponSlotComponent::ShowWeapon(AFPSWeapon* Weapon, bool bMakeVisible)
{
	if (!Weapon)
	{
		return;
	}
	Weapon->SetActorHiddenInGame(!bMakeVisible);
	Weapon->SetActorEnableCollision(bMakeVisible);
}

void UWeaponSlotComponent::ClearWeaponInSlot(int32 SlotIndex)
{
	if (!IsValidSlotIndex(SlotIndex))
	{
		return;
	}

	// 무기 액터 완전 정리 (Unequip - 인벤토리 복귀용)
	if (SpawnedWeapons[SlotIndex])
	{
		// 먼저 detach
		DetachWeaponFromSlot(SlotIndex, false);

		// 그리고 파괴 (인벤토리 복귀이므로 월드에서 제거)
		SpawnedWeapons[SlotIndex]->Destroy();
		SpawnedWeapons[SlotIndex] = nullptr;
	}

	// 아이템 데이터 정리
	WeaponSlots[SlotIndex] = nullptr;
}

void UWeaponSlotComponent::ClearAllWeapons()
{
	for (int32 i = 0; i < SpawnedWeapons.Num(); ++i)
	{
		ClearWeaponInSlot(i);
	}

	UE_LOG(LogTemp, Log, TEXT("모든 무기 정리 완료"));
}

void UWeaponSlotComponent::DropWeaponToWorld(UWeaponItemData* WeaponItem, const FVector& DropLocation)
{
	if (!WeaponItem || !WeaponItem->IsValidWeapon())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// TODO: 나중에 PickupActor 시스템 구현 시 여기서 드롭 처리
	// 지금은 로그만 출력
	UE_LOG(LogTemp, Log, TEXT("무기를 월드에 드롭: %s at %s"),
		*WeaponItem->GetItemName(), *DropLocation.ToString());

	// 임시: 간단한 StaticMesh로 드롭 표시 (나중에 PickupActor로 대체)
	/*
	AActor* DropActor = World->SpawnActor<AActor>(DropLocation, FRotator::ZeroRotator);
	if (DropActor)
	{
		UStaticMeshComponent* MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DropMesh"));
		// 여기서 무기 메시 설정 등...
	}
	*/
}

bool UWeaponSlotComponent::IsValidSlotIndex(int32 SlotIndex) const
{
	return SlotIndex >= 0 && SlotIndex < static_cast<int32>(EWeaponSlot::Max);
}