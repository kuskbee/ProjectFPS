// ItemDropTableDataAsset.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDropTable.h"
#include "ItemDropTableDataAsset.generated.h"

/**
 * 아이템 드롭 테이블 DataAsset
 * - Blueprint에서 쉽게 편집 가능
 * - 재사용 가능한 드롭 프로필
 */
UCLASS(BlueprintType)
class PROJECTFPS_API UItemDropTableDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 드롭 테이블 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Drop")
	FItemDropTable DropTable;

	/** 편의 함수: 드롭 추첨 */
	UFUNCTION(BlueprintCallable, Category = "Item Drop")
	TArray<UBaseItemData*> RollDrops() const
	{
		return DropTable.RollDrops();
	}
};
