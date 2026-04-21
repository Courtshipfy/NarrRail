#include "Factories/NarrRailStoryFactory.h"
#include "Importers/NarrRailYamlParser.h"
#include "Runtime/NarrRailStoryAsset.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"

UNarrRailStoryFactory::UNarrRailStoryFactory()
{
	bCreateNew = false;
	bEditAfterNew = true;
	bEditorImport = true;
	SupportedClass = UNarrRailStoryAsset::StaticClass();

	// Register file formats
	Formats.Add(TEXT("yaml;NarrRail Story Script"));
	Formats.Add(TEXT("yml;NarrRail Story Script"));
}

bool UNarrRailStoryFactory::DoesSupportClass(UClass* Class)
{
	return Class == UNarrRailStoryAsset::StaticClass();
}

UClass* UNarrRailStoryFactory::ResolveSupportedClass()
{
	return UNarrRailStoryAsset::StaticClass();
}

FText UNarrRailStoryFactory::GetDisplayName() const
{
	return FText::FromString(TEXT("NarrRail Story Script"));
}

bool UNarrRailStoryFactory::FactoryCanImport(const FString& Filename)
{
	FString Extension = FPaths::GetExtension(Filename);
	return Extension == TEXT("yaml") || Extension == TEXT("yml");
}

UObject* UNarrRailStoryFactory::FactoryCreateFile(
	UClass* InClass,
	UObject* InParent,
	FName InName,
	EObjectFlags Flags,
	const FString& Filename,
	const TCHAR* Parms,
	FFeedbackContext* Warn,
	bool& bOutOperationCancelled)
{
	bOutOperationCancelled = false;

	// Parse YAML file
	FNarrRailYamlParser Parser;
	FNarrRailScriptData ScriptData;
	FString ErrorMessage;

	if (!Parser.ParseFile(Filename, ScriptData, ErrorMessage))
	{
		FText ErrorTitle = FText::FromString(TEXT("Import Failed"));
		FText ErrorText = FText::Format(
			FText::FromString(TEXT("Failed to import {0}:\n\n{1}")),
			FText::FromString(FPaths::GetCleanFilename(Filename)),
			FText::FromString(ErrorMessage)
		);
		FMessageDialog::Open(EAppMsgType::Ok, ErrorText, ErrorTitle);

		if (Warn)
		{
			Warn->Logf(ELogVerbosity::Error, TEXT("NarrRail Import Error: %s"), *ErrorMessage);
		}

		return nullptr;
	}

	// Create new asset
	UNarrRailStoryAsset* NewAsset = NewObject<UNarrRailStoryAsset>(
		InParent,
		InClass,
		InName,
		Flags
	);

	if (!NewAsset)
	{
		return nullptr;
	}

	// Fill asset data
	NewAsset->SchemaVersion = ScriptData.SchemaVersion;
	NewAsset->StoryId = FName(*ScriptData.StoryId);
	NewAsset->EntryNodeId = FName(*ScriptData.EntryNodeId);
	NewAsset->Variables = ScriptData.Variables;
	NewAsset->Nodes = ScriptData.Nodes;
	NewAsset->Edges = ScriptData.Edges;

	// Mark as modified
	NewAsset->MarkPackageDirty();

	// Log success
	if (Warn)
	{
		Warn->Logf(ELogVerbosity::Log, TEXT("Successfully imported NarrRail story: %s"), *ScriptData.StoryId);
		Warn->Logf(ELogVerbosity::Log, TEXT("  - %d variables"), ScriptData.Variables.Num());
		Warn->Logf(ELogVerbosity::Log, TEXT("  - %d nodes"), ScriptData.Nodes.Num());
		Warn->Logf(ELogVerbosity::Log, TEXT("  - %d edges"), ScriptData.Edges.Num());
	}

	return NewAsset;
}
