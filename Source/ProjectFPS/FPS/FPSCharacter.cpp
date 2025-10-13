// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/FPSCharacter.h"
#include "FPS/CharacterAttributeSet.h"
#include "FPS/GameplayEffect_Heal.h"
#include "FPS/Weapons/FPSWeapon.h"
#include "FPS/Components/WeaponSlotComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/GameModeBase.h" // AGameModeBase를 위해 추가
#include "FPS/FPSGameModeBase.h" // 우리 게임모드를 위해 추가
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "FPS/Items/WeaponItemData.h"
#include "Blueprint/UserWidget.h"
#include "UI/DamageNumberWidget.h"

// 기본값 설정
AFPSCharacter::AFPSCharacter()
{
 	// 이 캐릭터가 매 프레임 Tick()을 호출하도록 설정
	PrimaryActorTick.bCanEverTick = true;

	// Ability System Component 생성
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComp"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	AttributeSet = CreateDefaultSubobject<UCharacterAttributeSet>(TEXT("AttributeSet"));

	// 무기 슬롯 컴포넌트 생성
	WeaponSlotComponent = CreateDefaultSubobject<UWeaponSlotComponent>(TEXT("WeaponSlotComponent"));

	// 1인칭 시점용 메시 컴포넌트 생성 (소유자에게만 보임)
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMesh->SetupAttachment(GetMesh()); // 3인칭 메시에 부착
	FirstPersonMesh->SetOnlyOwnerSee(true); // 소유 플레이어에게만 보임
	//FirstPersonMesh->SetOwnerNoSee(false); // Not hidden from others (though it's only seen by owner anyway)
	FirstPersonMesh->SetCollisionProfileName(TEXT("NoCollision")); // 1인칭 메시는 충돌 없음

	// 1인칭 카메라 생성 (이제 FirstPersonMesh에 직접 부착)
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.f, 0.f)); // 샘플 코드의 위치/회전 값 유지
	FirstPersonCameraComponent->bUsePawnControlRotation = true; // 컨트롤러 회전에 따라 카메라 회전
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// 3인칭 시점용 메시 컴포넌트 생성 (타인에게 보임)
	// 기본 GetMesh() 컴포넌트임
	GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), FRotator(0, -90, 0));
	GetMesh()->SetOwnerNoSee(true); // 소유 플레이어로부터 3인칭 메시 숨김
	//GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh")); // Set a proper collision profile
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	// 캐릭터 이동 설정
	//GetCharacterMovement()->bOrientRotationToMovement = true;

    // 컨트롤러 회전이 캐릭터의 yaw 회전을 조작하도록 설정
	/*bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;*/

	// 기본 GameplayEffect 클래스 설정
	HealEffect = UGameplayEffect_Heal::StaticClass();
}

UAbilitySystemComponent* AFPSCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// 게임 시작 또는 스폰될 때 호출
void AFPSCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 기본 AnimInstance 클래스들 저장 (무기 비활성화 시 복원용)
	if (FirstPersonMesh)
	{
		DefaultFirstPersonAnimClass = FirstPersonMesh->GetAnimClass();
	}
	if (GetMesh())
	{
		DefaultThirdPersonAnimClass = GetMesh()->GetAnimClass();
	}

	// DamageNumberWidget 클래스 설정 (CharacterAttributeSet의 static 변수)
	if (DamageNumberWidgetClass)
	{
		UCharacterAttributeSet::DamageNumberWidgetClass = DamageNumberWidgetClass;
	}

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
			
		// Health 속성 변경에 바인딩
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetHealthAttribute()).AddUObject(this, &AFPSCharacter::OnHealthChanged);

		// Stamina 속성 변경에 바인딩
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetStaminaAttribute()).AddUObject(this, &AFPSCharacter::OnStaminaChanged);

		// Shield 속성 변경에 바인딩
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetShieldAttribute()).AddUObject(this, &AFPSCharacter::OnShieldChanged);

		// 기본 어빌리티들 부여
		if (HasAuthority())
		{
			for (const auto& AbilityClass : DefaultAbilities)
			{
				if (AbilityClass)
				{
					FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, this);
					AbilitySystemComponent->GiveAbility(AbilitySpec);
					UE_LOG(LogTemp, Log, TEXT("어빌리티 부여: %s"), *AbilityClass->GetName());
				}
			}
		}
	}
}

// 매 프레임 호출
void AFPSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 이동 시 크로스헤어 확산 업데이트
	if (WeaponSlotComponent)
	{
		AFPSWeapon* CurrentWeapon = WeaponSlotComponent->GetCurrentWeaponActor();
		if (CurrentWeapon && CurrentWeapon->GetWeaponItemData())
		{
			// 캐릭터 이동 속도 가져오기
			FVector Velocity = GetVelocity();
			float Speed = Velocity.Size2D();

			// 이동 속도에 비례한 확산 (0 ~ 최대 10)
			float MovementSpread = FMath::Clamp(Speed / 100.0f, 0.0f, 10.0f);

			// HUD에 이동 확산 설정
			UpdateCrosshairMovementSpread(MovementSpread);
		}
	}
}

void AFPSCharacter::ServerNotifyPlayerDeath_Implementation()
{
	// 서버에서만 실행됨
	if (HasAuthority())
	{
		if (AGameModeBase* GameMode = GetWorld()->GetAuthGameMode())
		{
			if (AFPSGameModeBase* FPSGameMode = Cast<AFPSGameModeBase>(GameMode))
			{
				FPSGameMode->PlayerDied(GetController());
			}
		}
	}
}

void AFPSCharacter::OnPlayerDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("캐릭터 사망!"));

	// 사망 상태 설정 (발사체 Blueprint에서 체크 가능)
	bIsAlive = false;

	// 입력 비활성화
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}
	
	// 움직임 정지
	GetCharacterMovement()->StopMovementImmediately();

	// 메시 숨기기
	GetMesh()->SetVisibility(false);
	if (FirstPersonMesh)
	{
		FirstPersonMesh->SetVisibility(false);
	}

	// 캡슐 컴포넌트 충돌 비활성화
	//GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 플레이어 캐릭터인 경우 GameMode에 사망 알림
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		ServerNotifyPlayerDeath();
	}
	else // AI 또는 조종되지 않는 캐릭터인 경우
	{
		Destroy(); // 즉시 파괴
	}
}

void AFPSCharacter::OnPlayerRespawn()
{
	UE_LOG(LogTemp, Warning, TEXT("플레이어 리스폰 상태 복구 시작"));

	// 먼저 체력부터 회복 (bIsAlive가 false인 상태에서)
	if (AbilitySystemComponent && AttributeSet)
	{
		// 체력 회복 전 상태 로깅
		float CurrentHealth = AttributeSet->GetHealth();
		float MaxHealthValue = AttributeSet->GetMaxHealth();
		UE_LOG(LogTemp, Warning, TEXT("체력 회복 시작: 현재=%f, 최대=%f, bIsAlive=%s"),
			CurrentHealth, MaxHealthValue, bIsAlive ? TEXT("true") : TEXT("false"));

		// GameplayEffect_Heal을 사용하여 체력 회복
		if (HealEffect && MaxHealthValue > 0.0f)
		{
			// GameplayEffectSpec을 생성하여 체력 회복
			FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(HealEffect, 1.0f, ContextHandle);

			if (SpecHandle.IsValid())
			{
				// GameplayEffect를 적용하여 Health를 MaxHealth로 설정
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				UE_LOG(LogTemp, Warning, TEXT("GameplayEffect_Heal 적용: %f -> MaxHealth"), CurrentHealth);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("HealEffect가 없거나 MaxHealth가 0 이하입니다. HealEffect=%s, MaxHealth=%f"),
				HealEffect ? TEXT("유효함") : TEXT("없음"), MaxHealthValue);
		}
	}

	// 체력 회복 후에 생존 상태 복구 (이제 안전함)
	bIsAlive = true;
	UE_LOG(LogTemp, Warning, TEXT("bIsAlive = true로 설정"));

	// 입력 재활성화
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		EnableInput(PC);
		UE_LOG(LogTemp, Warning, TEXT("입력 재활성화 완료"));
	}

	// 움직임 재활성화
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	UE_LOG(LogTemp, Warning, TEXT("움직임 재활성화 완료"));

	// 메시 다시 표시
	GetMesh()->SetVisibility(true);
	if (FirstPersonMesh)
	{
		FirstPersonMesh->SetVisibility(true);
	}
	UE_LOG(LogTemp, Warning, TEXT("메시 표시 완료"));

	UE_LOG(LogTemp, Warning, TEXT("플레이어 리스폰 상태 복구 완료"));
}

// ========================================
// IFPSWeaponHolder 인터페이스 구현
// ========================================

void AFPSCharacter::AttachWeaponMeshes(AFPSWeapon* Weapon)
{
	if (!Weapon)
	{
		return;
	}

	// 1인칭 무기 메시를 1인칭 캐릭터 메시에 부착
	if (USkeletalMeshComponent* WeaponFirstPersonMesh = Weapon->GetFirstPersonMesh())
	{
		WeaponFirstPersonMesh->AttachToComponent(
			FirstPersonMesh,
			FAttachmentTransformRules::KeepRelativeTransform,
			FirstPersonWeaponSocket
		);
	}

	// 3인칭 무기 메시를 3인칭 캐릭터 메시에 부착
	if (USkeletalMeshComponent* WeaponThirdPersonMesh = Weapon->GetThirdPersonMesh())
	{
		WeaponThirdPersonMesh->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::KeepRelativeTransform,
			ThirdPersonWeaponSocket
		);
	}
}

void AFPSCharacter::PlayFiringMontage(UAnimMontage* Montage)
{
	if (!Montage)
	{
		return;
	}

	// 1인칭 메시에서 몽타주 재생
	if (FirstPersonMesh && FirstPersonMesh->GetAnimInstance())
	{
		FirstPersonMesh->GetAnimInstance()->Montage_Play(Montage);
	}

	// 3인칭 메시에서 몽타주 재생
	if (GetMesh() && GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(Montage);
	}
}

void AFPSCharacter::PlayReloadMontage(UAnimMontage* Montage)
{
	if (!Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("AFPSCharacter::PlayReloadMontage: 몽타주가 null입니다"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("AFPSCharacter::PlayReloadMontage: 리로드 몽타주 재생 시작 - %s"), *Montage->GetName());

	// 1인칭 메시에서 몽타주 재생
	if (FirstPersonMesh && FirstPersonMesh->GetAnimInstance())
	{
		FirstPersonMesh->GetAnimInstance()->Montage_Play(Montage);
		UE_LOG(LogTemp, Warning, TEXT("AFPSCharacter::PlayReloadMontage: FirstPersonMesh에서 재생"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AFPSCharacter::PlayReloadMontage: FirstPersonMesh 또는 AnimInstance가 null"));
	}

	// 3인칭 메시에서 몽타주 재생 (멀티플레이어용)
	if (GetMesh() && GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(Montage);
		UE_LOG(LogTemp, Warning, TEXT("AFPSCharacter::PlayReloadMontage: ThirdPersonMesh에서 재생"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AFPSCharacter::PlayReloadMontage: ThirdPersonMesh 또는 AnimInstance가 null"));
	}
}

void AFPSCharacter::AddWeaponRecoil(float Recoil)
{
	if (Controller && Recoil > 0.0f)
	{
		// 컨트롤러에 pitch 반동 추가
		AddControllerPitchInput(-Recoil);
	}
}

void AFPSCharacter::UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize)
{
	//
}

FVector AFPSCharacter::GetWeaponTargetLocation()
{
	if (!FirstPersonCameraComponent)
	{
		return GetActorLocation() + GetActorForwardVector() * MaxAimDistance;
	}

	// 카메라 위치와 회전 가져오기
	FVector CameraLocation = FirstPersonCameraComponent->GetComponentLocation();
	FVector CameraForward = FirstPersonCameraComponent->GetForwardVector();

	// 카메라에서 라인 트레이스 수행
	FVector TraceEnd = CameraLocation + (CameraForward * MaxAimDistance);

	FHitResult HitResult;
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);
	TraceParams.bTraceComplex = true;

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		CameraLocation,
		TraceEnd,
		ECollisionChannel::ECC_Visibility,
		TraceParams
	);

	if (bHit)
	{
		return HitResult.Location;
	}

	return TraceEnd;
}

void AFPSCharacter::AddWeaponClass(const TSubclassOf<AFPSWeapon>& WeaponClass)
{
	// TODO: 나중에 WeaponItemData 시스템으로 교체
	// 임시로 레거시 시스템 유지
	if (!WeaponClass || !GetWorld())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("AddWeaponClass: WeaponSlotComponent 시스템으로 교체 필요"));
}