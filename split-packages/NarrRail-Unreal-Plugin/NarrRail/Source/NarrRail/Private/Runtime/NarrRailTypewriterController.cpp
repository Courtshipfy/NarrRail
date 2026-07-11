#include "Runtime/NarrRailTypewriterController.h"

#include "Runtime/NarrRailDialogueSettings.h"

void UNarrRailTypewriterController::StartTyping(const FString& InFullText)
{
    FullText = InFullText;
    VisibleText.Empty();
    CurrentIndex = 0;
    CarrySeconds = 0.0f;
    PendingExtraDelay = 0.0f;

    bCompleted = FullText.IsEmpty();
    bTyping = !bCompleted;

    if (bCompleted)
    {
        VisibleText = FullText;
    }
}

void UNarrRailTypewriterController::TickTyping(float DeltaSeconds)
{
    if (!bTyping || bCompleted)
    {
        return;
    }

    const UNarrRailDialogueSettings* Settings = GetDefault<UNarrRailDialogueSettings>();
    const float CPS = FMath::Max(1.0f, Settings ? Settings->CharsPerSecond : 30.0f);
    const float SecondsPerChar = 1.0f / CPS;

    CarrySeconds += FMath::Max(0.0f, DeltaSeconds);

    while (CurrentIndex < FullText.Len())
    {
        if (PendingExtraDelay > 0.0f)
        {
            if (CarrySeconds < PendingExtraDelay)
            {
                break;
            }
            CarrySeconds -= PendingExtraDelay;
            PendingExtraDelay = 0.0f;
        }

        if (CarrySeconds < SecondsPerChar)
        {
            break;
        }

        CarrySeconds -= SecondsPerChar;

        const TCHAR Ch = FullText[CurrentIndex];
        VisibleText.AppendChar(Ch);
        CurrentIndex++;

        if (IsSentenceEndChar(Ch))
        {
            PendingExtraDelay += Settings ? Settings->SentenceEndExtraDelay : 0.15f;
        }
        else if (IsPunctuationChar(Ch))
        {
            PendingExtraDelay += Settings ? Settings->PunctuationExtraDelay : 0.08f;
        }
    }

    if (CurrentIndex >= FullText.Len())
    {
        bTyping = false;
        bCompleted = true;
        VisibleText = FullText;
    }
}

void UNarrRailTypewriterController::CompleteImmediately()
{
    VisibleText = FullText;
    CurrentIndex = FullText.Len();
    bTyping = false;
    bCompleted = true;
    CarrySeconds = 0.0f;
    PendingExtraDelay = 0.0f;
}

bool UNarrRailTypewriterController::IsSentenceEndChar(const TCHAR Ch)
{
    return Ch == TEXT('.') || Ch == TEXT('!') || Ch == TEXT('?')
        || Ch == TEXT('。') || Ch == TEXT('！') || Ch == TEXT('？');
}

bool UNarrRailTypewriterController::IsPunctuationChar(const TCHAR Ch)
{
    return Ch == TEXT(',') || Ch == TEXT('，') || Ch == TEXT('、')
        || Ch == TEXT(';') || Ch == TEXT('；') || Ch == TEXT(':') || Ch == TEXT('：');
}
