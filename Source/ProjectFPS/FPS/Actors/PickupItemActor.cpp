// Fill out your copyright notice in the Description page of Project Settings.

#include "PickupItemActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "../Components/PickupTriggerComponent.h"
#include "../Components/InventoryComponent.h"
#include "../Items/BaseItemData.h"
#include "../FPSPlayerCharacter.h"

APickupItemActor::APickupItemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Static Mesh 컴포넌트 생성 (포션, 탄약 등)
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	RootComponent = StaticMeshComponent;
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMeshComponent->SetForceDisableNanite(true);

	// Skeletal Mesh 컴포넌트 생성 (무기, 장비 등)
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetupAttachment(RootComponent);
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Pickup Trigger 컴포넌트 생성
	PickupTrigger = CreateDefaultSubobject<UPickupTriggerComponent>(TEXT("PickupTrigger"));
	PickupTrigger->SetupAttachment(RootComponent);
	PickupTrigger->SetSphereRadius(150.0f);  // 픽업 범위
}

void APickupItemActor::BeginPlay()
{
	Super::BeginPlay();

	// ItemData에서 메시 설정
	if (ItemData)
	{
		// WorldMesh 가져오기
		UObject* WorldMesh = ItemData->GetWorldMesh();

		// StaticMesh인 경우
		if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(WorldMesh))
		{
			StaticMeshComponent->SetStaticMesh(StaticMesh);
			StaticMeshComponent->SetVisibility(true);
			SkeletalMeshComponent->SetVisibility(false);
		}
		// SkeletalMesh인 경우
		else if (USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(WorldMesh))
		{
			SkeletalMeshComponent->SetSkeletalMesh(SkeletalMesh);
			SkeletalMeshComponent->SetVisibility(true);
			StaticMeshComponent->SetVisibility(false);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("PickupItemActor: WorldMesh가 설정되지 않았습니다 - %s"),
				*ItemData->GetItemName());
		}
	}
}

bool APickupItemActor::CanBePickedUp(AFPSCharacter* Character)
{
	// 드롭된 상태이고, ItemData가 있으면 픽업 가능
	return bIsDropped && ItemData != nullptr;
}

bool APickupItemActor::OnPickedUp(AFPSCharacter* Character)
{
	if (!Character || !ItemData)
	{
		return false;
	}

	// FPSPlayerCharacter로 캐스팅 (InventoryComponent 접근용)
	AFPSPlayerCharacter* PlayerCharacter = Cast<AFPSPlayerCharacter>(Character);
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnPickedUp: FPSPlayerCharacter가 아닙니다"));
		return false;
	}

	// InventoryComponent 가져오기
	UInventoryComponent* InventoryComp = PlayerCharacter->GetInventoryComponent();
	if (!InventoryComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnPickedUp: InventoryComponent가 없습니다"));
		return false;
	}

	// 인벤토리에 자동 배치 시도
	int32 OutX, OutY;
	bool bPlaced = InventoryComp->AutoPlaceItem(ItemData, OutX, OutY);

	if (bPlaced)
	{
		UE_LOG(LogTemp, Log, TEXT("픽업 성공: %s → 인벤토리 (%d, %d)"), *ItemData->GetItemName(), OutX, OutY);

		// 픽업 성공 → Actor 파괴
		Destroy();
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("픽업 실패: %s - 인벤토리 공간 부족"), *ItemData->GetItemName());
		return false;
	}
}

FString APickupItemActor::GetPickupDisplayName() const
{
	return ItemData ? ItemData->GetItemName() : TEXT("Unknown Item");
}

void APickupItemActor::SetItemData(UBaseItemData* InItemData)
{
	ItemData = InItemData;

	// BeginPlay 이후에 호출된 경우 메시 즉시 설정
	if (HasActorBegunPlay() && ItemData)
	{
		UObject* WorldMesh = ItemData->GetWorldMesh();

		if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(WorldMesh))
		{
			StaticMeshComponent->SetStaticMesh(StaticMesh);
			StaticMeshComponent->SetVisibility(true);
			SkeletalMeshComponent->SetVisibility(false);
		}
		else if (USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(WorldMesh))
		{
			SkeletalMeshComponent->SetSkeletalMesh(SkeletalMesh);
			SkeletalMeshComponent->SetVisibility(true);
			StaticMeshComponent->SetVisibility(false);
		}
	}
}
