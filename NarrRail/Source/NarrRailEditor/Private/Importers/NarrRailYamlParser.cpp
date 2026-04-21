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

static bool ParseMeta(const YAML::Node& MetaNode, FNarrRailScriptData& OutData, FString& OutError);
static bool ParseVariables(const YAML::Node& VarsNode, FNarrRailScriptData& OutData, FString& OutError);
static bool ParseNodes(const YAML::Node& NodesNode, FNarrRailScriptData& OutData, FString& OutError);
static bool ParseEdges(const YAML::Node& EdgesNode, FNarrRailScriptData& OutData, FString& OutError);

static bool ParseDialogue(const YAML::Node& DialogueNode, FNarrRailDialoguePayload& OutDialogue);
static bool ParseChoices(const YAML::Node& ChoicesNode, TArray<FNarrRailChoiceOption>& OutChoices);
static bool ParseActions(const YAML::Node& ActionsNode, TArray<FNarrRailNodeAction>& OutActions);
static bool ParseCondition(const YAML::Node& ConditionNode, FNarrRailConditionExpression& OutCondition);
static bool ParseVariableRef(const YAML::Node& VarNode, FNarrRailVariableRef& OutVarRef);

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

// Enum conversion helpers
static ENarrRailNodeType ParseNodeType(const FString& TypeStr)
{
	if (TypeStr == TEXT("Dialogue")) return ENarrRailNodeType::Dialogue;
	if (TypeStr == TEXT("Choice")) return ENarrRailNodeType::Choice;
	if (TypeStr == TEXT("Jump")) return ENarrRailNodeType::Jump;
	if (TypeStr == TEXT("SetVariable")) return ENarrRailNodeType::SetVariable;
	if (TypeStr == TEXT("EmitEvent")) return ENarrRailNodeType::EmitEvent;
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

		OutData.Variables.Add(VarDef);
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

		// Parse choices (if present)
		if (NodeYaml["choices"] && NodeYaml["choices"].IsSequence())
		{
			ParseChoices(NodeYaml["choices"], Node.Choices);
		}

		// Parse jump target (if present)
		if (NodeYaml["jumpTargetNodeId"])
		{
			Node.JumpTargetNodeId = FName(UTF8_TO_TCHAR(NodeYaml["jumpTargetNodeId"].as<std::string>().c_str()));
		}

		// Parse enter actions (if present)
		if (NodeYaml["enterActions"] && NodeYaml["enterActions"].IsSequence())
		{
			ParseActions(NodeYaml["enterActions"], Node.EnterActions);
		}

		// Parse exit actions (if present)
		if (NodeYaml["exitActions"] && NodeYaml["exitActions"].IsSequence())
		{
			ParseActions(NodeYaml["exitActions"], Node.ExitActions);
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

	for (const auto& EdgeYaml : EdgesNode)
	{
		FNarrRailNodeEdge Edge;

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

		// Condition (optional)
		if (EdgeYaml["condition"] && !EdgeYaml["condition"].IsNull())
		{
			ParseCondition(EdgeYaml["condition"], Edge.Condition);
		}

		OutData.Edges.Add(Edge);
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

// Parse choices
static bool ParseChoices(const YAML::Node& ChoicesNode, TArray<FNarrRailChoiceOption>& OutChoices)
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

		if (ChoiceYaml["availability"] && !ChoiceYaml["availability"].IsNull())
		{
			ParseCondition(ChoiceYaml["availability"], Choice.Availability);
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
	if (ConditionNode["logic"])
	{
		FString LogicStr = UTF8_TO_TCHAR(ConditionNode["logic"].as<std::string>().c_str());
		OutCondition.Logic = ParseLogicType(LogicStr);
	}

	if (ConditionNode["terms"] && ConditionNode["terms"].IsSequence())
	{
		for (const auto& TermYaml : ConditionNode["terms"])
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

			OutCondition.Terms.Add(Term);
		}
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

	if (VarNode["type"])
	{
		FString TypeStr = UTF8_TO_TCHAR(VarNode["type"].as<std::string>().c_str());
		OutVarRef.VariableType = ParseVariableType(TypeStr);
	}

	if (VarNode["scope"])
	{
		FString ScopeStr = UTF8_TO_TCHAR(VarNode["scope"].as<std::string>().c_str());
		OutVarRef.bGlobalScope = (ScopeStr == TEXT("Global"));
	}

	return true;
}
