// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Pickupable.generated.h"

class AFPSCharacter;

UINTERFACE(MinimalAPI)
class UPickupable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 플레이어가 픽업할 수 있는 모든 객체가 구현해야 하는 인터페이스
 */
class PROJECTFPS_API IPickupable
{
	GENERATED_BODY()

public:
	/** 이 객체를 픽업할 수 있는지 확인 */
	virtual bool CanBePickedUp(AFPSCharacter* Character) = 0;

	/** 픽업 시도 시 호출 (성공 시 true 반환) */
	virtual bool OnPickedUp(AFPSCharacter* Character) = 0;

	/** 픽업 가능할 때 표시할 이름 */
	virtual FString GetPickupDisplayName() const = 0;
};