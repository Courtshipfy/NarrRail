#include "NarrRailEditorModule.h"
#include "Debug/NarrRailDebugger.h"
#include "Debug/NarrRailDebugHUD.h"
#include "AssetTypeActions_NarrRailStory.h"
#include "Importers/NarrRailStoryReimportHandler.h"
#include "Runtime/NarrRailStoryAsset.h"
#include "Modules/ModuleManager.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "LevelEditor.h"
#include "EditorReimportHandler.h"
#include "EditorFramework/AssetImportData.h"
#include "HAL/IConsoleManager.h"
#include "Engine/Engine.h"
#include "GameFramework/HUD.h"
#include "Misc/CoreDelegates.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "Misc/PackageName.h"
#include "Misc/ScopedSlowTask.h"
#include "Styling/AppStyle.h"
#include "DrawDebugHelpers.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"

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

	// Register asset tools
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// Register NarrRail asset category
	EAssetTypeCategories::Type NarrRailCategory = AssetTools.RegisterAdvancedAssetCategory(
		FName(TEXT("NarrRail")),
		FText::FromString(TEXT("NarrRail"))
	);

	// Register asset type actions
	TSharedPtr<FAssetTypeActions_NarrRailStory> NarrRailAssetActions =
		MakeShareable(new FAssetTypeActions_NarrRailStory(NarrRailCategory));
	AssetTools.RegisterAssetTypeActions(NarrRailAssetActions.ToSharedRef());
	CreatedAssetTypeActions.Add(NarrRailAssetActions);

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

	StoryReimportHandler = MakeUnique<FNarrRailStoryReimportHandler>();
	FReimportManager::Instance()->RegisterHandler(*StoryReimportHandler);

	// Add a toolbar button for one-click bulk reimport.
	if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		ToolbarExtender = MakeShared<FExtender>();
		ToolbarExtender->AddToolBarExtension(
			"Play",
			EExtensionHook::After,
			nullptr,
			FToolBarExtensionDelegate::CreateRaw(this, &FNarrRailEditorModule::AddToolbarExtension)
		);
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}

void FNarrRailEditorModule::ShutdownModule()
{
	// Unregister asset type actions
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (auto& AssetTypeAction : CreatedAssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(AssetTypeAction.ToSharedRef());
		}
	}
	CreatedAssetTypeActions.Empty();

	if (StoryReimportHandler.IsValid())
	{
		FReimportManager::Instance()->UnregisterHandler(*StoryReimportHandler);
		StoryReimportHandler.Reset();
	}

	if (ToolbarExtender.IsValid() && FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditorModule.GetToolBarExtensibilityManager()->RemoveExtender(ToolbarExtender);
		ToolbarExtender.Reset();
	}

	// 注销委托
	if (OnEndFrameHandle.IsValid())
	{
		FCoreDelegates::OnEndFrame.Remove(OnEndFrameHandle);
		OnEndFrameHandle.Reset();
	}
}

void FNarrRailEditorModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.BeginSection("NarrRailTools");
	Builder.AddToolBarButton(
		FUIAction(FExecuteAction::CreateRaw(this, &FNarrRailEditorModule::ReimportAllNarrRailStories)),
		NAME_None,
		LOCTEXT("NarrRail_ReimportAll_Label", "Reimport Stories"),
		LOCTEXT("NarrRail_ReimportAll_Tooltip", "Reimport all NarrRail story assets from their source YAML files."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Import")
	);
	Builder.EndSection();
}

void FNarrRailEditorModule::ReimportAllNarrRailStories()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	const FString CurrentLanguage = FInternationalization::Get().GetCurrentLanguage()->GetName();
	const bool bUseChinese = CurrentLanguage.StartsWith(TEXT("zh"), ESearchCase::IgnoreCase);

	TArray<FAssetData> StoryAssets;
	AssetRegistry.GetAssetsByClass(UNarrRailStoryAsset::StaticClass()->GetClassPathName(), StoryAssets, true);

	// Keep only one top-level asset per package.
	// Some legacy packages may contain extra standalone exports (same package, different asset names),
	// which would otherwise be counted/reimported repeatedly.
	TArray<FAssetData> UniquePackageStoryAssets;
	UniquePackageStoryAssets.Reserve(StoryAssets.Num());
	for (const FAssetData& AssetData : StoryAssets)
	{
		const FString PackagePath = AssetData.PackageName.ToString();
		const FString PackageShortName = FPackageName::GetShortName(PackagePath);
		if (AssetData.AssetName.ToString().Equals(PackageShortName, ESearchCase::CaseSensitive))
		{
			UniquePackageStoryAssets.Add(AssetData);
		}
		else
		{
			if (bUseChinese)
			{
				UE_LOG(
					LogNarrRailDebug,
					Warning,
					TEXT("NarrRail 批量重导入已跳过重复导出对象：%s（包=%s，期望主对象名=%s）"),
					*AssetData.GetObjectPathString(),
					*PackagePath,
					*PackageShortName
				);
			}
			else
			{
				UE_LOG(
					LogNarrRailDebug,
					Warning,
					TEXT("NarrRail bulk reimport skipped duplicate top-level export: %s (package=%s, expected=%s)"),
					*AssetData.GetObjectPathString(),
					*PackagePath,
					*PackageShortName
				);
			}
		}
	}

	int32 Succeeded = 0;
	int32 Failed = 0;
	int32 Cancelled = 0;
	int32 SkippedMissingSource = 0;

	auto ResolveReadableSourcePath = [](const FString& InPath, FString& OutResolvedAbsPath) -> bool
	{
		if (InPath.IsEmpty())
		{
			return false;
		}

		TArray<FString> Candidates;
		Candidates.Reserve(4);
		Candidates.Add(InPath);
		Candidates.Add(FPaths::ConvertRelativePathToFull(InPath));

		if (InPath.StartsWith(TEXT("/")) || InPath.StartsWith(TEXT("\\")))
		{
			const FString Trimmed = InPath.RightChop(1);
			Candidates.Add(FPaths::Combine(FPaths::ProjectDir(), Trimmed));
			Candidates.Add(FPaths::Combine(FPaths::RootDir(), Trimmed));
		}

		for (FString Candidate : Candidates)
		{
			FPaths::NormalizeFilename(Candidate);
			FPaths::CollapseRelativeDirectories(Candidate);
			if (FPaths::FileExists(Candidate))
			{
				OutResolvedAbsPath = MoveTemp(Candidate);
				return true;
			}
		}

		return false;
	};

	const FText ProgressText = bUseChinese
		? FText::FromString(TEXT("正在重导入 NarrRail 剧情资产..."))
		: FText::FromString(TEXT("Reimporting NarrRail story assets..."));
	FScopedSlowTask SlowTask(static_cast<float>(UniquePackageStoryAssets.Num()), ProgressText);
	SlowTask.MakeDialog(true);

	for (const FAssetData& AssetData : UniquePackageStoryAssets)
	{
		SlowTask.EnterProgressFrame(1.0f, FText::FromName(AssetData.AssetName));
		if (SlowTask.ShouldCancel())
		{
			Cancelled += (UniquePackageStoryAssets.Num() - Succeeded - Failed - SkippedMissingSource);
			break;
		}

		UNarrRailStoryAsset* StoryAsset = Cast<UNarrRailStoryAsset>(AssetData.GetAsset());
		if (StoryAsset == nullptr)
		{
			++Failed;
			continue;
		}

		const UAssetImportData* ImportData = StoryAsset->GetAssetImportData();
		const FString SourceFilename = ImportData ? ImportData->GetFirstFilename() : FString();
		FString ResolvedSourcePath;
		if (SourceFilename.IsEmpty() || !ResolveReadableSourcePath(SourceFilename, ResolvedSourcePath))
		{
			++SkippedMissingSource;
			if (bUseChinese)
			{
				UE_LOG(
					LogNarrRailDebug,
					Warning,
					TEXT("NarrRail 批量重导入已跳过（源文件不存在）：%s | 源路径='%s'"),
					*StoryAsset->GetPathName(),
					*SourceFilename
				);
			}
			else
			{
				UE_LOG(
					LogNarrRailDebug,
					Warning,
					TEXT("NarrRail bulk reimport skipped (missing source): %s | source='%s'"),
					*StoryAsset->GetPathName(),
					*SourceFilename
				);
			}
			continue;
		}

		const bool bReimported = FReimportManager::Instance()->Reimport(StoryAsset, false, true);
		if (bReimported)
		{
			++Succeeded;
		}
		else
		{
			++Failed;
			UE_LOG(LogNarrRailDebug, Warning, TEXT("NarrRail bulk reimport failed: %s"), *StoryAsset->GetPathName());
		}
	}

	const FText SummaryPattern = bUseChinese
		? FText::FromString(TEXT("重导入完成。\n总计（去重后包数）：{0}\n已跳过（重复导出对象）：{1}\n成功：{2}\n失败：{3}\n已跳过（源文件不存在）：{4}\n已取消：{5}"))
		: FText::FromString(TEXT("Reimport finished.\nTotal(unique packages): {0}\nSkipped(duplicate exports): {1}\nSucceeded: {2}\nFailed: {3}\nSkipped(missing source): {4}\nCancelled: {5}"));
	const FText Summary = FText::Format(
		SummaryPattern,
		FText::AsNumber(UniquePackageStoryAssets.Num()),
		FText::AsNumber(StoryAssets.Num() - UniquePackageStoryAssets.Num()),
		FText::AsNumber(Succeeded),
		FText::AsNumber(Failed),
		FText::AsNumber(SkippedMissingSource),
		FText::AsNumber(Cancelled)
	);

	FMessageDialog::Open(EAppMsgType::Ok, Summary, LOCTEXT("NarrRail_ReimportAll_Title", "NarrRail Reimport All"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FNarrRailEditorModule, NarrRailEditor)
