#pragma once

#include "ApptData.generated.h"

USTRUCT(BlueprintType)
struct FItemData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<class AApptItem> ItemClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UTexture2D* ItemImage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float ItemCost;
};