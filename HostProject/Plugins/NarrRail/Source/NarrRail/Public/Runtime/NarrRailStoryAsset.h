#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Runtime/NarrRailStoryTypes.h"
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
    TArray<FNarrRailVariableRef> Variables;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    TArray<FNarrRailNode> Nodes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    TArray<FNarrRailNodeEdge> Edges;

    virtual void PostLoad() override;

private:
    void UpgradeSchemaIfNeeded();
};
