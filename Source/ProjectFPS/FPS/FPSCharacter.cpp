// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS/FPSCharacter.h"
#include "FPS/CharacterAttributeSet.h"
#include "FPS/Weapons/FPSWeapon.h"
#include "AbilitySystemComponent.h"
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

// Sets default values
AFPSCharacter::AFPSCharacter()
{
 	// Set this character to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;

	// Create Ability System Component
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComp"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	AttributeSet = CreateDefaultSubobject<UCharacterAttributeSet>(TEXT("AttributeSet"));

	// Create a mesh component for the 1st person view (visible only to owner)
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMesh->SetupAttachment(GetMesh()); // Attach to the 3rd person mesh
	FirstPersonMesh->SetOnlyOwnerSee(true); // Only visible to owning player
	//FirstPersonMesh->SetOwnerNoSee(false); // Not hidden from others (though it's only seen by owner anyway)
	FirstPersonMesh->SetCollisionProfileName(TEXT("NoCollision")); // No collision for 1st person mesh

	// 1인칭 카메라 생성 (이제 FirstPersonMesh에 직접 부착)
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.f, 0.f)); // 샘플 코드의 위치/회전 값 유지
	FirstPersonCameraComponent->bUsePawnControlRotation = true; // 컨트롤러 회전에 따라 카메라 회전
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// Create a mesh component for the 3rd person view (visible to others)
	// This is the default GetMesh() component
	GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), FRotator(0, -90, 0));
	GetMesh()->SetOwnerNoSee(true); // Hide 3rd person mesh from owning player
	//GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh")); // Set a proper collision profile
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	// Configure character movement
	//GetCharacterMovement()->bOrientRotationToMovement = true;
	
    // Let the controller rotation drive the character's yaw rotation.
	/*bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;*/
}

UAbilitySystemComponent* AFPSCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// Called when the game starts or when spawned
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

		// Bind to Health attribute change
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetHealthAttribute()).AddUObject(this, &AFPSCharacter::OnHealthChanged);

		if (HasAuthority() && FireAbility)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(FireAbility, 1, 0, this));
		}
	}

	//:TEST:
	GiveDefaultWeapon();
}

// Called every frame
void AFPSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent >(PlayerInputComponent))
	{
		// Fire
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AFPSCharacter::FireAbilityPressed);

		// Move
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFPSCharacter::Move);

		// Look
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFPSCharacter::Look);

		// Jump
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
	}
}

void AFPSCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
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

	// 체력이 0 이하가 되면 사망 처리
	if (Data.NewValue <= 0.0f)
	{
		OnPlayerDeath();
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
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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

void AFPSCharacter::FireAbilityPressed(const FInputActionValue& Value)
{
	const bool bPressed = Value.Get<bool>();
	if (bPressed)
	{
		// Use weapon system if we have a current weapon
		if (CurrentWeapon)
		{
			CurrentWeapon->StartFiring();
		}
		// Fallback to old direct ability activation for backward compatibility
		else if (AbilitySystemComponent && FireAbility)
		{
			AbilitySystemComponent->TryActivateAbilityByClass(FireAbility);
		}
	}
	else
	{
		// Stop firing when button is released
		if (CurrentWeapon)
		{
			CurrentWeapon->StopFiring();
		}
	}
}

void AFPSCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AFPSCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

// ========================================
// IFPSWeaponHolder Interface Implementation
// ========================================

void AFPSCharacter::AttachWeaponMeshes(AFPSWeapon* Weapon)
{
	if (!Weapon)
	{
		return;
	}

	// Attach first person weapon mesh to first person character mesh
	if (USkeletalMeshComponent* WeaponFirstPersonMesh = Weapon->GetFirstPersonMesh())
	{
		WeaponFirstPersonMesh->AttachToComponent(
			FirstPersonMesh,
			FAttachmentTransformRules::KeepRelativeTransform,
			FirstPersonWeaponSocket
		);
	}

	// Attach third person weapon mesh to third person character mesh
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

	// Play montage on first person mesh
	if (FirstPersonMesh && FirstPersonMesh->GetAnimInstance())
	{
		FirstPersonMesh->GetAnimInstance()->Montage_Play(Montage);
	}

	// Play montage on third person mesh
	if (GetMesh() && GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(Montage);
	}
}

void AFPSCharacter::AddWeaponRecoil(float Recoil)
{
	if (Controller && Recoil > 0.0f)
	{
		// Add pitch recoil to controller
		AddControllerPitchInput(-Recoil);
	}
}

void AFPSCharacter::UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize)
{
	// TODO: Update HUD/UI with ammo information
	// This would typically update a UMG widget
	UE_LOG(LogTemp, Log, TEXT("Ammo: %d/%d"), CurrentAmmo, MagazineSize);
}

FVector AFPSCharacter::GetWeaponTargetLocation()
{
	if (!FirstPersonCameraComponent)
	{
		return GetActorLocation() + GetActorForwardVector() * MaxAimDistance;
	}

	// Get camera location and rotation
	FVector CameraLocation = FirstPersonCameraComponent->GetComponentLocation();
	FVector CameraForward = FirstPersonCameraComponent->GetForwardVector();

	// Perform line trace from camera
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
	if (!WeaponClass || !GetWorld())
	{
		return;
	}

	// Spawn the weapon
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;

	AFPSWeapon* NewWeapon = GetWorld()->SpawnActor<AFPSWeapon>(WeaponClass, SpawnParams);
	if (NewWeapon)
	{
		// Add to owned weapons
		OwnedWeapons.Add(NewWeapon);

		// If no current weapon, equip this one
		if (!CurrentWeapon)
		{
			EquipWeapon(NewWeapon);
		}
	}
}

void AFPSCharacter::OnWeaponActivated(AFPSWeapon* Weapon)
{
	if (!Weapon)
	{
		return;
	}

	// Set anim instance classes if provided
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
}

void AFPSCharacter::OnWeaponDeactivated(AFPSWeapon* Weapon)
{
	// TODO: Reset anim instance classes to default if needed
	// This would require storing the original anim instance classes
}

void AFPSCharacter::OnSemiWeaponRefire()
{
	// TODO: Implement if needed for semi-auto weapon feedback
	// This could be used to show UI indicators or play sounds
}

// ========================================
// Weapon Management Functions
// ========================================

void AFPSCharacter::EquipWeapon(AFPSWeapon* Weapon)
{
	if (!Weapon)
	{
		return;
	}

	// Unequip current weapon first
	UnequipCurrentWeapon();

	// Set new current weapon
	CurrentWeapon = Weapon;

	// Activate the weapon
	CurrentWeapon->ActivateWeapon();
}

void AFPSCharacter::UnequipCurrentWeapon()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->DeactivateWeapon();
		CurrentWeapon = nullptr;
	}
}

void AFPSCharacter::GiveDefaultWeapon()
{
	// GAS 학습 포인트: 기본 무기 지급
	// AddWeaponClass는 IFPSWeaponHolder 인터페이스 함수
	// 무기를 스폰하고 자동으로 장착까지 처리
	if (DefaultWeaponClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("기본 무기 지급: %s"), *DefaultWeaponClass->GetName());
		AddWeaponClass(DefaultWeaponClass);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("기본 무기 클래스가 설정되지 않음!"));
	}
}