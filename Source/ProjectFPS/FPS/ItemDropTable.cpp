// ItemDropTable.cpp

#include "ItemDropTable.h"
#include "Items/BaseItemData.h"

TArray<UBaseItemData*> FItemDropTable::RollDrops() const
{
	TArray<UBaseItemData*> DroppedItems;

	for (const FItemDropEntry& Entry : DropEntries)
	{
		// 확률 체크
		if (FMath::FRand() <= Entry.DropChance)
		{
			// 드롭 개수 랜덤
			int32 DropCount = FMath::RandRange(Entry.MinCount, Entry.MaxCount);

			// 아이템 데이터 복제 (CurrentStackSize 설정)
			if (DropCount > 0 && Entry.ItemData)
			{
				UBaseItemData* DroppedItem = DuplicateObject(Entry.ItemData, nullptr);
				if (DroppedItem)
				{
					DroppedItem->CurrentStackSize = DropCount;
					DroppedItems.Add(DroppedItem);
				}
			}
		}
	}

	return DroppedItems;
}
