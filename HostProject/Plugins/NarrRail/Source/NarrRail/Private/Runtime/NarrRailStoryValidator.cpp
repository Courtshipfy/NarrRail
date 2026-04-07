#include "Runtime/NarrRailStoryValidator.h"

namespace NarrRailValidation
{
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

    if (StoryAsset->SchemaVersion <= 0)
    {
        NarrRailValidation::AddIssue(Issues, ENarrRailValidationSeverity::Error, NAME_None, TEXT("SchemaVersion must be greater than zero."));
    }

    TSet<FName> Variables;
    for (const FNarrRailVariableRef& Var : StoryAsset->Variables)
    {
        if (Var.VariableName == NAME_None)
        {
            NarrRailValidation::AddIssue(Issues, ENarrRailValidationSeverity::Error, NAME_None, TEXT("Variable definition has empty name."));
            continue;
        }

        if (Variables.Contains(Var.VariableName))
        {
            NarrRailValidation::AddIssue(Issues, ENarrRailValidationSeverity::Error, Var.VariableName, TEXT("Duplicate variable name detected."));
            continue;
        }

        Variables.Add(Var.VariableName);
    }

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

    if (!NarrRailValidation::ContainsNode(NodeIds, StoryAsset->EntryNodeId))
    {
        NarrRailValidation::AddIssue(Issues, ENarrRailValidationSeverity::Error, StoryAsset->EntryNodeId, TEXT("EntryNodeId does not exist."));
    }

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
