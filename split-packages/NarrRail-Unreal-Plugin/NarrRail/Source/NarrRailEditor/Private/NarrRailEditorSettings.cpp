#include "NarrRailEditorSettings.h"

UNarrRailEditorSettings::UNarrRailEditorSettings()
{
    StoryAssetRootPath = TEXT("/Game/NarrRailStories");
}

FName UNarrRailEditorSettings::GetCategoryName() const
{
    return TEXT("Plugins");
}

FName UNarrRailEditorSettings::GetContainerName() const
{
    return TEXT("Project");
}

FName UNarrRailEditorSettings::GetSectionName() const
{
    return TEXT("NarrRail");
}
