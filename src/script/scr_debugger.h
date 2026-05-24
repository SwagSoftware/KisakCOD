#pragma once
#include "scr_variable.h"
#include "scr_parser.h"
#include "scr_yacc.h"

#include <ui/ui_shared.h>

#define ONLY_LOCAL_CLIENT_NUM 0
#define KEYCATCH_SCRIPT 2

struct debugger_sval_s // sizeof=0x4
{
    debugger_sval_s *next;
};
static_assert(sizeof(debugger_sval_s) == 0x4);

struct scr_localVar_t // sizeof=0x8
{                                       // ...
    uint32_t name;                  // ...
    uint32_t sourcePos;             // ...
};
static_assert(sizeof(scr_localVar_t) == 0x8);

#define LOCAL_VAR_STACK_SIZE 64
#define MAX_SWITCH_CASES 1024

struct scr_block_s // sizeof=0x218
{
    int abortLevel;
    int localVarsCreateCount;
    int localVarsPublicCount;
    int localVarsCount;
    uint8_t localVarsInitBits[8];
    scr_localVar_t localVars[LOCAL_VAR_STACK_SIZE];
};
static_assert(sizeof(scr_block_s) == 0x218);

union sval_u // sizeof=0x4
{                                       // ...
    sval_u& operator=(const sval_u &other)
    {
        this->type = other.type;
        return *this;
    }
    sval_u &operator=(sval_u &other)
    {
        this->type = other.type;
        return *this;
    }



    sval_u()
    {
    }
    sval_u(int i)
    {
        intValue = i;
    }
    Enum_t type;
    uint32_t stringValue;
    uint32_t idValue;
    float floatValue;
    int intValue;
    sval_u *node;
    uint32_t sourcePosValue;
    const char *codePosValue;
    const char *debugString;
    scr_block_s *block;
};
static_assert(sizeof(sval_u) == 0x4);

struct ScriptExpression_t // sizeof=0xC
{                                       // ...
    sval_u parseData;                   // ...
    int breakonExpr;                    // ...
    debugger_sval_s *exprHead;          // ...
};
static_assert(sizeof(ScriptExpression_t) == 0xC);

struct Scr_SelectedLineInfo // sizeof=0xC
{                                       // ...
    int selectedId;
    int oldSelectedLine;
    bool oldFocusOnSelectedLine;
    // padding byte
    // padding byte
    // padding byte
};
static_assert(sizeof(Scr_SelectedLineInfo) == 0xC);

struct Scr_Breakpoint // sizeof=0x1C
{                                       // ...
    int line;
    uint32_t bufferIndex;
    char *codePos;
    struct Scr_WatchElement_s *element;
    int builtinIndex;
    Scr_Breakpoint *next;               // ...
    Scr_Breakpoint **prev;
};
static_assert(sizeof(Scr_Breakpoint) == 0x1C);

struct Scr_WatchElement_s // sizeof=0x64
{
    ScriptExpression_t expr;
    const char *valueText;
    const char *refText;
    bool directObject;
    // padding byte
    // padding byte
    // padding byte
    uint32_t objectId;
    uint8_t objectType;
    uint8_t oldObjectType;
    bool expand;
    uint8_t breakpointType;
    bool hitBreakpoint;
    bool changed;
    bool valueDefined;
    bool threadList;
    bool endonList;
    // padding byte
    // padding byte
    // padding byte
    VariableValue value;
    uint32_t fieldName;
    uint32_t childCount;
    uint32_t hardcodedCount;
    int id;
    Scr_Breakpoint *breakpoint;
    const char *deadCodePos;
    uint32_t bufferIndex;
    uint32_t sourcePos;
    int changedTime;
    Scr_WatchElement_s *parent;
    Scr_WatchElement_s *childArrayHead;
    Scr_WatchElement_s *childHead;
    Scr_WatchElement_s *next;
};
static_assert(sizeof(Scr_WatchElement_s) == 0x64);

struct Scr_OpcodeList_s // sizeof=0x8
{
    char *codePos;
    Scr_OpcodeList_s *next;
};
static_assert(sizeof(Scr_OpcodeList_s) == 0x8);

struct Scr_WatchElementNode_s // sizeof=0x8
{
    Scr_WatchElement_s *element;
    Scr_WatchElementNode_s *next;
};
static_assert(sizeof(Scr_WatchElementNode_s) == 0x8);

struct Scr_WatchElementDoubleNode_t // sizeof=0x8
{
    Scr_WatchElementNode_s *list;
    Scr_WatchElementNode_s *removedList;
};
static_assert(sizeof(Scr_WatchElementDoubleNode_t) == 0x8);

struct scrDebuggerGlob_t // sizeof=0x2B8
{                                       // ...
    int prevMouseTime;                  // ...
    float prevMousePos[2];              // ...
    UI_ScrollPane scriptScrollPane;     // ...
    UI_ScrollPane miscScrollPane;       // ...
    Scr_ScriptList scriptList;          // ...
    Scr_OpenScriptList openScriptList;  // ...
    Scr_ScriptWatch scriptWatch;        // ...
    Scr_ScriptCallStack scriptCallStack; // ...
    UI_VerticalDivider mainWindow;      // ...
    char *breakpoints;                  // ...
    int breakpointOpcode;               // ...
    const char *breakpointCodePos;      // ...
    Scr_SourcePos_t breakpointPos;      // ...
    bool atBreakpoint;                  // ...
    // padding byte
    // padding byte
    // padding byte
    char *nextBreakpointCodePos;        // ...
    uint32_t nextBreakpointThreadId; // ...
    bool nextBreakpointCodePosMasked;   // ...
    // padding byte
    // padding byte
    // padding byte
    char *killThreadCodePos;            // ...
    bool kill_thread;                   // ...
    // padding byte
    // padding byte
    // padding byte
    VariableValue *breakpointTop;       // ...
    bool run_debugger;                  // ...
    // padding byte
    // padding byte
    // padding byte
    int step_mode;                      // ...
    Scr_OpcodeList_s *assignHead;       // ...
    char *assignHeadCodePos;            // ...
    bool assignBreakpointSet;           // ...
    bool add;                           // ...
    // padding byte
    // padding byte
    Scr_WatchElement_s *currentElement; // ...
    uint32_t removeId;              // ...
    Scr_WatchElementDoubleNode_t **variableBreakpoints; // ...
    bool debugger_inited_main;          // ...
    bool debugger_inited;               // ...
    bool debugger_inited_system;        // ...
    // padding byte
    uint32_t objectId;              // ...
    char *colBuf;                       // ...
    int prevBreakpointLineNum;          // ...
    bool disableBreakpoints;            // ...
    bool showConsole;                   // ...
    // padding byte
    // padding byte
    volatile int disableDebuggerRemote;
    int breakpointCount;                // ...
    int gainFocusTime;                  // ...
};
static_assert(sizeof(scrDebuggerGlob_t) == 0x2B8);

void __cdecl TRACK_scr_debugger();
void __cdecl Scr_KeyEvent(int key);
void __cdecl Scr_AddDebugText(char *text);
void __cdecl Scr_AddManualBreakpoint(uint8_t *codePos);
void __cdecl Scr_AddBreakpoint(const uint8_t *codePos);
char *__cdecl Scr_FindBreakpointInfo(const char *codePos);
Scr_Breakpoint *__cdecl Scr_AllocBreakpoint();
void __cdecl Scr_FreeBreakpoint(Scr_Breakpoint *breakpoint);
Scr_WatchElement_s *__cdecl Scr_ReadElement();
void __cdecl Scr_FreeLineBreakpoint(Scr_Breakpoint *breakpoint, bool deleteElement);
void __cdecl Scr_RemoveManualBreakpoint(uint8_t *codePos);
void __cdecl Scr_RemoveBreakpoint(uint8_t *codePos);
void __cdecl Scr_WriteElement(Scr_WatchElement_s *element);
void __cdecl Scr_MonitorCommand(const char *text);
Scr_WatchElement_s *Scr_ResumeBreakpoints();
void __cdecl Scr_SetTempBreakpoint(char *codePos, uint32_t threadId);
void __cdecl Scr_FreeDebugMem(void *ptr);
uint32_t *__cdecl Scr_AllocDebugMem(int size, const char *name);
Scr_WatchElement_s *__cdecl Scr_GetElementRoot(Scr_WatchElement_s *element);
void __cdecl Scr_FreeWatchElementChildrenStrict(Scr_WatchElement_s *element);
void __cdecl Scr_FreeWatchElementChildren(Scr_WatchElement_s *element);
void __cdecl Scr_RemoveValue(Scr_WatchElement_s *element);
void __cdecl Scr_FreeWatchElementText(Scr_WatchElement_s *element);
bool __cdecl Scr_IsSortWatchElement(Scr_WatchElement_s *element);
int __cdecl CompareArrayIndices(uint32_t *arg1, uint32_t *arg2);
void __cdecl Scr_DeltaElementValueText(Scr_WatchElement_s *element, const char *oldValueText);
void __cdecl Scr_SetNonFieldElementRefText(Scr_WatchElement_s *element);
void __cdecl Scr_PostSetText(Scr_WatchElement_s *element);
const char *__cdecl Scr_GetElementThreadPos(Scr_WatchElement_s *element);
void __cdecl Scr_SetElementRefText(Scr_WatchElement_s *element, char *fieldText);
void __cdecl Scr_ConnectElementChildren(Scr_WatchElement_s *parentElement);
void __cdecl Scr_SortElementChildren(Scr_WatchElement_s *parentElement);
int __cdecl CompareThreadElements(int *arg1, int *arg2);
Scr_WatchElement_s *__cdecl Scr_CreateWatchElement(char *text, Scr_WatchElement_s **prevElem, const char *name);
void __cdecl Scr_Evaluate();
void __cdecl Scr_CheckBreakonNotify(
    uint32_t notifyListOwnerId,
    uint32_t stringValue,
    VariableValue *top,
    char *pos,
    uint32_t localId);
void __cdecl Scr_SpecialBreakpoint(VariableValue *top, char *pos, uint32_t localId, int opcode, int type);
char __cdecl Scr_AllowBreakpoint(char *pos);
Scr_OpcodeList_s *Scr_UnbreakAllAssignmentPos();
void __cdecl Scr_RemoveAssignmentBreakpoint(uint8_t *codePos);
bool __cdecl Scr_RefToVariable(uint32_t id, int isObject);
Scr_OpcodeList_s *Scr_BreakOnAllAssignmentPos();
void __cdecl Scr_AddAssignmentBreakpoint(uint8_t *codePos);
void Scr_Step();
void __cdecl Scr_InitDebuggerMain();
void __cdecl Scr_ShutdownDebuggerMain();
void __cdecl Scr_InitDebugger();
void __cdecl Scr_ShutdownDebugger();
void __cdecl Scr_InitDebuggerSystem();
void Scr_InitBreakpoints();
void __cdecl Scr_ShutdownDebuggerSystem(int restart);
void __cdecl Scr_AddAssignmentPos(char *codePos);
void __cdecl Scr_RunDebuggerRemote();
void __cdecl Scr_RunDebugger();
Scr_WatchElement_s *Scr_DisplayDebugger();
void __cdecl Scr_WatchElementHitBreakpoint(Scr_WatchElement_s *element, bool enabled);
void __cdecl Scr_ShowConsole();
void Scr_HitBreakpointInternal();
int __cdecl Scr_HitBreakpoint(VariableValue *top, char *pos, uint32_t localId, int hitBreakpoint);
bool __cdecl Scr_ConditionalExpression(Scr_WatchElement_s *element, uint32_t localId);
void __cdecl Scr_HitBuiltinBreakpoint(
    VariableValue *top,
    const char *pos,
    uint32_t localId,
    int opcode,
    int builtinIndex,
    uint32_t outparamcount);
void __cdecl Scr_DebugKillThread(uint32_t threadId, const char *codePos);
void __cdecl Scr_DebugTerminateThread(int topThread);
int __cdecl Scr_UpdateDebugSocket();
void Scr_ToggleBreakpointRemote();
void Scr_SelectScriptLineRemote();
void __cdecl Scr_SetMiscScrollPaneComp(struct UI_LinesComponent *comp);
void Scr_UpdateWatchHeightRemote();
void Scr_SelectElementRemote();
void Scr_ToggleExpandElementRemote();
void Scr_PasteElementRemote();
Scr_WatchElement_s *Scr_InsertElementRemote();
void Scr_DeleteElementRemote();
Scr_WatchElement_s *Scr_BackspaceElementRemote();
void Scr_FreeWatchElementChildrenRemote();
bool Scr_SetElementObjectTypeRemote();
int Scr_SetElementThreadStartRemote();
void Scr_SetElementValueTextRemote();
void Scr_SetNonFieldRefTextRemote();
void Scr_SetElementRefTextRemote();
void Scr_SortElementChildrenRemote();
void Scr_SetChildCountRemote();
void Scr_AddTextRemote();
void Scr_CloneElementRemote();
void Scr_ToggleWatchElementBreakpointRemote();
void Scr_UpdateRemote();
void Scr_HitBreakpointRemote();
void Scr_WatchElementHitBreakpointRemote();
void Scr_KeepAliveRemote();
void Scr_SortHitBreakpointsTopRemote();
void __cdecl Sys_ConsolePrintRemote(int localClientNum);
void __cdecl Scr_UpdateDebugger();
char __cdecl Scr_WatchElementHasSameValue(Scr_WatchElement_s *element, VariableValue *newValue);
int __cdecl Scr_HitAssignmentBreakpoint(VariableValue *top, char *pos, uint32_t localId, int forceBreak);
bool __cdecl Scr_IgnoreErrors();

void Scr_EnableBreakpoints(bool enable);
bool Scr_CanDrawScript();
void __cdecl Scr_DrawScript();

void Scr_UpdateRemoteDebugger();

extern scrDebuggerGlob_t scrDebuggerGlob;
extern Scr_Breakpoint g_breakpoints[128];
extern Scr_Breakpoint *g_breakpointsHead;

extern uint32_t g_breakonObject;
extern uint32_t g_breakonString;
extern int g_breakonHit;

#ifdef KISAK_SP
inline bool g_kisakScriptDebuggerHack = false;
#endif