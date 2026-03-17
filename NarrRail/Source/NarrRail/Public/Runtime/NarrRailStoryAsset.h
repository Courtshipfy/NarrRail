#pragma once

#include "CoreMinimal.h"
#include "Engine/PrimaryDataAsset.h"
#include "Runtime/NarrRailStoryTypes.h"
#include "NarrRailStoryAsset.generated.h"

// 剧情主资产。保存剧情图定义，是运行时和编辑器的核心输入。
UCLASS(BlueprintType)
class NARRRAIL_API UNarrRailStoryAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // 剧情标识，用于外部索引或存档绑定。
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    FName StoryId = NAME_None;

    // 入口节点 ID。执行器 Start 时从该节点开始。
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    FName EntryNodeId = NAME_None;

    // 节点集合。
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    TArray<FNarrRailNode> Nodes;

    // 显式边集合。
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    TArray<FNarrRailNodeEdge> Edges;
};
