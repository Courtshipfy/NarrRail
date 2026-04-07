#include "Runtime/NarrRailStoryAsset.h"

void UNarrRailStoryAsset::PostLoad()
{
    Super::PostLoad();
    UpgradeSchemaIfNeeded();
}

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
