#pragma once

#include "Modules/ModuleManager.h"

class FAssetTypeActions_Base;

class FNarrRailEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    TArray<TSharedPtr<FAssetTypeActions_Base>> CreatedAssetTypeActions;
};
