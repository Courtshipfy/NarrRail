#include "Runtime/NarrRailStoryValidator.h"

namespace NarrRailValidation
{
// 统一追加校验问题，避免重复样板代码。
static void AddIssue(
    TArray<FNarrRailValidationIssue>& OutIssues,
    const ENarrRailValidationSeverity Severity,
    const FName NodeId,
    const TCHAR* Message)
{
    FNarrRailValidationIssue& Issue = OutIssues.AddDefaulted_GetRef();
    Issue.Severity = Severity;
    Issue.NodeId = NodeId;
    Issue.Message = Message;
}

// 检查节点 ID 是否有效且存在于集合中。
static bool ContainsNode(const TSet<FName>& NodeIds, const FName NodeId)
{
    return NodeId != NAME_None && NodeIds.Contains(NodeId);
}
}

TArray<FNarrRailValidationIssue> UNarrRailStoryValidator::ValidateStoryAsset(const UNarrRailStoryAsset* StoryAsset)
{
    TArray<FNarrRailValidationIssue> Issues;

    if (StoryAsset == nullptr)
    {
        NarrRailValidation::AddIssue(Issues, ENarrRailValidationSeverity::Error, NAME_None, TEXT("Story asset is null."));
        return Issues;
    }

    // 第一阶段：收集节点并检测空 ID / 重复 ID。
    TSet<FName> NodeIds;
    NodeIds.Reserve(StoryAsset->Nodes.Num());

    for (const FNarrRailNode& Node : StoryAsset->Nodes)
    {
        if (Node.NodeId == NAME_None)
        {
            NarrRailValidation::AddIssue(Issues, ENarrRailValidationSeverity::Error, NAME_None, TEXT("Node has empty NodeId."));
            continue;
        }

        if (NodeIds.Contains(Node.NodeId))
        {
            NarrRailValidation::AddIssue(Issues, ENarrRailValidationSeverity::Error, Node.NodeId, TEXT("Duplicate NodeId detected."));
            continue;
        }

        NodeIds.Add(Node.NodeId);
    }

    // 第二阶段：入口节点合法性检查。
    if (!NarrRailValidation::ContainsNode(NodeIds, StoryAsset->EntryNodeId))
    {
        NarrRailValidation::AddIssue(Issues, ENarrRailValidationSeverity::Error, StoryAsset->EntryNodeId, TEXT("EntryNodeId does not exist."));
    }

    // 第三阶段：检查显式边引用是否有效。
    TSet<FName> ReferencedTargets;

    for (const FNarrRailNodeEdge& Edge : StoryAsset->Edges)
    {
        const bool bValidSource = NarrRailValidation::ContainsNode(NodeIds, Edge.SourceNodeId);
        const bool bValidTarget = NarrRailValidation::ContainsNode(NodeIds, Edge.TargetNodeId);

        if (!bValidSource)
        {
            NarrRailValidation::AddIssue(Issues, ENarrRailValidationSeverity::Error, Edge.SourceNodeId, TEXT("Edge source node does not exist."));
        }

        if (!bValidTarget)
        {
            NarrRailValidation::AddIssue(Issues, ENarrRailValidationSeverity::Error, Edge.TargetNodeId, TEXT("Edge target node does not exist."));
        }

        if (bValidTarget)
        {
            ReferencedTargets.Add(Edge.TargetNodeId);
        }
    }

    // 第四阶段：检查节点内隐式引用（Jump / Choice）。
    for (const FNarrRailNode& Node : StoryAsset->Nodes)
    {
        if (Node.NodeType == ENarrRailNodeType::Jump && Node.JumpTargetNodeId != NAME_None)
        {
            if (!NarrRailValidation::ContainsNode(NodeIds, Node.JumpTargetNodeId))
            {
                NarrRailValidation::AddIssue(Issues, ENarrRailValidationSeverity::Error, Node.NodeId, TEXT("Jump target node does not exist."));
            }
            else
            {
                ReferencedTargets.Add(Node.JumpTargetNodeId);
            }
        }

        for (const FNarrRailChoiceOption& Option : Node.Choices)
        {
            if (Option.TargetNodeId == NAME_None)
            {
                NarrRailValidation::AddIssue(Issues, ENarrRailValidationSeverity::Error, Node.NodeId, TEXT("Choice option has empty target node."));
                continue;
            }

            if (!NarrRailValidation::ContainsNode(NodeIds, Option.TargetNodeId))
            {
                NarrRailValidation::AddIssue(Issues, ENarrRailValidationSeverity::Error, Node.NodeId, TEXT("Choice option target node does not exist."));
                continue;
            }

            ReferencedTargets.Add(Option.TargetNodeId);
        }
    }

    // 第五阶段：检测孤立节点（无入边，且不是入口）。
    for (const FNarrRailNode& Node : StoryAsset->Nodes)
    {
        if (Node.NodeId == StoryAsset->EntryNodeId)
        {
            continue;
        }

        if (!ReferencedTargets.Contains(Node.NodeId))
        {
            NarrRailValidation::AddIssue(Issues, ENarrRailValidationSeverity::Warning, Node.NodeId, TEXT("Node is isolated (no incoming references)."));
        }
    }

    return Issues;
}
