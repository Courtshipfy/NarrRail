#include "Misc/AutomationTest.h"

#include "Runtime/NarrRailGlobalConfigAsset.h"
#include "Runtime/NarrRailGlobalStateSubsystem.h"
#include "Runtime/NarrRailStoryAsset.h"
#include "Runtime/NarrRailStorySession.h"
#include "Engine/GameInstance.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace NarrRailSaveSnapshotTests
{
static FNarrRailVariableDefinition MakeVariable(const FName Name, const ENarrRailVariableType Type, const FString& DefaultValue, const bool bGlobalScope = false)
{
	FNarrRailVariableDefinition Definition;
	Definition.VariableName = Name;
	Definition.VariableType = Type;
	Definition.DefaultValue = DefaultValue;
	Definition.bGlobalScope = bGlobalScope;
	return Definition;
}

static UNarrRailStoryAsset* MakeStory(const TCHAR* ObjectName, const FName StoryId)
{
	UNarrRailStoryAsset* Story = NewObject<UNarrRailStoryAsset>(GetTransientPackage(), FName(ObjectName));
	Story->StoryId = StoryId;
	Story->EntryNodeId = TEXT("Intro");
	Story->Variables.Add(MakeVariable(TEXT("Score"), ENarrRailVariableType::Int, TEXT("0")));

	FNarrRailNode Intro;
	Intro.NodeId = TEXT("Intro");
	Intro.NodeType = ENarrRailNodeType::MultiDialogue;
	Intro.MultiDialogue.SpeakerId = TEXT("Narrator");

	FNarrRailDialogueLine LineA;
	LineA.TextKey = TEXT("LineA");
	Intro.MultiDialogue.Lines.Add(LineA);

	FNarrRailDialogueLine LineB;
	LineB.TextKey = TEXT("LineB");
	Intro.MultiDialogue.Lines.Add(LineB);

	Story->Nodes.Add(Intro);
	return Story;
}

static UNarrRailGlobalConfigAsset* MakeGlobalConfig(const TCHAR* PackageName, const TCHAR* ObjectName)
{
	UPackage* Package = CreatePackage(PackageName);
	UNarrRailGlobalConfigAsset* Config = NewObject<UNarrRailGlobalConfigAsset>(Package, FName(ObjectName), RF_Public);
	Config->Variables.Add(MakeVariable(TEXT("Trust"), ENarrRailVariableType::Int, TEXT("0"), true));

	FNarrRailPresetSpeaker Speaker;
	Speaker.SpeakerId = TEXT("Alice");
	Speaker.DisplayName = TEXT("Alice Display");
	Speaker.Color = FLinearColor::Red;
	Config->PresetSpeakers.Add(Speaker);

	return Config;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNarrRailSessionSnapshotRestoreSameStoryTest,
	"NarrRail.Save.SessionSnapshot.RestoreSameStory",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNarrRailSessionSnapshotRestoreSameStoryTest::RunTest(const FString& Parameters)
{
	using namespace NarrRailSaveSnapshotTests;

	UNarrRailStoryAsset* Story = MakeStory(TEXT("NR_SaveSnapshot_SameStory"), TEXT("SameStory"));

	UNarrRailStorySession* SourceSession = NewObject<UNarrRailStorySession>(GetTransientPackage());
	TestEqual(TEXT("Source Initialize succeeds"), SourceSession->Initialize(Story).Code, ENarrRailRuntimeResultCode::Success);
	TestEqual(TEXT("Source Start succeeds"), SourceSession->Start().Code, ENarrRailRuntimeResultCode::Success);
	TestEqual(TEXT("Source advances inside multi-dialogue"), SourceSession->Next().Code, ENarrRailRuntimeResultCode::Success);
	TestTrue(TEXT("Source SetVariableInt succeeds"), SourceSession->SetVariableInt(TEXT("Score"), 7).IsSuccess());

	const FNarrRailStorySessionSnapshot Snapshot = SourceSession->GetSessionSnapshot();

	UNarrRailStorySession* RestoredSession = NewObject<UNarrRailStorySession>(GetTransientPackage());
	TestEqual(TEXT("Restored Initialize succeeds"), RestoredSession->Initialize(Story).Code, ENarrRailRuntimeResultCode::Success);
	TestTrue(TEXT("Restored pre-state can change"), RestoredSession->SetVariableInt(TEXT("Score"), 1).IsSuccess());

	const FNarrRailRuntimeResult RestoreResult = RestoredSession->RestoreSessionSnapshot(Snapshot, false);
	TestEqual(TEXT("Restore succeeds"), RestoreResult.Code, ENarrRailRuntimeResultCode::Success);
	TestEqual(TEXT("Current node restored"), RestoredSession->GetCurrentNodeId(), FName(TEXT("Intro")));
	TestEqual(TEXT("Multi-dialogue line restored"), RestoredSession->GetCurrentMultiDialogueLineIndex(), 1);
	TestEqual(TEXT("Session state restored"), RestoredSession->GetSessionState(), ENarrRailSessionState::Running);

	int32 Score = 0;
	TestTrue(TEXT("Restored Score can be read"), RestoredSession->GetVariableInt(TEXT("Score"), Score).IsSuccess());
	TestEqual(TEXT("Local variable value restored"), Score, 7);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNarrRailSessionSnapshotRejectWrongStoryTest,
	"NarrRail.Save.SessionSnapshot.RejectWrongStoryAsset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNarrRailSessionSnapshotRejectWrongStoryTest::RunTest(const FString& Parameters)
{
	using namespace NarrRailSaveSnapshotTests;

	UNarrRailStoryAsset* StoryA = MakeStory(TEXT("NR_SaveSnapshot_StoryA"), TEXT("StoryA"));
	UNarrRailStoryAsset* StoryB = MakeStory(TEXT("NR_SaveSnapshot_StoryB"), TEXT("StoryB"));

	UNarrRailStorySession* SourceSession = NewObject<UNarrRailStorySession>(GetTransientPackage());
	TestEqual(TEXT("Source Initialize succeeds"), SourceSession->Initialize(StoryA).Code, ENarrRailRuntimeResultCode::Success);
	TestEqual(TEXT("Source Start succeeds"), SourceSession->Start().Code, ENarrRailRuntimeResultCode::Success);

	UNarrRailStorySession* WrongSession = NewObject<UNarrRailStorySession>(GetTransientPackage());
	TestEqual(TEXT("Wrong Initialize succeeds"), WrongSession->Initialize(StoryB).Code, ENarrRailRuntimeResultCode::Success);

	const FNarrRailRuntimeResult RestoreResult = WrongSession->RestoreSessionSnapshot(SourceSession->GetSessionSnapshot(), false);
	TestEqual(TEXT("Wrong story snapshot is rejected"), RestoreResult.Code, ENarrRailRuntimeResultCode::InvalidInput);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNarrRailGlobalStateSnapshotRestoreDefinitionsTest,
	"NarrRail.Save.GlobalStateSnapshot.RestoresPresetSpeakersAndValues",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNarrRailGlobalStateSnapshotRestoreDefinitionsTest::RunTest(const FString& Parameters)
{
	using namespace NarrRailSaveSnapshotTests;

	UNarrRailGlobalConfigAsset* Config = MakeGlobalConfig(TEXT("/NarrRailAutomation/SaveSnapshotGlobalConfig"), TEXT("SaveSnapshotGlobalConfig"));

	UGameInstance* SourceGameInstance = NewObject<UGameInstance>(GetTransientPackage());
	UNarrRailGlobalStateSubsystem* SourceState = NewObject<UNarrRailGlobalStateSubsystem>(SourceGameInstance);
	FString ErrorMessage;
	TestTrue(TEXT("ApplyGlobalConfig succeeds"), SourceState->ApplyGlobalConfig(Config, ErrorMessage));
	TestNotNull(TEXT("Source global variables exist"), SourceState->GetGlobalVariableContainer());
	TestTrue(TEXT("Source Trust write succeeds"), SourceState->GetGlobalVariableContainer()->SetInt(TEXT("Trust"), 42).IsSuccess());

	const FNarrRailGlobalStateSnapshot Snapshot = SourceState->GetGlobalStateSnapshot();
	TestTrue(TEXT("Snapshot records applied global config path"), Snapshot.AppliedGlobalConfigPaths.Num() > 0);

	UGameInstance* RestoredGameInstance = NewObject<UGameInstance>(GetTransientPackage());
	UNarrRailGlobalStateSubsystem* RestoredState = NewObject<UNarrRailGlobalStateSubsystem>(RestoredGameInstance);
	ErrorMessage.Reset();
	TestTrue(TEXT("RestoreGlobalStateSnapshot succeeds"), RestoredState->RestoreGlobalStateSnapshot(Snapshot, ErrorMessage));

	FNarrRailPresetSpeaker RestoredSpeaker;
	TestTrue(TEXT("Preset speaker restored"), RestoredState->GetPresetSpeaker(TEXT("Alice"), RestoredSpeaker));
	TestEqual(TEXT("Preset speaker display name restored"), RestoredSpeaker.DisplayName, FString(TEXT("Alice Display")));

	int32 Trust = 0;
	TestNotNull(TEXT("Restored global variables exist"), RestoredState->GetGlobalVariableContainer());
	TestTrue(TEXT("Restored Trust can be read"), RestoredState->GetGlobalVariableContainer()->GetInt(TEXT("Trust"), Trust).IsSuccess());
	TestEqual(TEXT("Global variable value restored"), Trust, 42);

	return true;
}

#endif
