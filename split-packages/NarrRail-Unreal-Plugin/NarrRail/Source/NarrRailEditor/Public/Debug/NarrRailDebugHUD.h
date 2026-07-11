#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Runtime/NarrRailStorySession.h"

/**
 * NarrRail 调试 HUD 绘制器
 * 用于 showdebug narrrail 命令的屏幕显示
 */
class NARRRAILEDITOR_API FNarrRailDebugHUD
{
public:
    /**
     * 绘制调试信息到屏幕
     * @param Canvas 画布
     * @param Session 会话对象
     */
    static void DrawDebugInfo(class UCanvas* Canvas, const UNarrRailStorySession* Session);

private:
    static void DrawSessionState(class UCanvas* Canvas, const UNarrRailStorySession* Session, float& YPos);
    static void DrawCurrentNode(class UCanvas* Canvas, const UNarrRailStorySession* Session, float& YPos);
    static void DrawVariables(class UCanvas* Canvas, const UNarrRailStorySession* Session, float& YPos);
    static void DrawNodeHistory(class UCanvas* Canvas, const UNarrRailStorySession* Session, float& YPos);
    static void DrawCurrentChoices(class UCanvas* Canvas, const UNarrRailStorySession* Session, float& YPos);
    static void DrawLastChoice(class UCanvas* Canvas, const UNarrRailStorySession* Session, float& YPos);

    static FString GetSessionStateString(ENarrRailSessionState State);
    static FString GetNodeTypeString(ENarrRailNodeType NodeType);
};
