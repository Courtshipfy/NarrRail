#include "Runtime/NarrRailStorySession.h"

#include "Algo/Sort.h"

namespace NarrRailRuntime
{
static bool IsConditionSupportedAndTrue(const FNarrRailConditionExpression& Condition)
{
    // 当前阶段先支持“无条件”边；有条件表达式将在变量系统完成后接入求值。
    return Condition.Terms.Num() == 0;
}
}

FNarrRailRuntimeResult UNarrRailStorySession::Initialize(const UNarrRailStoryAsset* InStoryAsset)
{
    if (InStoryAsset == nullptr)
    {
        SessionState = ENarrRailSessionState::Error;
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidInput, TEXT("Story asset is null."));
    }

    StoryAsset = InStoryAsset;
    SessionState = ENarrRailSessionState::Idle;
    CurrentNodeId = NAME_None;
    NodeHistory.Reset();

    return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::Success, TEXT("Story session initialized."));
}

FNarrRailRuntimeResult UNarrRailStorySession::Start(const FName OverrideEntryNodeId)
{
    if (StoryAsset == nullptr)
    {
        SessionState = ENarrRailSessionState::Error;
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidState, TEXT("Session not initialized."));
    }

    const FName EntryNodeId = (OverrideEntryNodeId != NAME_None) ? OverrideEntryNodeId : StoryAsset->EntryNodeId;
    NodeHistory.Reset();

    return AdvanceToNode(EntryNodeId);
}

FNarrRailRuntimeResult UNarrRailStorySession::Next()
{
    if (StoryAsset == nullptr)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidState, TEXT("Session not initialized."));
    }

    if (SessionState == ENarrRailSessionState::Completed)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::Completed, TEXT("Session already completed."), CurrentNodeId);
    }

    if (SessionState == ENarrRailSessionState::WaitingForChoice)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidState, TEXT("Current node requires Choose()."), CurrentNodeId);
    }

    const FNarrRailNode* Node = FindNode(CurrentNodeId);
    if (Node == nullptr)
    {
        SessionState = ENarrRailSessionState::Error;
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::MissingNode, TEXT("Current node not found."), CurrentNodeId);
    }

    if (Node->NodeType == ENarrRailNodeType::End)
    {
        SessionState = ENarrRailSessionState::Completed;
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::Completed, TEXT("Reached end node."), CurrentNodeId);
    }

    if (Node->NodeType == ENarrRailNodeType::Choice)
    {
        SessionState = ENarrRailSessionState::WaitingForChoice;
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidState, TEXT("Current node requires Choose()."), CurrentNodeId);
    }

    if (Node->NodeType == ENarrRailNodeType::Jump)
    {
        if (Node->JumpTargetNodeId == NAME_None)
        {
            SessionState = ENarrRailSessionState::Error;
            return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidInput, TEXT("Jump node has empty target."), CurrentNodeId);
        }

        return AdvanceToNode(Node->JumpTargetNodeId);
    }

    FName NextNodeId = NAME_None;
    const FNarrRailRuntimeResult ResolveResult = ResolveNextByEdge(*Node, NextNodeId);
    if (ResolveResult.Code != ENarrRailRuntimeResultCode::Success)
    {
        if (ResolveResult.Code == ENarrRailRuntimeResultCode::Completed)
        {
            SessionState = ENarrRailSessionState::Completed;
        }
        return ResolveResult;
    }

    return AdvanceToNode(NextNodeId);
}

FNarrRailRuntimeResult UNarrRailStorySession::Choose(const int32 ChoiceIndex)
{
    if (StoryAsset == nullptr)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidState, TEXT("Session not initialized."));
    }

    const FNarrRailNode* Node = FindNode(CurrentNodeId);
    if (Node == nullptr)
    {
        SessionState = ENarrRailSessionState::Error;
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::MissingNode, TEXT("Current node not found."), CurrentNodeId);
    }

    if (Node->NodeType != ENarrRailNodeType::Choice)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidState, TEXT("Current node is not a choice node."), CurrentNodeId);
    }

    if (!Node->Choices.IsValidIndex(ChoiceIndex))
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidInput, TEXT("Choice index out of range."), CurrentNodeId);
    }

    const FNarrRailChoiceOption& Option = Node->Choices[ChoiceIndex];
    if (Option.TargetNodeId == NAME_None)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidInput, TEXT("Choice target node is empty."), CurrentNodeId);
    }

    return AdvanceToNode(Option.TargetNodeId);
}

FNarrRailRuntimeResult UNarrRailStorySession::Stop()
{
    if (StoryAsset == nullptr)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidState, TEXT("Session not initialized."));
    }

    SessionState = ENarrRailSessionState::Completed;
    return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::Completed, TEXT("Session stopped."), CurrentNodeId);
}

bool UNarrRailStorySession::GetCurrentNode(FNarrRailNode& OutNode) const
{
    const FNarrRailNode* Node = FindNode(CurrentNodeId);
    if (Node == nullptr)
    {
        return false;
    }

    OutNode = *Node;
    return true;
}

TArray<FNarrRailChoiceOption> UNarrRailStorySession::GetCurrentChoices() const
{
    const FNarrRailNode* Node = FindNode(CurrentNodeId);
    if (Node == nullptr || Node->NodeType != ENarrRailNodeType::Choice)
    {
        return {};
    }

    return Node->Choices;
}

FNarrRailRuntimeResult UNarrRailStorySession::AdvanceToNode(const FName TargetNodeId)
{
    const FNarrRailNode* Node = FindNode(TargetNodeId);
    if (Node == nullptr)
    {
        SessionState = ENarrRailSessionState::Error;
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::MissingNode, TEXT("Target node not found."), TargetNodeId);
    }

    CurrentNodeId = TargetNodeId;
    NodeHistory.Add(CurrentNodeId);

    if (Node->NodeType == ENarrRailNodeType::End)
    {
        SessionState = ENarrRailSessionState::Completed;
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::Completed, TEXT("Reached end node."), CurrentNodeId);
    }

    if (Node->NodeType == ENarrRailNodeType::Choice)
    {
        SessionState = ENarrRailSessionState::WaitingForChoice;
    }
    else
    {
        SessionState = ENarrRailSessionState::Running;
    }

    return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::Success, TEXT("Advanced to node."), CurrentNodeId);
}

FNarrRailRuntimeResult UNarrRailStorySession::ResolveNextByEdge(const FNarrRailNode& FromNode, FName& OutNextNodeId) const
{
    if (StoryAsset == nullptr)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidState, TEXT("Session not initialized."));
    }

    TArray<const FNarrRailNodeEdge*> CandidateEdges;
    for (const FNarrRailNodeEdge& Edge : StoryAsset->Edges)
    {
        if (Edge.SourceNodeId == FromNode.NodeId)
        {
            CandidateEdges.Add(&Edge);
        }
    }

    if (CandidateEdges.Num() == 0)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::Completed, TEXT("No outgoing edge. Session completed."), FromNode.NodeId);
    }

    Algo::SortBy(CandidateEdges, [](const FNarrRailNodeEdge* Edge) { return Edge->Priority; });

    for (const FNarrRailNodeEdge* Edge : CandidateEdges)
    {
        if (!NarrRailRuntime::IsConditionSupportedAndTrue(Edge->Condition))
        {
            continue;
        }

        OutNextNodeId = Edge->TargetNodeId;
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::Success, TEXT("Resolved next node by edge."), OutNextNodeId);
    }

    return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::Completed, TEXT("No satisfied outgoing edge. Session completed."), FromNode.NodeId);
}

const FNarrRailNode* UNarrRailStorySession::FindNode(const FName NodeId) const
{
    if (StoryAsset == nullptr || NodeId == NAME_None)
    {
        return nullptr;
    }

    for (const FNarrRailNode& Node : StoryAsset->Nodes)
    {
        if (Node.NodeId == NodeId)
        {
            return &Node;
        }
    }

    return nullptr;
}
