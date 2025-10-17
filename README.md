# ProjectFPS - GAS 기반 FPS 포트폴리오

## 🎮 프로젝트 개요

### 프로젝트 목표
언리얼 엔진의 `Gameplay Ability System(GAS)`을 심층적으로 학습하고, 이를 활용하여 확장 가능한 스킬트리와 인벤토리 시스템을 구현한 FPS 포트폴리오입니다.

### 개발 기간
- **2025.09 ~ 2025.10** (약 1개월)

### 개발 플랫폼
- **Platform**: PC
- **Engine**: Unreal Engine 5.6

---

## ✨ 주요 기능

### 1. Gameplay Ability System (GAS)

#### **동적 데미지 계산**
- **SetByCaller**: 런타임에 동적으로 데미지 값 전달
- **크리티컬 시스템**: 5~20% 확률, 150~200% 배율
```cpp
// FPSWeapon.cpp
float AFPSWeapon::CalculateFinalDamage(/*...*/) const {
    //...
    float CritChance = PlayerAttrSet->GetCritChance();
    float CritDamage = PlayerAttrSet->GetCritDamage();

    if (FMath::FRand() < CritChance) {
        return BaseDamage * CritDamage;  // 크리티컬!
    }
    return BaseDamage;
}

// FPSProjectile.cpp
bool AFPSProjectile::ApplyDamageToTarget(AActor* Target)
{
	//...
	FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
	ContextHandle.AddInstigator(GetInstigator(), this);
	
	FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, ContextHandle);
	if (SpecHandle.IsValid())
	{
		// Damage 값을 GameplayEffect Magnitude로 설정
		SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), -Damage);
		
		//...
	
		TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		return true;
	}
}

```

#### **PostGameplayEffectExecute - Shield 우선 소모**
```cpp
// CharacterAttributeSet.cpp
void UCharacterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) {
    if (Data.EvaluatedData.Attribute == GetHealthAttribute()) {
        float DamageMagnitude = Data.EvaluatedData.Magnitude;

        if (DamageMagnitude < 0.0f) {
            float AbsoluteDamage = -DamageMagnitude;
            float CurrentShield = GetShield();

            if (CurrentShield > 0.0f) {
                float ShieldDamage = FMath::Min(CurrentShield, AbsoluteDamage);
                SetShield(CurrentShield - ShieldDamage);
                SetHealth(GetHealth() + ShieldDamage);  // Shield가 막은 만큼 복구
            }
        }
    }
}
```

#### **Duration ≠ Cooldown 분리**
- Ability 지속 시간과 독립적인 쿨다운 관리
- `ApplyCooldown()` 수동 적용으로 유연한 제어
```cpp
// GameplayAbility_ShieldBarrier.cpp
void UGameplayAbility_ShieldBarrier::ActivateAbility(...) {
    if (!CommitCheck(...)) { EndAbility(...); return; }
    CommitExecute(...);      // 내부적으로 쿨다운 적용
    SpawnBarrierVisual();    // Ability 로직 (20초 지속)
}
```

---

### 2. 스킬트리 시스템

![Skill Tree Demo](docs/images/skilltree.gif)

#### **DataAsset 기반 스킬 데이터**
```cpp
// BaseSkillData.h
UCLASS(BlueprintType)
class UBaseSkillData : public UPrimaryDataAsset {
    FGameplayTag SkillID;                           // Skill.Shield.Tier1
    TArray<FGameplayTag> PrerequisiteSkills;        // 선행 스킬 (OR 조건)
    TArray<FGameplayTag> MutuallyExclusiveSkills;   // 상호배타적 스킬
    TArray<TSubclassOf<UGameplayEffect>> SkillEffects;      // 습득 시 적용
    TArray<TSubclassOf<UGameplayAbility>> SkillAbilities;   // 액티브 스킬용
};
```

#### **분기형 스킬트리**
- **Shield 브랜치**: 방어 특화 (MaxHealth, Shield 증가 → Shield Barrier 액티브 스킬)
- **Crit 브랜치**: 공격 특화 (CritChance, CritDamage 증가 → Berserker 액티브 스킬)
- **PrerequisiteSkills OR 조건**: 상위 브랜치를 해제 했는지 체크
- **MutuallyExclusiveSkills 조건**: 두 브랜치 중 하나만 선택할 수 있도록 체크

#### **액티브 스킬**
| 스킬 | 효과 | Duration | Cooldown |
|------|------|----------|----------|
| Shield Barrier | 외부 공격 차단 방어막 | 20초 | 60초 |
| Berserker | 공격속도 +50%, 이동속도 +30% | 10초 | 30초 |

---

### 3. 인벤토리 시스템 (디아블로2 스타일)

![Inventory Demo](docs/images/inventory.gif)

#### **8x6 그리드 시스템**
- **가변 크기 아이템**: 1x1 (포션), 2x4 (무기)
- **O(1) Origin 탐색**: Item Slot의 기준(=좌상단)이 되는 인덱스 캐싱으로 성능 최적화
- **자동 스택 병합**: ItemID 기반 같은 아이템 자동 합침

```cpp
// InventoryComponent.cpp
struct FInventorySlot {
    TObjectPtr<UBaseItemData> ItemData;  // Origin에만 저장
    bool bIsOccupied;
    bool bIsOrigin;
    FIntPoint OriginPos;  // O(1) 탐색용 캐싱
};

// O(n) → O(1) 최적화
bool FindItemOrigin(int32 GridX, int32 GridY, int32& OutOriginX, int32& OutOriginY) const {
    //...
    FIntPoint OriginPos = GridSlots[SlotIndex].OriginPos;
    OutOriginX = OriginPos.X;  // 즉시 반환!
    OutOriginY = OriginPos.Y;
    return true;
}
```

#### **드래그 앤 드롭 시스템**
- 그리드 ↔ 그리드: 아이템 재배치
- 그리드 ↔ 무기 슬롯: 장착/해제
- 무기 슬롯 ↔ 무기 슬롯: 무기 교환 (Swap)

---

### 4. 무기 시스템

#### **2슬롯 캐싱 방식**
- **Primary / Secondary 슬롯**: 무기 생성 후 보관
- **Show/Hide 전환**: 스폰/파괴 없이 상태 보존
- **런타임 정보 보존**: 탄약, 내구도 등


#### **리로드 시스템**
- **AnimNotify_RefillAmmo**: 애니메이션 특정 시점에 정밀 탄약 보충
- **GameplayAbility_Reload**: GAS 기반 리로드 능력
- **리로드 중 발사 차단**: 활성 Ability Tag 감지

---

### 아키텍처 패턴
```
인터페이스 패턴
├── IPickupable (픽업 가능 아이템)
├── IFPSWeaponHolder (무기 보유자)
└── 다형성으로 확장 가능

컴포넌트 기반 설계
├── InventoryComponent (인벤토리 관리)
├── WeaponSlotComponent (무기 슬롯 관리)
├── SkillComponent (스킬 관리)
└── 재사용 가능한 모듈화

델리게이트 패턴
├── OnInventoryChanged (인벤토리 변경 시 UI 갱신)
├── OnActiveSkillChanged (액티브 스킬 변경)
└── OnHealthChanged, OnStaminaChanged (Attribute 델리게이트)
```

---

## 📂 시스템 구조

### 프로젝트 구조
```
ProjectFPS/
├── Source/ProjectFPS/FPS/
│   ├── FPSCharacter.h/cpp                    # 기본 FPS 캐릭터
│   ├── FPSPlayerCharacter.h/cpp              # 플레이어 전용 (입력, HUD)
│   ├── CharacterAttributeSet.h/cpp           # 공통 Attribute (Health, Shield, Stamina)
│   ├── PlayerAttributeSet.h/cpp              # 플레이어 Attribute (SkillPoint, CritChance)
│   │
│   ├── Weapons/
│   │   ├── FPSWeapon.h/cpp                   # 무기 기본 클래스
│   │   ├── FPSWeaponHolder.h                 # 무기 홀더 인터페이스
│   │   └── FPSProjectile.h/cpp               # 발사체
│   │
│   ├── Items/
│   │   ├── BaseItemData.h/cpp                # 아이템 마스터 데이터
│   │   ├── WeaponItemData.h/cpp              # 무기 데이터
│   │   └── ConsumableItemData.h/cpp          # 소모품 데이터
│   │
│   ├── Components/
│   │   ├── InventoryComponent.h/cpp          # 그리드 인벤토리
│   │   ├── WeaponSlotComponent.h/cpp         # 무기 슬롯 (2슬롯)
│   │   ├── SkillComponent.h/cpp              # 스킬 관리
│   │   └── PickupTriggerComponent.h/cpp      # 픽업 감지
│   │
│   ├── UI/
│   │   ├── PlayerHUD.h/cpp                   # 체력/스태미나/탄약 HUD
│   │   ├── InventoryWidget.h/cpp             # 인벤토리 UI
│   │   ├── SkillTreeWidget.h/cpp             # 스킬트리 UI
│   │   ├── ActiveSkillWidget.h/cpp           # Q키 액티브 스킬 UI
│   │   └── ToastManagerWidget.h/cpp          # Toast 메시지
│   │
│   ├── AI/
│   │   ├── FPSEnemyCharacter.h/cpp           # 적 캐릭터
│   │   └── FPSEnemyAIController.h/cpp        # AI 컨트롤러
│   │
│   ├── GameplayAbility_FireProjectile.h/cpp
│   ├── GameplayAbility_Reload.h/cpp
│   ├── GameplayAbility_Sprint.h/cpp
│   ├── GameplayAbility_ShieldBarrier.h/cpp
│   ├── GameplayAbility_Berserker.h/cpp
│   └── GameplayAbility_UseConsumable.h/cpp
│
└── Content/
    ├── Blueprints/
    │   ├── BP_FPSPlayerCharacter            # 플레이어 Blueprint
    │   ├── BP_FPSEnemyCharacter             # 적 Blueprint
    │   └── Weapons/
    │       └── BP_FPSProjectile             # 발사체 Blueprint
    │
    ├── Data/
    │   ├── Skills/                          # 스킬 DataAsset
    │   │   ├── ShieldBranch/
    │   │   └── CritBranch/
    │   ├── Items/                           # 아이템 DataAsset
    │   │   ├── Weapon/
    │   │   └── Potion/
    │   └── Drops/                           # 드롭 테이블 DataAsset
    │
    └── UI/
        ├── WB_PlayerHUD                     # HUD Widget
        ├── WB_Inventory                     # 인벤토리 Widget
        ├── WB_SkillTree                     # 스킬트리 Widget
        └── WB_ActiveSkill                   # 액티브 스킬 Widget
```


## 🎥 시연 영상

### YouTube
[![Portfolio Video](https://img.youtube.com/vi/uJkcR_4VAIM/maxresdefault.jpg)](https://www.youtube.com/watch?v=uJkcR_4VAIM)

**주요 씬 타임스탬프**:
- [00:07] 전투 시스템 (치명타, 방어막, 스프린트)
- [00:52] 스킬트리 시스템 (분기형, 액티브 스킬)
- [01:55] 인벤토리 시스템 (그리드, 포션 사용, 무기 슬롯)


## 📄 라이선스

이 프로젝트는 포트폴리오 목적으로 개발되었으며, 학습 및 비상업적 용도로만 사용 가능합니다.


</br>
<div align="center">

![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.6-blue)
![Language](https://img.shields.io/badge/Language-C%2B%2B%20(90%25)-orange)
![License](https://img.shields.io/badge/License-Portfolio-green)

</div>