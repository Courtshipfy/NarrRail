#include "Runtime/NarrRailStorySession.h"

#include "Algo/Sort.h"

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
    // 从全局列表中移除
    ActiveSessions.Remove(this);
    Super::BeginDestroy();
}

FNarrRailRuntimeResult UNarrRailStorySession::Initialize(const UNarrRailStoryAsset* InStoryAsset)
{
    if (InStoryAsset == nullptr)
    {
        SessionState = ENarrRailSessionState::Error;
        return FNarrRailRuntimeResult::Make(ENarrRailRuntimeResultCode::InvalidInput, TEXT("Story asset is null."));
    }

    StoryAsset = InStoryAsset;
    SessionState = ENarrRailSessionState::Idle;
    StateBeforePause = ENarrRailSessionState::Idle;

    // 创建变量容器
    if (VariableContainer == nullptr)
    {
        VariableContainer = NewObject<UNarrRailVariableContainer>(this);
    }

    ResetSessionContextFromAsset();

    // 注册到全局列表
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
    if (VariableContainer == nullptr)
    {
        return FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound, TEXT("Variable container not initialized."));
    }
    return VariableContainer->GetBool(VariableName, OutValue);
}

FNarrRailVariableResult UNarrRailStorySession::GetVariableInt(FName VariableName, int32& OutValue) const
{
    if (VariableContainer == nullptr)
    {
        return FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound, TEXT("Variable container not initialized."));
    }
    return VariableContainer->GetInt(VariableName, OutValue);
}

FNarrRailVariableResult UNarrRailStorySession::GetVariableFloat(FName VariableName, float& OutValue) const
{
    if (VariableContainer == nullptr)
    {
        return FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound, TEXT("Variable container not initialized."));
    }
    return VariableContainer->GetFloat(VariableName, OutValue);
}

FNarrRailVariableResult UNarrRailStorySession::GetVariableString(FName VariableName, FString& OutValue) const
{
    if (VariableContainer == nullptr)
    {
        return FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound, TEXT("Variable container not initialized."));
    }
    return VariableContainer->GetString(VariableName, OutValue);
}

FNarrRailVariableResult UNarrRailStorySession::SetVariableBool(FName VariableName, bool Value)
{
    if (VariableContainer == nullptr)
    {
        return FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound, TEXT("Variable container not initialized."));
    }
    return VariableContainer->SetBool(VariableName, Value);
}

FNarrRailVariableResult UNarrRailStorySession::SetVariableInt(FName VariableName, int32 Value)
{
    if (VariableContainer == nullptr)
    {
        return FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound, TEXT("Variable container not initialized."));
    }
    return VariableContainer->SetInt(VariableName, Value);
}

FNarrRailVariableResult UNarrRailStorySession::SetVariableFloat(FName VariableName, float Value)
{
    if (VariableContainer == nullptr)
    {
        return FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound, TEXT("Variable container not initialized."));
    }
    return VariableContainer->SetFloat(VariableName, Value);
}

FNarrRailVariableResult UNarrRailStorySession::SetVariableString(FName VariableName, const FString& Value)
{
    if (VariableContainer == nullptr)
    {
        return FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound, TEXT("Variable container not initialized."));
    }
    return VariableContainer->SetString(VariableName, Value);
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

    // 直接使用资产中的变量定义初始化变量容器
    VariableContainer->Initialize(StoryAsset->Variables);

    // 同步变量快照到 Context（用于存档兼容）
    Context.VariableSnapshot = VariableContainer->GetSnapshot();
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
                if (MatchedEdge == nullptr && BranchIndex == 0)
                {
                    MatchedEdge = FindEdgeByHandle(FName(TEXT("condition-true")));
                }

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
                FallbackEdge = FindEdgeByHandle(FName(TEXT("condition-false")));
            }

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
            ? FName(TEXT("condition-true"))
            : FName(TEXT("condition-false"));
        const FName ModernHandle = bConditionResult
            ? FName(TEXT("condition-0"))
            : FName(TEXT("condition-fallback"));

        const FNarrRailNodeEdge* MatchedEdge = FindEdgeByHandle(ExpectedHandle);
        if (MatchedEdge == nullptr)
        {
            MatchedEdge = FindEdgeByHandle(ModernHandle);
        }

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
    if (Term.Variable.VariableName == NAME_None || VariableContainer == nullptr)
    {
        return false;
    }

    FString CurrentValue;
    FNarrRailVariableResult GetResult = VariableContainer->GetVariable(Term.Variable.VariableName, CurrentValue);
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
    if (VariableContainer == nullptr)
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

            FNarrRailVariableResult Result = VariableContainer->SetVariable(Action.Variable.VariableName, Action.Value);
            if (!Result.IsSuccess())
            {
                OutErrorMessage = FString::Printf(TEXT("Set variable failed: %s"), *Result.ErrorMessage);
                return false;
            }
            break;
        }

        case ENarrRailActionType::Add:
        {
            if (Action.Variable.VariableName == NAME_None)
            {
                OutErrorMessage = TEXT("Action variable name is empty.");
                return false;
            }

            FNarrRailVariableResult Result;
            if (Action.Variable.VariableType == ENarrRailVariableType::Int)
            {
                int32 Delta = 0;
                if (!NarrRailRuntime::TryParseInt(Action.Value, Delta))
                {
                    OutErrorMessage = TEXT("Int action parse failed.");
                    return false;
                }
                Result = VariableContainer->AddInt(Action.Variable.VariableName, Delta);
            }
            else if (Action.Variable.VariableType == ENarrRailVariableType::Float)
            {
                float Delta = 0.f;
                if (!NarrRailRuntime::TryParseFloat(Action.Value, Delta))
                {
                    OutErrorMessage = TEXT("Float action parse failed.");
                    return false;
                }
                Result = VariableContainer->AddFloat(Action.Variable.VariableName, Delta);
            }
            else
            {
                OutErrorMessage = TEXT("Add only supports Int/Float variables.");
                return false;
            }

            if (!Result.IsSuccess())
            {
                OutErrorMessage = FString::Printf(TEXT("Add variable failed: %s"), *Result.ErrorMessage);
                return false;
            }
            break;
        }

        case ENarrRailActionType::Subtract:
        {
            if (Action.Variable.VariableName == NAME_None)
            {
                OutErrorMessage = TEXT("Action variable name is empty.");
                return false;
            }

            FNarrRailVariableResult Result;
            if (Action.Variable.VariableType == ENarrRailVariableType::Int)
            {
                int32 Delta = 0;
                if (!NarrRailRuntime::TryParseInt(Action.Value, Delta))
                {
                    OutErrorMessage = TEXT("Int action parse failed.");
                    return false;
                }
                Result = VariableContainer->SubtractInt(Action.Variable.VariableName, Delta);
            }
            else if (Action.Variable.VariableType == ENarrRailVariableType::Float)
            {
                float Delta = 0.f;
                if (!NarrRailRuntime::TryParseFloat(Action.Value, Delta))
                {
                    OutErrorMessage = TEXT("Float action parse failed.");
                    return false;
                }
                Result = VariableContainer->SubtractFloat(Action.Variable.VariableName, Delta);
            }
            else
            {
                OutErrorMessage = TEXT("Subtract only supports Int/Float variables.");
                return false;
            }

            if (!Result.IsSuccess())
            {
                OutErrorMessage = FString::Printf(TEXT("Subtract variable failed: %s"), *Result.ErrorMessage);
                return false;
            }
            break;
        }

        default:
            OutErrorMessage = TEXT("Unsupported action type.");
            return false;
        }
    }

    // 同步变量快照到 Context（用于存档兼容）
    Context.VariableSnapshot = VariableContainer->GetSnapshot();

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

