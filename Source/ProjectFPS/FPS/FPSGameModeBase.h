// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FPSGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTFPS_API AFPSGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFPSGameModeBase();

	// 플레이어가 사망했을 때 호출 (리스폰 처리)
	UFUNCTION(BlueprintCallable)
	void PlayerDied(AController* PlayerController);
};
