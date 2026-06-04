#include "NarrRailEditorModule.h"
#include "Debug/NarrRailDebugger.h"
#include "Debug/NarrRailDebugHUD.h"
#include "AssetTypeActions_NarrRailStory.h"
#include "Importers/NarrRailStoryReimportHandler.h"
#include "Importers/NarrRailYamlParser.h"
#include "NarrRailEditorSettings.h"
#include "Runtime/NarrRailGlobalConfigAsset.h"
#include "Runtime/NarrRailStoryAsset.h"
#include "Runtime/NarrRailStoryValidator.h"
#include "Modules/ModuleManager.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "DesktopPlatformModule.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Application/SlateApplication.h"
#include "LevelEditor.h"
#include "EditorReimportHandler.h"
#include "EditorFramework/AssetImportData.h"
#include "Editor/EditorEngine.h"
#include "FileHelpers.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformProcess.h"
#include "Engine/Engine.h"
#include "GameFramework/HUD.h"
#include "Misc/FileHelper.h"
#include "Misc/CoreDelegates.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "Misc/PackageName.h"
#include "Misc/ScopedSlowTask.h"
#include "ObjectTools.h"
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

namespace
{
	struct FNarrRailRepositorySyncStats
	{
		int32 Created = 0;
		int32 Updated = 0;
		int32 Deleted = 0;
		int32 Failed = 0;
		int32 Skipped = 0;
		TArray<FString> Errors;
	};

	static FString SanitizeLongPackageSegment(const FString& Raw)
	{
		FString Result;
		Result.Reserve(Raw.Len());

		for (const TCHAR Ch : Raw)
		{
			if (FChar::IsAlnum(Ch) || Ch == TEXT('_'))
			{
				Result.AppendChar(Ch);
			}
			else
			{
				Result.AppendChar(TEXT('_'));
			}
		}

		while (Result.Contains(TEXT("__")))
		{
			Result.ReplaceInline(TEXT("__"), TEXT("_"));
		}

		Result.TrimStartAndEndInline();
		Result.RemoveFromStart(TEXT("_"));
		Result.RemoveFromEnd(TEXT("_"));
		return Result.IsEmpty() ? TEXT("Item") : Result;
	}

	static FString NormalizeContentRootPath(FString Path)
	{
		Path.TrimStartAndEndInline();
		FPaths::NormalizeFilename(Path);
		while (Path.EndsWith(TEXT("/")))
		{
			Path.LeftChopInline(1);
		}
		return Path.IsEmpty() ? TEXT("/Game/NarrRailStories") : Path;
	}

	static bool ResolveRepositoryPath(const FString& InPath, FString& OutPath)
	{
		if (InPath.IsEmpty())
		{
			return false;
		}

		OutPath = FPaths::ConvertRelativePathToFull(InPath);
		FPaths::NormalizeDirectoryName(OutPath);
		FPaths::CollapseRelativeDirectories(OutPath);
		return FPaths::DirectoryExists(OutPath);
	}

	static bool PromptForRepositoryPath(const FString& InitialPath, FString& OutPath)
	{
		IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
		if (DesktopPlatform == nullptr)
		{
			return false;
		}

		const void* ParentWindowHandle = nullptr;
		if (FSlateApplication::IsInitialized())
		{
			const TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindBestParentWindowForDialogs(nullptr);
			if (ParentWindow.IsValid() && ParentWindow->GetNativeWindow().IsValid())
			{
				ParentWindowHandle = ParentWindow->GetNativeWindow()->GetOSWindowHandle();
			}
		}

		FString DefaultPath = InitialPath;
		if (!DefaultPath.IsEmpty())
		{
			DefaultPath = FPaths::ConvertRelativePathToFull(DefaultPath);
			FPaths::NormalizeDirectoryName(DefaultPath);
		}

		FString SelectedPath;
		const bool bSelected = DesktopPlatform->OpenDirectoryDialog(
			ParentWindowHandle,
			TEXT("Select NarrRail Story Repository"),
			DefaultPath,
			SelectedPath
		);

		if (!bSelected || SelectedPath.IsEmpty())
		{
			return false;
		}

		FPaths::NormalizeDirectoryName(SelectedPath);
		FPaths::CollapseRelativeDirectories(SelectedPath);
		OutPath = MoveTemp(SelectedPath);
		return FPaths::DirectoryExists(OutPath);
	}

	static bool RunGitCommand(const FString& RepositoryPath, const FString& Arguments, FString& OutStdout, FString& OutStderr, int32& OutReturnCode)
	{
		const FString GitArguments = FString::Printf(TEXT("-C \"%s\" %s"), *RepositoryPath, *Arguments);
		OutReturnCode = -1;
		return FPlatformProcess::ExecProcess(TEXT("git"), *GitArguments, &OutReturnCode, &OutStdout, &OutStderr);
	}

	static bool IsGitRepository(const FString& RepositoryPath)
	{
		FString Stdout;
		FString Stderr;
		int32 ReturnCode = -1;
		if (!RunGitCommand(RepositoryPath, TEXT("rev-parse --is-inside-work-tree"), Stdout, Stderr, ReturnCode))
		{
			return false;
		}

		Stdout.TrimStartAndEndInline();
		return ReturnCode == 0 && Stdout.Equals(TEXT("true"), ESearchCase::IgnoreCase);
	}

	static bool PullGitRepository(const FString& RepositoryPath, FString& OutMessage)
	{
		if (!IsGitRepository(RepositoryPath))
		{
			OutMessage = TEXT("Selected story repository is not a Git working tree; skipped git pull.");
			return true;
		}

		FString Stdout;
		FString Stderr;
		int32 ReturnCode = -1;
		if (!RunGitCommand(RepositoryPath, TEXT("pull --ff-only"), Stdout, Stderr, ReturnCode))
		{
			OutMessage = TEXT("Failed to start git. Make sure Git is installed and available in PATH.");
			return false;
		}

		OutMessage = Stdout;
		if (!Stderr.IsEmpty())
		{
			if (!OutMessage.IsEmpty())
			{
				OutMessage += TEXT("\n");
			}
			OutMessage += Stderr;
		}
		OutMessage.TrimStartAndEndInline();

		return ReturnCode == 0;
	}

	static FString MakeRepositoryAssetRoot(const FString& ContentRoot, const FString& RepositoryPath)
	{
		FString RepoName = FPaths::GetCleanFilename(RepositoryPath);
		if (RepoName.IsEmpty())
		{
			RepoName = TEXT("StoryRepository");
		}

		return NormalizeContentRootPath(ContentRoot) / SanitizeLongPackageSegment(RepoName);
	}

	static bool MakeRelativeRepositoryPath(const FString& RepositoryPath, const FString& SourceFile, FString& OutRelativePath)
	{
		FString RepoWithSlash = RepositoryPath;
		FPaths::NormalizeDirectoryName(RepoWithSlash);
		if (!RepoWithSlash.EndsWith(TEXT("/")))
		{
			RepoWithSlash += TEXT("/");
		}

		OutRelativePath = SourceFile;
		FPaths::NormalizeFilename(OutRelativePath);
		return FPaths::MakePathRelativeTo(OutRelativePath, *RepoWithSlash);
	}

	static FString MakePackagePathForSource(const FString& TargetRoot, const FString& RelativeSourcePath)
	{
		FString WithoutExtension = RelativeSourcePath;
		FPaths::NormalizeFilename(WithoutExtension);
		WithoutExtension = FPaths::ChangeExtension(WithoutExtension, TEXT(""));
		WithoutExtension.RemoveFromEnd(TEXT("."));

		TArray<FString> Parts;
		WithoutExtension.ParseIntoArray(Parts, TEXT("/"), true);

		FString PackagePath = TargetRoot;
		for (const FString& Part : Parts)
		{
			PackagePath /= SanitizeLongPackageSegment(Part);
		}
		return PackagePath;
	}

	static void UpdateImportData(UObject* Asset, const FString& SourceFile)
	{
		FString Normalized = FPaths::ConvertRelativePathToFull(SourceFile);
		FPaths::NormalizeFilename(Normalized);
		FPaths::CollapseRelativeDirectories(Normalized);

		if (UNarrRailStoryAsset* StoryAsset = Cast<UNarrRailStoryAsset>(Asset))
		{
			if (UAssetImportData* ImportData = StoryAsset->EnsureAssetImportData())
			{
				ImportData->UpdateFilenameOnly(Normalized);
			}
		}
		else if (UNarrRailGlobalConfigAsset* ConfigAsset = Cast<UNarrRailGlobalConfigAsset>(Asset))
		{
			if (UAssetImportData* ImportData = ConfigAsset->EnsureAssetImportData())
			{
				ImportData->UpdateFilenameOnly(Normalized);
			}
		}
	}

	static void ApplyStoryData(UNarrRailStoryAsset* StoryAsset, const FNarrRailScriptData& ScriptData)
	{
		StoryAsset->SchemaVersion = ScriptData.SchemaVersion;
		StoryAsset->StoryId = FName(*ScriptData.StoryId);
		StoryAsset->EntryNodeId = FName(*ScriptData.EntryNodeId);
		StoryAsset->Variables = ScriptData.Variables;
		StoryAsset->Nodes = ScriptData.Nodes;
		StoryAsset->Edges = ScriptData.Edges;
	}

	static void ApplyGlobalConfigData(UNarrRailGlobalConfigAsset* ConfigAsset, const FNarrRailGlobalConfigData& ConfigData)
	{
		ConfigAsset->SchemaVersion = ConfigData.SchemaVersion;
		ConfigAsset->Variables = ConfigData.Variables;
		ConfigAsset->PresetSpeakers = ConfigData.PresetSpeakers;
	}
}

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
		FUIAction(FExecuteAction::CreateRaw(this, &FNarrRailEditorModule::SyncStoryRepository)),
		NAME_None,
		LOCTEXT("NarrRail_SyncRepository_Label", "Sync Stories"),
		LOCTEXT("NarrRail_SyncRepository_Tooltip", "Sync NarrRail story assets from the configured local story repository."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Import")
	);
	Builder.EndSection();
}

void FNarrRailEditorModule::SyncStoryRepository()
{
	UNarrRailEditorSettings* Settings = GetMutableDefault<UNarrRailEditorSettings>();
	if (Settings == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NarrRail_Sync_NoSettings", "NarrRail editor settings are unavailable."));
		return;
	}

	FString RepositoryPath;
	if (!ResolveRepositoryPath(Settings->StoryRepositoryPath.Path, RepositoryPath))
	{
		FString SelectedRepositoryPath;
		if (!PromptForRepositoryPath(Settings->StoryRepositoryPath.Path, SelectedRepositoryPath))
		{
			FMessageDialog::Open(
				EAppMsgType::Ok,
				LOCTEXT("NarrRail_Sync_NoRepoSelected", "No valid story repository folder was selected.")
			);
			return;
		}

		Settings->Modify();
		Settings->StoryRepositoryPath.Path = SelectedRepositoryPath;
		Settings->SaveConfig();
		RepositoryPath = SelectedRepositoryPath;
	}

	const FString ContentRoot = NormalizeContentRootPath(Settings->StoryAssetRootPath);
	if (!ContentRoot.StartsWith(TEXT("/Game/")) && ContentRoot != TEXT("/Game"))
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			FText::Format(
				LOCTEXT("NarrRail_Sync_InvalidContentRoot", "Story Asset Root Path must be under /Game:\n{0}"),
				FText::FromString(ContentRoot)
			)
		);
		return;
	}

	const FString TargetRoot = MakeRepositoryAssetRoot(ContentRoot, RepositoryPath);

	if (Settings->bPullGitRepositoryBeforeSync)
	{
		FString GitMessage;
		if (!PullGitRepository(RepositoryPath, GitMessage))
		{
			FMessageDialog::Open(
				EAppMsgType::Ok,
				FText::Format(
					LOCTEXT("NarrRail_Sync_GitPullFailed", "Failed to pull the story repository before sync:\n{0}\n\n{1}"),
					FText::FromString(RepositoryPath),
					FText::FromString(GitMessage)
				)
			);
			return;
		}

		if (!GitMessage.IsEmpty())
		{
			UE_LOG(LogNarrRailDebug, Log, TEXT("NarrRail story repository git pull:\n%s"), *GitMessage);
		}
	}

	TArray<FString> SourceFiles;
	IFileManager::Get().FindFilesRecursive(SourceFiles, *RepositoryPath, TEXT("*.nrstory"), true, false);

	if (SourceFiles.Num() == 0)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			FText::Format(
				LOCTEXT("NarrRail_Sync_NoFiles", "No .nrstory files were found in:\n{0}"),
				FText::FromString(RepositoryPath)
			)
		);
		return;
	}

	FNarrRailYamlParser Parser;
	FNarrRailRepositorySyncStats Stats;
	TMap<FString, FString> ExpectedPackageToSource;
	TMap<FString, ENarrRailScriptFileKind> ExpectedPackageKind;

	for (FString SourceFile : SourceFiles)
	{
		FPaths::NormalizeFilename(SourceFile);
		FString RelativePath;
		if (!MakeRelativeRepositoryPath(RepositoryPath, SourceFile, RelativePath))
		{
			++Stats.Failed;
			Stats.Errors.Add(FString::Printf(TEXT("Failed to compute relative path: %s"), *SourceFile));
			continue;
		}

		FString KindError;
		const ENarrRailScriptFileKind FileKind = Parser.DetectFileKind(SourceFile, KindError);
		if (FileKind == ENarrRailScriptFileKind::Unknown)
		{
			++Stats.Failed;
			Stats.Errors.Add(FString::Printf(TEXT("%s: %s"), *RelativePath, *KindError));
			continue;
		}

		const FString PackageName = MakePackagePathForSource(TargetRoot, RelativePath);
		if (!FPackageName::IsValidLongPackageName(PackageName))
		{
			++Stats.Failed;
			Stats.Errors.Add(FString::Printf(TEXT("Invalid package path for %s -> %s"), *RelativePath, *PackageName));
			continue;
		}

		if (ExpectedPackageToSource.Contains(PackageName))
		{
			++Stats.Failed;
			Stats.Errors.Add(FString::Printf(
				TEXT("Package path collision:\n  %s\n  %s\n  -> %s"),
				*ExpectedPackageToSource[PackageName],
				*SourceFile,
				*PackageName
			));
			continue;
		}

		ExpectedPackageToSource.Add(PackageName, SourceFile);
		ExpectedPackageKind.Add(PackageName, FileKind);
	}

	if (ExpectedPackageToSource.Num() == 0)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			FText::Format(
				LOCTEXT("NarrRail_Sync_NoValidFiles", "No valid NarrRail files were found in:\n{0}\n\nErrors:\n{1}"),
				FText::FromString(RepositoryPath),
				FText::FromString(FString::Join(Stats.Errors, TEXT("\n")))
			)
		);
		return;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> ExistingAssets;
	AssetRegistry.GetAssetsByPath(FName(*TargetRoot), ExistingAssets, true);

	TArray<FAssetData> AssetsToDelete;
	for (const FAssetData& AssetData : ExistingAssets)
	{
		const bool bManagedClass =
			AssetData.AssetClassPath == UNarrRailStoryAsset::StaticClass()->GetClassPathName() ||
			AssetData.AssetClassPath == UNarrRailGlobalConfigAsset::StaticClass()->GetClassPathName();

		if (!bManagedClass)
		{
			continue;
		}

		if (!ExpectedPackageToSource.Contains(AssetData.PackageName.ToString()))
		{
			AssetsToDelete.Add(AssetData);
		}
	}

	if (AssetsToDelete.Num() > 0)
	{
		TArray<FString> DeleteNames;
		const int32 PreviewCount = FMath::Min(AssetsToDelete.Num(), 20);
		for (int32 Index = 0; Index < PreviewCount; ++Index)
		{
			DeleteNames.Add(AssetsToDelete[Index].GetObjectPathString());
		}
		if (AssetsToDelete.Num() > PreviewCount)
		{
			DeleteNames.Add(FString::Printf(TEXT("... and %d more"), AssetsToDelete.Num() - PreviewCount));
		}

		const EAppReturnType::Type ConfirmResult = FMessageDialog::Open(
			EAppMsgType::YesNo,
			FText::Format(
				LOCTEXT("NarrRail_Sync_DeleteConfirm", "The following NarrRail assets no longer exist in the story repository and will be deleted from:\n{0}\n\n{1}\n\nContinue?"),
				FText::FromString(TargetRoot),
				FText::FromString(FString::Join(DeleteNames, TEXT("\n")))
			)
		);

		if (ConfirmResult != EAppReturnType::Yes)
		{
			Stats.Skipped += AssetsToDelete.Num();
			AssetsToDelete.Empty();
		}
	}

	const FText ProgressText = FText::Format(
		LOCTEXT("NarrRail_Sync_Progress", "Syncing NarrRail story repository to {0}..."),
		FText::FromString(TargetRoot)
	);
	FScopedSlowTask SlowTask(static_cast<float>(ExpectedPackageToSource.Num() + AssetsToDelete.Num()), ProgressText);
	SlowTask.MakeDialog(true);

	TArray<UPackage*> PackagesToSave;

	TArray<FString> PackageNames;
	ExpectedPackageToSource.GetKeys(PackageNames);
	PackageNames.Sort([&ExpectedPackageKind](const FString& Left, const FString& Right)
	{
		const ENarrRailScriptFileKind LeftKind = ExpectedPackageKind[Left];
		const ENarrRailScriptFileKind RightKind = ExpectedPackageKind[Right];
		if (LeftKind != RightKind)
		{
			return LeftKind == ENarrRailScriptFileKind::GlobalConfig;
		}
		return Left < Right;
	});

	UNarrRailGlobalConfigAsset* RepositoryGlobalConfigAsset = nullptr;

	for (const FString& PackageName : PackageNames)
	{
		SlowTask.EnterProgressFrame(1.0f, FText::FromString(FPackageName::GetShortName(PackageName)));
		if (SlowTask.ShouldCancel())
		{
			++Stats.Skipped;
			continue;
		}

		const FString& SourceFile = ExpectedPackageToSource[PackageName];
		const ENarrRailScriptFileKind FileKind = ExpectedPackageKind[PackageName];
		const FString ObjectName = FPackageName::GetShortName(PackageName);

		UPackage* Package = CreatePackage(*PackageName);
		if (Package == nullptr)
		{
			++Stats.Failed;
			Stats.Errors.Add(FString::Printf(TEXT("Failed to create package: %s"), *PackageName));
			continue;
		}

		const FString ObjectPath = PackageName + TEXT(".") + ObjectName;
		UObject* ExistingObject = LoadObject<UObject>(nullptr, *ObjectPath);

		if (FileKind == ENarrRailScriptFileKind::Story)
		{
			FNarrRailScriptData ScriptData;
			FString ErrorMessage;
			if (!Parser.ParseFile(SourceFile, ScriptData, ErrorMessage))
			{
				++Stats.Failed;
				Stats.Errors.Add(FString::Printf(TEXT("%s: %s"), *SourceFile, *ErrorMessage));
				continue;
			}

			UNarrRailStoryAsset* StoryAsset = Cast<UNarrRailStoryAsset>(ExistingObject);
			if (ExistingObject != nullptr && StoryAsset == nullptr)
			{
				++Stats.Failed;
				Stats.Errors.Add(FString::Printf(TEXT("Existing asset has wrong type: %s"), *ExistingObject->GetPathName()));
				continue;
			}

			const bool bCreated = StoryAsset == nullptr;
			if (bCreated)
			{
				StoryAsset = NewObject<UNarrRailStoryAsset>(Package, UNarrRailStoryAsset::StaticClass(), *ObjectName, RF_Public | RF_Standalone);
			}
			else
			{
				StoryAsset->Modify();
			}

			ApplyStoryData(StoryAsset, ScriptData);
			StoryAsset->GlobalConfig = RepositoryGlobalConfigAsset;
			bool bHasValidationError = false;
			const TArray<FNarrRailValidationIssue> Issues = UNarrRailStoryValidator::ValidateStoryAssetWithGlobalConfig(StoryAsset, RepositoryGlobalConfigAsset);
			for (const FNarrRailValidationIssue& Issue : Issues)
			{
				if (Issue.Severity == ENarrRailValidationSeverity::Error)
				{
					bHasValidationError = true;
					Stats.Errors.Add(FString::Printf(TEXT("%s: validation error on '%s': %s"), *SourceFile, *Issue.NodeId.ToString(), *Issue.Message));
				}
			}
			if (bHasValidationError)
			{
				++Stats.Failed;
				continue;
			}
			UpdateImportData(StoryAsset, SourceFile);
			StoryAsset->PostEditChange();
			StoryAsset->MarkPackageDirty();

			if (bCreated)
			{
				FAssetRegistryModule::AssetCreated(StoryAsset);
				++Stats.Created;
			}
			else
			{
				++Stats.Updated;
			}
		}
		else if (FileKind == ENarrRailScriptFileKind::GlobalConfig)
		{
			FNarrRailGlobalConfigData ConfigData;
			FString ErrorMessage;
			if (!Parser.ParseGlobalConfigFile(SourceFile, ConfigData, ErrorMessage))
			{
				++Stats.Failed;
				Stats.Errors.Add(FString::Printf(TEXT("%s: %s"), *SourceFile, *ErrorMessage));
				continue;
			}

			UNarrRailGlobalConfigAsset* ConfigAsset = Cast<UNarrRailGlobalConfigAsset>(ExistingObject);
			if (ExistingObject != nullptr && ConfigAsset == nullptr)
			{
				++Stats.Failed;
				Stats.Errors.Add(FString::Printf(TEXT("Existing asset has wrong type: %s"), *ExistingObject->GetPathName()));
				continue;
			}

			const bool bCreated = ConfigAsset == nullptr;
			if (bCreated)
			{
				ConfigAsset = NewObject<UNarrRailGlobalConfigAsset>(Package, UNarrRailGlobalConfigAsset::StaticClass(), *ObjectName, RF_Public | RF_Standalone);
			}
			else
			{
				ConfigAsset->Modify();
			}

			ApplyGlobalConfigData(ConfigAsset, ConfigData);
			if (RepositoryGlobalConfigAsset == nullptr)
			{
				RepositoryGlobalConfigAsset = ConfigAsset;
			}
			else
			{
				++Stats.Failed;
				Stats.Errors.Add(FString::Printf(TEXT("Multiple GlobalConfig files found in one repository. Using %s and skipping %s."), *RepositoryGlobalConfigAsset->GetPathName(), *SourceFile));
				continue;
			}
			UpdateImportData(ConfigAsset, SourceFile);
			ConfigAsset->PostEditChange();
			ConfigAsset->MarkPackageDirty();

			if (bCreated)
			{
				FAssetRegistryModule::AssetCreated(ConfigAsset);
				++Stats.Created;
			}
			else
			{
				++Stats.Updated;
			}
		}

		PackagesToSave.AddUnique(Package);
	}

	if (AssetsToDelete.Num() > 0)
	{
		for (const FAssetData& AssetData : AssetsToDelete)
		{
			SlowTask.EnterProgressFrame(1.0f, FText::FromString(AssetData.GetObjectPathString()));
		}

		const int32 DeletedCount = ObjectTools::DeleteAssets(AssetsToDelete, false);
		Stats.Deleted += DeletedCount;
		if (DeletedCount < AssetsToDelete.Num())
		{
			Stats.Failed += (AssetsToDelete.Num() - DeletedCount);
		}
	}

	if (PackagesToSave.Num() > 0)
	{
		UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, true);
	}

	FString ErrorText;
	if (Stats.Errors.Num() > 0)
	{
		const int32 ErrorPreviewCount = FMath::Min(Stats.Errors.Num(), 12);
		TArray<FString> ErrorPreview;
		for (int32 Index = 0; Index < ErrorPreviewCount; ++Index)
		{
			ErrorPreview.Add(Stats.Errors[Index]);
		}
		if (Stats.Errors.Num() > ErrorPreviewCount)
		{
			ErrorPreview.Add(FString::Printf(TEXT("... and %d more"), Stats.Errors.Num() - ErrorPreviewCount));
		}
		ErrorText = TEXT("\n\nErrors:\n") + FString::Join(ErrorPreview, TEXT("\n"));
	}

	const FText Summary = FText::Format(
		LOCTEXT("NarrRail_Sync_Summary", "Story repository sync finished.\nRepository: {0}\nTarget: {1}\nCreated: {2}\nUpdated: {3}\nDeleted: {4}\nFailed: {5}\nSkipped: {6}{7}"),
		FText::FromString(RepositoryPath),
		FText::FromString(TargetRoot),
		FText::AsNumber(Stats.Created),
		FText::AsNumber(Stats.Updated),
		FText::AsNumber(Stats.Deleted),
		FText::AsNumber(Stats.Failed),
		FText::AsNumber(Stats.Skipped),
		FText::FromString(ErrorText)
	);

	FMessageDialog::Open(EAppMsgType::Ok, Summary, LOCTEXT("NarrRail_Sync_Title", "NarrRail Story Repository Sync"));
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
