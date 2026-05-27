using NarrRail.Tooling.Models;

namespace NarrRail.Tooling.Validation;

/// <summary>
/// Story script validator
/// </summary>
public class ScriptValidator
{
    private static readonly HashSet<string> ValidNodeTypes = new()
    {
        "Dialogue", "MultiDialogue", "Choice", "Jump", "SetVariable", "EmitEvent", "Condition", "End"
    };

    private static readonly HashSet<string> ValidVariableTypes = new()
    {
        "Bool", "Int", "Float", "String"
    };

    private static readonly HashSet<string> ValidScopes = new()
    {
        "Session", "Global"
    };

    private static readonly HashSet<string> ValidOperators = new()
    {
        "==", "!=", ">", ">=", "<", "<="
    };

    private static readonly HashSet<string> ValidActionTypes = new()
    {
        "Set", "Add", "Subtract", "EmitEvent"
    };

    private static readonly HashSet<string> ValidLogicTypes = new()
    {
        "All", "Any"
    };

    private static readonly HashSet<string> ValidChoiceModes = new()
    {
        "SinglePass", "ExhaustiveUntilComplete"
    };

    /// <summary>
    /// Validate story script
    /// </summary>
    public ValidationResult Validate(StoryScript script)
    {
        var result = new ValidationResult();

        // Schema validation
        ValidateSchema(script, result);

        // Semantic validation
        if (result.IsValid)
        {
            ValidateSemantics(script, result);
        }

        return result;
    }

    private void ValidateSchema(StoryScript script, ValidationResult result)
    {
        // Validate meta
        if (script.Meta == null)
        {
            result.AddError("Missing 'meta' section");
            return;
        }

        if (script.Meta.SchemaVersion != 1)
        {
            result.AddError($"Unsupported schema version: {script.Meta.SchemaVersion}. Expected: 1", "meta.schemaVersion");
        }

        if (string.IsNullOrWhiteSpace(script.Meta.StoryId))
        {
            result.AddError("Missing or empty 'storyId'", "meta.storyId");
        }

        if (string.IsNullOrWhiteSpace(script.Meta.EntryNodeId))
        {
            result.AddError("Missing or empty 'entryNodeId'", "meta.entryNodeId");
        }

        // Validate variables
        ValidateVariables(script.Variables, result);

        // Validate nodes
        ValidateNodes(script.Nodes, result);

        // Validate edges
        ValidateEdges(script.Edges, result);
    }

    private void ValidateVariables(List<VariableDefinition> variables, ValidationResult result)
    {
        var variableNames = new HashSet<string>();

        foreach (var variable in variables)
        {
            if (string.IsNullOrWhiteSpace(variable.Name))
            {
                result.AddError("Variable has empty name", "variables[]");
                continue;
            }

            if (!variableNames.Add(variable.Name))
            {
                result.AddError($"Duplicate variable name: '{variable.Name}'", $"variables[{variable.Name}]");
            }

            if (!ValidVariableTypes.Contains(variable.Type))
            {
                result.AddError($"Invalid variable type: '{variable.Type}'. Valid types: {string.Join(", ", ValidVariableTypes)}",
                    $"variables[{variable.Name}].type");
            }

            if (!string.IsNullOrEmpty(variable.Scope) && !ValidScopes.Contains(variable.Scope))
            {
                result.AddError($"Invalid variable scope: '{variable.Scope}'. Valid scopes: {string.Join(", ", ValidScopes)}",
                    $"variables[{variable.Name}].scope");
            }
        }
    }

    private void ValidateNodes(List<StoryNode> nodes, ValidationResult result)
    {
        var nodeIds = new HashSet<string>();

        foreach (var node in nodes)
        {
            if (string.IsNullOrWhiteSpace(node.NodeId))
            {
                result.AddError("Node has empty nodeId", "nodes[]");
                continue;
            }

            if (!nodeIds.Add(node.NodeId))
            {
                result.AddError($"Duplicate node ID: '{node.NodeId}'", $"nodes[{node.NodeId}]");
            }

            if (!ValidNodeTypes.Contains(node.NodeType))
            {
                result.AddError($"Invalid node type: '{node.NodeType}'. Valid types: {string.Join(", ", ValidNodeTypes)}",
                    $"nodes[{node.NodeId}].nodeType");
            }

            // Validate node-specific fields
            ValidateNodeSpecificFields(node, result);

            // Validate actions
            ValidateActions(node.EnterActions, $"nodes[{node.NodeId}].enterActions", result);
            ValidateActions(node.ExitActions, $"nodes[{node.NodeId}].exitActions", result);
        }
    }

    private void ValidateNodeSpecificFields(StoryNode node, ValidationResult result)
    {
        switch (node.NodeType)
        {
            case "Dialogue":
                if (node.Dialogue == null)
                {
                    result.AddError($"Dialogue node '{node.NodeId}' missing dialogue payload", $"nodes[{node.NodeId}].dialogue");
                }
                else if (string.IsNullOrWhiteSpace(node.Dialogue.TextKey))
                {
                    result.AddWarning($"Dialogue node '{node.NodeId}' has empty textKey", $"nodes[{node.NodeId}].dialogue.textKey");
                }
                break;

            case "MultiDialogue":
                if (node.MultiDialogue == null)
                {
                    result.AddError($"MultiDialogue node '{node.NodeId}' missing multiDialogue payload", $"nodes[{node.NodeId}].multiDialogue");
                }
                else if (node.MultiDialogue.Lines == null || node.MultiDialogue.Lines.Count == 0)
                {
                    result.AddError($"MultiDialogue node '{node.NodeId}' has no lines", $"nodes[{node.NodeId}].multiDialogue.lines");
                }
                else
                {
                    for (var i = 0; i < node.MultiDialogue.Lines.Count; i++)
                    {
                        if (string.IsNullOrWhiteSpace(node.MultiDialogue.Lines[i].TextKey))
                        {
                            result.AddWarning($"MultiDialogue node '{node.NodeId}' has empty textKey at line {i + 1}",
                                $"nodes[{node.NodeId}].multiDialogue.lines[{i}].textKey");
                        }
                    }
                }
                break;

            case "Choice":
                if (node.Choices == null || node.Choices.Count == 0)
                {
                    result.AddError($"Choice node '{node.NodeId}' has no choices", $"nodes[{node.NodeId}].choices");
                }
                else
                {
                    foreach (var choice in node.Choices)
                    {
                        if (string.IsNullOrWhiteSpace(choice.TargetNodeId))
                        {
                            result.AddError($"Choice in node '{node.NodeId}' has empty targetNodeId",
                                $"nodes[{node.NodeId}].choices[]");
                        }
                    }
                }

                if (!string.IsNullOrWhiteSpace(node.ChoiceMode) && !ValidChoiceModes.Contains(node.ChoiceMode))
                {
                    result.AddError($"Choice node '{node.NodeId}' has invalid choiceMode: '{node.ChoiceMode}'. Valid modes: {string.Join(", ", ValidChoiceModes)}",
                        $"nodes[{node.NodeId}].choiceMode");
                }

                if (node.ChoiceMode == "ExhaustiveUntilComplete" && string.IsNullOrWhiteSpace(node.ChoiceCompletionTargetNodeId))
                {
                    result.AddError($"Choice node '{node.NodeId}' requires choiceCompletionTargetNodeId in ExhaustiveUntilComplete mode",
                        $"nodes[{node.NodeId}].choiceCompletionTargetNodeId");
                }
                break;

            case "Jump":
                if (string.IsNullOrWhiteSpace(node.JumpTargetNodeId))
                {
                    result.AddError($"Jump node '{node.NodeId}' has empty jumpTargetNodeId",
                        $"nodes[{node.NodeId}].jumpTargetNodeId");
                }
                break;

            case "Condition":
                if (node.Condition == null)
                {
                    result.AddError($"Condition node '{node.NodeId}' missing condition expression", $"nodes[{node.NodeId}].condition");
                }
                else
                {
                    ValidateCondition(node.Condition, $"nodes[{node.NodeId}].condition", result);
                }
                break;
        }
    }

    private void ValidateEdges(List<StoryEdge> edges, ValidationResult result)
    {
        foreach (var edge in edges)
        {
            if (string.IsNullOrWhiteSpace(edge.SourceNodeId))
            {
                result.AddError("Edge has empty sourceNodeId", "edges[]");
            }

            if (string.IsNullOrWhiteSpace(edge.TargetNodeId))
            {
                result.AddError("Edge has empty targetNodeId", "edges[]");
            }
        }
    }

    private void ValidateCondition(ConditionExpression condition, string location, ValidationResult result)
    {
        if (!ValidLogicTypes.Contains(condition.Logic))
        {
            result.AddError($"Invalid logic type: '{condition.Logic}'. Valid types: {string.Join(", ", ValidLogicTypes)}",
                $"{location}.logic");
        }

        if (condition.Terms == null)
        {
            result.AddError("Condition terms must be an array", $"{location}.terms");
            return;
        }

        foreach (var term in condition.Terms)
        {
            if (term.Variable == null)
            {
                result.AddError("Condition term missing variable reference", $"{location}.terms[]");
                continue;
            }

            if (!ValidOperators.Contains(term.Operator))
            {
                result.AddError($"Invalid operator: '{term.Operator}'. Valid operators: {string.Join(", ", ValidOperators)}",
                    $"{location}.terms[].operator");
            }
        }
    }

    private void ValidateActions(List<NodeAction> actions, string location, ValidationResult result)
    {
        foreach (var action in actions)
        {
            if (!ValidActionTypes.Contains(action.ActionType))
            {
                result.AddError($"Invalid action type: '{action.ActionType}'. Valid types: {string.Join(", ", ValidActionTypes)}",
                    $"{location}[].actionType");
            }

            if (action.ActionType != "EmitEvent" && action.Variable == null)
            {
                result.AddError($"Action type '{action.ActionType}' requires variable reference", $"{location}[]");
            }

            if (action.ActionType == "EmitEvent" && string.IsNullOrWhiteSpace(action.EventId))
            {
                result.AddError("EmitEvent action requires eventId", $"{location}[]");
            }
        }
    }

    private void ValidateSemantics(StoryScript script, ValidationResult result)
    {
        var nodeIds = new HashSet<string>(script.Nodes.Select(n => n.NodeId));
        var variableNames = new HashSet<string>(script.Variables.Select(v => v.Name));
        var nodeById = script.Nodes.ToDictionary(n => n.NodeId, n => n);

        // Check entry node exists
        if (!nodeIds.Contains(script.Meta.EntryNodeId))
        {
            result.AddError($"Entry node '{script.Meta.EntryNodeId}' does not exist in nodes", "meta.entryNodeId");
        }

        // Check edge references
        foreach (var edge in script.Edges)
        {
            if (!nodeIds.Contains(edge.SourceNodeId))
            {
                result.AddError($"Edge references non-existent source node: '{edge.SourceNodeId}'",
                    $"edges[{edge.SourceNodeId}->{edge.TargetNodeId}]");
            }

            if (!nodeIds.Contains(edge.TargetNodeId))
            {
                result.AddError($"Edge references non-existent target node: '{edge.TargetNodeId}'",
                    $"edges[{edge.SourceNodeId}->{edge.TargetNodeId}]");
            }

            ValidateEdgeSourceHandle(edge, nodeById, result);
        }

        // Check choice target references
        foreach (var node in script.Nodes.Where(n => n.NodeType == "Choice"))
        {
            foreach (var choice in node.Choices)
            {
                if (!nodeIds.Contains(choice.TargetNodeId))
                {
                    result.AddError($"Choice in node '{node.NodeId}' references non-existent target: '{choice.TargetNodeId}'",
                        $"nodes[{node.NodeId}].choices[]");
                }

            }

            if (!string.IsNullOrWhiteSpace(node.ChoiceCompletionTargetNodeId) &&
                !nodeIds.Contains(node.ChoiceCompletionTargetNodeId))
            {
                result.AddError($"Choice node '{node.NodeId}' references non-existent completion target: '{node.ChoiceCompletionTargetNodeId}'",
                    $"nodes[{node.NodeId}].choiceCompletionTargetNodeId");
            }
        }

        // Check jump target references
        foreach (var node in script.Nodes.Where(n => n.NodeType == "Jump"))
        {
            if (!string.IsNullOrEmpty(node.JumpTargetNodeId) && !nodeIds.Contains(node.JumpTargetNodeId))
            {
                result.AddError($"Jump node '{node.NodeId}' references non-existent target: '{node.JumpTargetNodeId}'",
                    $"nodes[{node.NodeId}].jumpTargetNodeId");
            }
        }

        // Check condition node branch completeness and variable references
        foreach (var node in script.Nodes.Where(n => n.NodeType == "Condition"))
        {
            ValidateConditionNodeBranches(node, script.Edges, nodeIds, result);

            if (node.Condition != null)
            {
                ValidateConditionVariables(node.Condition, variableNames, $"nodes[{node.NodeId}].condition", result);
            }
        }

        foreach (var node in script.Nodes)
        {
            ValidateActionsVariables(node.EnterActions, variableNames, $"nodes[{node.NodeId}].enterActions", result);
            ValidateActionsVariables(node.ExitActions, variableNames, $"nodes[{node.NodeId}].exitActions", result);
        }

        // Check for orphaned nodes (warning)
        var referencedNodes = new HashSet<string> { script.Meta.EntryNodeId };
        foreach (var edge in script.Edges)
        {
            referencedNodes.Add(edge.TargetNodeId);
        }
        foreach (var node in script.Nodes.Where(n => n.NodeType == "Choice"))
        {
            foreach (var choice in node.Choices)
            {
                referencedNodes.Add(choice.TargetNodeId);
            }

            if (!string.IsNullOrEmpty(node.ChoiceCompletionTargetNodeId))
            {
                referencedNodes.Add(node.ChoiceCompletionTargetNodeId);
            }
        }
        foreach (var node in script.Nodes.Where(n => n.NodeType == "Jump"))
        {
            if (!string.IsNullOrEmpty(node.JumpTargetNodeId))
            {
                referencedNodes.Add(node.JumpTargetNodeId);
            }
        }

        foreach (var node in script.Nodes)
        {
            if (!referencedNodes.Contains(node.NodeId) && node.NodeType != "End")
            {
                result.AddWarning($"Orphaned node (not reachable from entry): '{node.NodeId}'", $"nodes[{node.NodeId}]");
            }
        }

        // Check for cycles (simple detection)
        DetectCycles(script, result);
    }

    private void ValidateEdgeSourceHandle(StoryEdge edge, Dictionary<string, StoryNode> nodeById, ValidationResult result)
    {
        if (!nodeById.TryGetValue(edge.SourceNodeId, out var sourceNode))
        {
            return;
        }

        if (string.IsNullOrWhiteSpace(edge.SourceHandle))
        {
            if (sourceNode.NodeType == "Condition")
            {
                result.AddError($"Condition edge from '{edge.SourceNodeId}' missing sourceHandle; expected condition-true or condition-false",
                    $"edges[{edge.SourceNodeId}->{edge.TargetNodeId}].sourceHandle");
            }

            return;
        }

        if (sourceNode.NodeType == "Condition")
        {
            if (edge.SourceHandle != "condition-true" && edge.SourceHandle != "condition-false")
            {
                result.AddError($"Condition edge from '{edge.SourceNodeId}' has invalid sourceHandle '{edge.SourceHandle}'. Expected condition-true or condition-false",
                    $"edges[{edge.SourceNodeId}->{edge.TargetNodeId}].sourceHandle");
            }

            return;
        }

        if (edge.SourceHandle.StartsWith("condition-", StringComparison.Ordinal))
        {
            result.AddError($"Non-Condition node '{edge.SourceNodeId}' cannot use condition sourceHandle '{edge.SourceHandle}'",
                $"edges[{edge.SourceNodeId}->{edge.TargetNodeId}].sourceHandle");
            return;
        }

        if (sourceNode.NodeType == "Choice")
        {
            ValidateChoiceSourceHandle(edge, sourceNode, result);
            return;
        }

        if (edge.SourceHandle.StartsWith("choice-", StringComparison.Ordinal))
        {
            result.AddError($"Non-Choice node '{edge.SourceNodeId}' cannot use choice sourceHandle '{edge.SourceHandle}'",
                $"edges[{edge.SourceNodeId}->{edge.TargetNodeId}].sourceHandle");
        }
    }

    private void ValidateChoiceSourceHandle(StoryEdge edge, StoryNode sourceNode, ValidationResult result)
    {
        if (edge.SourceHandle == "choice-complete")
        {
            if (sourceNode.ChoiceMode != "ExhaustiveUntilComplete")
            {
                result.AddWarning($"Choice node '{sourceNode.NodeId}' uses choice-complete outside ExhaustiveUntilComplete mode",
                    $"edges[{edge.SourceNodeId}->{edge.TargetNodeId}].sourceHandle");
            }

            return;
        }

        if (!edge.SourceHandle.StartsWith("choice-", StringComparison.Ordinal))
        {
            result.AddWarning($"Choice edge from '{edge.SourceNodeId}' uses non-standard sourceHandle '{edge.SourceHandle}'",
                $"edges[{edge.SourceNodeId}->{edge.TargetNodeId}].sourceHandle");
            return;
        }

        var indexText = edge.SourceHandle["choice-".Length..];
        if (!int.TryParse(indexText, out var index) || index < 0 || index >= sourceNode.Choices.Count)
        {
            result.AddError($"Choice edge from '{edge.SourceNodeId}' has out-of-range sourceHandle '{edge.SourceHandle}'",
                $"edges[{edge.SourceNodeId}->{edge.TargetNodeId}].sourceHandle");
        }
    }

    private void ValidateConditionNodeBranches(StoryNode node, List<StoryEdge> edges, HashSet<string> nodeIds, ValidationResult result)
    {
        var outgoing = edges.Where(e => e.SourceNodeId == node.NodeId).ToList();
        var trueEdges = outgoing.Where(e => e.SourceHandle == "condition-true").ToList();
        var falseEdges = outgoing.Where(e => e.SourceHandle == "condition-false").ToList();

        if (trueEdges.Count != 1)
        {
            result.AddError($"Condition node '{node.NodeId}' must have exactly one condition-true outgoing edge",
                $"nodes[{node.NodeId}].condition-true");
        }

        if (falseEdges.Count != 1)
        {
            result.AddError($"Condition node '{node.NodeId}' must have exactly one condition-false outgoing edge",
                $"nodes[{node.NodeId}].condition-false");
        }

        foreach (var edge in trueEdges.Concat(falseEdges))
        {
            if (!nodeIds.Contains(edge.TargetNodeId))
            {
                result.AddError($"Condition node '{node.NodeId}' branch target does not exist: '{edge.TargetNodeId}'",
                    $"edges[{edge.SourceNodeId}->{edge.TargetNodeId}]");
            }
        }
    }

    private void ValidateConditionVariables(ConditionExpression condition, HashSet<string> variableNames, string location, ValidationResult result)
    {
        if (condition.Terms == null)
        {
            return;
        }

        foreach (var term in condition.Terms)
        {
            if (term.Variable == null)
            {
                continue;
            }

            if (string.IsNullOrWhiteSpace(term.Variable.Name))
            {
                result.AddError("Condition term has empty variable name", $"{location}.terms[].variable.name");
            }
            else if (!variableNames.Contains(term.Variable.Name))
            {
                result.AddError($"Condition references undefined variable: '{term.Variable.Name}'", $"{location}.terms[].variable.name");
            }

            if (!string.IsNullOrEmpty(term.Variable.Type) && !ValidVariableTypes.Contains(term.Variable.Type))
            {
                result.AddError($"Invalid condition variable type: '{term.Variable.Type}'. Valid types: {string.Join(", ", ValidVariableTypes)}",
                    $"{location}.terms[].variable.type");
            }

            if (!string.IsNullOrEmpty(term.Variable.Scope) && !ValidScopes.Contains(term.Variable.Scope))
            {
                result.AddError($"Invalid condition variable scope: '{term.Variable.Scope}'. Valid scopes: {string.Join(", ", ValidScopes)}",
                    $"{location}.terms[].variable.scope");
            }
        }
    }

    private void ValidateActionsVariables(List<NodeAction> actions, HashSet<string> variableNames, string location, ValidationResult result)
    {
        for (var i = 0; i < actions.Count; i++)
        {
            var action = actions[i];
            if (action.ActionType == "EmitEvent" || action.Variable == null)
            {
                continue;
            }

            if (!variableNames.Contains(action.Variable.Name))
            {
                result.AddError($"Action references undefined variable: '{action.Variable.Name}'", $"{location}[{i}].variable.name");
            }
        }
    }

    private void DetectCycles(StoryScript script, ValidationResult result)
    {
        var graph = new Dictionary<string, List<string>>();

        // Build adjacency list
        foreach (var node in script.Nodes)
        {
            graph[node.NodeId] = new List<string>();
        }

        foreach (var edge in script.Edges)
        {
            if (graph.ContainsKey(edge.SourceNodeId))
            {
                graph[edge.SourceNodeId].Add(edge.TargetNodeId);
            }
        }

        foreach (var node in script.Nodes.Where(n => n.NodeType == "Choice"))
        {
            foreach (var choice in node.Choices)
            {
                if (graph.ContainsKey(node.NodeId))
                {
                    graph[node.NodeId].Add(choice.TargetNodeId);
                }
            }
        }

        foreach (var node in script.Nodes.Where(n => n.NodeType == "Jump"))
        {
            if (!string.IsNullOrEmpty(node.JumpTargetNodeId) && graph.ContainsKey(node.NodeId))
            {
                graph[node.NodeId].Add(node.JumpTargetNodeId);
            }
        }

        // DFS cycle detection
        var visited = new HashSet<string>();
        var recursionStack = new HashSet<string>();

        bool HasCycle(string nodeId)
        {
            if (recursionStack.Contains(nodeId))
            {
                return true;
            }

            if (visited.Contains(nodeId))
            {
                return false;
            }

            visited.Add(nodeId);
            recursionStack.Add(nodeId);

            if (graph.ContainsKey(nodeId))
            {
                foreach (var neighbor in graph[nodeId])
                {
                    if (HasCycle(neighbor))
                    {
                        return true;
                    }
                }
            }

            recursionStack.Remove(nodeId);
            return false;
        }

        foreach (var nodeId in graph.Keys)
        {
            if (HasCycle(nodeId))
            {
                result.AddWarning($"Potential cycle detected involving node: '{nodeId}'", $"nodes[{nodeId}]");
                break; // Report only once
            }
        }
    }
}
