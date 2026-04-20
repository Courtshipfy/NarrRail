#include "NarrRailEditorModule.h"
#include "Debug/NarrRailDebugger.h"
#include "Debug/NarrRailDebugHUD.h"
#include "Modules/ModuleManager.h"
#include "HAL/IConsoleManager.h"
#include "Engine/Engine.h"
#include "GameFramework/HUD.h"
#include "Misc/CoreDelegates.h"
#include "DrawDebugHelpers.h"

#define LOCTEXT_NAMESPACE "FNarrRailEditorModule"

// HUD 显示开关
static bool bShowNarrRailDebugHUD = false;
static FDelegateHandle OnEndFrameHandle;

// 控制台命令：开关 HUD 显示
static FAutoConsoleCommand ToggleDebugHUDCmd(
	TEXT("narrrail.debug.hud"),
	TEXT("Toggle NarrRail debug HUD display"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		bShowNarrRailDebugHUD = !bShowNarrRailDebugHUD;
		UE_LOG(LogNarrRailDebug, Log, TEXT("NarrRail Debug HUD: %s"), bShowNarrRailDebugHUD ? TEXT("ON") : TEXT("OFF"));
	})
);

// 控制台命令：打印会话状态
static FAutoConsoleCommand PrintSessionCmd(
	TEXT("narrrail.debug.session"),
	TEXT("Print current session state (auto-detects active session)"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		UNarrRailDebugger::PrintSessionState(nullptr, false);
	})
);

// 控制台命令：打印当前节点
static FAutoConsoleCommand PrintNodeCmd(
	TEXT("narrrail.debug.node"),
	TEXT("Print current node information"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		UNarrRailDebugger::PrintCurrentNode(nullptr);
	})
);

// 控制台命令：打印所有变量
static FAutoConsoleCommand PrintVarsCmd(
	TEXT("narrrail.debug.vars"),
	TEXT("Print all variables"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		UNarrRailDebugger::PrintAllVariables(nullptr);
	})
);

// 控制台命令：打印节点历史
static FAutoConsoleCommand PrintHistoryCmd(
	TEXT("narrrail.debug.history"),
	TEXT("Print node history"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		UNarrRailDebugger::PrintNodeHistory(nullptr, 10);
	})
);

// 控制台命令：打印当前选项
static FAutoConsoleCommand PrintChoicesCmd(
	TEXT("narrrail.debug.choices"),
	TEXT("Print current choices"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		UNarrRailDebugger::PrintCurrentChoices(nullptr);
	})
);

// 控制台命令：打印最后选择
static FAutoConsoleCommand PrintLastChoiceCmd(
	TEXT("narrrail.debug.lastchoice"),
	TEXT("Print last choice information"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		UNarrRailDebugger::PrintLastChoice(nullptr);
	})
);

// 控制台命令：打印完整调试信息
static FAutoConsoleCommand PrintAllCmd(
	TEXT("narrrail.debug.all"),
	TEXT("Print full debug information"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		UNarrRailDebugger::PrintFullDebugInfo(nullptr);
	})
);

// 控制台命令：列出所有活跃会话
static FAutoConsoleCommand PrintSessionsCmd(
	TEXT("narrrail.debug.sessions"),
	TEXT("List all active sessions"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		UNarrRailDebugger::PrintAllActiveSessions();
	})
);

void FNarrRailEditorModule::StartupModule()
{
	// 注册每帧回调用于绘制 HUD
	OnEndFrameHandle = FCoreDelegates::OnEndFrame.AddLambda([]()
	{
		if (!bShowNarrRailDebugHUD)
		{
			return;
		}

		// 获取活跃会话
		TArray<UNarrRailStorySession*> ActiveSessions = UNarrRailStorySession::GetAllActiveSessions();

		UWorld* World = nullptr;
		if (GEngine && GEngine->GameViewport)
		{
			World = GEngine->GameViewport->GetWorld();
		}

		if (World == nullptr)
		{
			return;
		}

		float YPos = 50.0f;
		const float LineHeight = 15.0f;
		const FVector2D ScreenPos(10.0f, YPos);

		// 检查会话状态
		if (ActiveSessions.Num() == 0)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, TEXT("NarrRail Debug: No active sessions"), false);
			return;
		}

		if (ActiveSessions.Num() > 1)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Red,
				FString::Printf(TEXT("NarrRail Debug: Multiple sessions detected (%d)"), ActiveSessions.Num()), false);

			for (int32 i = 0; i < ActiveSessions.Num(); ++i)
			{
				FString StateStr;
				switch (ActiveSessions[i]->GetSessionState())
				{
					case ENarrRailSessionState::Idle: StateStr = TEXT("Idle"); break;
					case ENarrRailSessionState::Running: StateStr = TEXT("Running"); break;
					case ENarrRailSessionState::WaitingForChoice: StateStr = TEXT("WaitingForChoice"); break;
					case ENarrRailSessionState::Paused: StateStr = TEXT("Paused"); break;
					case ENarrRailSessionState::Completed: StateStr = TEXT("Completed"); break;
					case ENarrRailSessionState::Error: StateStr = TEXT("Error"); break;
					default: StateStr = TEXT("Unknown"); break;
				}

				GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White,
					FString::Printf(TEXT("  [%d] %s - %s"), i, *ActiveSessions[i]->GetDebugName(), *StateStr), false);
			}
			return;
		}

		// 使用唯一会话
		const UNarrRailStorySession* Session = ActiveSessions[0];

		// 标题
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Cyan, TEXT("=== NarrRail Debug ==="), false);

		// 会话状态
		FString StateStr;
		switch (Session->GetSessionState())
		{
			case ENarrRailSessionState::Idle: StateStr = TEXT("Idle"); break;
			case ENarrRailSessionState::Running: StateStr = TEXT("Running"); break;
			case ENarrRailSessionState::WaitingForChoice: StateStr = TEXT("WaitingForChoice"); break;
			case ENarrRailSessionState::Paused: StateStr = TEXT("Paused"); break;
			case ENarrRailSessionState::Completed: StateStr = TEXT("Completed"); break;
			case ENarrRailSessionState::Error: StateStr = TEXT("Error"); break;
			default: StateStr = TEXT("Unknown"); break;
		}

		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, TEXT("Session State:"), false);
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, FString::Printf(TEXT("  State: %s"), *StateStr), false);
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, FString::Printf(TEXT("  Current Node: %s"), *Session->GetCurrentNodeId().ToString()), false);
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, FString::Printf(TEXT("  History Count: %d"), Session->GetHistory().Num()), false);

		// 当前节点
		FNarrRailNode CurrentNode;
		if (Session->GetCurrentNode(CurrentNode))
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, TEXT("Current Node:"), false);

			FString NodeTypeStr;
			switch (CurrentNode.NodeType)
			{
				case ENarrRailNodeType::Dialogue: NodeTypeStr = TEXT("Dialogue"); break;
				case ENarrRailNodeType::Choice: NodeTypeStr = TEXT("Choice"); break;
				case ENarrRailNodeType::Jump: NodeTypeStr = TEXT("Jump"); break;
				case ENarrRailNodeType::SetVariable: NodeTypeStr = TEXT("SetVariable"); break;
				case ENarrRailNodeType::EmitEvent: NodeTypeStr = TEXT("EmitEvent"); break;
				case ENarrRailNodeType::End: NodeTypeStr = TEXT("End"); break;
				default: NodeTypeStr = TEXT("Unknown"); break;
			}

			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, FString::Printf(TEXT("  Type: %s"), *NodeTypeStr), false);

			if (CurrentNode.NodeType == ENarrRailNodeType::Dialogue)
			{
				GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, FString::Printf(TEXT("  Speaker: %s"), *CurrentNode.Dialogue.SpeakerId.ToString()), false);
				GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, FString::Printf(TEXT("  Text: %s"), *CurrentNode.Dialogue.TextKey), false);
			}
		}

		// 变量
		UNarrRailVariableContainer* VarContainer = Session->GetVariableContainer();
		if (VarContainer)
		{
			TMap<FName, FString> Snapshot = VarContainer->GetSnapshot();
			if (Snapshot.Num() > 0)
			{
				GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, TEXT("Variables:"), false);
				for (const auto& Pair : Snapshot)
				{
					GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Cyan, FString::Printf(TEXT("  %s = %s"), *Pair.Key.ToString(), *Pair.Value), false);
				}
			}
		}

		// 当前选项
		TArray<FNarrRailChoiceOption> Choices = Session->GetCurrentChoices();
		if (Choices.Num() > 0)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, TEXT("Current Choices:"), false);
			for (int32 i = 0; i < Choices.Num(); ++i)
			{
				GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green,
					FString::Printf(TEXT("  [%d] %s -> %s"), i, *Choices[i].TextKey, *Choices[i].TargetNodeId.ToString()), false);
			}
		}

		// 最后选择
		FNarrRailLastChoiceInfo LastChoice = Session->GetLastChoice();
		if (LastChoice.bValid)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, TEXT("Last Choice:"), false);
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Magenta,
				FString::Printf(TEXT("  [%d] %s -> %s"), LastChoice.ChoiceIndex, *LastChoice.ChoiceTextKey, *LastChoice.TargetNodeId.ToString()), false);
		}

		// 节点历史
		const TArray<FName>& History = Session->GetHistory();
		if (History.Num() > 0)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, TEXT("Node History (last 5):"), false);
			int32 StartIndex = FMath::Max(0, History.Num() - 5);
			for (int32 i = StartIndex; i < History.Num(); ++i)
			{
				GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Silver, FString::Printf(TEXT("  [%d] %s"), i, *History[i].ToString()), false);
			}
		}
	});

	UE_LOG(LogNarrRailDebug, Log, TEXT("NarrRail Editor Module started. Debug commands available:"));
	UE_LOG(LogNarrRailDebug, Log, TEXT("  narrrail.debug.hud        - Toggle on-screen debug HUD"));
	UE_LOG(LogNarrRailDebug, Log, TEXT("  narrrail.debug.session    - Print session state"));
	UE_LOG(LogNarrRailDebug, Log, TEXT("  narrrail.debug.node       - Print current node"));
	UE_LOG(LogNarrRailDebug, Log, TEXT("  narrrail.debug.vars       - Print all variables"));
	UE_LOG(LogNarrRailDebug, Log, TEXT("  narrrail.debug.history    - Print node history"));
	UE_LOG(LogNarrRailDebug, Log, TEXT("  narrrail.debug.choices    - Print current choices"));
	UE_LOG(LogNarrRailDebug, Log, TEXT("  narrrail.debug.lastchoice - Print last choice"));
	UE_LOG(LogNarrRailDebug, Log, TEXT("  narrrail.debug.all        - Print full debug info"));
	UE_LOG(LogNarrRailDebug, Log, TEXT("  narrrail.debug.sessions   - List all active sessions"));
	UE_LOG(LogNarrRailDebug, Log, TEXT("All commands auto-detect the active session."));
}

void FNarrRailEditorModule::ShutdownModule()
{
	// 注销委托
	if (OnEndFrameHandle.IsValid())
	{
		FCoreDelegates::OnEndFrame.Remove(OnEndFrameHandle);
		OnEndFrameHandle.Reset();
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FNarrRailEditorModule, NarrRailEditor)
