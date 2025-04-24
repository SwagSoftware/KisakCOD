#pragma once
#include "scr_variable.h"

struct debugger_sval_s // sizeof=0x4
{
    debugger_sval_s *next;
};
struct scr_localVar_t // sizeof=0x8
{                                       // ...
    unsigned int name;                  // ...
    unsigned int sourcePos;             // ...
};
struct scr_block_s // sizeof=0x218
{
    int abortLevel;
    int localVarsCreateCount;
    int localVarsPublicCount;
    int localVarsCount;
    unsigned __int8 localVarsInitBits[8];
    scr_localVar_t localVars[64];
};
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
        type = i;
    }
    int type;
    unsigned int stringValue;
    unsigned int idValue;
    float floatValue;
    int intValue;
    sval_u *node;
    unsigned int sourcePosValue;
    const char *codePosValue;
    const char *debugString;
    scr_block_s *block;
};
struct ScriptExpression_t // sizeof=0xC
{                                       // ...
    sval_u parseData;                   // ...
    int breakonExpr;                    // ...
    debugger_sval_s *exprHead;          // ...
};

struct __declspec(align(4)) Scr_SelectedLineInfo // sizeof=0xC
{                                       // ...
    int selectedId;
    int oldSelectedLine;
    bool oldFocusOnSelectedLine;
    // padding byte
    // padding byte
    // padding byte
};

struct Scr_WatchElement_s // sizeof=0x64
{
    ScriptExpression_t expr;
    const char *valueText;
    const char *refText;
    bool directObject;
    // padding byte
    // padding byte
    // padding byte
    unsigned int objectId;
    unsigned __int8 objectType;
    unsigned __int8 oldObjectType;
    bool expand;
    unsigned __int8 breakpointType;
    bool hitBreakpoint;
    bool changed;
    bool valueDefined;
    bool threadList;
    bool endonList;
    // padding byte
    // padding byte
    // padding byte
    VariableValue value;
    unsigned int fieldName;
    unsigned int childCount;
    unsigned int hardcodedCount;
    int id;
    Scr_Breakpoint *breakpoint;
    const char *deadCodePos;
    unsigned int bufferIndex;
    unsigned int sourcePos;
    int changedTime;
    Scr_WatchElement_s *parent;
    Scr_WatchElement_s *childArrayHead;
    Scr_WatchElement_s *childHead;
    Scr_WatchElement_s *next;
};

struct Scr_Breakpoint // sizeof=0x1C
{                                       // ...
    int line;
    unsigned int bufferIndex;
    char *codePos;
    Scr_WatchElement_s *element;
    int builtinIndex;
    Scr_Breakpoint *next;               // ...
    Scr_Breakpoint **prev;
};

struct Scr_OpcodeList_s // sizeof=0x8
{
    char *codePos;
    Scr_OpcodeList_s *next;
};

struct Scr_WatchElementNode_s // sizeof=0x8
{
    Scr_WatchElement_s *element;
    Scr_WatchElementNode_s *next;
};

struct Scr_WatchElementDoubleNode_t // sizeof=0x8
{
    Scr_WatchElementNode_s *list;
    Scr_WatchElementNode_s *removedList;
};


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
    unsigned int nextBreakpointThreadId; // ...
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
    unsigned int removeId;              // ...
    Scr_WatchElementDoubleNode_t **variableBreakpoints; // ...
    bool debugger_inited_main;          // ...
    bool debugger_inited;               // ...
    bool debugger_inited_system;        // ...
    // padding byte
    unsigned int objectId;              // ...
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

void __cdecl TRACK_scr_debugger();
void __cdecl Scr_KeyEvent(int key);
void __cdecl Scr_AddDebugText(char *text);
void __cdecl Scr_AddManualBreakpoint(unsigned __int8 *codePos);
void __cdecl Scr_AddBreakpoint(const unsigned __int8 *codePos);
char *__cdecl Scr_FindBreakpointInfo(const char *codePos);
Scr_Breakpoint *__cdecl Scr_AllocBreakpoint();
void __cdecl Scr_FreeBreakpoint(Scr_Breakpoint *breakpoint);
Scr_WatchElement_s *__cdecl Scr_ReadElement();
void __cdecl Scr_FreeLineBreakpoint(Scr_Breakpoint *breakpoint, bool deleteElement);
void __cdecl Scr_RemoveManualBreakpoint(unsigned __int8 *codePos);
void __cdecl Scr_RemoveBreakpoint(unsigned __int8 *codePos);
void __cdecl Scr_WriteElement(Scr_WatchElement_s *element);
void __cdecl Scr_MonitorCommand(const char *text);
Scr_WatchElement_s *Scr_ResumeBreakpoints();
void __cdecl Scr_SetTempBreakpoint(char *codePos, unsigned int threadId);
void __cdecl Scr_FreeDebugMem(void *ptr);
unsigned int *__cdecl Scr_AllocDebugMem(int size, const char *name);
Scr_WatchElement_s *__cdecl Scr_GetElementRoot(Scr_WatchElement_s *element);
void __cdecl Scr_FreeWatchElementChildrenStrict(Scr_WatchElement_s *element);
void __cdecl Scr_FreeWatchElementChildren(Scr_WatchElement_s *element);
void __cdecl Scr_RemoveValue(Scr_WatchElement_s *element);
void __cdecl Scr_FreeWatchElementText(Scr_WatchElement_s *element);
bool __cdecl Scr_IsSortWatchElement(Scr_WatchElement_s *element);
int __cdecl CompareArrayIndices(unsigned int *arg1, unsigned int *arg2);
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
    unsigned int notifyListOwnerId,
    unsigned int stringValue,
    VariableValue *top,
    char *pos,
    unsigned int localId);
void __cdecl Scr_SpecialBreakpoint(VariableValue *top, char *pos, unsigned int localId, int opcode, int type);
char __cdecl Scr_AllowBreakpoint(char *pos);
Scr_OpcodeList_s *Scr_UnbreakAllAssignmentPos();
void __cdecl Scr_RemoveAssignmentBreakpoint(unsigned __int8 *codePos);
bool __cdecl Scr_RefToVariable(unsigned int id, int isObject);
Scr_OpcodeList_s *Scr_BreakOnAllAssignmentPos();
void __cdecl Scr_AddAssignmentBreakpoint(unsigned __int8 *codePos);
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
int __cdecl Scr_HitBreakpoint(VariableValue *top, char *pos, unsigned int localId, int hitBreakpoint);
bool __cdecl Scr_ConditionalExpression(Scr_WatchElement_s *element, unsigned int localId);
void __cdecl Scr_HitBuiltinBreakpoint(
    VariableValue *top,
    const char *pos,
    unsigned int localId,
    int opcode,
    int builtinIndex,
    unsigned int outparamcount);
void __cdecl Scr_DebugKillThread(unsigned int threadId, const char *codePos);
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
int __cdecl Scr_HitAssignmentBreakpoint(VariableValue *top, char *pos, unsigned int localId, int forceBreak);
bool __cdecl Scr_IgnoreErrors();


extern scrDebuggerGlob_t scrDebuggerGlob;
extern Scr_Breakpoint g_breakpoints[128];
extern Scr_Breakpoint *g_breakpointsHead;

extern unsigned int g_breakonObject;
extern unsigned int g_breakonString;
extern int g_breakonHit;