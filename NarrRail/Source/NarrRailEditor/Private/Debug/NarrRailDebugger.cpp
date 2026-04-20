#include "Debug/NarrRailDebugger.h"
#include "NarrRailEditorModule.h"
#include "Runtime/NarrRailStoryAsset.h"
#include "Runtime/NarrRailVariableContainer.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogNarrRailDebug);

const UNarrRailStorySession* UNarrRailDebugger::FindUniqueSession(const UNarrRailStorySession* ProvidedSession)
{
    // 如果提供了会话，直接使用
    if (ProvidedSession != nullptr)
    {
        return ProvidedSession;
    }

    // 自动查找活跃会话
    TArray<UNarrRailStorySession*> ActiveSessions = UNarrRailStorySession::GetAllActiveSessions();

    if (ActiveSessions.Num() == 0)
    {
        UE_LOG(LogNarrRailDebug, Warning, TEXT("[NarrRail Debug] No active sessions found."));
        return nullptr;
    }

    if (ActiveSessions.Num() > 1)
    {
        UE_LOG(LogNarrRailDebug, Error, TEXT("[NarrRail Debug] MULTIPLE SESSIONS DETECTED (%d sessions)! This is not supported by the debugger."), ActiveSessions.Num());
        UE_LOG(LogNarrRailDebug, Error, TEXT("[NarrRail Debug] Active sessions:"));
        for (int32 i = 0; i < ActiveSessions.Num(); ++i)
        {
            FString DebugName = ActiveSessions[i]->GetDebugName();
            FString SessionInfo = DebugName.IsEmpty() ? FString::Printf(TEXT("Session %d (unnamed)"), i) : FString::Printf(TEXT("Session %d: %s"), i, *DebugName);
            UE_LOG(LogNarrRailDebug, Error, TEXT("  - %s"), *SessionInfo);
        }
        UE_LOG(LogNarrRailDebug, Error, TEXT("[NarrRail Debug] Please use SetDebugName() to name your sessions, or pass the session explicitly to debug functions."));
        return nullptr;
    }

    // 找到唯一会话
    UE_LOG(LogNarrRailDebug, Log, TEXT("[NarrRail Debug] Auto-detected session: %s"),
        ActiveSessions[0]->GetDebugName().IsEmpty() ? TEXT("(unnamed)") : *ActiveSessions[0]->GetDebugName());
    return ActiveSessions[0];
}

void UNarrRailDebugger::PrintSessionState(const UNarrRailStorySession* Session, bool bVerbose)
{
    Session = FindUniqueSession(Session);
    if (Session == nullptr)
    {
        return;
    }

    PrintSeparator(TEXT("SESSION STATE"));

    UE_LOG(LogNarrRailDebug, Log, TEXT("State: %s"), *GetSessionStateString(Session->GetSessionState()));
    UE_LOG(LogNarrRailDebug, Log, TEXT("Current Node: %s"), *Session->GetCurrentNodeId().ToString());

    const TArray<FName>& History = Session->GetHistory();
    UE_LOG(LogNarrRailDebug, Log, TEXT("History Count: %d"), History.Num());

    if (bVerbose)
    {
        PrintCurrentNode(Session);
        PrintAllVariables(Session);
    }

    PrintSeparator();
}

void UNarrRailDebugger::PrintCurrentNode(const UNarrRailStorySession* Session)
{
    Session = FindUniqueSession(Session);
    if (Session == nullptr)
    {
        return;
    }

    FNarrRailNode CurrentNode;
    if (!Session->GetCurrentNode(CurrentNode))
    {
        UE_LOG(LogNarrRailDebug, Warning, TEXT("[NarrRail Debug] No current node."));
        return;
    }

    PrintSeparator(TEXT("CURRENT NODE"));

    UE_LOG(LogNarrRailDebug, Log, TEXT("Node ID: %s"), *CurrentNode.NodeId.ToString());
    UE_LOG(LogNarrRailDebug, Log, TEXT("Node Type: %s"), *GetNodeTypeString(CurrentNode.NodeType));

    if (CurrentNode.NodeType == ENarrRailNodeType::Dialogue)
    {
        UE_LOG(LogNarrRailDebug, Log, TEXT("  Speaker: %s"), *CurrentNode.Dialogue.SpeakerId.ToString());
        UE_LOG(LogNarrRailDebug, Log, TEXT("  Text: %s"), *CurrentNode.Dialogue.TextKey);
    }
    else if (CurrentNode.NodeType == ENarrRailNodeType::Choice)
    {
        UE_LOG(LogNarrRailDebug, Log, TEXT("  Choices: %d"), CurrentNode.Choices.Num());
        for (int32 i = 0; i < CurrentNode.Choices.Num(); ++i)
        {
            UE_LOG(LogNarrRailDebug, Log, TEXT("    [%d] %s -> %s"), i, *CurrentNode.Choices[i].TextKey, *CurrentNode.Choices[i].TargetNodeId.ToString());
        }
    }
    else if (CurrentNode.NodeType == ENarrRailNodeType::Jump)
    {
        UE_LOG(LogNarrRailDebug, Log, TEXT("  Jump Target: %s"), *CurrentNode.JumpTargetNodeId.ToString());
    }

    if (CurrentNode.EnterActions.Num() > 0)
    {
        UE_LOG(LogNarrRailDebug, Log, TEXT("  Enter Actions: %d"), CurrentNode.EnterActions.Num());
        for (const FNarrRailNodeAction& Action : CurrentNode.EnterActions)
        {
            UE_LOG(LogNarrRailDebug, Log, TEXT("    %s %s = %s"),
                *GetActionTypeString(Action.ActionType),
                *Action.Variable.VariableName.ToString(),
                *Action.Value);
        }
    }

    if (CurrentNode.ExitActions.Num() > 0)
    {
        UE_LOG(LogNarrRailDebug, Log, TEXT("  Exit Actions: %d"), CurrentNode.ExitActions.Num());
    }

    PrintSeparator();
}

void UNarrRailDebugger::PrintAllVariables(const UNarrRailStorySession* Session)
{
    Session = FindUniqueSession(Session);
    if (Session == nullptr)
    {
        return;
    }

    UNarrRailVariableContainer* VarContainer = Session->GetVariableContainer();
    if (VarContainer == nullptr)
    {
        UE_LOG(LogNarrRailDebug, Warning, TEXT("[NarrRail Debug] Variable container is null."));
        return;
    }

    PrintSeparator(TEXT("VARIABLES"));

    TMap<FName, FString> Snapshot = VarContainer->GetSnapshot();
    if (Snapshot.Num() == 0)
    {
        UE_LOG(LogNarrRailDebug, Log, TEXT("  (No variables defined)"));
    }
    else
    {
        for (const auto& Pair : Snapshot)
        {
            UE_LOG(LogNarrRailDebug, Log, TEXT("  %s = %s"), *Pair.Key.ToString(), *Pair.Value);
        }
    }

    PrintSeparator();
}

void UNarrRailDebugger::PrintNodeHistory(const UNarrRailStorySession* Session, int32 MaxCount)
{
    Session = FindUniqueSession(Session);
    if (Session == nullptr)
    {
        return;
    }

    const TArray<FName>& History = Session->GetHistory();

    PrintSeparator(TEXT("NODE HISTORY"));

    if (History.Num() == 0)
    {
        UE_LOG(LogNarrRailDebug, Log, TEXT("  (No history)"));
    }
    else
    {
        int32 StartIndex = (MaxCount > 0 && History.Num() > MaxCount) ? (History.Num() - MaxCount) : 0;
        for (int32 i = StartIndex; i < History.Num(); ++i)
        {
            UE_LOG(LogNarrRailDebug, Log, TEXT("  [%d] %s"), i, *History[i].ToString());
        }
    }

    PrintSeparator();
}

void UNarrRailDebugger::PrintCurrentChoices(const UNarrRailStorySession* Session)
{
    Session = FindUniqueSession(Session);
    if (Session == nullptr)
    {
        return;
    }

    TArray<FNarrRailChoiceOption> Choices = Session->GetCurrentChoices();

    PrintSeparator(TEXT("CURRENT CHOICES"));

    if (Choices.Num() == 0)
    {
        UE_LOG(LogNarrRailDebug, Log, TEXT("  (No choices available)"));
    }
    else
    {
        for (int32 i = 0; i < Choices.Num(); ++i)
        {
            UE_LOG(LogNarrRailDebug, Log, TEXT("  [%d] %s -> %s"), i, *Choices[i].TextKey, *Choices[i].TargetNodeId.ToString());
        }
    }

    PrintSeparator();
}

void UNarrRailDebugger::PrintLastChoice(const UNarrRailStorySession* Session)
{
    Session = FindUniqueSession(Session);
    if (Session == nullptr)
    {
        return;
    }

    FNarrRailLastChoiceInfo LastChoice = Session->GetLastChoice();

    PrintSeparator(TEXT("LAST CHOICE"));

    if (!LastChoice.bValid)
    {
        UE_LOG(LogNarrRailDebug, Log, TEXT("  (No choice made yet)"));
    }
    else
    {
        UE_LOG(LogNarrRailDebug, Log, TEXT("  Choice Node: %s"), *LastChoice.ChoiceNodeId.ToString());
        UE_LOG(LogNarrRailDebug, Log, TEXT("  Choice Index: %d"), LastChoice.ChoiceIndex);
        UE_LOG(LogNarrRailDebug, Log, TEXT("  Choice Text: %s"), *LastChoice.ChoiceTextKey);
        UE_LOG(LogNarrRailDebug, Log, TEXT("  Target Node: %s"), *LastChoice.TargetNodeId.ToString());
    }

    PrintSeparator();
}

void UNarrRailDebugger::PrintStoryStructure(const UNarrRailStoryAsset* StoryAsset)
{
    if (StoryAsset == nullptr)
    {
        UE_LOG(LogNarrRailDebug, Warning, TEXT("[NarrRail Debug] Story asset is null."));
        return;
    }

    PrintSeparator(TEXT("STORY STRUCTURE"));

    UE_LOG(LogNarrRailDebug, Log, TEXT("Story ID: %s"), *StoryAsset->StoryId.ToString());
    UE_LOG(LogNarrRailDebug, Log, TEXT("Entry Node: %s"), *StoryAsset->EntryNodeId.ToString());
    UE_LOG(LogNarrRailDebug, Log, TEXT("Schema Version: %d"), StoryAsset->SchemaVersion);
    UE_LOG(LogNarrRailDebug, Log, TEXT("Variables: %d"), StoryAsset->Variables.Num());
    UE_LOG(LogNarrRailDebug, Log, TEXT("Nodes: %d"), StoryAsset->Nodes.Num());
    UE_LOG(LogNarrRailDebug, Log, TEXT("Edges: %d"), StoryAsset->Edges.Num());

    // 打印变量定义
    if (StoryAsset->Variables.Num() > 0)
    {
        UE_LOG(LogNarrRailDebug, Log, TEXT("\nVariables:"));
        for (const FNarrRailVariableDefinition& Var : StoryAsset->Variables)
        {
            UE_LOG(LogNarrRailDebug, Log, TEXT("  %s (%s, %s) = %s"),
                *Var.VariableName.ToString(),
                *GetVariableTypeString(Var.VariableType),
                Var.bGlobalScope ? TEXT("Global") : TEXT("Session"),
                *Var.DefaultValue);
        }
    }

    // 打印节点列表
    if (StoryAsset->Nodes.Num() > 0)
    {
        UE_LOG(LogNarrRailDebug, Log, TEXT("\nNodes:"));
        for (const FNarrRailNode& Node : StoryAsset->Nodes)
        {
            UE_LOG(LogNarrRailDebug, Log, TEXT("  %s (%s)"), *Node.NodeId.ToString(), *GetNodeTypeString(Node.NodeType));
        }
    }

    // 打印边列表
    if (StoryAsset->Edges.Num() > 0)
    {
        UE_LOG(LogNarrRailDebug, Log, TEXT("\nEdges:"));
        for (const FNarrRailNodeEdge& Edge : StoryAsset->Edges)
        {
            FString ConditionStr = (Edge.Condition.Terms.Num() > 0) ? FString::Printf(TEXT(" [%d conditions]"), Edge.Condition.Terms.Num()) : TEXT("");
            UE_LOG(LogNarrRailDebug, Log, TEXT("  %s -> %s (Priority: %d)%s"),
                *Edge.SourceNodeId.ToString(),
                *Edge.TargetNodeId.ToString(),
                Edge.Priority,
                *ConditionStr);
        }
    }

    PrintSeparator();
}

void UNarrRailDebugger::ShowDebugOnScreen(UObject* WorldContextObject, const UNarrRailStorySession* Session, float DisplayTime)
{
    Session = FindUniqueSession(Session);
    if (Session == nullptr || WorldContextObject == nullptr)
    {
        return;
    }

    FString DebugText;

    // 会话状态
    DebugText += FString::Printf(TEXT("State: %s\n"), *GetSessionStateString(Session->GetSessionState()));
    DebugText += FString::Printf(TEXT("Current Node: %s\n"), *Session->GetCurrentNodeId().ToString());

    // 当前节点信息
    FNarrRailNode CurrentNode;
    if (Session->GetCurrentNode(CurrentNode))
    {
        DebugText += FString::Printf(TEXT("Node Type: %s\n"), *GetNodeTypeString(CurrentNode.NodeType));

        if (CurrentNode.NodeType == ENarrRailNodeType::Dialogue)
        {
            DebugText += FString::Printf(TEXT("Speaker: %s\n"), *CurrentNode.Dialogue.SpeakerId.ToString());
            DebugText += FString::Printf(TEXT("Text: %s\n"), *CurrentNode.Dialogue.TextKey);
        }
    }

    // 变量
    UNarrRailVariableContainer* VarContainer = Session->GetVariableContainer();
    if (VarContainer)
    {
        TMap<FName, FString> Snapshot = VarContainer->GetSnapshot();
        if (Snapshot.Num() > 0)
        {
            DebugText += TEXT("\nVariables:\n");
            for (const auto& Pair : Snapshot)
            {
                DebugText += FString::Printf(TEXT("  %s = %s\n"), *Pair.Key.ToString(), *Pair.Value);
            }
        }
    }

    // 显示到屏幕
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, DisplayTime, FColor::Cyan, DebugText);
    }
}

void UNarrRailDebugger::PrintFullDebugInfo(const UNarrRailStorySession* Session)
{
    Session = FindUniqueSession(Session);
    if (Session == nullptr)
    {
        return;
    }

    PrintSessionState(Session, false);
    PrintCurrentNode(Session);
    PrintAllVariables(Session);
    PrintNodeHistory(Session, 10);
    PrintLastChoice(Session);
}

void UNarrRailDebugger::PrintAllActiveSessions()
{
    TArray<UNarrRailStorySession*> ActiveSessions = UNarrRailStorySession::GetAllActiveSessions();

    PrintSeparator(TEXT("ACTIVE SESSIONS"));

    if (ActiveSessions.Num() == 0)
    {
        UE_LOG(LogNarrRailDebug, Log, TEXT("  (No active sessions)"));
    }
    else
    {
        UE_LOG(LogNarrRailDebug, Log, TEXT("  Total: %d session(s)"), ActiveSessions.Num());
        for (int32 i = 0; i < ActiveSessions.Num(); ++i)
        {
            FString DebugName = ActiveSessions[i]->GetDebugName();
            FString SessionInfo = DebugName.IsEmpty() ? TEXT("(unnamed)") : DebugName;
            UE_LOG(LogNarrRailDebug, Log, TEXT("  [%d] %s - State: %s, Node: %s"),
                i,
                *SessionInfo,
                *GetSessionStateString(ActiveSessions[i]->GetSessionState()),
                *ActiveSessions[i]->GetCurrentNodeId().ToString());
        }
    }

    PrintSeparator();
}

// === Helper Functions ===

FString UNarrRailDebugger::GetSessionStateString(ENarrRailSessionState State)
{
    switch (State)
    {
        case ENarrRailSessionState::Idle: return TEXT("Idle");
        case ENarrRailSessionState::Running: return TEXT("Running");
        case ENarrRailSessionState::WaitingForChoice: return TEXT("WaitingForChoice");
        case ENarrRailSessionState::Paused: return TEXT("Paused");
        case ENarrRailSessionState::Completed: return TEXT("Completed");
        case ENarrRailSessionState::Error: return TEXT("Error");
        default: return TEXT("Unknown");
    }
}

FString UNarrRailDebugger::GetNodeTypeString(ENarrRailNodeType NodeType)
{
    switch (NodeType)
    {
        case ENarrRailNodeType::Dialogue: return TEXT("Dialogue");
        case ENarrRailNodeType::Choice: return TEXT("Choice");
        case ENarrRailNodeType::Jump: return TEXT("Jump");
        case ENarrRailNodeType::SetVariable: return TEXT("SetVariable");
        case ENarrRailNodeType::EmitEvent: return TEXT("EmitEvent");
        case ENarrRailNodeType::End: return TEXT("End");
        default: return TEXT("Unknown");
    }
}

FString UNarrRailDebugger::GetVariableTypeString(ENarrRailVariableType VarType)
{
    switch (VarType)
    {
        case ENarrRailVariableType::Bool: return TEXT("Bool");
        case ENarrRailVariableType::Int: return TEXT("Int");
        case ENarrRailVariableType::Float: return TEXT("Float");
        case ENarrRailVariableType::String: return TEXT("String");
        default: return TEXT("Unknown");
    }
}

FString UNarrRailDebugger::GetComparisonOpString(ENarrRailComparisonOp Op)
{
    switch (Op)
    {
        case ENarrRailComparisonOp::Equal: return TEXT("==");
        case ENarrRailComparisonOp::NotEqual: return TEXT("!=");
        case ENarrRailComparisonOp::Greater: return TEXT(">");
        case ENarrRailComparisonOp::GreaterOrEqual: return TEXT(">=");
        case ENarrRailComparisonOp::Less: return TEXT("<");
        case ENarrRailComparisonOp::LessOrEqual: return TEXT("<=");
        default: return TEXT("?");
    }
}

FString UNarrRailDebugger::GetActionTypeString(ENarrRailActionType ActionType)
{
    switch (ActionType)
    {
        case ENarrRailActionType::Set: return TEXT("Set");
        case ENarrRailActionType::Add: return TEXT("Add");
        case ENarrRailActionType::Subtract: return TEXT("Subtract");
        case ENarrRailActionType::EmitEvent: return TEXT("EmitEvent");
        default: return TEXT("Unknown");
    }
}

void UNarrRailDebugger::PrintSeparator(const FString& Title)
{
    if (Title.IsEmpty())
    {
        UE_LOG(LogNarrRailDebug, Log, TEXT("========================================"));
    }
    else
    {
        UE_LOG(LogNarrRailDebug, Log, TEXT("======== %s ========"), *Title);
    }
}
