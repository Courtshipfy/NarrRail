#include "Runtime/NarrRailStoryAsset.h"
#include "EditorFramework/AssetImportData.h"

void UNarrRailStoryAsset::PostLoad()
{
    Super::PostLoad();

#if WITH_EDITORONLY_DATA
    if (AssetImportData == nullptr)
    {
        AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
    }
#endif

    UpgradeSchemaIfNeeded();
}

#if WITH_EDITOR
UAssetImportData* UNarrRailStoryAsset::EnsureAssetImportData()
{
    if (AssetImportData == nullptr)
    {
        AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
    }
    return AssetImportData;
}

UAssetImportData* UNarrRailStoryAsset::GetAssetImportData() const
{
    return const_cast<UNarrRailStoryAsset*>(this)->EnsureAssetImportData();
}
#endif

void UNarrRailStoryAsset::UpgradeSchemaIfNeeded()
{
    if (SchemaVersion <= 0)
    {
        SchemaVersion = LatestSchemaVersion;
    }

    if (SchemaVersion < LatestSchemaVersion)
    {
        SchemaVersion = LatestSchemaVersion;
    }
}
