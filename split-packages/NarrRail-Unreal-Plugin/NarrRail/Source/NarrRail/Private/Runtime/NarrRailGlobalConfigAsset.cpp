#include "Runtime/NarrRailGlobalConfigAsset.h"

#if WITH_EDITOR
#include "EditorFramework/AssetImportData.h"

UAssetImportData* UNarrRailGlobalConfigAsset::EnsureAssetImportData()
{
    if (AssetImportData == nullptr)
    {
        AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
    }
    return AssetImportData;
}

UAssetImportData* UNarrRailGlobalConfigAsset::GetAssetImportData() const
{
    return const_cast<UNarrRailGlobalConfigAsset*>(this)->EnsureAssetImportData();
}
#endif
