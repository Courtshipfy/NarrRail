#include "NarrRailPlayerController.h"

#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "NarrRailHostSaveGame.h"
#include "Runtime/NarrRailGlobalStateSubsystem.h"
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
    if (Session == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[NarrRailHost] LoadNarrRailState failed: no bound session."));
        return false;
    }

    USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex);
    UNarrRailHostSaveGame* SaveGame = Cast<UNarrRailHostSaveGame>(Loaded);
    if (SaveGame == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("[NarrRailHost] LoadNarrRailState failed: slot '%s' has no NarrRail save."), *SlotName);
        return false;
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
