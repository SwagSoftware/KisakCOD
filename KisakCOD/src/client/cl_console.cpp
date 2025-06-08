#include "client.h"
#include <qcommon/mem_track.h>

#include <client_mp/client_mp.h>
#include <qcommon/cmd.h>
#include <stringed/stringed_hooks.h>
#include <win32/win_local.h>
#include <cgame_mp/cg_local_mp.h>
#include <gfx_d3d/r_rendercmds.h>
#include <qcommon/threads.h>
#include <universal/com_files.h>
#include <buildnumber_mp.h>

//int con_inputMaxMatchesShown 827b26dc     cl_console.obj
//int g_console_field_width 827b26e0     cl_console.obj
//float g_console_char_height 827b26e4     cl_console.obj
//char (*)[29] con_gameMsgWindowNFadeInTime_Names 82850d78     cl_console.obj
//char (*)[55] con_gameMsgWindowNFadeOutTime_Descs 82850df0     cl_console.obj

//char (*)[48] con_gameMsgWindowNSplitscreenScale_Descs 82850ef8     cl_console.obj
//char (*)[26] con_gameMsgWindowNMsgTime_Names 82851020     cl_console.obj
//char (*)[73] con_gameMsgWindowNLineCount_Descs 82851088     cl_console.obj
//char (*)[28] con_gameMsgWindowNLineCount_Names 828511b0     cl_console.obj
//char (*)[84] con_gameMsgWindowNScrollTime_Descs 82851220     cl_console.obj
//char (*)[69] con_gameMsgWindowNMsgTime_Descs 82851390     cl_console.obj
//char (*)[30] con_gameMsgWindowNFadeOutTime_Names 828514a8     cl_console.obj
//char (*)[35] con_gameMsgWindowNSplitscreenScale_Names 82851520     cl_console.obj
//char (*)[29] con_gameMsgWindowNScrollTime_Names 828739a8     cl_console.obj
//char (*)[54] con_gameMsgWindowNFadeInTime_Descs 82873a20     cl_console.obj
//BOOL con_ignoreMatchPrefixOnly 82851380     cl_console.obj
// 
const dvar_t *con_typewriterColorGlowFailed;
const dvar_t *con_typewriterColorGlowCompleted;
const dvar_t *con_typewriterColorGlowCheckpoint;
const dvar_t *cl_deathMessageWidth;
const dvar_t *con_typewriterColorBase;
const dvar_t *con_matchPrefixOnly;
const dvar_t *con_typewriterColorGlowUpdated;
const dvar_t *con_typewriterDecayDuration;
const dvar_t *con_typewriterDecayStartTime;
const dvar_t *con_restricted;
const dvar_t *con_typewriterPrintSpeed;

const dvar_t *con_inputBoxColor;
const dvar_t *con_inputHintBoxColor;
const dvar_t *con_outputBarColor;
const dvar_t *con_outputSliderColor;
const dvar_t *con_errormessagetime;
const dvar_t *con_minicontime;
const dvar_t *con_miniconlines;
const dvar_t *con_outputWindowColor;

Console con;

int g_console_field_width = 620;
float g_console_char_height = 16.0f;

int callDepth;

char con_gameMsgWindowNMsgTime_Descs[4][69];
char con_gameMsgWindowNMsgTime_Names[4][26];
const dvar_s *con_gameMsgWindowNMsgTime[4];
const float defaultGameMessageTimes[4] = { 5.0f, 8.0f, 5.0f, 5.0f };
char con_gameMsgWindowNLineCount_Descs[4][73];
char con_gameMsgWindowNLineCount_Names[4][28];
const dvar_s *con_gameMsgWindowNLineCount[4];
const int defaultGameMessageWindowLineCounts[4] = { 4, 5, 7, 5 };
char con_gameMsgWindowNScrollTime_Descs[4][84];
char con_gameMsgWindowNScrollTime_Names[4][29];
const dvar_s *con_gameMsgWindowNScrollTime[4];
char con_gameMsgWindowNFadeInTime_Descs[4][54];
char con_gameMsgWindowNFadeInTime_Names[4][29];
const dvar_s *con_gameMsgWindowNFadeInTime[4];
char con_gameMsgWindowNFadeOutTime_Descs[4][55];
char con_gameMsgWindowNFadeOutTime_Names[4][30];
const dvar_s *con_gameMsgWindowNFadeOutTime[4];
char con_gameMsgWindowNSplitscreenScale_Descs[4][48];
char con_gameMsgWindowNSplitscreenScale_Names[4][35];
const dvar_s *con_gameMsgWindowNSplitscreenScale[4];

ConDrawInputGlob conDrawInputGlob;
bool con_ignoreMatchPrefixOnly;
int con_inputMaxMatchesShown = 24;

const float con_versionColor[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
const float con_screenPadding = 4.0f;
const float con_inputCommandMatchColor[4] = { 0.8f, 0.8f, 1.0f, 1.0f };
const float con_inputDvarMatchColor[4] = { 1.0f, 1.0f, 0.8f, 1.0f };
const float con_inputDvarValueColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
const float con_inputDvarInactiveValueColor[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
const float con_inputDvarInfoColor[4] = { 0.8f, 0.8f, 1.0f, 1.0f };
const float con_inputDvarDescriptionColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

const float con_inputBoxPad = 6.0f;
const float con_outputVertStart = 32.0f;
const float con_outputTextPad = 6.0f;
const float con_outputBarSize = 10.0f;

void __cdecl TRACK_cl_console()
{
    track_static_alloc_internal(&con, 84684, "con", 10);
}

void __cdecl Con_ToggleConsole()
{
    int localClientNum; // [esp+0h] [ebp-4h]

    Field_Clear(&g_consoleField);
    Con_CancelAutoComplete();
    g_consoleField.widthInPixels = g_console_field_width;
    g_consoleField.charHeight = g_console_char_height;
    g_consoleField.fixedSize = 1;
    con.outputVisible = 0;
    for (localClientNum = 0; localClientNum < 1; ++localClientNum)
        clientUIActives[localClientNum].keyCatchers ^= 1u;
}

void __cdecl Con_OpenConsole(int localClientNum)
{
    if (!Key_IsCatcherActive(localClientNum, 1))
        Con_ToggleConsole();
}

void __cdecl Con_OpenConsoleOutput(int localClientNum)
{
    if (Key_IsCatcherActive(localClientNum, 1))
        con.outputVisible = 1;
}

void __cdecl Con_CloseConsole(int localClientNum)
{
    if (Key_IsCatcherActive(localClientNum, 1))
        Con_ToggleConsole();
}

void __cdecl Con_GetTextCopy(char *text, int maxSize)
{
    unsigned int end; // [esp+0h] [ebp-Ch]
    int begin; // [esp+4h] [ebp-8h]
    int totalSize; // [esp+8h] [ebp-4h]

    if (con.consoleWindow.activeLineCount)
    {
        begin = con.consoleWindow.lines[con.consoleWindow.firstLineIndex].textBufPos;
        end = con.consoleWindow.textBufPos;
        totalSize = con.consoleWindow.textBufPos - begin;
        if (con.consoleWindow.textBufPos - begin < 0)
            totalSize += con.consoleWindow.textBufSize;
        if (totalSize > maxSize - 1)
        {
            begin += totalSize - (maxSize - 1);
            if (begin > con.consoleWindow.textBufSize)
                begin -= con.consoleWindow.textBufSize;
            totalSize = maxSize - 1;
        }
        if (begin >= con.consoleWindow.textBufPos)
        {
            memcpy(
                (unsigned __int8 *)text,
                (unsigned __int8 *)&con.consoleWindow.circularTextBuffer[begin],
                con.consoleWindow.textBufSize - begin);
            memcpy(
                (unsigned __int8 *)&text[con.consoleWindow.textBufSize - begin],
                (unsigned __int8 *)con.consoleWindow.circularTextBuffer,
                end);
        }
        else
        {
            memcpy(
                (unsigned __int8 *)text,
                (unsigned __int8 *)&con.consoleWindow.circularTextBuffer[begin],
                con.consoleWindow.textBufPos - begin);
        }
        text[totalSize] = 0;
    }
    else
    {
        *text = 0;
    }
}

void __cdecl Con_TimeJumped(int localClientNum, int serverTime)
{
    unsigned int gameWindowIndex; // [esp+0h] [ebp-4h]

    Con_ResetMessageWindowTimes(&con.consoleWindow, serverTime);
    for (gameWindowIndex = 0; gameWindowIndex < 4; ++gameWindowIndex)
        Con_ResetMessageWindowTimes(
            (MessageWindow *)&con.color[4630 * localClientNum - 2582 + 13 * gameWindowIndex],
            serverTime);
    Con_ResetMessageWindowTimes((MessageWindow *)&con.color[4630 * localClientNum - 1122], serverTime);
    Con_ResetMessageWindowTimes((MessageWindow *)&con.color[4630 * localClientNum - 53], serverTime);
}

void __cdecl Con_ResetMessageWindowTimes(MessageWindow *msgwnd, int serverTime)
{
    int duration; // [esp+0h] [ebp-14h]
    Message *message; // [esp+4h] [ebp-10h]
    int lineOffset; // [esp+8h] [ebp-Ch]
    MessageLine *line; // [esp+Ch] [ebp-8h]
    unsigned int lineIndex; // [esp+10h] [ebp-4h]

    for (lineOffset = 0; lineOffset < msgwnd->activeLineCount; ++lineOffset)
    {
        if (msgwnd->lineCount <= 0)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                423,
                0,
                "%s\n\t(msgwnd->lineCount) = %i",
                "(msgwnd->lineCount > 0)",
                msgwnd->lineCount);
        lineIndex = (lineOffset + msgwnd->firstLineIndex) % msgwnd->lineCount;
        if (lineIndex >= msgwnd->lineCount)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                426,
                0,
                "lineIndex doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                lineIndex,
                msgwnd->lineCount);
        line = &msgwnd->lines[lineIndex];
        if (line->messageIndex >= (unsigned int)msgwnd->lineCount)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                429,
                0,
                "line->messageIndex doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                line->messageIndex,
                msgwnd->lineCount);
        message = &msgwnd->messages[line->messageIndex];
        duration = message->endTime - message->startTime;
        message->startTime = serverTime;
        message->endTime = duration + serverTime;
    }
}

void __cdecl Con_TimeNudged(int localClientNum, int serverTimeNudge)
{
    unsigned int gameWindowIndex; // [esp+0h] [ebp-8h]
    int serverTime; // [esp+4h] [ebp-4h]

    serverTime = CL_GetLocalClientGlobals(localClientNum)->serverTime;
    Con_NudgeMessageWindowTimes(&con.consoleWindow, serverTimeNudge, serverTime);
    for (gameWindowIndex = 0; gameWindowIndex < 4; ++gameWindowIndex)
        Con_NudgeMessageWindowTimes(
            (MessageWindow *)&con.color[4630 * localClientNum - 2582 + 13 * gameWindowIndex],
            serverTimeNudge,
            serverTime);
    Con_NudgeMessageWindowTimes((MessageWindow *)&con.color[4630 * localClientNum - 1122], serverTimeNudge, serverTime);
    Con_NudgeMessageWindowTimes((MessageWindow *)&con.color[4630 * localClientNum - 53], serverTimeNudge, serverTime);
}

void __cdecl Con_NudgeMessageWindowTimes(MessageWindow *msgwnd, int serverTimeNudge, int serverTime)
{
    int duration; // [esp+0h] [ebp-18h]
    Message *message; // [esp+4h] [ebp-14h]
    int lineOffset; // [esp+8h] [ebp-10h]
    int lastMessageIndex; // [esp+Ch] [ebp-Ch]
    MessageLine *line; // [esp+10h] [ebp-8h]
    unsigned int lineIndex; // [esp+14h] [ebp-4h]

    lastMessageIndex = -1;
    for (lineOffset = 0; lineOffset < msgwnd->activeLineCount; ++lineOffset)
    {
        if (msgwnd->lineCount <= 0)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                463,
                0,
                "%s\n\t(msgwnd->lineCount) = %i",
                "(msgwnd->lineCount > 0)",
                msgwnd->lineCount);
        lineIndex = (lineOffset + msgwnd->firstLineIndex) % msgwnd->lineCount;
        if (lineIndex >= msgwnd->lineCount)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                466,
                0,
                "lineIndex doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                lineIndex,
                msgwnd->lineCount);
        line = &msgwnd->lines[lineIndex];
        if (line->messageIndex >= (unsigned int)msgwnd->lineCount)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                469,
                0,
                "line->messageIndex doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                line->messageIndex,
                msgwnd->lineCount);
        if (line->messageIndex != lastMessageIndex)
        {
            lastMessageIndex = line->messageIndex;
            message = &msgwnd->messages[line->messageIndex];
            message->startTime += serverTimeNudge;
            message->endTime += serverTimeNudge;
            if (message->startTime < 0)
            {
                message->endTime -= message->startTime;
                message->startTime = 0;
            }
            if (message->startTime > serverTime + 1000)
            {
                duration = message->endTime - message->startTime;
                message->startTime = serverTime + 1000;
                message->endTime = duration + message->startTime;
            }
        }
    }
}

void __cdecl Con_ClearNotify(int localClientNum)
{
    unsigned int gameWindowIndex; // [esp+0h] [ebp-4h]

    for (gameWindowIndex = 0; gameWindowIndex < 4; ++gameWindowIndex)
        Con_ClearMessageWindow((MessageWindow *)&con.color[4630 * localClientNum - 2582 + 13 * gameWindowIndex]);
}

void __cdecl Con_ClearMessageWindow(MessageWindow *msgwnd)
{
    if (!msgwnd)
        MyAssertHandler(".\\client\\cl_console.cpp", 512, 0, "%s", "msgwnd");
    if (!msgwnd->messages)
        MyAssertHandler(".\\client\\cl_console.cpp", 513, 0, "%s", "msgwnd->messages");
    if (!msgwnd->lines)
        MyAssertHandler(".\\client\\cl_console.cpp", 514, 0, "%s", "msgwnd->lines");
    msgwnd->textBufPos = 0;
    msgwnd->messageIndex = 0;
    msgwnd->firstLineIndex = 0;
    msgwnd->activeLineCount = 0;
}

void __cdecl Con_ClearErrors(int localClientNum)
{
    Con_ClearMessageWindow((MessageWindow *)&con.color[4630 * localClientNum - 53]);
}

void __cdecl Con_CheckResize()
{
    float x; // [esp+0h] [ebp-2Ch]
    float xa; // [esp+0h] [ebp-2Ch]
    float v2; // [esp+Ch] [ebp-20h]
    float v3; // [esp+10h] [ebp-1Ch]
    float v4; // [esp+14h] [ebp-18h]
    float v5; // [esp+18h] [ebp-14h]
    float v6; // [esp+1Ch] [ebp-10h]
    float v7; // [esp+20h] [ebp-Ch]
    float v8; // [esp+24h] [ebp-8h]
    float v9; // [esp+28h] [ebp-4h]

    v9 = ScrPlace_ApplyX(&scrPlaceFull, 4.0, 1);
    v5 = floor(v9);
    con.screenMin[0] = v5;
    v8 = ScrPlace_ApplyY(&scrPlaceFull, 4.0, 1);
    v4 = floor(v8);
    con.screenMin[1] = v4;
    x = -4.0;
    v7 = ScrPlace_ApplyX(&scrPlaceFull, x, 3);
    v3 = floor(v7);
    con.screenMax[0] = v3;
    xa = -4.0;
    v6 = ScrPlace_ApplyY(&scrPlaceFull, xa, 3);
    v2 = floor(v6);
    con.screenMax[1] = v2;
    if (cls.consoleFont)
    {
        con.fontHeight = R_TextHeight(cls.consoleFont);
        if (con.fontHeight <= 0)
            MyAssertHandler(".\\client\\cl_console.cpp", 554, 0, "%s", "con.fontHeight > 0");
        con.visibleLineCount = (int)(con.screenMax[1] - con.screenMin[1] - (double)(2 * con.fontHeight) - 6.0 * 4.0)
            / con.fontHeight;
        con.visiblePixelWidth = (int)(con.screenMax[0] - con.screenMin[0] - 10.0 - 6.0 * 3.0);
    }
    else
    {
        con.fontHeight = 0;
        con.visibleLineCount = 0;
        con.visiblePixelWidth = 0;
    }
}

cmd_function_s Con_ChatModePublic_f_VAR;
cmd_function_s Con_ChatModeTeam_f_VAR;
cmd_function_s Con_Clear_f_VAR;

void __cdecl Con_Init()
{
    int i; // [esp+0h] [ebp-4h]

    con_restricted = Dvar_RegisterBool("monkeytoy", 1, 1/*DVAR_FLAG_SAVED*/, "Restrict console access"); // KISAK: just enable console by default
    con_matchPrefixOnly = Dvar_RegisterBool(
        "con_matchPrefixOnly",
        1,
        1u,
        "Only match the prefix when listing matching Dvars");
    Field_Clear(&g_consoleField);
    g_consoleField.widthInPixels = g_console_field_width;
    g_consoleField.charHeight = g_console_char_height;
    g_consoleField.fixedSize = 1;
    for (i = 0; i < 32; ++i)
    {
        Field_Clear(&historyEditLines[i]);
        historyEditLines[i].widthInPixels = g_console_field_width;
        historyEditLines[i].charHeight = g_console_char_height;
        historyEditLines[i].fixedSize = 1;
    }
    conDrawInputGlob.matchIndex = -1;
    Cmd_AddCommandInternal("chatmodepublic", Con_ChatModePublic_f, &Con_ChatModePublic_f_VAR);
    Cmd_AddCommandInternal("chatmodeteam", Con_ChatModeTeam_f, &Con_ChatModeTeam_f_VAR);
    Cmd_AddCommandInternal("clear", Con_Clear_f, &Con_Clear_f_VAR);
}

void __cdecl SetupChatField(int localClientNum, int teamChat, int widthInPixels)
{
    PlayerKeyState *chatField; // [esp+4h] [ebp-10h]
    int width; // [esp+8h] [ebp-Ch] BYREF
    int height; // [esp+Ch] [ebp-8h] BYREF
    float aspect; // [esp+10h] [ebp-4h] BYREF

    CL_GetScreenDimensions(&width, &height, &aspect);
    playerKeys[localClientNum].chat_team = teamChat;
    chatField = &playerKeys[localClientNum];
    Field_Clear(&chatField->chatField);
    chatField->chatField.widthInPixels = widthInPixels;
    chatField->chatField.fixedSize = 0;
    if (height <= 768)
        chatField->chatField.charHeight = 16.0;
    else
        chatField->chatField.charHeight = 10.0;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client\\../client_mp/client_mp.h",
            1063,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    clientUIActives[0].keyCatchers ^= 0x20u;
}

void __cdecl Con_ChatModePublic_f()
{
    SetupChatField(0, 0, 588);
}

void __cdecl Con_ChatModeTeam_f()
{
    SetupChatField(0, 1, 543);
}

void __cdecl Con_Clear_f()
{
    Con_ClearMessageWindow(&con.consoleWindow);
    con.lineOffset = 0;
    con.displayLineOffset = 0;
}

void __cdecl Con_InitClientAssets()
{
    Con_CheckResize();
}

void __cdecl Con_InitMessageBuffer()
{
    int localClientNum; // [esp+38h] [ebp-14h]
    MessageBuffer *msgBuf; // [esp+40h] [ebp-Ch]
    unsigned int gameWindowIndex; // [esp+48h] [ebp-4h]

    for (localClientNum = 0; localClientNum < 1; ++localClientNum)
    {
        msgBuf = &con.messageBuffer[localClientNum];
        for (gameWindowIndex = 0; gameWindowIndex < 4; ++gameWindowIndex)
        {
            Con_InitMessageWindow(
                &msgBuf->gamemsgWindows[gameWindowIndex],
                msgBuf->gamemsgMessages[gameWindowIndex],
                msgBuf->gamemsgLines[gameWindowIndex],
                msgBuf->gamemsgText[gameWindowIndex],
                con_gameMsgWindowNLineCount[gameWindowIndex]->current.integer + 3,
                3,
                2048,
                (int)((con_gameMsgWindowNScrollTime[gameWindowIndex]->current.value * 1000.0) + 9.313225746154785e-10),
                (int)((con_gameMsgWindowNFadeInTime[gameWindowIndex]->current.value * 1000.0) + 9.313225746154785e-10),
                (int)((con_gameMsgWindowNFadeOutTime[gameWindowIndex]->current.value * 1000.0) + 9.313225746154785e-10));
        }
        Con_InitMessageWindow(
            &msgBuf->miniconWindow,
            msgBuf->miniconMessages,
            msgBuf->miniconLines,
            msgBuf->miniconText,
            con_miniconlines->current.integer,
            0,
            4096,
            0,
            0,
            1);
        Con_InitMessageWindow(
            &msgBuf->errorWindow,
            msgBuf->errorMessages,
            msgBuf->errorLines,
            msgBuf->errorText,
            5,
            0,
            1024,
            0,
            1,
            1);
    }
}

void __cdecl Con_InitMessageWindow(
    MessageWindow *msgwnd,
    Message *messages,
    MessageLine *lines,
    char *text,
    int lineCount,
    int padding,
    int textPoolSize,
    int scrollTime,
    int fadeIn,
    int fadeOut)
{
    if (!msgwnd)
        MyAssertHandler(".\\client\\cl_console.cpp", 569, 0, "%s", "msgwnd");
    if (!lines)
        MyAssertHandler(".\\client\\cl_console.cpp", 570, 0, "%s", "lines");
    if (lineCount < padding)
        MyAssertHandler(".\\client\\cl_console.cpp", 571, 0, "lineCount >= padding\n\t%i, %i", lineCount, padding);
    msgwnd->lines = lines;
    msgwnd->messages = messages;
    msgwnd->circularTextBuffer = text;
    msgwnd->textBufPos = 0;
    msgwnd->textBufSize = textPoolSize;
    msgwnd->firstLineIndex = 0;
    msgwnd->activeLineCount = 0;
    msgwnd->messageIndex = 0;
    msgwnd->lineCount = lineCount;
    msgwnd->padding = padding;
    msgwnd->scrollTime = scrollTime;
    if (fadeOut <= 0)
        MyAssertHandler(".\\client\\cl_console.cpp", 589, 0, "%s\n\t(fadeOut) = %i", "(fadeOut > 0)", fadeOut);
    msgwnd->fadeIn = fadeIn;
    msgwnd->fadeOut = fadeOut;
}

void __cdecl CL_ConsolePrint(int localClientNum, int channel, const char *txt, int duration, int pixelWidth, int flags)
{
    if (!txt)
        MyAssertHandler(".\\client\\cl_console.cpp", 1323, 0, "%s", "txt");
    if (cl_noprint && !cl_noprint->current.enabled && channel != 6)
    {
        if (!con.initialized)
        {
            Con_OneTimeInit();
            if (!con.initialized)
                MyAssertHandler(".\\client\\cl_console.cpp", 1334, 0, "%s", "con.initialized");
        }
        Sys_EnterCriticalSection(CRITSECT_CONSOLE);
        CL_ConsolePrint_AddLine(localClientNum, channel, txt, duration, pixelWidth, 55, flags);
        Sys_LeaveCriticalSection(CRITSECT_CONSOLE);
    }
}

void Con_OneTimeInit()
{
    DvarLimits min; // [esp+10h] [ebp-E4h]
    DvarLimits mina; // [esp+10h] [ebp-E4h]
    DvarLimits minb; // [esp+10h] [ebp-E4h]
    DvarLimits minc; // [esp+10h] [ebp-E4h]
    DvarLimits mind; // [esp+10h] [ebp-E4h]
    DvarLimits mine; // [esp+10h] [ebp-E4h]
    DvarLimits minf; // [esp+10h] [ebp-E4h]
    DvarLimits ming; // [esp+10h] [ebp-E4h]
    DvarLimits minh; // [esp+10h] [ebp-E4h]
    DvarLimits mini; // [esp+10h] [ebp-E4h]
    DvarLimits minj; // [esp+10h] [ebp-E4h]
    DvarLimits mink; // [esp+10h] [ebp-E4h]
    DvarLimits minl; // [esp+10h] [ebp-E4h]
    DvarLimits minm; // [esp+10h] [ebp-E4h]
    DvarLimits minn; // [esp+10h] [ebp-E4h]
    DvarLimits mino; // [esp+10h] [ebp-E4h]
    DvarLimits minp; // [esp+10h] [ebp-E4h]
    float v17; // [esp+40h] [ebp-B4h]
    float v18; // [esp+64h] [ebp-90h]
    unsigned int gameWindowIndex; // [esp+ECh] [ebp-8h]
    char *dvarDesc; // [esp+F0h] [ebp-4h]
    char *dvarDesca; // [esp+F0h] [ebp-4h]
    char *dvarDescb; // [esp+F0h] [ebp-4h]
    char *dvarDescc; // [esp+F0h] [ebp-4h]
    char *dvarDescd; // [esp+F0h] [ebp-4h]
    char *dvarDesce; // [esp+F0h] [ebp-4h]

    min.value.max = 1.0;
    min.value.min = 0.0;
    con_inputBoxColor = Dvar_RegisterVec4(
        "con_inputBoxColor",
        0.25,
        0.25,
        0.2,
        1.0,
        min,
        1u,
        "Color of the console input box");
    mina.value.max = 1.0;
    mina.value.min = 0.0;
    con_inputHintBoxColor = Dvar_RegisterVec4(
        "con_inputHintBoxColor",
        0.40000001,
        0.40000001,
        0.34999999,
        1.0,
        mina,
        1u,
        "Color of the console input hint box");
    minb.value.max = 1.0;
    minb.value.min = 0.0;
    con_outputBarColor = Dvar_RegisterVec4(
        "con_outputBarColor",
        1.0,
        1.0,
        0.94999999,
        0.60000002,
        minb,
        1u,
        "Color of the console output slider bar");
    minc.value.max = 1.0;
    minc.value.min = 0.0;
    con_outputSliderColor = Dvar_RegisterVec4(
        "con_outputSliderColor",
        0.15000001,
        0.15000001,
        0.1,
        0.60000002,
        minc,
        1u,
        "Color of the console slider");
    mind.value.max = 1.0;
    mind.value.min = 0.0;
    con_outputWindowColor = Dvar_RegisterVec4(
        "con_outputWindowColor",
        0.34999999,
        0.34999999,
        0.30000001,
        0.75,
        mind,
        1u,
        "Color of the console output");
    for (gameWindowIndex = 0; gameWindowIndex < 4; ++gameWindowIndex)
    {
        dvarDesc = con_gameMsgWindowNMsgTime_Descs[gameWindowIndex];
        sprintf(con_gameMsgWindowNMsgTime_Names[gameWindowIndex], "con_gameMsgWindow%dMsgTime", gameWindowIndex);
        if (strlen(con_gameMsgWindowNMsgTime_Names[gameWindowIndex]) != 25)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                1083,
                0,
                "%s",
                "strlen( dvarName ) == sizeof( con_gameMsgWindowNMsgTime_Names[gameWindowIndex] ) - 1");
        sprintf(dvarDesc, "On screen time for game messages in seconds in game message window %d", gameWindowIndex);
        if (strlen(con_gameMsgWindowNMsgTime_Descs[gameWindowIndex]) != 68)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                1083,
                0,
                "%s",
                "strlen( dvarDesc ) == sizeof( con_gameMsgWindowNMsgTime_Descs[gameWindowIndex] ) - 1");
        mine.value.max = FLT_MAX;
        mine.value.min = 0.0;
        con_gameMsgWindowNMsgTime[gameWindowIndex] = Dvar_RegisterFloat(
            con_gameMsgWindowNMsgTime_Names[gameWindowIndex],
            defaultGameMessageTimes[gameWindowIndex],
            mine,
            1u,
            dvarDesc);
        dvarDesca = con_gameMsgWindowNLineCount_Descs[gameWindowIndex];
        sprintf(con_gameMsgWindowNLineCount_Names[gameWindowIndex], "con_gameMsgWindow%dLineCount", gameWindowIndex);
        if (strlen(con_gameMsgWindowNLineCount_Names[gameWindowIndex]) != 27)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                1086,
                0,
                "%s",
                "strlen( dvarName ) == sizeof( con_gameMsgWindowNLineCount_Names[gameWindowIndex] ) - 1");
        sprintf(dvarDesca, "Maximum number of lines of text visible at once in game message window %d", gameWindowIndex);
        if (strlen(con_gameMsgWindowNLineCount_Descs[gameWindowIndex]) != 72)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                1086,
                0,
                "%s",
                "strlen( dvarDesc ) == sizeof( con_gameMsgWindowNLineCount_Descs[gameWindowIndex] ) - 1");
        con_gameMsgWindowNLineCount[gameWindowIndex] = Dvar_RegisterInt(
            con_gameMsgWindowNLineCount_Names[gameWindowIndex],
            defaultGameMessageWindowLineCounts[gameWindowIndex],
            (DvarLimits)0x900000001LL,
            1u,
            dvarDesca);
        dvarDescb = con_gameMsgWindowNScrollTime_Descs[gameWindowIndex];
        sprintf(con_gameMsgWindowNScrollTime_Names[gameWindowIndex], "con_gameMsgWindow%dScrollTime", gameWindowIndex);
        if (strlen(con_gameMsgWindowNScrollTime_Names[gameWindowIndex]) != 28)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                1090,
                0,
                "%s",
                "strlen( dvarName ) == sizeof( con_gameMsgWindowNScrollTime_Names[gameWindowIndex] ) - 1");
        sprintf(
            dvarDescb,
            "Time to scroll messages when the oldest message is removed in game message window %d",
            gameWindowIndex);
        if (strlen(con_gameMsgWindowNScrollTime_Descs[gameWindowIndex]) != 83)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                1090,
                0,
                "%s",
                "strlen( dvarDesc ) == sizeof( con_gameMsgWindowNScrollTime_Descs[gameWindowIndex] ) - 1");
        minf.value.max = FLT_MAX;
        minf.value.min = 0.0;
        con_gameMsgWindowNScrollTime[gameWindowIndex] = Dvar_RegisterFloat(
            con_gameMsgWindowNScrollTime_Names[gameWindowIndex],
            0.25,
            minf,
            1u,
            dvarDescb);
        dvarDescc = con_gameMsgWindowNFadeInTime_Descs[gameWindowIndex];
        sprintf(con_gameMsgWindowNFadeInTime_Names[gameWindowIndex], "con_gameMsgWindow%dFadeInTime", gameWindowIndex);
        if (strlen(con_gameMsgWindowNFadeInTime_Names[gameWindowIndex]) != 28)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                1093,
                0,
                "%s",
                "strlen( dvarName ) == sizeof( con_gameMsgWindowNFadeInTime_Names[gameWindowIndex] ) - 1");
        sprintf(dvarDescc, "Time to fade in new messages in game message window %d", gameWindowIndex);
        if (strlen(con_gameMsgWindowNFadeInTime_Descs[gameWindowIndex]) != 53)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                1093,
                0,
                "%s",
                "strlen( dvarDesc ) == sizeof( con_gameMsgWindowNFadeInTime_Descs[gameWindowIndex] ) - 1");
        if (gameWindowIndex == 2)
            v18 = 0.75;
        else
            v18 = 0.25;
        ming.value.max = FLT_MAX;
        ming.value.min = 0.0;
        con_gameMsgWindowNFadeInTime[gameWindowIndex] = Dvar_RegisterFloat(
            con_gameMsgWindowNFadeInTime_Names[gameWindowIndex],
            v18,
            ming,
            1u,
            dvarDescc);
        dvarDescd = con_gameMsgWindowNFadeOutTime_Descs[gameWindowIndex];
        sprintf(con_gameMsgWindowNFadeOutTime_Names[gameWindowIndex], "con_gameMsgWindow%dFadeOutTime", gameWindowIndex);
        if (strlen(con_gameMsgWindowNFadeOutTime_Names[gameWindowIndex]) != 29)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                1096,
                0,
                "%s",
                "strlen( dvarName ) == sizeof( con_gameMsgWindowNFadeOutTime_Names[gameWindowIndex] ) - 1");
        sprintf(dvarDescd, "Time to fade out old messages in game message window %d", gameWindowIndex);
        if (strlen(con_gameMsgWindowNFadeOutTime_Descs[gameWindowIndex]) != 54)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                1096,
                0,
                "%s",
                "strlen( dvarDesc ) == sizeof( con_gameMsgWindowNFadeOutTime_Descs[gameWindowIndex] ) - 1");
        if (gameWindowIndex == 1)
            v17 = 0.0099999998;
        else
            v17 = 0.5;
        minh.value.max = FLT_MAX;
        minh.value.min = 0.0099999998;
        con_gameMsgWindowNFadeOutTime[gameWindowIndex] = Dvar_RegisterFloat(
            con_gameMsgWindowNFadeOutTime_Names[gameWindowIndex],
            v17,
            minh,
            1u,
            dvarDescd);
        dvarDesce = con_gameMsgWindowNSplitscreenScale_Descs[gameWindowIndex];
        sprintf(
            con_gameMsgWindowNSplitscreenScale_Names[gameWindowIndex],
            "con_gameMsgWindow%dSplitscreenScale",
            gameWindowIndex);
        if (strlen(con_gameMsgWindowNSplitscreenScale_Names[gameWindowIndex]) != 34)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                1099,
                0,
                "%s",
                "strlen( dvarName ) == sizeof( con_gameMsgWindowNSplitscreenScale_Names[gameWindowIndex] ) - 1");
        sprintf(dvarDesce, "Scaling of game message window %d in splitscreen", gameWindowIndex);
        if (strlen(con_gameMsgWindowNSplitscreenScale_Descs[gameWindowIndex]) != 47)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                1099,
                0,
                "%s",
                "strlen( dvarDesc ) == sizeof( con_gameMsgWindowNSplitscreenScale_Descs[gameWindowIndex] ) - 1");
        mini.value.max = FLT_MAX;
        mini.value.min = 0.0;
        con_gameMsgWindowNSplitscreenScale[gameWindowIndex] = Dvar_RegisterFloat(
            con_gameMsgWindowNSplitscreenScale_Names[gameWindowIndex],
            1.5,
            mini,
            1u,
            dvarDesce);
    }
    minj.value.max = FLT_MAX;
    minj.value.min = 0.0;
    con_errormessagetime = Dvar_RegisterFloat(
        "con_errormessagetime",
        8.0,
        minj,
        1u,
        "Onscreen time for error messages in seconds");
    mink.value.max = FLT_MAX;
    mink.value.min = 0.0;
    con_minicontime = Dvar_RegisterFloat(
        "con_minicontime",
        4.0,
        mink,
        1u,
        "Onscreen time for minicon messages in seconds");
    con_miniconlines = Dvar_RegisterInt(
        "con_miniconlines",
        5,
        (DvarLimits)0x6400000000LL,
        1u,
        "Number of lines in the minicon message window");
    con_typewriterPrintSpeed = Dvar_RegisterInt(
        "con_typewriterPrintSpeed",
        50,
        (DvarLimits)0x7FFFFFFF00000000LL,
        1u,
        "Time (in milliseconds) to print each letter in the line.");
    con_typewriterDecayStartTime = Dvar_RegisterInt(
        "con_typewriterDecayStartTime",
        6000,
        (DvarLimits)0x7FFFFFFF00000000LL,
        1u,
        "Time (in milliseconds) to spend between the build and disolve phases.");
    con_typewriterDecayDuration = Dvar_RegisterInt(
        "con_typewriterDecayDuration",
        700,
        (DvarLimits)0x7FFFFFFF00000000LL,
        1u,
        "Time (in milliseconds) to spend disolving the line away.");
    minl.value.max = 1.0;
    minl.value.min = 0.0;
    con_typewriterColorBase = Dvar_RegisterVec3(
        "con_typewriterColorBase",
        1.0,
        1.0,
        1.0,
        minl,
        0x1000u,
        "Base color of typewritten objective text.");
    minm.value.max = 1.0;
    minm.value.min = 0.0;
    con_typewriterColorGlowUpdated = Dvar_RegisterVec4(
        "con_typewriterColorGlowUpdated",
        0.0,
        0.60000002,
        0.18000001,
        1.0,
        minm,
        1u,
        "Color of typewritten objective text.");
    minn.value.max = 1.0;
    minn.value.min = 0.0;
    con_typewriterColorGlowCompleted = Dvar_RegisterVec4(
        "con_typewriterColorGlowCompleted",
        0.0,
        0.30000001,
        0.80000001,
        1.0,
        minn,
        1u,
        "Color of typewritten objective text.");
    mino.value.max = 1.0;
    mino.value.min = 0.0;
    con_typewriterColorGlowFailed = Dvar_RegisterVec4(
        "con_typewriterColorGlowFailed",
        0.80000001,
        0.0,
        0.0,
        1.0,
        mino,
        1u,
        "Color of typewritten objective text.");
    minp.value.max = 1.0;
    minp.value.min = 0.0;
    con_typewriterColorGlowCheckpoint = Dvar_RegisterVec4(
        "con_typewriterColorGlowCheckpoint",
        0.60000002,
        0.5,
        0.60000002,
        1.0,
        minp,
        1u,
        "Color of typewritten objective text.");
    Con_InitMessageWindow(
        &con.consoleWindow,
        con.consoleMessages,
        con.consoleLines,
        con.consoleText,
        1024,
        0,
        0x8000,
        0,
        1,
        1);
    Con_InitMessageBuffer();
    con.color[0] = 1.0;
    con.color[1] = 1.0;
    con.color[2] = 1.0;
    con.color[3] = 1.0;
    Con_CheckResize();
    con.initialized = 1;
}

char __cdecl CL_ConsolePrint_AddLine(
    int localClientNum,
    int channel,
    const char *txt,
    int duration,
    int pixelWidth,
    char color,
    int flags)
{
    const char *v8; // eax
    char *v9; // edx
    const char *v10; // eax
    char *v13; // [esp+18h] [ebp-44h]
    const char *v14; // [esp+1Ch] [ebp-40h]
    const char *v15; // [esp+28h] [ebp-34h]
    const char *v16; // [esp+34h] [ebp-28h]
    int c; // [esp+38h] [ebp-24h]
    int atStartOfBrokenLine; // [esp+3Ch] [ebp-20h]
    Font_s *font; // [esp+40h] [ebp-1Ch]
    float xScale; // [esp+44h] [ebp-18h]
    const char *wrapPosition; // [esp+50h] [ebp-Ch]
    float fontScale; // [esp+54h] [ebp-8h]
    const char *text; // [esp+58h] [ebp-4h] BYREF

    if (!txt)
        MyAssertHandler(".\\client\\cl_console.cpp", 1163, 0, "%s", "txt");
    if (color < 48 || color > 57)
        MyAssertHandler(
            ".\\client\\cl_console.cpp",
            1164,
            0,
            "color not in [COLOR_FIRST, COLOR_LAST]\n\t%i not in [%i, %i]",
            color,
            48,
            57);
    if (callDepth)
        return color;
    callDepth = 1;
    Con_UpdateNotifyMessage(localClientNum, channel, duration, flags);
    if (channel != con.prevChannel && con.lineOffset)
        Con_Linefeed(localClientNum, con.prevChannel, flags);
    if (channel == 2 || channel == 3 || channel == 4)
    {
        fontScale = (float)12.0 / 48.0;
        font = UI_GetFontHandle(&scrPlaceView[localClientNum], channel != 3 ? 0 : 4, fontScale);
        xScale = R_NormalizedTextScale(font, fontScale);
    }
    else
    {
        font = cls.consoleFont;
        xScale = 1.0;
    }
    if (!pixelWidth)
        pixelWidth = con.visiblePixelWidth;
    v16 = R_TextLineWrapPosition(txt, 512 - con.lineOffset, pixelWidth, font, xScale);
    if (!v16)
        MyAssertHandler(".\\client\\cl_console.cpp", 1144, 0, "%s", "lineWrapPos");
    wrapPosition = *v16 != 0 ? v16 : 0;
    if (txt == wrapPosition && con.lineOffset)
    {
        Con_Linefeed(localClientNum, channel, flags);
        v15 = R_TextLineWrapPosition(txt, 512 - con.lineOffset, pixelWidth, font, xScale);
        if (!v15)
            MyAssertHandler(".\\client\\cl_console.cpp", 1144, 0, "%s", "lineWrapPos");
        wrapPosition = *v15 != 0 ? v15 : 0;
    }
    text = txt;
    atStartOfBrokenLine = 0;
    while (*text)
    {
        c = SEH_ReadCharFromString(&text, 0);
        if (c == 10)
        {
            if (wrapPosition)
            {
                if (font && wrapPosition != text && *text)
                {
                    v8 = va("font is %s, wrapPosition is %s, text is %s, txt is %s", "valid", wrapPosition, text, txt);
                    MyAssertHandler(
                        ".\\client\\cl_console.cpp",
                        1218,
                        0,
                        "%s\n\t%s",
                        "!font || wrapPosition == text || !text[0]",
                        v8);
                }
            }
            else
            {
                wrapPosition = text;
            }
        }
        else
        {
            if (c != 94)
                goto LABEL_58;
            if (!text || *text == 94 || *text < 48 || *text > 57)
            {
                if (*text != 1 && *text != 2)
                {
                LABEL_58:
                    if (c != 32 || !atStartOfBrokenLine)
                    {
                        if (c > 255)
                        {
                            if (con.lineOffset >= 0x200)
                                MyAssertHandler(
                                    ".\\client\\cl_console.cpp",
                                    1259,
                                    0,
                                    "con.lineOffset doesn't index sizeof( con.textTempLine )\n\t%i not in [0, %i)",
                                    con.lineOffset,
                                    512);
                            con.textTempLine[con.lineOffset++] = BYTE1(c);
                            c = (unsigned __int8)c;
                            if (con.lineOffset >= 0x201)
                                MyAssertHandler(
                                    ".\\client\\cl_console.cpp",
                                    1263,
                                    0,
                                    "con.lineOffset doesn't index sizeof( con.textTempLine ) + 1\n\t%i not in [0, %i)",
                                    con.lineOffset,
                                    513);
                        }
                        if (con.lineOffset >= 0x200)
                            MyAssertHandler(
                                ".\\client\\cl_console.cpp",
                                1265,
                                0,
                                "con.lineOffset doesn't index sizeof( con.textTempLine )\n\t%i not in [0, %i)",
                                con.lineOffset,
                                512);
                        con.textTempLine[con.lineOffset++] = c;
                        if (con.lineOffset >= 0x201)
                            MyAssertHandler(
                                ".\\client\\cl_console.cpp",
                                1268,
                                0,
                                "con.lineOffset doesn't index sizeof( con.textTempLine ) + 1\n\t%i not in [0, %i)",
                                con.lineOffset,
                                513);
                        atStartOfBrokenLine = 0;
                    }
                    goto LABEL_70;
                }
                if (con.lineOffset >= 0x1F9)
                    MyAssertHandler(
                        ".\\client\\cl_console.cpp",
                        1236,
                        0,
                        "con.lineOffset doesn't index sizeof( con.textTempLine ) - CONTXTCMD_LEN_HUDICON\n\t%i not in [0, %i)",
                        con.lineOffset,
                        505);
                if (!IsValidMaterialHandle(*(Material *const *)(text + 3)))
                    MyAssertHandler(
                        ".\\client\\cl_console.cpp",
                        1238,
                        0,
                        "%s",
                        "IsValidMaterialHandle( *reinterpret_cast< const MaterialHandle * >( &text[CONTXTCMD_ARG_HUDICON_MATERIAL] ) )");
                con.textTempLine[con.lineOffset++] = 94;
                v9 = &con.textTempLine[con.lineOffset];
                v10 = text;
                *(unsigned int *)v9 = *(unsigned int *)text;
                *((_WORD *)v9 + 2) = *((_WORD *)v10 + 2);
                v9[6] = v10[6];
                con.lineOffset += 7;
                text += 7;
                if (con.lineOffset >= 0x201)
                    MyAssertHandler(
                        ".\\client\\cl_console.cpp",
                        1244,
                        0,
                        "con.lineOffset doesn't index sizeof( con.textTempLine ) + 1\n\t%i not in [0, %i)",
                        con.lineOffset,
                        513);
                atStartOfBrokenLine = 0;
            }
            else
            {
                if (con.lineOffset >= 0x1FF)
                    MyAssertHandler(
                        ".\\client\\cl_console.cpp",
                        1225,
                        0,
                        "con.lineOffset doesn't index sizeof( con.textTempLine ) - 1\n\t%i not in [0, %i)",
                        con.lineOffset,
                        511);
                color = *text;
                con.textTempLine[con.lineOffset++] = 94;
                con.textTempLine[con.lineOffset++] = color;
                if (con.lineOffset >= 0x201)
                    MyAssertHandler(
                        ".\\client\\cl_console.cpp",
                        1229,
                        0,
                        "con.lineOffset doesn't index sizeof( con.textTempLine ) + 1\n\t%i not in [0, %i)",
                        con.lineOffset,
                        513);
                ++text;
                atStartOfBrokenLine = 0;
            }
        }
    LABEL_70:
        if (text == wrapPosition)
        {
            Con_Linefeed(localClientNum, channel, flags);
            if (c != 10)
            {
                atStartOfBrokenLine = 1;
                if (color != 55)
                {
                    if (color < 48 || color > 57)
                        MyAssertHandler(
                            ".\\client\\cl_console.cpp",
                            1282,
                            0,
                            "color not in [COLOR_FIRST, COLOR_LAST]\n\t%i not in [%i, %i]",
                            color,
                            48,
                            57);
                    if (con.lineOffset >= 0x1FF)
                        MyAssertHandler(
                            ".\\client\\cl_console.cpp",
                            1283,
                            0,
                            "con.lineOffset doesn't index sizeof( con.textTempLine ) - 1\n\t%i not in [0, %i)",
                            con.lineOffset,
                            511);
                    con.textTempLine[con.lineOffset] = 94;
                    //*(_BYTE *)(con.lineOffset + 11724761) = color;
                    con.consoleText[con.lineOffset + 1] = color;
                    con.lineOffset += 2;
                    if (con.lineOffset >= 0x201)
                        MyAssertHandler(
                            ".\\client\\cl_console.cpp",
                            1287,
                            0,
                            "con.lineOffset doesn't index sizeof( con.textTempLine ) + 1\n\t%i not in [0, %i)",
                            con.lineOffset,
                            513);
                }
            }
            v13 = (char *)text;
            if (atStartOfBrokenLine)
            {
                while (*v13 == 32)
                    ++v13;
            }
            v14 = R_TextLineWrapPosition(v13, 512 - con.lineOffset, pixelWidth, font, xScale);
            if (!v14)
                MyAssertHandler(".\\client\\cl_console.cpp", 1144, 0, "%s", "lineWrapPos");
            wrapPosition = *v14 != 0 ? v14 : 0;
        }
    }
    if (con.lineOffset)
    {
        if (channel == 2 || channel == 3 || channel == 4)
            Con_Linefeed(localClientNum, channel, flags);
        else
            Con_UpdateNotifyLine(localClientNum, channel, 0, flags);
    }
    con.prevChannel = channel;
    --callDepth;
    return color;
}

void __cdecl Con_UpdateNotifyMessage(int localClientNum, unsigned int channel, int duration, int flags)
{
    print_msg_dest_t dest; // [esp+0h] [ebp-4h]

    if (!Con_IsChannelOpen(channel))
        MyAssertHandler(".\\client\\cl_console.cpp", 976, 0, "%s", "Con_IsChannelOpen(channel)");
    Con_UpdateNotifyMessageWindow(localClientNum, channel, duration, flags, CON_DEST_MINICON);
    for (dest = CON_DEST_GAME_FIRST; (unsigned int)dest <= CON_DEST_GAME4; ++dest)
        Con_UpdateNotifyMessageWindow(localClientNum, channel, duration, flags, dest);
    if (!com_developer)
        MyAssertHandler(".\\client\\cl_console.cpp", 982, 0, "%s", "com_developer");
    if (com_developer->current.integer)
        Con_UpdateNotifyMessageWindow(localClientNum, channel, duration, flags, CON_DEST_ERROR);
}

void __cdecl Con_UpdateNotifyMessageWindow(
    int localClientNum,
    unsigned int channel,
    int duration,
    int flags,
    print_msg_dest_t dest)
{
    MessageWindow *DestWindow; // eax

    if (Con_IsChannelVisible(dest, channel, flags))
    {
        if (!duration)
            duration = Con_GetDefaultMsgDuration(dest);
        if (duration < 0)
            duration = 0;
        DestWindow = Con_GetDestWindow(localClientNum, dest);
        Con_UpdateMessage(localClientNum, DestWindow, duration);
    }
}

int __cdecl Con_GetDefaultMsgDuration(print_msg_dest_t dest)
{
    float v2; // [esp+8h] [ebp-38h]
    float v3; // [esp+1Ch] [ebp-24h]
    float v4; // [esp+30h] [ebp-10h]

    if (dest == CON_DEST_MINICON)
    {
        v4 = con_minicontime->current.value * 1000.0;
        return (int)(v4 + 9.313225746154785e-10);
    }
    else if (dest == CON_DEST_ERROR)
    {
        v3 = con_errormessagetime->current.value * 1000.0;
        return (int)(v3 + 9.313225746154785e-10);
    }
    else
    {
        if (dest < CON_DEST_GAME_FIRST || dest > CON_DEST_GAME4)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                668,
                0,
                "%s",
                "dest >= CON_DEST_GAME_FIRST && dest <= CON_DEST_GAME_LAST");
        v2 = con_gameMsgWindowNLineCount[dest - 3]->current.value * 1000.0;
        return (int)(v2 + 9.313225746154785e-10);
    }
}

void __cdecl Con_UpdateMessage(int localClientNum, MessageWindow *msgwnd, int duration)
{
    Message *message; // [esp+0h] [ebp-4h]

    if (!msgwnd)
        MyAssertHandler(".\\client\\cl_console.cpp", 678, 0, "%s", "msgwnd");
    if (msgwnd->messageIndex >= (unsigned int)msgwnd->lineCount)
        MyAssertHandler(
            ".\\client\\cl_console.cpp",
            679,
            0,
            "msgwnd->messageIndex doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
            msgwnd->messageIndex,
            msgwnd->lineCount);
    if (!msgwnd->lineCount)
        MyAssertHandler(".\\client\\cl_console.cpp", 682, 0, "%s", "msgwnd->lineCount != 0");
    msgwnd->messageIndex = (msgwnd->messageIndex + 1) % msgwnd->lineCount;
    message = &msgwnd->messages[msgwnd->messageIndex];
    if (localClientNum >= 1)
        message->startTime = 0;
    else
        message->startTime = CL_GetLocalClientGlobals(localClientNum)->serverTime;
    message->endTime = duration + message->startTime;
}

MessageWindow *__cdecl Con_GetDestWindow(int localClientNum, print_msg_dest_t dest)
{
    switch (dest)
    {
    case CON_DEST_CONSOLE:
        return &con.consoleWindow;
    case CON_DEST_MINICON:
        return (MessageWindow *)&con.color[4630 * localClientNum - 1122];
    case CON_DEST_ERROR:
        return (MessageWindow *)&con.color[4630 * localClientNum - 53];
    }
    if (dest < CON_DEST_GAME_FIRST || dest > CON_DEST_GAME4)
        MyAssertHandler(
            ".\\client\\cl_console.cpp",
            951,
            0,
            "%s",
            "dest >= CON_DEST_GAME_FIRST && dest <= CON_DEST_GAME_LAST");
    return (MessageWindow *)&con.color[4630 * localClientNum - 2621 + 13 * dest];
}

void __cdecl Con_UpdateNotifyLine(int localClientNum, unsigned int channel, bool lineFeed, int flags)
{
    print_msg_dest_t dest; // [esp+0h] [ebp-4h]

    if (!Con_IsChannelOpen(channel))
        MyAssertHandler(".\\client\\cl_console.cpp", 1001, 0, "%s", "Con_IsChannelOpen(channel)");
    Con_UpdateNotifyLineWindow(localClientNum, channel, lineFeed, flags, CON_DEST_CONSOLE);
    Con_UpdateNotifyLineWindow(localClientNum, channel, lineFeed, flags, CON_DEST_MINICON);
    for (dest = CON_DEST_GAME_FIRST; (unsigned int)dest <= CON_DEST_GAME4; ++dest)
        Con_UpdateNotifyLineWindow(localClientNum, channel, lineFeed, flags, dest);
    if (!com_developer)
        MyAssertHandler(".\\client\\cl_console.cpp", 1008, 0, "%s", "com_developer");
    if (com_developer->current.integer)
        Con_UpdateNotifyLineWindow(localClientNum, channel, lineFeed, flags, CON_DEST_ERROR);
}

void __cdecl Con_UpdateNotifyLineWindow(
    int localClientNum,
    unsigned int channel,
    bool lineFeed,
    int flags,
    print_msg_dest_t dest)
{
    MessageWindow *DestWindow; // eax

    if (Con_IsChannelVisible(dest, channel, flags))
    {
        DestWindow = Con_GetDestWindow(localClientNum, dest);
        Con_UpdateMessageWindowLine(localClientNum, DestWindow, lineFeed, flags);
    }
}

void __cdecl Con_UpdateMessageWindowLine(int localClientNum, MessageWindow *msgwnd, int linefeed, int flags)
{
    int newPadLineOffset; // [esp+0h] [ebp-14h]
    Message *message; // [esp+4h] [ebp-10h]
    unsigned int imod; // [esp+8h] [ebp-Ch]
    MessageLine *line; // [esp+Ch] [ebp-8h]
    MessageLine *linea; // [esp+Ch] [ebp-8h]
    int serverTime; // [esp+10h] [ebp-4h]

    if (!msgwnd)
        MyAssertHandler(".\\client\\cl_console.cpp", 893, 0, "%s", "msgwnd");
    if (msgwnd->firstLineIndex >= (unsigned int)msgwnd->lineCount)
        MyAssertHandler(
            ".\\client\\cl_console.cpp",
            894,
            0,
            "msgwnd->firstLineIndex doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
            msgwnd->firstLineIndex,
            msgwnd->lineCount);
    if (msgwnd->messageIndex >= (unsigned int)msgwnd->lineCount)
        MyAssertHandler(
            ".\\client\\cl_console.cpp",
            895,
            0,
            "msgwnd->messageIndex doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
            msgwnd->messageIndex,
            msgwnd->lineCount);
    if (localClientNum >= 1)
        serverTime = 0;
    else
        serverTime = CL_GetLocalClientGlobals(localClientNum)->serverTime;
    line = &msgwnd->lines[(msgwnd->activeLineCount + msgwnd->firstLineIndex) % msgwnd->lineCount];
    line->messageIndex = msgwnd->messageIndex;
    line->typingStartTime = 0;
    line->lastTypingSoundTime = 0;
    line->flags = flags;
    line->typingStartTime = GetNextValidPrintTimeForLine(localClientNum, msgwnd, flags);
    if (line->typingStartTime)
        msgwnd->messages[line->messageIndex].endTime = line->typingStartTime + PrintTimeTotal(msgwnd, line);
    Con_CopyCurrentConsoleLineText(msgwnd, line);
    if (linefeed)
    {
        if (msgwnd->activeLineCount == msgwnd->lineCount)
            Con_FreeFirstMessageWindowLine(msgwnd);
        newPadLineOffset = ++msgwnd->activeLineCount - (msgwnd->lineCount - msgwnd->padding);
        if (newPadLineOffset > 0)
        {
            imod = (msgwnd->firstLineIndex + newPadLineOffset - 1) % msgwnd->lineCount;
            if (imod >= msgwnd->lineCount)
                MyAssertHandler(
                    ".\\client\\cl_console.cpp",
                    925,
                    0,
                    "imod doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                    imod,
                    msgwnd->lineCount);
            linea = &msgwnd->lines[imod];
            if (linea->messageIndex >= (unsigned int)msgwnd->lineCount)
                MyAssertHandler(
                    ".\\client\\cl_console.cpp",
                    927,
                    0,
                    "line->messageIndex doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                    linea->messageIndex,
                    msgwnd->lineCount);
            message = &msgwnd->messages[linea->messageIndex];
            if (message->endTime - msgwnd->fadeOut > serverTime)
            {
                if (message->endTime < message->startTime)
                    MyAssertHandler(
                        ".\\client\\cl_console.cpp",
                        932,
                        0,
                        "message->endTime >= message->startTime\n\t%i, %i",
                        message->endTime,
                        message->startTime);
                message->endTime = msgwnd->fadeOut + serverTime;
            }
        }
    }
}

void __cdecl Con_FreeFirstMessageWindowLine(MessageWindow *msgwnd)
{
    int activeLineCount; // [esp+0h] [ebp-4h]

    if (msgwnd->activeLineCount <= 0)
        MyAssertHandler(".\\client\\cl_console.cpp", 696, 0, "%s", "msgwnd->activeLineCount > 0");
    --msgwnd->activeLineCount;
    if (++msgwnd->firstLineIndex == msgwnd->lineCount)
        msgwnd->firstLineIndex = 0;
    if (msgwnd == &con.consoleWindow && --con.displayLineOffset < con.visibleLineCount)
    {
        if (con.consoleWindow.activeLineCount < con.visibleLineCount)
            activeLineCount = con.consoleWindow.activeLineCount;
        else
            activeLineCount = con.visibleLineCount;
        con.displayLineOffset = activeLineCount;
    }
}

void __cdecl Con_CopyCurrentConsoleLineText(MessageWindow *msgwnd, MessageLine *msgLine)
{
    unsigned int poolRemaining; // [esp+0h] [ebp-4h]

    if (!msgLine)
        MyAssertHandler(".\\client\\cl_console.cpp", 736, 0, "%s", "msgLine");
    while (Con_NeedToFreeMessageWindowLine(msgwnd, con.lineOffset + 1))
        Con_FreeFirstMessageWindowLine(msgwnd);
    poolRemaining = msgwnd->textBufSize - msgwnd->textBufPos;
    if (con.lineOffset > poolRemaining)
    {
        memcpy(
            (unsigned __int8 *)&msgwnd->circularTextBuffer[msgwnd->textBufPos],
            (unsigned __int8 *)con.textTempLine,
            poolRemaining);
        memcpy(
            (unsigned __int8 *)msgwnd->circularTextBuffer,
            (unsigned __int8 *)&con.textTempLine[poolRemaining],
            con.lineOffset - poolRemaining);
    }
    else
    {
        memcpy(
            (unsigned __int8 *)&msgwnd->circularTextBuffer[msgwnd->textBufPos],
            (unsigned __int8 *)con.textTempLine,
            con.lineOffset);
    }
    msgLine->textBufPos = msgwnd->textBufPos;
    msgLine->textBufSize = con.lineOffset;
    if ((msgwnd->textBufSize & (msgwnd->textBufSize - 1)) != 0)
        MyAssertHandler(".\\client\\cl_console.cpp", 755, 0, "%s", "IsPowerOf2( msgwnd->textBufSize )");
    msgwnd->textBufPos = (msgwnd->textBufSize - 1) & (con.lineOffset + msgwnd->textBufPos);
    msgwnd->circularTextBuffer[msgwnd->textBufPos] = 10;
    msgwnd->textBufPos = (msgwnd->textBufSize - 1) & (msgwnd->textBufPos + 1);
}

bool __cdecl Con_NeedToFreeMessageWindowLine(MessageWindow *msgwnd, int charCount)
{
    int pastLastChar; // [esp+8h] [ebp-8h]
    MessageLine *line; // [esp+Ch] [ebp-4h]

    if (!msgwnd->activeLineCount)
        return 0;
    if (msgwnd->lineCount <= 0)
        MyAssertHandler(".\\client\\cl_console.cpp", 720, 0, "%s", "msgwnd->lineCount > 0");
    line = &msgwnd->lines[msgwnd->firstLineIndex];
    if ((msgwnd->textBufSize & (msgwnd->textBufSize - 1)) != 0)
        MyAssertHandler(".\\client\\cl_console.cpp", 723, 0, "%s", "IsPowerOf2( msgwnd->textBufSize )");
    pastLastChar = (msgwnd->textBufSize - 1) & (charCount + msgwnd->textBufPos);
    if (pastLastChar < msgwnd->textBufPos)
        return line->textBufPos >= msgwnd->textBufPos || line->textBufPos < pastLastChar;
    else
        return line->textBufPos >= msgwnd->textBufPos && line->textBufPos < pastLastChar;
}

int __cdecl PrintTimeTotal(MessageWindow *msgwnd, MessageLine *line)
{
    int time; // [esp+4h] [ebp-4h]

    if ((line->flags & 1) == 0)
        return 0;
    time = con_typewriterPrintSpeed->current.integer * PrintableCharsCount(msgwnd, line);
    if (time < con_typewriterDecayStartTime->current.integer)
        time = con_typewriterDecayStartTime->current.integer;
    return con_typewriterDecayDuration->current.integer + time;
}

int __cdecl PrintableCharsCount(const MessageWindow *msgwnd, MessageLine *line)
{
    bool v3; // [esp+0h] [ebp-18h]
    int usedCharCnt; // [esp+4h] [ebp-14h] BYREF
    char c[4]; // [esp+8h] [ebp-10h] BYREF
    int letter; // [esp+Ch] [ebp-Ch]
    int printedCnt; // [esp+10h] [ebp-8h]
    int idx; // [esp+14h] [ebp-4h]

    if (line->textBufSize)
        v3 = (msgwnd->textBufSize & (msgwnd->textBufSize - 1)) == 0;
    else
        v3 = 1;
    if (!v3)
        MyAssertHandler(
            ".\\client\\cl_console.cpp",
            799,
            0,
            "%s",
            "line->textBufSize ? IsPowerOf2( msgwnd->textBufSize ) : true");
    printedCnt = 0;
    idx = 0;
    while (idx < line->textBufSize)
    {
        c[0] = msgwnd->circularTextBuffer[(msgwnd->textBufSize - 1) & (idx + line->textBufPos)];
        c[1] = msgwnd->circularTextBuffer[(msgwnd->textBufSize - 1) & (line->textBufPos + idx + 1)];
        letter = SEH_DecodeLetter(c[0], c[1], &usedCharCnt, 0);
        idx += usedCharCnt;
        ++printedCnt;
        if (letter == 94)
        {
            c[0] = msgwnd->circularTextBuffer[(msgwnd->textBufSize - 1) & (idx + line->textBufPos)];
            if (c)
            {
                if (c[0] != 94 && c[0] >= 48 && c[0] <= 57)
                    ++idx;
            }
        }
    }
    return printedCnt;
}

int __cdecl GetNextValidPrintTimeForLine(int localClientNum, MessageWindow *msgwnd, char flags)
{
    int nextPrintTime; // [esp+0h] [ebp-10h]
    MessageLine *line; // [esp+4h] [ebp-Ch]
    int lineIdx; // [esp+8h] [ebp-8h]
    int serverTime; // [esp+Ch] [ebp-4h]

    if ((flags & 1) == 0)
        return 0;
    serverTime = CL_GetLocalClientGlobals(localClientNum)->serverTime;
    lineIdx = LatestActiveTypewrittenLineIdx(msgwnd);
    if (lineIdx == -1)
        return serverTime + 250;
    line = &msgwnd->lines[lineIdx];
    nextPrintTime = line->typingStartTime + PrintTimeWriteOut(msgwnd, line) + 150;
    if (nextPrintTime - serverTime >= 250)
        return nextPrintTime;
    else
        return serverTime + 250;
}

int __cdecl LatestActiveTypewrittenLineIdx(MessageWindow *msgwnd)
{
    int idx; // [esp+8h] [ebp-4h]

    if (!msgwnd->activeLineCount)
        return -1;
    for (idx = msgwnd->activeLineCount - 1; idx >= 0; --idx)
    {
        if ((msgwnd->lines[(idx + msgwnd->firstLineIndex) % msgwnd->lineCount].flags & 1) != 0)
            return (idx + msgwnd->firstLineIndex) % msgwnd->lineCount;
    }
    return -1;
}

int __cdecl PrintTimeWriteOut(MessageWindow *msgwnd, MessageLine *line)
{
    if ((line->flags & 1) != 0)
        return con_typewriterPrintSpeed->current.integer * PrintableCharsCount(msgwnd, line);
    else
        return 0;
}

void __cdecl Con_Linefeed(int localClientNum, unsigned int channel, int flags)
{
    Con_UpdateNotifyLine(localClientNum, channel, 1, flags);
    con.lineOffset = 0;
    if (con.displayLineOffset == con.consoleWindow.activeLineCount - 1)
        ++con.displayLineOffset;
}

void __cdecl CL_ConsoleFixPosition()
{
    CL_ConsolePrint(0, 0, "\n", 0, 0, 0);
    con.displayLineOffset = con.consoleWindow.activeLineCount - 1;
}

void __cdecl CL_DeathMessagePrint(
    int localClientNum,
    char *attackerName,
    char attackerColorIndex,
    char *victimName,
    char victimColorIndex,
    Material *iconShader,
    float iconWidth,
    float iconHeight,
    bool horzFlipIcon)
{
    unsigned int deathMsgLen; // [esp+10h] [ebp-410h]
    unsigned int deathMsgLena; // [esp+10h] [ebp-410h]
    unsigned int deathMsgLenb; // [esp+10h] [ebp-410h]
    unsigned int deathMsgLenc; // [esp+10h] [ebp-410h]
    unsigned int deathMsgLend; // [esp+10h] [ebp-410h]
    unsigned int deathMsgLene; // [esp+10h] [ebp-410h]
    unsigned int deathMsgLenf; // [esp+10h] [ebp-410h]
    unsigned int deathMsgLeng; // [esp+10h] [ebp-410h]
    unsigned int deathMsgLenh; // [esp+10h] [ebp-410h]
    char deathMsg[1024]; // [esp+18h] [ebp-408h] BYREF
    int color; // [esp+41Ch] [ebp-4h]

    if (!attackerName)
        MyAssertHandler(".\\client\\cl_console.cpp", 1468, 0, "%s", "attackerName != NULL");
    if (!victimName)
        MyAssertHandler(".\\client\\cl_console.cpp", 1469, 0, "%s", "victimName != NULL");
    if (!&victimColorIndex || victimColorIndex == 94 || victimColorIndex < 48 || victimColorIndex > 57)
        MyAssertHandler(".\\client\\cl_console.cpp", 1470, 0, "%s", "I_IsColorIndex( &victimColorIndex )");
    if (!&attackerColorIndex || attackerColorIndex == 94 || attackerColorIndex < 48 || attackerColorIndex > 57)
        MyAssertHandler(".\\client\\cl_console.cpp", 1471, 0, "%s", "I_IsColorIndex( &attackerColorIndex )");
    if (!cl_noprint || !cl_noprint->current.enabled)
    {
        if (!con.initialized)
        {
            Con_OneTimeInit();
            if (!con.initialized)
                MyAssertHandler(".\\client\\cl_console.cpp", 1480, 0, "%s", "con.initialized");
        }
        deathMsgLen = 0;
        if (con.lineOffset)
            Con_Linefeed(localClientNum, con.prevChannel, 0);
        color = ColorIndex(0x37u);
        if (*attackerName)
        {
            if (attackerColorIndex < 48 || attackerColorIndex > 57)
                MyAssertHandler(
                    ".\\client\\cl_console.cpp",
                    1494,
                    0,
                    "attackerColorIndex not in [COLOR_FIRST, COLOR_LAST]\n\t%i not in [%i, %i]",
                    attackerColorIndex,
                    48,
                    57);
            deathMsg[0] = 94;
            if (!attackerColorIndex)
                MyAssertHandler(".\\client\\cl_console.cpp", 1403, 0, "%s", "c != '\\0'");
            deathMsg[1] = attackerColorIndex;
            deathMsgLena = CL_AddDeathMessageString(deathMsg, 2u, 0x400u, attackerName);
            if (deathMsgLena + 1 > 0x400)
                MyAssertHandler(".\\client\\cl_console.cpp", 1404, 0, "%s", "deathMsgLen + 1 <= deathMsgMaxLen");
            deathMsg[deathMsgLena] = 94;
            deathMsgLenb = deathMsgLena + 1;
            if (deathMsgLenb + 1 > 0x400)
                MyAssertHandler(".\\client\\cl_console.cpp", 1404, 0, "%s", "deathMsgLen + 1 <= deathMsgMaxLen");
            deathMsg[deathMsgLenb] = 55;
            deathMsgLenc = deathMsgLenb + 1;
            if (deathMsgLenc + 1 > 0x400)
                MyAssertHandler(".\\client\\cl_console.cpp", 1404, 0, "%s", "deathMsgLen + 1 <= deathMsgMaxLen");
            deathMsg[deathMsgLenc] = 32;
            deathMsgLen = deathMsgLenc + 1;
        }
        deathMsgLend = CL_AddDeathMessageIcon(
            deathMsg,
            deathMsgLen,
            0x400u,
            iconShader,
            iconWidth,
            iconHeight,
            horzFlipIcon);
        if (victimColorIndex < 48 || victimColorIndex > 57)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                1505,
                0,
                "victimColorIndex not in [COLOR_FIRST, COLOR_LAST]\n\t%i not in [%i, %i]",
                victimColorIndex,
                48,
                57);
        if (deathMsgLend + 1 > 0x400)
            MyAssertHandler(".\\client\\cl_console.cpp", 1404, 0, "%s", "deathMsgLen + 1 <= deathMsgMaxLen");
        deathMsg[deathMsgLend] = 32;
        deathMsgLene = deathMsgLend + 1;
        if (deathMsgLene + 1 > 0x400)
            MyAssertHandler(".\\client\\cl_console.cpp", 1404, 0, "%s", "deathMsgLen + 1 <= deathMsgMaxLen");
        deathMsg[deathMsgLene] = 94;
        deathMsgLenf = deathMsgLene + 1;
        if (!victimColorIndex)
            MyAssertHandler(".\\client\\cl_console.cpp", 1403, 0, "%s", "c != '\\0'");
        if (deathMsgLenf + 1 > 0x400)
            MyAssertHandler(".\\client\\cl_console.cpp", 1404, 0, "%s", "deathMsgLen + 1 <= deathMsgMaxLen");
        deathMsg[deathMsgLenf] = victimColorIndex;
        deathMsgLeng = CL_AddDeathMessageString(deathMsg, deathMsgLenf + 1, 0x400u, victimName);
        if (deathMsgLeng + 1 > 0x400)
            MyAssertHandler(".\\client\\cl_console.cpp", 1404, 0, "%s", "deathMsgLen + 1 <= deathMsgMaxLen");
        deathMsg[deathMsgLeng] = 10;
        deathMsgLenh = deathMsgLeng + 1;
        if (deathMsgLenh >= 0x400)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                1512,
                0,
                "deathMsgLen < ARRAY_COUNT( deathMsg )\n\t%i, %i",
                deathMsgLenh,
                1024);
        deathMsg[deathMsgLenh] = 0;
        CL_ConsolePrint(localClientNum, 5, deathMsg, 0, con.visiblePixelWidth, 0);
    }
}

unsigned int __cdecl CL_AddDeathMessageString(
    char *deathMsg,
    unsigned int deathMsgLen,
    unsigned int deathMsgMaxLen,
    char *string)
{
    char v5; // [esp+3h] [ebp-1h]

    while (*string)
    {
        v5 = *string;
        if (deathMsgLen + 1 > deathMsgMaxLen)
            MyAssertHandler(".\\client\\cl_console.cpp", 1404, 0, "%s", "deathMsgLen + 1 <= deathMsgMaxLen");
        deathMsg[deathMsgLen++] = v5;
        ++string;
    }
    return deathMsgLen;
}

unsigned int __cdecl CL_AddDeathMessageIcon(
    char *deathMsg,
    unsigned int deathMsgLen,
    unsigned int deathMsgMaxLen,
    Material *iconShader,
    float iconWidth,
    float iconHeight,
    bool horzFlipIcon)
{
    char v8; // [esp+7h] [ebp-41h]
    char v9; // [esp+23h] [ebp-25h]
    unsigned int deathMsgLena; // [esp+54h] [ebp+Ch]
    unsigned int deathMsgLenb; // [esp+54h] [ebp+Ch]
    unsigned int deathMsgLenc; // [esp+54h] [ebp+Ch]
    unsigned int deathMsgLend; // [esp+54h] [ebp+Ch]
    unsigned int deathMsgLene; // [esp+54h] [ebp+Ch]

    if (iconWidth <= 0.0)
        MyAssertHandler(".\\client\\cl_console.cpp", 1432, 0, "%s", "iconWidth > 0");
    if (iconHeight <= 0.0)
        MyAssertHandler(".\\client\\cl_console.cpp", 1433, 0, "%s", "iconHeight > 0");
    if (!IsValidMaterialHandle(iconShader))
        MyAssertHandler(".\\client\\cl_console.cpp", 1436, 0, "%s", "IsValidMaterialHandle( iconShader )");
    if (deathMsgLen + 1 > deathMsgMaxLen)
        MyAssertHandler(".\\client\\cl_console.cpp", 1404, 0, "%s", "deathMsgLen + 1 <= deathMsgMaxLen");
    deathMsg[deathMsgLen] = 94;
    deathMsgLena = deathMsgLen + 1;
    if (horzFlipIcon == -1)
        MyAssertHandler(".\\client\\cl_console.cpp", 1403, 0, "%s", "c != '\\0'");
    if (deathMsgLena + 1 > deathMsgMaxLen)
        MyAssertHandler(".\\client\\cl_console.cpp", 1404, 0, "%s", "deathMsgLen + 1 <= deathMsgMaxLen");
    deathMsg[deathMsgLena] = horzFlipIcon + 1;
    deathMsgLenb = deathMsgLena + 1;
    v9 = CL_DeathMessageIconDimension(iconWidth);
    if (!v9)
        MyAssertHandler(".\\client\\cl_console.cpp", 1403, 0, "%s", "c != '\\0'");
    if (deathMsgLenb + 1 > deathMsgMaxLen)
        MyAssertHandler(".\\client\\cl_console.cpp", 1404, 0, "%s", "deathMsgLen + 1 <= deathMsgMaxLen");
    deathMsg[deathMsgLenb] = v9;
    deathMsgLenc = deathMsgLenb + 1;
    v8 = CL_DeathMessageIconDimension(iconHeight);
    if (!v8)
        MyAssertHandler(".\\client\\cl_console.cpp", 1403, 0, "%s", "c != '\\0'");
    if (deathMsgLenc + 1 > deathMsgMaxLen)
        MyAssertHandler(".\\client\\cl_console.cpp", 1404, 0, "%s", "deathMsgLen + 1 <= deathMsgMaxLen");
    deathMsg[deathMsgLenc] = v8;
    deathMsgLend = deathMsgLenc + 1;
    if (deathMsgLend + 4 > deathMsgMaxLen)
        MyAssertHandler(".\\client\\cl_console.cpp", 1449, 0, "%s", "deathMsgLen + sizeof( iconShader ) <= deathMsgMaxLen");
    *(unsigned int *)&deathMsg[deathMsgLend] = (unsigned int)iconShader;
    deathMsgLene = deathMsgLend + 4;
    if (deathMsgLene - deathMsgLen != 8)
        MyAssertHandler(
            ".\\client\\cl_console.cpp",
            1455,
            0,
            "%s",
            "deathMsgLen - startDeathMsgLen == CONTXTCMD_LEN_HUDICON + 1");
    return deathMsgLene;
}

int __cdecl CL_DeathMessageIconDimension(float size)
{
    int v2; // [esp+0h] [ebp-1Ch]
    int v3; // [esp+4h] [ebp-18h]
    float v4; // [esp+Ch] [ebp-10h]

    v4 = size * 32.0;
    if ((int)(v4 + 9.313225746154785e-10) < 127)
        v3 = (int)(v4 + 9.313225746154785e-10);
    else
        v3 = 127;
    if (v3 > 16)
        v2 = v3;
    else
        v2 = 16;
    return v2 + 16;
}

void __cdecl Con_AutoCompleteFromList(
    const char **strings,
    unsigned int stringCount,
    const char *prefix,
    char *completed,
    unsigned int sizeofCompleted)
{
    int v5; // [esp+0h] [ebp-20h]
    char *string; // [esp+10h] [ebp-10h]
    unsigned int charIndex; // [esp+14h] [ebp-Ch]
    unsigned int stringIndex; // [esp+1Ch] [ebp-4h]

    v5 = strlen(prefix);
    *completed = 0;
    for (stringIndex = 0; stringIndex < stringCount; ++stringIndex)
    {
        string = (char *)strings[stringIndex];
        if (!I_strnicmp(prefix, string, v5))
        {
            if (*completed)
            {
                for (charIndex = v5; string[charIndex] == completed[charIndex] && completed[charIndex]; ++charIndex)
                    ;
                completed[charIndex] = 0;
            }
            else
            {
                I_strncpyz(completed, string, sizeofCompleted);
            }
        }
    }
}

const char *__cdecl Con_TokenizeInput()
{
    const char *cmd; // [esp+0h] [ebp-4h]

    Cmd_TokenizeString(g_consoleField.buffer);
    cmd = Cmd_Argv(0);
    if (*cmd == 92 || *cmd == 47)
        ++cmd;
    while (isspace(*cmd))
        ++cmd;
    return cmd;
}

char __cdecl Con_AnySpaceAfterCommand()
{
    int charIndex; // [esp+0h] [ebp-4h]

    for (charIndex = 0; isspace(g_consoleField.buffer[charIndex]); ++charIndex)
        ;
    while (g_consoleField.buffer[charIndex])
    {
        if (isspace(g_consoleField.buffer[charIndex]))
            return 1;
        ++charIndex;
    }
    return 0;
}

bool __cdecl Con_IsAutoCompleteMatch(const char *query, const char *matchToText, int matchTextLen)
{
    int matchLetter; // [esp+0h] [ebp-Ch]
    int matchTextPos; // [esp+4h] [ebp-8h]
    const char *queryPos; // [esp+8h] [ebp-4h]

    if (!query)
        MyAssertHandler(".\\client\\cl_console.cpp", 1823, 0, "%s", "query");
    if (!matchToText)
        MyAssertHandler(".\\client\\cl_console.cpp", 1824, 0, "%s", "matchToText");
    if (!matchTextLen)
        MyAssertHandler(".\\client\\cl_console.cpp", 1825, 0, "%s", "matchTextLen");
    if (!con_ignoreMatchPrefixOnly && con_matchPrefixOnly->current.enabled)
        return I_strnicmp(query, matchToText, matchTextLen) == 0;
    matchTextPos = 0;
    matchLetter = tolower(*matchToText);
    for (queryPos = query; *queryPos; ++queryPos)
    {
        if (tolower(*queryPos) == matchLetter)
        {
            if (++matchTextPos == matchTextLen)
                return 1;
            matchLetter = tolower(matchToText[matchTextPos]);
        }
        else if (con_matchPrefixOnly->current.enabled)
        {
            if (matchTextPos)
            {
                matchTextPos = 0;
                matchLetter = tolower(*matchToText);
            }
        }
    }
    return 0;
}

bool __cdecl Con_HasTooManyMatchesToShow()
{
    return conDrawInputGlob.matchCount > con_inputMaxMatchesShown;
}

bool __cdecl Con_IsDvarCommand(const char *cmd)
{
    if (!I_stricmp(cmd, "set"))
        return 1;
    if (!I_stricmp(cmd, "seta"))
        return 1;
    if (!I_stricmp(cmd, "sets"))
        return 1;
    if (!I_stricmp(cmd, "reset"))
        return 1;
    if (I_stricmp(cmd, "toggle"))
        return I_stricmp(cmd, "togglep") == 0;
    return 1;
}

char __cdecl Con_CycleAutoComplete(int step)
{
    if (!conDrawInputGlob.mayAutoComplete
        || conDrawInputGlob.matchCount <= 1
        || conDrawInputGlob.matchCount >= con_inputMaxMatchesShown
        || conDrawInputGlob.hasExactMatch && Con_AnySpaceAfterCommand())
    {
        return 0;
    }
    conDrawInputGlob.matchIndex += step;
    if (conDrawInputGlob.matchIndex >= 0)
    {
        if (conDrawInputGlob.matchIndex >= conDrawInputGlob.matchCount)
            conDrawInputGlob.matchIndex = 0;
    }
    else
    {
        conDrawInputGlob.matchIndex = conDrawInputGlob.matchCount - 1;
    }
    return 1;
}

bool __cdecl Con_HasActiveAutoComplete()
{
    return conDrawInputGlob.matchIndex >= 0 && conDrawInputGlob.autoCompleteChoice[0];
}

char __cdecl Con_CommitToAutoComplete()
{
    char v1; // [esp+13h] [ebp-11h]
    char *buffer; // [esp+18h] [ebp-Ch]
    ConDrawInputGlob *v3; // [esp+1Ch] [ebp-8h]
    const char *originalCommand; // [esp+20h] [ebp-4h]

    if (!Con_HasActiveAutoComplete())
        return 0;
    originalCommand = Con_TokenizeInput();
    if (Con_IsDvarCommand(originalCommand))
    {
        Com_sprintf(g_consoleField.buffer, 0x100u, "%s %s", originalCommand, conDrawInputGlob.autoCompleteChoice);
    }
    else
    {
        v3 = &conDrawInputGlob;
        buffer = g_consoleField.buffer;
        do
        {
            v1 = v3->autoCompleteChoice[0];
            *buffer = v3->autoCompleteChoice[0];
            v3 = (ConDrawInputGlob *)((char *)v3 + 1);
            ++buffer;
        } while (v1);
    }
    Cmd_EndTokenizedString();
    g_consoleField.cursor = strlen(g_consoleField.buffer);
    g_consoleField.buffer[g_consoleField.cursor++] = 32;
    g_consoleField.buffer[g_consoleField.cursor] = 0;
    g_consoleField.drawWidth = SEH_PrintStrlen(g_consoleField.buffer);
    Con_CancelAutoComplete();
    return 1;
}

char __cdecl Con_CancelAutoComplete()
{
    if (!Con_HasActiveAutoComplete())
        return 0;
    conDrawInputGlob.matchIndex = -1;
    conDrawInputGlob.autoCompleteChoice[0] = 0;
    return 1;
}

void __cdecl Con_AllowAutoCompleteCycling(bool isAllowed)
{
    conDrawInputGlob.mayAutoComplete = isAllowed;
}

void __cdecl Con_DrawGameMessageWindow(
    int localClientNum,
    unsigned int windowIndex,
    int xPos,
    int yPos,
    int horzAlign,
    int vertAlign,
    Font_s *font,
    float fontScale,
    float *color,
    int textStyle,
    char textAlignMode,
    msgwnd_mode_t mode)
{
    float v12; // [esp+Ch] [ebp-18h]

    if (!cg_paused->current.integer)
    {
        if (windowIndex >= 4)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                2735,
                0,
                "windowIndex doesn't index GAMEMSG_WINDOW_COUNT\n\t%i not in [0, %i)",
                windowIndex,
                4);
        v12 = fontScale * 48.0;
        Con_DrawMessageWindow(
            localClientNum,
            (MessageWindow *)&con.color[4630 * localClientNum - 2582 + 13 * windowIndex],
            xPos,
            yPos,
            (int)(v12 + 9.313225746154785e-10),
            horzAlign,
            vertAlign,
            font,
            color,
            textStyle,
            1.0,
            mode,
            textAlignMode);
    }
}

void __cdecl Con_DrawMessageWindow(
    int localClientNum,
    MessageWindow *msgwnd,
    int x,
    int y,
    int charHeight,
    int horzAlign,
    int vertAlign,
    Font_s *font,
    float *color,
    int textStyle,
    float msgwndScale,
    msgwnd_mode_t mode,
    char textAlignMode)
{
    if (!msgwnd)
        MyAssertHandler(".\\client\\cl_console.cpp", 2688, 0, "%s", "msgwnd");
    if (!CL_ShouldntDrawMessageWindow(localClientNum))
    {
        switch (mode)
        {
        case MWM_BOTTOMUP_ALIGN_TOP:
        case MWM_TOPDOWN_ALIGN_BOTTOM:
            Con_DrawMessageWindowOldToNew(
                localClientNum,
                msgwnd,
                x,
                y,
                charHeight,
                horzAlign,
                vertAlign,
                mode == MWM_TOPDOWN_ALIGN_BOTTOM,
                font,
                color,
                textStyle,
                msgwndScale,
                textAlignMode);
            break;
        case MWM_BOTTOMUP_ALIGN_BOTTOM:
        case MWM_TOPDOWN_ALIGN_TOP:
            Con_DrawMessageWindowNewToOld(
                localClientNum,
                msgwnd,
                x,
                y,
                charHeight,
                horzAlign,
                vertAlign,
                mode == MWM_BOTTOMUP_ALIGN_BOTTOM,
                font,
                color,
                textStyle,
                msgwndScale,
                textAlignMode);
            break;
        default:
            if (!alwaysfails)
                MyAssertHandler(".\\client\\cl_console.cpp", 2706, 0, "unhandled case");
            break;
        }
    }
}

void __cdecl Con_DrawMessageWindowNewToOld(
    int localClientNum,
    MessageWindow *msgwnd,
    int x,
    int y,
    int hudCharHeight,
    int horzAlign,
    int vertAlign,
    bool up,
    Font_s *font,
    float *color,
    int textStyle,
    float msgwndScale,
    char textAlignMode)
{
    float v13; // [esp+8h] [ebp-68h]
    float v14; // [esp+Ch] [ebp-64h]
    float v15; // [esp+10h] [ebp-60h]
    float v16; // [esp+18h] [ebp-58h]
    float v17; // [esp+2Ch] [ebp-44h]
    float v18; // [esp+3Ch] [ebp-34h]
    float finalColor[4]; // [esp+40h] [ebp-30h] BYREF
    Message *message; // [esp+50h] [ebp-20h]
    float lerpFactor; // [esp+54h] [ebp-1Ch]
    int lineOffset; // [esp+58h] [ebp-18h]
    int imod; // [esp+5Ch] [ebp-14h]
    int time; // [esp+60h] [ebp-10h]
    MessageLine *line; // [esp+64h] [ebp-Ch]
    int charHeight; // [esp+68h] [ebp-8h]
    int serverTime; // [esp+6Ch] [ebp-4h]

    if (!msgwnd)
        MyAssertHandler(".\\client\\cl_console.cpp", 2543, 0, "%s", "msgwnd");
    serverTime = CL_GetLocalClientGlobals(localClientNum)->serverTime;
    Con_CullFinishedLines(serverTime, msgwnd);
    charHeight = hudCharHeight;
    if (!up)
        y -= charHeight;
    for (lineOffset = 0; lineOffset < msgwnd->activeLineCount; ++lineOffset)
    {
        if (msgwnd->lineCount <= 0)
            MyAssertHandler(".\\client\\cl_console.cpp", 2554, 0, "%s", "msgwnd->lineCount > 0");
        imod = (lineOffset + msgwnd->firstLineIndex) % msgwnd->lineCount;
        if ((unsigned int)imod >= msgwnd->lineCount)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                2556,
                0,
                "imod doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                imod,
                msgwnd->lineCount);
        line = &msgwnd->lines[imod];
        if (line->messageIndex >= (unsigned int)msgwnd->lineCount)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                2558,
                0,
                "line->messageIndex doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                line->messageIndex,
                msgwnd->lineCount);
        message = &msgwnd->messages[line->messageIndex];
        if (message->startTime < 0 || message->startTime > serverTime + 1000)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                2560,
                0,
                "message->startTime not in [0, serverTime + CON_MSG_TIME_DRIFT_BUFFER]\n\t%i not in [%i, %i]",
                message->startTime,
                0,
                serverTime + 1000);
        time = serverTime - message->startTime;
        if (time < msgwnd->scrollTime)
        {
            lerpFactor = 1.0 - (double)time / (double)msgwnd->scrollTime;
            v15 = lerpFactor - 1.0;
            if (v15 < 0.0)
                v18 = lerpFactor;
            else
                v18 = 1.0;
            v14 = 0.0 - lerpFactor;
            if (v14 < 0.0)
                v13 = v18;
            else
                v13 = 0.0;
            lerpFactor = v13;
            if (up)
            {
                v17 = (double)charHeight * lerpFactor;
                y += (int)(v17 + 9.313225746154785e-10);
            }
            else
            {
                v16 = (double)charHeight * lerpFactor;
                y -= (int)(v16 + 9.313225746154785e-10);
            }
        }
    }
    finalColor[0] = *color;
    finalColor[1] = color[1];
    finalColor[2] = color[2];
    finalColor[3] = color[3];
    for (lineOffset = msgwnd->activeLineCount - 1; lineOffset >= 0; --lineOffset)
    {
        imod = (lineOffset + msgwnd->firstLineIndex) % msgwnd->lineCount;
        if ((unsigned int)imod >= msgwnd->lineCount)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                2581,
                0,
                "imod doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                imod,
                msgwnd->lineCount);
        line = &msgwnd->lines[imod];
        if (line->messageIndex >= (unsigned int)msgwnd->lineCount)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                2584,
                0,
                "line->messageIndex doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                line->messageIndex,
                msgwnd->lineCount);
        message = &msgwnd->messages[line->messageIndex];
        if (up)
            y -= charHeight;
        else
            y += charHeight;
        if (serverTime - message->endTime < 0)
        {
            finalColor[3] = Con_GetMessageAlpha(message, msgwnd, serverTime, 1) * color[3];
            Con_DrawMessageLineOnHUD(
                localClientNum,
                &scrPlaceView[localClientNum],
                x,
                y,
                charHeight,
                horzAlign,
                vertAlign,
                font,
                msgwnd,
                imod,
                finalColor,
                textStyle,
                msgwndScale,
                textAlignMode);
        }
    }
}

float MY_GLOWCOLOR[4] = { 0.0f, 0.3f, 0.0f, 1.0f };

void __cdecl Con_DrawMessageLineOnHUD(
    int localClientNum,
    const ScreenPlacement *scrPlace,
    int x,
    int y,
    int charHeight,
    int horzAlign,
    int vertAlign,
    Font_s *font,
    const MessageWindow *msgwnd,
    int lineIdx,
    float *color,
    int textStyle,
    float msgwndScale,
    char textAlignMode)
{
    int v14; // [esp+3Ch] [ebp-44h]
    int v15; // [esp+48h] [ebp-38h]
    float scale; // [esp+4Ch] [ebp-34h]
    DvarValue *glowColor; // [esp+50h] [ebp-30h]
    float typewriterColor[4]; // [esp+54h] [ebp-2Ch] BYREF
    float xScale; // [esp+64h] [ebp-1Ch] BYREF
    float yAdj; // [esp+68h] [ebp-18h] BYREF
    float xAdj; // [esp+6Ch] [ebp-14h] BYREF
    int time; // [esp+70h] [ebp-10h]
    MessageLine *line; // [esp+74h] [ebp-Ch]
    float fontScale; // [esp+78h] [ebp-8h]
    float yScale; // [esp+7Ch] [ebp-4h] BYREF

    time = CL_GetLocalClientGlobals(localClientNum)->serverTime;
    if (LineVisible(msgwnd, lineIdx, time))
    {
        line = &msgwnd->lines[lineIdx];
        if ((line->flags & 1) != 0)
            font = UI_GetFontHandle(0, 6, 1.0);
        fontScale = (double)charHeight / 48.0;
        scale = fontScale * msgwndScale;
        xScale = R_NormalizedTextScale(font, scale);
        yScale = xScale;
        v15 = textAlignMode & 3;
        if (v15 == 1)
        {
            x -= (int)((double)R_ConsoleTextWidth(
                msgwnd->circularTextBuffer,
                msgwnd->textBufSize,
                line->textBufPos,
                line->textBufSize,
                font)
                * xScale
                * 0.5);
        }
        else if (v15 == 2)
        {
            x -= (int)((double)R_ConsoleTextWidth(
                msgwnd->circularTextBuffer,
                msgwnd->textBufSize,
                line->textBufPos,
                line->textBufSize,
                font)
                * xScale);
        }
        v14 = textAlignMode & 0xC;
        if (v14 == 4)
        {
            y += (int)((double)R_TextHeight(font) * yScale);
        }
        else if (v14 == 8)
        {
            y += (int)((double)R_TextHeight(font) * yScale * 0.5);
        }
        xAdj = (float)x;
        yAdj = (float)y;
        ScrPlace_ApplyRect(scrPlace, &xAdj, &yAdj, &xScale, &yScale, horzAlign, vertAlign);
        if ((line->flags & 1) != 0)
        {
            typewriterColor[0] = con_typewriterColorBase->current.value;
            typewriterColor[1] = con_typewriterColorBase->current.vector[1];
            typewriterColor[2] = con_typewriterColorBase->current.vector[2];
            typewriterColor[3] = 1.0;
            if ((line->flags & 0x10) != 0)
            {
                glowColor = (DvarValue*)&con_typewriterColorGlowCheckpoint->current;
            }
            else if ((line->flags & 4) != 0)
            {
                glowColor = (DvarValue *)&con_typewriterColorGlowCompleted->current;
            }
            else if ((line->flags & 8) != 0)
            {
                glowColor = (DvarValue *)&con_typewriterColorGlowFailed->current;
            }
            else
            {
                glowColor = (DvarValue *)&con_typewriterColorGlowUpdated->current;
            }
            TypewriterSounds(localClientNum, msgwnd, line);
            R_AddCmdDrawConsoleTextPulseFX(
                msgwnd->circularTextBuffer,
                msgwnd->textBufSize,
                line->textBufPos,
                line->textBufSize,
                font,
                xAdj,
                yAdj,
                xScale,
                yScale,
                typewriterColor,
                textStyle,
                &glowColor->value,
                line->typingStartTime,
                con_typewriterPrintSpeed->current.integer,
                con_typewriterDecayStartTime->current.integer,
                con_typewriterDecayDuration->current.integer,
                cgMedia.textDecodeCharacters,
                cgMedia.textDecodeCharactersGlow);
        }
        else if ((line->flags & 0x20) != 0)
        {
            R_AddCmdDrawConsoleTextSubtitle(
                msgwnd->circularTextBuffer,
                msgwnd->textBufSize,
                line->textBufPos,
                line->textBufSize,
                font,
                xAdj,
                yAdj,
                xScale,
                yScale,
                color,
                textStyle,
                MY_GLOWCOLOR);
        }
        else
        {
            R_AddCmdDrawConsoleText(
                msgwnd->circularTextBuffer,
                msgwnd->textBufSize,
                line->textBufPos,
                line->textBufSize,
                font,
                xAdj,
                yAdj,
                xScale,
                yScale,
                color,
                textStyle);
        }
    }
}

bool __cdecl LineVisible(const MessageWindow *msgwnd, int lineIdx, int time)
{
    if (!msgwnd)
        MyAssertHandler(".\\client\\cl_console.cpp", 2361, 0, "%s", "msgwnd");
    return time >= msgwnd->lines[lineIdx].typingStartTime;
}

void __cdecl TypewriterSounds(int localClientNum, const MessageWindow *msgwnd, MessageLine *line)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-8h]
    int strLength; // [esp+4h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    strLength = PrintableCharsCount(msgwnd, line);
    CL_PlayTextFXPulseSounds(
        localClientNum,
        LocalClientGlobals->serverTime,
        strLength,
        line->typingStartTime,
        con_typewriterPrintSpeed->current.integer,
        con_typewriterDecayStartTime->current.integer,
        &line->lastTypingSoundTime);
}

void __cdecl Con_CullFinishedLines(int serverTime, MessageWindow *msgwnd)
{
    const MessageLine *line; // [esp+4h] [ebp-4h]

    if (!msgwnd)
        MyAssertHandler(".\\client\\cl_console.cpp", 2472, 0, "%s", "msgwnd");
    if (msgwnd->lineCount <= 0)
        MyAssertHandler(".\\client\\cl_console.cpp", 2473, 0, "%s", "msgwnd->lineCount > 0");
    while (msgwnd->activeLineCount)
    {
        line = &msgwnd->lines[msgwnd->firstLineIndex];
        if (line->messageIndex >= (unsigned int)msgwnd->lineCount)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                2479,
                0,
                "line->messageIndex doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                line->messageIndex,
                msgwnd->lineCount);
        if (serverTime - msgwnd->messages[line->messageIndex].endTime < 0)
            break;
        Con_FreeFirstMessageWindowLine(msgwnd);
    }
}

double __cdecl Con_GetMessageAlpha(Message *message, MessageWindow *msgwnd, int serverTime, bool scrollsIntoPlace)
{
    float v6; // [esp+4h] [ebp-14h]
    float curalpha; // [esp+14h] [ebp-4h]

    curalpha = 1.0;
    if (!message)
        MyAssertHandler(".\\client\\cl_console.cpp", 2493, 0, "%s", "message");
    if (!msgwnd)
        MyAssertHandler(".\\client\\cl_console.cpp", 2494, 0, "%s", "msgwnd");
    if (message->endTime - serverTime < msgwnd->fadeOut)
    {
        if (msgwnd->fadeOut <= 0)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                2501,
                0,
                "%s\n\t(msgwnd->fadeOut) = %i",
                "(msgwnd->fadeOut > 0)",
                msgwnd->fadeOut);
        curalpha = (double)(message->endTime - serverTime) / (double)msgwnd->fadeOut * curalpha;
    }
    if (scrollsIntoPlace && msgwnd->fadeIn < msgwnd->scrollTime)
    {
        if (serverTime - message->startTime < msgwnd->scrollTime)
        {
            if (serverTime - message->startTime <= msgwnd->scrollTime - msgwnd->fadeIn)
                return 0.0;
            if (msgwnd->fadeIn <= 0)
                MyAssertHandler(
                    ".\\client\\cl_console.cpp",
                    2513,
                    0,
                    "%s\n\t(msgwnd->fadeIn) = %i",
                    "(msgwnd->fadeIn > 0)",
                    msgwnd->fadeIn);
            curalpha = (double)(serverTime - (msgwnd->scrollTime - msgwnd->fadeIn + message->startTime))
                / (double)msgwnd->fadeIn
                * curalpha;
        }
    }
    else if (msgwnd->fadeIn && serverTime - message->startTime < msgwnd->fadeIn)
    {
        curalpha = (double)(serverTime - message->startTime) / (double)msgwnd->fadeIn * curalpha;
    }
    v6 = 0.0 - curalpha;
    if (v6 < 0.0)
        return curalpha;
    else
        return (float)0.0;
}

void __cdecl Con_DrawMessageWindowOldToNew(
    int localClientNum,
    MessageWindow *msgwnd,
    int x,
    int y,
    int charHeight,
    int horzAlign,
    int vertAlign,
    bool up,
    Font_s *font,
    float *color,
    int textStyle,
    float msgwndScale,
    char textAlignMode)
{
    float v13; // [esp+Ch] [ebp-58h]
    float v14; // [esp+20h] [ebp-44h]
    clientActive_t *LocalClientGlobals; // [esp+30h] [ebp-34h]
    float finalColor[4]; // [esp+34h] [ebp-30h] BYREF
    Message *message; // [esp+44h] [ebp-20h]
    int lineOffset; // [esp+48h] [ebp-1Ch]
    int imod; // [esp+4Ch] [ebp-18h]
    int time; // [esp+50h] [ebp-14h]
    MessageLine *line; // [esp+54h] [ebp-10h]
    int v; // [esp+58h] [ebp-Ch]
    int groupsize; // [esp+5Ch] [ebp-8h]
    int serverTime; // [esp+60h] [ebp-4h]

    if (!msgwnd)
        MyAssertHandler(".\\client\\cl_console.cpp", 2616, 0, "%s", "msgwnd");
    serverTime = CL_GetLocalClientGlobals(localClientNum)->serverTime;
    Con_CullFinishedLines(serverTime, msgwnd);
    groupsize = 0;
    v = y;
    if (up)
        v -= charHeight;
    finalColor[0] = *color;
    finalColor[1] = color[1];
    finalColor[2] = color[2];
    finalColor[3] = color[3];
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    for (lineOffset = 0; lineOffset < msgwnd->activeLineCount; ++lineOffset)
    {
        if (msgwnd->lineCount <= 0)
            MyAssertHandler(".\\client\\cl_console.cpp", 2632, 0, "%s", "msgwnd->lineCount > 0");
        imod = (lineOffset + msgwnd->firstLineIndex) % msgwnd->lineCount;
        if ((unsigned int)imod >= msgwnd->lineCount)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                2634,
                0,
                "imod doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                imod,
                msgwnd->lineCount);
        line = &msgwnd->lines[imod];
        if (line->messageIndex >= (unsigned int)msgwnd->lineCount)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                2636,
                0,
                "line->messageIndex doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                line->messageIndex,
                msgwnd->lineCount);
        message = &msgwnd->messages[line->messageIndex];
        if (message->startTime < 0 || message->startTime > LocalClientGlobals->serverTime + 1000)
            MyAssertHandler(
                ".\\client\\cl_console.cpp",
                2639,
                0,
                "message->startTime not in [0, cl->serverTime + CON_MSG_TIME_DRIFT_BUFFER]\n\t%i not in [%i, %i]",
                message->startTime,
                0,
                LocalClientGlobals->serverTime + 1000);
        if (LocalClientGlobals->serverTime <= message->endTime)
        {
            if (LocalClientGlobals->serverTime > message->endTime - msgwnd->scrollTime)
            {
                time = LocalClientGlobals->serverTime - (message->endTime - msgwnd->scrollTime);
                if (time > 0)
                {
                    if (msgwnd->scrollTime <= 0)
                        MyAssertHandler(
                            ".\\client\\cl_console.cpp",
                            2650,
                            0,
                            "%s\n\t(msgwnd->scrollTime) = %i",
                            "(msgwnd->scrollTime > 0)",
                            msgwnd->scrollTime);
                    if (up)
                    {
                        v14 = (double)charHeight * ((double)time / (double)msgwnd->scrollTime);
                        v += (int)(v14 + 9.313225746154785e-10);
                    }
                    else
                    {
                        v13 = (double)charHeight * ((double)time / (double)msgwnd->scrollTime);
                        v -= (int)(v13 + 9.313225746154785e-10);
                    }
                }
            }
            finalColor[3] = Con_GetMessageAlpha(message, msgwnd, LocalClientGlobals->serverTime, 0) * color[3];
            Con_DrawMessageLineOnHUD(
                localClientNum,
                &scrPlaceView[localClientNum],
                x,
                v,
                charHeight,
                horzAlign,
                vertAlign,
                font,
                msgwnd,
                imod,
                finalColor,
                textStyle,
                msgwndScale,
                textAlignMode);
            if (up)
                v -= charHeight;
            else
                v += charHeight;
        }
    }
}

bool __cdecl CL_ShouldntDrawMessageWindow(int localClientNum)
{
    return CL_GetLocalClientGlobals(localClientNum)->snap.ps.pm_type != 5 && !CL_ShouldDisplayHud(localClientNum);
}

void __cdecl Con_DrawMiniConsole(int localClientNum, int xPos, int yPos, float alpha)
{
    Font_s *font; // [esp+Ch] [ebp-18h]
    float color[4]; // [esp+14h] [ebp-10h] BYREF

    font = UI_GetFontHandle(&scrPlaceView[localClientNum], 0, 1.0);
    if (con_miniconlines->current.integer > 0x64u)
        MyAssertHandler(
            ".\\client\\cl_console.cpp",
            2757,
            0,
            "%s\n\t(con_miniconlines->current.integer) = %i",
            "(con_miniconlines->current.integer >= 0 && con_miniconlines->current.integer <= 100)",
            con_miniconlines->current.integer);
    if (con.messageBuffer[0].miniconWindow.lineCount != con_miniconlines->current.integer)
    {
        con.messageBuffer[0].miniconWindow.lineCount = con_miniconlines->current.integer;
        Con_ClearMiniConsole(localClientNum);
    }
    color[0] = 1.0;
    color[1] = 1.0;
    color[2] = 1.0;
    color[3] = alpha;
    Con_DrawMessageWindow(
        localClientNum,
        (MessageWindow *)&con.color[4630 * localClientNum - 1122],
        xPos,
        yPos,
        12,
        1,
        1,
        font,
        color,
        3,
        1.0,
        MWM_BOTTOMUP_ALIGN_TOP,
        4);
}

void __cdecl Con_ClearMiniConsole(int localClientNum)
{
    Con_ClearMessageWindow((MessageWindow *)&con.color[4630 * localClientNum - 1122]);
}

void __cdecl Con_DrawErrors(int localClientNum, int xPos, int yPos, float alpha)
{
    Font_s *font; // [esp+Ch] [ebp-14h]
    float color[4]; // [esp+10h] [ebp-10h] BYREF

    font = UI_GetFontHandle(&scrPlaceView[localClientNum], 0, 1.0);
    color[0] = 1.0;
    color[1] = 1.0;
    color[2] = 1.0;
    color[3] = alpha;
    Con_DrawMessageWindow(
        localClientNum,
        (MessageWindow *)&con.color[4630 * localClientNum - 53],
        xPos,
        yPos,
        12,
        1,
        1,
        font,
        color,
        3,
        1.0,
        MWM_BOTTOMUP_ALIGN_TOP,
        4);
}

bool __cdecl Con_IsValidGameMessageWindow(unsigned int windowIndex)
{
    return windowIndex < 4;
}

bool __cdecl Con_IsGameMessageWindowActive(int localClientNum, unsigned int windowIndex)
{
    if (windowIndex >= 4)
        MyAssertHandler(
            ".\\client\\cl_console.cpp",
            2791,
            0,
            "windowIndex doesn't index GAMEMSG_WINDOW_COUNT\n\t%i not in [0, %i)",
            windowIndex,
            4);
    return SLODWORD(con.color[4630 * localClientNum - 2571 + 13 * windowIndex]) > 0;
}

void __cdecl Con_DrawSay(int localClientNum, int x, int y)
{
    char *v3; // eax
    int v4; // eax
    float textY; // [esp+2Ch] [ebp-1Ch]
    float textX; // [esp+30h] [ebp-18h]
    Font_s *font; // [esp+34h] [ebp-14h]
    char *string; // [esp+3Ch] [ebp-Ch]
    float normalizedScale; // [esp+44h] [ebp-4h]
    float normalizedScalea; // [esp+44h] [ebp-4h]

    if (Key_IsCatcherActive(localClientNum, 32))
    {
        if (playerKeys[localClientNum].chat_team)
            v3 = SEH_SafeTranslateString((char*)"EXE_SAYTEAM");
        else
            v3 = SEH_SafeTranslateString((char*)"EXE_SAY");
        string = va("%s: ", v3);
        normalizedScale = playerKeys[localClientNum].chatField.charHeight / 48.0;
        font = UI_GetFontHandle(&scrPlaceView[localClientNum], 0, normalizedScale);
        normalizedScalea = R_NormalizedTextScale(font, normalizedScale);
        textX = (float)x;
        textY = (float)(y + (int)((double)R_TextHeight(font) * normalizedScalea));
        CL_DrawText(
            &scrPlaceView[localClientNum],
            string,
            0x7FFFFFFF,
            font,
            textX,
            textY,
            1,
            1,
            normalizedScalea,
            normalizedScalea,
            colorWhite,
            3);
        v4 = (int)((double)R_TextWidth(string, 0, font) * normalizedScalea);
        Field_Draw(localClientNum, &playerKeys[localClientNum].chatField, x + v4, y, 1, 1);
    }
}

void __cdecl Con_ToggleConsoleOutput()
{
    con.outputVisible = !con.outputVisible;
}

void __cdecl Con_DrawConsole(int localClientNum)
{
    Con_CheckResize();
    if (Key_IsCatcherActive(localClientNum, 1))
        Con_DrawSolidConsole(localClientNum);
}

void __cdecl Con_DrawSolidConsole(int localClientNum)
{
    Sys_EnterCriticalSection(CRITSECT_CONSOLE);
    if (con.lineOffset)
        Con_Linefeed(localClientNum, con.prevChannel, 0);
    Sys_LeaveCriticalSection(CRITSECT_CONSOLE);
    if (!Key_IsCatcherActive(localClientNum, 1))
        con.outputVisible = 0;
    if (con.outputVisible)
        Con_DrawOuputWindow();
    Con_DrawInput(localClientNum);
}


void __cdecl Con_DrawInput(int localClientNum)
{
    bool v1; // [esp+10h] [ebp-3Ch]
    int matchCount; // [esp+34h] [ebp-18h]
    char *tooManyMatchesStr; // [esp+38h] [ebp-14h]
    int inputTextLenPrev; // [esp+3Ch] [ebp-10h]
    char *promptString; // [esp+40h] [ebp-Ch]
    const char *originalCommand; // [esp+48h] [ebp-4h]

    if (!Sys_IsMainThread() && !Sys_IsRenderThread())
        MyAssertHandler(".\\client\\cl_console.cpp", 2163, 0, "%s", "Sys_IsMainThread() || Sys_IsRenderThread()");
    if (Key_IsCatcherActive(localClientNum, 1) && Sys_IsMainThread())
    {
        promptString = va("%s: %s> ", "CoD4 MP", "1.0");
        conDrawInputGlob.fontHeight = (float)R_TextHeight(cls.consoleFont);
        conDrawInputGlob.x = con.screenMin[0] + 6.0;
        conDrawInputGlob.y = con.screenMin[1] + 6.0;
        conDrawInputGlob.leftX = conDrawInputGlob.x;
        ConDrawInput_Box(1, &con_inputBoxColor->current.value);
        ConDrawInput_TextAndOver(promptString, con_versionColor);
        conDrawInputGlob.leftX = conDrawInputGlob.x;
        g_consoleField.widthInPixels = (int)(con.screenMax[0] - 6.0 - conDrawInputGlob.x);
        inputTextLenPrev = conDrawInputGlob.inputTextLen;
        conDrawInputGlob.inputText = Con_TokenizeInput();
        conDrawInputGlob.inputTextLen = strlen(conDrawInputGlob.inputText);
        conDrawInputGlob.autoCompleteChoice[0] = 0;
        if (inputTextLenPrev != conDrawInputGlob.inputTextLen)
            Con_CancelAutoComplete();
        if (conDrawInputGlob.inputTextLen
            && ((originalCommand = conDrawInputGlob.inputText, Cmd_Argc() <= 1)
                || !Con_IsDvarCommand(conDrawInputGlob.inputText)
                ? (v1 = 0)
                : (v1 = 1),
                !v1
                || (conDrawInputGlob.inputText = Cmd_Argv(1),
                    (conDrawInputGlob.inputTextLen = strlen(conDrawInputGlob.inputText)) != 0)))
        {
            if (con_matchPrefixOnly->current.enabled)
            {
                conDrawInputGlob.hasExactMatch = 0;
                conDrawInputGlob.matchCount = 0;
                con_ignoreMatchPrefixOnly = 1;
                Dvar_ForEachName((void(__cdecl *)(const char *))ConDrawInput_IncrMatchCounter);
                if (!v1)
                    Cmd_ForEach((void(__cdecl *)(const char *))ConDrawInput_IncrMatchCounter);
                if (conDrawInputGlob.matchCount > con_inputMaxMatchesShown)
                {
                    conDrawInputGlob.hasExactMatch = 0;
                    conDrawInputGlob.matchCount = 0;
                    con_ignoreMatchPrefixOnly = 0;
                    Dvar_ForEachName((void(__cdecl *)(const char *))ConDrawInput_IncrMatchCounter);
                    Cmd_ForEach((void(__cdecl *)(const char *))ConDrawInput_IncrMatchCounter);
                    if (!conDrawInputGlob.matchCount)
                    {
                        conDrawInputGlob.hasExactMatch = 0;
                        conDrawInputGlob.matchCount = 0;
                        con_ignoreMatchPrefixOnly = 1;
                        Dvar_ForEachName((void(__cdecl *)(const char *))ConDrawInput_IncrMatchCounter);
                        if (!v1)
                            Cmd_ForEach((void(__cdecl *)(const char *))ConDrawInput_IncrMatchCounter);
                    }
                }
            }
            else
            {
                conDrawInputGlob.hasExactMatch = 0;
                conDrawInputGlob.matchCount = 0;
                con_ignoreMatchPrefixOnly = 0;
                Dvar_ForEachName((void(__cdecl *)(const char *))ConDrawInput_IncrMatchCounter);
                if (!v1)
                    Cmd_ForEach((void(__cdecl *)(const char *))ConDrawInput_IncrMatchCounter);
            }
            matchCount = conDrawInputGlob.matchCount;
            if (conDrawInputGlob.matchCount)
            {
                if (conDrawInputGlob.matchIndex >= conDrawInputGlob.matchCount || !conDrawInputGlob.autoCompleteChoice[0])
                    conDrawInputGlob.matchIndex = -1;
                if (conDrawInputGlob.matchIndex < 0)
                    Con_DrawInputPrompt(localClientNum);
                else
                    Con_DrawAutoCompleteChoice(localClientNum, v1, originalCommand);
                conDrawInputGlob.y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
                conDrawInputGlob.y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
                conDrawInputGlob.x = conDrawInputGlob.leftX;
                if (matchCount <= con_inputMaxMatchesShown)
                {
                    if (matchCount == 1 || conDrawInputGlob.hasExactMatch && Con_AnySpaceAfterCommand())
                    {
                        Dvar_ForEachName((void(__cdecl *)(const char *))ConDrawInput_DetailedDvarMatch);
                        if (!v1)
                            Cmd_ForEach((void(__cdecl *)(const char *))ConDrawInput_DetailedCmdMatch);
                    }
                    else
                    {
                        ConDrawInput_Box(matchCount, &con_inputHintBoxColor->current.value);
                        Dvar_ForEachName((void(__cdecl *)(const char *))ConDrawInput_DvarMatch);
                        if (!v1)
                            Cmd_ForEach((void(__cdecl *)(const char *))ConDrawInput_CmdMatch);
                    }
                }
                else
                {
                    tooManyMatchesStr = va(
                        "%i matches (too many to show here, press shift+tilde to open full console)",
                        matchCount);
                    ConDrawInput_Box(1, &con_inputHintBoxColor->current.value);
                    ConDrawInput_Text(tooManyMatchesStr, con_inputDvarMatchColor);
                }
                Cmd_EndTokenizedString();
            }
            else
            {
                Con_DrawInputPrompt(localClientNum);
                Cmd_EndTokenizedString();
            }
        }
        else
        {
            Con_AllowAutoCompleteCycling(0);
            Con_DrawInputPrompt(localClientNum);
            Cmd_EndTokenizedString();
        }
    }
}

void __cdecl ConDrawInput_Text(char *str, const float *color)
{
    float y; // [esp+1Ch] [ebp-4h]

    if (!str)
        MyAssertHandler(".\\client\\cl_console.cpp", 1645, 0, "%s", "str");
    y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
    R_AddCmdDrawText(str, 0x7FFFFFFF, cls.consoleFont, conDrawInputGlob.x, y, 1.0, 1.0, 0.0, color, 0);
}

void __cdecl ConDrawInput_TextAndOver(char *str, const float *color)
{
    if (!str)
        MyAssertHandler(".\\client\\cl_console.cpp", 1672, 0, "%s", "str");
    ConDrawInput_Text(str, color);
    conDrawInputGlob.x = (double)ConDrawInput_TextWidth(str) + conDrawInputGlob.x;
}

int __cdecl ConDrawInput_TextWidth(const char *text)
{
    if (!text)
        MyAssertHandler(".\\client\\cl_console.cpp", 1638, 0, "%s", "text");
    return R_TextWidth(text, 0, cls.consoleFont);
}

void __cdecl ConDrawInput_Box(int lines, const float *color)
{
    float x; // [esp+14h] [ebp-10h]
    float y; // [esp+18h] [ebp-Ch]
    float h; // [esp+1Ch] [ebp-8h]
    float w; // [esp+20h] [ebp-4h]

    x = conDrawInputGlob.x - 6.0;
    y = conDrawInputGlob.y - 6.0;
    w = con.screenMax[0] - con.screenMin[0] - (x - con.screenMin[0]);
    h = (double)lines * conDrawInputGlob.fontHeight + 6.0 * 2.0;
    ConDraw_Box(x, y, w, h, color);
}

void __cdecl ConDraw_Box(float x, float y, float w, float h, const float *color)
{
    float v5; // [esp+28h] [ebp-18h]
    float v6; // [esp+2Ch] [ebp-14h]
    float darkColor[4]; // [esp+30h] [ebp-10h] BYREF

    R_AddCmdDrawStretchPic(x, y, w, h, 0.0, 0.0, 0.0, 0.0, color, cls.whiteMaterial);
    Vec4Scale(color, 0.5, darkColor);
    darkColor[3] = color[3];
    R_AddCmdDrawStretchPic(x, y, 2.0, h, 0.0, 0.0, 0.0, 0.0, darkColor, cls.whiteMaterial);
    v6 = x + w - 2.0;
    R_AddCmdDrawStretchPic(v6, y, 2.0, h, 0.0, 0.0, 0.0, 0.0, darkColor, cls.whiteMaterial);
    R_AddCmdDrawStretchPic(x, y, w, 2.0, 0.0, 0.0, 0.0, 0.0, darkColor, cls.whiteMaterial);
    v5 = y + h - 2.0;
    R_AddCmdDrawStretchPic(x, v5, w, 2.0, 0.0, 0.0, 0.0, 0.0, darkColor, cls.whiteMaterial);
}

void __cdecl ConDrawInput_IncrMatchCounter(char *str)
{
    if (Con_IsAutoCompleteMatch(str, conDrawInputGlob.inputText, conDrawInputGlob.inputTextLen))
    {
        if (conDrawInputGlob.matchCount == conDrawInputGlob.matchIndex)
            I_strncpyz(conDrawInputGlob.autoCompleteChoice, str, 64);
        ++conDrawInputGlob.matchCount;
        if (!str[conDrawInputGlob.inputTextLen])
            conDrawInputGlob.hasExactMatch = 1;
    }
}

void __cdecl ConDrawInput_DvarMatch(char *str)
{
    char *VariantString; // eax

    if (!str)
        MyAssertHandler(".\\client\\cl_console.cpp", 1874, 0, "%s", "str");
    if (Con_IsAutoCompleteMatch(str, conDrawInputGlob.inputText, conDrawInputGlob.inputTextLen))
    {
        ConDrawInput_TextLimitChars(str, 24, con_inputDvarMatchColor);
        conDrawInputGlob.x = conDrawInputGlob.x + 300.0;
        VariantString = (char *)Dvar_GetVariantString(str);
        ConDrawInput_TextLimitChars(VariantString, 40, con_inputDvarValueColor);
        conDrawInputGlob.y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
        conDrawInputGlob.x = conDrawInputGlob.leftX;
    }
}

void __cdecl ConDrawInput_TextLimitChars(char *str, int maxChars, const float *color)
{
    float y; // [esp+1Ch] [ebp-4h]

    if (!str)
        MyAssertHandler(".\\client\\cl_console.cpp", 1652, 0, "%s", "str");
    y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
    R_AddCmdDrawText(str, maxChars, cls.consoleFont, conDrawInputGlob.x, y, 1.0, 1.0, 0.0, color, 0);
}

void __cdecl ConDrawInput_DetailedDvarMatch(char *str)
{
    char *v1; // eax
    char *v2; // eax
    char *v3; // eax
    __int64 v4; // [esp-Ch] [ebp-428h]
    bool hasLatchedValue; // [esp+7h] [ebp-415h]
    int infoLineCount; // [esp+8h] [ebp-414h] BYREF
    char dvarInfo[1024]; // [esp+Ch] [ebp-410h] BYREF
    int descriptionLineCount; // [esp+410h] [ebp-Ch]
    const dvar_s *dvar; // [esp+414h] [ebp-8h]
    int lineIndex; // [esp+418h] [ebp-4h]

    if (!str)
        MyAssertHandler(".\\client\\cl_console.cpp", 1999, 0, "%s", "str");
    if (Con_IsAutoCompleteMatch(str, conDrawInputGlob.inputText, conDrawInputGlob.inputTextLen)
        && (!conDrawInputGlob.hasExactMatch || !str[conDrawInputGlob.inputTextLen]))
    {
        dvar = Dvar_FindVar(str);
        if (!dvar)
            MyAssertHandler(".\\client\\cl_console.cpp", 2007, 0, "%s", "dvar");
        hasLatchedValue = Dvar_HasLatchedValue(dvar);
        if (hasLatchedValue)
            ConDrawInput_Box(3, &con_inputHintBoxColor->current.value);
        else
            ConDrawInput_Box(2, &con_inputHintBoxColor->current.value);
        ConDrawInput_TextLimitChars(str, 24, con_inputDvarMatchColor);
        conDrawInputGlob.x = conDrawInputGlob.x + 300.0;
        v1 = (char *)Dvar_DisplayableValue(dvar);
        ConDrawInput_TextLimitChars(v1, 40, con_inputDvarValueColor);
        conDrawInputGlob.y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
        conDrawInputGlob.x = conDrawInputGlob.leftX;
        if (hasLatchedValue)
        {
            ConDrawInput_Text((char*)"  latched value", con_inputDvarInactiveValueColor);
            conDrawInputGlob.x = conDrawInputGlob.x + 300.0;
            v2 = (char *)Dvar_DisplayableLatchedValue(dvar);
            ConDrawInput_TextLimitChars(v2, 40, con_inputDvarInactiveValueColor);
            conDrawInputGlob.y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
            conDrawInputGlob.x = conDrawInputGlob.leftX;
        }
        ConDrawInput_Text((char*)"  default", con_inputDvarInactiveValueColor);
        conDrawInputGlob.x = conDrawInputGlob.x + 300.0;
        v3 = (char *)Dvar_DisplayableResetValue(dvar);
        ConDrawInput_TextLimitChars(v3, 40, con_inputDvarInactiveValueColor);
        conDrawInputGlob.y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
        conDrawInputGlob.y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
        conDrawInputGlob.x = conDrawInputGlob.leftX;
        Dvar_DomainToString_GetLines(dvar->type, (DvarLimits*)&dvar->domain, dvarInfo, 1024, &infoLineCount);
        if (dvar->description)
            descriptionLineCount = ConDrawInput_GetDvarDescriptionLines(dvar);
        else
            descriptionLineCount = 0;
        ConDrawInput_Box(descriptionLineCount + infoLineCount + 1, &con_inputHintBoxColor->current.value);
        if (dvar->description)
        {
            ConDrawInput_Text((char *)dvar->description, con_inputDvarDescriptionColor);
            for (lineIndex = 0; lineIndex < descriptionLineCount; ++lineIndex)
            {
                conDrawInputGlob.y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
                conDrawInputGlob.x = conDrawInputGlob.leftX;
            }
        }
        ConDrawInput_Text(dvarInfo, con_inputDvarInfoColor);
        conDrawInputGlob.y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
        conDrawInputGlob.x = conDrawInputGlob.leftX;
        if (dvar->type == 6 && Cmd_Argc() == 2)
            ConDrawInput_AutoCompleteArg(dvar->domain.enumeration.strings, dvar->domain.enumeration.stringCount);
    }
}

void __cdecl ConDrawInput_AutoCompleteArg(const char **stringList, int stringCount)
{
    int ArgChar; // eax
    Font_s *consoleFont; // [esp+10h] [ebp-4B0h]
    int matchIndex; // [esp+48h] [ebp-478h]
    signed int matchCount; // [esp+4Ch] [ebp-474h]
    char matchBuffer[1024]; // [esp+50h] [ebp-470h] BYREF
    int prefixLen; // [esp+458h] [ebp-68h]
    int matchLenMax; // [esp+45Ch] [ebp-64h]
    int matchLen; // [esp+460h] [ebp-60h]
    const char *prefix; // [esp+464h] [ebp-5Ch]
    char *matches[16]; // [esp+468h] [ebp-58h] BYREF
    int matchBufferUsed; // [esp+4A8h] [ebp-18h]
    int stringIndex; // [esp+4ACh] [ebp-14h]
    float x; // [esp+4B0h] [ebp-10h]
    float y; // [esp+4B4h] [ebp-Ch]
    float h; // [esp+4B8h] [ebp-8h]
    float w; // [esp+4BCh] [ebp-4h]

    prefix = Cmd_Argv(1);
    prefixLen = strlen(prefix);
    if (prefixLen)
    {
        matchCount = 0;
        matchBufferUsed = 0;
        matchLenMax = 0;
        for (stringIndex = 0; stringIndex < stringCount; ++stringIndex)
        {
            if (!I_strnicmp(prefix, stringList[stringIndex], prefixLen))
            {
                if (matchCount == 16 || matchBufferUsed + strlen(stringList[stringIndex]) >= 0x400)
                    return;
                matches[matchCount] = &matchBuffer[matchBufferUsed];
                Com_StripExtension((char *)stringList[stringIndex], matches[matchCount]);
                matchBufferUsed += strlen(matches[matchCount]) + 1;
                matchLen = ConDrawInput_TextWidth(matches[matchCount]);
                if (matchLenMax < matchLen)
                    matchLenMax = matchLen;
                ++matchCount;
            }
        }
        if (matchCount)
        {
            qsort(matches, matchCount, 4u, (int(__cdecl *)(const void *, const void *))ConDrawInput_CompareStrings);
            consoleFont = cls.consoleFont;
            ArgChar = ConDrawInput_TextFieldFirstArgChar();
            x = (double)R_TextWidth(g_consoleField.buffer, ArgChar, consoleFont) + conDrawInputGlob.leftX - 6.0;
            y = con.screenMin[1] + conDrawInputGlob.fontHeight + 6.0;
            w = (double)matchLenMax + 6.0 + 6.0;
            h = (double)matchCount * conDrawInputGlob.fontHeight + 6.0 + 6.0;
            ConDraw_Box(x, y, w, h, &con_inputHintBoxColor->current.value);
            conDrawInputGlob.x = x + 6.0;
            conDrawInputGlob.y = y + 6.0;
            conDrawInputGlob.leftX = conDrawInputGlob.x;
            for (matchIndex = 0; matchIndex < matchCount; ++matchIndex)
            {
                ConDrawInput_Text(matches[matchIndex], con_inputDvarInfoColor);
                conDrawInputGlob.y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
                conDrawInputGlob.x = conDrawInputGlob.leftX;
            }
        }
    }
}

int __cdecl ConDrawInput_CompareStrings(const char **e0, const char **e1)
{
    return I_stricmp(*e0, *e1);
}

int __cdecl ConDrawInput_TextFieldFirstArgChar()
{
    int charIndex; // [esp+0h] [ebp-4h]

    for (charIndex = 0; isspace(g_consoleField.buffer[charIndex]); ++charIndex)
        ;
    while (!isspace(g_consoleField.buffer[charIndex]))
        ++charIndex;
    while (isspace(g_consoleField.buffer[charIndex]))
        ++charIndex;
    return charIndex;
}

int __cdecl ConDrawInput_GetDvarDescriptionLines(const dvar_s *dvar)
{
    signed int v1; // kr00_4
    int linecount; // [esp+10h] [ebp-Ch]
    int index; // [esp+14h] [ebp-8h]

    if (!dvar->description)
        MyAssertHandler(".\\client\\cl_console.cpp", 1978, 0, "%s", "dvar->description");
    v1 = strlen(dvar->description);
    linecount = 1;
    for (index = 0; index < v1; ++index)
    {
        if (dvar->description[index] == 10)
            ++linecount;
    }
    return linecount;
}

void __cdecl ConDrawInput_DetailedCmdMatch(char *str)
{
    int fileCount; // [esp+0h] [ebp-8h] BYREF
    const char **files; // [esp+4h] [ebp-4h]

    if (!str)
        MyAssertHandler(".\\client\\cl_console.cpp", 2068, 0, "%s", "str");
    if (Con_IsAutoCompleteMatch(str, conDrawInputGlob.inputText, conDrawInputGlob.inputTextLen)
        && (!conDrawInputGlob.hasExactMatch || !str[conDrawInputGlob.inputTextLen]))
    {
        ConDrawInput_Box(1, &con_inputHintBoxColor->current.value);
        ConDrawInput_Text(str, con_inputCommandMatchColor);
        conDrawInputGlob.y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
        conDrawInputGlob.x = conDrawInputGlob.leftX;
        if (Cmd_Argc() == 2)
        {
            files = Cmd_GetAutoCompleteFileList(str, &fileCount);
            if (fileCount)
            {
                ConDrawInput_AutoCompleteArg(files, fileCount);
                FS_FreeFileList(files);
            }
        }
    }
}

void __cdecl ConDrawInput_CmdMatch(char *str)
{
    if (!str)
        MyAssertHandler(".\\client\\cl_console.cpp", 2095, 0, "%s", "str");
    if (Con_IsAutoCompleteMatch(str, conDrawInputGlob.inputText, conDrawInputGlob.inputTextLen))
    {
        ConDrawInput_Text(str, con_inputCommandMatchColor);
        conDrawInputGlob.y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
        conDrawInputGlob.x = conDrawInputGlob.leftX;
    }
}

void __cdecl Con_DrawAutoCompleteChoice(int localClientNum, bool isDvarCommand, const char *originalCommand)
{
    int drawLen; // [esp+10h] [ebp-114h]
    char colorCodedLine[256]; // [esp+14h] [ebp-110h] BYREF
    int cursorPos; // [esp+118h] [ebp-Ch]
    int x; // [esp+11Ch] [ebp-8h]
    int y; // [esp+120h] [ebp-4h]

    cursorPos = Con_GetAutoCompleteColorCodedString(
        conDrawInputGlob.autoCompleteChoice,
        conDrawInputGlob.inputText,
        conDrawInputGlob.inputTextLen,
        isDvarCommand,
        originalCommand,
        colorCodedLine);
    x = (int)conDrawInputGlob.x;
    y = (int)conDrawInputGlob.y;
    if (&colorCodedLine[strlen(colorCodedLine) + 1] == &colorCodedLine[1])
        MyAssertHandler(".\\client\\cl_console.cpp", 2117, 0, "%s", "strlen( colorCodedLine ) > 0");
    drawLen = SEH_PrintStrlen(colorCodedLine);
    if (drawLen <= 0)
        MyAssertHandler(
            ".\\client\\cl_console.cpp",
            2119,
            0,
            "%s\n\t(colorCodedLine) = %s",
            "(drawLen > 0)",
            colorCodedLine);
    Field_DrawTextOverride(localClientNum, &g_consoleField, x, y, 5, 5, colorCodedLine, drawLen, cursorPos);
}

unsigned int __cdecl Con_GetAutoCompleteColorCodedString(
    char *query,
    const char *matchToText,
    int matchTextLen,
    bool isDvarCommand,
    const char *originalCommand,
    char *colorCoded)
{
    unsigned int prefixLen; // [esp+0h] [ebp-4h]

    if (isDvarCommand)
        prefixLen = sprintf(colorCoded, "^2%s ", originalCommand);
    else
        prefixLen = 0;
    if (con_matchPrefixOnly->current.enabled)
        return prefixLen
        + Con_GetAutoCompleteColorCodedStringContiguous(query, matchToText, matchTextLen, &colorCoded[prefixLen]);
    else
        return prefixLen
        + Con_GetAutoCompleteColorCodedStringDiscontiguous(query, matchToText, matchTextLen, &colorCoded[prefixLen]);
}

int __cdecl Con_GetAutoCompleteColorCodedStringDiscontiguous(
    const char *query,
    const char *matchToText,
    int matchTextLen,
    char *colorCoded)
{
    int v4; // eax
    char v6; // [esp+13h] [ebp-29h]
    char *v7; // [esp+18h] [ebp-24h]
    const char *v8; // [esp+1Ch] [ebp-20h]
    const char *v9; // [esp+20h] [ebp-1Ch]
    int matchLetter; // [esp+24h] [ebp-18h]
    int colorCodedPos; // [esp+28h] [ebp-14h]
    int colorCodedPosb; // [esp+28h] [ebp-14h]
    int colorCodedPosa; // [esp+28h] [ebp-14h]
    char wasMatching; // [esp+2Fh] [ebp-Dh]
    int matchTextPos; // [esp+30h] [ebp-Ch]
    const char *queryPos; // [esp+34h] [ebp-8h]
    char isMatching; // [esp+3Bh] [ebp-1h]

    if (!query)
        MyAssertHandler(".\\client\\cl_console.cpp", 1721, 0, "%s", "query");
    if (!matchToText)
        MyAssertHandler(".\\client\\cl_console.cpp", 1722, 0, "%s", "matchToText");
    if (!matchTextLen)
        MyAssertHandler(".\\client\\cl_console.cpp", 1723, 0, "%s", "matchTextLen");
    wasMatching = 0;
    matchTextPos = 0;
    colorCodedPos = 0;
    matchLetter = tolower(*matchToText);
    for (queryPos = query; *queryPos; ++queryPos)
    {
        v4 = tolower(*queryPos);
        isMatching = v4 == matchLetter;
        if (v4 == matchLetter)
            matchLetter = tolower(matchToText[++matchTextPos]);
        if (isMatching != wasMatching)
        {
            wasMatching = isMatching;
            if (isMatching)
                v9 = "^2";
            else
                v9 = "^7";
            v8 = v9;
            v7 = &colorCoded[colorCodedPos];
            do
            {
                v6 = *v8;
                *v7++ = *v8++;
            } while (v6);
            colorCodedPos += 2;
        }
        colorCoded[colorCodedPos++] = *queryPos;
    }
    strcpy(&colorCoded[colorCodedPos], "^7");
    colorCodedPosb = colorCodedPos + 2;
    colorCoded[colorCodedPosb] = 32;
    colorCodedPosa = colorCodedPosb + 1;
    colorCoded[colorCodedPosa] = 0;
    if (!strlen(colorCoded))
        MyAssertHandler(".\\client\\cl_console.cpp", 1754, 0, "%s", "strlen( colorCoded ) > 0");
    return colorCodedPosa;
}

int __cdecl Con_GetAutoCompleteColorCodedStringContiguous(
    char *query,
    const char *matchToText,
    int matchTextLen,
    char *colorCoded)
{
    char v5; // [esp+13h] [ebp-59h]
    char *v6; // [esp+18h] [ebp-54h]
    char *v7; // [esp+1Ch] [ebp-50h]
    char v8; // [esp+43h] [ebp-29h]
    char *v9; // [esp+48h] [ebp-24h]
    char *v10; // [esp+4Ch] [ebp-20h]
    unsigned int v11; // [esp+50h] [ebp-1Ch]
    int colorCodedPos; // [esp+60h] [ebp-Ch]
    int colorCodedPosb; // [esp+60h] [ebp-Ch]
    int colorCodedPosc; // [esp+60h] [ebp-Ch]
    int colorCodedPosa; // [esp+60h] [ebp-Ch]
    int colorCodedPosd; // [esp+60h] [ebp-Ch]
    char *queryPos; // [esp+64h] [ebp-8h]

    if (!query)
        MyAssertHandler(".\\client\\cl_console.cpp", 1765, 0, "%s", "query");
    if (!matchToText)
        MyAssertHandler(".\\client\\cl_console.cpp", 1766, 0, "%s", "matchToText");
    if (!matchTextLen)
        MyAssertHandler(".\\client\\cl_console.cpp", 1767, 0, "%s", "matchTextLen");
    colorCodedPos = 0;
    queryPos = (char *)I_stristr(query, matchToText);
    if (queryPos)
    {
        strncpy((unsigned __int8 *)colorCoded, (unsigned __int8 *)query, queryPos - query);
        strcpy(&colorCoded[queryPos - query], "^2");
        colorCodedPosb = queryPos - query + 2;
        v11 = strlen(matchToText);
        strncpy((unsigned __int8 *)&colorCoded[colorCodedPosb], (unsigned __int8 *)queryPos, v11);
        colorCodedPosc = v11 + colorCodedPosb;
        strcpy(&colorCoded[colorCodedPosc], "^7");
        colorCodedPosa = colorCodedPosc + 2;
        v10 = &queryPos[v11];
        v9 = &colorCoded[colorCodedPosa];
        do
        {
            v8 = *v10;
            *v9++ = *v10++;
        } while (v8);
        colorCodedPosd = strlen(&queryPos[v11]) + colorCodedPosa;
        colorCoded[colorCodedPosd] = 32;
        colorCodedPos = colorCodedPosd + 1;
        colorCoded[colorCodedPos] = 0;
    }
    else
    {
        if (!strlen(query))
            MyAssertHandler(".\\client\\cl_console.cpp", 1792, 0, "%s", "strlen( query ) > 0");
        v7 = query;
        v6 = colorCoded;
        do
        {
            v5 = *v7;
            *v6++ = *v7++;
        } while (v5);
    }
    if (!strlen(colorCoded))
        MyAssertHandler(".\\client\\cl_console.cpp", 1796, 0, "%s", "strlen( colorCoded ) > 0");
    return colorCodedPos;
}

void __cdecl Con_DrawInputPrompt(int localClientNum)
{
    Field_Draw(localClientNum, &g_consoleField, (int)conDrawInputGlob.x, (int)conDrawInputGlob.y, 5, 5);
}

void Con_DrawOuputWindow()
{
    float width; // [esp+14h] [ebp-10h]
    float widtha; // [esp+14h] [ebp-10h]
    float height; // [esp+18h] [ebp-Ch]
    float heighta; // [esp+18h] [ebp-Ch]
    float heightb; // [esp+18h] [ebp-Ch]
    float x; // [esp+1Ch] [ebp-8h]
    float xa; // [esp+1Ch] [ebp-8h]
    float y; // [esp+20h] [ebp-4h]
    float ya; // [esp+20h] [ebp-4h]

    x = con.screenMin[0];
    width = con.screenMax[0] - con.screenMin[0];
    height = con.screenMax[1] - con.screenMin[1];
    y = con.screenMin[1] + 32.0;
    heighta = height - 32.0;
    ConDraw_Box(con.screenMin[0], y, width, heighta, &con_outputWindowColor->current.value);
    xa = x + 6.0;
    ya = y + 6.0;
    widtha = width - (6.0 + 6.0);
    heightb = heighta - (6.0 + 6.0);
    Con_DrawOutputVersion(xa, ya, widtha, heightb);
    Con_DrawOutputScrollBar(xa, ya, widtha, heightb);
    Con_DrawOutputText(xa, ya);
}

void __cdecl Con_DrawOutputScrollBar(float x, float y, float width, float height)
{
    float h; // [esp+14h] [ebp-3Ch]
    float v5; // [esp+18h] [ebp-38h]
    float v6; // [esp+1Ch] [ebp-34h]
    float v7; // [esp+20h] [ebp-30h]
    float v8; // [esp+24h] [ebp-2Ch]
    float v9; // [esp+28h] [ebp-28h]
    float v10; // [esp+38h] [ebp-18h]
    float v11; // [esp+3Ch] [ebp-14h]
    float v12; // [esp+40h] [ebp-10h]
    float scale; // [esp+48h] [ebp-8h]
    float xa; // [esp+58h] [ebp+8h]
    float ya; // [esp+5Ch] [ebp+Ch]

    xa = x + width - 10.0;
    ConDraw_Box(xa, y, 10.0, height, &con_outputBarColor->current.value);
    if (con.consoleWindow.activeLineCount > con.visibleLineCount)
    {
        scale = 1.0 / (double)(con.consoleWindow.activeLineCount - con.visibleLineCount);
        v11 = scale * (double)(con.displayLineOffset - con.visibleLineCount);
        v9 = v11 - 1.0;
        if (v9 < 0.0)
            v12 = scale * (double)(con.displayLineOffset - con.visibleLineCount);
        else
            v12 = 1.0;
        v8 = 0.0 - v11;
        if (v8 < 0.0)
            v7 = v12;
        else
            v7 = 0.0;
        v10 = height * (scale * (double)con.visibleLineCount);
        v6 = ceil(v10);
        v5 = 10.0 - v6;
        if (v5 < 0.0)
            h = v6;
        else
            h = 10.0;
        ya = (y + height - h - y) * v7 + y;
        ConDraw_Box(xa, ya, 10.0, h, &con_outputSliderColor->current.value);
    }
    else
    {
        ConDraw_Box(xa, y, 10.0, height, &con_outputSliderColor->current.value);
    }
}

void __cdecl Con_DrawOutputText(float x, float y)
{
    int rowCount; // [esp+1Ch] [ebp-24h]
    int firstRow; // [esp+20h] [ebp-20h]
    float color[4]; // [esp+28h] [ebp-18h] BYREF
    int lineIndex; // [esp+38h] [ebp-8h]
    int rowIndex; // [esp+3Ch] [ebp-4h]

    CL_LookupColor(0, 0x37u, color);
    if (!con.fontHeight)
        MyAssertHandler(".\\client\\cl_console.cpp", 2877, 0, "%s", "con.fontHeight");
    rowCount = con.visibleLineCount;
    firstRow = con.displayLineOffset - con.visibleLineCount;
    if (con.displayLineOffset - con.visibleLineCount < 0)
    {
        y = y - (double)(con.fontHeight * firstRow);
        rowCount = con.displayLineOffset;
        firstRow = 0;
    }
    for (rowIndex = 0; rowIndex < rowCount; ++rowIndex)
    {
        lineIndex = (rowIndex + firstRow + con.consoleWindow.firstLineIndex) % con.consoleWindow.lineCount;
        y = (double)con.fontHeight + y;
        R_AddCmdDrawConsoleText(
            con.consoleWindow.circularTextBuffer,
            con.consoleWindow.textBufSize,
            con.consoleWindow.lines[lineIndex].textBufPos,
            con.consoleWindow.lines[lineIndex].textBufSize,
            cls.consoleFont,
            x,
            y,
            1.0,
            1.0,
            color,
            0);
    }
}

void __cdecl Con_DrawOutputVersion(float x, float y, float width, float height)
{
    char *VersionString; // [esp-8h] [ebp-Ch]
    float ya; // [esp+10h] [ebp+Ch]

    ya = height - 16.0 + y;
    VersionString = Con_GetVersionString();
    SCR_DrawSmallStringExt((int)x, (int)ya, VersionString, con_versionColor);
}

char *__cdecl Con_GetVersionString()
{
    const char *BuildNumber; // eax

    BuildNumber = getBuildNumber();
    return va("Build %s %s", BuildNumber, "win-x86");
}

void __cdecl Con_PageUp()
{
    int activeLineCount; // [esp+0h] [ebp-4h]

    con.displayLineOffset -= 2;
    if (con.displayLineOffset < con.visibleLineCount)
    {
        if (con.consoleWindow.activeLineCount < con.visibleLineCount)
            activeLineCount = con.consoleWindow.activeLineCount;
        else
            activeLineCount = con.visibleLineCount;
        con.displayLineOffset = activeLineCount;
    }
}

void __cdecl Con_PageDown()
{
    int activeLineCount; // [esp+0h] [ebp-8h]

    if (con.displayLineOffset + 2 < con.consoleWindow.activeLineCount)
        activeLineCount = con.displayLineOffset + 2;
    else
        activeLineCount = con.consoleWindow.activeLineCount;
    con.displayLineOffset = activeLineCount;
}

void __cdecl Con_Top()
{
    int activeLineCount; // [esp+0h] [ebp-4h]

    if (con.consoleWindow.activeLineCount < con.visibleLineCount)
        activeLineCount = con.consoleWindow.activeLineCount;
    else
        activeLineCount = con.visibleLineCount;
    con.displayLineOffset = activeLineCount;
}

void __cdecl Con_Bottom()
{
    con.displayLineOffset = con.consoleWindow.activeLineCount;
}

void __cdecl Con_Close(int localClientNum)
{
    int client; // [esp+0h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client\\../client_mp/client_mp.h",
            1063,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].isRunning)
    {
        Field_Clear(&g_consoleField);
        Con_CancelAutoComplete();
        Con_ClearNotify(localClientNum);
        Con_ClearMiniConsole(localClientNum);
        Con_ClearErrors(localClientNum);
        for (client = 0; client < 1; ++client)
            clientUIActives[client].keyCatchers &= ~1u;
    }
}

bool __cdecl Con_IsActive(int localClientNum)
{
    return Key_IsCatcherActive(localClientNum, 1);
}

void __cdecl CL_PlayTextFXPulseSounds(
    unsigned int localClientNum,
    int currentTime,
    int strLength,
    int fxBirthTime,
    int fxLetterTime,
    int fxDecayStartTime,
    int *soundTimeKeeper)
{
    int timeElapsed; // [esp+8h] [ebp-Ch]
    int lastSoundTime; // [esp+Ch] [ebp-8h]
    int decayStartTime; // [esp+10h] [ebp-4h]

    timeElapsed = currentTime - fxBirthTime;
    lastSoundTime = *soundTimeKeeper - fxBirthTime;
    decayStartTime = fxDecayStartTime;
    if (fxDecayStartTime < fxLetterTime * strLength)
        decayStartTime = fxLetterTime * strLength;
    if (timeElapsed >= 0)
    {
        if (timeElapsed <= decayStartTime)
        {
            if (timeElapsed < fxLetterTime * strLength)
            {
                if (!fxLetterTime)
                    MyAssertHandler(".\\client\\cl_console.cpp", 3070, 0, "%s", "fxLetterTime");
                if (lastSoundTime < fxLetterTime * (timeElapsed / fxLetterTime))
                {
                    SND_PlayLocalSoundAliasByName(localClientNum, "ui_pulse_text_type", SASYS_CGAME);
                    *soundTimeKeeper = currentTime;
                }
            }
        }
        else if (lastSoundTime < decayStartTime)
        {
            SND_PlayLocalSoundAliasByName(localClientNum, "ui_pulse_text_delete", SASYS_CGAME);
            *soundTimeKeeper = currentTime;
        }
    }
}

