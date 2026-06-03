#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "NarrRailEditorSettings.generated.h"

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="NarrRail"))
class NARRRAILEDITOR_API UNarrRailEditorSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UNarrRailEditorSettings();

    UPROPERTY(Config, EditAnywhere, Category="Story Repository", meta=(DisplayName="Story Repository Path", ToolTip="Local folder that contains NarrRail .nrstory files. Sync Stories will mirror this folder into UE assets."))
    FDirectoryPath StoryRepositoryPath;

    UPROPERTY(Config, EditAnywhere, Category="Story Repository", meta=(DisplayName="Pull Git Repository Before Sync", ToolTip="If enabled and the story repository is a Git working tree, Sync Stories runs 'git pull --ff-only' before importing assets."))
    bool bPullGitRepositoryBeforeSync = true;

    UPROPERTY(Config, EditAnywhere, Category="Story Repository", meta=(DisplayName="Story Asset Root Path (UE Content)", ToolTip="UE Content root used for synced story assets. The repository folder name is appended under this path, for example /Game/NarrRailStories/MyRepo."))
    FString StoryAssetRootPath;

    virtual FName GetContainerName() const override;
    virtual FName GetCategoryName() const override;
    virtual FName GetSectionName() const override;
};
