#include "NarrRailPlayerController.h"

#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "NarrRailHostSaveGame.h"
#include "Runtime/NarrRailGlobalStateSubsystem.h"
#include "Runtime/NarrRailGlobalConfigAsset.h"
#include "Runtime/NarrRailTypewriterController.h"

ANarrRailPlayerController::ANarrRailPlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
}

void ANarrRailPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (InputComponent)
    {
        InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &ANarrRailPlayerController::HandleAdvancePressed);
    }
}

void ANarrRailPlayerController::BindNarrRailSession(UNarrRailStorySession* InSession)
{
    Session = InSession;
}

void ANarrRailPlayerController::BindTypewriterController(UNarrRailTypewriterController* InTypewriter)
{
    Typewriter = InTypewriter;
}

bool ANarrRailPlayerController::SaveNarrRailState(const FString& SlotName, const int32 UserIndex)
{
    if (Session == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[NarrRailHost] SaveNarrRailState failed: no bound session."));
        return false;
    }

    UNarrRailHostSaveGame* SaveGame = Cast<UNarrRailHostSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UNarrRailHostSaveGame::StaticClass()));
    if (SaveGame == nullptr)
    {
        return false;
    }

    SaveGame->SessionSnapshot = Session->GetSessionSnapshot();
    SaveGame->SavedAtUtc = FDateTime::UtcNow().ToIso8601();

    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UNarrRailGlobalStateSubsystem* GlobalState = GameInstance->GetSubsystem<UNarrRailGlobalStateSubsystem>())
        {
            SaveGame->GlobalStateSnapshot = GlobalState->GetGlobalStateSnapshot();
        }
    }

    return UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, UserIndex);
}

bool ANarrRailPlayerController::LoadNarrRailState(const FString& SlotName, const int32 UserIndex, const bool bRefreshPresenter)
{
    USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex);
    UNarrRailHostSaveGame* SaveGame = Cast<UNarrRailHostSaveGame>(Loaded);
    if (SaveGame == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[NarrRailHost] LoadNarrRailState failed: slot '%s' has no NarrRail save."), *SlotName);
        return false;
    }

    const FSoftObjectPath& StoryAssetPath = SaveGame->SessionSnapshot.StoryAssetPath;
    const bool bNeedsSessionFromSave =
        Session == nullptr ||
        (!StoryAssetPath.IsNull() && Session->GetSessionSnapshot().StoryAssetPath != StoryAssetPath);

    if (bNeedsSessionFromSave)
    {
        UNarrRailStoryAsset* StoryAsset = Cast<UNarrRailStoryAsset>(StoryAssetPath.TryLoad());
        if (StoryAsset == nullptr)
        {
            UE_LOG(LogTemp, Warning, TEXT("[NarrRailHost] LoadNarrRailState failed: story asset '%s' could not be loaded."), *StoryAssetPath.ToString());
            return false;
        }

        const FSoftObjectPath& GlobalConfigPath = SaveGame->SessionSnapshot.GlobalConfigPath;
        UNarrRailGlobalConfigAsset* GlobalConfigAsset = nullptr;
        if (!GlobalConfigPath.IsNull())
        {
            GlobalConfigAsset = Cast<UNarrRailGlobalConfigAsset>(GlobalConfigPath.TryLoad());
            if (GlobalConfigAsset == nullptr)
            {
                UE_LOG(LogTemp, Warning, TEXT("[NarrRailHost] LoadNarrRailState failed: global config asset '%s' could not be loaded."), *GlobalConfigPath.ToString());
                return false;
            }
        }

        TScriptInterface<INarrRailDialoguePresenterInterface> ExistingPresenter;
        if (Session != nullptr)
        {
            ExistingPresenter = Session->GetDialoguePresenter();
        }

        UNarrRailStorySession* NewSession = NewObject<UNarrRailStorySession>(this);
        if (NewSession == nullptr)
        {
            return false;
        }

        const FNarrRailRuntimeResult InitResult = GlobalConfigAsset != nullptr
            ? NewSession->InitializeWithGlobalConfig(StoryAsset, GlobalConfigAsset)
            : NewSession->Initialize(StoryAsset);
        if (InitResult.Code != ENarrRailRuntimeResultCode::Success)
        {
            UE_LOG(LogTemp, Warning, TEXT("[NarrRailHost] LoadNarrRailState failed: session init failed: %s"), *InitResult.Message);
            return false;
        }

        if (ExistingPresenter.GetObject() != nullptr)
        {
            NewSession->RegisterDialoguePresenter(ExistingPresenter);
        }

        Session = NewSession;
    }

    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UNarrRailGlobalStateSubsystem* GlobalState = GameInstance->GetSubsystem<UNarrRailGlobalStateSubsystem>())
        {
            FString ErrorMessage;
            if (!GlobalState->RestoreGlobalStateSnapshot(SaveGame->GlobalStateSnapshot, ErrorMessage))
            {
                UE_LOG(LogTemp, Warning, TEXT("[NarrRailHost] Restore global state failed: %s"), *ErrorMessage);
                return false;
            }
        }
    }

    const FNarrRailRuntimeResult Result = Session->RestoreSessionSnapshot(SaveGame->SessionSnapshot, bRefreshPresenter);
    if (Result.Code != ENarrRailRuntimeResultCode::Success)
    {
        UE_LOG(LogTemp, Warning, TEXT("[NarrRailHost] Restore session failed: %s"), *Result.Message);
        return false;
    }

    return true;
}

void ANarrRailPlayerController::HandleAdvancePressed()
{
    if (Typewriter && Typewriter->IsTyping())
    {
        Typewriter->CompleteImmediately();
        return;
    }

    if (Session == nullptr)
    {
        return;
    }

    const ENarrRailSessionState State = Session->GetSessionState();
    if (State == ENarrRailSessionState::WaitingForChoice)
    {
        return;
    }

    Session->Next();
}
