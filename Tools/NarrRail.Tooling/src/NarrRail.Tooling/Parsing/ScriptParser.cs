using YamlDotNet.Serialization;
using YamlDotNet.Serialization.NamingConventions;
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
            var script = _deserializer.Deserialize<StoryScript>(yaml);
            return script ?? throw new InvalidOperationException("Failed to deserialize YAML");
        }
        catch (YamlDotNet.Core.YamlException ex)
        {
            throw new InvalidOperationException($"YAML syntax error: {ex.Message}", ex);
        }
    }
}
