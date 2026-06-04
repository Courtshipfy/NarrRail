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

static FString NormalizeCompareText(const FString& Value)
{
    FString Out = Value;
    Out.TrimStartAndEndInline();
    Out.ToLowerInline();
    return Out;
}

static FString GetConditionTermSignature(const FNarrRailConditionTerm& Term)
{
    return FString::Printf(
        TEXT("%s:%d:%s"),
        *Term.Variable.VariableName.ToString(),
        static_cast<int32>(Term.Operator),
        *NormalizeCompareText(Term.CompareValue));
}

static FString GetConditionBranchSignature(const FNarrRailConditionBranch& Branch)
{
    TArray<FString> TermSignatures;
    TermSignatures.Reserve(Branch.Terms.Num());
    for (const FNarrRailConditionTerm& Term : Branch.Terms)
    {
        TermSignatures.Add(GetConditionTermSignature(Term));
    }
    TermSignatures.Sort();

    return FString::Printf(
        TEXT("%d|%s"),
        static_cast<int32>(Branch.Logic),
        *FString::Join(TermSignatures, TEXT("|")));
}

static bool TryParseNumericCompareValue(const FString& Text, double& OutValue)
{
    FString Trimmed = Text;
    Trimmed.TrimStartAndEndInline();
    if (!Trimmed.IsNumeric())
    {
        return false;
    }

    OutValue = FCString::Atod(*Trimmed);
    return true;
}

static FString FindNumericRangeConflict(const FName VariableName, const TArray<const FNarrRailConditionTerm*>& Terms)
{
    struct FBound
    {
        double Value = 0.0;
        bool bStrict = false;
        bool bSet = false;
    };

    FBound Lower;
    FBound Upper;

    for (const FNarrRailConditionTerm* Term : Terms)
    {
        if (Term == nullptr)
        {
            continue;
        }

        double Value = 0.0;
        if (!TryParseNumericCompareValue(Term->CompareValue, Value))
        {
            continue;
        }

        if (Term->Operator == ENarrRailComparisonOp::Greater || Term->Operator == ENarrRailComparisonOp::GreaterOrEqual)
        {
            const bool bStrict = Term->Operator == ENarrRailComparisonOp::Greater;
            if (!Lower.bSet || Value > Lower.Value || (Value == Lower.Value && bStrict))
            {
                Lower.Value = Value;
                Lower.bStrict = bStrict;
                Lower.bSet = true;
            }
        }
        else if (Term->Operator == ENarrRailComparisonOp::Less || Term->Operator == ENarrRailComparisonOp::LessOrEqual)
        {
            const bool bStrict = Term->Operator == ENarrRailComparisonOp::Less;
            if (!Upper.bSet || Value < Upper.Value || (Value == Upper.Value && bStrict))
            {
                Upper.Value = Value;
                Upper.bStrict = bStrict;
                Upper.bSet = true;
            }
        }
    }

    if (!Lower.bSet || !Upper.bSet)
    {
        return FString();
    }

    if (Lower.Value > Upper.Value || (Lower.Value == Upper.Value && (Lower.bStrict || Upper.bStrict)))
    {
        return FString::Printf(TEXT("%s numeric range is empty."), *VariableName.ToString());
    }

    return FString();
}

static FString FindSimpleConditionContradiction(const FNarrRailConditionBranch& Branch)
{
    if (Branch.Logic != ENarrRailConditionLogic::All)
    {
        return FString();
    }

    TMap<FName, TArray<const FNarrRailConditionTerm*>> TermsByVariable;
    for (const FNarrRailConditionTerm& Term : Branch.Terms)
    {
        if (Term.Variable.VariableName == NAME_None)
        {
            continue;
        }
        TermsByVariable.FindOrAdd(Term.Variable.VariableName).Add(&Term);
    }

    for (const TPair<FName, TArray<const FNarrRailConditionTerm*>>& Pair : TermsByVariable)
    {
        TSet<FString> EqualityValues;
        for (const FNarrRailConditionTerm* Term : Pair.Value)
        {
            if (Term == nullptr)
            {
                continue;
            }
            if (Term->Operator == ENarrRailComparisonOp::Equal)
            {
                EqualityValues.Add(NormalizeCompareText(Term->CompareValue));
            }
        }

        if (EqualityValues.Num() > 1)
        {
            return FString::Printf(TEXT("%s is required to equal multiple different values."), *Pair.Key.ToString());
        }

        if (EqualityValues.Num() == 1)
        {
            FString EqualValue;
            for (const FString& Value : EqualityValues)
            {
                EqualValue = Value;
                break;
            }

            for (const FNarrRailConditionTerm* Term : Pair.Value)
            {
                if (Term != nullptr &&
                    Term->Operator == ENarrRailComparisonOp::NotEqual &&
                    NormalizeCompareText(Term->CompareValue) == EqualValue)
                {
                    return FString::Printf(TEXT("%s is required to equal and not equal %s."), *Pair.Key.ToString(), *EqualValue);
                }
            }
        }

        const FString NumericConflict = FindNumericRangeConflict(Pair.Key, Pair.Value);
        if (!NumericConflict.IsEmpty())
        {
            return NumericConflict;
        }
    }

    return FString();
}

static bool MayConditionBranchesOverlap(const FNarrRailConditionBranch& Left, const FNarrRailConditionBranch& Right)
{
    if (Left.Terms.Num() == 0 || Right.Terms.Num() == 0)
    {
        return true;
    }

    if (Left.Logic != ENarrRailConditionLogic::All || Right.Logic != ENarrRailConditionLogic::All)
    {
        return false;
    }

    FNarrRailConditionBranch Combined;
    Combined.Logic = ENarrRailConditionLogic::All;
    Combined.Terms.Append(Left.Terms);
    Combined.Terms.Append(Right.Terms);
    return FindSimpleConditionContradiction(Combined).IsEmpty();
}

static void ValidateConditionBranchUniqueness(
    TArray<FNarrRailValidationIssue>& OutIssues,
    const FName NodeId,
    const TArray<FNarrRailConditionBranch>& Branches)
{
    TMap<FString, int32> SeenSignatures;

    for (int32 Index = 0; Index < Branches.Num(); ++Index)
    {
        const FString Signature = GetConditionBranchSignature(Branches[Index]);
        if (const int32* ExistingIndex = SeenSignatures.Find(Signature))
        {
            AddIssue(
                OutIssues,
                ENarrRailValidationSeverity::Error,
                NodeId,
                FString::Printf(TEXT("Condition branch %d duplicates branch %d."), Index, *ExistingIndex));
        }
        else
        {
            SeenSignatures.Add(Signature, Index);
        }

        const FString Contradiction = FindSimpleConditionContradiction(Branches[Index]);
        if (!Contradiction.IsEmpty())
        {
            AddIssue(
                OutIssues,
                ENarrRailValidationSeverity::Warning,
                NodeId,
                FString::Printf(TEXT("Condition branch %d may never match: %s"), Index, *Contradiction));
        }
    }

    for (int32 LeftIndex = 0; LeftIndex < Branches.Num(); ++LeftIndex)
    {
        for (int32 RightIndex = LeftIndex + 1; RightIndex < Branches.Num(); ++RightIndex)
        {
            if (GetConditionBranchSignature(Branches[LeftIndex]) == GetConditionBranchSignature(Branches[RightIndex]))
            {
                continue;
            }

            if (MayConditionBranchesOverlap(Branches[LeftIndex], Branches[RightIndex]))
            {
                AddIssue(
                    OutIssues,
                    ENarrRailValidationSeverity::Warning,
                    NodeId,
                    FString::Printf(
                        TEXT("Condition branches %d and %d may both match; the first matching branch wins."),
                        LeftIndex,
                        RightIndex));
            }
        }
    }
}

static void ValidateActions(
    TArray<FNarrRailValidationIssue>& OutIssues,
    const FName NodeId,
    const TArray<FNarrRailNodeAction>& Actions,
    const TMap<FName, ENarrRailVariableType>& VariableTypes)
{
    for (const FNarrRailNodeAction& Action : Actions)
    {
        if (Action.ActionType == ENarrRailActionType::EmitEvent)
        {
            if (Action.EventId == NAME_None)
            {
                AddIssue(OutIssues, ENarrRailValidationSeverity::Warning, NodeId, TEXT("EmitEvent action has empty EventId."));
            }
            continue;
        }

        if (Action.Variable.VariableName == NAME_None)
        {
            AddIssue(OutIssues, ENarrRailValidationSeverity::Error, NodeId, TEXT("Variable action has empty variable name."));
            continue;
        }

        const ENarrRailVariableType* DefinedType = VariableTypes.Find(Action.Variable.VariableName);
        if (DefinedType == nullptr)
        {
            AddIssue(
                OutIssues,
                ENarrRailValidationSeverity::Error,
                NodeId,
                FString::Printf(TEXT("Variable action references unknown variable '%s'."), *Action.Variable.VariableName.ToString()));
            continue;
        }

        if ((Action.ActionType == ENarrRailActionType::Add || Action.ActionType == ENarrRailActionType::Subtract) &&
            *DefinedType != ENarrRailVariableType::Int &&
            *DefinedType != ENarrRailVariableType::Float)
        {
            AddIssue(
                OutIssues,
                ENarrRailValidationSeverity::Error,
                NodeId,
                FString::Printf(TEXT("Variable action '%s' only supports Int/Float variables."), *Action.Variable.VariableName.ToString()));
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
    const UNarrRailGlobalConfigAsset* LinkedGlobalConfig = nullptr;
    if (StoryAsset != nullptr && !StoryAsset->GlobalConfig.IsNull())
    {
        LinkedGlobalConfig = StoryAsset->GlobalConfig.LoadSynchronous();
    }

    return ValidateStoryAssetWithGlobalConfig(StoryAsset, LinkedGlobalConfig);
}

TArray<FNarrRailValidationIssue> UNarrRailStoryValidator::ValidateStoryAssetWithGlobalConfig(const UNarrRailStoryAsset* StoryAsset, const UNarrRailGlobalConfigAsset* GlobalConfigAsset)
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
    TMap<FName, ENarrRailVariableType> VariableTypes;
    if (GlobalConfigAsset != nullptr)
    {
        for (const FNarrRailVariableDefinition& Var : GlobalConfigAsset->Variables)
        {
            if (Var.VariableName == NAME_None)
            {
                NarrRailValidation::AddIssue(Issues, ENarrRailValidationSeverity::Error, NAME_None, TEXT("Global variable definition has empty name."));
                continue;
            }

            if (Variables.Contains(Var.VariableName))
            {
                NarrRailValidation::AddIssue(Issues, ENarrRailValidationSeverity::Error, Var.VariableName, TEXT("Duplicate global variable name detected."));
                continue;
            }

            Variables.Add(Var.VariableName);
            VariableTypes.Add(Var.VariableName, Var.VariableType);
        }
    }

    for (const FNarrRailVariableDefinition& Var : StoryAsset->Variables)
    {
        if (Var.VariableName == NAME_None)
        {
            NarrRailValidation::AddIssue(Issues, ENarrRailValidationSeverity::Error, NAME_None, TEXT("Variable definition has empty name."));
            continue;
        }

        if (Variables.Contains(Var.VariableName))
        {
            NarrRailValidation::AddIssue(Issues, ENarrRailValidationSeverity::Error, Var.VariableName, TEXT("Variable name conflicts with another story/global variable."));
            continue;
        }

        Variables.Add(Var.VariableName);
        VariableTypes.Add(Var.VariableName, Var.VariableType);
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
        NarrRailValidation::ValidateActions(Issues, Node.NodeId, Node.EnterActions, VariableTypes);
        NarrRailValidation::ValidateActions(Issues, Node.NodeId, Node.ExitActions, VariableTypes);

        if (Node.NodeType == ENarrRailNodeType::SetVariable && Node.EnterActions.Num() == 0)
        {
            NarrRailValidation::AddIssue(
                Issues,
                ENarrRailValidationSeverity::Error,
                Node.NodeId,
                TEXT("SetVariable node requires at least one action."));
        }

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
                    if (!NarrRailValidation::IsModernConditionHandle(HandleText))
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
                    if (!HandleCounts.Contains(RequiredHandle))
                    {
                        NarrRailValidation::AddIssue(
                            Issues,
                            ENarrRailValidationSeverity::Error,
                            Node.NodeId,
                            FString::Printf(TEXT("Condition branch %d is missing outgoing handle '%s'."), BranchIndex, *RequiredHandle.ToString()));
                    }
                }

                NarrRailValidation::ValidateConditionBranchUniqueness(Issues, Node.NodeId, Node.Condition.Branches);

                if (!HandleCounts.Contains(FName(TEXT("condition-fallback"))))
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

                if (!HandleCounts.Contains(FName(TEXT("condition-0"))))
                {
                    NarrRailValidation::AddIssue(
                        Issues,
                        ENarrRailValidationSeverity::Error,
                        Node.NodeId,
                        TEXT("Legacy Condition node is missing condition-0 outgoing edge."));
                }

                if (!HandleCounts.Contains(FName(TEXT("condition-fallback"))))
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
