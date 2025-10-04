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
#include "FPS/Interfaces/Pickupable.h"
#include "FPS/UI/PlayerHUD.h"
#include "FPS/Items/WeaponItemData.h"
#include "Blueprint/UserWidget.h"

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

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
			if (MouseLookMappingContext)
			{
				Subsystem->AddMappingContext(MouseLookMappingContext, 1);
			}
		}
	}

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		// 기본 AnimInstance 클래스들 저장 (무기 비활성화 시 복원용)
		if (FirstPersonMesh)
		{
			DefaultFirstPersonAnimClass = FirstPersonMesh->GetAnimClass();
		}
		if (GetMesh())
		{
			DefaultThirdPersonAnimClass = GetMesh()->GetAnimClass();
		}

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

	// WeaponHUD 생성 (플레이어 캐릭터인 경우에만)
	if (IsPlayerControlled() && WeaponHUDClass)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			WeaponHUDWidget = CreateWidget<UPlayerHUD>(PC, WeaponHUDClass);
			if (WeaponHUDWidget)
			{
				WeaponHUDWidget->AddToViewport();
				UE_LOG(LogTemp, Log, TEXT("WeaponHUD 생성 및 화면에 추가 완료"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("WeaponHUD 생성 실패"));
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

// 입력에 기능을 바인딩하기 위해 호출
void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent >(PlayerInputComponent))
	{
		// 발사
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AFPSCharacter::FireAbilityPressed);

		// 이동
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFPSCharacter::Move);

		// 시점 변경
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFPSCharacter::Look);

		// 점프
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);

		// 무기 전환
		if (SwitchWeaponAction)
		{
			EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AFPSCharacter::SwitchWeaponSlot);
		}

		// 1번 무기 (Primary)
		if (PrimaryWeaponAction)
		{
			EnhancedInputComponent->BindAction(PrimaryWeaponAction, ETriggerEvent::Triggered, this, &AFPSCharacter::SwitchToPrimaryWeapon);
		}

		// 2번 무기 (Secondary)
		if (SecondaryWeaponAction)
		{
			EnhancedInputComponent->BindAction(SecondaryWeaponAction, ETriggerEvent::Triggered, this, &AFPSCharacter::SwitchToSecondaryWeapon);
		}

		// 아이템 픽업 (E키)
		if (PickupAction)
		{
			EnhancedInputComponent->BindAction(PickupAction, ETriggerEvent::Triggered, this, &AFPSCharacter::TryPickupItem);
		}

		// 리로드 (R키)
		if (ReloadAction)
		{
			EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AFPSCharacter::ReloadPressed);
		}

		// Shift 질주
		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AFPSCharacter::Sprint);
		}
	}
}

void AFPSCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	// HUD 업데이트
	if (WeaponHUDWidget && AttributeSet)
	{
		WeaponHUDWidget->UpdateHealthBar(Data.NewValue, AttributeSet->GetMaxHealth());
	}

	// 디버그 메시지 출력
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.f,
			FColor::Red,
			FString::Printf(TEXT("현재 체력: %f"), Data.NewValue)
		);
	}

	// 체력이 0 이하가 되면 사망 처리 (단, 이미 죽은 상태가 아닐 때만)
	if (Data.NewValue <= 0.0f && bIsAlive)
	{
		UE_LOG(LogTemp, Warning, TEXT("체력 0 이하 - 사망 처리 시작 (bIsAlive: %s)"), bIsAlive ? TEXT("true") : TEXT("false"));
		OnPlayerDeath();
	}
	else if (Data.NewValue <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("체력 0 이하이지만 이미 죽은 상태 - 사망 처리 건너뜀"));
	}
}

void AFPSCharacter::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
	// HUD 업데이트
	if (WeaponHUDWidget && AttributeSet)
	{
		WeaponHUDWidget->UpdateStaminaBar(Data.NewValue, AttributeSet->GetMaxStamina());
	}

	UE_LOG(LogTemp, VeryVerbose, TEXT("스태미나 변경: %.1f / %.1f"), Data.NewValue, AttributeSet ? AttributeSet->GetMaxStamina() : 0.0f);

	// 스태미나가 0 이하가 되면 Sprint Ability 자동 종료
	if (Data.NewValue <= 0.0f && AbilitySystemComponent)
	{
		FGameplayTag SprintTag = FGameplayTag::RequestGameplayTag(FName("Ability.Sprint"));
		TArray<FGameplayAbilitySpec*> ActiveAbilities;
		AbilitySystemComponent->GetActivatableGameplayAbilitySpecsByAllMatchingTags(
			FGameplayTagContainer(SprintTag),
			ActiveAbilities
		);

		for (FGameplayAbilitySpec* Spec : ActiveAbilities)
		{
			if (Spec && Spec->IsActive())
			{
				AbilitySystemComponent->CancelAbilityHandle(Spec->Handle);
				UE_LOG(LogTemp, Warning, TEXT("스태미나 고갈: Sprint Ability 자동 종료"));
			}
		}
	}
}

void AFPSCharacter::OnShieldChanged(const FOnAttributeChangeData& Data)
{
	// HUD 업데이트
	if (WeaponHUDWidget && AttributeSet)
	{
		WeaponHUDWidget->UpdateShieldBar(Data.NewValue, AttributeSet->GetMaxShield());
	}

	UE_LOG(LogTemp, VeryVerbose, TEXT("쉴드 변경: %.1f / %.1f"), Data.NewValue, AttributeSet ? AttributeSet->GetMaxShield() : 0.0f);
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

void AFPSCharacter::FireAbilityPressed(const FInputActionValue& Value)
{
	const bool bPressed = Value.Get<bool>();
	if (bPressed)
	{
		// 새로운 WeaponSlotComponent 시스템 사용
		if (WeaponSlotComponent)
		{
			if (AFPSWeapon* CurrentWeapon = WeaponSlotComponent->GetCurrentWeaponActor())
			{
				CurrentWeapon->StartFiring();
			}
		}
		// 하위 호환성을 위해 태그 기반 어빌리티 활성화로 폴백
		else if (AbilitySystemComponent)
		{
			bool bSuccess = AbilitySystemComponent->TryActivateAbilitiesByTag(
				FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Ability.Fire")))
			);

			if (!bSuccess)
			{
				UE_LOG(LogTemp, Warning, TEXT("FireAbilityPressed: 발사 어빌리티 활성화 실패"));
			}
		}
	}
	else
	{
		// 버튼을 떼면 발사 중지
		if (WeaponSlotComponent)
		{
			if (AFPSWeapon* CurrentWeapon = WeaponSlotComponent->GetCurrentWeaponActor())
			{
				CurrentWeapon->StopFiring();
			}
		}
	}
}

void AFPSCharacter::Move(const FInputActionValue& Value)
{
	// 입력은 Vector2D 형태
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// 이동 입력 추가
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AFPSCharacter::Look(const FInputActionValue& Value)
{
	// 입력은 Vector2D 형태
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// 컨트롤러에 yaw 및 pitch 입력 추가
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
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
	if (!WeaponHUDWidget)
	{
		return;
	}

	// 탄약 정보 업데이트
	WeaponHUDWidget->UpdateAmmoDisplay(CurrentAmmo, MagazineSize);

	// 현재 무기 이름도 업데이트
	if (WeaponSlotComponent)
	{
		if (AFPSWeapon* CurrentWeapon = WeaponSlotComponent->GetCurrentWeaponActor())
		{
			if (UWeaponItemData* WeaponData = CurrentWeapon->GetWeaponItemData())
			{
				WeaponHUDWidget->UpdateWeaponName(WeaponData->GetItemName());
			}
		}
		else
		{
			WeaponHUDWidget->UpdateWeaponName("");
		}

		// 슬롯 상태도 업데이트
		FString PrimaryWeaponName = "";
		FString SecondaryWeaponName = "";

		if (UWeaponItemData* PrimaryData = WeaponSlotComponent->GetWeaponInSlot(EWeaponSlot::Primary))
		{
			PrimaryWeaponName = PrimaryData->GetItemName();
		}

		if (UWeaponItemData* SecondaryData = WeaponSlotComponent->GetWeaponInSlot(EWeaponSlot::Secondary))
		{
			SecondaryWeaponName = SecondaryData->GetItemName();
		}

		int32 ActiveSlotNumber = 0;
		EWeaponSlot CurrentSlot = WeaponSlotComponent->GetActiveSlot();
		if (CurrentSlot == EWeaponSlot::Primary) ActiveSlotNumber = 1;
		else if (CurrentSlot == EWeaponSlot::Secondary) ActiveSlotNumber = 2;

		WeaponHUDWidget->UpdateWeaponSlots(PrimaryWeaponName, SecondaryWeaponName, ActiveSlotNumber);
	}

	UE_LOG(LogTemp, VeryVerbose, TEXT("WeaponHUD 업데이트: %d/%d"), CurrentAmmo, MagazineSize);
}

void AFPSCharacter::UpdateCrosshairFiringSpread(float Spread)
{
	if (!WeaponHUDWidget)
	{
		return;
	}

	// HUD에 발사 확산 설정
	WeaponHUDWidget->SetCrosshairFiringSpread(Spread);
}

void AFPSCharacter::UpdateCrosshairMovementSpread(float Spread)
{
	if (!WeaponHUDWidget)
	{
		return;
	}

	// HUD에 이동 확산 설정
	WeaponHUDWidget->SetCrosshairMovementSpread(Spread);
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

void AFPSCharacter::OnWeaponActivated(AFPSWeapon* Weapon)
{
	if (!Weapon)
	{
		return;
	}

	// 제공된 애님 인스턴스 클래스 설정
	if (TSubclassOf<UAnimInstance> FPAnimClass = Weapon->GetFirstPersonAnimInstanceClass())
	{
		if (FirstPersonMesh)
		{
			FirstPersonMesh->SetAnimInstanceClass(FPAnimClass);
		}
	}

	if (TSubclassOf<UAnimInstance> TPAnimClass = Weapon->GetThirdPersonAnimInstanceClass())
	{
		if (GetMesh())
		{
			GetMesh()->SetAnimInstanceClass(TPAnimClass);
		}
	}

	// 무기의 BaseCrosshairSpread 설정
	if (WeaponHUDWidget && Weapon->GetWeaponItemData())
	{
		float WeaponBaseSpread = Weapon->GetWeaponItemData()->CrosshairBaseSpread;
		WeaponHUDWidget->SetBaseCrosshairSpread(WeaponBaseSpread);
	}
}

void AFPSCharacter::OnWeaponDeactivated(AFPSWeapon* Weapon)
{
	if (WeaponSlotComponent)
	{
		if (AFPSWeapon* CurrentWeapon = WeaponSlotComponent->GetCurrentWeaponActor())
		{
			CurrentWeapon->StopFiring();
		}
	}

	// 무기 관련 모든 활성화된 어빌리티 취소
	if (AbilitySystemComponent)
	{
		// 발사 및 리로드 어빌리티 강제 취소
		FGameplayTagContainer AbilitiesToCancel;
		AbilitiesToCancel.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Fire")));
		AbilitiesToCancel.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Reload")));

		AbilitySystemComponent->CancelAbilities(&AbilitiesToCancel);

		UE_LOG(LogTemp, Warning, TEXT("OnWeaponDeactivated: 무기 관련 어빌리티들 취소"));
	}

	// 기본 애님 인스턴스 클래스로 복원
	if (DefaultFirstPersonAnimClass && FirstPersonMesh)
	{
		FirstPersonMesh->SetAnimInstanceClass(DefaultFirstPersonAnimClass);
		UE_LOG(LogTemp, Log, TEXT("1인칭 애니메이션을 기본값으로 복원"));
	}

	if (DefaultThirdPersonAnimClass && GetMesh())
	{
		GetMesh()->SetAnimInstanceClass(DefaultThirdPersonAnimClass);
		UE_LOG(LogTemp, Log, TEXT("3인칭 애니메이션을 기본값으로 복원"));
	}

	// 무기 해제 시 BaseCrosshairSpread를 0으로 초기화
	if (WeaponHUDWidget)
	{
		WeaponHUDWidget->SetBaseCrosshairSpread(0.0f);
	}
}

void AFPSCharacter::OnSemiWeaponRefire()
{
	// TODO: 반자동 무기 피드백에 필요하면 구현
	// UI 표시기를 보여주거나 사운드를 재생하는데 사용할 수 있음
}

// ========================================
// 무기 슬롯 입력 처리 함수들
// ========================================

void AFPSCharacter::SwitchWeaponSlot(const FInputActionValue& Value)
{
	if (WeaponSlotComponent)
	{
		WeaponSlotComponent->SwitchToNextSlot();
	}
}

void AFPSCharacter::SwitchToPrimaryWeapon(const FInputActionValue& Value)
{
	if (WeaponSlotComponent)
	{
		WeaponSlotComponent->SwitchToSlotByNumber(1); // 1 = Primary
	}
}

void AFPSCharacter::SwitchToSecondaryWeapon(const FInputActionValue& Value)
{
	if (WeaponSlotComponent)
	{
		WeaponSlotComponent->SwitchToSlotByNumber(2); // 2 = Secondary
	}
}

void AFPSCharacter::GiveDefaultWeapon()
{
	//
	UE_LOG(LogTemp, Warning, TEXT("GiveDefaultWeapon() - 테스트"));
}

void AFPSCharacter::TryPickupItem(const FInputActionValue& Value)
{
	// E키가 눌렸는지 확인
	const bool bPressed = Value.Get<bool>();
	if (!bPressed)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("E키 픽업 시도"));

	// 플레이어 위치에서 SphereTrace 수행
	FVector PlayerLocation = GetActorLocation();
	float PickupRange = 200.0f; // 픽업 범위

	 // 오브젝트 타입 필터
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

	// 충돌 무시 액터 설정
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	// 오버랩 결과
    TArray<AActor*> OutActors;
    const bool bAny = UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        PlayerLocation,
        PickupRange,
        ObjectTypes,
        /*ClassFilter=*/AActor::StaticClass(),
        ActorsToIgnore,
        OutActors
    );

    if (!bAny)
    {
        UE_LOG(LogTemp, Log, TEXT("주변에 픽업 가능한 아이템이 없습니다"));
        return;
    }

	// 가장 가까운 Pickupable 아이템 고르기 (+ 상태 체크)
	IPickupable* ClosestPickupable = nullptr;
	float ClosestDistance = FLT_MAX;

	for (AActor* OverlapActor : OutActors)
	{
		if (OverlapActor)
		{
			if (IPickupable* Pickupable = Cast<IPickupable>(OverlapActor))
			{
				// 드롭된 상태인 아이템만 고려
				if (Pickupable->IsDropped())
				{
					float Distance = FVector::Dist(PlayerLocation, OverlapActor->GetActorLocation());
					if (Distance < ClosestDistance)
					{
						ClosestDistance = Distance;
						ClosestPickupable = Pickupable;
					}
				}
			}
		}
	}
	
	// 가장 가까운 아이템 픽업 시도
	if (ClosestPickupable)
	{
		UE_LOG(LogTemp, Log, TEXT("가장 가까운 아이템 픽업 시도: %s"), *ClosestPickupable->GetPickupDisplayName());
		ClosestPickupable->OnPickedUp(this);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("픽업 가능한 아이템을 찾을 수 없습니다"));
	}
}

void AFPSCharacter::ReloadPressed(const FInputActionValue& Value)
{
	// R키가 눌렸는지 확인
	const bool bPressed = Value.Get<bool>();
	if (!bPressed)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("R키 리로드 시도"));

	// GameplayAbility_Reload 활성화
	if (AbilitySystemComponent)
	{
		// 리로드 어빌리티 클래스로 어빌리티 활성화 시도
		// 어빌리티 클래스는 Blueprint에서 설정해야 함
		bool bSuccess = AbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Ability.Reload"))));

		if (!bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("ReloadPressed: 리로드 어빌리티 활성화 실패 (어빌리티가 없거나 조건 불충족)"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ReloadPressed: AbilitySystemComponent가 null입니다"));
	}
}

void AFPSCharacter::Sprint(const FInputActionValue& Value)
{
	// Shift 키 상태 확인 (true = 눌림, false = 뗌)
	const bool bIsSprinting = Value.Get<bool>();

	if (!AbilitySystemComponent)
	{
		return;
	}

	if (bIsSprinting)
	{
		// Shift 눌림: Sprint Ability 활성화
		UE_LOG(LogTemp, Log, TEXT("Shift 키 눌림: Sprint 시작 시도"));
		bool bSuccess = AbilitySystemComponent->TryActivateAbilitiesByTag(
			FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Ability.Sprint")))
		);

		if (!bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("Sprint: 어빌리티 활성화 실패 (스태미나 부족 또는 어빌리티 없음)"));
		}
	}
	else
	{
		// Shift 뗌: Sprint Ability 종료
		UE_LOG(LogTemp, Log, TEXT("Shift 키 뗌: Sprint 종료 시도"));

		// 활성화된 Sprint Ability 찾아서 종료
		FGameplayTag SprintTag = FGameplayTag::RequestGameplayTag(FName("Ability.Sprint"));
		TArray<FGameplayAbilitySpec*> ActiveAbilities;
		AbilitySystemComponent->GetActivatableGameplayAbilitySpecsByAllMatchingTags(
			FGameplayTagContainer(SprintTag),
			ActiveAbilities
		);

		for (FGameplayAbilitySpec* Spec : ActiveAbilities)
		{
			if (Spec && Spec->IsActive())
			{
				AbilitySystemComponent->CancelAbilityHandle(Spec->Handle);
				UE_LOG(LogTemp, Log, TEXT("Sprint Ability 종료됨"));
			}
		}
	}
}