#include "Importers/NarrRailStoryReimportHandler.h"

#include "Importers/NarrRailYamlParser.h"
#include "Runtime/NarrRailStoryAsset.h"
#include "EditorFramework/AssetImportData.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY_STATIC(LogNarrRailReimport, Log, All);

namespace
{
    static bool ResolveReadableSourcePath(const FString& InPath, FString& OutResolvedAbsPath)
    {
        if (InPath.IsEmpty())
        {
            return false;
        }

        TArray<FString> Candidates;
        Candidates.Reserve(4);
        Candidates.Add(InPath);
        Candidates.Add(FPaths::ConvertRelativePathToFull(InPath));

        if (InPath.StartsWith(TEXT("/")) || InPath.StartsWith(TEXT("\\")))
        {
            const FString Trimmed = InPath.RightChop(1);
            Candidates.Add(FPaths::Combine(FPaths::ProjectDir(), Trimmed));
            Candidates.Add(FPaths::Combine(FPaths::RootDir(), Trimmed));
        }

        for (FString Candidate : Candidates)
        {
            FPaths::NormalizeFilename(Candidate);
            FPaths::CollapseRelativeDirectories(Candidate);
            if (FPaths::FileExists(Candidate))
            {
                OutResolvedAbsPath = MoveTemp(Candidate);
                return true;
            }
        }

        return false;
    }
}

bool FNarrRailStoryReimportHandler::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
    const UNarrRailStoryAsset* StoryAsset = Cast<UNarrRailStoryAsset>(Obj);
    if (StoryAsset == nullptr)
    {
        return false;
    }

    const UAssetImportData* ImportData = StoryAsset->GetAssetImportData();
    if (ImportData == nullptr)
    {
        return false;
    }

    ImportData->ExtractFilenames(OutFilenames);
    return OutFilenames.Num() > 0;
}

void FNarrRailStoryReimportHandler::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
    UNarrRailStoryAsset* StoryAsset = Cast<UNarrRailStoryAsset>(Obj);
    if (StoryAsset == nullptr || NewReimportPaths.Num() == 0)
    {
        return;
    }

    if (UAssetImportData* ImportData = StoryAsset->GetAssetImportData())
    {
        FString Normalized = FPaths::ConvertRelativePathToFull(NewReimportPaths[0]);
        FPaths::NormalizeFilename(Normalized);
        FPaths::CollapseRelativeDirectories(Normalized);
        ImportData->UpdateFilenameOnly(Normalized);
    }
}

EReimportResult::Type FNarrRailStoryReimportHandler::Reimport(UObject* Obj)
{
    UNarrRailStoryAsset* StoryAsset = Cast<UNarrRailStoryAsset>(Obj);
    if (StoryAsset == nullptr)
    {
        return EReimportResult::Failed;
    }

    UAssetImportData* ImportData = StoryAsset->GetAssetImportData();
    if (ImportData == nullptr)
    {
        UE_LOG(LogNarrRailReimport, Error, TEXT("Reimport failed: asset has no import data (%s)"), *GetPathNameSafe(StoryAsset));
        return EReimportResult::Failed;
    }

    const FString SourceFilename = ImportData->GetFirstFilename();
    if (SourceFilename.IsEmpty())
    {
        UE_LOG(LogNarrRailReimport, Error, TEXT("Reimport failed: source filename is empty (%s)"), *GetPathNameSafe(StoryAsset));
        return EReimportResult::Failed;
    }

    FString AbsSourceFilename;
    if (!ResolveReadableSourcePath(SourceFilename, AbsSourceFilename))
    {
        UE_LOG(LogNarrRailReimport, Error, TEXT("Reimport failed: source file not found. raw='%s'"), *SourceFilename);
        return EReimportResult::Failed;
    }

    FNarrRailYamlParser Parser;
    FNarrRailScriptData ScriptData;
    FString ErrorMessage;
    if (!Parser.ParseFile(AbsSourceFilename, ScriptData, ErrorMessage))
    {
        UE_LOG(LogNarrRailReimport, Error, TEXT("Reimport failed: YAML parse error (%s) -> %s"), *AbsSourceFilename, *ErrorMessage);
        return EReimportResult::Failed;
    }

    StoryAsset->Modify();
    StoryAsset->SchemaVersion = ScriptData.SchemaVersion;
    StoryAsset->StoryId = FName(*ScriptData.StoryId);
    StoryAsset->EntryNodeId = FName(*ScriptData.EntryNodeId);
    StoryAsset->Variables = ScriptData.Variables;
    StoryAsset->Nodes = ScriptData.Nodes;
    StoryAsset->Edges = ScriptData.Edges;
    ImportData->UpdateFilenameOnly(AbsSourceFilename);

    StoryAsset->PostEditChange();
    StoryAsset->MarkPackageDirty();

    UE_LOG(LogNarrRailReimport, Log, TEXT("Reimport succeeded: %s (%s)"), *GetPathNameSafe(StoryAsset), *AbsSourceFilename);
    return EReimportResult::Succeeded;
}

int32 FNarrRailStoryReimportHandler::GetPriority() const
{
    return 0;
}
