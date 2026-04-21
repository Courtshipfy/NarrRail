#pragma once

#include "CoreMinimal.h"
#include "Runtime/NarrRailStoryTypes.h"

/**
 * Intermediate data structure for parsed YAML script
 */
struct FNarrRailScriptData
{
	// Meta
	int32 SchemaVersion = 1;
	FString StoryId;
	FString EntryNodeId;

	// Variables
	TArray<FNarrRailVariableDefinition> Variables;

	// Nodes
	TArray<FNarrRailNode> Nodes;

	// Edges
	TArray<FNarrRailNodeEdge> Edges;
};

/**
 * YAML parser for NarrRail story scripts
 */
class FNarrRailYamlParser
{
public:
	/**
	 * Parse YAML file and convert to UE data structures
	 * @param FilePath Path to .narrrail.yaml file
	 * @param OutData Parsed data output
	 * @param OutErrorMessage Error message if parsing fails
	 * @return true if parsing succeeded
	 */
	bool ParseFile(const FString& FilePath, FNarrRailScriptData& OutData, FString& OutErrorMessage);
};
