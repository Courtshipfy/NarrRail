#include "NarrRailEditorModule.h"

#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FNarrRailEditorModule"

void FNarrRailEditorModule::StartupModule()
{
}

void FNarrRailEditorModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FNarrRailEditorModule, NarrRailEditor)
