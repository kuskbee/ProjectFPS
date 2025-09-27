// Fill out your copyright notice in the Description page of Project Settings.

#include "PickupTriggerComponent.h"
#include "FPS/Interfaces/Pickupable.h"
#include "FPS/FPSCharacter.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

UPickupTriggerComponent::UPickupTriggerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;

	// 기본 설정
	InitSphereRadius(150.0f);
	SetCollisionProfileName(TEXT("Trigger"));
	SetGenerateOverlapEvents(true);
	
	// 기본값
	PickupCooldown = 0.5f;
	bShowPickupMessage = true;
	LastPickupAttemptTime = 0.0f;
}

void UPickupTriggerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Overlap 이벤트 바인딩
	OnComponentBeginOverlap.AddDynamic(this, &UPickupTriggerComponent::OnSphereBeginOverlap);

	// 소유자가 IPickupable을 구현하는지 확인
	if (!GetPickupableOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("PickupTriggerComponent: 소유자가 IPickupable 인터페이스를 구현하지 않습니다! %s"),
			GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
	}
}

void UPickupTriggerComponent::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// FPSCharacter인지 확인
	if (AFPSCharacter* Character = Cast<AFPSCharacter>(OtherActor))
	{
		// 픽업 시도
		TryPickup(Character);
	}
}

bool UPickupTriggerComponent::TryPickup(AFPSCharacter* Character)
{
	if (!Character)
	{
		return false;
	}

	// 쿨다운 체크
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastPickupAttemptTime < PickupCooldown)
	{
		return false;
	}
	LastPickupAttemptTime = CurrentTime;

	// IPickupable 인터페이스 확인
	IPickupable* PickupableOwner = GetPickupableOwner();
	if (!PickupableOwner)
	{
		UE_LOG(LogTemp, Warning, TEXT("TryPickup: 소유자가 IPickupable을 구현하지 않습니다"));
		return false;
	}

	// 픽업 가능한지 확인
	if (!PickupableOwner->CanBePickedUp(Character))
	{
		UE_LOG(LogTemp, Log, TEXT("TryPickup: 픽업 불가능 상태"));
		return false;
	}

	// 픽업 시도
	if (PickupableOwner->OnPickedUp(Character))
	{
		// 성공 메시지 표시
		if (bShowPickupMessage && GEngine)
		{
			FString DisplayName = PickupableOwner->GetPickupDisplayName();
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
				FString::Printf(TEXT("%s 픽업 완료!"), *DisplayName));
		}

		UE_LOG(LogTemp, Log, TEXT("픽업 성공: %s"), *PickupableOwner->GetPickupDisplayName());
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("픽업 실패: %s"), *PickupableOwner->GetPickupDisplayName());
		return false;
	}
}

bool UPickupTriggerComponent::CanPickup(AFPSCharacter* Character) const
{
	if (!Character)
	{
		return false;
	}

	IPickupable* PickupableOwner = GetPickupableOwner();
	if (!PickupableOwner)
	{
		return false;
	}

	return PickupableOwner->CanBePickedUp(Character);
}

void UPickupTriggerComponent::SetPickupRange(float NewRadius)
{
	SetSphereRadius(NewRadius);
	UE_LOG(LogTemp, Log, TEXT("픽업 범위 설정: %.1f"), NewRadius);
}

IPickupable* UPickupTriggerComponent::GetPickupableOwner() const
{
	if (AActor* Owner = GetOwner())
	{
		return Cast<IPickupable>(Owner);
	}
	return nullptr;
}