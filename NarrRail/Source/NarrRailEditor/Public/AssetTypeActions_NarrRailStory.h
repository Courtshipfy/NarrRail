#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

/**
 * Asset type actions for UNarrRailStoryAsset
 */
class FAssetTypeActions_NarrRailStory : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_NarrRailStory(EAssetTypeCategories::Type InAssetCategory);

	// IAssetTypeActions interface
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override;
	virtual void GetActions(const TArray<UObject*>& InObjects, struct FToolMenuSection& Section) override;

private:
	EAssetTypeCategories::Type MyAssetCategory;
};
