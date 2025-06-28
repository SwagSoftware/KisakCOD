#pragma once
#include <qcommon/qcommon.h>
#include <ui_mp/ui_mp.h>

enum msgwnd_mode_t : __int32
{                                       // ...
    MWM_BOTTOMUP_ALIGN_TOP = 0x0,
    MWM_BOTTOMUP_ALIGN_BOTTOM = 0x1,
    MWM_TOPDOWN_ALIGN_TOP = 0x2,
    MWM_TOPDOWN_ALIGN_BOTTOM = 0x3,
};

enum print_msg_dest_t : __int32
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
inline print_msg_dest_t &operator++(print_msg_dest_t &e, int i)
{
    ++e;
    return e;
}

struct ConDrawInputGlob // sizeof=0x64
{                                       // ...
    char autoCompleteChoice[64];        // ...
    int matchIndex;                     // ...
    int matchCount;                     // ...
    const char *inputText;              // ...
    int inputTextLen;                   // ...
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
    int messageIndex;
    int textBufPos;
    int textBufSize;
    int typingStartTime;
    int lastTypingSoundTime;
    int flags;
};
struct Message // sizeof=0x8
{                                       // ...
    int startTime;
    int endTime;
};
struct MessageWindow // sizeof=0x34
{                                       // ...
    MessageLine *lines;                 // ...
    Message *messages;
    char *circularTextBuffer;           // ...
    int textBufSize;                    // ...
    int lineCount;                      // ...
    int padding;
    int scrollTime;
    int fadeIn;
    int fadeOut;
    int textBufPos;                     // ...
    int firstLineIndex;                 // ...
    int activeLineCount;                // ...
    int messageIndex;
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
    int initialized;                    // ...
    MessageWindow consoleWindow;        // ...
    MessageLine consoleLines[1024];     // ...
    Message consoleMessages[1024];      // ...
    char consoleText[32768];            // ...
    char textTempLine[512];             // ...
    unsigned int lineOffset;            // ...
    int displayLineOffset;              // ...
    int prevChannel;                    // ...
    bool outputVisible;                 // ...
    // padding byte
    // padding byte
    // padding byte
    int fontHeight;                     // ...
    int visibleLineCount;               // ...
    int visiblePixelWidth;              // ...
    float screenMin[2];                 // ...
    float screenMax[2];                 // ...
    MessageBuffer messageBuffer[1];     // ...
    float color[4];                     // ...
};

// cl_console
void __cdecl TRACK_cl_console();
void __cdecl Con_ToggleConsole();
void __cdecl Con_OpenConsole(int localClientNum);
void __cdecl Con_OpenConsoleOutput(int localClientNum);
void __cdecl Con_CloseConsole(int localClientNum);
void __cdecl Con_GetTextCopy(char *text, int maxSize);
void __cdecl Con_TimeJumped(int localClientNum, int serverTime);
void __cdecl Con_ResetMessageWindowTimes(MessageWindow *msgwnd, int serverTime);
void __cdecl Con_TimeNudged(int localClientNum, int serverTimeNudge);
void __cdecl Con_NudgeMessageWindowTimes(MessageWindow *msgwnd, int serverTimeNudge, int serverTime);
void __cdecl Con_ClearNotify(int localClientNum);
void __cdecl Con_ClearMessageWindow(MessageWindow *msgwnd);
void __cdecl Con_ClearErrors(int localClientNum);
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
    int lineCount,
    int padding,
    int textPoolSize,
    int scrollTime,
    int fadeIn,
    int fadeOut);
void __cdecl CL_ConsolePrint(int localClientNum, int channel, const char *txt, int duration, int pixelWidth, int flags);
void Con_OneTimeInit();
char __cdecl CL_ConsolePrint_AddLine(
    int localClientNum,
    int channel,
    const char *txt,
    int duration,
    int pixelWidth,
    char color,
    int flags);
void __cdecl Con_UpdateNotifyMessage(int localClientNum, unsigned int channel, int duration, int flags);
void __cdecl Con_UpdateNotifyMessageWindow(
    int localClientNum,
    unsigned int channel,
    int duration,
    int flags,
    print_msg_dest_t dest);
int __cdecl Con_GetDefaultMsgDuration(print_msg_dest_t dest);
void __cdecl Con_UpdateMessage(int localClientNum, MessageWindow *msgwnd, int duration);
MessageWindow *__cdecl Con_GetDestWindow(int localClientNum, print_msg_dest_t dest);
void __cdecl Con_UpdateNotifyLine(int localClientNum, unsigned int channel, bool lineFeed, int flags);
void __cdecl Con_UpdateNotifyLineWindow(
    int localClientNum,
    unsigned int channel,
    bool lineFeed,
    int flags,
    print_msg_dest_t dest);
void __cdecl Con_UpdateMessageWindowLine(int localClientNum, MessageWindow *msgwnd, int linefeed, int flags);
void __cdecl Con_FreeFirstMessageWindowLine(MessageWindow *msgwnd);
void __cdecl Con_CopyCurrentConsoleLineText(MessageWindow *msgwnd, MessageLine *msgLine);
bool __cdecl Con_NeedToFreeMessageWindowLine(MessageWindow *msgwnd, int charCount);
int __cdecl PrintTimeTotal(MessageWindow *msgwnd, MessageLine *line);
int __cdecl PrintableCharsCount(const MessageWindow *msgwnd, MessageLine *line);
int __cdecl GetNextValidPrintTimeForLine(int localClientNum, MessageWindow *msgwnd, char flags);
int __cdecl LatestActiveTypewrittenLineIdx(MessageWindow *msgwnd);
int __cdecl PrintTimeWriteOut(MessageWindow *msgwnd, MessageLine *line);
void __cdecl Con_Linefeed(int localClientNum, unsigned int channel, int flags);
void __cdecl CL_ConsoleFixPosition();
void __cdecl CL_DeathMessagePrint(
    int localClientNum,
    char *attackerName,
    char attackerColorIndex,
    char *victimName,
    char victimColorIndex,
    struct Material *iconShader,
    float iconWidth,
    float iconHeight,
    bool horzFlipIcon);
unsigned int __cdecl CL_AddDeathMessageString(
    char *deathMsg,
    unsigned int deathMsgLen,
    unsigned int deathMsgMaxLen,
    char *string);
unsigned int __cdecl CL_AddDeathMessageIcon(
    char *deathMsg,
    unsigned int deathMsgLen,
    unsigned int deathMsgMaxLen,
    struct Material *iconShader,
    float iconWidth,
    float iconHeight,
    bool horzFlipIcon);
int __cdecl CL_DeathMessageIconDimension(float size);
void __cdecl Con_AutoCompleteFromList(
    const char **strings,
    unsigned int stringCount,
    const char *prefix,
    char *completed,
    unsigned int sizeofCompleted);
const char *__cdecl Con_TokenizeInput();
char __cdecl Con_AnySpaceAfterCommand();
bool __cdecl Con_IsAutoCompleteMatch(const char *query, const char *matchToText, int matchTextLen);
bool __cdecl Con_HasTooManyMatchesToShow();
bool __cdecl Con_IsDvarCommand(const char *cmd);
char __cdecl Con_CycleAutoComplete(int step);
bool __cdecl Con_HasActiveAutoComplete();
char __cdecl Con_CommitToAutoComplete();
char __cdecl Con_CancelAutoComplete();
void __cdecl Con_AllowAutoCompleteCycling(bool isAllowed);
void __cdecl Con_DrawGameMessageWindow(
    int localClientNum,
    unsigned int windowIndex,
    int xPos,
    int yPos,
    int horzAlign,
    int vertAlign,
    struct Font_s *font,
    float fontScale,
    float *color,
    int textStyle,
    char textAlignMode,
    msgwnd_mode_t mode);
void __cdecl Con_DrawMessageWindow(
    int localClientNum,
    MessageWindow *msgwnd,
    int x,
    int y,
    int charHeight,
    int horzAlign,
    int vertAlign,
    struct Font_s *font,
    float *color,
    int textStyle,
    float msgwndScale,
    msgwnd_mode_t mode,
    char textAlignMode);
void __cdecl Con_DrawMessageWindowNewToOld(
    int localClientNum,
    MessageWindow *msgwnd,
    int x,
    int y,
    int hudCharHeight,
    int horzAlign,
    int vertAlign,
    bool up,
    struct Font_s *font,
    float *color,
    int textStyle,
    float msgwndScale,
    char textAlignMode);
void __cdecl Con_DrawMessageLineOnHUD(
    int localClientNum,
    const ScreenPlacement *scrPlace,
    int x,
    int y,
    int charHeight,
    int horzAlign,
    int vertAlign,
    struct Font_s *font,
    const MessageWindow *msgwnd,
    int lineIdx,
    float *color,
    int textStyle,
    float msgwndScale,
    char textAlignMode);
bool __cdecl LineVisible(const MessageWindow *msgwnd, int lineIdx, int time);
void __cdecl TypewriterSounds(int localClientNum, const MessageWindow *msgwnd, MessageLine *line);
void __cdecl Con_CullFinishedLines(int serverTime, MessageWindow *msgwnd);
double __cdecl Con_GetMessageAlpha(Message *message, MessageWindow *msgwnd, int serverTime, bool scrollsIntoPlace);
void __cdecl Con_DrawMessageWindowOldToNew(
    int localClientNum,
    MessageWindow *msgwnd,
    int x,
    int y,
    int charHeight,
    int horzAlign,
    int vertAlign,
    bool up,
    struct Font_s *font,
    float *color,
    int textStyle,
    float msgwndScale,
    char textAlignMode);
bool __cdecl CL_ShouldntDrawMessageWindow(int localClientNum);
void __cdecl Con_DrawMiniConsole(int localClientNum, int xPos, int yPos, float alpha);
void __cdecl Con_ClearMiniConsole(int localClientNum);
void __cdecl Con_DrawErrors(int localClientNum, int xPos, int yPos, float alpha);
bool __cdecl Con_IsValidGameMessageWindow(unsigned int windowIndex);
bool __cdecl Con_IsGameMessageWindowActive(int localClientNum, unsigned int windowIndex);
void __cdecl Con_DrawSay(int localClientNum, int x, int y);
void __cdecl Con_ToggleConsoleOutput();
void __cdecl Con_DrawConsole(int localClientNum);
void __cdecl Con_DrawSolidConsole(int localClientNum);
void __cdecl Con_DrawInput(int localClientNum);
void __cdecl ConDrawInput_Text(char *str, const float *color);
void __cdecl ConDrawInput_TextAndOver(char *str, const float *color);
int __cdecl ConDrawInput_TextWidth(const char *text);
void __cdecl ConDrawInput_Box(int lines, const float *color);
void __cdecl ConDraw_Box(float x, float y, float w, float h, const float *color);
void __cdecl ConDrawInput_IncrMatchCounter(char *str);
void __cdecl ConDrawInput_DvarMatch(char *str);
void __cdecl ConDrawInput_TextLimitChars(char *str, int maxChars, const float *color);
void __cdecl ConDrawInput_DetailedDvarMatch(char *str);
void __cdecl ConDrawInput_AutoCompleteArg(const char **stringList, int stringCount);
int __cdecl ConDrawInput_CompareStrings(const char **e0, const char **e1);
int __cdecl ConDrawInput_TextFieldFirstArgChar();
int __cdecl ConDrawInput_GetDvarDescriptionLines(const dvar_s *dvar);
void __cdecl ConDrawInput_DetailedCmdMatch(char *str);
void __cdecl ConDrawInput_CmdMatch(char *str);
void __cdecl Con_DrawAutoCompleteChoice(int localClientNum, bool isDvarCommand, const char *originalCommand);
unsigned int __cdecl Con_GetAutoCompleteColorCodedString(
    char *query,
    const char *matchToText,
    int matchTextLen,
    bool isDvarCommand,
    const char *originalCommand,
    char *colorCoded);
int __cdecl Con_GetAutoCompleteColorCodedStringDiscontiguous(
    const char *query,
    const char *matchToText,
    int matchTextLen,
    char *colorCoded);
int __cdecl Con_GetAutoCompleteColorCodedStringContiguous(
    char *query,
    const char *matchToText,
    int matchTextLen,
    char *colorCoded);
void __cdecl Con_DrawInputPrompt(int localClientNum);
void Con_DrawOuputWindow();
void __cdecl Con_DrawOutputScrollBar(float x, float y, float width, float height);
void __cdecl Con_DrawOutputText(float x, float y);
void __cdecl Con_DrawOutputVersion(float x, float y, float width, float height);
char *__cdecl Con_GetVersionString();
void __cdecl Con_PageUp();
void __cdecl Con_PageDown();
void __cdecl Con_Top();
void __cdecl Con_Bottom();
void __cdecl Con_Close(int localClientNum);
bool __cdecl Con_IsActive(int localClientNum);
void __cdecl CL_PlayTextFXPulseSounds(
    unsigned int localClientNum,
    int currentTime,
    int strLength,
    int fxBirthTime,
    int fxLetterTime,
    int fxDecayStartTime,
    int *soundTimeKeeper);

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
extern int con_inputMaxMatchesShown;


// cl_keys
enum LocSelInputState : __int32
{                                       // ...
    LOC_SEL_INPUT_NONE = 0x0,
    LOC_SEL_INPUT_CONFIRM = 0x1,
    LOC_SEL_INPUT_CANCEL = 0x2,
};
struct keyname_t // sizeof=0x8
{
    const char *name;
    int keynum;
};
struct KeyState // sizeof=0xC
{                                       // ...
    int down;                           // ...
    int repeats;
    const char *binding;                // ...
};
struct PlayerKeyState // sizeof=0xD28
{                                       // ...
    field_t chatField;
    int chat_team;                      // ...
    int overstrikeMode;                 // ...
    int anyKeyDown;                     // ...
    KeyState keys[256];                 // ...
    LocSelInputState locSelInputState;  // ...
};
void __cdecl TRACK_cl_keys();
void __cdecl Field_DrawTextOverride(
    int localClientNum,
    const field_t *edit,
    int x,
    int y,
    int horzAlign,
    int vertAlign,
    char *str,
    int drawLen,
    int cursorPos);
void __cdecl Field_Draw(int localClientNum, field_t *edit, int x, int y, int horzAlign, int vertAlign);
void __cdecl Field_AdjustScroll(const ScreenPlacement *scrPlace, field_t *edit);
void __cdecl Console_Key(int localClientNum, int key);
char __cdecl Field_KeyDownEvent(int localClientNum, const ScreenPlacement *scrPlace, field_t *edit, int key);
char __cdecl Field_Paste(int localClientNum, const ScreenPlacement *scrPlace, field_t *edit);
bool __cdecl Field_CharEvent(int localClientNum, const ScreenPlacement *scrPlace, field_t *edit, int ch);
void CompleteCommand();
void __cdecl PrintMatches(const char *s);
void __cdecl ConcatRemaining(char *src, char *start);
int keyConcatArgs();
void CompleteCmdArgument();
void __cdecl ReplaceConsoleInputArgument(int replaceCount, char *replacement);
void CompleteDvarArgument();
void __cdecl UpdateMatches(bool searchCmds, int *matchLenAfterCmds, int *matchLenAfterDvars);
void __cdecl FindMatches(char *s);
bool __cdecl Console_IsRconCmd(const char *commandString);
char __cdecl Console_IsClientDisconnected();
int __cdecl Key_GetOverstrikeMode(int localClientNum);
void __cdecl Key_SetOverstrikeMode(int localClientNum, int state);
int __cdecl Key_IsDown(int localClientNum, int keynum);
const char *__cdecl Key_KeynumToString(int keynum, int translate);
void __cdecl Key_SetBinding(int localClientNum, int keynum, char *binding);
const char *__cdecl Key_GetBinding(int localClientNum, unsigned int keynum);
int __cdecl Key_GetCommandAssignment(int localClientNum, const char *command, int *twokeys);
int __cdecl Key_GetCommandAssignmentInternal(int localClientNum, const char *command, int *twokeys);
bool __cdecl Key_IsCommandBound(int localClientNum, const char *command);
void __cdecl Key_Unbind_f();
int __cdecl Key_StringToKeynum(const char *str);
void __cdecl Key_Unbindall_f();
void __cdecl Key_Bind_f();
void __cdecl Key_WriteBindings(int localClientNum, int f);
int __cdecl Key_WriteBindingsToBuffer(int localClientNum, char *buffer, int bufferSize);
void __cdecl Key_Bindlist_f();
void __cdecl CL_InitKeyCommands();
bool __cdecl CL_IsConsoleKey(int key);
void __cdecl CL_KeyEvent(int localClientNum, int key, int down, unsigned int time);
void __cdecl Message_Key(int localClientNum, int key);
bool __cdecl CL_MouseInputShouldBypassMenus(int localClientNum, int key);
void __cdecl CL_CharEvent(int localClientNum, int key);
void __cdecl CL_ConsoleCharEvent(int localClientNum, int key);
void __cdecl Key_ClearStates(int localClientNum);
int __cdecl CL_GetKeyBinding(int localClientNum, const char *command, char (*keyNames)[128]);
int __cdecl CL_GetKeyBindingInternal(int localClientNum, const char *command, char (*keyNames)[128]);
void __cdecl Key_Shutdown();
bool __cdecl Key_IsCatcherActive(int localClientNum, int mask);
void __cdecl Key_AddCatcher(int localClientNum, int orMask);
void __cdecl Key_RemoveCatcher(int localClientNum, int andMask);
void __cdecl Key_SetCatcher(int localClientNum, int catcher);

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
    int viewportX,
    int viewportY,
    int viewportWidth,
    int viewportHeight);
void __cdecl ScrPlace_SetupUnsafeViewport(
    ScreenPlacement *scrPlace,
    int viewportX,
    int viewportY,
    int viewportWidth,
    int viewportHeight);
double __cdecl ScrPlace_ApplyX(const ScreenPlacement *scrPlace, float x, int horzAlign);
double __cdecl ScrPlace_ApplyY(const ScreenPlacement *scrPlace, float y, int vertAlign);
void __cdecl ScrPlace_ApplyRect(
    const ScreenPlacement *scrPlace,
    float *x,
    float *y,
    float *w,
    float *h,
    int horzAlign,
    int vertAlign);

extern struct ScreenPlacement scrPlaceView[1];
extern struct ScreenPlacement scrPlaceFull;
extern float cg_hudSplitscreenScale;
extern ScreenPlacement scrPlaceFullUnsafe;

extern PlayerKeyState playerKeys[1];




// con_channels
char __cdecl Con_OpenChannel(char *name, bool allowScript);
bool __cdecl Con_ScriptHasPermission(unsigned int channel);
bool __cdecl Con_GetChannel(const char *name, int *channel_result);
bool __cdecl Con_IsChannelOpen(unsigned int channel);
bool __cdecl Con_IsChannelVisible(print_msg_dest_t dest, unsigned int channel, int errorflags);
void __cdecl Con_WriteFilterConfigString(int f);
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
void __cdecl Con_CloseChannelInternal(unsigned int channel);



// cl_devgui
void __cdecl CL_DevGuiFrame(int localClientNum);
void __cdecl CL_CreateDevGui();
void __cdecl CL_DestroyDevGui();



// cl_debugdata
struct clientDebugStringInfo_t;
struct clientDebugLineInfo_t;

void __cdecl CL_AddDebugString(
    const float *xyz,
    const float *color,
    float scale,
    const char *text,
    int fromServer,
    int duration);
bool __cdecl CreateDebugStringsIfNeeded();
void __cdecl AddDebugStringInternal(
    const float *xyz,
    const float *color,
    float scale,
    const char *text,
    int duration,
    clientDebugStringInfo_t *info);
void __cdecl CL_AddDebugLine(
    const float *start,
    const float *end,
    const float *color,
    int depthTest,
    int duration,
    int fromServer);
bool __cdecl CreateDebugLinesIfNeeded();
void __cdecl AddDebugLineInternal(
    const float *start,
    const float *end,
    const float *color,
    int depthTest,
    int duration,
    clientDebugLineInfo_t *info);
void __cdecl CL_AddDebugStarWithText(
    const float *point,
    const float *starColor,
    const float *textColor,
    const char *string,
    float fontsize,
    int duration,
    int fromServer);
void __cdecl CL_AddDebugStar(const float *point, const float *color, int duration, int fromServer);
void __cdecl CL_FlushDebugClientData();
void __cdecl FlushDebugStrings(clientDebugStringInfo_t *info, int fromServer);
void __cdecl FlushDebugLines(clientDebugLineInfo_t *info, int fromServer);
void __cdecl CL_UpdateDebugClientData();
void __cdecl CL_FlushDebugServerData();
void __cdecl CL_UpdateDebugServerData();
void __cdecl CL_ShutdownDebugData();



// cl_cin
int __cdecl CIN_PlayCinematic(int localClientNum, char *arg);
void __cdecl CL_PlayCinematic_f();
void __cdecl CL_PlayUnskippableCinematic_f();
void __cdecl SCR_DrawCinematic(int localClientNum);
void __cdecl SCR_StopCinematic(int localClientNum);


extern int g_console_field_width;
extern float g_console_char_height;
extern field_t historyEditLines[32];

extern int dvar_modifiedFlags;
