namespace NarrRail.Tooling.Models;

/// <summary>
/// Root structure of a NarrRail story script
/// </summary>
public class StoryScript
{
    public StoryMeta Meta { get; set; } = new();
    public List<VariableDefinition> Variables { get; set; } = new();
    public List<StoryNode> Nodes { get; set; } = new();
    public List<StoryEdge> Edges { get; set; } = new();
}

/// <summary>
/// Story metadata
/// </summary>
public class StoryMeta
{
    public int SchemaVersion { get; set; }
    public string StoryId { get; set; } = string.Empty;
    public string EntryNodeId { get; set; } = string.Empty;
}

/// <summary>
/// Variable definition
/// </summary>
public class VariableDefinition
{
    public string Name { get; set; } = string.Empty;
    public string Type { get; set; } = string.Empty; // Bool, Int, Float, String
    public string Scope { get; set; } = "Session"; // Session, Global
    public string DefaultValue { get; set; } = string.Empty;
}

/// <summary>
/// Story node
/// </summary>
public class StoryNode
{
    public string NodeId { get; set; } = string.Empty;
    public string NodeType { get; set; } = string.Empty; // Dialogue, Choice, Jump, SetVariable, EmitEvent, End
    public DialoguePayload? Dialogue { get; set; }
    public List<ChoiceOption> Choices { get; set; } = new();
    public string JumpTargetNodeId { get; set; } = string.Empty;
    public List<NodeAction> EnterActions { get; set; } = new();
    public List<NodeAction> ExitActions { get; set; } = new();
}

/// <summary>
/// Dialogue payload
/// </summary>
public class DialoguePayload
{
    public string SpeakerId { get; set; } = string.Empty;
    public string TextKey { get; set; } = string.Empty;
    public float SpeechRate { get; set; } = 1.0f;
    public string VoiceAsset { get; set; } = string.Empty;
}

/// <summary>
/// Choice option
/// </summary>
public class ChoiceOption
{
    public string TextKey { get; set; } = string.Empty;
    public string TargetNodeId { get; set; } = string.Empty;
    public ConditionExpression? Availability { get; set; }
}

/// <summary>
/// Story edge
/// </summary>
public class StoryEdge
{
    public string SourceNodeId { get; set; } = string.Empty;
    public string TargetNodeId { get; set; } = string.Empty;
    public int Priority { get; set; } = 0;
    public ConditionExpression? Condition { get; set; }
}

/// <summary>
/// Condition expression
/// </summary>
public class ConditionExpression
{
    public string Logic { get; set; } = "All"; // All, Any
    public List<ConditionTerm> Terms { get; set; } = new();
}

/// <summary>
/// Condition term
/// </summary>
public class ConditionTerm
{
    public VariableReference? Variable { get; set; }
    public string Operator { get; set; } = string.Empty; // ==, !=, >, >=, <, <=
    public string CompareValue { get; set; } = string.Empty;
}

/// <summary>
/// Variable reference
/// </summary>
public class VariableReference
{
    public string Name { get; set; } = string.Empty;
    public string Type { get; set; } = string.Empty;
    public string Scope { get; set; } = "Session";
}

/// <summary>
/// Node action
/// </summary>
public class NodeAction
{
    public string ActionType { get; set; } = string.Empty; // Set, Add, Subtract, EmitEvent
    public VariableReference? Variable { get; set; }
    public string Value { get; set; } = string.Empty;
    public string EventId { get; set; } = string.Empty;
}
