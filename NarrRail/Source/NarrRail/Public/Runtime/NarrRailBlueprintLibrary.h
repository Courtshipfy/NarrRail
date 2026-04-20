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
     * 添加带条件的边
     * @param StoryAsset 剧情资产
     * @param SourceNodeId 源节点 ID
     * @param TargetNodeId 目标节点 ID
     * @param Priority 优先级（数字越小优先级越高）
     * @param Condition 条件表达式
     * @return 是否添加成功
     */
    UFUNCTION(BlueprintCallable, Category = "NarrRail|Asset")
    static bool AddEdgeWithCondition(UNarrRailStoryAsset* StoryAsset, FName SourceNodeId, FName TargetNodeId, int32 Priority, const FNarrRailConditionExpression& Condition);

    /**
     * 添加变量定义到剧情资产
     * @param StoryAsset 剧情资产
     * @param VariableName 变量名
     * @param VariableType 变量类型
     * @param bGlobalScope 是否为全局作用域（false = 会话级）
     * @param DefaultValue 默认值（字符串形式）
     * @return 是否添加成功
     */
    UFUNCTION(BlueprintCallable, Category = "NarrRail|Asset")
    static bool AddVariableDefinition(UNarrRailStoryAsset* StoryAsset, FName VariableName, ENarrRailVariableType VariableType, bool bGlobalScope, const FString& DefaultValue);

    /**
     * 给节点添加进入动作
     * @param StoryAsset 剧情资产
     * @param NodeId 节点 ID
     * @param Action 动作
     * @return 是否添加成功
     */
    UFUNCTION(BlueprintCallable, Category = "NarrRail|Asset")
    static bool AddNodeEnterAction(UNarrRailStoryAsset* StoryAsset, FName NodeId, const FNarrRailNodeAction& Action);

    /**
     * 给节点添加退出动作
     * @param StoryAsset 剧情资产
     * @param NodeId 节点 ID
     * @param Action 动作
     * @return 是否添加成功
     */
    UFUNCTION(BlueprintCallable, Category = "NarrRail|Asset")
    static bool AddNodeExitAction(UNarrRailStoryAsset* StoryAsset, FName NodeId, const FNarrRailNodeAction& Action);

    /**
     * 创建一个简单的选项
     * @param TextKey 选项文本键
     * @param TargetNodeId 目标节点 ID
     * @return 选项结构
     */
    UFUNCTION(BlueprintPure, Category = "NarrRail|Asset")
    static FNarrRailChoiceOption MakeSimpleChoice(const FString& TextKey, FName TargetNodeId);

    /**
     * 创建变量引用
     * @param VariableName 变量名
     * @param VariableType 变量类型
     * @param bGlobalScope 是否为全局作用域
     * @return 变量引用结构
     */
    UFUNCTION(BlueprintPure, Category = "NarrRail|Asset")
    static FNarrRailVariableRef MakeVariableRef(FName VariableName, ENarrRailVariableType VariableType, bool bGlobalScope);

    /**
     * 创建条件项
     * @param Variable 变量引用
     * @param Operator 比较运算符
     * @param CompareValue 比较值（字符串形式）
     * @return 条件项结构
     */
    UFUNCTION(BlueprintPure, Category = "NarrRail|Asset")
    static FNarrRailConditionTerm MakeConditionTerm(const FNarrRailVariableRef& Variable, ENarrRailComparisonOp Operator, const FString& CompareValue);

    /**
     * 创建条件表达式
     * @param Logic 逻辑运算符（All/Any）
     * @param Terms 条件项列表
     * @return 条件表达式结构
     */
    UFUNCTION(BlueprintPure, Category = "NarrRail|Asset")
    static FNarrRailConditionExpression MakeConditionExpression(ENarrRailConditionLogic Logic, const TArray<FNarrRailConditionTerm>& Terms);

    /**
     * 创建节点动作
     * @param ActionType 动作类型
     * @param Variable 变量引用
     * @param Value 值（字符串形式）
     * @return 动作结构
     */
    UFUNCTION(BlueprintPure, Category = "NarrRail|Asset")
    static FNarrRailNodeAction MakeNodeAction(ENarrRailActionType ActionType, const FNarrRailVariableRef& Variable, const FString& Value);

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
