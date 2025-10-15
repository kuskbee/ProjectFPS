// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/Animation/AnimNotify_RefillAmmo.h"
#include "FPS/Weapons/FPSWeapon.h"
#include "FPS/Weapons/FPSWeaponHolder.h"
#include "FPS/Components/WeaponSlotComponent.h"
#include "FPS/Items/WeaponItemData.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UAnimNotify_RefillAmmo::UAnimNotify_RefillAmmo()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 200, 100, 255); // 주황색으로 표시
#endif
}

void UAnimNotify_RefillAmmo::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimNotify_RefillAmmo: MeshComp가 null입니다"));
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimNotify_RefillAmmo: Owner가 null입니다"));
		return;
	}

	// WeaponHolder 인터페이스를 구현한 액터인지 확인
	if (IFPSWeaponHolder* WeaponHolder = Cast<IFPSWeaponHolder>(Owner))
	{
		// WeaponSlotComponent를 통해 현재 활성화된 무기 가져오기
		if (UWeaponSlotComponent* WeaponSlotComp = Owner->FindComponentByClass<UWeaponSlotComponent>())
		{
			UWeaponItemData* ActiveWeaponItem = WeaponSlotComp->GetActiveWeaponItem();
			if (ActiveWeaponItem)
			{
				// 탄약 보충
				ActiveWeaponItem->RefillAmmo();

				UE_LOG(LogTemp, Log, TEXT("AnimNotify_RefillAmmo: %s 무기 탄약 보충 완료 (%d/%d)"),
					*ActiveWeaponItem->GetItemName(),
					ActiveWeaponItem->CurrentAmmo,
					ActiveWeaponItem->MagazineSize);

				// HUD 업데이트
				WeaponHolder->UpdateWeaponHUD(ActiveWeaponItem->CurrentAmmo, ActiveWeaponItem->MagazineSize);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("AnimNotify_RefillAmmo: 활성화된 무기가 없습니다"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("AnimNotify_RefillAmmo: WeaponSlotComponent를 찾을 수 없습니다"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimNotify_RefillAmmo: Owner가 IFPSWeaponHolder 인터페이스를 구현하지 않습니다"));
	}
}

#if WITH_EDITOR
FString UAnimNotify_RefillAmmo::GetNotifyName_Implementation() const
{
	return TEXT("Refill Ammo");
}
#endif