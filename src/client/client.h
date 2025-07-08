#pragma once
#include <qcommon/qcommon.h>

#include <ui/ui_shared.h>
#ifdef KISAK_MP
#include <qcommon/msg_mp.h>
#elif KISAK_SP
#include <qcommon/msg.h>
#endif

enum msgwnd_mode_t : int32_t
{                                       // ...
    MWM_BOTTOMUP_ALIGN_TOP = 0x0,
    MWM_BOTTOMUP_ALIGN_BOTTOM = 0x1,
    MWM_TOPDOWN_ALIGN_TOP = 0x2,
    MWM_TOPDOWN_ALIGN_BOTTOM = 0x3,
};

enum print_msg_dest_t : int32_t
{                                       // ...
    CON_DEST_CONSOLE = 0x0,
    CON_DEST_MINICON = 0x1,
    CON_DEST_ERROR = 0x2,
    CON_DEST_GAME_FIRST = 0x3,
    CON_DEST_GAME1 = 0x3,
    CON_DEST_GAME2 = 0x4,
    CON_DEST_GAME3 = 0x5,
    CON_DEST_GAME4 = 0x6,
    CON_DEST_GAME_LAST = 0x6,
    CON_DEST_COUNT = 0x7,
};
inline print_msg_dest_t &operator++(print_msg_dest_t &e) {
    e = static_cast<print_msg_dest_t>(static_cast<int>(e) + 1);
    return e;
}
inline print_msg_dest_t &operator++(print_msg_dest_t &e, int32_t i)
{
    ++e;
    return e;
}

struct ConDrawInputGlob // sizeof=0x64
{                                       // ...
    char autoCompleteChoice[64];        // ...
    int32_t matchIndex;                     // ...
    int32_t matchCount;                     // ...
    const char *inputText;              // ...
    int32_t inputTextLen;                   // ...
    bool hasExactMatch;                 // ...
    bool mayAutoComplete;               // ...
    // padding byte
    // padding byte
    float x;                            // ...
    float y;                            // ...
    float leftX;                        // ...
    float fontHeight;                   // ...
};
struct MessageLine // sizeof=0x18
{                                       // ...
    int32_t messageIndex;
    int32_t textBufPos;
    int32_t textBufSize;
    int32_t typingStartTime;
    int32_t lastTypingSoundTime;
    int32_t flags;
};
struct Message // sizeof=0x8
{                                       // ...
    int32_t startTime;
    int32_t endTime;
};
struct MessageWindow // sizeof=0x34
{                                       // ...
    MessageLine *lines;                 // ...
    Message *messages;
    char *circularTextBuffer;           // ...
    int32_t textBufSize;                    // ...
    int32_t lineCount;                      // ...
    int32_t padding;
    int32_t scrollTime;
    int32_t fadeIn;
    int32_t fadeOut;
    int32_t textBufPos;                     // ...
    int32_t firstLineIndex;                 // ...
    int32_t activeLineCount;                // ...
    int32_t messageIndex;
};
struct MessageBuffer // sizeof=0x4858
{                                       // ...
    char gamemsgText[4][2048];
    MessageWindow gamemsgWindows[4];    // ...
    MessageLine gamemsgLines[4][12];
    Message gamemsgMessages[4][12];
    char miniconText[4096];
    MessageWindow miniconWindow;        // ...
    MessageLine miniconLines[100];
    Message miniconMessages[100];
    char errorText[1024];
    MessageWindow errorWindow;          // ...
    MessageLine errorLines[5];
    Message errorMessages[5];
};
struct Console // sizeof=0x14ACC
{                                       // ...
    int32_t initialized;                    // ...
    MessageWindow consoleWindow;        // ...
    MessageLine consoleLines[1024];     // ...
    Message consoleMessages[1024];      // ...
    char consoleText[32768];            // ...
    char textTempLine[512];             // ...
    uint32_t lineOffset;            // ...
    int32_t displayLineOffset;              // ...
    int32_t prevChannel;                    // ...
    bool outputVisible;                 // ...
    // padding byte
    // padding byte
    // padding byte
    int32_t fontHeight;                     // ...
    int32_t visibleLineCount;               // ...
    int32_t visiblePixelWidth;              // ...
    float screenMin[2];                 // ...
    float screenMax[2];                 // ...
    MessageBuffer messageBuffer[1];     // ...
    float color[4];                     // ...
};

// cl_console
void __cdecl TRACK_cl_console();
void __cdecl Con_ToggleConsole();
void __cdecl Con_OpenConsole(int32_t localClientNum);
void __cdecl Con_OpenConsoleOutput(int32_t localClientNum);
void __cdecl Con_CloseConsole(int32_t localClientNum);
void __cdecl Con_GetTextCopy(char *text, int32_t maxSize);
void __cdecl Con_TimeJumped(int32_t localClientNum, int32_t serverTime);
void __cdecl Con_ResetMessageWindowTimes(MessageWindow *msgwnd, int32_t serverTime);
void __cdecl Con_TimeNudged(int32_t localClientNum, int32_t serverTimeNudge);
void __cdecl Con_NudgeMessageWindowTimes(MessageWindow *msgwnd, int32_t serverTimeNudge, int32_t serverTime);
void __cdecl Con_ClearNotify(int32_t localClientNum);
void __cdecl Con_ClearMessageWindow(MessageWindow *msgwnd);
void __cdecl Con_ClearErrors(int32_t localClientNum);
void __cdecl Con_CheckResize();
void __cdecl Con_Init();
void __cdecl Con_ChatModePublic_f();
void __cdecl Con_ChatModeTeam_f();
void __cdecl Con_Clear_f();
void __cdecl Con_InitClientAssets();
void __cdecl Con_InitMessageBuffer();
void __cdecl Con_InitMessageWindow(
    MessageWindow *msgwnd,
    Message *messages,
    MessageLine *lines,
    char *text,
    int32_t lineCount,
    int32_t padding,
    int32_t textPoolSize,
    int32_t scrollTime,
    int32_t fadeIn,
    int32_t fadeOut);
void __cdecl CL_ConsolePrint(int32_t localClientNum, int32_t channel, const char *txt, int32_t duration, int32_t pixelWidth, int32_t flags);
void Con_OneTimeInit();
char __cdecl CL_ConsolePrint_AddLine(
    int32_t localClientNum,
    int32_t channel,
    const char *txt,
    int32_t duration,
    int32_t pixelWidth,
    char color,
    int32_t flags);
void __cdecl Con_UpdateNotifyMessage(int32_t localClientNum, uint32_t channel, int32_t duration, int32_t flags);
void __cdecl Con_UpdateNotifyMessageWindow(
    int32_t localClientNum,
    uint32_t channel,
    int32_t duration,
    int32_t flags,
    print_msg_dest_t dest);
int32_t __cdecl Con_GetDefaultMsgDuration(print_msg_dest_t dest);
void __cdecl Con_UpdateMessage(int32_t localClientNum, MessageWindow *msgwnd, int32_t duration);
MessageWindow *__cdecl Con_GetDestWindow(int32_t localClientNum, print_msg_dest_t dest);
void __cdecl Con_UpdateNotifyLine(int32_t localClientNum, uint32_t channel, bool lineFeed, int32_t flags);
void __cdecl Con_UpdateNotifyLineWindow(
    int32_t localClientNum,
    uint32_t channel,
    bool lineFeed,
    int32_t flags,
    print_msg_dest_t dest);
void __cdecl Con_UpdateMessageWindowLine(int32_t localClientNum, MessageWindow *msgwnd, int32_t linefeed, int32_t flags);
void __cdecl Con_FreeFirstMessageWindowLine(MessageWindow *msgwnd);
void __cdecl Con_CopyCurrentConsoleLineText(MessageWindow *msgwnd, MessageLine *msgLine);
bool __cdecl Con_NeedToFreeMessageWindowLine(MessageWindow *msgwnd, int32_t charCount);
int32_t __cdecl PrintTimeTotal(MessageWindow *msgwnd, MessageLine *line);
int32_t __cdecl PrintableCharsCount(const MessageWindow *msgwnd, MessageLine *line);
int32_t __cdecl GetNextValidPrintTimeForLine(int32_t localClientNum, MessageWindow *msgwnd, char flags);
int32_t __cdecl LatestActiveTypewrittenLineIdx(MessageWindow *msgwnd);
int32_t __cdecl PrintTimeWriteOut(MessageWindow *msgwnd, MessageLine *line);
void __cdecl Con_Linefeed(int32_t localClientNum, uint32_t channel, int32_t flags);
void __cdecl CL_ConsoleFixPosition();
void __cdecl CL_DeathMessagePrint(
    int32_t localClientNum,
    char *attackerName,
    char attackerColorIndex,
    char *victimName,
    char victimColorIndex,
    struct Material *iconShader,
    float iconWidth,
    float iconHeight,
    bool horzFlipIcon);
uint32_t __cdecl CL_AddDeathMessageString(
    char *deathMsg,
    uint32_t deathMsgLen,
    uint32_t deathMsgMaxLen,
    char *string);
uint32_t __cdecl CL_AddDeathMessageIcon(
    char *deathMsg,
    uint32_t deathMsgLen,
    uint32_t deathMsgMaxLen,
    struct Material *iconShader,
    float iconWidth,
    float iconHeight,
    bool horzFlipIcon);
int32_t __cdecl CL_DeathMessageIconDimension(float size);
void __cdecl Con_AutoCompleteFromList(
    const char **strings,
    uint32_t stringCount,
    const char *prefix,
    char *completed,
    uint32_t sizeofCompleted);
const char *__cdecl Con_TokenizeInput();
char __cdecl Con_AnySpaceAfterCommand();
bool __cdecl Con_IsAutoCompleteMatch(const char *query, const char *matchToText, int32_t matchTextLen);
bool __cdecl Con_HasTooManyMatchesToShow();
bool __cdecl Con_IsDvarCommand(const char *cmd);
char __cdecl Con_CycleAutoComplete(int32_t step);
bool __cdecl Con_HasActiveAutoComplete();
char __cdecl Con_CommitToAutoComplete();
char __cdecl Con_CancelAutoComplete();
void __cdecl Con_AllowAutoCompleteCycling(bool isAllowed);
void __cdecl Con_DrawGameMessageWindow(
    int32_t localClientNum,
    uint32_t windowIndex,
    int32_t xPos,
    int32_t yPos,
    int32_t horzAlign,
    int32_t vertAlign,
    struct Font_s *font,
    float fontScale,
    float *color,
    int32_t textStyle,
    char textAlignMode,
    msgwnd_mode_t mode);
void __cdecl Con_DrawMessageWindow(
    int32_t localClientNum,
    MessageWindow *msgwnd,
    int32_t x,
    int32_t y,
    int32_t charHeight,
    int32_t horzAlign,
    int32_t vertAlign,
    struct Font_s *font,
    float *color,
    int32_t textStyle,
    float msgwndScale,
    msgwnd_mode_t mode,
    char textAlignMode);
void __cdecl Con_DrawMessageWindowNewToOld(
    int32_t localClientNum,
    MessageWindow *msgwnd,
    int32_t x,
    int32_t y,
    int32_t hudCharHeight,
    int32_t horzAlign,
    int32_t vertAlign,
    bool up,
    struct Font_s *font,
    float *color,
    int32_t textStyle,
    float msgwndScale,
    char textAlignMode);
void __cdecl Con_DrawMessageLineOnHUD(
    int32_t localClientNum,
    const ScreenPlacement *scrPlace,
    int32_t x,
    int32_t y,
    int32_t charHeight,
    int32_t horzAlign,
    int32_t vertAlign,
    struct Font_s *font,
    const MessageWindow *msgwnd,
    int32_t lineIdx,
    float *color,
    int32_t textStyle,
    float msgwndScale,
    char textAlignMode);
bool __cdecl LineVisible(const MessageWindow *msgwnd, int32_t lineIdx, int32_t time);
void __cdecl TypewriterSounds(int32_t localClientNum, const MessageWindow *msgwnd, MessageLine *line);
void __cdecl Con_CullFinishedLines(int32_t serverTime, MessageWindow *msgwnd);
double __cdecl Con_GetMessageAlpha(Message *message, MessageWindow *msgwnd, int32_t serverTime, bool scrollsIntoPlace);
void __cdecl Con_DrawMessageWindowOldToNew(
    int32_t localClientNum,
    MessageWindow *msgwnd,
    int32_t x,
    int32_t y,
    int32_t charHeight,
    int32_t horzAlign,
    int32_t vertAlign,
    bool up,
    struct Font_s *font,
    float *color,
    int32_t textStyle,
    float msgwndScale,
    char textAlignMode);
bool __cdecl CL_ShouldntDrawMessageWindow(int32_t localClientNum);
void __cdecl Con_DrawMiniConsole(int32_t localClientNum, int32_t xPos, int32_t yPos, float alpha);
void __cdecl Con_ClearMiniConsole(int32_t localClientNum);
void __cdecl Con_DrawErrors(int32_t localClientNum, int32_t xPos, int32_t yPos, float alpha);
bool __cdecl Con_IsValidGameMessageWindow(uint32_t windowIndex);
bool __cdecl Con_IsGameMessageWindowActive(int32_t localClientNum, uint32_t windowIndex);
void __cdecl Con_DrawSay(int32_t localClientNum, int32_t x, int32_t y);
void __cdecl Con_ToggleConsoleOutput();
void __cdecl Con_DrawConsole(int32_t localClientNum);
void __cdecl Con_DrawSolidConsole(int32_t localClientNum);
void __cdecl Con_DrawInput(int32_t localClientNum);
void __cdecl ConDrawInput_Text(char *str, const float *color);
void __cdecl ConDrawInput_TextAndOver(char *str, const float *color);
int32_t __cdecl ConDrawInput_TextWidth(const char *text);
void __cdecl ConDrawInput_Box(int32_t lines, const float *color);
void __cdecl ConDraw_Box(float x, float y, float w, float h, const float *color);
void __cdecl ConDrawInput_IncrMatchCounter(char *str);
void __cdecl ConDrawInput_DvarMatch(char *str);
void __cdecl ConDrawInput_TextLimitChars(char *str, int32_t maxChars, const float *color);
void __cdecl ConDrawInput_DetailedDvarMatch(char *str);
void __cdecl ConDrawInput_AutoCompleteArg(const char **stringList, int32_t stringCount);
int32_t __cdecl ConDrawInput_CompareStrings(const char **e0, const char **e1);
int32_t __cdecl ConDrawInput_TextFieldFirstArgChar();
int32_t __cdecl ConDrawInput_GetDvarDescriptionLines(const dvar_s *dvar);
void __cdecl ConDrawInput_DetailedCmdMatch(char *str);
void __cdecl ConDrawInput_CmdMatch(char *str);
void __cdecl Con_DrawAutoCompleteChoice(int32_t localClientNum, bool isDvarCommand, const char *originalCommand);
uint32_t __cdecl Con_GetAutoCompleteColorCodedString(
    char *query,
    const char *matchToText,
    int32_t matchTextLen,
    bool isDvarCommand,
    const char *originalCommand,
    char *colorCoded);
int32_t __cdecl Con_GetAutoCompleteColorCodedStringDiscontiguous(
    const char *query,
    const char *matchToText,
    int32_t matchTextLen,
    char *colorCoded);
int32_t __cdecl Con_GetAutoCompleteColorCodedStringContiguous(
    char *query,
    const char *matchToText,
    int32_t matchTextLen,
    char *colorCoded);
void __cdecl Con_DrawInputPrompt(int32_t localClientNum);
void Con_DrawOuputWindow();
void __cdecl Con_DrawOutputScrollBar(float x, float y, float width, float height);
void __cdecl Con_DrawOutputText(float x, float y);
void __cdecl Con_DrawOutputVersion(float x, float y, float width, float height);
char *__cdecl Con_GetVersionString();
void __cdecl Con_PageUp();
void __cdecl Con_PageDown();
void __cdecl Con_Top();
void __cdecl Con_Bottom();
void __cdecl Con_Close(int32_t localClientNum);
bool __cdecl Con_IsActive(int32_t localClientNum);
void __cdecl CL_PlayTextFXPulseSounds(
    uint32_t localClientNum,
    int32_t currentTime,
    int32_t strLength,
    int32_t fxBirthTime,
    int32_t fxLetterTime,
    int32_t fxDecayStartTime,
    int32_t *soundTimeKeeper);

extern const dvar_t *con_typewriterColorGlowFailed;
extern const dvar_t *con_typewriterColorGlowCompleted;
extern const dvar_t *con_typewriterColorGlowCheckpoint;
extern const dvar_t *cl_deathMessageWidth;
extern const dvar_t *con_typewriterColorBase;
extern const dvar_t *con_matchPrefixOnly;
extern const dvar_t *con_typewriterColorGlowUpdated;
extern const dvar_t *con_typewriterDecayDuration;
extern const dvar_t *con_typewriterDecayStartTime;
extern const dvar_t *con_restricted;
extern const dvar_t *con_typewriterPrintSpeed;

extern const dvar_t *con_inputBoxColor;
extern const dvar_t *con_inputHintBoxColor;
extern const dvar_t *con_outputBarColor;
extern const dvar_t *con_outputSliderColor;
extern const dvar_t *con_errormessagetime;
extern const dvar_t *con_minicontime;
extern const dvar_t *con_miniconlines;
extern const dvar_t *con_outputWindowColor;

extern ConDrawInputGlob conDrawInputGlob;
extern bool con_ignoreMatchPrefixOnly;
extern int32_t con_inputMaxMatchesShown;


// cl_keys
enum LocSelInputState : int32_t
{                                       // ...
    LOC_SEL_INPUT_NONE = 0x0,
    LOC_SEL_INPUT_CONFIRM = 0x1,
    LOC_SEL_INPUT_CANCEL = 0x2,
};
struct keyname_t // sizeof=0x8
{
    const char *name;
    int32_t keynum;
};
struct KeyState // sizeof=0xC
{                                       // ...
    int32_t down;                           // ...
    int32_t repeats;
    const char *binding;                // ...
};
struct PlayerKeyState // sizeof=0xD28
{                                       // ...
    field_t chatField;
    int32_t chat_team;                      // ...
    int32_t overstrikeMode;                 // ...
    int32_t anyKeyDown;                     // ...
    KeyState keys[256];                 // ...
    LocSelInputState locSelInputState;  // ...
};
void __cdecl TRACK_cl_keys();
void __cdecl Field_DrawTextOverride(
    int32_t localClientNum,
    const field_t *edit,
    int32_t x,
    int32_t y,
    int32_t horzAlign,
    int32_t vertAlign,
    char *str,
    int32_t drawLen,
    int32_t cursorPos);
void __cdecl Field_Draw(int32_t localClientNum, field_t *edit, int32_t x, int32_t y, int32_t horzAlign, int32_t vertAlign);
void __cdecl Field_AdjustScroll(const ScreenPlacement *scrPlace, field_t *edit);
void __cdecl Console_Key(int32_t localClientNum, int32_t key);
char __cdecl Field_KeyDownEvent(int32_t localClientNum, const ScreenPlacement *scrPlace, field_t *edit, int32_t key);
char __cdecl Field_Paste(int32_t localClientNum, const ScreenPlacement *scrPlace, field_t *edit);
bool __cdecl Field_CharEvent(int32_t localClientNum, const ScreenPlacement *scrPlace, field_t *edit, int32_t ch);
void CompleteCommand();
void __cdecl PrintMatches(const char *s);
void __cdecl ConcatRemaining(char *src, char *start);
int32_t keyConcatArgs();
void CompleteCmdArgument();
void __cdecl ReplaceConsoleInputArgument(int32_t replaceCount, char *replacement);
void CompleteDvarArgument();
void __cdecl UpdateMatches(bool searchCmds, int32_t *matchLenAfterCmds, int32_t *matchLenAfterDvars);
void __cdecl FindMatches(char *s);
bool __cdecl Console_IsRconCmd(const char *commandString);
char __cdecl Console_IsClientDisconnected();
int32_t __cdecl Key_GetOverstrikeMode(int32_t localClientNum);
void __cdecl Key_SetOverstrikeMode(int32_t localClientNum, int32_t state);
int32_t __cdecl Key_IsDown(int32_t localClientNum, int32_t keynum);
const char *__cdecl Key_KeynumToString(int32_t keynum, int32_t translate);
void __cdecl Key_SetBinding(int32_t localClientNum, int32_t keynum, char *binding);
const char *__cdecl Key_GetBinding(int32_t localClientNum, uint32_t keynum);
int32_t __cdecl Key_GetCommandAssignment(int32_t localClientNum, const char *command, int32_t *twokeys);
int32_t __cdecl Key_GetCommandAssignmentInternal(int32_t localClientNum, const char *command, int32_t *twokeys);
bool __cdecl Key_IsCommandBound(int32_t localClientNum, const char *command);
void __cdecl Key_Unbind_f();
int32_t __cdecl Key_StringToKeynum(const char *str);
void __cdecl Key_Unbindall_f();
void __cdecl Key_Bind_f();
void __cdecl Key_WriteBindings(int32_t localClientNum, int32_t f);
int32_t __cdecl Key_WriteBindingsToBuffer(int32_t localClientNum, char *buffer, int32_t bufferSize);
void __cdecl Key_Bindlist_f();
void __cdecl CL_InitKeyCommands();
bool __cdecl CL_IsConsoleKey(int32_t key);
void __cdecl CL_KeyEvent(int32_t localClientNum, int32_t key, int32_t down, uint32_t time);
void __cdecl Message_Key(int32_t localClientNum, int32_t key);
bool __cdecl CL_MouseInputShouldBypassMenus(int32_t localClientNum, int32_t key);
void __cdecl CL_CharEvent(int32_t localClientNum, int32_t key);
void __cdecl CL_ConsoleCharEvent(int32_t localClientNum, int32_t key);
void __cdecl Key_ClearStates(int32_t localClientNum);
int32_t __cdecl CL_GetKeyBinding(int32_t localClientNum, const char *command, char (*keyNames)[128]);
int32_t __cdecl CL_GetKeyBindingInternal(int32_t localClientNum, const char *command, char (*keyNames)[128]);
void __cdecl Key_Shutdown();
bool __cdecl Key_IsCatcherActive(int32_t localClientNum, int32_t mask);
void __cdecl Key_AddCatcher(int32_t localClientNum, int32_t orMask);
void __cdecl Key_RemoveCatcher(int32_t localClientNum, int32_t andMask);
void __cdecl Key_SetCatcher(int32_t localClientNum, int32_t catcher);

extern field_t g_consoleField;


// screen_placement
void __cdecl ScrPlace_SetupFloatViewport(
    ScreenPlacement *scrPlace,
    float viewportX,
    float viewportY,
    float viewportWidth,
    float viewportHeight);
void __cdecl ScrPlace_CalcSafeAreaOffsets(
    float viewportX,
    float viewportY,
    float viewportWidth,
    float viewportHeight,
    float horzAspectScale,
    float *realViewableMin,
    float *realViewableMax,
    float *virtualViewableMin,
    float *virtualViewableMax);
void __cdecl ScrPlace_SetupViewport(
    ScreenPlacement *scrPlace,
    int32_t viewportX,
    int32_t viewportY,
    int32_t viewportWidth,
    int32_t viewportHeight);
void __cdecl ScrPlace_SetupUnsafeViewport(
    ScreenPlacement *scrPlace,
    int32_t viewportX,
    int32_t viewportY,
    int32_t viewportWidth,
    int32_t viewportHeight);
double __cdecl ScrPlace_ApplyX(const ScreenPlacement *scrPlace, float x, int32_t horzAlign);
double __cdecl ScrPlace_ApplyY(const ScreenPlacement *scrPlace, float y, int32_t vertAlign);
void __cdecl ScrPlace_ApplyRect(
    const ScreenPlacement *scrPlace,
    float *x,
    float *y,
    float *w,
    float *h,
    int32_t horzAlign,
    int32_t vertAlign);

extern struct ScreenPlacement scrPlaceView[1];
extern struct ScreenPlacement scrPlaceFull;
extern float cg_hudSplitscreenScale;
extern ScreenPlacement scrPlaceFullUnsafe;

extern PlayerKeyState playerKeys[1];




// con_channels
char __cdecl Con_OpenChannel(char *name, bool allowScript);
bool __cdecl Con_ScriptHasPermission(uint32_t channel);
bool __cdecl Con_GetChannel(const char *name, int32_t *channel_result);
bool __cdecl Con_IsChannelOpen(uint32_t channel);
bool __cdecl Con_IsChannelVisible(print_msg_dest_t dest, uint32_t channel, int32_t errorflags);
void __cdecl Con_WriteFilterConfigString(int32_t f);
void __cdecl Con_InitGameMsgChannels();
void __cdecl Con_InitChannelsForDestFromList(print_msg_dest_t dest, const char *channelNames);
void __cdecl Con_FilterShowChannel(print_msg_dest_t dest, const char *channelName, bool show);
void __cdecl Con_InitChannels();
void __cdecl Con_ChannelList_f();
void __cdecl Con_FilterAdd_f();
void __cdecl Con_FilterAdd(bool show);
void __cdecl Con_FilterRemove_f();
void __cdecl Con_FilterList_f();
void __cdecl Con_ShutdownChannels();
void __cdecl Con_CloseChannelInternal(uint32_t channel);



// cl_devgui
void __cdecl CL_DevGuiFrame(int32_t localClientNum);
void __cdecl CL_CreateDevGui();
void __cdecl CL_DestroyDevGui();



// cl_debugdata
struct clientDebugStringInfo_t;
struct clientDebugLineInfo_t;

void CL_RecordServerDebugData();
void __cdecl CL_AddDebugString(
    const float *xyz,
    const float *color,
    float scale,
    const char *text,
    int32_t fromServer,
    int32_t duration);
bool __cdecl CreateDebugStringsIfNeeded();
void __cdecl AddDebugStringInternal(
    const float *xyz,
    const float *color,
    float scale,
    const char *text,
    int32_t duration,
    clientDebugStringInfo_t *info);
void __cdecl CL_AddDebugLine(
    const float *start,
    const float *end,
    const float *color,
    int32_t depthTest,
    int32_t duration,
    int32_t fromServer);
bool __cdecl CreateDebugLinesIfNeeded();
void __cdecl AddDebugLineInternal(
    const float *start,
    const float *end,
    const float *color,
    int32_t depthTest,
    int32_t duration,
    clientDebugLineInfo_t *info);
void __cdecl CL_AddDebugStarWithText(
    const float *point,
    const float *starColor,
    const float *textColor,
    const char *string,
    float fontsize,
    int32_t duration,
    int32_t fromServer);
void __cdecl CL_AddDebugStar(const float *point, const float *color, int32_t duration, int32_t fromServer);
void __cdecl CL_FlushDebugClientData();
void __cdecl FlushDebugStrings(clientDebugStringInfo_t *info, int32_t fromServer);
void __cdecl FlushDebugLines(clientDebugLineInfo_t *info, int32_t fromServer);
void __cdecl CL_UpdateDebugClientData();
void __cdecl CL_FlushDebugServerData();
void __cdecl CL_UpdateDebugServerData();
void __cdecl CL_ShutdownDebugData();



// cl_cin
int32_t __cdecl CIN_PlayCinematic(int32_t localClientNum, char *arg);
void __cdecl CL_PlayCinematic_f();
void __cdecl CL_PlayUnskippableCinematic_f();
void __cdecl SCR_DrawCinematic(int32_t localClientNum);
void __cdecl SCR_StopCinematic(int32_t localClientNum);


extern int32_t g_console_field_width;
extern float g_console_char_height;
extern field_t historyEditLines[32];

extern int32_t dvar_modifiedFlags;


#ifdef KISAK_SP

enum connstate_t : int32_t
{
    CA_DISCONNECTED = 0x0,
    CA_CINEMATIC = 0x1,
    CA_LOGO = 0x2,
    CA_LOADING = 0x3,
    CA_ACTIVE = 0x4,
    CA_MAP_RESTART = 0x5,
};

enum SaveBufferState : int32_t
{
    MEMCLEAR = 0x0,
    MEMALLOCATING = 0x1,
    MEMALLOCATED = 0x2,
    SAVING = 0x3,
    AWAITING_COMMIT = 0x4,
    COMMITTED = 0x5,
    LOADING = 0x6,
};
struct SaveHeader
{
    int32_t saveVersion;
    int32_t gameCheckSum;
    int32_t saveCheckSum;
    int32_t saveId;
    bool isUsingScriptChecksum;
    int32_t scrCheckSum[3];
    char mapName[256];
    char buildNumber[128];
    char campaign[256];
    char screenShotName[64];
    char description[256];
    char filename[64];
    int32_t health;
    int32_t skill;
    bool internalSave;
    bool demoPlayback;
    qtime_s time;
    int32_t bodySize;
};
struct SaveGame
{
    MemoryFile memFile;
    SaveBufferState saveState;
    SaveHeader header;
    bool isUsingGlobalBuffer;
    bool isDirectWriteActive;
    bool isWrittenToDevice;
    bool suppressPlayerNotify;
};
struct snapshot_s
{
    int32_t snapFlags;
    int32_t serverTime;
    playerState_s ps;
    int32_t numEntities;
    int32_t entityNums[2048];
    int32_t serverCommandSequence;
};

struct clientUIActive_t
{
    bool isRunning;
    bool cgameInitialized;
    bool cgameInitCalled;
    bool isLoadComplete;
    int32_t keyCatchers;
    bool displayHUDWithKeycatchUI;
    connstate_t connectionState;
    int32_t nextScrollTime;
};

struct clientLogo_t
{
    int32_t startTime;
    int32_t duration;
    int32_t fadein;
    int32_t fadeout;
    Material *material[2];
};

struct __declspec(align(4)) vidConfig_t
{
    uint32_t sceneWidth;
    uint32_t sceneHeight;
    uint32_t displayWidth;
    uint32_t displayHeight;
    float displayFrequency;
    int32_t isWideScreen;
    int32_t isHiDef;
    int32_t isFullscreen;
    float aspectRatioWindow;
    float aspectRatioScenePixel;
    float aspectRatioDisplayPixel;
    uint32_t maxTextureSize;
    uint32_t maxTextureMaps;
    bool deviceSupportsGamma;
};

struct clientStatic_t
{
    int32_t quit;
    char servername[256];
    int32_t rendererStarted;
    int32_t soundStarted;
    int32_t uiStarted;
    volatile int32_t scriptError;
    int32_t devGuiStarted;
    int32_t frametime;
    int32_t animFrametime;
    int32_t realtime;
    int32_t realFrametime;
    clientLogo_t logo;
    Font_s *consoleFont;
    bool demoplaying;
    bool demoPending;
    bool demorecording;
    bool isTimeDemo;
    char demoName[64];
    void *demofile;
    void *demobuf;
    void *timeDemoLog;
    int32_t timeDemoFrames;
    int32_t timeDemoStart;
    int32_t timeDemoPrev;
    int32_t timeDemoBaseTime;
    vidConfig_t vidConfig;
    Material *whiteMaterial;
    Material *consoleMaterial;
    clientDebug_t debug;
    float renderForward[3];
    float renderPos[3];
};

struct clSnapshot_t
{
    int32_t valid;
    int32_t snapFlags;
    int32_t serverTime;
    int32_t messageNum;
    int32_t cmdNum;
    playerState_s ps;
    int32_t numEntities;
    int32_t parseEntitiesNum;
    int32_t serverCommandNum;
};

enum StanceState : int32_t
{
    CL_STANCE_STAND = 0x0,
    CL_STANCE_CROUCH = 0x1,
    CL_STANCE_PRONE = 0x2,
};

struct clientActive_t
{
    clSnapshot_t snap;
    int32_t serverTime;
    uint16_t configstrings[2815];
    char mapname[64];
    bool usingAds;
    int32_t parseEntitiesNum;
    int32_t mouseDx[2];
    int32_t mouseDy[2];
    int32_t mouseIndex;
    bool stanceHeld;
    StanceState stance;
    StanceState stancePosition;
    int32_t stanceTime;
    int32_t cgameUserCmdWeapon;
    int32_t cgameUserCmdOffHandIndex;
    float cgameUserCmdGunPitch;
    float cgameUserCmdGunYaw;
    float cgameUserCmdGunXOfs;
    float cgameUserCmdGunYOfs;
    float cgameUserCmdGunZOfs;
    float cgameFOVSensitivityScale;
    float cgameMaxPitchSpeed;
    float cgameMaxYawSpeed;
    int32_t cgameExtraButtons;
    float viewangles[3];
    usercmd_s cmds[64];
    int32_t cmdNumber;
    int32_t cmdNumberAcknowledge;
    int32_t bCmdForceValues;
    int32_t iForceButtons;
    int32_t iForceWeapon;
    int32_t forceOffhand;
    int32_t skelTimeStamp;
    volatile int32_t skelMemPos;
    char skelMemory[524288];
    char *skelMemoryStart;
    bool allowedAllocSkel;
    clSnapshot_t snapshots[1];
    int32_t parseEntityNums[2048];
};

struct serverCommandsHeader_t
{
    int32_t rover;
    int32_t sequence;
    int32_t sent;
};

struct serverCommands_s
{
    serverCommandsHeader_t header;
    char buf[8192];
    int32_t commands[256];
};

struct clientConnection_t
{
    int32_t lastPacketTime;
    int32_t challenge;
    int32_t reliableSequence;
    int32_t reliableAcknowledge;
    char reliableCommands[256][1024];
    int32_t serverMessageSequence;
    serverCommands_s serverCommands;
};

void __cdecl CL_SetLocalClientConnectionState(int32_t localClientNum, connstate_t connstate);
void __cdecl TRACK_cl_cgame();
void __cdecl CL_GetScreenDimensions(uint32_t *width, uint32_t *height, float *aspect);
float __cdecl CL_GetScreenAspectRatioDisplayPixel();
int32_t __cdecl CL_GetUserCmd(int32_t localClientNum, int32_t cmdNumber, usercmd_s *ucmd);
int32_t __cdecl CL_GetCurrentCmdNumber(int32_t localClientNum);
void __cdecl CL_GetCurrentSnapshotNumber(int32_t localClientNum, int32_t *snapshotNumber, int32_t *serverTime);
int32_t __cdecl CL_GetSnapshot(int32_t localClientNum, snapshot_s *snapshot);
void __cdecl CL_SetUserCmdWeapons(int32_t localClientNum, int32_t weapon, int32_t offHandIndex);
void __cdecl CL_SetUserCmdAimValues(
    int32_t localClientNum,
    double gunPitch,
    double gunYaw,
    double gunXOfs,
    double gunYOfs,
    double gunZOfs);
void __cdecl CL_SetFOVSensitivityScale(int32_t localClientNum, double scale);
void __cdecl CL_SetExtraButtons(int32_t localClientNum, int32_t buttons);
void CL_ConfigstringModified();
void __cdecl CL_Restart();
int32_t __cdecl CL_PreprocessServerCommand(const char *s);
int32_t __cdecl CL_CGameNeedsServerCommand(int32_t localClientNum, int32_t serverCommandNumber);
void __cdecl CL_ArchiveServerCommands(MemoryFile *memFile);
void __cdecl CL_LoadServerCommands(SaveGame *save);
void __cdecl CL_ShutdownCGame();
int32_t __cdecl CL_DObjCreateSkelForBone(DObj_s *obj, int32_t boneIndex);
void __cdecl LoadWorld(const char *name, int32_t savegame);
void __cdecl CL_SubtitlePrint(int32_t localClientNum, const char *text, int32_t duration, int32_t pixelWidth);
const char *__cdecl CL_GetConfigString(int32_t localClientNum, uint32_t configStringIndex);
snd_alias_t *__cdecl CL_PickSoundAlias(const char *aliasname);
void __cdecl CL_FinishLoadingModels();
void __cdecl CL_GetViewForward(float *forward);
void __cdecl CL_GetViewPos(float *pos);
void __cdecl CL_RenderScene(const refdef_s *fd);
void __cdecl CL_SetFullScreenViewport();
void __cdecl CL_SetViewport(int32_t x, int32_t y, int32_t width, int32_t height);
void __cdecl CL_DrawStretchPicPhysical(
    double x,
    double y,
    double w,
    double h,
    double s1,
    double t1,
    double s2,
    double t2,
    const float *color,
    Material *material);
void __cdecl CL_DrawStretchPicPhysicalFlipST(
    double x,
    double y,
    double w,
    double h,
    double s1,
    double t1,
    double s2,
    double t2,
    const float *color,
    Material *material);
void __cdecl CL_DrawStretchPic(
    const ScreenPlacement *scrPlace,
    double x,
    double y,
    double w,
    double h,
    int32_t horzAlign,
    int32_t vertAlign,
    double s1,
    double t1,
    double s2,
    double t2,
    const float *color,
    Material *material,
    int32_t a14,
    int32_t a15,
    float a16,
    float a17,
    float a18,
    float a19,
    float a20,
    float a21,
    float a22,
    float a23,
    float a24,
    float a25,
    float a26,
    float a27);
void __cdecl CL_DrawStretchPicPhysicalRotateXY(
    double x,
    double y,
    double w,
    double h,
    double s1,
    double t1,
    double s2,
    double t2,
    const float *color,
    Material *material,
    float angle);
void __cdecl CL_DrawStretchPicFlipST(
    const ScreenPlacement *scrPlace,
    double x,
    double y,
    double w,
    double h,
    int32_t horzAlign,
    int32_t vertAlign,
    double s1,
    double t1,
    double s2,
    double t2,
    const float *color,
    Material *material,
    int32_t a14,
    int32_t a15,
    float a16,
    float a17,
    float a18,
    float a19,
    float a20,
    float a21,
    float a22,
    float a23,
    float a24,
    float a25,
    float a26,
    float a27);
void __cdecl CL_DrawStretchPicRotatedST(
    const ScreenPlacement *scrPlace,
    double x,
    double y,
    double w,
    double h,
    int32_t horzAlign,
    int32_t vertAlign,
    double centerS,
    double centerT,
    double radiusST,
    double scaleFinalS,
    const float *color,
    Material *material,
    int32_t a14,
    int32_t a15,
    float scaleFinalT,
    float a17,
    float angle,
    float a19,
    float a20,
    float a21,
    float a22,
    float a23,
    float a24,
    float a25,
    float a26,
    float a27);
void __cdecl CL_ProjectionSet2D();
void __cdecl CL_ProjectionSet3D();
void __cdecl CL_CapTurnRate(int32_t localClientNum, double maxPitchSpeed, double maxYawSpeed);
void __cdecl CL_SetViewAngles(int32_t localClientNum, float *angles);
void __cdecl CL_StartLoading(const char *mapname);
void __cdecl CL_InitCGame(int32_t localClientNum, int32_t savegame);
void __cdecl CL_FirstSnapshot();
void __cdecl CL_SetActive();
void __cdecl CL_CreateNextSnap();
char *__cdecl CL_TimeDemoLogBaseName(const char *mapname);
void CL_UpdateTimeDemo();
void __cdecl CL_SetCGameTime(int32_t localClientNum);
void __cdecl CL_SetADS(int32_t localClientNum, bool ads);
void __cdecl CL_ArchiveClientState(MemoryFile *memFile, int32_t segmentIndex);
void __cdecl CL_LookupColor(uint8_t c, float *color);
bool __cdecl CL_IsCgameInitialized(int32_t localClientNum);



// cl_main
void __cdecl TRACK_cl_main();
int32_t __cdecl CL_GetLocalClientActiveCount();
int32_t __cdecl CL_GetFirstActiveLocalClient();
bool __cdecl CL_IsLocalClientActive(int32_t clientNum);
void __cdecl CL_SetLocalClientActive(int32_t clientNum, bool active);
int32_t __cdecl CL_LocalClientNumFromControllerIndex(uint32_t controllerIndex);
int32_t __cdecl CL_ControllerIndexFromClientNum(int32_t clientIndex);
int32_t __cdecl CL_GetFirstActiveControllerIndex();
int32_t __cdecl CL_AllLocalClientsInactive();
void __cdecl CL_RunOncePerClientFrame(int32_t localClientNum, int32_t msec);
void __cdecl CL_DumpReliableCommand(int32_t cmdIndex, const char *cmd);
void __cdecl CL_DumpReliableCommands(clientConnection_t *clc);
void __cdecl CL_AddReliableCommand(int32_t localClientNum, const char *cmd);
void __cdecl CL_ShutdownDevGui();
void __cdecl CL_ShutdownHunkUsers();
void __cdecl CL_ShutdownDemo();
void __cdecl CL_SaveSettings(MemoryFile *memFile);
void __cdecl CL_RestoreSettings(MemoryFile *memFile);
void __cdecl CL_MapLoading_CalcMovieToPlay(
    const char *buffer,
    const char *inMapName,
    char *outMovieName,
    int32_t a4,
    int32_t a5,
    int32_t a6,
    int32_t a7,
    int32_t a8,
    int32_t a9,
    int32_t a10,
    int32_t a11,
    const char *a12,
    int32_t a13,
    const char *a14,
    int32_t a15,
    char *a16);
void __cdecl CL_MapLoading_CalcMovieToPlay_FastFile(const char *inMapName, char *outMovieName, const char *a3);
void __cdecl CL_MapLoading_StartCinematic(const char *mapname, double volume, int32_t a3, const char *a4);
void __cdecl CL_MapLoading(const char *mapname);
void __cdecl CL_ResetSkeletonCache();
void __cdecl CL_ClearState(int32_t a1, int32_t a2);
void __cdecl CL_Disconnect(int32_t localClientNum);
void __cdecl CL_ForwardCommandToServer(int32_t localClientNum, const char *string);
void __cdecl CL_ForwardToServer_f();
void __cdecl CL_ConnectResponse();
void __cdecl CL_InitLoad(const char *mapname);
void __cdecl CL_PacketEvent(msg_t *msg, int32_t serverMessageSequence);
void __cdecl CL_SetFrametime(int32_t frametime, int32_t animFrametime);
void __cdecl CheckForConsoleGuidePause(int32_t localClientNum);
void __cdecl CL_Frame(
    int32_t localClientNum,
    int32_t msec,
    int32_t a3,
    int32_t a4,
    int32_t a5,
    int32_t a6,
    int32_t a7,
    int32_t a8,
    int32_t a9,
    int32_t a10,
    int32_t a11,
    int32_t a12,
    int32_t a13,
    int32_t a14);
bool __cdecl CL_IsLocalClientInGame(int32_t localClientNum);
bool __cdecl CL_IsUIActive(const int32_t localClientNum);
void __cdecl CL_InitRenderer();
void __cdecl CL_ShutdownRenderer(int32_t destroyWindow);
void CL_DevGuiDvar_f();
void CL_DevGuiCmd_f();
void CL_DevGuiOpen_f();
void CL_InitDevGui();
void __cdecl CL_StartHunkUsers();
int32_t __cdecl CL_ScaledMilliseconds();
void __cdecl CL_InitRef();
void __cdecl CL_VoidCommand();
void __cdecl CL_startMultiplayer_f();
void __cdecl CL_ShellExecute_URL_f();
void __cdecl CL_IncAnimWeight_f(int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6, __int64 a7);
void __cdecl CL_DecAnimWeight_f(int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6, __int64 a7);
void __cdecl CL_StopLogo(int32_t localClientNum);
void __cdecl CL_PlayLogo_f();
void __cdecl CL_StopLogoOrCinematic(int32_t localClientNum);
void __cdecl CL_InitOnceForAllClients();
void __cdecl CL_StopControllerRumbles();
void CL_Pause_f();
void __cdecl CL_Shutdown(int32_t localClientNum);
void __cdecl CL_DrawTextPhysical(
    const char *text,
    int32_t maxChars,
    Font_s *font,
    double x,
    double y,
    double xScale,
    double yScale,
    const float *color,
    int32_t style);
void __cdecl CL_DrawTextPhysicalWithEffects(
    const char *text,
    int32_t maxChars,
    Font_s *font,
    double x,
    double y,
    double xScale,
    double yScale,
    const float *color,
    int32_t style,
    const float *glowColor,
    Material *fxMaterial,
    Material *fxMaterialGlow,
    int32_t fxBirthTime,
    int32_t fxLetterTime,
    int32_t fxDecayStartTime,
    int32_t fxDecayDuration);
void __cdecl CL_DrawText(
    const ScreenPlacement *scrPlace,
    const char *text,
    int32_t maxChars,
    Font_s *font,
    double x,
    double y,
    int32_t horzAlign,
    int32_t vertAlign,
    double xScale,
    double yScale,
    const float *color,
    int32_t style,
    double a13,
    double a14,
    double a15,
    double a16,
    float a17,
    float a18,
    float a19,
    float a20,
    float a21,
    float a22,
    float a23,
    float a24,
    float a25,
    float a26,
    float a27,
    float a28,
    float a29,
    float a30,
    float a31,
    float a32,
    float a33,
    float a34,
    float a35,
    float a36,
    float a37,
    float a38);
void __cdecl CL_DrawTextRotate(
    const ScreenPlacement *scrPlace,
    const char *text,
    int32_t maxChars,
    Font_s *font,
    double x,
    double y,
    double rotation,
    int32_t horzAlign,
    int32_t vertAlign,
    double xScale,
    double yScale,
    const float *color,
    int32_t style,
    double a14,
    double a15,
    double a16,
    float a17,
    float a18,
    float a19,
    float a20,
    float a21,
    float a22,
    float a23,
    float a24,
    float a25,
    float a26,
    float a27,
    float a28,
    float a29,
    float a30,
    float a31,
    float a32,
    float a33,
    float a34,
    float a35,
    int32_t a36,
    float a37,
    float a38,
    float a39,
    float a40);
void __cdecl CL_DrawTextPhysicalWithCursor(
    const char *text,
    int32_t maxChars,
    Font_s *font,
    double x,
    double y,
    double xScale,
    double yScale,
    const float *color,
    int32_t style,
    int32_t cursorPos,
    char cursor);
void __cdecl CL_DrawTextWithCursor(
    const ScreenPlacement *scrPlace,
    const char *text,
    int32_t maxChars,
    Font_s *font,
    double x,
    double y,
    int32_t horzAlign,
    int32_t vertAlign,
    double xScale,
    double yScale,
    const float *color,
    int32_t style,
    double a13,
    double a14,
    double a15,
    double a16,
    int32_t cursorPos,
    char cursor,
    int32_t a19,
    int32_t a20,
    int32_t a21,
    int32_t a22,
    int32_t a23,
    int32_t a24,
    int32_t a25,
    int32_t a26,
    int32_t a27,
    float a28,
    int32_t a29,
    float a30,
    int32_t a31,
    int32_t a32,
    int32_t a33,
    int32_t a34,
    int32_t a35,
    float a36,
    int32_t a37,
    float a38);
Font_s *__cdecl CL_RegisterFont(const char *fontName, int32_t imageTrack);
void __cdecl CL_SetSkipRendering(bool skip);
bool __cdecl CL_SkipRendering();
void __cdecl CL_UpdateSound();
void __cdecl CL_ShutdownAll();
void __cdecl CL_DisconnectLocalClient();
void __cdecl CL_Disconnect_f();
void __cdecl CL_ShutdownRef();
void __cdecl CL_DrawLogo(const float *a1, Material *a2);
void __cdecl CL_Init(int32_t localClientNum);


extern clientConnection_t clientConnections[1];
extern clientUIActive_t clientUIActives[1];
extern clientActive_t clients[1];
extern clientStatic_t cls;

#endif