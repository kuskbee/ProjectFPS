// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/FPSPlayerCharacter.h"
#include "FPS/CharacterAttributeSet.h"
#include "FPS/PlayerAttributeSet.h"
#include "FPS/UI/PlayerHUD.h"
#include "FPS/UI/SkillTreeWidget.h"
#include "FPS/Components/WeaponSlotComponent.h"
#include "FPS/Components/SkillComponent.h"
#include "FPS/Weapons/FPSWeapon.h"
#include "FPS/Items/WeaponItemData.h"
#include "FPS/Interfaces/Pickupable.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Abilities/GameplayAbility.h"

AFPSPlayerCharacter::AFPSPlayerCharacter()
{
	// SkillComponent 생성
	SkillComponent = CreateDefaultSubobject<USkillComponent>(TEXT("SkillComponent"));
}

void AFPSPlayerCharacter::BeginPlay()
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

	// PlayerAttributeSet 초기화
	if (AbilitySystemComponent)
	{
		const UPlayerAttributeSet* ExistingSet = AbilitySystemComponent->GetSet<UPlayerAttributeSet>();
		if (!ExistingSet)
		{
			UPlayerAttributeSet* NewSet = NewObject<UPlayerAttributeSet>(this);
			AbilitySystemComponent->AddAttributeSetSubobject(NewSet);
			PlayerAttributeSet = NewSet;
		}
		else
		{
			PlayerAttributeSet = const_cast<UPlayerAttributeSet*>(ExistingSet);
		}

		// SkillPoint 속성 변경에 바인딩
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UPlayerAttributeSet::GetSkillPointAttribute()).AddUObject(this, &AFPSPlayerCharacter::OnSkillPointChanged);
	}

	// PlayerHUD 생성
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PlayerHUDClass)
		{
			PlayerHUDWidget = CreateWidget<UPlayerHUD>(PC, PlayerHUDClass);
			if (PlayerHUDWidget)
			{
				PlayerHUDWidget->AddToViewport();
				UE_LOG(LogTemp, Log, TEXT("PlayerHUD 위젯 생성 완료"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("PlayerHUD 생성 실패"));
			}
		}
	}
}

void AFPSPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 크로스헤어 확산 - 이동 중일 때
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

void AFPSPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Enhanced Input Component로 캐스팅
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 발사
		if (FireAction)
		{
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AFPSPlayerCharacter::FireAbilityPressed);
		}

		// 이동
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFPSPlayerCharacter::Move);
		}

		// 시점 변경
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFPSPlayerCharacter::Look);
		}

		// 점프
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		}
		
		// 무기 전환 입력
		if (SwitchWeaponAction)
		{
			EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AFPSPlayerCharacter::SwitchWeaponSlot);
		}

		if (PrimaryWeaponAction)
		{
			EnhancedInputComponent->BindAction(PrimaryWeaponAction, ETriggerEvent::Triggered, this, &AFPSPlayerCharacter::SwitchToPrimaryWeapon);
		}

		if (SecondaryWeaponAction)
		{
			EnhancedInputComponent->BindAction(SecondaryWeaponAction, ETriggerEvent::Triggered, this, &AFPSPlayerCharacter::SwitchToSecondaryWeapon);
		}

		// 픽업 입력
		if (PickupAction)
		{
			EnhancedInputComponent->BindAction(PickupAction, ETriggerEvent::Triggered, this, &AFPSPlayerCharacter::TryPickupItem);
		}

		// 리로드 입력
		if (ReloadAction)
		{
			EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AFPSPlayerCharacter::ReloadPressed);
		}

		// 스프린트 입력
		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AFPSPlayerCharacter::Sprint);
		}

		// 스킬트리 UI 토글 입력 (T키)
		if (ToggleSkillTreeAction)
		{
			EnhancedInputComponent->BindAction(ToggleSkillTreeAction, ETriggerEvent::Triggered, this, &AFPSPlayerCharacter::ToggleSkillTree);
		}

		// 액티브 스킬 사용 입력 (Q키)
		if (ActiveSkillAction)
		{
			EnhancedInputComponent->BindAction(ActiveSkillAction, ETriggerEvent::Triggered, this, &AFPSPlayerCharacter::UseActiveSkill);
		}

		// 테스트용 스킬 습득 입력 (K키)
		if (TestSkillAction)
		{
			EnhancedInputComponent->BindAction(TestSkillAction, ETriggerEvent::Triggered, this, &AFPSPlayerCharacter::TestAcquireSkill);
		}
	}
}

void AFPSPlayerCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	// HUD 업데이트
	if (PlayerHUDWidget && AttributeSet)
	{
		PlayerHUDWidget->UpdateHealthBar(Data.NewValue, AttributeSet->GetMaxHealth());
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

void AFPSPlayerCharacter::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
	// HUD 업데이트
	if (PlayerHUDWidget && AttributeSet)
	{
		PlayerHUDWidget->UpdateStaminaBar(Data.NewValue, AttributeSet->GetMaxStamina());
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

void AFPSPlayerCharacter::OnShieldChanged(const FOnAttributeChangeData& Data)
{
	// HUD 업데이트
	if (PlayerHUDWidget && AttributeSet)
	{
		PlayerHUDWidget->UpdateShieldBar(Data.NewValue, AttributeSet->GetMaxShield());
	}

	UE_LOG(LogTemp, VeryVerbose, TEXT("쉴드 변경: %.1f / %.1f"), Data.NewValue, AttributeSet ? AttributeSet->GetMaxShield() : 0.0f);
}

void AFPSPlayerCharacter::OnSkillPointChanged(const FOnAttributeChangeData& Data)
{
	// 스킬 포인트 변경 시 로그 출력 (나중에 UI 업데이트 추가)
	UE_LOG(LogTemp, Log, TEXT("SkillPoint 변경: %.0f"), Data.NewValue);

	// TODO: 스킬트리 UI가 있다면 여기서 업데이트
}

// === 플레이어 전용 입력 핸들러들 ===

void AFPSPlayerCharacter::Move(const FInputActionValue& Value)
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

void AFPSPlayerCharacter::Look(const FInputActionValue& Value)
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

void AFPSPlayerCharacter::FireAbilityPressed(const FInputActionValue& Value)
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

void AFPSPlayerCharacter::SwitchWeaponSlot(const FInputActionValue& Value)
{
	if (WeaponSlotComponent)
	{
		WeaponSlotComponent->SwitchToNextSlot();
	}
}

void AFPSPlayerCharacter::SwitchToPrimaryWeapon(const FInputActionValue& Value)
{
	if (WeaponSlotComponent)
	{
		WeaponSlotComponent->SwitchToSlotByNumber(1); // 1 = Primary
	}
}

void AFPSPlayerCharacter::SwitchToSecondaryWeapon(const FInputActionValue& Value)
{
	if (WeaponSlotComponent)
	{
		WeaponSlotComponent->SwitchToSlotByNumber(2); // 2 = Secondary
	}
}

void AFPSPlayerCharacter::TryPickupItem(const FInputActionValue& Value)
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

void AFPSPlayerCharacter::ReloadPressed(const FInputActionValue& Value)
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

void AFPSPlayerCharacter::Sprint(const FInputActionValue& Value)
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

// === IFPSWeaponHolder 인터페이스 오버라이드 (HUD 업데이트) ===

void AFPSPlayerCharacter::UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize)
{
	if (!PlayerHUDWidget)
	{
		return;
	}

	// 탄약 정보 업데이트
	PlayerHUDWidget->UpdateAmmoDisplay(CurrentAmmo, MagazineSize);

	// 현재 무기 이름도 업데이트
	if (WeaponSlotComponent)
	{
		if (AFPSWeapon* CurrentWeapon = WeaponSlotComponent->GetCurrentWeaponActor())
		{
			if (UWeaponItemData* WeaponData = CurrentWeapon->GetWeaponItemData())
			{
				PlayerHUDWidget->UpdateWeaponName(WeaponData->GetItemName());
			}
		}
		else
		{
			PlayerHUDWidget->UpdateWeaponName("");
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

		PlayerHUDWidget->UpdateWeaponSlots(PrimaryWeaponName, SecondaryWeaponName, ActiveSlotNumber);
	}

	UE_LOG(LogTemp, VeryVerbose, TEXT("WeaponHUD 업데이트: %d/%d"), CurrentAmmo, MagazineSize);
}

void AFPSPlayerCharacter::UpdateCrosshairFiringSpread(float Spread)
{
	if (PlayerHUDWidget)
	{
		PlayerHUDWidget->SetCrosshairFiringSpread(Spread);
	}
}

void AFPSPlayerCharacter::UpdateCrosshairMovementSpread(float Spread)
{
	if (PlayerHUDWidget)
	{
		PlayerHUDWidget->SetCrosshairMovementSpread(Spread);
	}
}

void AFPSPlayerCharacter::OnWeaponActivated(AFPSWeapon* Weapon)
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

	// HUD 업데이트
	if (PlayerHUDWidget && Weapon->GetWeaponItemData())
	{
		float WeaponBaseSpread = Weapon->GetWeaponItemData()->CrosshairBaseSpread;
		PlayerHUDWidget->SetBaseCrosshairSpread(WeaponBaseSpread);
	}
}

void AFPSPlayerCharacter::OnWeaponDeactivated(AFPSWeapon* Weapon)
{
	Super::OnWeaponDeactivated(Weapon);

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
	if (PlayerHUDWidget)
	{
		PlayerHUDWidget->SetBaseCrosshairSpread(0.0f);
	}
}

void AFPSPlayerCharacter::TestAcquireSkill(const FInputActionValue& Value)
{
	if (!SkillComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("TestAcquireSkill: SkillComponent가 없습니다."));
		return;
	}

	// 테스트용 스킬 ID (Blueprint DataAsset 생성 후 설정)
	FGameplayTag TestSkillTag = FGameplayTag::RequestGameplayTag(FName("Skill.Common.MaxHealth"));

	UE_LOG(LogTemp, Log, TEXT("TestAcquireSkill: 스킬 습득 시도 - %s"), *TestSkillTag.ToString());

	// 스킬 습득 시도
	ESkillAcquireResult Result = SkillComponent->TryAcquireSkill(TestSkillTag);

	// 결과 로그
	switch (Result)
	{
	case ESkillAcquireResult::Success:
		UE_LOG(LogTemp, Log, TEXT("TestAcquireSkill: 스킬 습득 성공!"));
		break;
	case ESkillAcquireResult::AlreadyAcquired:
		UE_LOG(LogTemp, Warning, TEXT("TestAcquireSkill: 이미 습득한 스킬입니다."));
		break;
	case ESkillAcquireResult::InsufficientPoints:
		UE_LOG(LogTemp, Warning, TEXT("TestAcquireSkill: 스킬 포인트가 부족합니다."));
		break;
	case ESkillAcquireResult::PrerequisiteNotMet:
		UE_LOG(LogTemp, Warning, TEXT("TestAcquireSkill: 선행 스킬이 필요합니다."));
		break;
	case ESkillAcquireResult::InvalidSkill:
		UE_LOG(LogTemp, Warning, TEXT("TestAcquireSkill: 유효하지 않은 스킬 ID입니다. SkillDataArray에 해당 스킬을 추가하세요."));
		break;
	}
}

void AFPSPlayerCharacter::UseActiveSkill(const FInputActionValue& Value)
{
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("UseActiveSkill: AbilitySystemComponent가 없음"));
		return;
	}

	// 현재 부여된 Ability 목록 출력 (디버깅)
	TArray<FGameplayAbilitySpec>& ActivatableAbilities = AbilitySystemComponent->GetActivatableAbilities();
	UE_LOG(LogTemp, Log, TEXT("=== 현재 부여된 Ability 목록 (%d개) ==="), ActivatableAbilities.Num());
	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities)
	{
		if (Spec.Ability)
		{
			// AbilityTags는 직접 접근
			const FGameplayTagContainer& SpecAbilityTags = Spec.Ability->AbilityTags;
			FString TagsString = SpecAbilityTags.ToStringSimple();
			UE_LOG(LogTemp, Log, TEXT("  - %s (Tags: %s)"), *Spec.Ability->GetName(), *TagsString);
		}
	}

	// SkillComponent에서 습득한 액티브 스킬 태그 가져오기
	if (!SkillComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("UseActiveSkill: SkillComponent가 없음"));
		return;
	}

	FGameplayTag ActiveSkillTag = SkillComponent->GetActiveSkillAbilityTag();
	if (!ActiveSkillTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UseActiveSkill: 습득한 액티브 스킬이 없음"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("찾으려는 태그: %s"), *ActiveSkillTag.ToString());

	// 액티브 스킬 활성화
	FGameplayTagContainer AbilityTags;
	AbilityTags.AddTag(ActiveSkillTag);

	bool bActivated = AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags);
	if (!bActivated)
	{
		UE_LOG(LogTemp, Warning, TEXT("UseActiveSkill: 액티브 스킬이 없거나 사용할 수 없음 (쿨다운 중?)"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("UseActiveSkill: 액티브 스킬 활성화 성공!"));
	}
}

void AFPSPlayerCharacter::ToggleSkillTree(const FInputActionValue& Value)
{
	// PlayerController 체크 (AI는 UI 사용 안함)
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	// 스킬트리 UI가 이미 열려 있으면 닫기
	if (SkillTreeWidget && SkillTreeWidget->IsInViewport())
	{
		SkillTreeWidget->RemoveFromParent();
		SkillTreeWidget = nullptr;

		// 마우스 커서 숨기기, 게임 모드로 복귀
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());

		UE_LOG(LogTemp, Log, TEXT("SkillTree UI 닫기"));
		return;
	}

	// 스킬트리 UI 열기
	if (!SkillComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("ToggleSkillTree: SkillComponent가 없습니다."));
		return;
	}

	if (!SkillTreeWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ToggleSkillTree: SkillTreeWidgetClass가 설정되지 않았습니다."));
		return;
	}

	// 위젯 생성
	SkillTreeWidget = CreateWidget<USkillTreeWidget>(PC, SkillTreeWidgetClass);
	if (SkillTreeWidget)
	{
		// SkillComponent 연동
		SkillTreeWidget->InitializeSkillTree(SkillComponent);

		// 스킬 포인트 표시
		if (PlayerAttributeSet)
		{
			SkillTreeWidget->UpdateSkillPointDisplay(PlayerAttributeSet->GetSkillPoint());
		}

		// 화면에 추가
		SkillTreeWidget->AddToViewport();

		// 마우스 커서 표시, UI 모드로 전환
		PC->SetShowMouseCursor(true);
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(SkillTreeWidget->TakeWidget());
		PC->SetInputMode(InputMode);

		UE_LOG(LogTemp, Log, TEXT("SkillTree UI 열기"));
	}
}
