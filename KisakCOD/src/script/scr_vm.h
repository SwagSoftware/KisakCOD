#pragma once
#include "scr_variable.h"

#include <xanim/xanim.h>

#include <universal/q_shared.h>

struct scr_block_s;
struct Scr_StringNode_s;

void Scr_Error(const char* error);

void __cdecl SCR_Init();
void GScr_GetAnimLength();
void __cdecl Scr_ErrorOnDefaultAsset(XAssetType type, const char* assetName);
void(__cdecl* __cdecl Scr_GetFunction(const char** pName, int* type))();
void __cdecl GScr_AddVector(const float* vVec);
void __cdecl GScr_Shutdown();
void __cdecl Scr_GetObjectField(unsigned int classnum, int entnum, int offset);
void __cdecl GScr_SetDynamicEntityField(gentity_s* ent, unsigned int index);
int __cdecl Scr_GetAnimsIndex(const XAnim_s* anims);
XAnim_s* __cdecl Scr_GetAnims(unsigned int index);
int __cdecl Scr_GetAnimTreeSize(unsigned int parentNode);
void __cdecl Scr_InitFromChildBlocks(scr_block_s** childBlocks, int childCount, scr_block_s* block);
Scr_StringNode_s* __cdecl Scr_GetStringList(const char* filename, char** pBuf);
void __cdecl Scr_InitDebuggerMain();
void __cdecl Scr_ShutdownDebuggerMain();
void __cdecl Scr_InitDebugger();
void __cdecl Scr_ShutdownDebugger();
void __cdecl Scr_InitDebuggerSystem();
void Scr_InitBreakpoints();
void __cdecl Scr_ShutdownDebuggerSystem(int restart);
void __cdecl Scr_ShutdownRemoteClient(int restart);
void __cdecl Scr_InitEvaluate();
void __cdecl Scr_ShutdownEvaluate();
int __cdecl Scr_GetFunctionHandle(const char* filename, const char* name);
int __cdecl Scr_GetStringUsage();
void __cdecl Scr_InitOpcodeLookup();
void __cdecl Scr_ShutdownOpcodeLookup();
unsigned int __cdecl Scr_GetFunctionLineNumInternal(const char* buf, unsigned int sourcePos, const char** startLine);
unsigned int __cdecl Scr_GetFunctionInfo(const char* buf, unsigned int sourcePos, char* line);
void __cdecl Scr_InitAllocNode();
void __cdecl Scr_ShutdownAllocNode();
void __cdecl Scr_ShutdownGameStrings();
void __cdecl Scr_InitVariables();
void __cdecl Scr_InitVariableRange(unsigned int begin, unsigned int end);
void __cdecl Scr_InitClassMap();
void __cdecl Scr_ShutdownVariables();
void __cdecl Scr_AddArrayKeys(unsigned int parentId);
void __cdecl TRACK_scr_vm();
void __cdecl Scr_ClearErrorMessage();
void __cdecl Scr_Init();
const dvar_s* Scr_VM_Init();
void __cdecl Scr_Settings(int developer, int developer_script, int abort_on_error);
void __cdecl Scr_Shutdown();
void VM_Shutdown();
void __cdecl Scr_SetLoading(int bLoading);
unsigned int __cdecl Scr_GetNumScriptThreads();
void __cdecl Scr_ClearOutParams();
char* __cdecl Scr_GetReturnPos(unsigned int* localId);
char* __cdecl Scr_GetNextCodepos(VariableValue* top, const char* pos, int opcode, int mode, unsigned int* localId);
void __cdecl VM_CancelNotify(unsigned int notifyListOwnerId, unsigned int startLocalId);
void __cdecl VM_CancelNotifyInternal(
    unsigned int notifyListOwnerId,
    unsigned int startLocalId,
    unsigned int notifyListId,
    unsigned int notifyNameListId,
    unsigned int stringValue);
bool __cdecl Scr_IsEndonThread(unsigned int localId);
unsigned int __cdecl Scr_GetWaittillThreadStackId(unsigned int localId, unsigned int startLocalId);
const char* __cdecl Scr_GetThreadPos(unsigned int localId);
const char* __cdecl Scr_GetStackThreadPos(unsigned int endLocalId, VariableStackBuffer* stackValue, bool killThread);
const char* __cdecl Scr_GetRunningThreadPos(unsigned int localId);
unsigned int __cdecl Scr_GetWaitThreadStackId(unsigned int localId, unsigned int startLocalId);
void __cdecl Scr_NotifyNum(
    unsigned int entnum,
    unsigned int classnum,
    unsigned int stringValue,
    unsigned int paramcount);
void __cdecl VM_Notify(unsigned int notifyListOwnerId, unsigned int stringValue, VariableValue* top);
void __cdecl Scr_TerminateThread(unsigned int localId);
void __cdecl Scr_TerminateRunningThread(unsigned int localId);
void __cdecl Scr_TerminateWaitThread(unsigned int localId, unsigned int startLocalId);
void __cdecl VM_TerminateStack(unsigned int endLocalId, unsigned int startLocalId, VariableStackBuffer* stackValue);
void __cdecl Scr_TerminateWaittillThread(unsigned int localId, unsigned int startLocalId);
void __cdecl Scr_CancelNotifyList(unsigned int notifyListOwnerId);
void __cdecl VM_TrimStack(unsigned int startLocalId, VariableStackBuffer* stackValue, bool fromEndon);
void __cdecl Scr_CancelWaittill(unsigned int startLocalId);
unsigned __int16 __cdecl Scr_ExecThread(int handle, unsigned int paramcount);
unsigned int __cdecl VM_Execute(unsigned int localId, const char* pos, unsigned int paramcount);
unsigned int __cdecl VM_Execute_0();
VariableValueInternal_u __cdecl GetDummyObject();
unsigned int __cdecl GetDummyFieldValue();
void VM_PrintJumpHistory();
VariableStackBuffer* __cdecl VM_ArchiveStack();
unsigned __int16 __cdecl Scr_ExecEntThreadNum(
    unsigned int entnum,
    unsigned int classnum,
    int handle,
    unsigned int paramcount);
void __cdecl Scr_AddExecThread(int handle, unsigned int paramcount);
void __cdecl Scr_FreeThread(unsigned __int16 handle);
void __cdecl Scr_ExecCode(const char* pos, unsigned int localId);
void __cdecl Scr_InitSystem(int sys);
void __cdecl Scr_ShutdownSystem(unsigned __int8 sys, int bComplete);
void __cdecl VM_TerminateTime(unsigned int timeId);
BOOL __cdecl Scr_IsSystemActive();
VariableUnion __cdecl Scr_GetInt(unsigned int index);
scr_anim_s __cdecl Scr_GetAnim(unsigned int index, XAnimTree_s* tree);
BOOL Scr_ErrorInternal();
double __cdecl Scr_GetFloat(unsigned int index);
VariableUnion __cdecl Scr_GetConstString(unsigned int index);
VariableUnion __cdecl Scr_GetConstLowercaseString(unsigned int index);
char* __cdecl Scr_GetString(unsigned int index);
VariableUnion __cdecl Scr_GetConstStringIncludeNull(unsigned int index);
char* __cdecl Scr_GetDebugString(unsigned int index);
VariableUnion __cdecl Scr_GetConstIString(unsigned int index);
char* __cdecl Scr_GetIString(unsigned int index);
void __cdecl Scr_GetVector(unsigned int index, float* vectorValue);
scr_entref_t __cdecl Scr_GetEntityRef(unsigned int index);
VariableUnion __cdecl Scr_GetObject(unsigned int index);
int __cdecl Scr_GetType(unsigned int index);
const char* __cdecl Scr_GetTypeName(unsigned int index);
unsigned int __cdecl Scr_GetPointerType(unsigned int index);
unsigned int __cdecl Scr_GetNumParam();
void __cdecl Scr_AddBool(unsigned int value);
void IncInParam();
void __cdecl Scr_AddInt(int value);
void __cdecl Scr_AddFloat(VariableUnion value);
void __cdecl Scr_AddAnim(scr_anim_s value);
void __cdecl Scr_AddUndefined();
void __cdecl Scr_AddObject(unsigned int id);
void __cdecl Scr_AddEntityNum(unsigned int entnum, unsigned int classnum);
void __cdecl Scr_AddStruct();
void __cdecl Scr_AddString(char* value);
void __cdecl Scr_AddIString(char* value);
void __cdecl Scr_AddConstString(unsigned int value);
void __cdecl Scr_AddVector(float* value);
void __cdecl Scr_MakeArray();
void __cdecl Scr_AddArray();
void __cdecl Scr_AddArrayStringIndexed(unsigned int stringValue);
void __cdecl Scr_Error(const char* error);
void __cdecl Scr_SetErrorMessage(char* error);
void __cdecl Scr_TerminalError(const char* error);
void __cdecl Scr_NeverTerminalError(char* error);
void __cdecl Scr_ParamError(unsigned int index, const char* error);
void __cdecl Scr_ObjectError(const char* error);
char __cdecl SetEntityFieldValue(unsigned int classnum, int entnum, int offset, VariableValue* value);
VariableValue __cdecl GetEntityFieldValue(unsigned int classnum, int entnum, int offset);
void __cdecl Scr_SetStructField(unsigned int structId, unsigned int index);
void __cdecl Scr_SetDynamicEntityField(unsigned int entnum, unsigned int classnum, unsigned int index);
void __cdecl Scr_IncTime();
void __cdecl Scr_RunCurrentThreads();
void VM_SetTime();
void __cdecl VM_Resume(unsigned int timeId);
void __cdecl VM_UnarchiveStack(unsigned int startLocalId, VariableStackBuffer* stackValue);
int __cdecl Scr_AddLocalVars(unsigned int localId);
void __cdecl Scr_ResetTimeout();
BOOL __cdecl Scr_IsStackClear();
void __cdecl Scr_StackClear();
void __cdecl Scr_ProfileUpdate();
void __cdecl Scr_ProfileBuiltinUpdate();
void __cdecl Scr_DoProfile(float minTime);
void __cdecl Scr_DoProfileBuiltin(float minTime);
char __cdecl Scr_PrintProfileBuiltinTimes(float minTime);
int __cdecl Scr_BuiltinCompare(_DWORD* a, _DWORD* b);
