#include "NarrRailPlayerController.h"

#include "InputCoreTypes.h"
#include "Runtime/NarrRailTypewriterController.h"

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
