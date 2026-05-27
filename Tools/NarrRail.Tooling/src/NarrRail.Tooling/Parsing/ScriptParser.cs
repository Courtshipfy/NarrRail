using YamlDotNet.Serialization;
using YamlDotNet.Serialization.NamingConventions;
using YamlDotNet.RepresentationModel;
using NarrRail.Tooling.Models;

namespace NarrRail.Tooling.Parsing;

/// <summary>
/// YAML script parser
/// </summary>
public class ScriptParser
{
    private readonly IDeserializer _deserializer;

    public ScriptParser()
    {
        _deserializer = new DeserializerBuilder()
            .WithNamingConvention(CamelCaseNamingConvention.Instance)
            .IgnoreUnmatchedProperties()
            .Build();
    }

    /// <summary>
    /// Parse YAML file to StoryScript
    /// </summary>
    public StoryScript ParseFile(string filePath)
    {
        if (!File.Exists(filePath))
        {
            throw new FileNotFoundException($"Script file not found: {filePath}");
        }

        var yaml = File.ReadAllText(filePath);
        return ParseYaml(yaml);
    }

    /// <summary>
    /// Parse YAML string to StoryScript
    /// </summary>
    public StoryScript ParseYaml(string yaml)
    {
        try
        {
            RejectDeprecatedConditionsOutsideConditionNodes(yaml);
            var script = _deserializer.Deserialize<StoryScript>(yaml);
            return script ?? throw new InvalidOperationException("Failed to deserialize YAML");
        }
        catch (YamlDotNet.Core.YamlException ex)
        {
            throw new InvalidOperationException($"YAML syntax error: {ex.Message}", ex);
        }
    }

    private static void RejectDeprecatedConditionsOutsideConditionNodes(string yaml)
    {
        var stream = new YamlStream();
        using var reader = new StringReader(yaml);
        stream.Load(reader);

        if (stream.Documents.Count == 0 ||
            stream.Documents[0].RootNode is not YamlMappingNode root)
        {
            return;
        }

        if (TryGetMappingValue(root, "edges", out var edgesNode) &&
            edgesNode is YamlSequenceNode edges)
        {
            for (var i = 0; i < edges.Children.Count; i++)
            {
                if (edges.Children[i] is YamlMappingNode edge &&
                    TryGetMappingValue(edge, "condition", out _))
                {
                    throw new InvalidOperationException(
                        $"edges[{i}].condition is deprecated. Use a Condition node with condition-true and condition-false sourceHandle outputs.");
                }
            }
        }

        if (TryGetMappingValue(root, "nodes", out var nodesNode) &&
            nodesNode is YamlSequenceNode nodes)
        {
            for (var nodeIndex = 0; nodeIndex < nodes.Children.Count; nodeIndex++)
            {
                if (nodes.Children[nodeIndex] is not YamlMappingNode node ||
                    !TryGetMappingValue(node, "choices", out var choicesNode) ||
                    choicesNode is not YamlSequenceNode choices)
                {
                    continue;
                }

                for (var choiceIndex = 0; choiceIndex < choices.Children.Count; choiceIndex++)
                {
                    if (choices.Children[choiceIndex] is YamlMappingNode choice &&
                        TryGetMappingValue(choice, "availability", out _))
                    {
                        throw new InvalidOperationException(
                            $"nodes[{nodeIndex}].choices[{choiceIndex}].availability is deprecated. Use a Condition node before the Choice node to control conditional flow.");
                    }
                }
            }
        }
    }

    private static bool TryGetMappingValue(YamlMappingNode mapping, string key, out YamlNode value)
    {
        foreach (var entry in mapping.Children)
        {
            if (entry.Key is YamlScalarNode scalar && scalar.Value == key)
            {
                value = entry.Value;
                return true;
            }
        }

        value = null!;
        return false;
    }
}
