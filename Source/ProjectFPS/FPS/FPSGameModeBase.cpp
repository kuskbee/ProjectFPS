// Fill out your copyright notice in the Description page of Project Settings.


#include "FPS/FPSGameModeBase.h"
#include "GameFramework/Controller.h" // For AController
#include "GameFramework/PlayerController.h" // For APlayerController

AFPSGameModeBase::AFPSGameModeBase()
{
}

void AFPSGameModeBase::PlayerDied(AController* PlayerController)
{
	if (PlayerController)
	{
		// 플레이어를 리스폰시킵니다.
		RestartPlayer(PlayerController);
	}
}

