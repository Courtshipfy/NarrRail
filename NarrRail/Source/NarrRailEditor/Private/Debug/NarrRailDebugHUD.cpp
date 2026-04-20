#include "Debug/NarrRailDebugHUD.h"
#include "Debug/NarrRailDebugger.h"
#include "Runtime/NarrRailVariableContainer.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

void FNarrRailDebugHUD::DrawDebugInfo(UCanvas* Canvas, const UNarrRailStorySession* Session)
{
    if (Canvas == nullptr)
    {
        return;
    }

    // 自动查找会话
    TArray<UNarrRailStorySession*> ActiveSessions = UNarrRailStorySession::GetAllActiveSessions();

    if (ActiveSessions.Num() == 0)
    {
        FCanvasTextItem TextItem(FVector2D(10, 50), FText::FromString(TEXT("NarrRail Debug: No active sessions")), GEngine->GetSmallFont(), FLinearColor::Yellow);
        Canvas->DrawItem(TextItem);
        return;
    }

    if (ActiveSessions.Num() > 1)
    {
        FCanvasTextItem TextItem(FVector2D(10, 50), FText::FromString(FString::Printf(TEXT("NarrRail Debug: Multiple sessions detected (%d)"), ActiveSessions.Num())), GEngine->GetSmallFont(), FLinearColor::Red);
        Canvas->DrawItem(TextItem);

        float YPos = 70;
        for (int32 i = 0; i < ActiveSessions.Num(); ++i)
        {
            FString SessionInfo = FString::Printf(TEXT("  [%d] %s - %s"),
                i,
                *ActiveSessions[i]->GetDebugName(),
                *GetSessionStateString(ActiveSessions[i]->GetSessionState()));
            FCanvasTextItem SessionItem(FVector2D(10, YPos), FText::FromString(SessionInfo), GEngine->GetSmallFont(), FLinearColor::White);
            Canvas->DrawItem(SessionItem);
            YPos += 15;
        }
        return;
    }

    // 使用唯一会话
    Session = ActiveSessions[0];

    float YPos = 50;

    // 标题
    FCanvasTextItem TitleItem(FVector2D(10, YPos), FText::FromString(TEXT("=== NarrRail Debug ===")), GEngine->GetMediumFont(), FLinearColor(0.0f, 1.0f, 1.0f));
    Canvas->DrawItem(TitleItem);
    YPos += 25;

    // 绘制各部分
    DrawSessionState(Canvas, Session, YPos);
    DrawCurrentNode(Canvas, Session, YPos);
    DrawVariables(Canvas, Session, YPos);
    DrawCurrentChoices(Canvas, Session, YPos);
    DrawLastChoice(Canvas, Session, YPos);
    DrawNodeHistory(Canvas, Session, YPos);
}

void FNarrRailDebugHUD::DrawSessionState(UCanvas* Canvas, const UNarrRailStorySession* Session, float& YPos)
{
    if (Session == nullptr)
    {
        return;
    }

    // 标题
    FCanvasTextItem HeaderItem(FVector2D(10, YPos), FText::FromString(TEXT("Session State:")), GEngine->GetSmallFont(), FLinearColor::Yellow);
    Canvas->DrawItem(HeaderItem);
    YPos += 15;

    // 状态
    FString StateStr = FString::Printf(TEXT("  State: %s"), *GetSessionStateString(Session->GetSessionState()));
    FCanvasTextItem StateItem(FVector2D(10, YPos), FText::FromString(StateStr), GEngine->GetSmallFont(), FLinearColor::White);
    Canvas->DrawItem(StateItem);
    YPos += 15;

    // 当前节点
    FString NodeStr = FString::Printf(TEXT("  Current Node: %s"), *Session->GetCurrentNodeId().ToString());
    FCanvasTextItem NodeItem(FVector2D(10, YPos), FText::FromString(NodeStr), GEngine->GetSmallFont(), FLinearColor::White);
    Canvas->DrawItem(NodeItem);
    YPos += 15;

    // 历史数量
    FString HistoryStr = FString::Printf(TEXT("  History Count: %d"), Session->GetHistory().Num());
    FCanvasTextItem HistoryItem(FVector2D(10, YPos), FText::FromString(HistoryStr), GEngine->GetSmallFont(), FLinearColor::White);
    Canvas->DrawItem(HistoryItem);
    YPos += 20;
}

void FNarrRailDebugHUD::DrawCurrentNode(UCanvas* Canvas, const UNarrRailStorySession* Session, float& YPos)
{
    if (Session == nullptr)
    {
        return;
    }

    FNarrRailNode CurrentNode;
    if (!Session->GetCurrentNode(CurrentNode))
    {
        return;
    }

    // 标题
    FCanvasTextItem HeaderItem(FVector2D(10, YPos), FText::FromString(TEXT("Current Node:")), GEngine->GetSmallFont(), FLinearColor::Yellow);
    Canvas->DrawItem(HeaderItem);
    YPos += 15;

    // 节点类型
    FString TypeStr = FString::Printf(TEXT("  Type: %s"), *GetNodeTypeString(CurrentNode.NodeType));
    FCanvasTextItem TypeItem(FVector2D(10, YPos), FText::FromString(TypeStr), GEngine->GetSmallFont(), FLinearColor::White);
    Canvas->DrawItem(TypeItem);
    YPos += 15;

    // 对话内容
    if (CurrentNode.NodeType == ENarrRailNodeType::Dialogue)
    {
        FString SpeakerStr = FString::Printf(TEXT("  Speaker: %s"), *CurrentNode.Dialogue.SpeakerId.ToString());
        FCanvasTextItem SpeakerItem(FVector2D(10, YPos), FText::FromString(SpeakerStr), GEngine->GetSmallFont(), FLinearColor::White);
        Canvas->DrawItem(SpeakerItem);
        YPos += 15;

        FString TextStr = FString::Printf(TEXT("  Text: %s"), *CurrentNode.Dialogue.TextKey);
        FCanvasTextItem TextItem(FVector2D(10, YPos), FText::FromString(TextStr), GEngine->GetSmallFont(), FLinearColor::Green);
        Canvas->DrawItem(TextItem);
        YPos += 15;
    }

    // 动作
    if (CurrentNode.EnterActions.Num() > 0)
    {
        FString ActionsStr = FString::Printf(TEXT("  Enter Actions: %d"), CurrentNode.EnterActions.Num());
        FCanvasTextItem ActionsItem(FVector2D(10, YPos), FText::FromString(ActionsStr), GEngine->GetSmallFont(), FLinearColor::White);
        Canvas->DrawItem(ActionsItem);
        YPos += 15;
    }

    YPos += 5;
}

void FNarrRailDebugHUD::DrawVariables(UCanvas* Canvas, const UNarrRailStorySession* Session, float& YPos)
{
    if (Session == nullptr)
    {
        return;
    }

    UNarrRailVariableContainer* VarContainer = Session->GetVariableContainer();
    if (VarContainer == nullptr)
    {
        return;
    }

    TMap<FName, FString> Snapshot = VarContainer->GetSnapshot();
    if (Snapshot.Num() == 0)
    {
        return;
    }

    // 标题
    FCanvasTextItem HeaderItem(FVector2D(10, YPos), FText::FromString(TEXT("Variables:")), GEngine->GetSmallFont(), FLinearColor::Yellow);
    Canvas->DrawItem(HeaderItem);
    YPos += 15;

    // 变量列表
    for (const auto& Pair : Snapshot)
    {
        FString VarStr = FString::Printf(TEXT("  %s = %s"), *Pair.Key.ToString(), *Pair.Value);
        FCanvasTextItem VarItem(FVector2D(10, YPos), FText::FromString(VarStr), GEngine->GetSmallFont(), FLinearColor(0.0f, 1.0f, 1.0f));
        Canvas->DrawItem(VarItem);
        YPos += 15;
    }

    YPos += 5;
}

void FNarrRailDebugHUD::DrawCurrentChoices(UCanvas* Canvas, const UNarrRailStorySession* Session, float& YPos)
{
    if (Session == nullptr)
    {
        return;
    }

    TArray<FNarrRailChoiceOption> Choices = Session->GetCurrentChoices();
    if (Choices.Num() == 0)
    {
        return;
    }

    // 标题
    FCanvasTextItem HeaderItem(FVector2D(10, YPos), FText::FromString(TEXT("Current Choices:")), GEngine->GetSmallFont(), FLinearColor::Yellow);
    Canvas->DrawItem(HeaderItem);
    YPos += 15;

    // 选项列表
    for (int32 i = 0; i < Choices.Num(); ++i)
    {
        FString ChoiceStr = FString::Printf(TEXT("  [%d] %s -> %s"), i, *Choices[i].TextKey, *Choices[i].TargetNodeId.ToString());
        FCanvasTextItem ChoiceItem(FVector2D(10, YPos), FText::FromString(ChoiceStr), GEngine->GetSmallFont(), FLinearColor::Green);
        Canvas->DrawItem(ChoiceItem);
        YPos += 15;
    }

    YPos += 5;
}

void FNarrRailDebugHUD::DrawLastChoice(UCanvas* Canvas, const UNarrRailStorySession* Session, float& YPos)
{
    if (Session == nullptr)
    {
        return;
    }

    FNarrRailLastChoiceInfo LastChoice = Session->GetLastChoice();
    if (!LastChoice.bValid)
    {
        return;
    }

    // 标题
    FCanvasTextItem HeaderItem(FVector2D(10, YPos), FText::FromString(TEXT("Last Choice:")), GEngine->GetSmallFont(), FLinearColor::Yellow);
    Canvas->DrawItem(HeaderItem);
    YPos += 15;

    // 选择信息
    FString ChoiceStr = FString::Printf(TEXT("  [%d] %s -> %s"),
        LastChoice.ChoiceIndex,
        *LastChoice.ChoiceTextKey,
        *LastChoice.TargetNodeId.ToString());
    FCanvasTextItem ChoiceItem(FVector2D(10, YPos), FText::FromString(ChoiceStr), GEngine->GetSmallFont(), FLinearColor(1.0f, 0.0f, 1.0f));
    Canvas->DrawItem(ChoiceItem);
    YPos += 20;
}

void FNarrRailDebugHUD::DrawNodeHistory(UCanvas* Canvas, const UNarrRailStorySession* Session, float& YPos)
{
    if (Session == nullptr)
    {
        return;
    }

    const TArray<FName>& History = Session->GetHistory();
    if (History.Num() == 0)
    {
        return;
    }

    // 标题
    FCanvasTextItem HeaderItem(FVector2D(10, YPos), FText::FromString(TEXT("Node History (last 5):")), GEngine->GetSmallFont(), FLinearColor::Yellow);
    Canvas->DrawItem(HeaderItem);
    YPos += 15;

    // 显示最后5条
    int32 StartIndex = FMath::Max(0, History.Num() - 5);
    for (int32 i = StartIndex; i < History.Num(); ++i)
    {
        FString HistoryStr = FString::Printf(TEXT("  [%d] %s"), i, *History[i].ToString());
        FCanvasTextItem HistoryItem(FVector2D(10, YPos), FText::FromString(HistoryStr), GEngine->GetSmallFont(), FLinearColor::Gray);
        Canvas->DrawItem(HistoryItem);
        YPos += 15;
    }
}

FString FNarrRailDebugHUD::GetSessionStateString(ENarrRailSessionState State)
{
    switch (State)
    {
        case ENarrRailSessionState::Idle: return TEXT("Idle");
        case ENarrRailSessionState::Running: return TEXT("Running");
        case ENarrRailSessionState::WaitingForChoice: return TEXT("WaitingForChoice");
        case ENarrRailSessionState::Paused: return TEXT("Paused");
        case ENarrRailSessionState::Completed: return TEXT("Completed");
        case ENarrRailSessionState::Error: return TEXT("Error");
        default: return TEXT("Unknown");
    }
}

FString FNarrRailDebugHUD::GetNodeTypeString(ENarrRailNodeType NodeType)
{
    switch (NodeType)
    {
        case ENarrRailNodeType::Dialogue: return TEXT("Dialogue");
        case ENarrRailNodeType::Choice: return TEXT("Choice");
        case ENarrRailNodeType::Jump: return TEXT("Jump");
        case ENarrRailNodeType::SetVariable: return TEXT("SetVariable");
        case ENarrRailNodeType::EmitEvent: return TEXT("EmitEvent");
        case ENarrRailNodeType::End: return TEXT("End");
        default: return TEXT("Unknown");
    }
}
