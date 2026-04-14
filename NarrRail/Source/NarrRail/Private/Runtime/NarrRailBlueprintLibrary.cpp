#include "Runtime/NarrRailBlueprintLibrary.h"

UNarrRailStoryAsset* UNarrRailBlueprintLibrary::CreateStoryAsset(UObject* WorldContextObject, FName StoryId, FName EntryNodeId)
{
    if (WorldContextObject == nullptr)
    {
        return nullptr;
    }

    UNarrRailStoryAsset* NewAsset = NewObject<UNarrRailStoryAsset>(WorldContextObject);
    if (NewAsset)
    {
        NewAsset->StoryId = StoryId;
        NewAsset->EntryNodeId = EntryNodeId;
        NewAsset->SchemaVersion = UNarrRailStoryAsset::LatestSchemaVersion;
    }

    return NewAsset;
}

bool UNarrRailBlueprintLibrary::AddDialogueNode(UNarrRailStoryAsset* StoryAsset, FName NodeId, FName SpeakerId, const FString& TextKey)
{
    if (StoryAsset == nullptr || NodeId == NAME_None)
    {
        return false;
    }

    FNarrRailNode NewNode;
    NewNode.NodeId = NodeId;
    NewNode.NodeType = ENarrRailNodeType::Dialogue;
    NewNode.Dialogue.SpeakerId = SpeakerId;
    NewNode.Dialogue.TextKey = TextKey;
    NewNode.Dialogue.SpeechRate = 1.0f;

    StoryAsset->Nodes.Add(NewNode);
    return true;
}

bool UNarrRailBlueprintLibrary::AddChoiceNode(UNarrRailStoryAsset* StoryAsset, FName NodeId, const TArray<FNarrRailChoiceOption>& Choices)
{
    if (StoryAsset == nullptr || NodeId == NAME_None)
    {
        return false;
    }

    FNarrRailNode NewNode;
    NewNode.NodeId = NodeId;
    NewNode.NodeType = ENarrRailNodeType::Choice;
    NewNode.Choices = Choices;

    StoryAsset->Nodes.Add(NewNode);
    return true;
}

bool UNarrRailBlueprintLibrary::AddEndNode(UNarrRailStoryAsset* StoryAsset, FName NodeId)
{
    if (StoryAsset == nullptr || NodeId == NAME_None)
    {
        return false;
    }

    FNarrRailNode NewNode;
    NewNode.NodeId = NodeId;
    NewNode.NodeType = ENarrRailNodeType::End;

    StoryAsset->Nodes.Add(NewNode);
    return true;
}

bool UNarrRailBlueprintLibrary::AddEdge(UNarrRailStoryAsset* StoryAsset, FName SourceNodeId, FName TargetNodeId, int32 Priority)
{
    if (StoryAsset == nullptr || SourceNodeId == NAME_None || TargetNodeId == NAME_None)
    {
        return false;
    }

    FNarrRailNodeEdge NewEdge;
    NewEdge.SourceNodeId = SourceNodeId;
    NewEdge.TargetNodeId = TargetNodeId;
    NewEdge.Priority = Priority;

    StoryAsset->Edges.Add(NewEdge);
    return true;
}

FNarrRailChoiceOption UNarrRailBlueprintLibrary::MakeSimpleChoice(const FString& TextKey, FName TargetNodeId)
{
    FNarrRailChoiceOption Choice;
    Choice.TextKey = TextKey;
    Choice.TargetNodeId = TargetNodeId;
    return Choice;
}

UNarrRailStorySession* UNarrRailBlueprintLibrary::CreateStorySession(UObject* WorldContextObject, const UNarrRailStoryAsset* StoryAsset)
{
    if (WorldContextObject == nullptr || StoryAsset == nullptr)
    {
        return nullptr;
    }

    UNarrRailStorySession* NewSession = NewObject<UNarrRailStorySession>(WorldContextObject);
    if (NewSession)
    {
        NewSession->Initialize(StoryAsset);
    }

    return NewSession;
}

bool UNarrRailBlueprintLibrary::IsResultSuccess(const FNarrRailRuntimeResult& Result)
{
    return Result.Code == ENarrRailRuntimeResultCode::Success;
}

bool UNarrRailBlueprintLibrary::IsResultCompleted(const FNarrRailRuntimeResult& Result)
{
    return Result.Code == ENarrRailRuntimeResultCode::Completed;
}

FNarrRailDialoguePayload UNarrRailBlueprintLibrary::GetDialogueFromNode(const FNarrRailNode& Node)
{
    return Node.Dialogue;
}

bool UNarrRailBlueprintLibrary::IsDialogueNode(const FNarrRailNode& Node)
{
    return Node.NodeType == ENarrRailNodeType::Dialogue;
}

bool UNarrRailBlueprintLibrary::IsChoiceNode(const FNarrRailNode& Node)
{
    return Node.NodeType == ENarrRailNodeType::Choice;
}

bool UNarrRailBlueprintLibrary::IsEndNode(const FNarrRailNode& Node)
{
    return Node.NodeType == ENarrRailNodeType::End;
}
