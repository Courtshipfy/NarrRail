#include "Runtime/NarrRailStoryValidator.h"

namespace NarrRailValidation
{
static void AddIssue(
    TArray<FNarrRailValidationIssue>& OutIssues,
    const ENarrRailValidationSeverity Severity,
    const FName NodeId,
    const FString& Message)
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

static void ValidateConditionTerms(
    TArray<FNarrRailValidationIssue>& OutIssues,
    const FName NodeId,
    const TArray<FNarrRailConditionTerm>& Terms,
    const TSet<FName>& Variables)
{
    for (const FNarrRailConditionTerm& Term : Terms)
    {
        if (Term.Variable.VariableName == NAME_None)
        {
            AddIssue(OutIssues, ENarrRailValidationSeverity::Error, NodeId, TEXT("Condition term has empty variable name."));
            continue;
        }

        if (!Variables.Contains(Term.Variable.VariableName))
        {
            AddIssue(
                OutIssues,
                ENarrRailValidationSeverity::Error,
                NodeId,
                FString::Printf(TEXT("Condition term references unknown variable '%s'."), *Term.Variable.VariableName.ToString()));
        }
    }
}

static bool IsModernConditionHandle(const FString& Handle)
{
    if (Handle == TEXT("condition-fallback"))
    {
        return true;
    }

    if (!Handle.StartsWith(TEXT("condition-")))
    {
        return false;
    }

    const FString IndexText = Handle.RightChop(10);
    if (IndexText.IsEmpty())
    {
        return false;
    }

    for (int32 Index = 0; Index < IndexText.Len(); ++Index)
    {
        if (!FChar::IsDigit(IndexText[Index]))
        {
            return false;
        }
    }

    return true;
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
    for (const FNarrRailVariableDefinition& Var : StoryAsset->Variables)
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
    TMap<FName, TArray<const FNarrRailNodeEdge*>> EdgesBySourceNode;

    for (const FNarrRailNodeEdge& Edge : StoryAsset->Edges)
    {
        EdgesBySourceNode.FindOrAdd(Edge.SourceNodeId).Add(&Edge);

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

        if (Node.NodeType == ENarrRailNodeType::Choice && Node.ChoiceMode == ENarrRailChoiceMode::ExhaustiveUntilComplete)
        {
            if (Node.ChoiceCompletionTargetNodeId == NAME_None)
            {
                NarrRailValidation::AddIssue(
                    Issues,
                    ENarrRailValidationSeverity::Error,
                    Node.NodeId,
                    TEXT("Exhaustive choice node requires ChoiceCompletionTargetNodeId."));
            }
            else if (!NarrRailValidation::ContainsNode(NodeIds, Node.ChoiceCompletionTargetNodeId))
            {
                NarrRailValidation::AddIssue(
                    Issues,
                    ENarrRailValidationSeverity::Error,
                    Node.NodeId,
                    TEXT("Exhaustive choice completion target node does not exist."));
            }
            else
            {
                ReferencedTargets.Add(Node.ChoiceCompletionTargetNodeId);
            }
        }

        if (Node.NodeType == ENarrRailNodeType::Condition)
        {
            const TArray<const FNarrRailNodeEdge*>* OutgoingEdges = EdgesBySourceNode.Find(Node.NodeId);
            TMap<FName, int32> HandleCounts;

            if (OutgoingEdges != nullptr)
            {
                for (const FNarrRailNodeEdge* Edge : *OutgoingEdges)
                {
                    if (Edge == nullptr)
                    {
                        continue;
                    }

                    if (Edge->SourceHandle == NAME_None)
                    {
                        NarrRailValidation::AddIssue(
                            Issues,
                            ENarrRailValidationSeverity::Error,
                            Node.NodeId,
                            TEXT("Condition outgoing edge requires a source handle."));
                        continue;
                    }

                    HandleCounts.FindOrAdd(Edge->SourceHandle) += 1;

                    const FString HandleText = Edge->SourceHandle.ToString();
                    if (HandleText == TEXT("condition-true") || HandleText == TEXT("condition-false"))
                    {
                        NarrRailValidation::AddIssue(
                            Issues,
                            ENarrRailValidationSeverity::Warning,
                            Node.NodeId,
                            TEXT("Condition node uses deprecated condition-true/condition-false handles."));
                    }
                    else if (!NarrRailValidation::IsModernConditionHandle(HandleText))
                    {
                        NarrRailValidation::AddIssue(
                            Issues,
                            ENarrRailValidationSeverity::Error,
                            Node.NodeId,
                            FString::Printf(TEXT("Condition node has invalid source handle '%s'."), *HandleText));
                    }
                    else if (HandleText != TEXT("condition-fallback") && Node.Condition.Branches.Num() > 0)
                    {
                        const int32 BranchIndex = FCString::Atoi(*HandleText.RightChop(10));
                        if (!Node.Condition.Branches.IsValidIndex(BranchIndex))
                        {
                            NarrRailValidation::AddIssue(
                                Issues,
                                ENarrRailValidationSeverity::Error,
                                Node.NodeId,
                                FString::Printf(TEXT("Condition source handle '%s' has no matching branch."), *HandleText));
                        }
                    }
                }
            }

            for (const TPair<FName, int32>& Pair : HandleCounts)
            {
                if (Pair.Value > 1)
                {
                    NarrRailValidation::AddIssue(
                        Issues,
                        ENarrRailValidationSeverity::Error,
                        Node.NodeId,
                        FString::Printf(TEXT("Condition node has duplicate outgoing handle '%s'."), *Pair.Key.ToString()));
                }
            }

            if (Node.Condition.Branches.Num() > 0)
            {
                for (int32 BranchIndex = 0; BranchIndex < Node.Condition.Branches.Num(); ++BranchIndex)
                {
                    const FNarrRailConditionBranch& Branch = Node.Condition.Branches[BranchIndex];
                    NarrRailValidation::ValidateConditionTerms(Issues, Node.NodeId, Branch.Terms, Variables);

                    const FName RequiredHandle(*FString::Printf(TEXT("condition-%d"), BranchIndex));
                    const bool bHasRequiredHandle = HandleCounts.Contains(RequiredHandle) ||
                        (BranchIndex == 0 && HandleCounts.Contains(FName(TEXT("condition-true"))));

                    if (!bHasRequiredHandle)
                    {
                        NarrRailValidation::AddIssue(
                            Issues,
                            ENarrRailValidationSeverity::Error,
                            Node.NodeId,
                            FString::Printf(TEXT("Condition branch %d is missing outgoing handle '%s'."), BranchIndex, *RequiredHandle.ToString()));
                    }
                }

                if (!HandleCounts.Contains(FName(TEXT("condition-fallback"))) &&
                    !HandleCounts.Contains(FName(TEXT("condition-false"))))
                {
                    NarrRailValidation::AddIssue(
                        Issues,
                        ENarrRailValidationSeverity::Warning,
                        Node.NodeId,
                        TEXT("Condition node has no fallback edge; runtime will error if no branch matches."));
                }
            }
            else
            {
                NarrRailValidation::ValidateConditionTerms(Issues, Node.NodeId, Node.Condition.Terms, Variables);

                if (!HandleCounts.Contains(FName(TEXT("condition-0"))) &&
                    !HandleCounts.Contains(FName(TEXT("condition-true"))))
                {
                    NarrRailValidation::AddIssue(
                        Issues,
                        ENarrRailValidationSeverity::Error,
                        Node.NodeId,
                        TEXT("Legacy Condition node is missing condition-0 or condition-true outgoing edge."));
                }

                if (!HandleCounts.Contains(FName(TEXT("condition-fallback"))) &&
                    !HandleCounts.Contains(FName(TEXT("condition-false"))))
                {
                    NarrRailValidation::AddIssue(
                        Issues,
                        ENarrRailValidationSeverity::Warning,
                        Node.NodeId,
                        TEXT("Legacy Condition node has no fallback/false edge; runtime will error when the condition is false."));
                }
            }
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
