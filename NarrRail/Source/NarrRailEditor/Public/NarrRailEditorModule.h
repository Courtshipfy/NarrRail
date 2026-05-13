#pragma once

#include "Modules/ModuleManager.h"

class FAssetTypeActions_Base;
class FExtender;
class FToolBarBuilder;
class FNarrRailStoryReimportHandler;

class FNarrRailEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    void AddToolbarExtension(FToolBarBuilder& Builder);
    void ReimportAllNarrRailStories();

    TArray<TSharedPtr<FAssetTypeActions_Base>> CreatedAssetTypeActions;
    TSharedPtr<FExtender> ToolbarExtender;
    TUniquePtr<FNarrRailStoryReimportHandler> StoryReimportHandler;
};
