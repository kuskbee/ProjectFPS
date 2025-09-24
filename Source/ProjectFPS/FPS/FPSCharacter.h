// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "InputActionValue.h"
#include "GameplayEffectTypes.h" // Added for FOnAttributeChangeData
#include "FPS/Weapons/FPSWeaponHolder.h"
#include "FPSCharacter.generated.h"

class USkeletalMeshComponent;
class UAbilitySystemComponent;
class UCharacterAttributeSet;
class UGameplayAbility;
class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;
class AFPSWeapon;
class UAnimMontage;
class UGameplayEffect;

UCLASS()
class PROJECTFPS_API AFPSCharacter : public ACharacter, public IAbilitySystemInterface, public IFPSWeaponHolder
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFPSCharacter();
	
	// IAbilitySystemInterface를 구현하기 위해 필요한 함수
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when Health attribute is changed
	virtual void OnHealthChanged(const FOnAttributeChangeData& Data);

	UFUNCTION(Server, Reliable)
	void ServerNotifyPlayerDeath();
	void ServerNotifyPlayerDeath_Implementation();

	// 플레이어 사망 처리
	virtual void OnPlayerDeath();

public:
	// 플레이어 리스폰 처리
	UFUNCTION(BlueprintCallable, Category="Player")
	virtual void OnPlayerRespawn();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Input Action handler functions
	void FireAbilityPressed(const FInputActionValue& Value);
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	// IFPSWeaponHolder interface implementation
	virtual void AttachWeaponMeshes(AFPSWeapon* Weapon) override;
	virtual void PlayFiringMontage(UAnimMontage* Montage) override;
	virtual void AddWeaponRecoil(float Recoil) override;
	virtual void UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize) override;
	virtual FVector GetWeaponTargetLocation() override;
	virtual void AddWeaponClass(const TSubclassOf<AFPSWeapon>& WeaponClass) override;
	virtual void OnWeaponActivated(AFPSWeapon* Weapon) override;
	virtual void OnWeaponDeactivated(AFPSWeapon* Weapon) override;
	virtual void OnSemiWeaponRefire() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UCharacterAttributeSet> AttributeSet;

	// First person camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

	FTimerHandle RespawnTimerHandle;

	// 플레이어 생존 상태 (Blueprint에서 발사체 충돌 체크용)
	UPROPERTY(BlueprintReadOnly, Category="Player State")
	bool bIsAlive = true;

	// Weapon system
	/** List of weapons owned by this character */
	UPROPERTY(BlueprintReadOnly, Category="Weapons")
	TArray<AFPSWeapon*> OwnedWeapons;

	/** Currently equipped weapon */
	UPROPERTY(BlueprintReadOnly, Category="Weapons")
	TObjectPtr<AFPSWeapon> CurrentWeapon;

	/** Name of the first person mesh weapon socket */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapons")
	FName FirstPersonWeaponSocket = FName("HandGrip_R");

	/** Name of the third person mesh weapon socket */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapons")
	FName ThirdPersonWeaponSocket = FName("HandGrip_R");

	/** Max distance to use for aim traces */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Aim", meta = (ClampMin = 0, ClampMax = 100000, Units = "cm"))
	float MaxAimDistance = 10000.0f;

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> FireAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayEffect> HealEffect;

	// Weapon management functions
	UFUNCTION(BlueprintCallable, Category="Weapons")
	void EquipWeapon(AFPSWeapon* Weapon);

	UFUNCTION(BlueprintCallable, Category="Weapons")
	void UnequipCurrentWeapon();

	UFUNCTION(BlueprintPure, Category="Weapons")
	AFPSWeapon* GetCurrentWeapon() const { return CurrentWeapon; }

	/** 테스트용: 기본 무기 자동 지급 */
	UFUNCTION(BlueprintCallable, Category="Weapons")
	virtual void GiveDefaultWeapon();

	/** 기본으로 지급할 무기 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapons")
	TSubclassOf<AFPSWeapon> DefaultWeaponClass;

	// Input
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> MouseLookMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;
};