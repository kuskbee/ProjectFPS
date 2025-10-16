// ItemDropTable.h

#pragma once

#include "CoreMinimal.h"
#include "ItemDropTable.generated.h"

class UBaseItemData;

/**
 * 아이템 드롭 엔트리
 * - 어떤 아이템을 얼마나 드롭할지 정의
 */
USTRUCT(BlueprintType)
struct FItemDropEntry
{
	GENERATED_BODY()

	/** 드롭할 아이템 데이터 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Drop")
	TObjectPtr<UBaseItemData> ItemData;

	/** 드롭 확률 (0.0 ~ 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Drop", meta = (ClampMin = 0.0, ClampMax = 1.0))
	float DropChance = 0.3f;

	/** 드롭 개수 (최소) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Drop", meta = (ClampMin = 1))
	int32 MinCount = 1;

	/** 드롭 개수 (최대) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Drop", meta = (ClampMin = 1))
	int32 MaxCount = 1;
};

/**
 * 아이템 드롭 테이블
 * - 여러 아이템의 드롭 확률을 정의
 */
USTRUCT(BlueprintType)
struct FItemDropTable
{
	GENERATED_BODY()

	/** 드롭 가능한 아이템 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Drop")
	TArray<FItemDropEntry> DropEntries;

	/** 랜덤 아이템 선택 (확률 기반) */
	TArray<UBaseItemData*> RollDrops() const;
};
