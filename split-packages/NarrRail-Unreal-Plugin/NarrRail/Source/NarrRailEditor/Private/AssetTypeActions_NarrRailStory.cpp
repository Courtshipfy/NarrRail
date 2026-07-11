#include "AssetTypeActions_NarrRailStory.h"
#include "Runtime/NarrRailStoryAsset.h"
#include "ToolMenuSection.h"

FAssetTypeActions_NarrRailStory::FAssetTypeActions_NarrRailStory(EAssetTypeCategories::Type InAssetCategory)
	: MyAssetCategory(InAssetCategory)
{
}

FText FAssetTypeActions_NarrRailStory::GetName() const
{
	return FText::FromString(TEXT("NarrRail Story"));
}

FColor FAssetTypeActions_NarrRailStory::GetTypeColor() const
{
	// Purple-blue color for NarrRail assets
	return FColor(100, 150, 200);
}

UClass* FAssetTypeActions_NarrRailStory::GetSupportedClass() const
{
	return UNarrRailStoryAsset::StaticClass();
}

uint32 FAssetTypeActions_NarrRailStory::GetCategories()
{
	return MyAssetCategory;
}

bool FAssetTypeActions_NarrRailStory::HasActions(const TArray<UObject*>& InObjects) const
{
	return false; // No custom actions for now
}

void FAssetTypeActions_NarrRailStory::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
	// Future: Add custom context menu actions here
	// e.g., "Validate Story", "Export to YAML", etc.
}
