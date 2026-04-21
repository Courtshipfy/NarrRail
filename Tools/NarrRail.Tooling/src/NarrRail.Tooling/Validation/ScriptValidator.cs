using NarrRail.Tooling.Models;

namespace NarrRail.Tooling.Validation;

/// <summary>
/// Story script validator
/// </summary>
public class ScriptValidator
{
    private static readonly HashSet<string> ValidNodeTypes = new()
    {
        "Dialogue", "Choice", "Jump", "SetVariable", "EmitEvent", "End"
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
                break;

            case "Jump":
                if (string.IsNullOrWhiteSpace(node.JumpTargetNodeId))
                {
                    result.AddError($"Jump node '{node.NodeId}' has empty jumpTargetNodeId",
                        $"nodes[{node.NodeId}].jumpTargetNodeId");
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

            if (edge.Condition != null)
            {
                ValidateCondition(edge.Condition, $"edges[{edge.SourceNodeId}->{edge.TargetNodeId}].condition", result);
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
