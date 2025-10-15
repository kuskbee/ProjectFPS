// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CharacterAttributeSet.generated.h"


// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class PROJECTFPS_API UCharacterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UCharacterAttributeSet();

	// ü�� �Ӽ�
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, Health);

	// �ִ� ü�� �Ӽ�
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, MaxHealth);

	// ���� �Ӽ�
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Mana)
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, Mana);

	// �ִ� ���� �Ӽ�
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxMana)
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, MaxMana);

	// ���¹̳� �Ӽ�
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, Stamina);

	// �ִ� ���¹̳� �Ӽ�
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, MaxStamina);

	// Shield
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Shield)
	FGameplayAttributeData Shield;
	ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, Shield);

	// Max Shield
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxShield)
	FGameplayAttributeData MaxShield;
	ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, MaxShield);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	virtual void OnRep_Mana(const FGameplayAttributeData& OldMana);

	UFUNCTION()
	virtual void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana);

	UFUNCTION()
	virtual void OnRep_Stamina(const FGameplayAttributeData& OldStamina);

	UFUNCTION()
	virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);

	UFUNCTION()
	virtual void OnRep_Shield(const FGameplayAttributeData& OldShield);

	UFUNCTION()
	virtual void OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield);

public:
	// Attribute 값 변경 전 클램핑 처리 (0 이하 방지, 최대값 초과 방지)
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// GameplayEffect 적용 후 호출 (Effect 제거 등 로직 처리)
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

protected:
	/** 데미지 숫자 위젯 스폰 (월드 공간에 표시) */
	void SpawnDamageNumberWidget(const FGameplayEffectModCallbackData& Data);

public:
	/** 데미지 숫자 위젯 클래스 (Blueprint에서 설정 가능, FPSCharacter에서 초기화) */
	static TSubclassOf<class UDamageNumberWidget> DamageNumberWidgetClass;
};
