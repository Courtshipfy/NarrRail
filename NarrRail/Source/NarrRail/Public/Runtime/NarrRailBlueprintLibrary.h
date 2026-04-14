#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Runtime/NarrRailStoryAsset.h"
#include "Runtime/NarrRailStorySession.h"
#include "NarrRailBlueprintLibrary.generated.h"

/**
 * Blueprint 函数库：提供便捷的蓝图接口用于创建和操作剧情资产
 */
UCLASS()
class NARRRAIL_API UNarrRailBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // === 资产创建辅助函数 ===

    /**
     * 创建一个空的剧情资产
     * @param StoryId 剧情 ID
     * @param EntryNodeId 入口节点 ID
     * @return 新创建的剧情资产
     */
    UFUNCTION(BlueprintCallable, Category = "NarrRail|Asset", meta = (WorldContext = "WorldContextObject"))
    static UNarrRailStoryAsset* CreateStoryAsset(UObject* WorldContextObject, FName StoryId, FName EntryNodeId);

    /**
     * 添加对话节点到剧情资产
     * @param StoryAsset 剧情资产
     * @param NodeId 节点 ID
     * @param SpeakerId 说话人 ID
     * @param TextKey 文本键（用于本地化）
     * @return 是否添加成功
     */
    UFUNCTION(BlueprintCallable, Category = "NarrRail|Asset")
    static bool AddDialogueNode(UNarrRailStoryAsset* StoryAsset, FName NodeId, FName SpeakerId, const FString& TextKey);

    /**
     * 添加选择节点到剧情资产
     * @param StoryAsset 剧情资产
     * @param NodeId 节点 ID
     * @param Choices 选项列表
     * @return 是否添加成功
     */
    UFUNCTION(BlueprintCallable, Category = "NarrRail|Asset")
    static bool AddChoiceNode(UNarrRailStoryAsset* StoryAsset, FName NodeId, const TArray<FNarrRailChoiceOption>& Choices);

    /**
     * 添加结束节点到剧情资产
     * @param StoryAsset 剧情资产
     * @param NodeId 节点 ID
     * @return 是否添加成功
     */
    UFUNCTION(BlueprintCallable, Category = "NarrRail|Asset")
    static bool AddEndNode(UNarrRailStoryAsset* StoryAsset, FName NodeId);

    /**
     * 添加边（连接两个节点）
     * @param StoryAsset 剧情资产
     * @param SourceNodeId 源节点 ID
     * @param TargetNodeId 目标节点 ID
     * @param Priority 优先级（数字越小优先级越高）
     * @return 是否添加成功
     */
    UFUNCTION(BlueprintCallable, Category = "NarrRail|Asset")
    static bool AddEdge(UNarrRailStoryAsset* StoryAsset, FName SourceNodeId, FName TargetNodeId, int32 Priority = 0);

    /**
     * 创建一个简单的选项
     * @param TextKey 选项文本键
     * @param TargetNodeId 目标节点 ID
     * @return 选项结构
     */
    UFUNCTION(BlueprintPure, Category = "NarrRail|Asset")
    static FNarrRailChoiceOption MakeSimpleChoice(const FString& TextKey, FName TargetNodeId);

    // === 会话辅助函数 ===

    /**
     * 创建一个新的剧情会话
     * @param WorldContextObject 世界上下文对象
     * @param StoryAsset 剧情资产
     * @return 新创建的会话对象
     */
    UFUNCTION(BlueprintCallable, Category = "NarrRail|Session", meta = (WorldContext = "WorldContextObject"))
    static UNarrRailStorySession* CreateStorySession(UObject* WorldContextObject, const UNarrRailStoryAsset* StoryAsset);

    /**
     * 检查运行时结果是否成功
     * @param Result 运行时结果
     * @return 是否成功
     */
    UFUNCTION(BlueprintPure, Category = "NarrRail|Session")
    static bool IsResultSuccess(const FNarrRailRuntimeResult& Result);

    /**
     * 检查运行时结果是否为完成状态
     * @param Result 运行时结果
     * @return 是否完成
     */
    UFUNCTION(BlueprintPure, Category = "NarrRail|Session")
    static bool IsResultCompleted(const FNarrRailRuntimeResult& Result);

    /**
     * 获取当前节点的对话内容
     * @param Node 节点
     * @return 对话内容
     */
    UFUNCTION(BlueprintPure, Category = "NarrRail|Session")
    static FNarrRailDialoguePayload GetDialogueFromNode(const FNarrRailNode& Node);

    /**
     * 检查节点是否为对话节点
     * @param Node 节点
     * @return 是否为对话节点
     */
    UFUNCTION(BlueprintPure, Category = "NarrRail|Session")
    static bool IsDialogueNode(const FNarrRailNode& Node);

    /**
     * 检查节点是否为选择节点
     * @param Node 节点
     * @return 是否为选择节点
     */
    UFUNCTION(BlueprintPure, Category = "NarrRail|Session")
    static bool IsChoiceNode(const FNarrRailNode& Node);

    /**
     * 检查节点是否为结束节点
     * @param Node 节点
     * @return 是否为结束节点
     */
    UFUNCTION(BlueprintPure, Category = "NarrRail|Session")
    static bool IsEndNode(const FNarrRailNode& Node);
};
