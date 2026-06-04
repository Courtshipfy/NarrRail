#include "Runtime/NarrRailStorySession.h"

#include "Algo/Sort.h"
#include "Engine/World.h"

// 全局活跃会话列表定义
TArray<UNarrRailStorySession*> UNarrRailStorySession::ActiveSessions;

namespace NarrRailRuntime
{
static bool TryParseBool(const FString& InValue, bool& OutValue)
{
    if (InValue.Equals(TEXT("true"), ESearchCase::IgnoreCase) || InValue == TEXT("1"))
    {
        OutValue = true;
        return true;
    }

    if (InValue.Equals(TEXT("false"), ESearchCase::IgnoreCase) || InValue == TEXT("0"))
    {
        OutValue = false;
        return true;
    }

    return false;
}

static bool TryParseInt(const FString& InValue, int32& OutValue)
{
    return LexTryParseString(OutValue, *InValue);
}

static bool TryParseFloat(const FString& InValue, float& OutValue)
{
    return LexTryParseString(OutValue, *InValue);
}
}

void UNarrRailStorySession::BeginDestroy()
{
    ActiveSessions.Remove(this);
    Super::BeginDestroy();
}


FNarrRailRuntimeResult UNarrRailStorySession::Initialize(const UNarrRailStoryAsset* InStoryAsset)
{
    const UNarrRailGlobalConfigAsset* LinkedGlobalConfig = nullptr;
    if (InStoryAsset != nullptr && !InStoryAsset->GlobalConfig.IsNull())
    {
        LinkedGlobalConfig = InStoryAsset->GlobalConfig.LoadSynchronous();
    }

    return InitializeWithGlobalConfig(InStoryAsset, LinkedGlobalConfig);
}

FNarrRailRuntimeResult UNarrRailStorySession::InitializeWithGlobalConfig(const UNarrRailStoryAsset* InStoryAsset, const UNarrRailGlobalConfigAsset* InGlobalConfig)
{
    if (InStoryAsset == nullptr)
    {
        SessionState = ENarrRailSessionState::Error;
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidInput, TEXT("Story asset is null."));
    }

    StoryAsset = InStoryAsset;
    GlobalConfigAsset = InGlobalConfig;
    SessionState = ENarrRailSessionState::Idle;
    StateBeforePause = ENarrRailSessionState::Idle;

    if (VariableContainer == nullptr)
    {
        VariableContainer = NewObject<UNarrRailVariableContainer>(this);
    }

    GlobalVariableContainer = nullptr;
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            if (UNarrRailGlobalStateSubsystem* GlobalState = GameInstance->GetSubsystem<UNarrRailGlobalStateSubsystem>())
            {
                FString GlobalError;
                if (!GlobalState->ApplyGlobalConfig(GlobalConfigAsset, GlobalError))
                {
                    SessionState = ENarrRailSessionState::Error;
                    return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidInput, *GlobalError);
                }

                TArray<FNarrRailVariableDefinition> StoryGlobalDefinitions;
                for (FNarrRailVariableDefinition Definition : StoryAsset->Variables)
                {
                    if (Definition.bGlobalScope)
                    {
                        Definition.bGlobalScope = true;
                        StoryGlobalDefinitions.Add(Definition);
                    }
                }

                if (StoryGlobalDefinitions.Num() > 0)
                {
                    if (UNarrRailVariableContainer* SharedVariables = GlobalState->GetGlobalVariableContainer())
                    {
                        if (!SharedVariables->AddDefinitions(StoryGlobalDefinitions, true, GlobalError))
                        {
                            SessionState = ENarrRailSessionState::Error;
                            return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidInput, *GlobalError);
                        }
                    }
                }

                GlobalVariableContainer = GlobalState->GetGlobalVariableContainer();
            }
        }
    }

    ResetSessionContextFromAsset();

    if (!ActiveSessions.Contains(this))
    {
        ActiveSessions.Add(this);
    }

    return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::Success, TEXT("Story session initialized."));
}

FNarrRailRuntimeResult UNarrRailStorySession::Start(const FName OverrideEntryNodeId)
{
    if (StoryAsset == nullptr)
    {
        SessionState = ENarrRailSessionState::Error;
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidState, TEXT("Session not initialized."));
    }

    const FName EntryNodeId = (OverrideEntryNodeId != NAME_None) ? OverrideEntryNodeId : StoryAsset->EntryNodeId;
    StateBeforePause = ENarrRailSessionState::Idle;
    ResetSessionContextFromAsset();
    CurrentMultiDialogueLineIndex = INDEX_NONE;
    ExhaustiveSelectedChoiceIndices.Reset();
    RuntimeVisibleChoiceIndexMap.Reset();
    ExhaustivePendingChoiceReturnStack.Reset();

    // 重置最后选择信息
    LastChoiceInfo = FNarrRailLastChoiceInfo();

    // 触发会话启动事件
    OnSessionStarted.Broadcast(EntryNodeId);

    return AdvanceToNode(EntryNodeId);
}

FNarrRailRuntimeResult UNarrRailStorySession::Next()
{
    if (StoryAsset == nullptr)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidState, TEXT("Session not initialized."));
    }

    if (SessionState == ENarrRailSessionState::Paused)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidState, TEXT("Session is paused. Call Resume() first."), Context.CurrentNodeId);
    }

    if (SessionState == ENarrRailSessionState::Completed)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::Completed, TEXT("Session already completed."), Context.CurrentNodeId);
    }

    if (SessionState == ENarrRailSessionState::WaitingForChoice)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidState, TEXT("Current node requires Choose()."), Context.CurrentNodeId);
    }

    const FNarrRailNode* Node = FindNode(Context.CurrentNodeId);
    if (Node == nullptr)
    {
        SessionState = ENarrRailSessionState::Error;
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::MissingNode, TEXT("Current node not found."), Context.CurrentNodeId);
    }

    if (Node->NodeType == ENarrRailNodeType::End)
    {
        FName ReturnNodeId;
        if (TryPopExhaustiveReturn(ReturnNodeId))
        {
            return AdvanceToNode(ReturnNodeId);
        }

        SessionState = ENarrRailSessionState::Completed;
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::Completed, TEXT("Reached end node."), Context.CurrentNodeId);
    }

    if (Node->NodeType == ENarrRailNodeType::Choice)
    {
        SessionState = ENarrRailSessionState::WaitingForChoice;
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidState, TEXT("Current node requires Choose()."), Context.CurrentNodeId);
    }

    if (Node->NodeType == ENarrRailNodeType::MultiDialogue)
    {
        const int32 TotalLines = Node->MultiDialogue.Lines.Num();
        if (TotalLines > 0 && CurrentMultiDialogueLineIndex + 1 < TotalLines)
        {
            CurrentMultiDialogueLineIndex += 1;

            UObject* PresenterObject = DialoguePresenter.GetObject();
            const bool bPresenterValid =
                PresenterObject != nullptr &&
                PresenterObject->GetClass()->ImplementsInterface(UNarrRailDialoguePresenterInterface::StaticClass());

            FNarrRailDialogueRequest Request;
            if (BuildMultiDialogueDisplay(*Node, Request))
            {
                if (bPresenterValid)
                {
                    INarrRailDialoguePresenterInterface::Execute_ShowDialogue(PresenterObject, Request);
                }

                FNarrRailNode DisplayNode = *Node;
                DisplayNode.NodeType = ENarrRailNodeType::Dialogue;
                DisplayNode.Dialogue.SpeakerId = Request.SpeakerId;
                DisplayNode.Dialogue.TextKey = Request.TextContent;
                DisplayNode.Dialogue.SpeechRate = Request.SpeechRate;
                OnNodeEntered.Broadcast(Context.CurrentNodeId, DisplayNode);

                SessionState = ENarrRailSessionState::Running;
                return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::Success, TEXT("Advanced to next multi-dialogue line."), Context.CurrentNodeId);
            }
        }
    }

    FString ActionError;
    if (!ExecuteActions(Node->ExitActions, ActionError))
    {
        SessionState = ENarrRailSessionState::Error;
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidInput, *ActionError, Context.CurrentNodeId);
    }

    // 触发节点退出事件
    OnNodeExited.Broadcast(Context.CurrentNodeId, *Node);

    if (Node->NodeType == ENarrRailNodeType::Jump)
    {
        if (Node->JumpTargetNodeId == NAME_None)
        {
            SessionState = ENarrRailSessionState::Error;
            return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidInput, TEXT("Jump node has empty target."), Context.CurrentNodeId);
        }

        return AdvanceToNode(Node->JumpTargetNodeId);
    }

    FName NextNodeId = NAME_None;
    const FNarrRailRuntimeResult ResolveResult = ResolveNextByEdge(*Node, NextNodeId);
    if (ResolveResult.Code != ENarrRailRuntimeResultCode::Success)
    {
        if (ResolveResult.Code == ENarrRailRuntimeResultCode::Completed)
        {
            FName ReturnNodeId;
            if (TryPopExhaustiveReturn(ReturnNodeId))
            {
                return AdvanceToNode(ReturnNodeId);
            }

            SessionState = ENarrRailSessionState::Completed;
        }
        return ResolveResult;
    }

    return AdvanceToNode(NextNodeId);
}

FNarrRailRuntimeResult UNarrRailStorySession::Choose(const int32 ChoiceIndex)
{
    if (StoryAsset == nullptr)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidState, TEXT("Session not initialized."));
    }

    if (SessionState == ENarrRailSessionState::Paused)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidState, TEXT("Session is paused. Call Resume() first."), Context.CurrentNodeId);
    }

    const FNarrRailNode* Node = FindNode(Context.CurrentNodeId);
    if (Node == nullptr)
    {
        SessionState = ENarrRailSessionState::Error;
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::MissingNode, TEXT("Current node not found."), Context.CurrentNodeId);
    }

    if (Node->NodeType != ENarrRailNodeType::Choice)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidState, TEXT("Current node is not a choice node."), Context.CurrentNodeId);
    }

    const TArray<int32>* VisibleIndexMap = RuntimeVisibleChoiceIndexMap.Find(Context.CurrentNodeId);
    const TArray<int32> AvailableIndices = BuildAvailableChoiceIndices(*Node);

    int32 ActualChoiceIndex = INDEX_NONE;

    if (VisibleIndexMap != nullptr)
    {
        // VisibleIndexMap 映射: UI 可见位置(0-based) → 原始Choices索引
        // ChoiceIndex 必须是 0-based 进入可见列表，不再做模糊的 1-based 猜测
        const int32 NumVisible = VisibleIndexMap->Num();

        if (ChoiceIndex >= 0 && ChoiceIndex < NumVisible)
        {
            ActualChoiceIndex = (*VisibleIndexMap)[ChoiceIndex];
        }
        else
        {
            return FNarrRailRuntimeResult::Make(
                ENarrRailRuntimeResultCode::InvalidInput,
                *FString::Printf(TEXT("Choice index %d out of visible range [0, %d)."), ChoiceIndex, NumVisible),
                Context.CurrentNodeId);
        }
    }
    else
    {
        // 无 VisibleIndexMap 时退化为原始索引，兼容 0/1-based
        if (Node->Choices.IsValidIndex(ChoiceIndex))
        {
            ActualChoiceIndex = ChoiceIndex;
        }
        else if (ChoiceIndex > 0 && Node->Choices.IsValidIndex(ChoiceIndex - 1))
        {
            // 兼容 1-based UI 传参（仅限无 VisibleIndexMap 的回退路径）
            ActualChoiceIndex = ChoiceIndex - 1;
        }
        else
        {
            return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidInput, TEXT("Choice index out of range."), Context.CurrentNodeId);
        }
    }

    if (!AvailableIndices.Contains(ActualChoiceIndex))
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidInput, TEXT("Choice is not available."), Context.CurrentNodeId);
    }

    const FNarrRailChoiceOption& Option = Node->Choices[ActualChoiceIndex];
    if (Option.TargetNodeId == NAME_None)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidInput, TEXT("Choice target node is empty."), Context.CurrentNodeId);
    }

    // 更新最后一次选择的信息
    LastChoiceInfo.ChoiceNodeId = Context.CurrentNodeId;
    LastChoiceInfo.ChoiceIndex = ActualChoiceIndex;
    LastChoiceInfo.TargetNodeId = Option.TargetNodeId;
    LastChoiceInfo.ChoiceTextKey = Option.TextKey;
    LastChoiceInfo.bValid = true;

    // 触发选择事件
    OnChoiceSelected.Broadcast(Context.CurrentNodeId, ActualChoiceIndex, Option.TargetNodeId);

    if (Node->ChoiceMode == ENarrRailChoiceMode::ExhaustiveUntilComplete)
    {
        TSet<int32>& Selected = ExhaustiveSelectedChoiceIndices.FindOrAdd(Context.CurrentNodeId);
        Selected.Add(ActualChoiceIndex);
        ExhaustivePendingChoiceReturnStack.Add(Context.CurrentNodeId);
    }

    FString ActionError;
    if (!ExecuteActions(Node->ExitActions, ActionError))
    {
        SessionState = ENarrRailSessionState::Error;
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidInput, *ActionError, Context.CurrentNodeId);
    }

    return AdvanceToNode(Option.TargetNodeId);
}

FNarrRailRuntimeResult UNarrRailStorySession::Pause()
{
    if (StoryAsset == nullptr)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidState, TEXT("Session not initialized."));
    }

    if (SessionState == ENarrRailSessionState::Paused)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidState, TEXT("Session already paused."), Context.CurrentNodeId);
    }

    if (SessionState == ENarrRailSessionState::Completed || SessionState == ENarrRailSessionState::Error)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidState, TEXT("Cannot pause completed or error session."), Context.CurrentNodeId);
    }

    StateBeforePause = SessionState;
    SessionState = ENarrRailSessionState::Paused;
    return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::Success, TEXT("Session paused."), Context.CurrentNodeId);
}

FNarrRailRuntimeResult UNarrRailStorySession::Resume()
{
    if (StoryAsset == nullptr)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidState, TEXT("Session not initialized."));
    }

    if (SessionState != ENarrRailSessionState::Paused)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidState, TEXT("Session is not paused."), Context.CurrentNodeId);
    }

    SessionState = StateBeforePause;
    return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::Success, TEXT("Session resumed."), Context.CurrentNodeId);
}

FNarrRailRuntimeResult UNarrRailStorySession::Stop()
{
    if (StoryAsset == nullptr)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidState, TEXT("Session not initialized."));
    }

    const FNarrRailNode* Node = FindNode(Context.CurrentNodeId);
    if (Node != nullptr)
    {
        FString ActionError;
        if (!ExecuteActions(Node->ExitActions, ActionError))
        {
            SessionState = ENarrRailSessionState::Error;
            return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidInput, *ActionError, Context.CurrentNodeId);
        }
    }

    SessionState = ENarrRailSessionState::Completed;

    // 触发会话结束事件
    OnSessionEnded.Broadcast(Context.CurrentNodeId, SessionState);

    return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::Completed, TEXT("Session stopped."), Context.CurrentNodeId);
}

int32 UNarrRailStorySession::GetCurrentMultiDialogueTotalLines() const
{
    const FNarrRailNode* Node = FindNode(Context.CurrentNodeId);
    if (Node == nullptr || Node->NodeType != ENarrRailNodeType::MultiDialogue)
    {
        return 0;
    }

    return Node->MultiDialogue.Lines.Num();
}

bool UNarrRailStorySession::IsCurrentNodeMultiDialogue() const
{
    const FNarrRailNode* Node = FindNode(Context.CurrentNodeId);
    return Node != nullptr && Node->NodeType == ENarrRailNodeType::MultiDialogue;
}

bool UNarrRailStorySession::GetCurrentNode(FNarrRailNode& OutNode) const
{
    const FNarrRailNode* Node = FindNode(Context.CurrentNodeId);
    if (Node == nullptr)
    {
        return false;
    }

    OutNode = *Node;
    return true;
}

TArray<FNarrRailChoiceOption> UNarrRailStorySession::GetCurrentChoices() const
{
    const FNarrRailNode* Node = FindNode(Context.CurrentNodeId);
    if (Node == nullptr || Node->NodeType != ENarrRailNodeType::Choice)
    {
        return {};
    }

    return BuildVisibleChoiceOptions(*Node);
}

// 便捷的变量访问接口实现
FNarrRailVariableResult UNarrRailStorySession::GetVariableBool(FName VariableName, bool& OutValue) const
{
    UNarrRailVariableContainer* Container = FindVariableContainerForName(VariableName);
    return Container != nullptr
        ? Container->GetBool(VariableName, OutValue)
        : FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound, TEXT("Variable container not initialized."));
}

FNarrRailVariableResult UNarrRailStorySession::GetVariableInt(FName VariableName, int32& OutValue) const
{
    UNarrRailVariableContainer* Container = FindVariableContainerForName(VariableName);
    return Container != nullptr
        ? Container->GetInt(VariableName, OutValue)
        : FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound, TEXT("Variable container not initialized."));
}

FNarrRailVariableResult UNarrRailStorySession::GetVariableFloat(FName VariableName, float& OutValue) const
{
    UNarrRailVariableContainer* Container = FindVariableContainerForName(VariableName);
    return Container != nullptr
        ? Container->GetFloat(VariableName, OutValue)
        : FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound, TEXT("Variable container not initialized."));
}

FNarrRailVariableResult UNarrRailStorySession::GetVariableString(FName VariableName, FString& OutValue) const
{
    UNarrRailVariableContainer* Container = FindVariableContainerForName(VariableName);
    return Container != nullptr
        ? Container->GetString(VariableName, OutValue)
        : FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound, TEXT("Variable container not initialized."));
}

FNarrRailVariableResult UNarrRailStorySession::SetVariableBool(FName VariableName, bool Value)
{
    UNarrRailVariableContainer* Container = FindVariableContainerForName(VariableName);
    FNarrRailVariableResult Result = Container != nullptr
        ? Container->SetBool(VariableName, Value)
        : FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound, TEXT("Variable container not initialized."));
    SyncVariableSnapshotToContext();
    return Result;
}

FNarrRailVariableResult UNarrRailStorySession::SetVariableInt(FName VariableName, int32 Value)
{
    UNarrRailVariableContainer* Container = FindVariableContainerForName(VariableName);
    FNarrRailVariableResult Result = Container != nullptr
        ? Container->SetInt(VariableName, Value)
        : FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound, TEXT("Variable container not initialized."));
    SyncVariableSnapshotToContext();
    return Result;
}

FNarrRailVariableResult UNarrRailStorySession::SetVariableFloat(FName VariableName, float Value)
{
    UNarrRailVariableContainer* Container = FindVariableContainerForName(VariableName);
    FNarrRailVariableResult Result = Container != nullptr
        ? Container->SetFloat(VariableName, Value)
        : FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound, TEXT("Variable container not initialized."));
    SyncVariableSnapshotToContext();
    return Result;
}

FNarrRailVariableResult UNarrRailStorySession::SetVariableString(FName VariableName, const FString& Value)
{
    UNarrRailVariableContainer* Container = FindVariableContainerForName(VariableName);
    FNarrRailVariableResult Result = Container != nullptr
        ? Container->SetString(VariableName, Value)
        : FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound, TEXT("Variable container not initialized."));
    SyncVariableSnapshotToContext();
    return Result;
}

bool UNarrRailStorySession::GetPresetSpeaker(FName SpeakerId, FNarrRailPresetSpeaker& OutSpeaker) const
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            if (const UNarrRailGlobalStateSubsystem* GlobalState = GameInstance->GetSubsystem<UNarrRailGlobalStateSubsystem>())
            {
                return GlobalState->GetPresetSpeaker(SpeakerId, OutSpeaker);
            }
        }
    }

    return false;
}

FString UNarrRailStorySession::ResolveSpeakerDisplayName(FName SpeakerId) const
{
    FNarrRailPresetSpeaker Speaker;
    if (GetPresetSpeaker(SpeakerId, Speaker) && !Speaker.DisplayName.IsEmpty())
    {
        return Speaker.DisplayName;
    }

    return SpeakerId.ToString();
}

FNarrRailStorySessionSnapshot UNarrRailStorySession::GetSessionSnapshot() const
{
    FNarrRailStorySessionSnapshot Snapshot;
    Snapshot.StoryId = StoryAsset != nullptr ? StoryAsset->StoryId : NAME_None;
    Snapshot.StoryAssetPath = StoryAsset != nullptr ? FSoftObjectPath(StoryAsset) : FSoftObjectPath();
    Snapshot.GlobalConfigPath = GlobalConfigAsset != nullptr ? FSoftObjectPath(GlobalConfigAsset) : FSoftObjectPath();
    Snapshot.SessionState = SessionState;
    Snapshot.StateBeforePause = StateBeforePause;
    Snapshot.CurrentNodeId = Context.CurrentNodeId;
    Snapshot.NodeHistory = Context.NodeHistory;
    Snapshot.EmittedEvents = Context.EmittedEvents;
    Snapshot.CurrentMultiDialogueLineIndex = CurrentMultiDialogueLineIndex;
    Snapshot.LastChoiceInfo = LastChoiceInfo;
    Snapshot.ExhaustivePendingChoiceReturnStack = ExhaustivePendingChoiceReturnStack;

    if (VariableContainer != nullptr)
    {
        Snapshot.LocalVariableSnapshot = VariableContainer->GetSnapshot();
    }

    for (const TPair<FName, TSet<int32>>& Pair : ExhaustiveSelectedChoiceIndices)
    {
        FNarrRailChoiceSelectionSnapshot& ChoiceSnapshot = Snapshot.ExhaustiveChoiceSelections.AddDefaulted_GetRef();
        ChoiceSnapshot.ChoiceNodeId = Pair.Key;
        ChoiceSnapshot.SelectedChoiceIndices = Pair.Value.Array();
        ChoiceSnapshot.SelectedChoiceIndices.Sort();
    }

    return Snapshot;
}

FNarrRailRuntimeResult UNarrRailStorySession::RestoreSessionSnapshot(const FNarrRailStorySessionSnapshot& Snapshot, const bool bRefreshPresenter)
{
    if (Snapshot.SnapshotVersion <= 0)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidInput, TEXT("Invalid NarrRail session snapshot version."));
    }

    if (StoryAsset == nullptr)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidState, TEXT("Session not initialized."));
    }

    if (Snapshot.StoryId != NAME_None && StoryAsset->StoryId != NAME_None && Snapshot.StoryId != StoryAsset->StoryId)
    {
        return FNarrRailRuntimeResult::Make(
            ENarrRailRuntimeResultCode::InvalidInput,
            *FString::Printf(TEXT("Snapshot StoryId '%s' does not match current StoryId '%s'."), *Snapshot.StoryId.ToString(), *StoryAsset->StoryId.ToString()));
    }

    if (Snapshot.CurrentNodeId != NAME_None && FindNode(Snapshot.CurrentNodeId) == nullptr)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::MissingNode, TEXT("Snapshot current node does not exist."), Snapshot.CurrentNodeId);
    }

    SessionState = Snapshot.SessionState;
    StateBeforePause = Snapshot.StateBeforePause;
    Context.CurrentNodeId = Snapshot.CurrentNodeId;
    Context.NodeHistory = Snapshot.NodeHistory;
    Context.EmittedEvents = Snapshot.EmittedEvents;
    CurrentMultiDialogueLineIndex = Snapshot.CurrentMultiDialogueLineIndex;
    LastChoiceInfo = Snapshot.LastChoiceInfo;
    ExhaustivePendingChoiceReturnStack = Snapshot.ExhaustivePendingChoiceReturnStack;
    RuntimeVisibleChoiceIndexMap.Reset();

    ExhaustiveSelectedChoiceIndices.Reset();
    for (const FNarrRailChoiceSelectionSnapshot& ChoiceSnapshot : Snapshot.ExhaustiveChoiceSelections)
    {
        if (ChoiceSnapshot.ChoiceNodeId == NAME_None)
        {
            continue;
        }

        TSet<int32>& Selected = ExhaustiveSelectedChoiceIndices.FindOrAdd(ChoiceSnapshot.ChoiceNodeId);
        for (const int32 ChoiceIndex : ChoiceSnapshot.SelectedChoiceIndices)
        {
            if (ChoiceIndex >= 0)
            {
                Selected.Add(ChoiceIndex);
            }
        }
    }

    if (VariableContainer != nullptr)
    {
        VariableContainer->RestoreFromSnapshot(Snapshot.LocalVariableSnapshot);
    }
    SyncVariableSnapshotToContext();

    const FNarrRailNode* CurrentNode = FindNode(Context.CurrentNodeId);
    if (CurrentNode != nullptr && CurrentNode->NodeType == ENarrRailNodeType::Choice)
    {
        RuntimeVisibleChoiceIndexMap.Add(Context.CurrentNodeId, BuildVisibleChoiceIndices(*CurrentNode));
    }

    if (bRefreshPresenter)
    {
        RefreshCurrentNodeAfterRestore();
    }

    return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::Success, TEXT("NarrRail session snapshot restored."), Context.CurrentNodeId);
}

void UNarrRailStorySession::RegisterDialoguePresenter(TScriptInterface<INarrRailDialoguePresenterInterface> Presenter)
{
    DialoguePresenter = Presenter;

    UObject* PresenterObject = DialoguePresenter.GetObject();
    const bool bImplementsInterface =
        PresenterObject != nullptr &&
        PresenterObject->GetClass()->ImplementsInterface(UNarrRailDialoguePresenterInterface::StaticClass());

    UE_LOG(LogTemp, Log, TEXT("[NarrRail] RegisterDialoguePresenter: Object=%s, InterfacePtrValid=%s, ImplementsInterface=%s"),
        PresenterObject ? *PresenterObject->GetName() : TEXT("None"),
        DialoguePresenter.GetInterface() ? TEXT("true") : TEXT("false"),
        bImplementsInterface ? TEXT("true") : TEXT("false"));
}

void UNarrRailStorySession::UnregisterDialoguePresenter()
{
    DialoguePresenter = nullptr;
}

void UNarrRailStorySession::ResetSessionContextFromAsset()
{
    Context = FNarrRailSessionContext{};
    CurrentMultiDialogueLineIndex = INDEX_NONE;
    RuntimeVisibleChoiceIndexMap.Reset();
    ExhaustivePendingChoiceReturnStack.Reset();

    if (StoryAsset == nullptr || VariableContainer == nullptr)
    {
        return;
    }

    TArray<FNarrRailVariableDefinition> LocalDefinitions;
    for (const FNarrRailVariableDefinition& Definition : StoryAsset->Variables)
    {
        if (!Definition.bGlobalScope)
        {
            LocalDefinitions.Add(Definition);
        }
    }

    VariableContainer->Initialize(LocalDefinitions);
    SyncVariableSnapshotToContext();
}

void UNarrRailStorySession::SyncVariableSnapshotToContext()
{
    Context.VariableSnapshot.Reset();

    if (GlobalVariableContainer != nullptr)
    {
        Context.VariableSnapshot.Append(GlobalVariableContainer->GetSnapshot());
    }

    if (VariableContainer != nullptr)
    {
        Context.VariableSnapshot.Append(VariableContainer->GetSnapshot());
    }
}

UNarrRailVariableContainer* UNarrRailStorySession::FindVariableContainerForName(const FName VariableName) const
{
    if (VariableContainer != nullptr && VariableContainer->HasVariable(VariableName))
    {
        return VariableContainer;
    }

    if (GlobalVariableContainer != nullptr && GlobalVariableContainer->HasVariable(VariableName))
    {
        return GlobalVariableContainer;
    }

    return VariableContainer;
}

void UNarrRailStorySession::RefreshCurrentNodeAfterRestore()
{
    const FNarrRailNode* Node = FindNode(Context.CurrentNodeId);
    if (Node == nullptr)
    {
        return;
    }

    UObject* PresenterObject = DialoguePresenter.GetObject();
    const bool bPresenterValid =
        PresenterObject != nullptr &&
        PresenterObject->GetClass()->ImplementsInterface(UNarrRailDialoguePresenterInterface::StaticClass());

    FNarrRailNode NodeForEvent = *Node;

    if (bPresenterValid)
    {
        if (Node->NodeType == ENarrRailNodeType::Dialogue)
        {
            FNarrRailDialogueRequest Request;
            Request.NodeId = Node->NodeId;
            Request.SpeakerId = Node->Dialogue.SpeakerId;
            Request.TextContent = Node->Dialogue.TextKey;
            Request.SpeechRate = Node->Dialogue.SpeechRate;
            Request.VoiceAsset = Node->Dialogue.VoiceAsset;
            Request.bAutoAdvance = false;
            INarrRailDialoguePresenterInterface::Execute_ShowDialogue(PresenterObject, Request);
        }
        else if (Node->NodeType == ENarrRailNodeType::MultiDialogue)
        {
            FNarrRailDialogueRequest Request;
            if (BuildMultiDialogueDisplay(*Node, Request))
            {
                INarrRailDialoguePresenterInterface::Execute_ShowDialogue(PresenterObject, Request);

                NodeForEvent.NodeType = ENarrRailNodeType::Dialogue;
                NodeForEvent.Dialogue.SpeakerId = Request.SpeakerId;
                NodeForEvent.Dialogue.TextKey = Request.TextContent;
                NodeForEvent.Dialogue.SpeechRate = Request.SpeechRate;
            }
        }
        else if (Node->NodeType == ENarrRailNodeType::Choice)
        {
            FNarrRailChoiceRequest Request;
            Request.NodeId = Node->NodeId;
            Request.Choices = BuildVisibleChoiceOptions(*Node);
            Request.Session = this;
            INarrRailDialoguePresenterInterface::Execute_ShowChoices(PresenterObject, Request);
        }
    }

    OnNodeEntered.Broadcast(Context.CurrentNodeId, NodeForEvent);

    if (Node->NodeType == ENarrRailNodeType::Choice)
    {
        OnChoicesReady.Broadcast(Context.CurrentNodeId, BuildVisibleChoiceOptions(*Node));
    }

    if (SessionState == ENarrRailSessionState::Completed)
    {
        OnSessionEnded.Broadcast(Context.CurrentNodeId, SessionState);
    }
}

bool UNarrRailStorySession::BuildMultiDialogueDisplay(const FNarrRailNode& Node, FNarrRailDialogueRequest& OutRequest) const
{
    const int32 TotalLines = Node.MultiDialogue.Lines.Num();
    if (TotalLines <= 0) return false;
    if (CurrentMultiDialogueLineIndex < 0 || CurrentMultiDialogueLineIndex >= TotalLines) return false;

    const FNarrRailDialogueLine& Line = Node.MultiDialogue.Lines[CurrentMultiDialogueLineIndex];

    OutRequest.NodeId = Node.NodeId;
    OutRequest.SpeakerId = Node.MultiDialogue.SpeakerId;
    OutRequest.TextContent = Line.TextKey;
    OutRequest.SpeechRate = 1.0f;
    OutRequest.VoiceAsset = nullptr;
    OutRequest.bAutoAdvance = false;
    return true;
}

bool UNarrRailStorySession::TryPopExhaustiveReturn(FName& OutReturnNodeId)
{
    if (ExhaustivePendingChoiceReturnStack.Num() > 0)
    {
        OutReturnNodeId = ExhaustivePendingChoiceReturnStack.Pop();
        return true;
    }

    OutReturnNodeId = NAME_None;
    return false;
}

FNarrRailRuntimeResult UNarrRailStorySession::AdvanceToNode(const FName TargetNodeId)
{
    const FNarrRailNode* Node = FindNode(TargetNodeId);
    if (Node == nullptr)
    {
        SessionState = ENarrRailSessionState::Error;
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::MissingNode, TEXT("Target node not found."), TargetNodeId);
    }

    if (ExhaustivePendingChoiceReturnStack.Num() > 0 &&
        ExhaustivePendingChoiceReturnStack.Last() == TargetNodeId)
    {
        ExhaustivePendingChoiceReturnStack.Pop();
    }

    Context.CurrentNodeId = TargetNodeId;
    Context.NodeHistory.Add(Context.CurrentNodeId);

    FString ActionError;
    if (!ExecuteActions(Node->EnterActions, ActionError))
    {
        SessionState = ENarrRailSessionState::Error;
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidInput, *ActionError, Context.CurrentNodeId);
    }

    if (Node->NodeType == ENarrRailNodeType::MultiDialogue)
    {
        CurrentMultiDialogueLineIndex = 0;
    }
    else
    {
        CurrentMultiDialogueLineIndex = INDEX_NONE;
    }

    // 通知 UI 显示器
    UObject* PresenterObject = DialoguePresenter.GetObject();
    const bool bPresenterValid =
        PresenterObject != nullptr &&
        PresenterObject->GetClass()->ImplementsInterface(UNarrRailDialoguePresenterInterface::StaticClass());

    FNarrRailNode EnteredNodeForEvent = *Node;

    // Choice 节点：提前计算可用选项，避免两个 Choice 块重复调用
    TArray<int32> ChoiceAvailableIndices;
    TArray<int32> ChoiceVisibleIndices;
    TArray<FNarrRailChoiceOption> ChoiceVisibleOptions;
    const bool bIsChoiceNode = (Node->NodeType == ENarrRailNodeType::Choice);
    if (bIsChoiceNode)
    {
        ChoiceAvailableIndices = BuildAvailableChoiceIndices(*Node);
        ChoiceVisibleIndices = BuildVisibleChoiceIndices(*Node);
        ChoiceVisibleOptions = BuildVisibleChoiceOptions(*Node);
    }

    if (bPresenterValid)
    {
        if (Node->NodeType == ENarrRailNodeType::Dialogue)
        {
            // 构建对话显示请求
            FNarrRailDialogueRequest Request;
            Request.NodeId = TargetNodeId;
            Request.SpeakerId = Node->Dialogue.SpeakerId;
            Request.TextContent = Node->Dialogue.TextKey;
            Request.SpeechRate = Node->Dialogue.SpeechRate;
            Request.VoiceAsset = Node->Dialogue.VoiceAsset;
            Request.bAutoAdvance = false;

            // 调用 UI 显示对话
            INarrRailDialoguePresenterInterface::Execute_ShowDialogue(PresenterObject, Request);
        }
        else if (Node->NodeType == ENarrRailNodeType::MultiDialogue)
        {
            FNarrRailDialogueRequest Request;
            if (BuildMultiDialogueDisplay(*Node, Request))
            {
                INarrRailDialoguePresenterInterface::Execute_ShowDialogue(PresenterObject, Request);

                EnteredNodeForEvent.NodeType = ENarrRailNodeType::Dialogue;
                EnteredNodeForEvent.Dialogue.SpeakerId = Request.SpeakerId;
                EnteredNodeForEvent.Dialogue.TextKey = Request.TextContent;
                EnteredNodeForEvent.Dialogue.SpeechRate = Request.SpeechRate;
            }
        }
        else if (bIsChoiceNode && !(Node->ChoiceMode == ENarrRailChoiceMode::ExhaustiveUntilComplete && ChoiceAvailableIndices.Num() == 0))
        {
            // 构建选项显示请求（使用预计算的可见选项）
            FNarrRailChoiceRequest Request;
            Request.NodeId = TargetNodeId;
            Request.Choices = ChoiceVisibleOptions;
            Request.Session = this;

            // 调用 UI 显示选项
            INarrRailDialoguePresenterInterface::Execute_ShowChoices(PresenterObject, Request);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[NarrRail] DialoguePresenter is invalid for interface call. Object=%s, InterfacePtrValid=%s"),
            PresenterObject ? *PresenterObject->GetName() : TEXT("None"),
            DialoguePresenter.GetInterface() ? TEXT("true") : TEXT("false"));
    }

    // 触发节点进入事件
    OnNodeEntered.Broadcast(Context.CurrentNodeId, EnteredNodeForEvent);

    if (Node->NodeType == ENarrRailNodeType::End)
    {
        FName ReturnNodeId;
        if (TryPopExhaustiveReturn(ReturnNodeId))
        {
            return AdvanceToNode(ReturnNodeId);
        }

        SessionState = ENarrRailSessionState::Completed;
        // 触发会话结束事件
        OnSessionEnded.Broadcast(Context.CurrentNodeId, SessionState);
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::Completed, TEXT("Reached end node."), Context.CurrentNodeId);
    }

    if (bIsChoiceNode)
    {
        if (Node->ChoiceMode == ENarrRailChoiceMode::ExhaustiveUntilComplete && ChoiceAvailableIndices.Num() == 0)
        {
            while (ExhaustivePendingChoiceReturnStack.Num() > 0 &&
                ExhaustivePendingChoiceReturnStack.Last() == Context.CurrentNodeId)
            {
                ExhaustivePendingChoiceReturnStack.Pop();
            }

            if (Node->ChoiceCompletionTargetNodeId == NAME_None)
            {
                SessionState = ENarrRailSessionState::Error;
                return FNarrRailRuntimeResult::Make(
                    ENarrRailRuntimeResultCode::InvalidInput,
                    TEXT("Exhaustive choice node requires ChoiceCompletionTargetNodeId when all choices are consumed."),
                    Context.CurrentNodeId);
            }

            SessionState = ENarrRailSessionState::Running;
            return AdvanceToNode(Node->ChoiceCompletionTargetNodeId);
        }

        RuntimeVisibleChoiceIndexMap.Add(Context.CurrentNodeId, ChoiceVisibleIndices);

        SessionState = ENarrRailSessionState::WaitingForChoice;
        OnChoicesReady.Broadcast(Context.CurrentNodeId, ChoiceVisibleOptions);
    }
    else
    {
        RuntimeVisibleChoiceIndexMap.Remove(Context.CurrentNodeId);
        SessionState = ENarrRailSessionState::Running;
    }

    if (Node->NodeType == ENarrRailNodeType::SetVariable)
    {
        return Next();
    }

    return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::Success, TEXT("Advanced to node."), Context.CurrentNodeId);
}

TArray<int32> UNarrRailStorySession::BuildAvailableChoiceIndices(const FNarrRailNode& ChoiceNode) const
{
    TArray<int32> Available;

    const TSet<int32>* Selected = ExhaustiveSelectedChoiceIndices.Find(ChoiceNode.NodeId);

    for (int32 Index = 0; Index < ChoiceNode.Choices.Num(); ++Index)
    {
        if (Selected != nullptr && Selected->Contains(Index))
        {
            continue;
        }

        Available.Add(Index);
    }

    return Available;
}

TArray<int32> UNarrRailStorySession::BuildVisibleChoiceIndices(const FNarrRailNode& ChoiceNode) const
{
    TArray<int32> Visible;
    const TSet<int32>* Selected = ExhaustiveSelectedChoiceIndices.Find(ChoiceNode.NodeId);
    const bool bExhaustiveMode = (ChoiceNode.ChoiceMode == ENarrRailChoiceMode::ExhaustiveUntilComplete);

    for (int32 Index = 0; Index < ChoiceNode.Choices.Num(); ++Index)
    {
        // SinglePass: 仅显示当前可选项；Exhaustive: 显示所有可见项，并通过 bHasBeenSelected 标记已选状态。
        if (!bExhaustiveMode && Selected != nullptr && Selected->Contains(Index))
        {
            continue;
        }

        Visible.Add(Index);
    }

    return Visible;
}

TArray<FNarrRailChoiceOption> UNarrRailStorySession::BuildVisibleChoiceOptions(const FNarrRailNode& ChoiceNode) const
{
    TArray<FNarrRailChoiceOption> VisibleChoices;
    const TArray<int32> VisibleIndices = BuildVisibleChoiceIndices(ChoiceNode);
    const TSet<int32>* Selected = ExhaustiveSelectedChoiceIndices.Find(ChoiceNode.NodeId);

    for (const int32 Index : VisibleIndices)
    {
        if (ChoiceNode.Choices.IsValidIndex(Index))
        {
            FNarrRailChoiceOption VisibleChoice = ChoiceNode.Choices[Index];
            VisibleChoice.bHasBeenSelected = (Selected != nullptr && Selected->Contains(Index));
            VisibleChoices.Add(MoveTemp(VisibleChoice));
        }
    }

    return VisibleChoices;
}

FNarrRailRuntimeResult UNarrRailStorySession::ResolveNextByEdge(const FNarrRailNode& FromNode, FName& OutNextNodeId) const
{
    if (StoryAsset == nullptr)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidState, TEXT("Session not initialized."));
    }

    TArray<const FNarrRailNodeEdge*> CandidateEdges;
    for (const FNarrRailNodeEdge& Edge : StoryAsset->Edges)
    {
        if (Edge.SourceNodeId == FromNode.NodeId)
        {
            CandidateEdges.Add(&Edge);
        }
    }

    if (CandidateEdges.Num() == 0)
    {
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::Completed, TEXT("No outgoing edge. Session completed."), FromNode.NodeId);
    }

    Algo::SortBy(CandidateEdges, [](const FNarrRailNodeEdge* Edge) { return Edge->Priority; });

    if (FromNode.NodeType == ENarrRailNodeType::Condition)
    {
        auto FindEdgeByHandle = [&CandidateEdges](const FName Handle) -> const FNarrRailNodeEdge*
        {
            for (const FNarrRailNodeEdge* Edge : CandidateEdges)
            {
                if (Edge->SourceHandle == Handle)
                {
                    return Edge;
                }
            }

            return nullptr;
        };

        if (FromNode.Condition.Branches.Num() > 0)
        {
            for (int32 BranchIndex = 0; BranchIndex < FromNode.Condition.Branches.Num(); ++BranchIndex)
            {
                const FNarrRailConditionBranch& Branch = FromNode.Condition.Branches[BranchIndex];
                if (!EvaluateConditionBranch(Branch))
                {
                    continue;
                }

                const FName ExpectedHandle(*FString::Printf(TEXT("condition-%d"), BranchIndex));
                const FNarrRailNodeEdge* MatchedEdge = FindEdgeByHandle(ExpectedHandle);

                if (MatchedEdge == nullptr)
                {
                    return FNarrRailRuntimeResult::Make(
                        ENarrRailRuntimeResultCode::InvalidInput,
                        *FString::Printf(TEXT("Condition node missing %s outgoing edge."), *ExpectedHandle.ToString()),
                        FromNode.NodeId);
                }

                OutNextNodeId = MatchedEdge->TargetNodeId;
                return FNarrRailRuntimeResult::Make(
                    ENarrRailRuntimeResultCode::Success,
                    *FString::Printf(TEXT("Resolved %s branch."), *ExpectedHandle.ToString()),
                    OutNextNodeId);
            }

            const FNarrRailNodeEdge* FallbackEdge = FindEdgeByHandle(FName(TEXT("condition-fallback")));

            if (FallbackEdge == nullptr)
            {
                return FNarrRailRuntimeResult::Make(
                    ENarrRailRuntimeResultCode::InvalidInput,
                    TEXT("Condition node missing condition-fallback outgoing edge."),
                    FromNode.NodeId);
            }

            OutNextNodeId = FallbackEdge->TargetNodeId;
            return FNarrRailRuntimeResult::Make(
                ENarrRailRuntimeResultCode::Success,
                TEXT("Resolved condition-fallback branch."),
                OutNextNodeId);
        }

        const bool bConditionResult = EvaluateConditionExpression(FromNode.Condition);
        const FName ExpectedHandle = bConditionResult
            ? FName(TEXT("condition-0"))
            : FName(TEXT("condition-fallback"));

        const FNarrRailNodeEdge* MatchedEdge = FindEdgeByHandle(ExpectedHandle);
        if (MatchedEdge != nullptr)
        {
            OutNextNodeId = MatchedEdge->TargetNodeId;
            return FNarrRailRuntimeResult::Make(
                ENarrRailRuntimeResultCode::Success,
                bConditionResult
                    ? TEXT("Resolved condition true branch.")
                    : TEXT("Resolved condition false branch."),
                OutNextNodeId);
        }

        return FNarrRailRuntimeResult::Make(
            ENarrRailRuntimeResultCode::InvalidInput,
            bConditionResult
                ? TEXT("Condition node missing condition-0 outgoing edge.")
                : TEXT("Condition node missing condition-fallback outgoing edge."),
            FromNode.NodeId);
    }

    for (const FNarrRailNodeEdge* Edge : CandidateEdges)
    {
        OutNextNodeId = Edge->TargetNodeId;
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::Success, TEXT("Resolved next node by edge."), OutNextNodeId);
    }

    return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::Completed, TEXT("No outgoing edge. Session completed."), FromNode.NodeId);
}

const FNarrRailNode* UNarrRailStorySession::FindNode(const FName NodeId) const
{
    if (StoryAsset == nullptr || NodeId == NAME_None)
    {
        return nullptr;
    }

    for (const FNarrRailNode& Node : StoryAsset->Nodes)
    {
        if (Node.NodeId == NodeId)
        {
            return &Node;
        }
    }

    return nullptr;
}

bool UNarrRailStorySession::EvaluateConditionExpression(const FNarrRailConditionExpression& Condition) const
{
    if (Condition.Terms.Num() == 0)
    {
        return true;
    }

    if (Condition.Logic == ENarrRailConditionLogic::All)
    {
        for (const FNarrRailConditionTerm& Term : Condition.Terms)
        {
            if (!EvaluateConditionTerm(Term))
            {
                return false;
            }
        }

        return true;
    }

    for (const FNarrRailConditionTerm& Term : Condition.Terms)
    {
        if (EvaluateConditionTerm(Term))
        {
            return true;
        }
    }

    return false;
}

bool UNarrRailStorySession::EvaluateConditionBranch(const FNarrRailConditionBranch& Branch) const
{
    if (Branch.Terms.Num() == 0)
    {
        return true;
    }

    if (Branch.Logic == ENarrRailConditionLogic::All)
    {
        for (const FNarrRailConditionTerm& Term : Branch.Terms)
        {
            if (!EvaluateConditionTerm(Term))
            {
                return false;
            }
        }

        return true;
    }

    for (const FNarrRailConditionTerm& Term : Branch.Terms)
    {
        if (EvaluateConditionTerm(Term))
        {
            return true;
        }
    }

    return false;
}

bool UNarrRailStorySession::EvaluateConditionTerm(const FNarrRailConditionTerm& Term) const
{
    if (Term.Variable.VariableName == NAME_None)
    {
        return false;
    }

    UNarrRailVariableContainer* Container = FindVariableContainerForName(Term.Variable.VariableName);
    if (Container == nullptr)
    {
        return false;
    }

    FString CurrentValue;
    FNarrRailVariableResult GetResult = Container->GetVariable(Term.Variable.VariableName, CurrentValue);
    if (!GetResult.IsSuccess())
    {
        return false;
    }

    switch (Term.Variable.VariableType)
    {
    case ENarrRailVariableType::Bool:
    {
        bool L = false;
        bool R = false;
        if (!NarrRailRuntime::TryParseBool(CurrentValue, L) || !NarrRailRuntime::TryParseBool(Term.CompareValue, R))
        {
            return false;
        }

        if (Term.Operator == ENarrRailComparisonOp::Equal)
        {
            return L == R;
        }
        if (Term.Operator == ENarrRailComparisonOp::NotEqual)
        {
            return L != R;
        }
        return false;
    }
    case ENarrRailVariableType::Int:
    {
        int32 L = 0;
        int32 R = 0;
        if (!NarrRailRuntime::TryParseInt(CurrentValue, L) || !NarrRailRuntime::TryParseInt(Term.CompareValue, R))
        {
            return false;
        }

        switch (Term.Operator)
        {
        case ENarrRailComparisonOp::Equal: return L == R;
        case ENarrRailComparisonOp::NotEqual: return L != R;
        case ENarrRailComparisonOp::Greater: return L > R;
        case ENarrRailComparisonOp::GreaterOrEqual: return L >= R;
        case ENarrRailComparisonOp::Less: return L < R;
        case ENarrRailComparisonOp::LessOrEqual: return L <= R;
        default: return false;
        }
    }
    case ENarrRailVariableType::Float:
    {
        float L = 0.f;
        float R = 0.f;
        if (!NarrRailRuntime::TryParseFloat(CurrentValue, L) || !NarrRailRuntime::TryParseFloat(Term.CompareValue, R))
        {
            return false;
        }

        switch (Term.Operator)
        {
        case ENarrRailComparisonOp::Equal: return FMath::IsNearlyEqual(L, R);
        case ENarrRailComparisonOp::NotEqual: return !FMath::IsNearlyEqual(L, R);
        case ENarrRailComparisonOp::Greater: return L > R;
        case ENarrRailComparisonOp::GreaterOrEqual: return L >= R;
        case ENarrRailComparisonOp::Less: return L < R;
        case ENarrRailComparisonOp::LessOrEqual: return L <= R;
        default: return false;
        }
    }
    case ENarrRailVariableType::String:
    default:
    {
        const int32 Compare = CurrentValue.Compare(Term.CompareValue, ESearchCase::CaseSensitive);
        switch (Term.Operator)
        {
        case ENarrRailComparisonOp::Equal: return Compare == 0;
        case ENarrRailComparisonOp::NotEqual: return Compare != 0;
        case ENarrRailComparisonOp::Greater: return Compare > 0;
        case ENarrRailComparisonOp::GreaterOrEqual: return Compare >= 0;
        case ENarrRailComparisonOp::Less: return Compare < 0;
        case ENarrRailComparisonOp::LessOrEqual: return Compare <= 0;
        default: return false;
        }
    }
    }
}

bool UNarrRailStorySession::ExecuteActions(const TArray<FNarrRailNodeAction>& Actions, FString& OutErrorMessage)
{
    if (VariableContainer == nullptr && GlobalVariableContainer == nullptr)
    {
        OutErrorMessage = TEXT("Variable container not initialized.");
        return false;
    }

    for (const FNarrRailNodeAction& Action : Actions)
    {
        switch (Action.ActionType)
        {
        case ENarrRailActionType::EmitEvent:
            if (Action.EventId != NAME_None)
            {
                Context.EmittedEvents.Add(Action.EventId);
            }
            break;

        case ENarrRailActionType::Set:
        {
            if (Action.Variable.VariableName == NAME_None)
            {
                OutErrorMessage = TEXT("Action variable name is empty.");
                return false;
            }

            UNarrRailVariableContainer* Container = FindVariableContainerForName(Action.Variable.VariableName);
            if (Container == nullptr)
            {
                OutErrorMessage = FString::Printf(TEXT("Variable '%s' not found."), *Action.Variable.VariableName.ToString());
                return false;
            }

            FNarrRailVariableResult Result = Container->SetVariable(Action.Variable.VariableName, Action.Value);
            if (!Result.IsSuccess())
            {
                OutErrorMessage = FString::Printf(TEXT("Set variable failed: %s"), *Result.ErrorMessage);
                return false;
            }
            break;
        }

        case ENarrRailActionType::Add:
        case ENarrRailActionType::Subtract:
        {
            if (Action.Variable.VariableName == NAME_None)
            {
                OutErrorMessage = TEXT("Action variable name is empty.");
                return false;
            }

            UNarrRailVariableContainer* Container = FindVariableContainerForName(Action.Variable.VariableName);
            if (Container == nullptr || !Container->HasVariable(Action.Variable.VariableName))
            {
                OutErrorMessage = FString::Printf(TEXT("Variable '%s' not found."), *Action.Variable.VariableName.ToString());
                return false;
            }

            FNarrRailVariableResult Result;
            const ENarrRailVariableType VariableType = Container->GetVariableType(Action.Variable.VariableName);
            const bool bAdd = Action.ActionType == ENarrRailActionType::Add;
            if (VariableType == ENarrRailVariableType::Int)
            {
                int32 Delta = 0;
                if (!NarrRailRuntime::TryParseInt(Action.Value, Delta))
                {
                    OutErrorMessage = TEXT("Int action parse failed.");
                    return false;
                }
                Result = bAdd
                    ? Container->AddInt(Action.Variable.VariableName, Delta)
                    : Container->SubtractInt(Action.Variable.VariableName, Delta);
            }
            else if (VariableType == ENarrRailVariableType::Float)
            {
                float Delta = 0.f;
                if (!NarrRailRuntime::TryParseFloat(Action.Value, Delta))
                {
                    OutErrorMessage = TEXT("Float action parse failed.");
                    return false;
                }
                Result = bAdd
                    ? Container->AddFloat(Action.Variable.VariableName, Delta)
                    : Container->SubtractFloat(Action.Variable.VariableName, Delta);
            }
            else
            {
                OutErrorMessage = bAdd
                    ? TEXT("Add only supports Int/Float variables.")
                    : TEXT("Subtract only supports Int/Float variables.");
                return false;
            }

            if (!Result.IsSuccess())
            {
                OutErrorMessage = FString::Printf(TEXT("Variable arithmetic failed: %s"), *Result.ErrorMessage);
                return false;
            }
            break;
        }

        default:
            OutErrorMessage = TEXT("Unsupported action type.");
            return false;
        }
    }

    SyncVariableSnapshotToContext();
    return true;
}

// === 全局会话管理 ===

TArray<UNarrRailStorySession*> UNarrRailStorySession::GetAllActiveSessions()
{
    // 清理已销毁的会话
    ActiveSessions.RemoveAll([](UNarrRailStorySession* Session)
    {
        return Session == nullptr || !IsValid(Session);
    });

    return ActiveSessions;
}

void UNarrRailStorySession::SetDebugName(const FString& InDebugName)
{
    DebugName = InDebugName;
}
