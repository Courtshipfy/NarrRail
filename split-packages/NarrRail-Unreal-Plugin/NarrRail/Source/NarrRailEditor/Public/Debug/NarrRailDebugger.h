#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Runtime/NarrRailStorySession.h"
#include "NarrRailDebugger.generated.h"

/**
 * NarrRail 调试器：提供运行时会话状态、节点信息、变量值的可视化调试
 *
 * 使用方法：
 * 1. 在控制台输入命令：
 *    - narrrail.debug.session  (打印当前会话状态)
 *    - narrrail.debug.node     (打印当前节点)
 *    - narrrail.debug.vars     (打印所有变量)
 *    - narrrail.debug.history  (打印节点历史)
 *    - narrrail.debug.all      (打印完整调试信息)
 *
 * 2. 在蓝图中调用调试函数
 */

DECLARE_LOG_CATEGORY_EXTERN(LogNarrRailDebug, Log, All);

UCLASS()
class NARRRAILEDITOR_API UNarrRailDebugger : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * 打印会话的完整状态信息
     * 如果不指定会话，自动查找当前唯一的活跃会话
     * @param Session 会话对象（可选，留空则自动查找）
     * @param bVerbose 是否显示详细信息
     */
    UFUNCTION(BlueprintCallable, Category = "NarrRail|Debug")
    static void PrintSessionState(const UNarrRailStorySession* Session = nullptr, bool bVerbose = false);

    /**
     * 打印当前节点信息
     * @param Session 会话对象（可选，留空则自动查找）
     */
    UFUNCTION(BlueprintCallable, Category = "NarrRail|Debug")
    static void PrintCurrentNode(const UNarrRailStorySession* Session = nullptr);

    /**
     * 打印所有变量的当前值
     * @param Session 会话对象（可选，留空则自动查找）
     */
    UFUNCTION(BlueprintCallable, Category = "NarrRail|Debug")
    static void PrintAllVariables(const UNarrRailStorySession* Session = nullptr);

    /**
     * 打印节点历史记录
     * @param Session 会话对象（可选，留空则自动查找）
     * @param MaxCount 最多显示多少条历史（0 = 全部）
     */
    UFUNCTION(BlueprintCallable, Category = "NarrRail|Debug")
    static void PrintNodeHistory(const UNarrRailStorySession* Session = nullptr, int32 MaxCount = 10);

    /**
     * 打印当前可用的选项
     * @param Session 会话对象（可选，留空则自动查找）
     */
    UFUNCTION(BlueprintCallable, Category = "NarrRail|Debug")
    static void PrintCurrentChoices(const UNarrRailStorySession* Session = nullptr);

    /**
     * 打印最后一次选择的信息
     * @param Session 会话对象（可选，留空则自动查找）
     */
    UFUNCTION(BlueprintCallable, Category = "NarrRail|Debug")
    static void PrintLastChoice(const UNarrRailStorySession* Session = nullptr);

    /**
     * 打印剧情资产的结构信息
     * @param StoryAsset 剧情资产
     */
    UFUNCTION(BlueprintCallable, Category = "NarrRail|Debug")
    static void PrintStoryStructure(const UNarrRailStoryAsset* StoryAsset);

    /**
     * 在屏幕上显示调试信息（持续显示）
     * @param WorldContextObject 世界上下文
     * @param Session 会话对象（可选，留空则自动查找）
     * @param DisplayTime 显示时长（秒）
     */
    UFUNCTION(BlueprintCallable, Category = "NarrRail|Debug", meta = (WorldContext = "WorldContextObject"))
    static void ShowDebugOnScreen(UObject* WorldContextObject, const UNarrRailStorySession* Session = nullptr, float DisplayTime = 5.0f);

    /**
     * 打印完整调试信息（会话 + 节点 + 变量 + 历史）
     * @param Session 会话对象（可选，留空则自动查找）
     */
    UFUNCTION(BlueprintCallable, Category = "NarrRail|Debug")
    static void PrintFullDebugInfo(const UNarrRailStorySession* Session = nullptr);

    /**
     * 列出所有活跃的会话
     */
    UFUNCTION(BlueprintCallable, Category = "NarrRail|Debug")
    static void PrintAllActiveSessions();

private:
    // 自动查找唯一会话，如果有多个会话则报错
    static const UNarrRailStorySession* FindUniqueSession(const UNarrRailStorySession* ProvidedSession);
    static FString GetSessionStateString(ENarrRailSessionState State);
    static FString GetNodeTypeString(ENarrRailNodeType NodeType);
    static FString GetVariableTypeString(ENarrRailVariableType VarType);
    static FString GetComparisonOpString(ENarrRailComparisonOp Op);
    static FString GetActionTypeString(ENarrRailActionType ActionType);
    static void PrintSeparator(const FString& Title = TEXT(""));
};
