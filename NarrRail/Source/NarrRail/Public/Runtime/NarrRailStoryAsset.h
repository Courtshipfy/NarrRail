#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Runtime/NarrRailStoryTypes.h"
#include "Runtime/NarrRailVariableContainer.h"
class UAssetImportData;
#include "NarrRailStoryAsset.generated.h"

UCLASS(BlueprintType)
class NARRRAIL_API UNarrRailStoryAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    static constexpr int32 LatestSchemaVersion = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    int32 SchemaVersion = LatestSchemaVersion;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    FName StoryId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    FName EntryNodeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    TArray<FNarrRailVariableDefinition> Variables;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    TArray<FNarrRailNode> Nodes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    TArray<FNarrRailNodeEdge> Edges;

#if WITH_EDITORONLY_DATA
    UPROPERTY(VisibleAnywhere, Instanced, Category = "ImportSettings")
    TObjectPtr<UAssetImportData> AssetImportData;
#endif

    virtual void PostLoad() override;

#if WITH_EDITOR
    UAssetImportData* EnsureAssetImportData();
    UAssetImportData* GetAssetImportData() const;
#endif

private:
    void UpgradeSchemaIfNeeded();
};
