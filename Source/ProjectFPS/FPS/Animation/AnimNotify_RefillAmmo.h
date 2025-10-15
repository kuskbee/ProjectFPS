// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_RefillAmmo.generated.h"

/**
 * 리로드 애니메이션 중 탄약을 보충하는 애니메이션 노티파이
 * 리로드 몽타주의 적절한 타이밍에 배치하여 현재 무기의 탄약을 최대로 채움
 */
UCLASS(const, hidecategories=Object, collapsecategories, meta=(DisplayName="Refill Ammo"))
class PROJECTFPS_API UAnimNotify_RefillAmmo : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_RefillAmmo();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

#if WITH_EDITOR
	virtual FString GetNotifyName_Implementation() const override;
#endif
};