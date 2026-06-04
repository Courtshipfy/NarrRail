#include "Importers/NarrRailYamlParser.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

// Disable warnings for yaml-cpp
#pragma warning(push)
#pragma warning(disable: 4996) // deprecated functions
#pragma warning(disable: 4244) // conversion warnings
#include "yaml-cpp/yaml.h"
#pragma warning(pop)

// Forward declare helper functions
static ENarrRailNodeType ParseNodeType(const FString& TypeStr);
static ENarrRailVariableType ParseVariableType(const FString& TypeStr);
static ENarrRailComparisonOp ParseOperator(const FString& OpStr);
static ENarrRailActionType ParseActionType(const FString& ActionStr);
static ENarrRailConditionLogic ParseLogicType(const FString& LogicStr);
static ENarrRailChoiceMode ParseChoiceMode(const FString& ChoiceModeStr);

static bool ParseMeta(const YAML::Node& MetaNode, FNarrRailScriptData& OutData, FString& OutError);
static bool ParseVariables(const YAML::Node& VarsNode, FNarrRailScriptData& OutData, FString& OutError);
static bool ParseVariables(const YAML::Node& VarsNode, TArray<FNarrRailVariableDefinition>& OutVariables, FString& OutError);
static bool ParsePresetSpeakers(const YAML::Node& SpeakersNode, TArray<FNarrRailPresetSpeaker>& OutSpeakers, FString& OutError);
static bool ParseNodes(const YAML::Node& NodesNode, FNarrRailScriptData& OutData, FString& OutError);
static bool ParseEdges(const YAML::Node& EdgesNode, FNarrRailScriptData& OutData, FString& OutError);

static bool ParseDialogue(const YAML::Node& DialogueNode, FNarrRailDialoguePayload& OutDialogue);
static bool ParseMultiDialogue(const YAML::Node& MultiDialogueNode, FNarrRailMultiDialoguePayload& OutMultiDialogue);
static bool ParseChoices(const YAML::Node& ChoicesNode, TArray<FNarrRailChoiceOption>& OutChoices, FString& OutError);
static bool ParseActions(const YAML::Node& ActionsNode, TArray<FNarrRailNodeAction>& OutActions);
static bool ParseCondition(const YAML::Node& ConditionNode, FNarrRailConditionExpression& OutCondition);
static bool ParseConditionTerms(const YAML::Node& TermsNode, TArray<FNarrRailConditionTerm>& OutTerms);
static bool ParseVariableRef(const YAML::Node& VarNode, FNarrRailVariableRef& OutVarRef);

struct FParsedNarrRailEdge
{
	FNarrRailNodeEdge Edge;
	bool bHasDeprecatedCondition = false;
	FNarrRailConditionExpression DeprecatedCondition;
	int32 OriginalIndex = 0;
};

bool FNarrRailYamlParser::ParseFile(const FString& FilePath, FNarrRailScriptData& OutData, FString& OutErrorMessage)
{
	// Read file content
	FString YamlContent;
	if (!FFileHelper::LoadFileToString(YamlContent, *FilePath))
	{
		OutErrorMessage = FString::Printf(TEXT("Failed to read file: %s"), *FilePath);
		return false;
	}

	try
	{
		// Parse YAML
		YAML::Node Root = YAML::Load(TCHAR_TO_UTF8(*YamlContent));

		// Parse meta
		if (!Root["meta"])
		{
			OutErrorMessage = TEXT("Missing 'meta' section");
			return false;
		}
		if (!ParseMeta(Root["meta"], OutData, OutErrorMessage))
		{
			return false;
		}

		// Parse variables
		if (Root["variables"])
		{
			if (!ParseVariables(Root["variables"], OutData, OutErrorMessage))
			{
				return false;
			}
		}

		// Parse nodes
		if (!Root["nodes"])
		{
			OutErrorMessage = TEXT("Missing 'nodes' section");
			return false;
		}
		if (!ParseNodes(Root["nodes"], OutData, OutErrorMessage))
		{
			return false;
		}

		// Parse edges
		if (Root["edges"])
		{
			if (!ParseEdges(Root["edges"], OutData, OutErrorMessage))
			{
				return false;
			}
		}

		return true;
	}
	catch (const YAML::Exception& e)
	{
		OutErrorMessage = FString::Printf(TEXT("YAML parse error: %s"), UTF8_TO_TCHAR(e.what()));
		return false;
	}
	catch (...)
	{
		OutErrorMessage = TEXT("Unknown error during YAML parsing");
		return false;
	}
}

ENarrRailScriptFileKind FNarrRailYamlParser::DetectFileKind(const FString& FilePath, FString& OutErrorMessage) const
{
	FString YamlContent;
	if (!FFileHelper::LoadFileToString(YamlContent, *FilePath))
	{
		OutErrorMessage = FString::Printf(TEXT("Failed to read file: %s"), *FilePath);
		return ENarrRailScriptFileKind::Unknown;
	}

	try
	{
		const YAML::Node Root = YAML::Load(TCHAR_TO_UTF8(*YamlContent));
		const YAML::Node MetaNode = Root["meta"];
		if (MetaNode && MetaNode["configType"])
		{
			const FString ConfigType = UTF8_TO_TCHAR(MetaNode["configType"].as<std::string>().c_str());
			if (ConfigType.Equals(TEXT("GlobalConfig"), ESearchCase::IgnoreCase))
			{
				return ENarrRailScriptFileKind::GlobalConfig;
			}
		}

		if (Root["nodes"] || Root["edges"] || (MetaNode && MetaNode["storyId"]))
		{
			return ENarrRailScriptFileKind::Story;
		}

		OutErrorMessage = TEXT("Unable to determine NarrRail script file kind");
		return ENarrRailScriptFileKind::Unknown;
	}
	catch (const YAML::Exception& e)
	{
		OutErrorMessage = FString::Printf(TEXT("YAML parse error: %s"), UTF8_TO_TCHAR(e.what()));
		return ENarrRailScriptFileKind::Unknown;
	}
	catch (...)
	{
		OutErrorMessage = TEXT("Unknown error during YAML parsing");
		return ENarrRailScriptFileKind::Unknown;
	}
}

bool FNarrRailYamlParser::ParseGlobalConfigFile(const FString& FilePath, FNarrRailGlobalConfigData& OutData, FString& OutErrorMessage)
{
	FString YamlContent;
	if (!FFileHelper::LoadFileToString(YamlContent, *FilePath))
	{
		OutErrorMessage = FString::Printf(TEXT("Failed to read file: %s"), *FilePath);
		return false;
	}

	try
	{
		const YAML::Node Root = YAML::Load(TCHAR_TO_UTF8(*YamlContent));
		const YAML::Node MetaNode = Root["meta"];
		if (!MetaNode)
		{
			OutErrorMessage = TEXT("Missing 'meta' section");
			return false;
		}

		if (MetaNode["schemaVersion"])
		{
			OutData.SchemaVersion = MetaNode["schemaVersion"].as<int>();
		}

		if (!MetaNode["configType"])
		{
			OutErrorMessage = TEXT("Missing meta.configType");
			return false;
		}

		const FString ConfigType = UTF8_TO_TCHAR(MetaNode["configType"].as<std::string>().c_str());
		if (!ConfigType.Equals(TEXT("GlobalConfig"), ESearchCase::IgnoreCase))
		{
			OutErrorMessage = FString::Printf(TEXT("Unsupported configType: %s"), *ConfigType);
			return false;
		}

		OutData.Variables.Empty();
		if (Root["variables"])
		{
			if (!ParseVariables(Root["variables"], OutData.Variables, OutErrorMessage))
			{
				return false;
			}
		}

		OutData.PresetSpeakers.Empty();
		if (Root["presetSpeakers"])
		{
			if (!ParsePresetSpeakers(Root["presetSpeakers"], OutData.PresetSpeakers, OutErrorMessage))
			{
				return false;
			}
		}

		return true;
	}
	catch (const YAML::Exception& e)
	{
		OutErrorMessage = FString::Printf(TEXT("YAML parse error: %s"), UTF8_TO_TCHAR(e.what()));
		return false;
	}
	catch (...)
	{
		OutErrorMessage = TEXT("Unknown error during YAML parsing");
		return false;
	}
}

// Enum conversion helpers
static ENarrRailNodeType ParseNodeType(const FString& TypeStr)
{
	if (TypeStr == TEXT("Dialogue")) return ENarrRailNodeType::Dialogue;
	if (TypeStr == TEXT("MultiDialogue")) return ENarrRailNodeType::MultiDialogue;
	if (TypeStr == TEXT("Choice")) return ENarrRailNodeType::Choice;
	if (TypeStr == TEXT("Jump")) return ENarrRailNodeType::Jump;
	if (TypeStr == TEXT("SetVariable")) return ENarrRailNodeType::SetVariable;
	if (TypeStr == TEXT("EmitEvent")) return ENarrRailNodeType::EmitEvent;
	if (TypeStr == TEXT("Condition")) return ENarrRailNodeType::Condition;
	if (TypeStr == TEXT("End")) return ENarrRailNodeType::End;
	return ENarrRailNodeType::Dialogue; // Default
}

static ENarrRailVariableType ParseVariableType(const FString& TypeStr)
{
	if (TypeStr == TEXT("Bool")) return ENarrRailVariableType::Bool;
	if (TypeStr == TEXT("Int")) return ENarrRailVariableType::Int;
	if (TypeStr == TEXT("Float")) return ENarrRailVariableType::Float;
	if (TypeStr == TEXT("String")) return ENarrRailVariableType::String;
	return ENarrRailVariableType::String; // Default
}

static ENarrRailComparisonOp ParseOperator(const FString& OpStr)
{
	if (OpStr == TEXT("==")) return ENarrRailComparisonOp::Equal;
	if (OpStr == TEXT("!=")) return ENarrRailComparisonOp::NotEqual;
	if (OpStr == TEXT(">")) return ENarrRailComparisonOp::Greater;
	if (OpStr == TEXT(">=")) return ENarrRailComparisonOp::GreaterOrEqual;
	if (OpStr == TEXT("<")) return ENarrRailComparisonOp::Less;
	if (OpStr == TEXT("<=")) return ENarrRailComparisonOp::LessOrEqual;
	return ENarrRailComparisonOp::Equal; // Default
}

static ENarrRailActionType ParseActionType(const FString& ActionStr)
{
	if (ActionStr == TEXT("Set")) return ENarrRailActionType::Set;
	if (ActionStr == TEXT("Add")) return ENarrRailActionType::Add;
	if (ActionStr == TEXT("Subtract")) return ENarrRailActionType::Subtract;
	if (ActionStr == TEXT("EmitEvent")) return ENarrRailActionType::EmitEvent;
	return ENarrRailActionType::Set; // Default
}

static ENarrRailConditionLogic ParseLogicType(const FString& LogicStr)
{
	if (LogicStr == TEXT("All")) return ENarrRailConditionLogic::All;
	if (LogicStr == TEXT("Any")) return ENarrRailConditionLogic::Any;
	return ENarrRailConditionLogic::All; // Default
}

static ENarrRailChoiceMode ParseChoiceMode(const FString& ChoiceModeStr)
{
	if (ChoiceModeStr == TEXT("ExhaustiveUntilComplete")) return ENarrRailChoiceMode::ExhaustiveUntilComplete;
	return ENarrRailChoiceMode::SinglePass; // Default
}

// Parse meta section
static bool ParseMeta(const YAML::Node& MetaNode, FNarrRailScriptData& OutData, FString& OutError)
{
	if (!MetaNode["schemaVersion"])
	{
		OutError = TEXT("Missing meta.schemaVersion");
		return false;
	}
	OutData.SchemaVersion = MetaNode["schemaVersion"].as<int>();

	if (!MetaNode["storyId"])
	{
		OutError = TEXT("Missing meta.storyId");
		return false;
	}
	OutData.StoryId = UTF8_TO_TCHAR(MetaNode["storyId"].as<std::string>().c_str());

	if (!MetaNode["entryNodeId"])
	{
		OutError = TEXT("Missing meta.entryNodeId");
		return false;
	}
	OutData.EntryNodeId = UTF8_TO_TCHAR(MetaNode["entryNodeId"].as<std::string>().c_str());

	return true;
}

// Parse variables section
static bool ParseVariables(const YAML::Node& VarsNode, FNarrRailScriptData& OutData, FString& OutError)
{
	return ParseVariables(VarsNode, OutData.Variables, OutError);
}

static bool ParseVariables(const YAML::Node& VarsNode, TArray<FNarrRailVariableDefinition>& OutVariables, FString& OutError)
{
	if (!VarsNode.IsSequence())
	{
		OutError = TEXT("'variables' must be an array");
		return false;
	}

	for (const auto& VarNode : VarsNode)
	{
		FNarrRailVariableDefinition VarDef;

		if (!VarNode["name"])
		{
			OutError = TEXT("Variable missing 'name' field");
			return false;
		}
		VarDef.VariableName = FName(UTF8_TO_TCHAR(VarNode["name"].as<std::string>().c_str()));

		if (!VarNode["type"])
		{
			OutError = TEXT("Variable missing 'type' field");
			return false;
		}
		FString TypeStr = UTF8_TO_TCHAR(VarNode["type"].as<std::string>().c_str());
		VarDef.VariableType = ParseVariableType(TypeStr);

		// Scope (optional, default to Session)
		if (VarNode["scope"])
		{
			FString ScopeStr = UTF8_TO_TCHAR(VarNode["scope"].as<std::string>().c_str());
			VarDef.bGlobalScope = (ScopeStr == TEXT("Global"));
		}

		// Default value (optional)
		if (VarNode["defaultValue"])
		{
			VarDef.DefaultValue = UTF8_TO_TCHAR(VarNode["defaultValue"].as<std::string>().c_str());
		}

		OutVariables.Add(VarDef);
	}

	return true;
}

static bool ParsePresetSpeakers(const YAML::Node& SpeakersNode, TArray<FNarrRailPresetSpeaker>& OutSpeakers, FString& OutError)
{
	if (!SpeakersNode.IsSequence())
	{
		OutError = TEXT("'presetSpeakers' must be an array");
		return false;
	}

	for (const auto& SpeakerNode : SpeakersNode)
	{
		FNarrRailPresetSpeaker Speaker;

		if (SpeakerNode.IsScalar())
		{
			Speaker.SpeakerId = FName(UTF8_TO_TCHAR(SpeakerNode.as<std::string>().c_str()));
		}
		else
		{
			if (!SpeakerNode["id"] && !SpeakerNode["speakerId"])
			{
				OutError = TEXT("Preset speaker missing 'id' or 'speakerId' field");
				return false;
			}

			const YAML::Node IdNode = SpeakerNode["speakerId"] ? SpeakerNode["speakerId"] : SpeakerNode["id"];
			Speaker.SpeakerId = FName(UTF8_TO_TCHAR(IdNode.as<std::string>().c_str()));
			if (SpeakerNode["displayName"])
			{
				Speaker.DisplayName = UTF8_TO_TCHAR(SpeakerNode["displayName"].as<std::string>().c_str());
			}
			if (SpeakerNode["color"])
			{
				const FString ColorText = UTF8_TO_TCHAR(SpeakerNode["color"].as<std::string>().c_str());
				if (ColorText.StartsWith(TEXT("#")))
				{
					Speaker.Color = FLinearColor(FColor::FromHex(ColorText.RightChop(1)));
				}
				else
				{
					Speaker.Color = FLinearColor::White;
					Speaker.Color.InitFromString(ColorText);
				}
			}
		}

		if (Speaker.SpeakerId.IsNone())
		{
			OutError = TEXT("Preset speaker id must not be empty");
			return false;
		}

		OutSpeakers.Add(Speaker);
	}

	return true;
}

// Parse nodes section
static bool ParseNodes(const YAML::Node& NodesNode, FNarrRailScriptData& OutData, FString& OutError)
{
	if (!NodesNode.IsSequence())
	{
		OutError = TEXT("'nodes' must be an array");
		return false;
	}

	for (const auto& NodeYaml : NodesNode)
	{
		FNarrRailNode Node;

		if (!NodeYaml["nodeId"])
		{
			OutError = TEXT("Node missing 'nodeId' field");
			return false;
		}
		Node.NodeId = FName(UTF8_TO_TCHAR(NodeYaml["nodeId"].as<std::string>().c_str()));

		if (!NodeYaml["nodeType"])
		{
			OutError = TEXT("Node missing 'nodeType' field");
			return false;
		}
		FString TypeStr = UTF8_TO_TCHAR(NodeYaml["nodeType"].as<std::string>().c_str());
		Node.NodeType = ParseNodeType(TypeStr);

		// Parse dialogue (if present)
		if (NodeYaml["dialogue"] && !NodeYaml["dialogue"].IsNull())
		{
			ParseDialogue(NodeYaml["dialogue"], Node.Dialogue);
		}

		// Parse multi dialogue (if present)
		if (NodeYaml["multiDialogue"] && !NodeYaml["multiDialogue"].IsNull())
		{
			ParseMultiDialogue(NodeYaml["multiDialogue"], Node.MultiDialogue);
		}

		// Parse choices (if present)
		if (NodeYaml["choices"] && NodeYaml["choices"].IsSequence())
		{
			if (!ParseChoices(NodeYaml["choices"], Node.Choices, OutError))
			{
				return false;
			}
		}

		// Parse choice mode & completion target (if present)
		if (NodeYaml["choiceMode"])
		{
			FString ChoiceModeStr = UTF8_TO_TCHAR(NodeYaml["choiceMode"].as<std::string>().c_str());
			Node.ChoiceMode = ParseChoiceMode(ChoiceModeStr);
		}

		if (NodeYaml["choiceCompletionTargetNodeId"])
		{
			Node.ChoiceCompletionTargetNodeId = FName(UTF8_TO_TCHAR(NodeYaml["choiceCompletionTargetNodeId"].as<std::string>().c_str()));
		}

		// Parse jump target (if present)
		if (NodeYaml["jumpTargetNodeId"])
		{
			Node.JumpTargetNodeId = FName(UTF8_TO_TCHAR(NodeYaml["jumpTargetNodeId"].as<std::string>().c_str()));
		}
		else if (NodeYaml["jump"] && NodeYaml["jump"]["targetNodeId"])
		{
			// Compatibility with web editor export format
			Node.JumpTargetNodeId = FName(UTF8_TO_TCHAR(NodeYaml["jump"]["targetNodeId"].as<std::string>().c_str()));
		}

		// Parse enter actions (if present)
		if (NodeYaml["enterActions"] && NodeYaml["enterActions"].IsSequence())
		{
			ParseActions(NodeYaml["enterActions"], Node.EnterActions);
		}

		if (NodeYaml["actions"] && NodeYaml["actions"].IsSequence())
		{
			ParseActions(NodeYaml["actions"], Node.EnterActions);
		}

		// Parse exit actions (if present)
		if (NodeYaml["exitActions"] && NodeYaml["exitActions"].IsSequence())
		{
			ParseActions(NodeYaml["exitActions"], Node.ExitActions);
		}

		// Parse condition expression (Condition node)
		if (NodeYaml["condition"] && !NodeYaml["condition"].IsNull())
		{
			ParseCondition(NodeYaml["condition"], Node.Condition);
		}

		OutData.Nodes.Add(Node);
	}

	return true;
}

// Parse edges section
static bool ParseEdges(const YAML::Node& EdgesNode, FNarrRailScriptData& OutData, FString& OutError)
{
	if (!EdgesNode.IsSequence())
	{
		OutError = TEXT("'edges' must be an array");
		return false;
	}

	TArray<FParsedNarrRailEdge> ParsedEdges;
	int32 OriginalIndex = 0;

	for (const auto& EdgeYaml : EdgesNode)
	{
		FParsedNarrRailEdge ParsedEdge;
		FNarrRailNodeEdge& Edge = ParsedEdge.Edge;
		ParsedEdge.OriginalIndex = OriginalIndex++;

		if (!EdgeYaml["sourceNodeId"])
		{
			OutError = TEXT("Edge missing 'sourceNodeId' field");
			return false;
		}
		Edge.SourceNodeId = FName(UTF8_TO_TCHAR(EdgeYaml["sourceNodeId"].as<std::string>().c_str()));

		if (!EdgeYaml["targetNodeId"])
		{
			OutError = TEXT("Edge missing 'targetNodeId' field");
			return false;
		}
		Edge.TargetNodeId = FName(UTF8_TO_TCHAR(EdgeYaml["targetNodeId"].as<std::string>().c_str()));

		// Priority (optional, default 0)
		if (EdgeYaml["priority"])
		{
			Edge.Priority = EdgeYaml["priority"].as<int>();
		}

		if (EdgeYaml["condition"])
		{
			ParsedEdge.bHasDeprecatedCondition = true;
			ParseCondition(EdgeYaml["condition"], ParsedEdge.DeprecatedCondition);
		}

		// Source handle (optional; used by Choice / Condition dedicated outputs)
		if (EdgeYaml["sourceHandle"])
		{
			Edge.SourceHandle = FName(UTF8_TO_TCHAR(EdgeYaml["sourceHandle"].as<std::string>().c_str()));
			if (Edge.SourceHandle == FName(TEXT("condition-true")))
			{
				Edge.SourceHandle = FName(TEXT("condition-0"));
			}
			else if (Edge.SourceHandle == FName(TEXT("condition-false")))
			{
				Edge.SourceHandle = FName(TEXT("condition-fallback"));
			}
		}

		ParsedEdges.Add(ParsedEdge);
	}

	TMap<FString, TArray<int32>> EdgeGroups;
	for (int32 EdgeIndex = 0; EdgeIndex < ParsedEdges.Num(); ++EdgeIndex)
	{
		const FNarrRailNodeEdge& Edge = ParsedEdges[EdgeIndex].Edge;
		const FString GroupKey = FString::Printf(TEXT("%s|%s"), *Edge.SourceNodeId.ToString(), *Edge.SourceHandle.ToString());
		EdgeGroups.FindOrAdd(GroupKey).Add(EdgeIndex);
	}

	int32 MigratedConditionIndex = 0;
	for (const TPair<FString, TArray<int32>>& Pair : EdgeGroups)
	{
		TArray<int32> GroupIndices = Pair.Value;
		GroupIndices.Sort([&ParsedEdges](const int32 LeftIndex, const int32 RightIndex)
		{
			const FParsedNarrRailEdge& Left = ParsedEdges[LeftIndex];
			const FParsedNarrRailEdge& Right = ParsedEdges[RightIndex];
			if (Left.Edge.Priority != Right.Edge.Priority)
			{
				return Left.Edge.Priority < Right.Edge.Priority;
			}

			return Left.OriginalIndex < Right.OriginalIndex;
		});

		bool bGroupHasDeprecatedCondition = false;
		for (const int32 EdgeIndex : GroupIndices)
		{
			if (ParsedEdges[EdgeIndex].bHasDeprecatedCondition)
			{
				bGroupHasDeprecatedCondition = true;
				break;
			}
		}

		if (!bGroupHasDeprecatedCondition)
		{
			for (const int32 EdgeIndex : GroupIndices)
			{
				OutData.Edges.Add(ParsedEdges[EdgeIndex].Edge);
			}
			continue;
		}

		FName CurrentSourceNodeId = ParsedEdges[GroupIndices[0]].Edge.SourceNodeId;
		FName CurrentSourceHandle = ParsedEdges[GroupIndices[0]].Edge.SourceHandle;
		int32 ChainPriority = ParsedEdges[GroupIndices[0]].Edge.Priority;

		for (const int32 EdgeIndex : GroupIndices)
		{
			const FParsedNarrRailEdge& ParsedEdge = ParsedEdges[EdgeIndex];
			const FNarrRailNodeEdge& OriginalEdge = ParsedEdge.Edge;

			if (!ParsedEdge.bHasDeprecatedCondition)
			{
				FNarrRailNodeEdge FallbackEdge = OriginalEdge;
				FallbackEdge.SourceNodeId = CurrentSourceNodeId;
				FallbackEdge.SourceHandle = CurrentSourceHandle;
				FallbackEdge.Priority = ChainPriority;
				OutData.Edges.Add(FallbackEdge);
				break;
			}

			FNarrRailNode ConditionNode;
			ConditionNode.NodeId = FName(*FString::Printf(
				TEXT("%s__migrated_condition_%d"),
				*OriginalEdge.SourceNodeId.ToString(),
				MigratedConditionIndex++));
			ConditionNode.NodeType = ENarrRailNodeType::Condition;
			ConditionNode.Condition = ParsedEdge.DeprecatedCondition;

			if (ConditionNode.Condition.Branches.Num() == 0)
			{
				FNarrRailConditionBranch Branch;
				Branch.Label = TEXT("Migrated edge condition");
				Branch.Logic = ConditionNode.Condition.Logic;
				Branch.Terms = ConditionNode.Condition.Terms;
				ConditionNode.Condition.Branches.Add(Branch);
			}

			OutData.Nodes.Add(ConditionNode);

			FNarrRailNodeEdge ToConditionEdge;
			ToConditionEdge.SourceNodeId = CurrentSourceNodeId;
			ToConditionEdge.TargetNodeId = ConditionNode.NodeId;
			ToConditionEdge.SourceHandle = CurrentSourceHandle;
			ToConditionEdge.Priority = ChainPriority;
			OutData.Edges.Add(ToConditionEdge);

			FNarrRailNodeEdge TrueEdge = OriginalEdge;
			TrueEdge.SourceNodeId = ConditionNode.NodeId;
			TrueEdge.SourceHandle = FName(TEXT("condition-0"));
			TrueEdge.Priority = 0;
			OutData.Edges.Add(TrueEdge);

			CurrentSourceNodeId = ConditionNode.NodeId;
			CurrentSourceHandle = FName(TEXT("condition-fallback"));
			ChainPriority = 0;
		}
	}

	return true;
}

// Parse dialogue payload
static bool ParseDialogue(const YAML::Node& DialogueNode, FNarrRailDialoguePayload& OutDialogue)
{
	if (DialogueNode["speakerId"])
	{
		OutDialogue.SpeakerId = FName(UTF8_TO_TCHAR(DialogueNode["speakerId"].as<std::string>().c_str()));
	}

	if (DialogueNode["textKey"])
	{
		OutDialogue.TextKey = UTF8_TO_TCHAR(DialogueNode["textKey"].as<std::string>().c_str());
	}

	if (DialogueNode["speechRate"])
	{
		OutDialogue.SpeechRate = DialogueNode["speechRate"].as<float>();
	}

	// VoiceAsset is optional and handled as soft object ptr
	// We don't parse it here as it requires asset path resolution

	return true;
}

// Parse multi dialogue payload
static bool ParseMultiDialogue(const YAML::Node& MultiDialogueNode, FNarrRailMultiDialoguePayload& OutMultiDialogue)
{
	if (MultiDialogueNode["speakerId"])
	{
		OutMultiDialogue.SpeakerId = FName(UTF8_TO_TCHAR(MultiDialogueNode["speakerId"].as<std::string>().c_str()));
	}

	OutMultiDialogue.Lines.Empty();
	if (MultiDialogueNode["lines"] && MultiDialogueNode["lines"].IsSequence())
	{
		for (const auto& LineYaml : MultiDialogueNode["lines"])
		{
			FNarrRailDialogueLine Line;
			if (LineYaml.IsScalar())
			{
				Line.TextKey = UTF8_TO_TCHAR(LineYaml.as<std::string>().c_str());
			}
			else if (LineYaml["textKey"])
			{
				Line.TextKey = UTF8_TO_TCHAR(LineYaml["textKey"].as<std::string>().c_str());
			}
			OutMultiDialogue.Lines.Add(Line);
		}
	}

	return true;
}

// Parse choices
static bool ParseChoices(const YAML::Node& ChoicesNode, TArray<FNarrRailChoiceOption>& OutChoices, FString& OutError)
{
	for (const auto& ChoiceYaml : ChoicesNode)
	{
		FNarrRailChoiceOption Choice;

		if (ChoiceYaml["textKey"])
		{
			Choice.TextKey = UTF8_TO_TCHAR(ChoiceYaml["textKey"].as<std::string>().c_str());
		}

		if (ChoiceYaml["targetNodeId"])
		{
			Choice.TargetNodeId = FName(UTF8_TO_TCHAR(ChoiceYaml["targetNodeId"].as<std::string>().c_str()));
		}

		if (ChoiceYaml["availability"])
		{
			OutError = TEXT("Choice availability is deprecated. Use a Condition node before the Choice node to control conditional flow.");
			return false;
		}

		OutChoices.Add(Choice);
	}

	return true;
}

// Parse actions
static bool ParseActions(const YAML::Node& ActionsNode, TArray<FNarrRailNodeAction>& OutActions)
{
	for (const auto& ActionYaml : ActionsNode)
	{
		FNarrRailNodeAction Action;

		if (ActionYaml["actionType"])
		{
			FString ActionTypeStr = UTF8_TO_TCHAR(ActionYaml["actionType"].as<std::string>().c_str());
			Action.ActionType = ParseActionType(ActionTypeStr);
		}

		if (ActionYaml["variable"] && !ActionYaml["variable"].IsNull())
		{
			ParseVariableRef(ActionYaml["variable"], Action.Variable);
		}

		if (ActionYaml["value"])
		{
			Action.Value = UTF8_TO_TCHAR(ActionYaml["value"].as<std::string>().c_str());
		}

		if (ActionYaml["eventId"])
		{
			Action.EventId = FName(UTF8_TO_TCHAR(ActionYaml["eventId"].as<std::string>().c_str()));
		}

		OutActions.Add(Action);
	}

	return true;
}

// Parse condition expression
static bool ParseCondition(const YAML::Node& ConditionNode, FNarrRailConditionExpression& OutCondition)
{
	OutCondition.Terms.Empty();
	OutCondition.Branches.Empty();

	if (ConditionNode["logic"])
	{
		FString LogicStr = UTF8_TO_TCHAR(ConditionNode["logic"].as<std::string>().c_str());
		OutCondition.Logic = ParseLogicType(LogicStr);
	}

	if (ConditionNode["branches"] && ConditionNode["branches"].IsSequence())
	{
		for (const auto& BranchYaml : ConditionNode["branches"])
		{
			FNarrRailConditionBranch Branch;

			if (BranchYaml["label"])
			{
				Branch.Label = UTF8_TO_TCHAR(BranchYaml["label"].as<std::string>().c_str());
			}

			if (BranchYaml["logic"])
			{
				FString LogicStr = UTF8_TO_TCHAR(BranchYaml["logic"].as<std::string>().c_str());
				Branch.Logic = ParseLogicType(LogicStr);
			}

			if (BranchYaml["terms"] && BranchYaml["terms"].IsSequence())
			{
				ParseConditionTerms(BranchYaml["terms"], Branch.Terms);
			}

			OutCondition.Branches.Add(Branch);
		}
	}

	if (ConditionNode["terms"] && ConditionNode["terms"].IsSequence())
	{
		ParseConditionTerms(ConditionNode["terms"], OutCondition.Terms);
	}

	return true;
}

static bool ParseConditionTerms(const YAML::Node& TermsNode, TArray<FNarrRailConditionTerm>& OutTerms)
{
	OutTerms.Empty();

	for (const auto& TermYaml : TermsNode)
	{
		FNarrRailConditionTerm Term;

		if (TermYaml["variable"] && !TermYaml["variable"].IsNull())
		{
			ParseVariableRef(TermYaml["variable"], Term.Variable);
		}

		if (TermYaml["operator"])
		{
			FString OpStr = UTF8_TO_TCHAR(TermYaml["operator"].as<std::string>().c_str());
			Term.Operator = ParseOperator(OpStr);
		}

		if (TermYaml["compareValue"])
		{
			Term.CompareValue = UTF8_TO_TCHAR(TermYaml["compareValue"].as<std::string>().c_str());
		}

		OutTerms.Add(Term);
	}

	return true;
}

// Parse variable reference
static bool ParseVariableRef(const YAML::Node& VarNode, FNarrRailVariableRef& OutVarRef)
{
	if (VarNode["name"])
	{
		OutVarRef.VariableName = FName(UTF8_TO_TCHAR(VarNode["name"].as<std::string>().c_str()));
	}
	else if (VarNode["variableName"])
	{
		OutVarRef.VariableName = FName(UTF8_TO_TCHAR(VarNode["variableName"].as<std::string>().c_str()));
	}

	if (VarNode["type"])
	{
		FString TypeStr = UTF8_TO_TCHAR(VarNode["type"].as<std::string>().c_str());
		OutVarRef.VariableType = ParseVariableType(TypeStr);
	}
	else if (VarNode["variableType"])
	{
		FString TypeStr = UTF8_TO_TCHAR(VarNode["variableType"].as<std::string>().c_str());
		OutVarRef.VariableType = ParseVariableType(TypeStr);
	}

	if (VarNode["scope"])
	{
		FString ScopeStr = UTF8_TO_TCHAR(VarNode["scope"].as<std::string>().c_str());
		OutVarRef.bGlobalScope = (ScopeStr == TEXT("Global"));
	}
	else if (VarNode["bGlobalScope"])
	{
		OutVarRef.bGlobalScope = VarNode["bGlobalScope"].as<bool>();
	}

	return true;
}
