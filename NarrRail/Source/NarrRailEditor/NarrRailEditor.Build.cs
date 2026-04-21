using UnrealBuildTool;
using System.IO;

public class NarrRailEditor : ModuleRules
{
    public NarrRailEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "CoreUObject",
            "Engine",
            "UnrealEd",
            "Slate",
            "SlateCore",
            "NarrRail",
            "AssetTools",
            "ContentBrowser"
        });

        // yaml-cpp integration
        string YamlCppPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../ThirdParty/yaml-cpp"));
        PublicIncludePaths.Add(Path.Combine(YamlCppPath, "include"));
        PrivateIncludePaths.Add(Path.Combine(YamlCppPath, "src"));

        PublicDefinitions.Add("YAML_CPP_STATIC_DEFINE");

        // Enable exceptions for yaml-cpp
        bEnableExceptions = true;
    }
}
