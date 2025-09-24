// Fill out your copyright notice in the Description page of Project Settings.


#include "FPS/FPSGameModeBase.h"
#include "FPS/FPSCharacter.h"
#include "GameFramework/Controller.h" // For AController
#include "GameFramework/PlayerController.h" // For APlayerController

AFPSGameModeBase::AFPSGameModeBase()
{
}

void AFPSGameModeBase::PlayerDied(AController* PlayerController)
{
	if (PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("플레이어 사망 - 리스폰 시작"));

		// 기존 플레이어를 랜덤 PlayerStart 위치로 이동
		if (APawn* ExistingPawn = PlayerController->GetPawn())
		{
			// 랜덤 PlayerStart 찾기
			AActor* PlayerStart = ChoosePlayerStart(PlayerController);
			if (PlayerStart)
			{
				// 플레이어를 새로운 위치로 이동
				FVector NewLocation = PlayerStart->GetActorLocation();
				FRotator NewRotation = PlayerStart->GetActorRotation();
				ExistingPawn->SetActorLocationAndRotation(NewLocation, NewRotation);
				UE_LOG(LogTemp, Warning, TEXT("플레이어를 새로운 위치로 이동: %s"), *NewLocation.ToString());
			}

			// 플레이어 상태 복구
			if (AFPSCharacter* FPSChar = Cast<AFPSCharacter>(ExistingPawn))
			{
				UE_LOG(LogTemp, Warning, TEXT("기존 FPS 캐릭터 상태 복구 호출"));
				FPSChar->OnPlayerRespawn();
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("플레이어 리스폰 완료"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerController가 null입니다!"));
	}
}

