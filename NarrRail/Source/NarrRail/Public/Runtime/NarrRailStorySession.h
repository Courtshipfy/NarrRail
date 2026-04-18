#pragma once

#include "CoreMinimal.h"
#include "Runtime/NarrRailStoryAsset.h"
#include "Runtime/NarrRailVariableContainer.h"
#include "NarrRailStorySession.generated.h"

UENUM(BlueprintType)
enum class ENarrRailRuntimeResultCode : uint8
{
    Success UMETA(DisplayName = "Success"),
    InvalidState UMETA(DisplayName = "Invalid State"),
    InvalidInput UMETA(DisplayName = "Invalid Input"),
    MissingNode UMETA(DisplayName = "Missing Node"),
    Completed UMETA(DisplayName = "Completed")
};

UENUM(BlueprintType)
enum class ENarrRailSessionState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Running UMETA(DisplayName = "Running"),
    WaitingForChoice UMETA(DisplayName = "Waiting For Choice"),
    Paused UMETA(DisplayName = "Paused"),
    Completed UMETA(DisplayName = "Completed"),
    Error UMETA(DisplayName = "Error")
};

// 会话上下文：单次剧情会话的运行时快照，保证会话状态彼此隔离。
USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailSessionContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    FName CurrentNodeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    TArray<FName> NodeHistory;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    TMap<FName, FString> VariableSnapshot;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    TArray<FName> EmittedEvents;
};

USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailRuntimeResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    ENarrRailRuntimeResultCode Code = ENarrRailRuntimeResultCode::Success;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    FString Message;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    FName NodeId = NAME_None;

    static FNarrRailRuntimeResult Make(const ENarrRailRuntimeResultCode InCode, const TCHAR* InMessage, const FName InNodeId = NAME_None)
    {
        FNarrRailRuntimeResult Result;
        Result.Code = InCode;
        Result.Message = InMessage;
        Result.NodeId = InNodeId;
        return Result;
    }
};

// 运行时事件委托声明
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarrRailSessionStartedDelegate, FName, EntryNodeId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNarrRailNodeEnteredDelegate, FName, NodeId, const FNarrRailNode&, Node);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNarrRailNodeExitedDelegate, FName, NodeId, const FNarrRailNode&, Node);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNarrRailSessionEndedDelegate, FName, LastNodeId, ENarrRailSessionState, EndState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNarrRailChoicesReadyDelegate, FName, NodeId, const TArray<FNarrRailChoiceOption>&, Choices);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FNarrRailChoiceSelectedDelegate, FName, NodeId, int32, ChoiceIndex, FName, TargetNodeId);

// 剧情会话执行器：负责运行时 Start/Next/Choose/Pause/Resume/Stop 流程推进。
UCLASS(BlueprintType)
class NARRRAIL_API UNarrRailStorySession : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "NarrRail|Runtime")
    FNarrRailRuntimeResult Initialize(const UNarrRailStoryAsset* InStoryAsset);

    UFUNCTION(BlueprintCallable, Category = "NarrRail|Runtime")
    FNarrRailRuntimeResult Start(FName OverrideEntryNodeId = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "NarrRail|Runtime")
    FNarrRailRuntimeResult Next();

    UFUNCTION(BlueprintCallable, Category = "NarrRail|Runtime")
    FNarrRailRuntimeResult Choose(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "NarrRail|Runtime")
    FNarrRailRuntimeResult Pause();

    UFUNCTION(BlueprintCallable, Category = "NarrRail|Runtime")
    FNarrRailRuntimeResult Resume();

    UFUNCTION(BlueprintCallable, Category = "NarrRail|Runtime")
    FNarrRailRuntimeResult Stop();

    UFUNCTION(BlueprintPure, Category = "NarrRail|Runtime")
    ENarrRailSessionState GetSessionState() const
    {
        return SessionState;
    }

    UFUNCTION(BlueprintPure, Category = "NarrRail|Runtime")
    FName GetCurrentNodeId() const
    {
        return Context.CurrentNodeId;
    }

    UFUNCTION(BlueprintPure, Category = "NarrRail|Runtime")
    bool GetCurrentNode(FNarrRailNode& OutNode) const;

    UFUNCTION(BlueprintPure, Category = "NarrRail|Runtime")
    TArray<FNarrRailChoiceOption> GetCurrentChoices() const;

    UFUNCTION(BlueprintPure, Category = "NarrRail|Runtime")
    const TArray<FName>& GetHistory() const
    {
        return Context.NodeHistory;
    }

    UFUNCTION(BlueprintPure, Category = "NarrRail|Runtime")
    FNarrRailSessionContext GetSessionContext() const
    {
        return Context;
    }

    // 获取最后一次选择的信息
    UFUNCTION(BlueprintPure, Category = "NarrRail|Runtime")
    FNarrRailLastChoiceInfo GetLastChoice() const
    {
        return LastChoiceInfo;
    }

    // 获取变量容器（用于直接访问变量系统）
    UFUNCTION(BlueprintPure, Category = "NarrRail|Runtime")
    UNarrRailVariableContainer* GetVariableContainer() const
    {
        return VariableContainer;
    }

    // 便捷的变量访问接口
    UFUNCTION(BlueprintPure, Category = "NarrRail|Runtime")
    FNarrRailVariableResult GetVariableBool(FName VariableName, bool& OutValue) const;

    UFUNCTION(BlueprintPure, Category = "NarrRail|Runtime")
    FNarrRailVariableResult GetVariableInt(FName VariableName, int32& OutValue) const;

    UFUNCTION(BlueprintPure, Category = "NarrRail|Runtime")
    FNarrRailVariableResult GetVariableFloat(FName VariableName, float& OutValue) const;

    UFUNCTION(BlueprintPure, Category = "NarrRail|Runtime")
    FNarrRailVariableResult GetVariableString(FName VariableName, FString& OutValue) const;

    UFUNCTION(BlueprintCallable, Category = "NarrRail|Runtime")
    FNarrRailVariableResult SetVariableBool(FName VariableName, bool Value);

    UFUNCTION(BlueprintCallable, Category = "NarrRail|Runtime")
    FNarrRailVariableResult SetVariableInt(FName VariableName, int32 Value);

    UFUNCTION(BlueprintCallable, Category = "NarrRail|Runtime")
    FNarrRailVariableResult SetVariableFloat(FName VariableName, float Value);

    UFUNCTION(BlueprintCallable, Category = "NarrRail|Runtime")
    FNarrRailVariableResult SetVariableString(FName VariableName, const FString& Value);

    // === 运行时事件委托 ===

    // 会话启动事件
    UPROPERTY(BlueprintAssignable, Category = "NarrRail|Events")
    FNarrRailSessionStartedDelegate OnSessionStarted;

    // 节点进入事件
    UPROPERTY(BlueprintAssignable, Category = "NarrRail|Events")
    FNarrRailNodeEnteredDelegate OnNodeEntered;

    // 节点退出事件
    UPROPERTY(BlueprintAssignable, Category = "NarrRail|Events")
    FNarrRailNodeExitedDelegate OnNodeExited;

    // 会话结束事件
    UPROPERTY(BlueprintAssignable, Category = "NarrRail|Events")
    FNarrRailSessionEndedDelegate OnSessionEnded;

    // 选项准备就绪事件
    UPROPERTY(BlueprintAssignable, Category = "NarrRail|Events")
    FNarrRailChoicesReadyDelegate OnChoicesReady;

    // 选项被选择事件
    UPROPERTY(BlueprintAssignable, Category = "NarrRail|Events")
    FNarrRailChoiceSelectedDelegate OnChoiceSelected;

private:
    void ResetSessionContextFromAsset();
    FString MakeDefaultVariableValue(ENarrRailVariableType VariableType) const;
    FNarrRailRuntimeResult AdvanceToNode(FName TargetNodeId);
    FNarrRailRuntimeResult ResolveNextByEdge(const FNarrRailNode& FromNode, FName& OutNextNodeId) const;
    const FNarrRailNode* FindNode(FName NodeId) const;

    bool EvaluateConditionExpression(const FNarrRailConditionExpression& Condition) const;
    bool EvaluateConditionTerm(const FNarrRailConditionTerm& Term) const;
    bool ExecuteActions(const TArray<FNarrRailNodeAction>& Actions, FString& OutErrorMessage);

private:
    UPROPERTY(Transient)
    TObjectPtr<const UNarrRailStoryAsset> StoryAsset;

    UPROPERTY(Transient)
    ENarrRailSessionState SessionState = ENarrRailSessionState::Idle;

    UPROPERTY(Transient)
    ENarrRailSessionState StateBeforePause = ENarrRailSessionState::Idle;

    UPROPERTY(Transient)
    FNarrRailSessionContext Context;

    UPROPERTY(Transient)
    TObjectPtr<UNarrRailVariableContainer> VariableContainer;

    UPROPERTY(Transient)
    FNarrRailLastChoiceInfo LastChoiceInfo;
};
