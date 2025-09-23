#include "scr_vm.h"

#include "scr_animtree.h"
#include "scr_debugger.h"
#include "scr_parser.h"
#include "scr_main.h"
#include "scr_stringlist.h"

#include <database/database.h>

#include <bgame/bg_local.h>

#ifdef KISAK_MP
#include <game_mp/g_public_mp.h>
#elif KISAK_SP
#include <game/g_scr_main.h>
#include <game/g_main.h>
#include <game/g_local.h>
#include <game/actor_script_cmd.h>
#endif

#include <universal/com_memory.h>
#include <universal/com_files.h>
#include <win32/win_net_debug.h>
#include <win32/win_input.h>
#include <client/client.h>
#include <qcommon/mem_track.h>
#include "scr_evaluate.h"
#include "scr_memorytree.h"
#include "scr_parsetree.h"
#include <universal/profile.h>
#include <gfx_d3d/r_dvars.h>
#include <qcommon/threads.h>
#include "scr_compiler.h"

#include <setjmp.h>


void Log(char const *format, ...)
{
    char buffer[4096];
    static bool bFirst = true;
    FILE *logFile;

    if (bFirst) {
        logFile = fopen("F:\\swaglord.txt", "w"); // create new log
        fprintf(logFile, "--Start of log--\n");
        bFirst = false;
    }
    else {
        logFile = fopen("F:\\swaglord.txt", "a"); // append to log
    }
    setbuf(logFile, NULL); // Turn off buffered I/O, decreases performance but if crash occurs, no unflushed buffer.
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 4096, format, args);
    fprintf(logFile, buffer);
    va_end(args);
    fclose(logFile);
}

#undef GetObject

#pragma warning(push)
#pragma warning( disable : 4146 ) // LWSS: disable C4146 `unary minus operator applied to unsigned type, result still unsigned`. This is used somewhat properly here (-Scr_ReadUnsignedShort())

scrVmPub_t scrVmPub;
scrVmGlob_t scrVmGlob;
jmp_buf g_script_error[33];
scrVmDebugPub_t scrVmDebugPub;

function_stack_t fs;

const dvar_s *logScriptTimes;

int opcode;
int caseCount;
int thread_count;

void __cdecl GScr_AddVector(const float* vVec)
{
    if (vVec)
        Scr_AddVector(vVec);
    else
        Scr_AddUndefined();
}

Scr_StringNode_s* __cdecl Scr_GetStringList(const char* filename, char** pBuf)
{
    Scr_StringNode_s* v3; // eax
    Scr_StringNode_s* head; // [esp+4h] [ebp-1Ch] BYREF
    char* buf; // [esp+8h] [ebp-18h]
    char* end; // [esp+Ch] [ebp-14h]
    int len; // [esp+10h] [ebp-10h]
    int f; // [esp+14h] [ebp-Ch] BYREF
    Scr_StringNode_s** pTail; // [esp+18h] [ebp-8h]
    char* text; // [esp+1Ch] [ebp-4h]

    len = FS_FOpenFileByMode((char*)filename, &f, FS_READ);
    if (len >= 0)
    {
        buf = (char*)Hunk_AllocDebugMem(len + 1);
        *pBuf = buf;
        FS_Read((unsigned char*)buf, len, f);
        buf[len] = 0;
        FS_FCloseFile(f);
        head = 0;
        pTail = &head;
        end = buf;
        for (text = buf; *text; text = end)
        {
            while (*end != 10 && *end != 13)
            {
                if (!*end)
                    goto LABEL_10;
                ++end;
            }
            *end++ = 0;
        LABEL_10:
            if (*end == 10)
                ++end;
            v3 = (Scr_StringNode_s*)Hunk_AllocDebugMem(8);
            *pTail = v3;
            v3->text = text;
            v3->next = 0;
            pTail = &v3->next;
        }
        return head;
    }
    else
    {
        *pBuf = 0;
        return 0;
    }
}

int __cdecl Scr_GetFunctionHandle(const char* filename, const char* name)
{
    VariableValue v3; // [esp+10h] [ebp-34h]
    unsigned int str; // [esp+18h] [ebp-2Ch]
    int result; // [esp+24h] [ebp-20h]
    unsigned int name2; // [esp+28h] [ebp-1Ch]
    unsigned int posId; // [esp+2Ch] [ebp-18h]
    unsigned int threadId; // [esp+30h] [ebp-14h]
    unsigned int fileId; // [esp+38h] [ebp-Ch]
    unsigned int nameId; // [esp+3Ch] [ebp-8h]
    unsigned int id; // [esp+40h] [ebp-4h]

    if (!scrCompilePub.scripts)
        MyAssertHandler(".\\script\\scr_main.cpp", 69, 0, "%s", "scrCompilePub.scripts");
    if (strlen(filename) >= 0x40)
        MyAssertHandler(".\\script\\scr_main.cpp", 70, 0, "%s", "strlen( filename ) < MAX_QPATH");
    name2 = Scr_CreateCanonicalFilename(filename);
    fileId = FindVariable(scrCompilePub.scripts, name2);
    SL_RemoveRefToString(name2);
    if (!fileId)
        return 0;
    id = FindObject(fileId);
    if (!id)
        MyAssertHandler(".\\script\\scr_main.cpp", 80, 0, "%s", "id");
    str = SL_FindLowercaseString(name).prev;
    if (!str)
        return 0;
    nameId = FindVariable(id, str);
    if (!nameId)
        return 0;
    if (GetValueType(nameId) != 1)
        return 0;
    threadId = FindObject(nameId);
    if (!threadId)
        MyAssertHandler(".\\script\\scr_main.cpp", 92, 0, "%s", "threadId");
    posId = FindVariable(threadId, 1u);
    if (!posId)
        MyAssertHandler(".\\script\\scr_main.cpp", 95, 0, "%s", "posId");
    v3 = Scr_EvalVariable(posId);
    if (v3.type != 7 && v3.type != 12)
        MyAssertHandler(
            ".\\script\\scr_main.cpp",
            99,
            0,
            "%s",
            "pos.type == VAR_CODE::pos || pos.type == VAR_DEVELOPER_CODE::pos");
    if (!Scr_IsInOpcodeMemory(v3.u.codePosValue))
        return 0;
    result = v3.u.intValue - (unsigned int)scrVarPub.programBuffer;
    if ((const char*)v3.u.intValue == scrVarPub.programBuffer)
        MyAssertHandler(".\\script\\scr_main.cpp", 106, 0, "%s", "result");
    return result;
}

int __cdecl Scr_GetStringUsage()
{
    return scrMemTreeGlob.totalAllocBuckets;
}

void __cdecl Scr_ShutdownGameStrings()
{
    SL_ShutdownSystem(1);
}

void __cdecl TRACK_scr_vm()
{
    track_static_alloc_internal(&scrVmGlob, 8232, "scrVmGlob", 7);
    track_static_alloc_internal(&scrVmPub, 17192, "scrVmPub", 7);
    track_static_alloc_internal(g_script_error, 2112, "g_script_error", 7);
    track_static_alloc_internal(&scrVmDebugPub, 147984, "scrVmDebugPub", 0);
}

void __cdecl Scr_ClearErrorMessage()
{
    scrVarPub.error_message = 0;
    scrVmGlob.dialog_error_message = 0;
    scrVarPub.error_index = 0;
}

void __cdecl Scr_Init()
{
    if (scrVarPub.bInited)
        MyAssertHandler(".\\script\\scr_vm.cpp", 169, 0, "%s", "!scrVarPub.bInited");
    Scr_InitClassMap();
    Scr_VM_Init();
    scrCompilePub.script_loading = 0;
    scrAnimPub.animtree_loading = 0;
    scrCompilePub.scripts = 0;
    scrCompilePub.loadedscripts = 0;
    scrAnimPub.animtrees = 0;
    scrCompilePub.builtinMeth = 0;
    scrCompilePub.builtinFunc = 0;
    scrVarPub.bInited = 1;
}

const dvar_s* Scr_VM_Init()
{
    const dvar_s* result; // eax

    scrVarPub.varUsagePos = "<script init variable>";
    scrVmPub.maxstack = &scrVmPub.stack[2047];
    scrVmPub.top = scrVmPub.stack;
    scrVmPub.function_count = 0;
    scrVmPub.function_frame = scrVmPub.function_frame_start;
    scrVmPub.localVars = (unsigned int*)&scrVmGlob.starttime;
    scrVarPub.evaluate = 0;
    scrVmPub.debugCode = 0;
    Scr_ClearErrorMessage();
    scrVmPub.terminal_error = 0;
    scrVmPub.outparamcount = 0;
    scrVmPub.inparamcount = 0;
    scrVarPub.tempVariable = AllocValue();
    scrVarPub.timeArrayId = 0;
    scrVarPub.pauseArrayId = 0;
    scrVarPub.levelId = 0;
    scrVarPub.gameId = 0;
    scrVarPub.animId = 0;
    scrVarPub.freeEntList = 0;
    scrVmPub.stack[0].type = VAR_CODEPOS;
    scrVmGlob.loading = 0;
    scrVmGlob.recordPlace = 0;
    scrVmGlob.lastFileName = 0;
    scrVmGlob.lastLine = 0;
    scrVarPub.ext_threadcount = 0;
    scrVarPub.numScriptThreads = 0;
    scrVarPub.varUsagePos = 0;
    result = Dvar_RegisterBool("logScriptTimes", 0, DVAR_NOFLAG, "Log times for every print called from script");
    logScriptTimes = result;
    return result;
}

void __cdecl Scr_Settings(int developer, int developer_script, int abort_on_error)
{
    iassert(!abort_on_error || developer);

    scrVarPub.developer = developer != 0;
    scrVarPub.developer_script = developer_script != 0;
    scrVmPub.abort_on_error = abort_on_error != 0;

#ifdef _DEBUG // LWSS ADD
    scrVarPub.developer_script = true;
#endif
}

void __cdecl Scr_Shutdown()
{
    if (scrVarPub.bInited)
    {
        scrVarPub.bInited = 0;
        VM_Shutdown();
        Scr_ShutdownVariables();
    }
}

void VM_Shutdown()
{
    if (scrVarPub.tempVariable)
    {
        FreeValue(scrVarPub.tempVariable);
        scrVarPub.tempVariable = 0;
    }
}

void __cdecl Scr_SetLoading(int bLoading)
{
    scrVmGlob.loading = bLoading;
}

unsigned int __cdecl Scr_GetNumScriptThreads()
{
    return scrVarPub.numScriptThreads;
}

void __cdecl Scr_ClearOutParams()
{
    while (scrVmPub.outparamcount)
    {
        RemoveRefToValue(scrVmPub.top->type, scrVmPub.top->u);
        --scrVmPub.top;
        --scrVmPub.outparamcount;
    }
}

char* __cdecl Scr_GetReturnPos(unsigned int* localId)
{
    char* pos; // [esp+0h] [ebp-4h]

    if (scrVmPub.function_count <= 1)
        return 0;
    pos = (char*)scrVmPub.function_frame[-1].fs.pos;
    if (pos == &g_EndPos)
        return 0;
    *localId = scrVmPub.function_frame[-1].fs.localId;
    return pos;
}

char* __cdecl Scr_GetNextCodepos(VariableValue* top, const char* pos, int opcode, int mode, unsigned int* localId)
{
    char* result; // eax
    int type; // eax
    int v7; // ecx
    const char* v8; // eax
    int v9; // [esp+4h] [ebp-7Ch]
    const char* v10; // [esp+38h] [ebp-48h]
    int v11; // [esp+40h] [ebp-40h]
    unsigned __int16 v12; // [esp+44h] [ebp-3Ch]
    unsigned __int16 v13; // [esp+54h] [ebp-2Ch]
    unsigned __int16 v14; // [esp+58h] [ebp-28h]
    unsigned int caseValue; // [esp+6Ch] [ebp-14h]
    int caseCount; // [esp+74h] [ebp-Ch]
    VariableValue value; // [esp+78h] [ebp-8h] BYREF
    const char* posb; // [esp+8Ch] [ebp+Ch]
    const char* posa; // [esp+8Ch] [ebp+Ch]
    const char* posc; // [esp+8Ch] [ebp+Ch]

    *localId = scrVmPub.function_frame->fs.localId;
    while (2)
    {
        ++pos;
        iassert(!scrVarPub.error_message);
        iassert(top);
        iassert(pos);
        iassert(scrVarPub.evaluate);
        if (mode == 2)
        {
            switch (opcode)
            {
            case 'O':
            case 'P':
            case 'T':
                goto $LN58_0;
            case 'Q':
            case 'U':
                goto $LN54_3;
            case 'R':
            case 'V':
                if (top->type != 1)
                    goto LABEL_19;
            $LN58_0:
                if (scrVmPub.function_count >= 32)
                    goto LABEL_19;
                *localId = 0;
                result = *(char**)pos;
                break;
            case 'S':
            case 'W':
                if (top[-1].type != 1)
                    goto LABEL_19;
            $LN54_3:
                if (top->type != 9 || scrVmPub.function_count >= 32)
                    goto LABEL_19;
                *localId = 0;
                result = (char*)top->u.intValue;
                break;
            default:
                goto LABEL_19;
            }
        }
        else
        {
        LABEL_19:
            switch (opcode)
            {
            case 0:
            case 1:
                return Scr_GetReturnPos(localId);
            case 2:
            case 3:
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
            case 20:
            case 24:
            case 25:
            case 26:
            case 27:
            case 28:
            case 29:
            case 32:
            case 33:
            case 35:
            case 36:
            case 37:
            case 38:
            case 49:
            case 52:
            case 53:
            case 54:
            case 57:
            case 60:
            case 76:
            case 77:
            case 78:
            case 81:
            case 83:
            case 88:
            case 89:
            case 91:
            case 92:
            case 93:
            case 100:
            case 101:
            case 102:
            case 103:
            case 104:
            case 105:
            case 106:
            case 107:
            case 108:
            case 109:
            case 110:
            case 111:
            case 112:
            case 113:
            case 114:
            case 115:
            case 116:
            case 117:
            case 118:
            case 120:
            case 121:
            case 122:
            case 123:
            case 126:
            case 127:
            case 128:
            case 135:
            case 136:
            case 137:
                goto LABEL_67;
            case 4:
            case 5:
            case 23:
            case 30:
            case 31:
            case 34:
            case 50:
            case 51:
            case 55:
            case 61:
            case 90:
            case 119:
            case 133:
            case 134:
                ++pos;
                goto LABEL_67;
            case 6:
            case 7:
            case 62:
            case 63:
            case 64:
            case 65:
            case 66:
            case 67:
            case 69:
            case 70:
            case 71:
            case 72:
            case 73:
            case 74:
                pos += 2;
                goto LABEL_67;
            case 8:
            case 19:
            case 21:
            case 79:
            case 80:
            case 82:
            case 85:
            case 87:
                pos += 4;
                goto LABEL_67;
            case 9:
                pos += 4;
                goto LABEL_67;
            case 10:
            case 11:
            case 22:
            case 39:
            case 40:
            case 41:
            case 42:
            case 43:
            case 44:
            case 45:
            case 46:
            case 47:
            case 48:
            case 56:
            case 58:
            case 59:
            case 130:
            case 131:
            case 132:
                pos += 2;
                goto LABEL_67;
            case 12:
                pos += 12;
                goto LABEL_67;
            case 68:
            case 75:
                pos += 3;
                goto LABEL_67;
            case 84:
            case 86:
            case 129:
                pos += 8;
                goto LABEL_67;
            case 94:
            case 96:
                type = top->type;
                value.u.intValue = top->u.intValue;
                value.type = (Vartype_t)type;
                AddRefToValue(type, value.u);
                Scr_CastBool(&value);
                v14 = *(_WORD*)pos;
                pos += 2;
                if (scrVarPub.error_message)
                    goto LABEL_67;
                if (value.type != 6)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 2452, 0, "%s", "value.type == VAR_INTEGER");
                if (value.u.intValue)
                    return (char*)pos;
                else
                    return (char*)&pos[v14];
            case 95:
            case 97:
                v7 = top->type;
                value.u.intValue = top->u.intValue;
                value.type = (Vartype_t)v7;
                AddRefToValue(v7, value.u);
                Scr_CastBool(&value);
                v13 = *(_WORD*)pos;
                pos += 2;
                if (scrVarPub.error_message)
                    goto LABEL_67;
                if (value.type != 6)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 2468, 0, "%s", "value.type == VAR_INTEGER");
                if (value.u.intValue)
                    return (char*)&pos[v13];
                else
                    return (char*)pos;
            case 98:
                return (char*)&pos[*(_DWORD*)pos + 4];
            case 99:
                return (char*)&pos[-*(unsigned __int16*)pos + 2];
            case 124:
                posb = &pos[*(_DWORD*)pos + 4];
                v12 = *(_WORD*)posb;
                posa = posb + 2;
                caseCount = v12;
                v9 = top->type;
                if (v9 == 2)
                {
                    caseValue = top->u.intValue;
                }
                else
                {
                    if (v9 != 6)
                        return (char*)&posa[8 * v12];
                    if (!IsValidArrayIndex(top->u.intValue))
                        return (char*)&posa[8 * v12];
                    caseValue = GetInternalVariableIndex(top->u.intValue);
                }
                if (!v12)
                    return (char*)posa;
                if (!caseValue)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 2516, 0, "%s", "caseValue");
                break;
            case 125:
                return (char*)&pos[8 * *(unsigned __int16*)pos + 2];
            default:
                if (!alwaysfails)
                {
                    v8 = va("unknown opcode %d", opcode);
                    MyAssertHandler(".\\script\\scr_vm.cpp", 2704, 0, v8);
                }
            LABEL_67:
                Scr_ClearErrorMessage();
                opcode = *pos;
                if (opcode == 57)
                    continue;
                return (char*)pos;
            }
            do
            {
                v11 = *(_DWORD*)posa;
                posc = posa + 4;
                v10 = *(const char**)posc;
                posa = posc + 4;
                if (v11 == caseValue)
                {
                    if (!v10)
                        MyAssertHandler(".\\script\\scr_vm.cpp", 2525, 0, "%s", "pos");
                    return (char*)v10;
                }
                --caseCount;
            } while (caseCount);
            if (!v11)
            {
                posa = v10;
                if (!v10)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 2535, 0, "%s", "pos");
            }
            return (char*)posa;
        }
        return result;
    }
}

void __cdecl VM_CancelNotify(unsigned int notifyListOwnerId, unsigned int startLocalId)
{
    unsigned int Variable; // eax
    unsigned __int16 ThreadNotifyName; // ax
    unsigned int v4; // eax
    unsigned int stringValue; // [esp+0h] [ebp-Ch]
    unsigned int notifyListId; // [esp+4h] [ebp-8h]
    VariableValueInternal_u notifyNameListId; // [esp+8h] [ebp-4h]

    Variable = FindVariable(notifyListOwnerId, 0x18000u);
    notifyListId = FindObject(Variable);
    ThreadNotifyName = Scr_GetThreadNotifyName(startLocalId);
    stringValue = ThreadNotifyName;
    if (!ThreadNotifyName)
        MyAssertHandler(".\\script\\scr_vm.cpp", 2750, 0, "%s", "stringValue");
    v4 = FindVariable(notifyListId, stringValue);
    notifyNameListId = FindObject(v4);
    VM_CancelNotifyInternal(notifyListOwnerId, startLocalId, notifyListId, notifyNameListId.u.stringValue, stringValue);
}

void __cdecl VM_CancelNotifyInternal(
    unsigned int notifyListOwnerId,
    unsigned int startLocalId,
    unsigned int notifyListId,
    unsigned int notifyNameListId,
    unsigned int stringValue)
{
    unsigned int Variable; // eax
    unsigned int v6; // eax

    if (stringValue != Scr_GetThreadNotifyName(startLocalId))
        MyAssertHandler(".\\script\\scr_vm.cpp", 2724, 0, "%s", "stringValue == Scr_GetThreadNotifyName( startLocalId )");
    Variable = FindVariable(notifyListOwnerId, 0x18000u);
    if (notifyListId != FindObject(Variable))
        MyAssertHandler(
            ".\\script\\scr_vm.cpp",
            2725,
            0,
            "%s",
            "notifyListId == FindObject( FindVariable( notifyListOwnerId, OBJECT_NOTIFY_LIST ) )");
    v6 = FindVariable(notifyListId, stringValue);
    if (notifyNameListId != FindObject(v6))
        MyAssertHandler(
            ".\\script\\scr_vm.cpp",
            2726,
            0,
            "%s",
            "notifyNameListId == FindObject( FindVariable( notifyListId, stringValue ) )");
    Scr_RemoveThreadNotifyName(startLocalId);
    RemoveObjectVariable(notifyNameListId, startLocalId);
    if (!GetArraySize(notifyNameListId))
    {
        RemoveVariable(notifyListId, stringValue);
        if (!GetArraySize(notifyListId))
            RemoveVariable(notifyListOwnerId, 0x18000u);
    }
}

bool __cdecl Scr_IsEndonThread(unsigned int localId)
{
    unsigned int stackId; // [esp+0h] [ebp-8h]
    unsigned int type; // [esp+4h] [ebp-4h]

    if (GetObjectType(localId) != 15)
        return 0;
    if (GetStartLocalId(localId) != localId)
        return 0;
    stackId = Scr_GetWaittillThreadStackId(localId, localId);
    type = GetValueType(stackId);
    if (type)
    {
        if (type != 10)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3348, 0, "%s", "(type == VAR_UNDEFINED) || (type == VAR_STACK)");
    }
    return type == 0;
}

unsigned int __cdecl Scr_GetWaittillThreadStackId(unsigned int localId, unsigned int startLocalId)
{
    unsigned __int16 ThreadNotifyName; // ax
    unsigned int ObjectVariable; // eax
    VariableValueInternal_u Object; // eax
    unsigned int v5; // eax
    VariableValueInternal_u* VariableValueAddress; // eax
    unsigned int Variable; // eax
    unsigned int v8; // eax
    VariableValueInternal_u v9; // eax
    unsigned int stringValue; // [esp+0h] [ebp-18h]
    VariableValueInternal_u notifyListId; // [esp+8h] [ebp-10h]
    unsigned int selfId; // [esp+14h] [ebp-4h]

    if (!scrVarPub.developer)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3318, 0, "%s", "scrVarPub.developer");
    ThreadNotifyName = Scr_GetThreadNotifyName(startLocalId);
    stringValue = ThreadNotifyName;
    if (!ThreadNotifyName)
        return FindVariable(startLocalId, 0x18001u);
    selfId = Scr_GetSelf(startLocalId);
    ObjectVariable = FindObjectVariable(scrVarPub.pauseArrayId, selfId);
    Object = FindObject(ObjectVariable);
    v5 = FindObjectVariable(Object.u.stringValue, startLocalId);
    VariableValueAddress = GetVariableValueAddress(v5);
    Variable = FindVariable(VariableValueAddress->u.intValue, 0x18000u);
    notifyListId = FindObject(Variable);
    v8 = FindVariable(notifyListId.u.stringValue, stringValue);
    v9 = FindObject(v8);
    return FindObjectVariable(v9.u.stringValue, startLocalId);
}

const char* __cdecl Scr_GetThreadPos(unsigned int localId)
{
    unsigned int ValueType; // eax
    unsigned int v3; // eax
    VariableValueInternal_u* VariableValueAddress; // eax
    unsigned int ObjectType; // [esp+0h] [ebp-10h]
    unsigned int stackId; // [esp+8h] [ebp-8h]
    unsigned int startLocalId; // [esp+Ch] [ebp-4h]

    if (!scrVarPub.developer)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3386, 0, "%s", "scrVarPub.developer");
    startLocalId = GetStartLocalId(localId);
    ObjectType = GetObjectType(startLocalId);
    switch (ObjectType)
    {
    case 0xEu:
        return Scr_GetRunningThreadPos(localId);
    case 0xFu:
        stackId = Scr_GetWaittillThreadStackId(localId, startLocalId);
        if (!stackId)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3396, 0, "%s", "stackId");
        if (GetValueType(stackId) != 10)
        {
            ValueType = GetValueType(stackId);
            MyAssertHandler(
                ".\\script\\scr_vm.cpp",
                3397,
                0,
                "%s\n\t(GetValueType( stackId )) = %i",
                "(GetValueType( stackId ) == VAR_STACK)",
                ValueType);
        }
        goto LABEL_21;
    case 0x10u:
        stackId = Scr_GetWaitThreadStackId(localId, startLocalId);
        if (!stackId)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3402, 0, "%s", "stackId");
        if (GetValueType(stackId) != 10)
        {
            v3 = GetValueType(stackId);
            MyAssertHandler(
                ".\\script\\scr_vm.cpp",
                3403,
                0,
                "%s\n\t(GetValueType( stackId )) = %i",
                "(GetValueType( stackId ) == VAR_STACK)",
                v3);
        }
    LABEL_21:
        VariableValueAddress = GetVariableValueAddress(stackId);
        return Scr_GetStackThreadPos(localId, VariableValueAddress->u.stackValue, 0);
    }
    if (!alwaysfails)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3407, 0, "unreachable");
    return 0;
}

const char* __cdecl Scr_GetStackThreadPos(unsigned int endLocalId, VariableStackBuffer* stackValue, bool killThread)
{
    const char* pos; // [esp+0h] [ebp-20h]
    unsigned int startLocalId; // [esp+4h] [ebp-1Ch]
    unsigned int localId; // [esp+8h] [ebp-18h]
    const char* buf; // [esp+Ch] [ebp-14h]
    const char* bufa; // [esp+Ch] [ebp-14h]
    int size; // [esp+10h] [ebp-10h]
    unsigned int parentLocalId; // [esp+14h] [ebp-Ch]
    VariableUnion u; // [esp+18h] [ebp-8h]

    if (!scrVarPub.developer)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3009, 0, "%s", "scrVarPub.developer");
    startLocalId = GetStartLocalId(endLocalId);
    if (!startLocalId)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3012, 0, "%s", "startLocalId");
    size = stackValue->size;
    localId = stackValue->localId;
    buf = &stackValue->buf[5 * size];
    pos = stackValue->pos;
    while (size)
    {
        bufa = buf - 4;
        u.intValue = *(int*)bufa;
        buf = bufa - 1;
        --size;
        if (*buf == 7)
        {
            parentLocalId = GetParentLocalId(localId);
            if (localId == endLocalId)
            {
                if (startLocalId == localId)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 3044, 0, "%s", "startLocalId != localId");
                break;
            }
            if (killThread)
                Scr_DebugKillThread(localId, pos);
            localId = parentLocalId;
            if (!u.codePosValue)
                MyAssertHandler(".\\script\\scr_vm.cpp", 3039, 0, "%s", "u.codePosValue");
            pos = (const char*)u.intValue;
        }
    }
#ifndef DEDICATED
    if (killThread)
        Scr_DebugKillThread(localId, pos);
#endif
    return pos;
}

const char* __cdecl Scr_GetRunningThreadPos(unsigned int localId)
{
    int function_count; // [esp+4h] [ebp-8h]

    if (!scrVarPub.developer)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3177, 0, "%s", "scrVarPub.developer");
    if (!scrVmPub.function_count)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3179, 0, "%s", "scrVmPub.function_count");
    for (function_count = scrVmPub.function_count; function_count; --function_count)
    {
        if (scrVmPub.function_frame_start[function_count].fs.localId == localId)
            return &g_EndPos != (char*)scrVmPub.stack[3 * function_count - 96].u.intValue
            ? (const char*)scrVmPub.stack[3 * function_count - 96].u.intValue
            : 0;
    }
    if (!alwaysfails)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3191, 0, "unreachable");
    return 0;
}

unsigned int __cdecl Scr_GetWaitThreadStackId(unsigned int localId, unsigned int startLocalId)
{
    unsigned int Variable; // eax
    unsigned int time; // [esp+0h] [ebp-8h]
    VariableValueInternal_u id; // [esp+4h] [ebp-4h]

    if (!scrVarPub.developer)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3231, 0, "%s", "scrVarPub.developer");
    time = Scr_GetThreadWaitTime(startLocalId);
    Variable = FindVariable(scrVarPub.timeArrayId, time);
    id = FindObject(Variable);
    return FindObjectVariable(id.u.stringValue, startLocalId);
}

void __cdecl Scr_NotifyNum(
    unsigned int entnum,
    unsigned int classnum,
    unsigned int stringValue,
    unsigned int paramcount)
{
    VariableValue* top; // [esp+34h] [ebp-14h]
    const char* varUsagePos; // [esp+38h] [ebp-10h]
    VariableValue* startTop; // [esp+3Ch] [ebp-Ch]
    int type; // [esp+40h] [ebp-8h]
    unsigned int id; // [esp+44h] [ebp-4h]
    unsigned int paramcounta; // [esp+5Ch] [ebp+14h]

    PROF_SCOPED("Scr_NotifyNum");

    SL_CheckExists(stringValue);
    varUsagePos = scrVarPub.varUsagePos;
    if (!scrVarPub.varUsagePos)
        scrVarPub.varUsagePos = "<script notify variable>";
    if (!scrVarPub.timeArrayId)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3670, 0, "%s", "scrVarPub.timeArrayId");
    if (paramcount > scrVmPub.inparamcount)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3671, 0, "%s", "paramcount <= scrVmPub.inparamcount");
    Scr_ClearOutParams();
    startTop = &scrVmPub.top[-(int)paramcount];
    paramcounta = scrVmPub.inparamcount - paramcount;
    id = FindEntityId(entnum, classnum);
    if (id)
    {
        if (scrVmDebugPub.checkBreakon)
        {
            top = scrVmPub.top;
            scrVmPub.inparamcount = 0;
            Scr_CheckBreakonNotify(id, stringValue, scrVmPub.top, 0, 0);
            scrVmPub.top = top;
            if (scrVmPub.outparamcount)
                MyAssertHandler(".\\script\\scr_vm.cpp", 3690, 0, "%s", "!scrVmPub.outparamcount");
        }
        type = startTop->type;
        startTop->type = VAR_PRECODEPOS;
        scrVmPub.inparamcount = 0;
        VM_Notify(id, stringValue, scrVmPub.top);
        startTop->type = (Vartype_t)type;
    }
    while (scrVmPub.top != startTop)
    {
        RemoveRefToValue(scrVmPub.top->type, scrVmPub.top->u);
        --scrVmPub.top;
    }
    scrVmPub.inparamcount = paramcounta;
    if (scrVmPub.outparamcount)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3708, 0, "%s", "!scrVmPub.outparamcount");
    scrVarPub.varUsagePos = varUsagePos;
    SL_CheckExists(stringValue);
}

void __cdecl VM_Notify(unsigned int notifyListOwnerId, unsigned int stringValue, VariableValue* top)
{
    unsigned int ObjectVariable; // eax
    Vartype_t type; // edx
    unsigned int Variable; // eax
    VariableValueInternal_u Array; // eax
    unsigned int v7; // [esp-4h] [ebp-64h]
    VariableStackBuffer *stackValue; // [esp+0h] [ebp-60h]
    unsigned int notifyListIndex; // [esp+4h] [ebp-5Ch]
    VariableValue tempValue2; // [esp+8h] [ebp-58h] BYREF
    VariableValue tempValue3; // [esp+10h] [ebp-50h] BYREF
    unsigned int stackId; // [esp+18h] [ebp-48h]
    unsigned int startLocalId; // [esp+1Ch] [ebp-44h]
    VariableStackBuffer* newStackValue; // [esp+20h] [ebp-40h]
    VariableValue tempValue5; // [esp+24h] [ebp-3Ch] BYREF
    VariableValue* currentValue; // [esp+2Ch] [ebp-34h]
    char* buf; // [esp+30h] [ebp-30h]
    unsigned int selfNameId; // [esp+34h] [ebp-2Ch]
    int size; // [esp+38h] [ebp-28h]
    int len; // [esp+3Ch] [ebp-24h]
    unsigned int notifyListId; // [esp+40h] [ebp-20h]
    unsigned int notifyNameListId; // [esp+44h] [ebp-1Ch]
    int newSize; // [esp+48h] [ebp-18h]
    int bufLen; // [esp+4Ch] [ebp-14h]
    bool bNoStack; // [esp+53h] [ebp-Dh]
    VariableValueInternal_u* tempValue; // [esp+54h] [ebp-Ch]
    unsigned int selfId; // [esp+58h] [ebp-8h]
    unsigned int notifyListEntry; // [esp+5Ch] [ebp-4h]

    notifyListId = FindVariable(notifyListOwnerId, 0x18000u);
    if (notifyListId)
    {
        notifyListId = FindObject(notifyListId);
        iassert(notifyListId);
        notifyNameListId = FindVariable(notifyListId, stringValue);
        if (notifyNameListId)
        {
            notifyNameListId = FindObject(notifyNameListId);
            iassert(notifyNameListId);
            AddRefToObject(notifyNameListId);
            iassert(!scrVarPub.evaluate);
            scrVarPub.evaluate = 1;
            notifyListEntry = notifyNameListId;

            while (1)
            {
                notifyListIndex = FindLastSibling(notifyListEntry);
            next:
                if (!notifyListIndex)
                    break;
                notifyListEntry = Scr_GetVarId(notifyListIndex);
                iassert(notifyListEntry);
                startLocalId = GetVariableKeyObject(notifyListEntry);
                selfId = Scr_GetSelf(startLocalId);
                ObjectVariable = FindObjectVariable(scrVarPub.pauseArrayId, selfId);
                selfNameId = FindObject(ObjectVariable);
                if (GetValueType(notifyListEntry))
                {
                    iassert(GetValueType(notifyListEntry) == VAR_STACK);
                    tempValue = GetVariableValueAddress(notifyListEntry);
                    stackValue = tempValue->u.stackValue;
                    if (*((byte *)stackValue->pos - 1) == OP_waittillmatch)
                    {
                        size = *stackValue->pos;
                        iassert(size >= 0);
                        iassert(size <= stackValue->size);
                        buf = &stackValue->buf[5 * (stackValue->size - size)];

                        for (currentValue = top; size; --currentValue)
                        {
                            iassert(currentValue->type != VAR_CODEPOS);
                            if (currentValue->type == VAR_PRECODEPOS)
                            {
LABEL_30:
                                notifyListIndex = FindPrevSibling(notifyListIndex);
                                goto next;
                            }
                            --size;
                            tempValue3.type = (Vartype_t)*(unsigned char*)buf;
                            buf += 1;
                            iassert(tempValue3.type != VAR_CODEPOS);

                            if (tempValue3.type == VAR_PRECODEPOS)
                                break;

                            tempValue3.u.codePosValue = *(const char**)buf;
                            buf += 4;

                            AddRefToValue(tempValue3.type, tempValue3.u);
                            type = currentValue->type;
                            tempValue2.u = currentValue->u;
                            tempValue2.type = type;
                            AddRefToValue(type, tempValue2.u);
                            Scr_EvalEquality(&tempValue3, &tempValue2);

                            if (scrVarPub.error_message)
                            {
                                RuntimeError(
                                    (char*)stackValue->pos,
                                    *stackValue->pos - size + 3,
                                    scrVarPub.error_message,
                                    scrVmGlob.dialog_error_message);
                                Scr_ClearErrorMessage();
                                notifyListIndex = FindPrevSibling(notifyListIndex);
                                goto next;
                            }

                            iassert(tempValue3.type == VAR_INTEGER);
                            if (!tempValue3.u.intValue)
                                goto LABEL_30;
                        }
                        ++stackValue->pos;
                        bNoStack = 1;
                    }
                    else
                    {
                        iassert(top->type != VAR_CODEPOS);
                        bNoStack = top->type == VAR_PRECODEPOS;
                    }
                    tempValue5.type = VAR_STACK;
                    tempValue5.u.stackValue = stackValue;
                    v7 = startLocalId;
                    Variable = GetVariable(scrVarPub.timeArrayId, scrVarPub.time);
                    Array = GetArray(Variable);
                    stackId = GetNewObjectVariable(Array, v7);
                    SetNewVariableValue(stackId, &tempValue5);
                    tempValue = GetVariableValueAddress(stackId);
                    VM_CancelNotifyInternal(notifyListOwnerId, startLocalId, notifyListId, notifyNameListId, stringValue);
                    RemoveObjectVariable(selfNameId, startLocalId);
                    if (!GetArraySize(selfNameId))
                        RemoveObjectVariable(scrVarPub.pauseArrayId, selfId);
                    Scr_SetThreadWaitTime(startLocalId, scrVarPub.time);
                    if (bNoStack)
                    {
                        notifyListEntry = notifyNameListId;
                    }
                    else
                    {
                        iassert(top->type != VAR_PRECODEPOS);
                        iassert(top->type != VAR_CODEPOS);
                        size = stackValue->size;
                        newSize = size;
                        currentValue = top;

                        do
                        {
                            ++newSize;
                            --currentValue;
                            iassert(currentValue->type != VAR_CODEPOS);
                        } while (currentValue->type != VAR_PRECODEPOS);

                        iassert(newSize >= 0 && newSize < (1 << 16));

                        len = 5 * size;
                        //bufLen = 5 * newSize + 11;
                        bufLen = 5 * newSize + (sizeof(VariableStackBuffer)-1);

                        if (!MT_Realloc(stackValue->bufLen, bufLen))
                        {
                            newStackValue = (VariableStackBuffer*)MT_Alloc(bufLen, 1);
                            newStackValue->bufLen = bufLen;
                            newStackValue->pos = stackValue->pos;
                            newStackValue->localId = stackValue->localId;
                            memcpy(newStackValue->buf, stackValue->buf, len);
                            MT_Free((unsigned char*)stackValue, stackValue->bufLen);
                            stackValue = newStackValue;
                            tempValue->u.stackValue = newStackValue;
                        }

                        stackValue->size = newSize;
                        buf = &stackValue->buf[len];
                        newSize -= size;

                        iassert(newSize);

                        do
                        {
                            ++currentValue;
                            AddRefToValue(currentValue->type, currentValue->u);
                            iassert((unsigned)currentValue->type < VAR_COUNT);
                            *buf++ = currentValue->type;
                            *(const char**)buf = currentValue->u.codePosValue;
                            buf += 4;
                            --newSize;
                        } while (newSize);

                        iassert(buf - (const char *)stackValue == bufLen);
                        notifyListEntry = notifyNameListId;
                    }
                }
                else
                {
                    VM_CancelNotifyInternal(notifyListOwnerId, startLocalId, notifyListId, notifyNameListId, stringValue);
                    Scr_KillEndonThread(startLocalId);
                    RemoveObjectVariable(selfNameId, startLocalId);
                    if (!GetArraySize(selfNameId))
                        RemoveObjectVariable(scrVarPub.pauseArrayId, selfId);
                    Scr_TerminateThread(selfId);
                    notifyListEntry = notifyNameListId;
                }
            }
            RemoveRefToObject(notifyNameListId);
            iassert(scrVarPub.evaluate);
            scrVarPub.evaluate = 0;
        }
    }
}

void __cdecl Scr_TerminateThread(unsigned int localId)
{
    unsigned int ObjectType; // [esp+0h] [ebp-8h]
    unsigned int startLocalId; // [esp+4h] [ebp-4h]

    startLocalId = GetStartLocalId(localId);
    ObjectType = GetObjectType(startLocalId);
    switch (ObjectType)
    {
    case 0xEu:
        Scr_TerminateRunningThread(localId);
        break;
    case 0xFu:
        Scr_TerminateWaittillThread(localId, startLocalId);
        break;
    case 0x10u:
        Scr_TerminateWaitThread(localId, startLocalId);
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3373, 0, "unreachable");
        break;
    }
}

void __cdecl Scr_TerminateRunningThread(unsigned int localId)
{
    int function_count; // [esp+0h] [ebp-Ch]
    int topThread; // [esp+4h] [ebp-8h]
    unsigned int threadId; // [esp+8h] [ebp-4h]

    iassert(scrVmPub.function_count);

    function_count = scrVmPub.function_count;
    topThread = scrVmPub.function_count;

    while (1)
    {
        iassert(function_count);

        threadId = scrVmPub.function_frame_start[function_count].fs.localId;
        if (threadId == localId)
            break;
        --function_count;
        if (!GetSafeParentLocalId(threadId))
            topThread = function_count;
    }
    while (topThread >= function_count)
    {
        if (scrVarPub.developer)
        {
            Scr_DebugTerminateThread(topThread);
        }
        else
        {
            scrVmPub.function_frame_start[topThread].fs.pos = &g_EndPos;
            //scrVmPub.stack[3 * topThread - 96].u.intValue = (int)&g_EndPos;
        }
        --topThread;
    }
}

void __cdecl Scr_TerminateWaitThread(unsigned int localId, unsigned int startLocalId)
{
    unsigned int Variable; // eax
    VariableStackBuffer* stackValue; // [esp+0h] [ebp-10h]
    unsigned int stackId; // [esp+4h] [ebp-Ch]
    unsigned int time; // [esp+8h] [ebp-8h]
    unsigned int id; // [esp+Ch] [ebp-4h]

    time = Scr_GetThreadWaitTime(startLocalId);
    Scr_ClearWaitTime(startLocalId);
    Variable = FindVariable(scrVarPub.timeArrayId, time);
    id = FindObject(Variable);
    stackId = FindObjectVariable(id, startLocalId);
    if (!stackId)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3208, 0, "%s", "stackId");
    if (GetValueType(stackId) != 10)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3209, 0, "%s", "GetValueType( stackId ) == VAR_STACK");
    stackValue = GetVariableValueAddress(stackId)->u.stackValue;
    if (scrVarPub.developer)
        Scr_GetStackThreadPos(localId, stackValue, 1);
    RemoveObjectVariable(id, startLocalId);
    if (!GetArraySize(id) && time != scrVarPub.time)
        RemoveVariable(scrVarPub.timeArrayId, time);
    VM_TerminateStack(localId, startLocalId, stackValue);
}

void __cdecl VM_TerminateStack(unsigned int endLocalId, unsigned int startLocalId, VariableStackBuffer* stackValue)
{
    unsigned int Variable; // eax
    VariableValueInternal_u Array; // eax
    unsigned int stackId; // [esp+0h] [ebp-24h]
    unsigned int localId; // [esp+4h] [ebp-20h]
    char* buf; // [esp+8h] [ebp-1Ch]
    const char* bufa; // [esp+8h] [ebp-1Ch]
    int size; // [esp+Ch] [ebp-18h]
    int sizea; // [esp+Ch] [ebp-18h]
    unsigned int parentLocalId; // [esp+10h] [ebp-14h]
    const char* u; // [esp+14h] [ebp-10h]
    VariableValue tempValue; // [esp+1Ch] [ebp-8h] BYREF

    if (!startLocalId)
        MyAssertHandler(".\\script\\scr_vm.cpp", 2932, 0, "%s", "startLocalId");
    size = stackValue->size;
    localId = stackValue->localId;
    buf = &stackValue->buf[5 * size];
    while (size)
    {
        bufa = buf - 4;
        u = *(const char**)bufa;
        buf = (char*)bufa - 1;
        --size;
        if (*buf == 7)
        {
            parentLocalId = GetParentLocalId(localId);
            Scr_KillThread(localId);
            RemoveRefToObject(localId);
            if (localId == endLocalId)
            {
                if (startLocalId == localId)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 2963, 0, "%s", "startLocalId != localId");
                sizea = size + 1;
                *buf = 0;
                if (stackValue->size < sizea)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 2968, 0, "%s", "stackValue->size >= size");
                Scr_SetThreadWaitTime(startLocalId, scrVarPub.time);
                if (!u)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 2971, 0, "%s", "u.codePosValue");
                stackValue->pos = u;
                stackValue->localId = parentLocalId;
                stackValue->size = sizea;
                tempValue.type = VAR_STACK;
                tempValue.u.stackValue = stackValue;
                Variable = GetVariable(scrVarPub.timeArrayId, scrVarPub.time);
                Array = GetArray(Variable);
                stackId = GetNewObjectVariable(Array.u.stringValue, startLocalId);
                SetNewVariableValue(stackId, &tempValue);
                return;
            }
            localId = parentLocalId;
        }
        else
        {
            RemoveRefToValue(*(unsigned __int8*)buf, (VariableUnion)u);
        }
    }
    if (localId != endLocalId)
        MyAssertHandler(".\\script\\scr_vm.cpp", 2984, 0, "%s", "localId == endLocalId");
    if (startLocalId != localId)
        MyAssertHandler(".\\script\\scr_vm.cpp", 2985, 0, "%s", "startLocalId == localId");
    Scr_KillThread(localId);
    RemoveRefToObject(localId);
    --scrVarPub.numScriptThreads;
    MT_Free((byte*)stackValue, stackValue->bufLen);
}

void __cdecl Scr_TerminateWaittillThread(unsigned int localId, unsigned int startLocalId)
{
    unsigned __int16 ThreadNotifyName; // ax
    unsigned int ObjectVariable; // eax
    unsigned int v4; // eax
    unsigned int Variable; // eax
    unsigned int v6; // eax
    VariableStackBuffer* stackValue; // [esp+0h] [ebp-20h]
    unsigned int stringValue; // [esp+4h] [ebp-1Ch]
    unsigned int stackId; // [esp+8h] [ebp-18h]
    unsigned int stackIda; // [esp+8h] [ebp-18h]
    unsigned int selfNameId; // [esp+Ch] [ebp-14h]
    unsigned int notifyListId; // [esp+10h] [ebp-10h]
    unsigned int notifyNameListId; // [esp+14h] [ebp-Ch]
    VariableUnion notifyListOwnerId; // [esp+18h] [ebp-8h]
    unsigned int selfId; // [esp+1Ch] [ebp-4h]

    ThreadNotifyName = Scr_GetThreadNotifyName(startLocalId);
    stringValue = ThreadNotifyName;
    if (ThreadNotifyName)
    {
        selfId = Scr_GetSelf(startLocalId);
        ObjectVariable = FindObjectVariable(scrVarPub.pauseArrayId, selfId);
        selfNameId = FindObject(ObjectVariable);
        v4 = FindObjectVariable(selfNameId, startLocalId);
        notifyListOwnerId.intValue = GetVariableValueAddress(v4)->u.intValue;
        Variable = FindVariable(notifyListOwnerId.stringValue, 0x18000u);
        notifyListId = FindObject(Variable);
        v6 = FindVariable(notifyListId, stringValue);
        notifyNameListId = FindObject(v6);
        stackId = FindObjectVariable(notifyNameListId, startLocalId);
        if (!stackId)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3276, 0, "%s", "stackId");
        if (GetValueType(stackId) != 10)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3277, 0, "%s", "GetValueType( stackId ) == VAR_STACK");
        stackValue = (VariableStackBuffer*)GetVariableValueAddress(stackId)->u.intValue;
        if (scrVarPub.developer)
            Scr_GetStackThreadPos(localId, stackValue, 1);
        VM_CancelNotifyInternal(notifyListOwnerId.stringValue, startLocalId, notifyListId, notifyNameListId, stringValue);
        RemoveObjectVariable(selfNameId, startLocalId);
        if (!GetArraySize(selfNameId))
            RemoveObjectVariable(scrVarPub.pauseArrayId, selfId);
    }
    else
    {
        stackIda = FindVariable(startLocalId, 0x18001u);
        if (!stackIda)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3293, 0, "%s", "stackId");
        if (GetValueType(stackIda) != 10)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3294, 0, "%s", "GetValueType( stackId ) == VAR_STACK");
        stackValue = (VariableStackBuffer*)GetVariableValueAddress(stackIda)->u.intValue;
        if (scrVarPub.developer)
            Scr_GetStackThreadPos(localId, stackValue, 1);
        RemoveVariable(startLocalId, 0x18001u);
    }
    VM_TerminateStack(localId, startLocalId, stackValue);
}

void __cdecl Scr_CancelNotifyList(unsigned int notifyListOwnerId)
{
    VariableValueInternal_u* VariableValueAddress; // eax
    VariableStackBuffer* stackValue; // [esp+0h] [ebp-1Ch]
    unsigned int stackId; // [esp+4h] [ebp-18h]
    unsigned int startLocalId; // [esp+8h] [ebp-14h]
    unsigned int selfStartLocalId; // [esp+Ch] [ebp-10h]
    unsigned int notifyListId; // [esp+10h] [ebp-Ch]
    unsigned int notifyNameListId; // [esp+14h] [ebp-8h]
    unsigned int selfLocalId; // [esp+18h] [ebp-4h]

    while (1)
    {
        notifyListId = FindVariable(notifyListOwnerId, 0x18000u);
        if (!notifyListId)
            break;
        notifyListId = FindObject(notifyListId);
        iassert(notifyListId);
        notifyNameListId = FindFirstSibling(notifyListId);
        if (!notifyNameListId)
            break;
        notifyNameListId = FindObject(notifyNameListId);
        iassert(notifyNameListId);
        stackId = FindFirstSibling(notifyNameListId);
        if (!stackId)
            break;
        startLocalId = GetVariableKeyObject(stackId);
        iassert(startLocalId);
        if (GetValueType(stackId) == VAR_STACK)
        {
            stackValue = (VariableStackBuffer*)GetVariableValueAddress(stackId)->u.intValue;
            Scr_CancelWaittill(startLocalId);
            VM_TrimStack(startLocalId, stackValue, 0);
        }
        else
        {
            AddRefToObject(startLocalId);
            Scr_CancelWaittill(startLocalId);
            selfLocalId = Scr_GetSelf(startLocalId);
            selfStartLocalId = GetStartLocalId(selfLocalId);
            stackId = FindVariable(selfStartLocalId, 0x18001u);
            if (stackId)
            {
                iassert(!Scr_GetThreadNotifyName(selfStartLocalId));
                iassert(GetValueType(stackId) == VAR_STACK);
                VariableValueAddress = GetVariableValueAddress(stackId);
                stackValue = (VariableStackBuffer*)VariableValueAddress->u.intValue;
                iassert(!stackValue->pos);
                VM_TrimStack(selfStartLocalId, stackValue, 1);
            }
            Scr_KillEndonThread(startLocalId);
            RemoveRefToEmptyObject(startLocalId);
        }
    }
}

void __cdecl VM_TrimStack(unsigned int startLocalId, VariableStackBuffer* stackValue, bool fromEndon)
{
    unsigned int NewVariable; // eax
    unsigned int localId; // [esp+0h] [ebp-20h]
    const char* buf; // [esp+4h] [ebp-1Ch]
    const char* bufa; // [esp+4h] [ebp-1Ch]
    int size; // [esp+8h] [ebp-18h]
    unsigned int parentLocalId; // [esp+Ch] [ebp-14h]
    VariableUnion u; // [esp+10h] [ebp-10h]
    VariableValue tempValue; // [esp+18h] [ebp-8h] BYREF

    iassert(startLocalId);

    size = stackValue->size;
    localId = stackValue->localId;
    buf = &stackValue->buf[5 * size];
    while (size)
    {
        bufa = buf - 4;
        u.intValue = *(int*)bufa;
        buf = bufa - 1;
        --size;
        if (*buf == 7)
        {
            if (FindObjectVariable(scrVarPub.pauseArrayId, localId))
            {
                iassert(startLocalId != localId);
                stackValue->localId = localId;
                stackValue->size = size + 1;
                Scr_StopThread(localId);
                if (!fromEndon)
                {
                    Scr_SetThreadNotifyName(startLocalId, 0);
                    stackValue->pos = 0;
                    tempValue.type = VAR_STACK;
                    tempValue.u.intValue = (int)stackValue;
                    NewVariable = GetNewVariable(startLocalId, 0x18001u);
                    SetNewVariableValue(NewVariable, &tempValue);
                }
                return;
            }
            parentLocalId = GetParentLocalId(localId);
            Scr_KillThread(localId);
            RemoveRefToObject(localId);
            localId = parentLocalId;
        }
        else
        {
            RemoveRefToValue(*(unsigned __int8*)buf, u);
        }
    }
    iassert(startLocalId == localId);
    if (fromEndon)
        RemoveVariable(startLocalId, 0x18001u);
    Scr_KillThread(startLocalId);
    RemoveRefToObject(startLocalId);
    --scrVarPub.numScriptThreads;
    MT_Free((unsigned char*)stackValue, stackValue->bufLen);
}

void __cdecl Scr_CancelWaittill(unsigned int startLocalId)
{
    unsigned int ObjectVariable; // eax
    unsigned int v2; // eax
    VariableValueInternal_u* VariableValueAddress; // eax
    VariableValueInternal_u selfNameId; // [esp+0h] [ebp-Ch]
    unsigned int selfId; // [esp+8h] [ebp-4h]

    selfId = Scr_GetSelf(startLocalId);
    ObjectVariable = FindObjectVariable(scrVarPub.pauseArrayId, selfId);
    selfNameId = FindObject(ObjectVariable);
    v2 = FindObjectVariable(selfNameId.u.stringValue, startLocalId);
    VariableValueAddress = GetVariableValueAddress(v2);
    VM_CancelNotify(VariableValueAddress->u.intValue, startLocalId);
    RemoveObjectVariable(selfNameId.u.stringValue, startLocalId);
    if (!GetArraySize(selfNameId.u.stringValue))
        RemoveObjectVariable(scrVarPub.pauseArrayId, selfId);
}

void VM_PrintJumpHistory()
{
    const char *pos; // [esp+0h] [ebp-8h]
    int index; // [esp+4h] [ebp-4h]

    Com_Printf(23, "********************************\n");
    Com_Printf(23, "Recent loop history (from most recent) :\n");
    index = scrVmDebugPub.jumpbackHistoryIndex;
    do
    {
        if (!index)
            index = 128;
        pos = scrVmDebugPub.jumpbackHistory[--index];
        if (!pos)
            break;
        Scr_PrintPrevCodePos(23, (char *)pos, 0);
    } while (index != scrVmDebugPub.jumpbackHistoryIndex);
    Com_Printf(23, "********************************\n");
}

VariableStackBuffer *__cdecl VM_ArchiveStack()
{
    VariableStackBuffer *stackValue; // [esp+0h] [ebp-18h]
    VariableValue *top; // [esp+4h] [ebp-14h]
    char *buf; // [esp+8h] [ebp-10h]
    unsigned int localId; // [esp+Ch] [ebp-Ch]
    int size; // [esp+10h] [ebp-8h]
    int bufLen; // [esp+14h] [ebp-4h]

    top = fs.top;
    size = fs.top - fs.startTop;
    if (size != size)
        MyAssertHandler(".\\script\\scr_vm.cpp", 2768, 0, "%s", "size == (unsigned short)size");
    bufLen = 5 * size + 11;
    if (bufLen != bufLen)
        MyAssertHandler(".\\script\\scr_vm.cpp", 2770, 0, "%s", "bufLen == (unsigned short)bufLen");
    stackValue = (VariableStackBuffer*) MT_Alloc(bufLen, 1);
    ++scrVarPub.numScriptThreads;
    localId = fs.localId;
    stackValue->localId = fs.localId;
    stackValue->size = size;
    stackValue->bufLen = bufLen;
    stackValue->pos = fs.pos;
    stackValue->time = scrVarPub.time;
    scrVmPub.localVars -= fs.localVarCount;
    buf = &stackValue->buf[5 * size];
    while (size)
    {
        buf -= 4;
        if (top->type == VAR_CODEPOS)
        {
            --scrVmPub.function_count;
            --scrVmPub.function_frame;
            //*bufa = scrVmPub.function_frame->fs.pos;
            *(uintptr_t *)buf = (uintptr_t)scrVmPub.function_frame->fs.pos;
            scrVmPub.localVars -= scrVmPub.function_frame->fs.localVarCount;
            localId = GetParentLocalId(localId);
        }
        else
        {
            *(uintptr_t*)buf = top->u.pointerValue;
        }
        --buf;
        if (top->type >= 0x100u)
            MyAssertHandler(".\\script\\scr_vm.cpp", 2805, 0, "%s", "top->type >= 0 && top->type < (1 << 8)");
        *buf = top->type;
        --top;
        --size;
    }
    --scrVmPub.function_count;
    --scrVmPub.function_frame;
    AddRefToObject(localId);
    fs.localId = localId;
    return stackValue;
}

unsigned __int16 __cdecl Scr_ExecThread(int handle, unsigned int paramcount)
{
    unsigned int v2; // eax
    const char *pos; // [esp+34h] [ebp-Ch]
    const char *varUsagePos; // [esp+38h] [ebp-8h]
    unsigned int id; // [esp+3Ch] [ebp-4h]

    pos = &scrVarPub.programBuffer[handle];
    if (!scrVmPub.function_count)
    {
        iassert(scrVmPub.localVars == scrVmGlob.localVarsStack - 1);
        //Profile_Begin(332);
        Scr_ResetTimeout();
    }
    iassert(scrVarPub.timeArrayId);
    iassert(handle);

    ++scrVarPub.ext_threadcount;
    Scr_IsInOpcodeMemory(pos);
    varUsagePos = scrVarPub.varUsagePos;
    scrVarPub.varUsagePos = pos + 1;
    AddRefToObject(scrVarPub.levelId);
    v2 = AllocThread(scrVarPub.levelId);
    id = VM_Execute(v2, pos, paramcount);
    scrVarPub.varUsagePos = varUsagePos;
    if (scrVarDebugPub)
        ++scrVarDebugPub->extRefCount[id];
    RemoveRefToValue(scrVmPub.top->type, scrVmPub.top->u);
    scrVmPub.top->type = VAR_UNDEFINED;
    --scrVmPub.top;
    --scrVmPub.inparamcount;
    if (!scrVmPub.function_count)
    {
        //Profile_EndInternal(0);
        iassert(scrVmPub.localVars == scrVmGlob.localVarsStack - 1);
    }
    return id;
}

unsigned int __cdecl GetDummyObject()
{
    ClearVariableValue(scrVarPub.tempVariable);
    return GetObject(scrVarPub.tempVariable);
}

unsigned int __cdecl GetDummyFieldValue()
{
    ClearVariableValue(scrVarPub.tempVariable);
    return scrVarPub.tempVariable;
}

const char *Scr_ReadCodePos(const char **pos)
{
    const char *value = *(reinterpret_cast<const char **>(const_cast<char *>(*pos)));
    *pos += sizeof(const char *);
    return value;
}

uintptr_t Scr_ReadUnsigned(const char **pos)
{
    uintptr_t value = *(reinterpret_cast<const uintptr_t *>(*pos));
    *pos += sizeof(uintptr_t);
    return value;
}

unsigned short Scr_ReadUnsignedShort(const char **pos)
{
    unsigned short value = *(reinterpret_cast<const unsigned short *>(*pos));
    *pos += sizeof(unsigned short);
    return value;
}

const unsigned int *Scr_ReadIntArray(const char **pos, int count)
{
    const unsigned int *value;

    value = reinterpret_cast<const unsigned int *>(*pos);
    *pos += sizeof(unsigned int) * count;
    return value;
}

float Scr_ReadFloat(const char **pos)
{
    float value = *(reinterpret_cast<const float *>(*pos));
    *pos += sizeof(float);
    return value;
}

const float *Scr_ReadVector(const char **pos)
{
    const float *value = reinterpret_cast<const float *>(*pos);
    *pos += (sizeof(float) * 3);
    return value;
}

unsigned int Scr_GetLocalVar(const char *pos)
{
    return scrVmPub.localVars[-*(pos)];
}

VariableStackBuffer *VM_ArchiveStack2(int size, const char *codePos, VariableValue *top, unsigned int localVarCount, unsigned int *localId)
{
    unsigned int id;
    char *buf;
    char *pos;
    VariableStackBuffer *stackBuf;
    int bufLen;

    //bufLen = 5 * size + 11;
    bufLen = 5 * size + sizeof(VariableStackBuffer);

    iassert(size == (unsigned short)size);
    iassert(bufLen == (unsigned short)bufLen);

    stackBuf = (VariableStackBuffer *)MT_Alloc(bufLen, 1);
    ++scrVarPub.numScriptThreads;
    id = *localId;
    stackBuf->localId = *localId;
    stackBuf->size = size;
    stackBuf->bufLen = bufLen;
    stackBuf->pos = codePos;
    stackBuf->time = scrVarPub.time;
    scrVmPub.localVars -= localVarCount;
    buf = &stackBuf->buf[5 * size];

    while (size)
    {
        pos = buf - 4;

        if (top->type == VAR_CODEPOS)
        {
            --scrVmPub.function_count;
            --scrVmPub.function_frame;
            *(intptr_t *)pos = (intptr_t)scrVmPub.function_frame->fs.pos;
            scrVmPub.localVars -= scrVmPub.function_frame->fs.localVarCount;
            id = GetParentLocalId(id);
        }
        else
        {
            *(intptr_t *)pos = (intptr_t)top->u.codePosValue;
        }

        buf = pos - 1;
        iassert(top->type >= 0 && top->type < (1 << 8));
        *buf = top->type;
        --top;
        --size;
    }

    --scrVmPub.function_count;
    --scrVmPub.function_frame;
    AddRefToObject(id);
    *localId = id;

    return stackBuf;
}

unsigned int Scr_EvalArrayIndex(unsigned int parentId, VariableValue *index)
{
    unsigned int stringValue;

    switch (index->type)
    {
    case VAR_INTEGER:
        if (IsValidArrayIndex(index->u.pointerValue))
            return GetArrayVariable(parentId, index->u.pointerValue);
        Scr_Error(va("array index %d out of range", index->u.pointerValue));
        return 0;

    case VAR_STRING:
        stringValue = GetVariable(parentId, index->u.stringValue);
        SL_RemoveRefToString(index->u.stringValue);
        return stringValue;

    default:
        Scr_Error(va("%s is not an array index", var_typename[index->type]));
        return 0;
    }
}

#define INC_TOP() iassert(fs.top >= scrVmPub.stack); ++fs.top; iassert(fs.top <= scrVmPub.maxstack);

unsigned int VM_ExecuteInternal()
{
    //int gOpcode;
    //int gParamCount;
    int builtInTime;
    int time;
    int timeSpent;
    unsigned int parentLocalId;
    unsigned int builtinIndex;
    VariableValue stackValue;
    int jumpOffset;
    unsigned int waitTime;
    unsigned int stringValue;
    unsigned int id;
    unsigned int threadId;
    const char *tempCodePos;
    VariableValue tempValue;
    unsigned int outparamcount;
    unsigned int selfId;
    unsigned int objectId;
    unsigned int fieldValueId;
    unsigned int fieldValueIndex;
    int profileIndex;
    int profileBit;
    //int gCaseCount;
    unsigned int caseValue;
    unsigned int currentCaseValue;
    unsigned int classnum;
    int entnum;
    const char *currentCodePos;
    unsigned char removeCount;
    scr_entref_t entref;
    unsigned int stackId;

    unsigned int profileEnable[33];
    unsigned int *profileEnablePos;
    profileEnable[0] = scrVmDebugPub.profileEnable[fs.localId];
    if (profileEnable[0])
        Profile_BeginScripts(profileEnable[0]);
    profileEnablePos = &profileEnable[0];

    ++g_script_error_level;
    iassert(g_script_error_level <= 32);

    //gParamCount = 0;
#pragma region ERROR_CHECKER
    if (setjmp(g_script_error[g_script_error_level]))
    {
        switch (opcode)
        {
        case OP_EvalLocalArrayRefCached0:
        case OP_EvalLocalArrayRefCached:
        case OP_EvalArrayRef:
        case OP_ClearArray:
        case OP_EvalLocalVariableRef:
            if (scrVarPub.error_index < 0)
                scrVarPub.error_index = 1;
            break;

        case OP_EvalSelfFieldVariable:
        case OP_EvalFieldVariable:
        case OP_ClearFieldVariable:
        case OP_SetVariableField:
        case OP_SetSelfFieldVariableField:
        case OP_inc:
        case OP_dec:
            scrVarPub.error_index = 0;
            break;

        case OP_CallBuiltin0:
        case OP_CallBuiltin1:
        case OP_CallBuiltin2:
        case OP_CallBuiltin3:
        case OP_CallBuiltin4:
        case OP_CallBuiltin5:
        case OP_CallBuiltin:
            if (scrVarPub.error_index > 0)
                scrVarPub.error_index = scrVmPub.outparamcount - scrVarPub.error_index + 1;
            break;

        case OP_CallBuiltinMethod0:
        case OP_CallBuiltinMethod1:
        case OP_CallBuiltinMethod2:
        case OP_CallBuiltinMethod3:
        case OP_CallBuiltinMethod4:
        case OP_CallBuiltinMethod5:
        case OP_CallBuiltinMethod:
            if (scrVarPub.error_index <= 0)
            {
                if (scrVarPub.error_index < 0)
                    scrVarPub.error_index = 1;
            }
            else
            {
                scrVarPub.error_index = scrVmPub.outparamcount - scrVarPub.error_index + 2;
            }
            break;

        default:
            break;
        }

        RuntimeError((char*)fs.pos, scrVarPub.error_index, scrVarPub.error_message, scrVmGlob.dialog_error_message);
        Scr_ClearErrorMessage();

        switch (opcode)
        {
        case OP_EvalLocalArrayCached:
        case OP_EvalArray:
            RemoveRefToValue(fs.top);
            fs.top--;
            RemoveRefToValue(fs.top);
            fs.top->type = VAR_UNDEFINED;
            break;

        case OP_EvalLocalArrayRefCached0:
        case OP_EvalLocalArrayRefCached:
        case OP_EvalArrayRef:
        case OP_EvalLocalVariableRef:
            fieldValueIndex = 0;
            fieldValueId = GetDummyFieldValue();
            RemoveRefToValue(fs.top);
            --fs.top;
            break;

        case OP_ClearArray:
        case OP_wait:
            RemoveRefToValue(fs.top);
            --fs.top;
            break;

        case OP_GetSelfObject:
            objectId = GetDummyObject();
            break;

        case OP_EvalSelfFieldVariable:
        case OP_EvalFieldVariable:
            fs.top->type = VAR_UNDEFINED;
            break;

        case OP_EvalSelfFieldVariableRef:
        case OP_EvalFieldVariableRef:
            fieldValueIndex = 0;
            fieldValueId = GetDummyFieldValue();
            break;

        case OP_ClearFieldVariable:
            if (scrVmPub.outparamcount)
            {
                iassert(scrVmPub.outparamcount == 1);
                iassert(scrVmPub.top->type == VAR_UNDEFINED);
                scrVmPub.outparamcount = 0;
            }
            break;

        case OP_checkclearparams:
            iassert(fs.top->type != VAR_CODEPOS);
            while (fs.top->type != VAR_PRECODEPOS)
            {
                RemoveRefToValue(fs.top);
                --fs.top;
                iassert(fs.top->type != VAR_CODEPOS);
            }
            fs.top->type = VAR_CODEPOS;
            break;

        case OP_SetVariableField:
            if (scrVmPub.outparamcount)
            {
                iassert(scrVmPub.outparamcount == 1);
                iassert(scrVmPub.top == fs.top);
                RemoveRefToValue(fs.top);
                scrVmPub.outparamcount = 0;
            }
            --fs.top;
            break;

        case OP_SetSelfFieldVariableField:
            RemoveRefToValue(fs.top);
            scrVmPub.outparamcount = 0;
            --fs.top;
            break;

        case OP_CallBuiltin0:
        case OP_CallBuiltin1:
        case OP_CallBuiltin2:
        case OP_CallBuiltin3:
        case OP_CallBuiltin4:
        case OP_CallBuiltin5:
        case OP_CallBuiltin:
        case OP_CallBuiltinMethod0:
        case OP_CallBuiltinMethod1:
        case OP_CallBuiltinMethod2:
        case OP_CallBuiltinMethod3:
        case OP_CallBuiltinMethod4:
        case OP_CallBuiltinMethod5:
        case OP_CallBuiltinMethod:
            Scr_ClearOutParams();
            fs.top = scrVmPub.top + 1;
            scrVmPub.top[1].type = VAR_UNDEFINED;
            break;

        case OP_ScriptFunctionCall2:
        case OP_ScriptFunctionCall:
        case OP_ScriptMethodCall:
            Scr_ReadCodePos(&fs.pos);
            goto methodcallpointer;

        case OP_ScriptFunctionCallPointer:
        case OP_ScriptMethodCallPointer:
methodcallpointer:
            iassert(fs.top->type != VAR_CODEPOS);
            while (fs.top->type != VAR_PRECODEPOS)
            {
                RemoveRefToValue(fs.top);
                --fs.top;
                iassert(fs.top->type != VAR_CODEPOS);
            }
            fs.top->type = VAR_UNDEFINED;
            break;

        case OP_ScriptThreadCall:
        case OP_ScriptMethodThreadCall:
            Scr_ReadCodePos(&fs.pos);
            goto scriptmethodthreadcallpointer;

        case OP_ScriptThreadCallPointer:
        case OP_ScriptMethodThreadCallPointer:
scriptmethodthreadcallpointer:
            for (outparamcount = Scr_ReadUnsigned(&fs.pos); outparamcount; --outparamcount)
                RemoveRefToValue(fs.top--);
            ++fs.top;
            fs.top->type = VAR_UNDEFINED;
            break;

        case OP_CastFieldObject:
            objectId = GetDummyObject();
            --fs.top;
            break;

        case OP_EvalLocalVariableObjectCached:
            ++fs.pos;
            objectId = GetDummyObject();
            break;

        case OP_JumpOnFalse:
        case OP_JumpOnTrue:
        case OP_JumpOnFalseExpr:
        case OP_JumpOnTrueExpr:
            Scr_ReadUnsignedShort(&fs.pos);
            --fs.top;
            break;

        case OP_jumpback:
            jumpOffset = Scr_ReadUnsignedShort(&fs.pos);
            fs.pos -= jumpOffset;
            break;

        case OP_bit_or:
        case OP_bit_ex_or:
        case OP_bit_and:
        case OP_equality:
        case OP_inequality:
        case OP_less:
        case OP_greater:
        case OP_less_equal:
        case OP_greater_equal:
        case OP_shift_left:
        case OP_shift_right:
        case OP_plus:
        case OP_minus:
        case OP_multiply:
        case OP_divide:
        case OP_mod:
            --fs.top;
            break;

        case OP_waittillmatch:
            ++fs.pos;
            goto endon;

        case OP_waittill:
        case OP_endon:
endon:
            RemoveRefToValue(fs.top);
            --fs.top;
            RemoveRefToValue(fs.top);
            --fs.top;
            break;

        case OP_notify:
            while (fs.top->type != VAR_PRECODEPOS)
            {
                RemoveRefToValue(fs.top);
                --fs.top;
                iassert(fs.top->type != VAR_CODEPOS);
            }
            RemoveRefToValue(fs.top);
            --fs.top;
            break;

        case OP_switch:
            if (caseCount)
            {
                do
                {
                    currentCaseValue = Scr_ReadUnsigned(&fs.pos);
                    currentCodePos = Scr_ReadCodePos(&fs.pos);
                    --caseCount;
                } while (caseCount);

                if (!currentCaseValue)
                {
                    fs.pos = currentCodePos;
                    iassert(fs.pos);
                }
            }
            RemoveRefToValue(fs.top);
            --fs.top;
            break;

        default:
            break;
        }
        iassert(!scrVmPub.inparamcount);
        iassert(!scrVmPub.outparamcount);
        if (scrVmPub.showError && !scrVmPub.debugCode && !Scr_IgnoreErrors())
        {
            scrVmPub.showError = 0;
            Scr_ShowConsole();
            Scr_HitBreakpoint(fs.top, (char *)fs.pos, fs.localId, 0);
        }
    }
#pragma endregion
    while (2)
    {
        iassert(!scrVarPub.error_message);
        iassert(!scrVarPub.error_index);
        iassert(!scrVmPub.outparamcount);
        iassert(!scrVmPub.inparamcount);

        opcode = *(unsigned char *)fs.pos++;
        // Log("0x%x\n", opcode);
interrupt_return:
        scrVarPub.varUsagePos = fs.pos;
        switch (opcode)
        {
        case OP_End:
            parentLocalId = GetSafeParentLocalId(fs.localId);
            Scr_KillThread(fs.localId);
            scrVmPub.localVars -= fs.localVarCount;
            iassert(fs.top->type != VAR_PRECODEPOS);
            while (fs.top->type != VAR_CODEPOS)
            {
                RemoveRefToValue(fs.top);
                --fs.top;
                iassert(fs.top->type != VAR_PRECODEPOS);
            }
            --scrVmPub.function_count;
            --scrVmPub.function_frame;
            if (!parentLocalId)
            {
                iassert(fs.top == fs.startTop);
thread_end:
                fs.startTop[1].type = VAR_UNDEFINED;
                goto thread_return;
            }
            iassert(fs.top->type == VAR_CODEPOS);
            fs.top->type = VAR_UNDEFINED;
            goto end_0;

        case OP_Return:
            parentLocalId = GetSafeParentLocalId(fs.localId);
            Scr_KillThread(fs.localId);
            scrVmPub.localVars -= fs.localVarCount;
            tempValue.u = fs.top->u;
            tempValue.type = fs.top->type;
            --fs.top;
            iassert(fs.top->type != VAR_PRECODEPOS);
            while (fs.top->type != VAR_CODEPOS)
            {
                RemoveRefToValue(fs.top);
                --fs.top;
                iassert(fs.top->type != VAR_PRECODEPOS);
            }
            --scrVmPub.function_count;
            --scrVmPub.function_frame;
            if (parentLocalId)
            {
                iassert(fs.top->type == VAR_CODEPOS);
                *fs.top = tempValue;
end_0:
                iassert(fs.top != fs.startTop);
                RemoveRefToObject(fs.localId);
                fs.pos = scrVmPub.function_frame->fs.pos;
                iassert(fs.pos);
                fs.localVarCount = scrVmPub.function_frame->fs.localVarCount;
                fs.localId = parentLocalId;
                continue;
            }

            iassert(fs.top == fs.startTop);
            fs.top[1] = tempValue;
thread_return:
            if (*profileEnablePos)
                Profile_EndScripts(*profileEnablePos);
            if (thread_count)
            {
                --profileEnablePos;
                --thread_count;
                RemoveRefToObject(fs.localId);
                fs = scrVmPub.function_frame->fs;
                fs.top->type = scrVmPub.function_frame->topType;
                ++fs.top;
                continue;
            }
            iassert(g_script_error_level >= 0);
            --g_script_error_level;
            return fs.localId;

        case OP_GetUndefined:
            INC_TOP();
            fs.top->type = VAR_UNDEFINED;
            continue;

        case OP_GetZero:
            INC_TOP();
            fs.top->type = VAR_INTEGER;
            fs.top->u.intValue = 0;
            continue;

        case OP_GetByte:
            INC_TOP();
            fs.top->type = VAR_INTEGER;
            fs.top->u.intValue = *(unsigned char *)fs.pos++;
            continue;

        case OP_GetNegByte:
            INC_TOP();
            fs.top->type = VAR_INTEGER;
            fs.top->u.intValue = -*(unsigned char *)fs.pos++;
            continue;

        case OP_GetUnsignedShort:
            INC_TOP();
            fs.top->type = VAR_INTEGER;
            fs.top->u.intValue = Scr_ReadUnsignedShort(&fs.pos);
            continue;

        case OP_GetNegUnsignedShort:
            INC_TOP();
            fs.top->type = VAR_INTEGER;
            fs.top->u.intValue = -Scr_ReadUnsignedShort(&fs.pos);
            continue;

        case OP_GetInteger:
            INC_TOP();
            fs.top->type = VAR_INTEGER;
            fs.top->u.intValue = Scr_ReadUnsigned(&fs.pos);
            continue;

        case OP_GetFloat:
            INC_TOP();
            fs.top->type = VAR_FLOAT;
            fs.top->u.floatValue = Scr_ReadFloat(&fs.pos);
            continue;

        case OP_GetString:
            INC_TOP();
            fs.top->type = VAR_STRING;
            fs.top->u.stringValue = Scr_ReadUnsignedShort(&fs.pos);
            SL_AddRefToString(fs.top->u.stringValue);
            continue;

        case OP_GetIString:
            INC_TOP();
            fs.top->type = VAR_ISTRING;
            fs.top->u.stringValue = Scr_ReadUnsignedShort(&fs.pos);
            SL_AddRefToString(fs.top->u.stringValue);
            continue;

        case OP_GetVector:
            INC_TOP();
            fs.top->type = VAR_VECTOR;
            fs.top->u.vectorValue = Scr_ReadVector(&fs.pos);
            continue;

        case OP_GetLevelObject:
            objectId = scrVarPub.levelId;
            continue;

        case OP_GetAnimObject:
            objectId = scrVarPub.animId;
            continue;

        case OP_GetSelf:
            INC_TOP();
            fs.top->type = VAR_POINTER;
            fs.top->u.pointerValue = Scr_GetSelf(fs.localId);
            AddRefToObject(fs.top->u.pointerValue);
            continue;

        case OP_GetLevel:
            INC_TOP();
            fs.top->type = VAR_POINTER;
            fs.top->u.pointerValue = scrVarPub.levelId;
            AddRefToObject(scrVarPub.levelId);
            continue;

        case OP_GetGame:
            INC_TOP();
            *fs.top = Scr_EvalVariable(scrVarPub.gameId);
            continue;

        case OP_GetAnim:
            INC_TOP();
            fs.top->type = VAR_POINTER;
            fs.top->u.pointerValue = scrVarPub.animId;
            AddRefToObject(scrVarPub.animId);
            continue;

        case OP_GetAnimation:
            INC_TOP();
            fs.top->type = VAR_ANIMATION;
            fs.top->u.pointerValue = Scr_ReadUnsigned(&fs.pos);
            continue;

        case OP_GetGameRef:
            fieldValueIndex = 0;
            fieldValueId = scrVarPub.gameId;
            continue;

        case OP_GetFunction:
            INC_TOP();
            fs.top->type = VAR_FUNCTION;
            fs.top->u.codePosValue = Scr_ReadCodePos(&fs.pos);
            continue;

        case OP_CreateLocalVariable:
            ++scrVmPub.localVars;
            ++fs.localVarCount;
            scrVmPub.localVars[0] = GetNewVariable(fs.localId, Scr_ReadUnsignedShort(&fs.pos));
            continue;

        case OP_RemoveLocalVariables:
            removeCount = *fs.pos++;
            scrVmPub.localVars -= removeCount;
            fs.localVarCount -= removeCount;
            while (removeCount)
            {
                RemoveNextVariable(fs.localId);
                --removeCount;
            }
            continue;

        case OP_EvalLocalVariableCached0:
            INC_TOP();
            *fs.top = Scr_EvalVariable(scrVmPub.localVars[0]);
            continue;

        case OP_EvalLocalVariableCached1:
            INC_TOP();
            *fs.top = Scr_EvalVariable(scrVmPub.localVars[-1]);
            continue;

        case OP_EvalLocalVariableCached2:
            INC_TOP();
            *fs.top = Scr_EvalVariable(scrVmPub.localVars[-2]);
            continue;

        case OP_EvalLocalVariableCached3:
            INC_TOP();
            *fs.top = Scr_EvalVariable(scrVmPub.localVars[-3]);
            continue;

        case OP_EvalLocalVariableCached4:
            INC_TOP();
            *fs.top = Scr_EvalVariable(scrVmPub.localVars[-4]);
            continue;

        case OP_EvalLocalVariableCached5:
            INC_TOP();
            *fs.top = Scr_EvalVariable(scrVmPub.localVars[-5]);
            continue;

        case OP_EvalLocalVariableCached:
            INC_TOP();
            *fs.top = Scr_EvalVariable(Scr_GetLocalVar(fs.pos));
            ++fs.pos;
            continue;

        case OP_EvalLocalArrayCached:
            INC_TOP();
            *fs.top = Scr_EvalVariable(Scr_GetLocalVar(fs.pos));
            ++fs.pos;
            Scr_EvalArray(fs.top, fs.top - 1);
            --fs.top;
            continue;

        case OP_EvalArray:
            Scr_EvalArray(fs.top, fs.top - 1);
            --fs.top;
            continue;

        case OP_EvalLocalArrayRefCached0:
            fieldValueId = scrVmPub.localVars[0];
            goto evalarrayref;

        case OP_EvalLocalArrayRefCached:
            fieldValueId = Scr_GetLocalVar(fs.pos++);
            goto evalarrayref;

        case OP_EvalArrayRef:
evalarrayref:
            objectId = Scr_EvalArrayRef(fieldValueId);
            if (fs.top->type == VAR_INTEGER)
            {
                if (!IsValidArrayIndex(fs.top->u.intValue))
                {
                    Scr_Error(va("array index %d out of range", fs.top->u.intValue));
                }
                fieldValueIndex = GetArrayVariableIndex(objectId, fs.top->u.intValue);
            }
            else if (fs.top->type == VAR_STRING)
            {
                fieldValueIndex = GetVariableIndexInternal(objectId, fs.top->u.stringValue);
                SL_RemoveRefToString(fs.top->u.stringValue);
            }
            else
            {
                Scr_Error(va("%s is not an array index", var_typename[fs.top->type]));
            }
            fieldValueId = Scr_GetVarId(fieldValueIndex);
            iassert(fieldValueId);
            --fs.top;
            continue;

        case OP_ClearArray:
            ClearArray(fieldValueId, fs.top);
            --fs.top;
            continue;

        case OP_EmptyArray:
            ++fs.top;
            fs.top->type = VAR_POINTER;
            fs.top->u.pointerValue = Scr_AllocArray();
            continue;

        case OP_GetSelfObject:
            objectId = Scr_GetSelf(fs.localId);
            if (IsFieldObject(objectId))
                continue;
            Scr_Error(va("%s is not an object", var_typename[GetObjectType(objectId)]));

        case OP_EvalLevelFieldVariable:
            objectId = scrVarPub.levelId;
            INC_TOP();
            *fs.top = Scr_EvalVariable(FindVariable(objectId, Scr_ReadUnsignedShort(&fs.pos)));
            continue;

        case OP_EvalAnimFieldVariable:
            objectId = scrVarPub.animId;
            INC_TOP();
            *fs.top = Scr_EvalVariable(FindVariable(objectId, Scr_ReadUnsignedShort(&fs.pos)));
            continue;

        case OP_EvalSelfFieldVariable:
            objectId = Scr_GetSelf(fs.localId);
            if (IsFieldObject(objectId))
            {
                goto EvalFieldVariable;
            }
            INC_TOP();
            Scr_ReadUnsignedShort(&fs.pos);
            Scr_Error(va("%s is not an object", var_typename[GetObjectType(objectId)]));

        case OP_EvalFieldVariable:
EvalFieldVariable:
            INC_TOP();
            *fs.top = Scr_FindVariableField(objectId, Scr_ReadUnsignedShort(&fs.pos));
            continue;

        case OP_EvalLevelFieldVariableRef:
            objectId = scrVarPub.levelId;
            goto EvalFieldVariableRef;

        case OP_EvalAnimFieldVariableRef:
            objectId = scrVarPub.animId;
            goto EvalFieldVariableRef;

        case OP_EvalSelfFieldVariableRef:
            objectId = Scr_GetSelf(fs.localId);
            goto EvalFieldVariableRef;

        case OP_EvalFieldVariableRef:
EvalFieldVariableRef:
            fieldValueIndex = Scr_GetVariableFieldIndex(objectId, Scr_ReadUnsignedShort(&fs.pos));
            fieldValueId = Scr_GetVarId(fieldValueIndex);
            continue;

        case OP_ClearFieldVariable:
            ClearVariableField(objectId, Scr_ReadUnsignedShort(&fs.pos), fs.top);
            continue;

        case OP_SafeCreateVariableFieldCached:
            ++scrVmPub.localVars;
            ++fs.localVarCount;
            scrVmPub.localVars[0] = GetNewVariable(fs.localId, Scr_ReadUnsignedShort(&fs.pos));
            goto SafeSetVariableFieldCached0;

        case OP_SafeSetVariableFieldCached0:
SafeSetVariableFieldCached0:
            iassert(fs.top->type != VAR_CODEPOS);
            if (fs.top->type != VAR_PRECODEPOS)
            {
                goto setlocalvariablefieldcached0;
            }
            continue;

        case OP_SafeSetVariableFieldCached:
            iassert(fs.top->type != VAR_CODEPOS);
            if (fs.top->type != VAR_PRECODEPOS)
            {
                goto setlocalvariablefieldcached;
            }
            ++fs.pos;
            continue;

        case OP_SafeSetWaittillVariableFieldCached:
            iassert(fs.top->type != VAR_PRECODEPOS);
            if (fs.top->type != VAR_CODEPOS)
            {
                goto setlocalvariablefieldcached;
            }
            ClearVariableValue(Scr_GetLocalVar(fs.pos));
            ++fs.pos;
            continue;

        case OP_clearparams:
            iassert(fs.top->type != VAR_PRECODEPOS);

            while (fs.top->type != VAR_CODEPOS)
            {
                RemoveRefToValue(fs.top);
                --fs.top;
                iassert(fs.top->type != VAR_PRECODEPOS);
            }
            continue;

        case OP_checkclearparams:
            iassert(fs.top->type != VAR_CODEPOS);
            if (fs.top->type == VAR_PRECODEPOS)
            {
                if (scrVarPub.numScriptValues > 0xF37E || scrVarPub.numScriptObjects > 0x7380)
                {
                    if (scrVmPub.showError)
                    {
                        Scr_DumpScriptThreads();
                        Scr_DumpScriptVariablesDefault();
                        Scr_Error("exceeded maximum number of script variables");
                    }
                    Sys_Error("exceeded maximum number of script variables");
                }
                fs.top->type = VAR_CODEPOS;
            }
            else
            {
                Scr_Error("function called with too many parameters");
            }
            continue;

        case OP_EvalLocalVariableRefCached0:
            fieldValueIndex = 0;
            fieldValueId = scrVmPub.localVars[0];
            continue;

        case OP_EvalLocalVariableRefCached:
            fieldValueIndex = 0;
            fieldValueId = Scr_GetLocalVar(fs.pos++);
            continue;

        case OP_SetLevelFieldVariableField:
            SetVariableValue(GetVariable(scrVarPub.levelId, Scr_ReadUnsignedShort(&fs.pos)), fs.top);
            --fs.top;
            continue;

        case OP_SetVariableField:
            goto LN403;

        case OP_SetAnimFieldVariableField:
            SetVariableValue(GetVariable(scrVarPub.animId, Scr_ReadUnsignedShort(&fs.pos)), fs.top);
            --fs.top;
            continue;

        case OP_SetSelfFieldVariableField:
            objectId = Scr_GetSelf(fs.localId);
            fieldValueIndex = Scr_GetVariableFieldIndex(objectId, Scr_ReadUnsignedShort(&fs.pos));
            fieldValueId = Scr_GetVarId(fieldValueIndex);
LN403:
            if (fieldValueIndex)
            {
                iassert(fieldValueId);
                if (fs.top->type)
                    SetVariableValue(fieldValueId, fs.top);
                else
                    RemoveVariableValue(objectId, fieldValueIndex);
            }
            else
            {
                SetVariableFieldValue(fieldValueId, fs.top);
            }

            --fs.top;
            continue;

        case OP_SetLocalVariableFieldCached0:
setlocalvariablefieldcached0:
            SetVariableValue(scrVmPub.localVars[0], fs.top);
            --fs.top;
            continue;

        case OP_SetLocalVariableFieldCached:
setlocalvariablefieldcached:
            SetVariableValue(Scr_GetLocalVar(fs.pos), fs.top);
            ++fs.pos;
            --fs.top;
            continue;

        case OP_CallBuiltin0:
        case OP_CallBuiltin1:
        case OP_CallBuiltin2:
        case OP_CallBuiltin3:
        case OP_CallBuiltin4:
        case OP_CallBuiltin5:
            iassert(!scrVmPub.outparamcount);
            scrVmPub.outparamcount = opcode - OP_CallBuiltin0;
            goto CallBuiltIn;

        case OP_CallBuiltin:
            iassert(!scrVmPub.outparamcount);
            scrVmPub.outparamcount = *(unsigned char *)fs.pos++;
CallBuiltIn:
            iassert(!scrVmPub.inparamcount);
            builtinIndex = Scr_ReadUnsignedShort(&fs.pos);
            scrVmPub.function_frame->fs.pos = fs.pos;
            if (scrVmDebugPub.func_table[builtinIndex].breakpointCount)
            {
                outparamcount = scrVmPub.outparamcount;
                Scr_HitBuiltinBreakpoint(fs.top, fs.pos, fs.localId, opcode, builtinIndex, scrVmPub.outparamcount);
                scrVmPub.outparamcount = outparamcount;
            }
            scrVmPub.top = fs.top;
            builtInTime = scrVmDebugPub.builtInTime;
            time = __rdtsc();
            ((void (*)(void))scrCompilePub.func_table[builtinIndex])();
            timeSpent = __rdtsc() - time;
            scrVmDebugPub.builtInTime = timeSpent + builtInTime;
            scrVmDebugPub.func_table[builtinIndex].prof += timeSpent;
            ++scrVmDebugPub.func_table[builtinIndex].usage;
            goto post_builtin;

        case OP_CallBuiltinMethod0:
        case OP_CallBuiltinMethod1:
        case OP_CallBuiltinMethod2:
        case OP_CallBuiltinMethod3:
        case OP_CallBuiltinMethod4:
        case OP_CallBuiltinMethod5:
            iassert(!scrVmPub.outparamcount);
            scrVmPub.outparamcount = opcode - OP_CallBuiltinMethod0;
            goto CallBuiltinMethod;
        case OP_CallBuiltinMethod:
            iassert(!scrVmPub.outparamcount);
            scrVmPub.outparamcount = *(unsigned char *)fs.pos++;
CallBuiltinMethod:
            iassert(!scrVmPub.inparamcount);
            scrVmPub.top = fs.top - 1;
            builtinIndex = Scr_ReadUnsignedShort(&fs.pos);
            if (fs.top->type != VAR_POINTER)
            {
                RemoveRefToValue(fs.top->type, fs.top->u);
                scrVarPub.error_index = -1;
                Scr_Error(va("%s is not an entity", var_typename[fs.top->type]));
            }
            objectId = fs.top->u.pointerValue;
            if (GetObjectType(objectId) == VAR_ENTITY)
            {
                entref = Scr_GetEntityIdRef(objectId);
                RemoveRefToObject(objectId);
                scrVmPub.function_frame->fs.pos = fs.pos;
                if (scrVmGlob.recordPlace)
                    Scr_GetFileAndLine(fs.pos, &scrVmGlob.lastFileName, &scrVmGlob.lastLine);
                if (scrVmDebugPub.func_table[builtinIndex].breakpointCount)
                {
                    if (scrVmPub.top != fs.top - 1)
                        MyAssertHandler(".\\script\\scr_vm.cpp", 1084, 0, "%s", "scrVmPub.top == fs.top - 1");
                    unsigned int backup = scrVmPub.outparamcount;
                    Scr_HitBuiltinBreakpoint(fs.top, fs.pos, fs.localId, opcode, builtinIndex, scrVmPub.outparamcount + 1);
                    scrVmPub.outparamcount = scrVmPub.outparamcount;
                    scrVmPub.top = fs.top - 1;
                }
                builtInTime = scrVmDebugPub.builtInTime;
                time = __rdtsc();
                ((void (*)(scr_entref_t))scrCompilePub.func_table[builtinIndex])(entref);
                timeSpent = __rdtsc() - time;
                scrVmDebugPub.builtInTime = timeSpent + builtInTime;
                scrVmDebugPub.func_table[builtinIndex].prof += timeSpent;
                ++scrVmDebugPub.func_table[builtinIndex].usage;
post_builtin:
                fs.top = scrVmPub.top;
                fs.pos = scrVmPub.function_frame->fs.pos;
                if (scrVmPub.outparamcount)
                {
                    outparamcount = scrVmPub.outparamcount;
                    scrVmPub.outparamcount = 0;
                    scrVmPub.top -= outparamcount;
                    do
                    {
                        RemoveRefToValue(fs.top);
                        --fs.top;
                        --outparamcount;
                    } while (outparamcount);
                }
                if (scrVmPub.inparamcount)
                {
                    iassert(scrVmPub.inparamcount == 1);
                    scrVmPub.inparamcount = 0;
                    iassert(fs.top == scrVmPub.top);
                }
                else
                {
                    iassert(fs.top == scrVmPub.top);
                    ++fs.top;
                    fs.top->type = VAR_UNDEFINED;
                }
                continue;
            }
            RemoveRefToObject(objectId);
            scrVarPub.error_index = -1;
            Scr_Error(va("%s is not an entity", var_typename[GetObjectType(objectId)]));

        case OP_wait: // VoroN: use sv_fps here??
            if (fs.top->type == VAR_FLOAT)
            {
                if (fs.top->u.floatValue < 0.0)
                    Scr_Error("negative wait is not allowed");
                waitTime = Q_rint(fs.top->u.floatValue * 20.0);
                //waitTime = Q_rint(top->u.floatValue * float(sv_fps->current.integer));
                if (!waitTime)
                    waitTime = fs.top->u.floatValue != 0.0;
            }
            else if (fs.top->type == VAR_INTEGER)
            {
                waitTime = 20 * fs.top->u.intValue;
                //waitTime = sv_fps->current.integer * top->u.intValue;
            }
            else
            {
                scrVarPub.error_index = 2;
                Scr_Error(va("type %s is not a float", var_typename[fs.top->type]));
            }
            // Com_Printf("%i\n", waitTime);
            if (waitTime > 0xFFFFFE)
            {
                scrVarPub.error_index = 2;
                if ((waitTime & 0x80000000) == 0)
                    Scr_Error("wait is too long");
                Scr_Error("negative wait is not allowed");
            }
            if (waitTime)
                Scr_ResetTimeout();
            waitTime = (scrVarPub.time + waitTime) & 0xFFFFFF;
            --fs.top;
            scrVmDebugPub.profileEnable[fs.localId] = *profileEnablePos;
            //stackValue.type = VAR_STACK;
            //stackValue.u.stackValue = VM_ArchiveStack2(top - startTop, pos, top, localVarCount, &localId);
            stackValue.type = VAR_STACK;
            stackValue.u.stackValue = VM_ArchiveStack();
            id = GetArray(GetVariable(scrVarPub.timeArrayId, waitTime));
            stackId = GetNewObjectVariable(id, fs.localId);
            SetNewVariableValue(stackId, &stackValue);
            Scr_SetThreadWaitTime(fs.localId, waitTime);
            goto thread_end;

        case OP_waittillFrameEnd:
            iassert(Scr_IsInOpcodeMemory(fs.pos));
            iassert(!(scrVarPub.time & ~VAR_NAME_LOW_MASK));
            scrVmDebugPub.profileEnable[fs.localId] = *profileEnablePos;
            stackValue.type = VAR_STACK;
            //stackValue.u.stackValue = VM_ArchiveStack2(top - startTop, pos, top, localVarCount, &localId);
            stackValue.u.stackValue = VM_ArchiveStack();
            id = GetArray(GetVariable(scrVarPub.timeArrayId, scrVarPub.time));
            stackId = GetNewObjectVariableReverse(id, fs.localId);
            SetNewVariableValue(stackId, &stackValue);
            Scr_SetThreadWaitTime(fs.localId, scrVarPub.time);
            goto thread_end;

        case OP_PreScriptCall:
            INC_TOP();
            fs.top->type = VAR_PRECODEPOS;
            continue;

        case OP_ScriptFunctionCall2:
            INC_TOP();
            fs.top->type = VAR_PRECODEPOS;
            goto ScriptFunctionCall;

        case OP_ScriptFunctionCall:
ScriptFunctionCall:
            if (scrVmPub.function_count < 31)
            {
                selfId = Scr_GetSelf(fs.localId);
                AddRefToObject(selfId);
                fs.localId = AllocChildThread(selfId, fs.localId);
                scrVmPub.function_frame->fs.pos = fs.pos;
                fs.pos = Scr_ReadCodePos(&scrVmPub.function_frame->fs.pos);
                goto function_call;
            }
            Scr_Error("script stack overflow (too many embedded function calls)");

        case OP_ScriptFunctionCallPointer:
            if (fs.top->type != VAR_FUNCTION)
            {
                Scr_Error(va("%s is not a function pointer", var_typename[fs.top->type]));
            }
            if (scrVmPub.function_count < 31)
            {
                selfId = Scr_GetSelf(fs.localId);
                AddRefToObject(selfId);
                fs.localId = AllocChildThread(selfId, fs.localId);
                scrVmPub.function_frame->fs.pos = fs.pos;
                fs.pos = fs.top->u.codePosValue;
                --fs.top;
                goto function_call;
            }
            scrVarPub.error_index = 1;
            Scr_Error("script stack overflow (too many embedded function calls)");

        case OP_ScriptMethodCall:
            if (fs.top->type != VAR_POINTER)
            {
                scrVarPub.error_index = 1;
                Scr_Error(va("%s is not an object", var_typename[fs.top->type]));
            }
            if (scrVmPub.function_count < 31)
            {
                fs.localId = AllocChildThread(fs.top->u.stringValue, fs.localId);
                --fs.top;
                scrVmPub.function_frame->fs.pos = fs.pos;
                fs.pos = Scr_ReadCodePos(&scrVmPub.function_frame->fs.pos);
                goto function_call;
            }
            Scr_Error("script stack overflow (too many embedded function calls)");

        case OP_ScriptMethodCallPointer:
            if (fs.top->type == VAR_FUNCTION)
            {
                tempCodePos = fs.top->u.codePosValue;
                --fs.top;
                if (fs.top->type != VAR_POINTER)
                {
                    scrVarPub.error_index = 2;
                    Scr_Error(va("%s is not an object", var_typename[fs.top->type]));
                }
                if (scrVmPub.function_count < 31)
                {
                    fs.localId = AllocChildThread(fs.top->u.stringValue, fs.localId);
                    --fs.top;
                    scrVmPub.function_frame->fs.pos = fs.pos;
                    fs.pos = tempCodePos;
                    goto function_call;
                }
                scrVarPub.error_index = 1;
                Scr_Error("script stack overflow (too many embedded function calls)");
            }
            RemoveRefToValue(fs.top--);
            Scr_Error(va("%s is not a function pointer", var_typename[fs.top[1].type]));

        case OP_ScriptThreadCall:
            if (scrVmPub.function_count < 31)
            {
                selfId = Scr_GetSelf(fs.localId);
                AddRefToObject(selfId);
                fs.localId = AllocThread(selfId);
                scrVmPub.function_frame->fs.pos = fs.pos;
                scrVmPub.function_frame->fs.startTop = fs.startTop;
                fs.pos = Scr_ReadCodePos(&scrVmPub.function_frame->fs.pos);
                fs.startTop = &fs.top[-Scr_ReadUnsigned(&scrVmPub.function_frame->fs.pos)];
                goto thread_call;
            }
            scrVarPub.error_index = 1;
            Scr_Error("script stack overflow (too many embedded function calls)");

        case OP_ScriptThreadCallPointer:
            if (fs.top->type == VAR_FUNCTION)
            {
                if (scrVmPub.function_count < 31)
                {
                    tempCodePos = fs.top->u.codePosValue;
                    --fs.top;
                    selfId = Scr_GetSelf(fs.localId);
                    AddRefToObject(selfId);
                    fs.localId = AllocThread(selfId);
                    scrVmPub.function_frame->fs.pos = fs.pos;
                    scrVmPub.function_frame->fs.startTop = fs.startTop;
                    fs.pos = tempCodePos;
                    fs.startTop = &fs.top[-Scr_ReadUnsigned(&scrVmPub.function_frame->fs.pos)];
                    goto thread_call;
                }
                scrVarPub.error_index = 1;
                Scr_Error("script stack overflow (too many embedded function calls)");
            }
            Scr_Error(va("%s is not a function pointer", var_typename[fs.top->type]));

        case OP_ScriptMethodThreadCall:
            if (fs.top->type != VAR_POINTER)
            {
                scrVarPub.error_index = 2;
                Scr_Error(va("%s is not an object", var_typename[fs.top->type]));
            }
            if (scrVmPub.function_count > 30)
            {
                scrVarPub.error_index = 1;
                Scr_Error("script stack overflow (too many embedded function calls)");
            }
            fs.localId = AllocThread(fs.top->u.stringValue);
            --fs.top;
            scrVmPub.function_frame->fs.pos = fs.pos;
            scrVmPub.function_frame->fs.startTop = fs.startTop;
            fs.pos = Scr_ReadCodePos(&scrVmPub.function_frame->fs.pos);
            fs.startTop = &fs.top[-Scr_ReadUnsigned(&scrVmPub.function_frame->fs.pos)];
            goto thread_call;

        case OP_ScriptMethodThreadCallPointer:
            if (fs.top->type != VAR_FUNCTION)
            {
                RemoveRefToValue(fs.top);
                --fs.top;
                Scr_Error(va("%s is not a function pointer", var_typename[fs.top[1].type]));
            }
            tempCodePos = fs.top->u.codePosValue;
            --fs.top;
            if (fs.top->type != VAR_POINTER)
            {
                scrVarPub.error_index = 2;
                Scr_Error(va("%s is not an object", var_typename[fs.top->type]));
            }
            if (scrVmPub.function_count > 30)
            {
                scrVarPub.error_index = 1;
                Scr_Error("script stack overflow (too many embedded function calls)");
            }
            fs.localId = AllocThread(fs.top->u.intValue);
            --fs.top;
            scrVmPub.function_frame->fs.pos = fs.pos;
            scrVmPub.function_frame->fs.startTop = fs.startTop;
            fs.pos = tempCodePos;
            fs.startTop = &fs.top[-Scr_ReadUnsigned(&scrVmPub.function_frame->fs.pos)];
thread_call:
            scrVmPub.function_frame->fs.top = fs.startTop;
            scrVmPub.function_frame->topType = fs.startTop->type;
            fs.startTop->type = VAR_PRECODEPOS;
            ++thread_count;
            *++profileEnablePos = 0;
function_call:
            scrVmPub.function_frame->fs.localVarCount = fs.localVarCount;
            fs.localVarCount = 0;
            ++scrVmPub.function_count;
            ++scrVmPub.function_frame;
            scrVmPub.function_frame->fs.localId = fs.localId;
            iassert(fs.pos);
            continue;

        case OP_DecTop:
            RemoveRefToValue(fs.top);
            --fs.top;
            continue;

        case OP_CastFieldObject:
            objectId = Scr_EvalFieldObject(scrVarPub.tempVariable, fs.top);
            --fs.top;
            continue;

        case OP_EvalLocalVariableObjectCached:
            objectId = Scr_EvalVariableObject(Scr_GetLocalVar(fs.pos));
            ++fs.pos;
            continue;

        case OP_CastBool:
            Scr_CastBool(fs.top);
            continue;

        case OP_BoolNot:
            Scr_EvalBoolNot(fs.top);
            continue;

        case OP_BoolComplement:
            Scr_EvalBoolComplement(fs.top);
            continue;

        case OP_JumpOnFalse:
            Scr_CastBool(fs.top);
            iassert(fs.top->type == VAR_INTEGER);
            jumpOffset = Scr_ReadUnsignedShort(&fs.pos);
            if (!fs.top->u.intValue)
                fs.pos += jumpOffset;
            --fs.top;
            continue;

        case OP_JumpOnTrue:
            Scr_CastBool(fs.top);
            iassert(fs.top->type == VAR_INTEGER);
            jumpOffset = Scr_ReadUnsignedShort(&fs.pos);
            if (fs.top->u.intValue)
                fs.pos += jumpOffset;
            --fs.top;
            continue;

        case OP_JumpOnFalseExpr:
            Scr_CastBool(fs.top);
            iassert(fs.top->type == VAR_INTEGER);
            jumpOffset = Scr_ReadUnsignedShort(&fs.pos);
            if (fs.top->u.intValue)
            {
                --fs.top;
                continue;
            }
            fs.pos += jumpOffset;
            continue;

        case OP_JumpOnTrueExpr:
            Scr_CastBool(fs.top);
            jumpOffset = Scr_ReadUnsignedShort(&fs.pos);
            if (!fs.top->u.intValue)
            {
                --fs.top;
                continue;
            }
            fs.pos += jumpOffset;
            continue;

        case OP_jump:
            jumpOffset = Scr_ReadUnsigned(&fs.pos);
            fs.pos += jumpOffset;
            continue;

#define INFINITE_LOOP_TIMEOUT 5000
        case OP_jumpback:
            if (scrVarPub.numScriptValues > 0xF37E || scrVarPub.numScriptObjects > 0x7380)
            {
                if (scrVmPub.showError)
                {
                    Scr_DumpScriptThreads();
                    Scr_DumpScriptVariablesDefault();
                    Scr_Error("exceeded maximum number of script variables");
                }
                Sys_Error("exceeded maximum number of script variables");
            }
            if ((unsigned int)(Sys_Milliseconds() - scrVmGlob.starttime) >= INFINITE_LOOP_TIMEOUT)
            {
                iassert(logScriptTimes);
                if (logScriptTimes->current.enabled)
                {
                    Com_Printf(23, "EXCEED TIME: %d\n", Sys_Milliseconds());
                }
                if (!scrVmGlob.loading)
                {
                    VM_PrintJumpHistory();
                    if (scrVmPub.showError)
                    {
                        iassert(!scrVmPub.debugCode);
                        Scr_DumpScriptThreads();
                        Scr_DumpScriptVariablesDefault();
                        Scr_Error("potential infinite loop in script");
                    }
                    if (!scrVmPub.abort_on_error)
                    {
                        Com_Printf(1, "script runtime error: potential infinite loop in script - killing thread.\n");
                        Scr_PrintPrevCodePos(CON_CHANNEL_DONT_FILTER, (char*)fs.pos, 0);
                        Scr_ResetTimeout();
                        while (1)
                        {
                            parentLocalId = GetSafeParentLocalId(fs.localId);
                            Scr_KillThread(fs.localId);
                            scrVmPub.localVars -= fs.localVarCount;
                            iassert(fs.top->type != VAR_PRECODEPOS);
                            while (fs.top->type != VAR_CODEPOS)
                            {
                                RemoveRefToValue(fs.top);
                                --fs.top;
                                iassert(fs.top->type != VAR_PRECODEPOS);
                            }
                            --scrVmPub.function_count;
                            --scrVmPub.function_frame;
                            if (!parentLocalId)
                                break;
                            iassert(fs.top != fs.startTop);
                            RemoveRefToObject(fs.localId);
                            iassert(fs.top->type == VAR_CODEPOS);
                            fs.pos = scrVmPub.function_frame->fs.pos;
                            iassert(fs.pos);
                            fs.localVarCount = scrVmPub.function_frame->fs.localVarCount;
                            fs.localId = parentLocalId;
                            --fs.top;
                        }
                        iassert(fs.top == fs.startTop);
                        goto thread_end;
                    }
                    Scr_TerminalError("potential infinite loop in script");
                }
                Com_Printf(1, "script runtime warning: potential infinite loop in script.\n");
                Scr_PrintPrevCodePos(CON_CHANNEL_DONT_FILTER, (char*)fs.pos, 0);
                jumpOffset = Scr_ReadUnsignedShort(&fs.pos);
                fs.pos -= jumpOffset;
                Scr_ResetTimeout();
            }
            else
            {
                scrVmDebugPub.jumpbackHistory[scrVmDebugPub.jumpbackHistoryIndex] = fs.pos;
                scrVmDebugPub.jumpbackHistoryIndex = (scrVmDebugPub.jumpbackHistoryIndex + 1) % 0x80u;
                jumpOffset = Scr_ReadUnsignedShort(&fs.pos);
                fs.pos -= jumpOffset;
            }
            continue;

        case OP_inc:
            INC_TOP();
            *fs.top = Scr_EvalVariableField(fieldValueId);
            if (fs.top->type == VAR_INTEGER)
            {
                ++fs.top->u.intValue;
                iassert(*fs.pos == OP_SetVariableField);
            }
            else
            {
                Scr_Error(va("++ must be applied to an int (applied to %s)", var_typename[fs.top->type]));
            }
            ++fs.pos;
            SetVariableFieldValue(fieldValueId, fs.top);
            --fs.top;
            continue;

        case OP_dec:
            INC_TOP();
            *fs.top = Scr_EvalVariableField(fieldValueId);
            if (fs.top->type == VAR_INTEGER)
            {
                --fs.top->u.intValue;
            }
            else
            {
                Scr_Error(va("-- must be applied to an int (applied to %s)", var_typename[fs.top->type]));
            }
            ++fs.pos;
            SetVariableFieldValue(fieldValueId, fs.top);
            --fs.top;
            continue;

        case OP_bit_or:
            Scr_EvalOr(fs.top - 1, fs.top);
            --fs.top;
            continue;

        case OP_bit_ex_or:
            Scr_EvalExOr(fs.top - 1, fs.top);
            --fs.top;
            continue;

        case OP_bit_and:
            Scr_EvalAnd(fs.top - 1, fs.top);
            --fs.top;
            continue;

        case OP_equality:
            Scr_EvalEquality(fs.top - 1, fs.top);
            --fs.top;
            continue;

        case OP_inequality:
            Scr_EvalInequality(fs.top - 1, fs.top);
            --fs.top;
            continue;

        case OP_less:
            Scr_EvalLess(fs.top - 1, fs.top);
            --fs.top;
            continue;

        case OP_greater:
            Scr_EvalGreater(fs.top - 1, fs.top);
            --fs.top;
            continue;

        case OP_less_equal:
            Scr_EvalLessEqual(fs.top - 1, fs.top);
            --fs.top;
            continue;

        case OP_greater_equal:
            Scr_EvalGreaterEqual(fs.top - 1, fs.top);
            --fs.top;
            continue;

        case OP_shift_left:
            Scr_EvalShiftLeft(fs.top - 1, fs.top);
            --fs.top;
            continue;

        case OP_shift_right:
            Scr_EvalShiftRight(fs.top - 1, fs.top);
            --fs.top;
            continue;

        case OP_plus:
            Scr_EvalPlus(fs.top - 1, fs.top);
            --fs.top;
            continue;

        case OP_minus:
            Scr_EvalMinus(fs.top - 1, fs.top);
            --fs.top;
            continue;

        case OP_multiply:
            Scr_EvalMultiply(fs.top - 1, fs.top);
            --fs.top;
            continue;

        case OP_divide:
            Scr_EvalDivide(fs.top - 1, fs.top);
            --fs.top;
            continue;

        case OP_mod:
            Scr_EvalMod(fs.top - 1, fs.top);
            --fs.top;
            continue;

        case OP_size:
            Scr_EvalSizeValue(fs.top);
            continue;

        case OP_waittillmatch:
        case OP_waittill:
            iassert(Scr_IsInOpcodeMemory(fs.pos));
            if (fs.top->type != VAR_POINTER)
            {
                scrVarPub.error_index = 2;
                Scr_Error(va("%s is not an object", var_typename[fs.top->type]));
            }
            if (!IsFieldObject(fs.top->u.pointerValue))
            {
                scrVarPub.error_index = 2;
                Scr_Error(va("%s is not an object", var_typename[GetObjectType(fs.top->u.pointerValue)]));
            }
            tempValue.u = fs.top->u;
            --fs.top;
            if (fs.top->type == VAR_STRING)
            {
                stringValue = fs.top->u.stringValue;
                --fs.top;

                iassert(GetObjectType(tempValue.u.pointerValue) != VAR_THREAD);
                iassert(GetObjectType(tempValue.u.pointerValue) != VAR_NOTIFY_THREAD);
                iassert(GetObjectType(tempValue.u.pointerValue) != VAR_TIME_THREAD);
                iassert(GetObjectType(tempValue.u.pointerValue) != VAR_CHILD_THREAD);
                iassert(GetObjectType(tempValue.u.pointerValue) != VAR_DEAD_THREAD);

                scrVmDebugPub.profileEnable[fs.localId] = *profileEnablePos;

                stackValue.type = VAR_STACK;
                //stackValue.u.stackValue = VM_ArchiveStack2(top - startTop, pos, top, localVarCount, &localId);
                stackValue.u.stackValue = VM_ArchiveStack();
                id = GetArray(GetVariable(GetArray(GetVariable(tempValue.u.stringValue, 0x18000)), stringValue));
                stackId = GetNewObjectVariable(id, fs.localId);
                SetNewVariableValue(stackId, &stackValue);
                tempValue.type = VAR_POINTER;
                SetNewVariableValue(GetNewObjectVariable(GetArray(GetObjectVariable(scrVarPub.pauseArrayId, Scr_GetSelf(fs.localId))), fs.localId), &tempValue);
                Scr_SetThreadNotifyName(fs.localId, stringValue);
                goto thread_end;
            }
            ++fs.top;
            scrVarPub.error_index = 3;
            Scr_Error("first parameter of waittill must evaluate to a string");

        case OP_notify:
            if (fs.top->type != VAR_POINTER)
            {
                scrVarPub.error_index = 2;
                Scr_Error(va("%s is not an object", var_typename[fs.top->type]));
            }
            id = fs.top->u.pointerValue;
            if (!IsFieldObject(id))
            {
                scrVarPub.error_index = 2;
                Scr_Error(va("%s is not an object", var_typename[GetObjectType(fs.top->u.pointerValue)]));
            }
            --fs.top;
            if (fs.top->type != VAR_STRING)
            {
                ++fs.top;
                scrVarPub.error_index = 1;
                Scr_Error("first parameter of notify must evaluate to a string");
            }
            stringValue = fs.top->u.stringValue;
            --fs.top;
            if (scrVmDebugPub.checkBreakon)
                Scr_CheckBreakonNotify(id, stringValue, fs.top, (char*)fs.pos, fs.localId);
            scrVmPub.function_frame->fs.pos = fs.pos;
            VM_Notify(id, stringValue, fs.top);
            fs.pos = scrVmPub.function_frame->fs.pos;
            RemoveRefToObject(id);
            SL_RemoveRefToString(stringValue);
            iassert(fs.top->type != VAR_CODEPOS);
            while (fs.top->type != VAR_PRECODEPOS)
            {
                RemoveRefToValue(fs.top);
                fs.top--;
                iassert(fs.top->type != VAR_CODEPOS);
            }
            --fs.top;
            continue;

        case OP_endon:
            if (fs.top->type != VAR_POINTER)
            {
                scrVarPub.error_index = 1;
                Scr_Error(va("%s is not an object", var_typename[fs.top->type]));
            }
            if (!IsFieldObject(fs.top->u.pointerValue))
            {
                scrVarPub.error_index = 1;
                Scr_Error(va("%s is not an object", var_typename[GetObjectType(fs.top->u.pointerValue)]));
            }
            if (fs.top[-1].type == VAR_STRING)
            {
                stringValue = fs.top[-1].u.stringValue;
                AddRefToObject(fs.localId);
                threadId = AllocThread(fs.localId);
                iassert(GetObjectType(fs.top->u.pointerValue) != VAR_THREAD);
                iassert(GetObjectType(fs.top->u.pointerValue) != VAR_NOTIFY_THREAD);
                iassert(GetObjectType(fs.top->u.pointerValue) != VAR_TIME_THREAD);
                iassert(GetObjectType(fs.top->u.pointerValue) != VAR_CHILD_THREAD);
                iassert(GetObjectType(fs.top->u.pointerValue) != VAR_DEAD_THREAD);
                GetObjectVariable(GetArray(GetVariable(GetArray(GetVariable(fs.top->u.stringValue, 0x18000)), stringValue)), threadId);
                RemoveRefToObject(threadId);
                tempValue.type = VAR_POINTER;
                tempValue.u = fs.top->u;
                SetNewVariableValue(GetNewObjectVariable(GetArray(GetObjectVariable(scrVarPub.pauseArrayId, fs.localId)), threadId), &tempValue);
                Scr_SetThreadNotifyName(threadId, stringValue);
                fs.top -= 2;
                continue;
            }
            Scr_Error("first parameter of endon must evaluate to a string");

        case OP_voidCodepos:
            INC_TOP();
            fs.top->type = VAR_PRECODEPOS;
            continue;

        case OP_switch:
            jumpOffset = Scr_ReadUnsigned(&fs.pos);
            fs.pos += jumpOffset;
            caseCount = Scr_ReadUnsignedShort(&fs.pos);
            if (fs.top->type == VAR_STRING)
            {
                caseValue = fs.top->u.stringValue;
                SL_RemoveRefToString(fs.top->u.stringValue);
            }
            else if (fs.top->type == VAR_INTEGER)
            {
                if (IsValidArrayIndex(fs.top->u.pointerValue))
                {
                    caseValue = GetInternalVariableIndex(fs.top->u.pointerValue);
                }
                else
                {
                    Scr_Error(va("switch index %d out of range", fs.top->u.intValue));
                }
            }
            else
            {
                Scr_Error(va("cannot switch on %s", var_typename[fs.top->type]));
            }
            if (!caseCount)
            {
            loop_dec_top:
                --fs.top;
                continue;
            }
            iassert(caseValue);
            do
            {
                currentCaseValue = Scr_ReadUnsigned(&fs.pos);
                currentCodePos = Scr_ReadCodePos(&fs.pos);
                if (currentCaseValue == caseValue)
                {
                    fs.pos = currentCodePos;
                    iassert(fs.pos);
                    goto loop_dec_top;
                }
                --caseCount;
            } while (caseCount);
            if (!currentCaseValue)
            {
                fs.pos = currentCodePos;
                iassert(fs.pos);
            }
            --fs.top;
            continue;

        case OP_endswitch:
            caseCount = Scr_ReadUnsignedShort(&fs.pos);
            Scr_ReadIntArray(&fs.pos, 2 * caseCount);
            continue;

        case OP_vector:
            fs.top -= 2;
            Scr_CastVector(fs.top);
            continue;

        case OP_NOP:
            continue;

        case OP_abort:
            iassert(g_script_error_level >= 0);
            --g_script_error_level;
            return 0;

        case OP_object:
            INC_TOP();
            classnum = Scr_ReadUnsigned(&fs.pos);
            entnum = Scr_ReadUnsigned(&fs.pos);
            fs.top->u.pointerValue = FindEntityId(entnum, classnum);
            if (!fs.top->u.pointerValue)
            {
                fs.top->type = VAR_UNDEFINED;
                Scr_Error("unknown object");
            }
            fs.top->type = VAR_POINTER;
            AddRefToObject(fs.top->u.pointerValue);
            continue;

        case OP_thread_object:
            INC_TOP();
            fs.top->u.pointerValue = Scr_ReadUnsignedShort(&fs.pos);
            fs.top->type = VAR_POINTER;
            AddRefToObject(fs.top->u.pointerValue);
            continue;

        case OP_EvalLocalVariable:
            INC_TOP();
            tempValue = Scr_EvalVariable(FindVariable(fs.localId, Scr_ReadUnsignedShort(&fs.pos)));
            fs.top->u = tempValue.u;
            fs.top->type = tempValue.type;
            continue;

        case OP_EvalLocalVariableRef:
            fieldValueIndex = 0;
            fieldValueId = FindVariable(fs.localId, Scr_ReadUnsignedShort(&fs.pos));
            if (fieldValueId)
                continue;
            Scr_Error("cannot create a new local variable in the debugger");

        case OP_prof_begin:
            profileIndex = *fs.pos++;
            profileBit = 1 << profileIndex;
            if (((1 << profileIndex) & *profileEnablePos) == 0)
            {
                *profileEnablePos |= profileBit;
                Profile_BeginScript(profileIndex);
            }
            continue;

        case OP_prof_end:
            profileIndex = *fs.pos++;
            profileBit = 1 << profileIndex;
            if (((1 << profileIndex) & *profileEnablePos) != 0)
            {
                *profileEnablePos &= ~profileBit;
                Profile_EndScript(profileIndex);
            }
            continue;

        case OP_breakpoint:
            if (!scrVarPub.developer)
                continue;
            opcode = Scr_HitBreakpoint(fs.top, (char*)fs.pos, fs.localId, 0);
            goto interrupt_return;

        case OP_assignmentBreakpoint:
            opcode = Scr_HitAssignmentBreakpoint(fs.top, (char*)fs.pos, fs.localId, 0);
            goto interrupt_return;

        case OP_manualAndAssignmentBreakpoint:
            opcode = Scr_HitAssignmentBreakpoint(fs.top, (char*)fs.pos, fs.localId, 1);
            goto interrupt_return;
        default:
            scrVmPub.terminal_error = 1;
            RuntimeErrorInternal(CON_CHANNEL_DONT_FILTER, (char*)fs.pos, 0, va("CODE ERROR: unknown opcode %d", opcode));
            continue;
        }
    }
}

unsigned int __cdecl VM_Execute(unsigned int localId, const char *pos, unsigned int paramcount)
{
    int time; // [esp+14h] [ebp-24h]
    function_stack_t fs_backup; // [esp+18h] [ebp-20h]
    VariableValue *startTop; // [esp+2Ch] [ebp-Ch]
    int type; // [esp+30h] [ebp-8h]
    int thread_count_backup; // [esp+34h] [ebp-4h]
    unsigned int localIda; // [esp+40h] [ebp+8h]
    unsigned int paramcounta; // [esp+48h] [ebp+10h]

    iassert(paramcount <= scrVmPub.inparamcount);
    Scr_ClearOutParams();
    startTop = &scrVmPub.top[-paramcount];
    paramcounta = scrVmPub.inparamcount - paramcount;
    if (scrVmPub.function_count >= 30)
    {
        Scr_KillThread(localId);
        scrVmPub.inparamcount = paramcounta + 1;
        iassert(!scrVmPub.outparamcount);
        while (paramcounta)
        {
            RemoveRefToValue(scrVmPub.top->type, scrVmPub.top->u);
            --scrVmPub.top;
            --paramcounta;
        }
        ++scrVmPub.top;
        scrVmPub.top->type = VAR_UNDEFINED;
        RuntimeError((char*)pos, 0, "script stack overflow (too many embedded function calls)", 0);
        return localId;
    }
    else
    {
        fs_backup = fs;
        thread_count_backup = thread_count;
        fs.localId = localId;
        fs.startTop = startTop;
        if (scrVmPub.function_count)
        {
            ++scrVmPub.function_count;
            ++scrVmPub.function_frame;
            scrVmPub.function_frame->fs.localId = 0;
        }
        scrVmPub.function_frame->fs.pos = pos;
        ++scrVmPub.function_count;
        ++scrVmPub.function_frame;
        scrVmPub.function_frame->fs.localId = localId;
        type = startTop->type;
        startTop->type = VAR_PRECODEPOS;
        scrVmPub.inparamcount = 0;
        fs.top = scrVmPub.top;
        fs.pos = pos;
        fs.localVarCount = 0;
        thread_count = 0;
        scrVmDebugPub.profileEnable[localId] = 0;
        if (scrVarPub.bScriptProfile)
        {
            scrVmDebugPub.builtInTime = 0;
            time = __rdtsc();
            localIda = VM_ExecuteInternal();
            if (!scrVmPub.function_count)
                Scr_AddProfileTime(pos, __rdtsc() - time, scrVmDebugPub.builtInTime);
        }
        else
        {
            localIda = VM_ExecuteInternal();
        }
        fs = fs_backup;
        thread_count = thread_count_backup;
        startTop->type = (Vartype_t)type;
        scrVmPub.top = startTop + 1;
        scrVmPub.inparamcount = paramcounta + 1;
        iassert(!scrVmPub.outparamcount);
        ClearVariableValue(scrVarPub.tempVariable);
        if (scrVmPub.function_count)
        {
            --scrVmPub.function_count;
            --scrVmPub.function_frame;
        }
        return localIda;
    }
}

unsigned __int16 __cdecl Scr_ExecEntThreadNum(
    unsigned int entnum,
    unsigned int classnum,
    int handle,
    unsigned int paramcount)
{
    unsigned int v4; // eax
    const char *pos; // [esp+34h] [ebp-10h]
    const char *varUsagePos; // [esp+38h] [ebp-Ch]
    unsigned int objId; // [esp+3Ch] [ebp-8h]
    unsigned int id; // [esp+40h] [ebp-4h]

    pos = &scrVarPub.programBuffer[handle];
    if (!scrVmPub.function_count)
    {
        if ((int *)scrVmPub.localVars != &scrVmGlob.starttime)
            MyAssertHandler(".\\script\\scr_vm.cpp", 4087, 0, "%s", "scrVmPub.localVars == scrVmGlob.localVarsStack - 1");
        //Profile_Begin(332);
        Scr_ResetTimeout();
    }
    if (!scrVarPub.timeArrayId)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4093, 0, "%s", "scrVarPub.timeArrayId");
    if (!handle)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4094, 0, "%s", "handle");
    ++scrVarPub.ext_threadcount;
    if (!Scr_IsInOpcodeMemory(pos))
        MyAssertHandler(".\\script\\scr_vm.cpp", 4100, 0, "%s", "Scr_IsInOpcodeMemory( pos )");
    varUsagePos = scrVarPub.varUsagePos;
    scrVarPub.varUsagePos = pos + 1;
    objId = Scr_GetEntityId(entnum, classnum);
    AddRefToObject(objId);
    v4 = AllocThread(objId);
    id = VM_Execute(v4, pos, paramcount);
    scrVarPub.varUsagePos = varUsagePos;
    if (scrVarDebugPub)
        ++scrVarDebugPub->extRefCount[id];
    RemoveRefToValue(scrVmPub.top->type, scrVmPub.top->u);
    scrVmPub.top->type = VAR_UNDEFINED;
    --scrVmPub.top;
    --scrVmPub.inparamcount;
    if (!scrVmPub.function_count)
    {
        //Profile_EndInternal(0);
        if ((int *)scrVmPub.localVars != &scrVmGlob.starttime)
            MyAssertHandler(".\\script\\scr_vm.cpp", 4129, 0, "%s", "scrVmPub.localVars == scrVmGlob.localVarsStack - 1");
    }
    return id;
}

void __cdecl Scr_AddExecThread(int handle, unsigned int paramcount)
{
    unsigned int v2; // eax
    unsigned int v3; // eax
    const char *pos; // [esp+30h] [ebp-8h]
    const char *varUsagePos; // [esp+34h] [ebp-4h]

    pos = &scrVarPub.programBuffer[handle];
    if (!scrVmPub.function_count)
    {
        if ((int *)scrVmPub.localVars != &scrVmGlob.starttime)
            MyAssertHandler(".\\script\\scr_vm.cpp", 4149, 0, "%s", "scrVmPub.localVars == scrVmGlob.localVarsStack - 1");
        //Profile_Begin(332);
        Scr_ResetTimeout();
    }
    if (!scrVarPub.timeArrayId)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4155, 0, "%s", "scrVarPub.timeArrayId");
    if (!handle)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4156, 0, "%s", "handle");
    if (!Scr_IsInOpcodeMemory(pos))
        MyAssertHandler(".\\script\\scr_vm.cpp", 4158, 0, "%s", "Scr_IsInOpcodeMemory( pos )");
    varUsagePos = scrVarPub.varUsagePos;
    scrVarPub.varUsagePos = pos + 1;
    AddRefToObject(scrVarPub.levelId);
    v2 = AllocThread(scrVarPub.levelId);
    v3 = VM_Execute(v2, pos, paramcount);
    RemoveRefToObject(v3);
    scrVarPub.varUsagePos = varUsagePos;
    ++scrVmPub.outparamcount;
    --scrVmPub.inparamcount;
    if (!scrVmPub.function_count)
    {
        //Profile_EndInternal(0);
        if ((int *)scrVmPub.localVars != &scrVmGlob.starttime)
            MyAssertHandler(".\\script\\scr_vm.cpp", 4179, 0, "%s", "scrVmPub.localVars == scrVmGlob.localVarsStack - 1");
    }
}

void __cdecl Scr_FreeThread(unsigned __int16 handle)
{
    if (!scrVarPub.timeArrayId)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4236, 0, "%s", "scrVarPub.timeArrayId");
    if (!handle)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4237, 0, "%s", "handle");
    if (scrVarDebugPub)
        --scrVarDebugPub->extRefCount[handle];
    RemoveRefToObject(handle);
    --scrVarPub.ext_threadcount;
}

void __cdecl Scr_ExecCode(const char *pos, unsigned int localId)
{
    unsigned int localIda; // [esp+Ch] [ebp+Ch]

    Scr_ResetTimeout();
    if (!scrVarPub.timeArrayId)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4256, 0, "%s", "scrVarPub.timeArrayId");
    if (scrVmPub.inparamcount)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4257, 0, "%s", "!scrVmPub.inparamcount");
    if (scrVmPub.outparamcount)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4258, 0, "%s", "!scrVmPub.outparamcount");
    if (scrVarPub.evaluate)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4260, 0, "%s", "!scrVarPub.evaluate");
    if (scrVmPub.debugCode)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4262, 0, "%s", "!scrVmPub.debugCode");
    scrVmPub.debugCode = 1;
    if (localId)
    {
        VM_Execute(localId, pos, 0);
    }
    else
    {
        AddRefToObject(scrVarPub.levelId);
        localIda = AllocThread(scrVarPub.levelId);
        VM_Execute(localIda, pos, 0);
        Scr_KillThread(localIda);
        RemoveRefToObject(localIda);
    }
    if (!scrVmPub.debugCode)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4278, 0, "%s", "scrVmPub.debugCode");
    scrVmPub.debugCode = 0;
    if (scrVmPub.inparamcount != 1)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4281, 0, "%s", "scrVmPub.inparamcount == 1");
    if (scrVmPub.outparamcount)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4282, 0, "%s", "!scrVmPub.outparamcount");
    if (scrVmPub.function_count)
    {
        --scrVmPub.function_count;
        --scrVmPub.function_frame;
    }
    --scrVmPub.top;
    scrVmPub.inparamcount = 0;
}

void __cdecl Scr_InitSystem(int sys)
{
    if (sys != 1)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4319, 0, "%s", "sys == SCR_SYS_GAME");
    if (scrVarPub.timeArrayId)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4320, 0, "%s", "!scrVarPub.timeArrayId");
    if (scrVarPub.ext_threadcount)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4322, 0, "%s", "!scrVarPub.ext_threadcount");
    if (scrVarPub.varUsagePos)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4326, 0, "%s", "!scrVarPub.varUsagePos");
    scrVarPub.varUsagePos = "<script init variable>";
    memset((unsigned __int8*)scrVmDebugPub.profileEnable, 0, sizeof(scrVmDebugPub.profileEnable));
    scrVarPub.timeArrayId = AllocObject();
    if (scrVarDebugPub)
        ++scrVarDebugPub->extRefCount[scrVarPub.timeArrayId];
    if (scrVarPub.pauseArrayId)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4340, 0, "%s", "!scrVarPub.pauseArrayId");
    scrVarPub.pauseArrayId = Scr_AllocArray();
    if (scrVarDebugPub)
        ++scrVarDebugPub->extRefCount[scrVarPub.pauseArrayId];
    if (scrVarPub.levelId)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4347, 0, "%s", "!scrVarPub.levelId");
    scrVarPub.levelId = AllocObject();
    if (scrVarDebugPub)
        ++scrVarDebugPub->extRefCount[scrVarPub.levelId];
    if (scrVarPub.animId)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4354, 0, "%s", "!scrVarPub.animId");
    scrVarPub.animId = AllocObject();
    if (scrVarDebugPub)
        ++scrVarDebugPub->extRefCount[scrVarPub.animId];
    if (scrVarPub.freeEntList)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4361, 0, "%s", "!scrVarPub.freeEntList");
    scrVarPub.time = 0;
    g_script_error_level = -1;
    Scr_InitDebuggerSystem();
    scrVarPub.varUsagePos = 0;
}

void __cdecl Scr_ShutdownSystem(unsigned __int8 sys, int bComplete)
{
    VariableValueInternal_u Object; // eax
    VariableValueInternal_u v3; // eax
    VariableUnion parentId; // [esp+0h] [ebp-Ch]
    int function_count; // [esp+4h] [ebp-8h]
    unsigned int id; // [esp+8h] [ebp-4h]
    unsigned int ida; // [esp+8h] [ebp-4h]
    unsigned int idb; // [esp+8h] [ebp-4h]

    iassert(sys == SCR_SYS_GAME);
    scrVarPub.varUsagePos = "<script shutdown variable>";
    Scr_ShutdownDebuggerSystem(0);
    Scr_FreeEntityList();
    if (scrVarPub.timeArrayId)
    {
        Scr_FreeGameVariable(bComplete);
        function_count = scrVmPub.function_count;
        scrVmPub.function_count = 0;
        for (id = FindFirstSibling(scrVarPub.timeArrayId); id; id = FindNextSibling(id))
        {
            Object = FindObject(id);
            VM_TerminateTime(Object.u.stringValue);
        }
        while (1)
        {
            ida = FindFirstSibling(scrVarPub.pauseArrayId);
            if (!ida)
                break;
            v3 = FindObject(ida);
            idb = FindFirstSibling(v3.u.stringValue);
            if (!idb)
                MyAssertHandler(".\\script\\scr_vm.cpp", 4417, 0, "%s", "id");
            parentId.intValue = GetVariableValueAddress(idb)->u.intValue;
            AddRefToObject(parentId.stringValue);
            Scr_CancelNotifyList(parentId.stringValue);
            RemoveRefToObject(parentId.stringValue);
        }
        if (!scrVarPub.levelId)
            MyAssertHandler(".\\script\\scr_vm.cpp", 4424, 0, "%s", "scrVarPub.levelId");
        ClearObject(scrVarPub.levelId);
        if (scrVarDebugPub)
            --scrVarDebugPub->extRefCount[scrVarPub.levelId];
        RemoveRefToEmptyObject(scrVarPub.levelId);
        scrVarPub.levelId = 0;
        if (!scrVarPub.animId)
            MyAssertHandler(".\\script\\scr_vm.cpp", 4433, 0, "%s", "scrVarPub.animId");
        ClearObject(scrVarPub.animId);
        if (scrVarDebugPub)
            --scrVarDebugPub->extRefCount[scrVarPub.animId];
        RemoveRefToEmptyObject(scrVarPub.animId);
        scrVarPub.animId = 0;
        if (!scrVarPub.timeArrayId)
            MyAssertHandler(".\\script\\scr_vm.cpp", 4442, 0, "%s", "scrVarPub.timeArrayId");
        ClearObject(scrVarPub.timeArrayId);
        if (scrVarDebugPub)
            --scrVarDebugPub->extRefCount[scrVarPub.timeArrayId];
        RemoveRefToEmptyObject(scrVarPub.timeArrayId);
        scrVarPub.timeArrayId = 0;
        if (!scrVarPub.pauseArrayId)
            MyAssertHandler(".\\script\\scr_vm.cpp", 4451, 0, "%s", "scrVarPub.pauseArrayId");
        if (GetArraySize(scrVarPub.pauseArrayId))
            MyAssertHandler(".\\script\\scr_vm.cpp", 4452, 0, "%s", "!GetArraySize( scrVarPub.pauseArrayId )");
        if (scrVarDebugPub)
            --scrVarDebugPub->extRefCount[scrVarPub.pauseArrayId];
        RemoveRefToEmptyObject(scrVarPub.pauseArrayId);
        scrVarPub.pauseArrayId = 0;
        if (scrVarPub.freeEntList)
            MyAssertHandler(".\\script\\scr_vm.cpp", 4460, 0, "%s", "!scrVarPub.freeEntList");
        Scr_FreeObjects();
        scrVarPub.varUsagePos = 0;
        if (function_count)
        {
            scrVarPub.bInited = 0;
            Scr_Init();
            if (scrStringDebugGlob)
                scrStringDebugGlob->ignoreLeaks = 1;
        }
        else
        {
            if (scrVarPub.ext_threadcount)
                MyAssertHandler(".\\script\\scr_vm.cpp", 4471, 0, "%s", "!scrVarPub.ext_threadcount");
            if (!Scr_IsStackClear())
                MyAssertHandler(".\\script\\scr_vm.cpp", 4473, 0, "%s", "Scr_IsStackClear()");
        }
    }
    else
    {
        scrVarPub.varUsagePos = 0;
    }
}

void __cdecl VM_TerminateTime(unsigned int timeId)
{
    VariableStackBuffer* stackValue; // [esp+0h] [ebp-Ch]
    unsigned int stackId; // [esp+4h] [ebp-8h]
    unsigned int startLocalId; // [esp+8h] [ebp-4h]

    if (!timeId)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3793, 0, "%s", "timeId");
    if (scrVmPub.function_count)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3794, 0, "%s", "!scrVmPub.function_count");
    AddRefToObject(timeId);
    while (1)
    {
        stackId = FindFirstSibling(timeId);
        if (!stackId)
            break;
        startLocalId = GetVariableKeyObject(stackId);
        if (!startLocalId)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3803, 0, "%s", "startLocalId");
        if (GetValueType(stackId) != 10)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3805, 0, "%s", "GetValueType( stackId ) == VAR_STACK");
        stackValue = (VariableStackBuffer*)GetVariableValueAddress(stackId)->u.intValue;
        RemoveObjectVariable(timeId, startLocalId);
        Scr_ClearWaitTime(startLocalId);
        VM_TerminateStack(startLocalId, startLocalId, stackValue);
    }
    RemoveRefToObject(timeId);
}

BOOL __cdecl Scr_IsSystemActive()
{
    return scrVarPub.timeArrayId && !scrVarPub.error_message;
}

int __cdecl Scr_GetInt(unsigned int index)
{
	VariableValue *entryValue;

	if ( index >= scrVmPub.outparamcount )
	{
		Scr_Error(va("parameter %d does not exist", index + 1));
		return 0;
	}

	entryValue = Scr_GetValue(index);

	if ( entryValue->type != VAR_INTEGER )
	{
		scrVarPub.error_index = index + 1;
		Scr_Error(va("type %s is not an int", var_typename[entryValue->type]));
	}

	return entryValue->u.intValue;
}

scr_anim_s __cdecl Scr_GetAnim(unsigned int index, XAnimTree_s* tree)
{
    XAnim_s* Anims; // esi
    const XAnim_s* v4; // eax
    const XAnim_s* v5; // eax
    const XAnim_s* v6; // eax
    const char* AnimDebugName; // eax
    const char* v8; // eax
    const char* v9; // [esp-8h] [ebp-18h]
    const char* AnimTreeDebugName; // [esp-4h] [ebp-14h]
    scr_anim_s anim; // [esp+8h] [ebp-8h]
    VariableValue* value; // [esp+Ch] [ebp-4h]

    if (index < scrVmPub.outparamcount)
    {
        value = &scrVmPub.top[-(int)index];
        if (value->type == 11)
        {
            anim = (scr_anim_s)value->u.intValue;
            if (!tree)
                return anim;
            Anims = Scr_GetAnims(anim.tree);
            if (Anims == XAnimGetAnims(tree))
                return anim;
            v4 = XAnimGetAnims(tree);
            AnimTreeDebugName = XAnimGetAnimTreeDebugName(v4);
            v5 = Scr_GetAnims(anim.tree);
            v9 = XAnimGetAnimTreeDebugName(v5);
            v6 = Scr_GetAnims(anim.tree);
            AnimDebugName = XAnimGetAnimDebugName(v6, anim.index);
            scrVarPub.error_message = va(
                "anim '%s' in animtree '%s' does not belong to the entity's animtree '%s'",
                AnimDebugName,
                v9,
                AnimTreeDebugName);
        }
        else
        {
            scrVarPub.error_message = va("type %s is not an anim", var_typename[value->type]);
        }
        RemoveRefToValue(value->type, value->u);
        value->type = VAR_UNDEFINED;
        scrVarPub.error_index = index + 1;
        Scr_ErrorInternal();
    }
    v8 = va("parameter %d does not exist", index + 1);
    Scr_Error(v8);
    return 0;
}

BOOL Scr_ErrorInternal()
{
    BOOL result; // eax

    iassert(scrVarPub.error_message);

    result = scrVarPub.evaluate;

    if (!scrVarPub.evaluate && !scrCompilePub.script_loading)
    {
        if (scrVmPub.function_count || scrVmPub.debugCode)
        {
            Com_PrintMessage(6, "throwing script exception: ", 0);
            Com_PrintMessage(6, (char*)scrVarPub.error_message, 0);
            Com_PrintMessage(6, "\n", 0);

            bcassert(g_script_error_level, ARRAY_COUNT(g_script_error));

            longjmp(g_script_error[g_script_error_level], -1); // KISAKTRYCATCH
        }
    error_2:
        Sys_Error("%s", scrVarPub.error_message);
    }

    if (scrVmPub.terminal_error)
        goto error_2;

    return result;
}

float __cdecl Scr_GetFloat(unsigned int index)
{
    VariableValue* value; // [esp+0h] [ebp-4h]

    if (index < scrVmPub.outparamcount)
    {
        value = &scrVmPub.top[-(int)index];
        if (value->type == 5)
            return value->u.floatValue;
        if (value->type == 6)
            return (double)value->u.intValue;
        scrVarPub.error_index = index + 1;
        Scr_Error(va("type %s is not a float", var_typename[value->type]));
    }
    Scr_Error(va("parameter %d does not exist", index + 1));
    return 0.0;
}

unsigned int __cdecl Scr_GetConstString(unsigned int index)
{
    const char* v2; // eax
    VariableValue* value; // [esp+0h] [ebp-4h]

    if (index >= scrVmPub.outparamcount)
    {
        Scr_Error(va("parameter %d does not exist", index + 1));
        return 0;
    }

    value = &scrVmPub.top[-(int)index];

    if (!Scr_CastString(value))
    {
        scrVarPub.error_index = index + 1;
        Scr_ErrorInternal();
    }

    iassert(value->type == VAR_STRING);
    SL_CheckExists(value->u.intValue);
    return value->u.stringValue;
}

unsigned int __cdecl Scr_GetConstLowercaseString(unsigned int index)
{
    const char* string; // [esp+0h] [ebp-2018h]
    unsigned int stringValue; // [esp+4h] [ebp-2014h]
    char str[8196]; // [esp+8h] [ebp-2010h] BYREF
    int i; // [esp+2010h] [ebp-8h]
    VariableValue* value; // [esp+2014h] [ebp-4h]

    if (index >= scrVmPub.outparamcount)
    {
        Scr_Error(va("parameter %d does not exist", index + 1));
        return 0;
    }

    value = &scrVmPub.top[-(int)index];

    if (!Scr_CastString(value))
    {
        scrVarPub.error_index = index + 1;
        Scr_ErrorInternal();
        Scr_Error(va("parameter %d does not exist", index + 1));
        return 0;
    }

    iassert(value->type == VAR_STRING);
    stringValue = value->u.stringValue;
    string = SL_ConvertToString(value->u.stringValue);

    for (i = 0; ; ++i)
    {
        str[i] = tolower(string[i]);
        if (!string[i])
            break;
    }
    
    iassert(value->type == VAR_STRING);

    value->u.stringValue = SL_GetString(str, 0);
    SL_RemoveRefToString(stringValue);
    SL_CheckExists(value->u.intValue);
    return value->u.stringValue;

}

const char* __cdecl Scr_GetString(unsigned int index)
{
    return SL_ConvertToString(Scr_GetConstString(index));
}

unsigned int __cdecl Scr_GetConstStringIncludeNull(unsigned int index)
{
    if (index >= scrVmPub.outparamcount || scrVmPub.top[-(int)index].type)
        return Scr_GetConstString(index);

    return 0;
}

const char* __cdecl Scr_GetDebugString(unsigned int index)
{
    const char* v2; // eax
    VariableValue* value; // [esp+0h] [ebp-4h]

    if (index >= scrVmPub.outparamcount)
    {
        v2 = va("parameter %d does not exist", index + 1);
        Scr_Error(v2);
        return 0;
    }
    else
    {
        value = &scrVmPub.top[-(int)index];
        Scr_CastDebugString(value);

        iassert(value->type == VAR_STRING); 
        return SL_ConvertToString(value->u.intValue);
    }
}

unsigned int __cdecl Scr_GetConstIString(unsigned int index)
{
	VariableValue *entryValue;

	if ( index >= scrVmPub.outparamcount )
	{
		Scr_Error(va("parameter %d does not exist", index + 1));
		return 0;
	}

	entryValue = Scr_GetValue(index);

	if ( entryValue->type != VAR_ISTRING )
	{
		scrVarPub.error_index = index + 1;
		Scr_Error(va("type %s is not a localized string", var_typename[entryValue->type]));
	}

	return entryValue->u.stringValue;
}

const char* __cdecl Scr_GetIString(unsigned int index)
{
    return SL_ConvertToString(Scr_GetConstIString(index));
}

void __cdecl Scr_GetVector(unsigned int index, float* vectorValue)
{
    const char* v2; // eax
    const char* v3; // eax
    const float* vecValue; // [esp+0h] [ebp-8h]
    VariableValue* value; // [esp+4h] [ebp-4h]

    if (index < scrVmPub.outparamcount)
    {
        value = &scrVmPub.top[-(int)index];
        if (value->type == 4)
        {
            vecValue = value->u.vectorValue;
            vectorValue[0] = vecValue[0];
            vectorValue[1] = vecValue[1];
            vectorValue[2] = vecValue[2];
            return;
        }
        scrVarPub.error_index = index + 1;
        v2 = va("type %s is not a vector", var_typename[value->type]);
        Scr_Error(v2);
    }
    v3 = va("parameter %d does not exist", index + 1);
    Scr_Error(v3);
}

scr_entref_t __cdecl Scr_GetEntityRef(unsigned int index)
{
    unsigned int ObjectType; // eax
    VariableValue* value; // [esp+8h] [ebp-8h]
    unsigned int id; // [esp+Ch] [ebp-4h]

    if (index < scrVmPub.outparamcount)
    {
        value = &scrVmPub.top[-(int)index];
        if (value->type == VAR_POINTER)
        {
            id = value->u.intValue;
            if (GetObjectType(value->u.intValue) == VAR_ENTITY)
                return Scr_GetEntityIdRef(id);
            scrVarPub.error_index = index + 1;
            ObjectType = GetObjectType(id);
            Scr_Error(va("type %s is not an entity", var_typename[ObjectType]));
        }
        scrVarPub.error_index = index + 1;
        Scr_Error(va("type %s is not an entity", var_typename[value->type]));
    }
    Scr_Error(va("parameter %d does not exist", index + 1));
    return 0;
}

unsigned int __cdecl Scr_GetObject(unsigned int paramnum)
{
	VariableValue *var;

	if (paramnum >= scrVmPub.outparamcount)
	{
		Scr_Error(va("parameter %d does not exist", paramnum + 1));
		return 0;
	}

	var = Scr_GetValue(paramnum);

	if (var->type == VAR_POINTER)
	{
		return var->u.pointerValue;
	}

	scrVarPub.error_index = paramnum + 1;
	Scr_Error(va("type %s is not an object", var_typename[var->type]));
	return 0;
}

int __cdecl Scr_GetType(unsigned int index)
{
    if (index < scrVmPub.outparamcount)
        return scrVmPub.top[-(int)index].type;
    Scr_Error(va("parameter %d does not exist", index + 1));
    return 0;
}

const char* __cdecl Scr_GetTypeName(unsigned int index)
{
    const char* v2; // eax

    if (index < scrVmPub.outparamcount)
        return var_typename[scrVmPub.top[-(int)index].type];
    v2 = va("parameter %d does not exist", index + 1);
    Scr_Error(v2);
    return 0;
}

unsigned int __cdecl Scr_GetPointerType(unsigned int index)
{
    if (index < scrVmPub.outparamcount)
    {
        if (scrVmPub.top[-(int)index].type == VAR_POINTER)
            return GetObjectType(scrVmPub.top[-(int)index].u.stringValue);
        Scr_Error(va("type %s is not an object", var_typename[scrVmPub.top[-(int)index].type]));
    }
    Scr_Error(va("parameter %d does not exist", index + 1));
    return 0;
}

unsigned int __cdecl Scr_GetNumParam()
{
    return scrVmPub.outparamcount;
}

void __cdecl Scr_AddBool(unsigned int value)
{
    iassert(value == 0 || value == 1);

    IncInParam();
    scrVmPub.top->type = VAR_INTEGER;
    scrVmPub.top->u.intValue = value;
}

void IncInParam()
{
    if ((scrVmPub.top < (VariableValue*)&scrVmGlob - 1 || scrVmPub.top >(VariableValue*) & scrVmGlob)
        && (scrVmPub.top < scrVmPub.stack || scrVmPub.top > scrVmPub.maxstack))
    {
        MyAssertHandler(
            ".\\script\\scr_vm.cpp",
            3894,
            0,
            "%s",
            "((scrVmPub.top >= scrVmGlob.eval_stack - 1) && (scrVmPub.top <= scrVmGlob.eval_stack)) || ((scrVmPub.top >= scrVmP"
            "ub.stack) && (scrVmPub.top <= scrVmPub.maxstack))");
    }
    Scr_ClearOutParams();
    if (scrVmPub.top == scrVmPub.maxstack)
        Sys_Error("Internal script stack overflow");
    ++scrVmPub.top;
    ++scrVmPub.inparamcount;
    if ((scrVmPub.top < (VariableValue*)&scrVmGlob || scrVmPub.top > &scrVmGlob.eval_stack[1])
        && (scrVmPub.top < scrVmPub.stack || scrVmPub.top > scrVmPub.maxstack))
    {
        MyAssertHandler(
            ".\\script\\scr_vm.cpp",
            3904,
            0,
            "%s",
            "((scrVmPub.top >= scrVmGlob.eval_stack) && (scrVmPub.top <= scrVmGlob.eval_stack + 1)) || ((scrVmPub.top >= scrVmP"
            "ub.stack) && (scrVmPub.top <= scrVmPub.maxstack))");
    }
}

void __cdecl Scr_AddInt(int value)
{
    IncInParam();
    scrVmPub.top->type = VAR_INTEGER;
    scrVmPub.top->u.intValue = value;
}

void __cdecl Scr_AddFloat(float value)
{
    IncInParam();
    scrVmPub.top->type = VAR_FLOAT;
    scrVmPub.top->u.floatValue = value;
}

void __cdecl Scr_AddAnim(scr_anim_s value)
{
    IncInParam();
    scrVmPub.top->type = VAR_ANIMATION;
    scrVmPub.top->u.codePosValue = value.linkPointer;
}

void __cdecl Scr_AddUndefined()
{
    IncInParam();
    scrVmPub.top->type = VAR_UNDEFINED;
}

void __cdecl Scr_AddObject(unsigned int id)
{
    if (!id)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4891, 0, "%s", "id");
    if (GetObjectType(id) == 14)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4894, 0, "%s", "GetObjectType( id ) != VAR_THREAD");
    if (GetObjectType(id) == 15)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4895, 0, "%s", "GetObjectType( id ) != VAR_NOTIFY_THREAD");
    if (GetObjectType(id) == 16)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4896, 0, "%s", "GetObjectType( id ) != VAR_TIME_THREAD");
    if (GetObjectType(id) == 17)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4897, 0, "%s", "GetObjectType( id ) != VAR_CHILD_THREAD");
    if (GetObjectType(id) == 22)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4898, 0, "%s", "GetObjectType( id ) != VAR_DEAD_THREAD");
    IncInParam();
    scrVmPub.top->type = VAR_POINTER;
    scrVmPub.top->u.intValue = id;
    AddRefToObject(id);
}

void __cdecl Scr_AddEntityNum(unsigned int entnum, unsigned int classnum)
{
    unsigned int EntityId; // eax
    const char* varUsagePos; // [esp+0h] [ebp-4h]

    varUsagePos = scrVarPub.varUsagePos;
    if (!scrVarPub.varUsagePos)
        scrVarPub.varUsagePos = "<script entity variable>";
    EntityId = Scr_GetEntityId(entnum, classnum);
    Scr_AddObject(EntityId);
    scrVarPub.varUsagePos = varUsagePos;
}

void __cdecl Scr_AddStruct()
{
    unsigned int id; // [esp+0h] [ebp-4h]

    id = AllocObject();
    Scr_AddObject(id);
    RemoveRefToObject(id);
}

void __cdecl Scr_AddString(const char* value)
{
    if (!value)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4946, 0, "%s", "value");
    IncInParam();
    scrVmPub.top->type = VAR_STRING;
    scrVmPub.top->u.intValue = SL_GetString(value, 0);
}

void __cdecl Scr_AddIString(const char* value)
{
    if (!value)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4956, 0, "%s", "value");
    IncInParam();
    scrVmPub.top->type = VAR_ISTRING;
    scrVmPub.top->u.intValue = SL_GetString(value, 0);
}

void __cdecl Scr_AddConstString(unsigned int value)
{
    if (!value)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4966, 0, "%s", "value");
    IncInParam();
    scrVmPub.top->type = VAR_STRING;
    scrVmPub.top->u.intValue = value;
    SL_AddRefToString(value);
}

void __cdecl Scr_AddVector(const float* value)
{
    IncInParam();
    scrVmPub.top->type = VAR_VECTOR;
    scrVmPub.top->u.intValue = (int)Scr_AllocVector(value);
}

void __cdecl Scr_MakeArray()
{
    IncInParam();
    scrVmPub.top->type = VAR_POINTER;
    scrVmPub.top->u.intValue = Scr_AllocArray();
}

void __cdecl Scr_AddArray()
{
    unsigned int ArraySize; // eax
    const char* varUsagePos; // [esp+0h] [ebp-8h]
    unsigned int id; // [esp+4h] [ebp-4h]

    varUsagePos = scrVarPub.varUsagePos;
    if (!scrVarPub.varUsagePos)
        scrVarPub.varUsagePos = "<script array variable>";
    iassert(scrVmPub.inparamcount);
    --scrVmPub.top;
    --scrVmPub.inparamcount;
    iassert(scrVmPub.top->type == VAR_POINTER);
    ArraySize = GetArraySize(scrVmPub.top->u.stringValue);
    id = GetNewArrayVariable(scrVmPub.top->u.stringValue, ArraySize);
    SetNewVariableValue(id, scrVmPub.top + 1);
    scrVarPub.varUsagePos = varUsagePos;
}

void __cdecl Scr_AddArrayStringIndexed(unsigned int stringValue)
{
    unsigned int id; // [esp+0h] [ebp-4h]

    if (!scrVmPub.inparamcount)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5019, 0, "%s", "scrVmPub.inparamcount");
    --scrVmPub.top;
    --scrVmPub.inparamcount;
    if (scrVmPub.top->type != 1)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5022, 0, "%s", "scrVmPub.top->type == VAR_POINTER");
    id = GetNewVariable(scrVmPub.top->u.stringValue, stringValue);
    SetNewVariableValue(id, scrVmPub.top + 1);
}

void __cdecl Scr_Error(const char* error)
{
    Scr_SetErrorMessage(error);
    Scr_ErrorInternal();
}

char error_message[1024];
void __cdecl Scr_SetErrorMessage(const char* error)
{
    if (!scrVarPub.error_message)
    {
        I_strncpyz(error_message, error, 1024);
        scrVarPub.error_message = error_message;
    }
}

void __cdecl Scr_TerminalError(const char* error)
{
    Scr_DumpScriptThreads();
    Scr_DumpScriptVariablesDefault();
    scrVmPub.terminal_error = 1;
    Scr_Error(error);
}

void __cdecl Scr_NeverTerminalError(const char* error)
{
    if (scrVmGlob.loading)
    {
        Scr_SetErrorMessage(error);
        longjmp(g_script_error[g_script_error_level], -1); // KISAKTRYCATCH
    }
    Scr_Error(error);
}

void __cdecl Scr_ParamError(unsigned int index, const char* error)
{
    if (index >= scrVmPub.outparamcount)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5078, 0, "%s", "index < scrVmPub.outparamcount");
    scrVarPub.error_index = index + 1;
    Scr_Error(error);
}

void __cdecl Scr_ObjectError(const char* error)
{
    scrVarPub.error_index = -1;
    Scr_Error(error);
}

char __cdecl SetEntityFieldValue(unsigned int classnum, int entnum, int offset, VariableValue* value)
{
    if (value - scrVmPub.stack <= 0)
        MyAssertHandler(
            ".\\script\\scr_vm.cpp",
            5093,
            0,
            "%s\n\t(value - scrVmPub.stack) = %i",
            "(value - scrVmPub.stack > 0)",
            value - scrVmPub.stack);
    if (value - scrVmPub.maxstack > 0)
        MyAssertHandler(
            ".\\script\\scr_vm.cpp",
            5094,
            0,
            "%s\n\t(value - scrVmPub.maxstack) = %i",
            "(value - scrVmPub.maxstack <= 0)",
            value - scrVmPub.maxstack);
    if (scrVmPub.inparamcount)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5095, 0, "%s", "!scrVmPub.inparamcount");
    if (scrVmPub.outparamcount)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5096, 0, "%s", "!scrVmPub.outparamcount");
    scrVmPub.outparamcount = 1;
    scrVmPub.top = value;
    if (Scr_SetObjectField(classnum, entnum, offset))
    {
        if (scrVmPub.inparamcount)
            MyAssertHandler(".\\script\\scr_vm.cpp", 5108, 0, "%s", "!scrVmPub.inparamcount");
        if (scrVmPub.outparamcount)
        {
            if (scrVmPub.outparamcount != 1)
                MyAssertHandler(".\\script\\scr_vm.cpp", 5112, 0, "%s", "scrVmPub.outparamcount == 1");
            RemoveRefToValue(scrVmPub.top->type, scrVmPub.top->u);
            --scrVmPub.top;
            scrVmPub.outparamcount = 0;
        }
        return 1;
    }
    else
    {
        if (scrVmPub.inparamcount)
            MyAssertHandler(".\\script\\scr_vm.cpp", 5102, 0, "%s", "!scrVmPub.inparamcount");
        if (scrVmPub.outparamcount != 1)
            MyAssertHandler(".\\script\\scr_vm.cpp", 5103, 0, "%s", "scrVmPub.outparamcount == 1");
        scrVmPub.outparamcount = 0;
        return 0;
    }
}

VariableValue __cdecl GetEntityFieldValue(unsigned int classnum, int entnum, int offset)
{
    iassert(!scrVmPub.inparamcount);
    iassert(!scrVmPub.outparamcount);

    scrVmPub.top = scrVmGlob.eval_stack - 1;
    scrVmGlob.eval_stack[0].type = VAR_UNDEFINED;

    Scr_GetObjectField(classnum, entnum, offset);
    iassert(!scrVmPub.inparamcount || scrVmPub.inparamcount == 1);
    iassert(!scrVmPub.outparamcount);
    iassert(scrVmPub.top - scrVmPub.inparamcount == scrVmGlob.eval_stack - 1);

    scrVmPub.inparamcount = 0;
    return scrVmGlob.eval_stack[0];
}

void __cdecl Scr_SetStructField(unsigned int structId, unsigned int index)
{
    unsigned int fieldValueId; // [esp+0h] [ebp-8h]
    unsigned int fieldValueIndex; // [esp+4h] [ebp-4h]

    if (scrVmPub.outparamcount)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5146, 0, "%s", "!scrVmPub.outparamcount");
    if (scrVmPub.inparamcount != 1)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5147, 0, "%s", "scrVmPub.inparamcount == 1");
    if (scrVarPub.varUsagePos)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5150, 0, "%s", "!scrVarPub.varUsagePos");
    scrVarPub.varUsagePos = "<radiant field variable>";
    fieldValueIndex = Scr_GetVariableFieldIndex(structId, index);
    fieldValueId = Scr_GetVarId(fieldValueIndex);
    if (scrVmPub.inparamcount != 1)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5157, 0, "%s", "scrVmPub.inparamcount == 1");
    scrVmPub.inparamcount = 0;
    SetVariableFieldValue(fieldValueId, scrVmPub.top);
    if (scrVmPub.inparamcount)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5162, 0, "%s", "!scrVmPub.inparamcount");
    if (scrVmPub.outparamcount)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5163, 0, "%s", "!scrVmPub.outparamcount");
    --scrVmPub.top;
    scrVarPub.varUsagePos = 0;
}

void __cdecl Scr_SetDynamicEntityField(unsigned int entnum, unsigned int classnum, unsigned int index)
{
    unsigned int entId; // [esp+0h] [ebp-4h]

    if (scrVarPub.varUsagePos)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5178, 0, "%s", "!scrVarPub.varUsagePos");
    scrVarPub.varUsagePos = "<radiant field variable>";
    entId = Scr_GetEntityId(entnum, classnum);
    if (GetObjectType(entId) != 20)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5183, 0, "%s", "GetObjectType( entId ) == VAR_ENTITY");
    scrVarPub.varUsagePos = 0;
    Scr_SetStructField(entId, index);
}

void __cdecl Scr_IncTime()
{
    Scr_RunCurrentThreads();
    Scr_FreeEntityList();
    if ((scrVarPub.time & 0xFF000000) != 0)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5198, 0, "%s", "!(scrVarPub.time & ~VAR_NAME_LOW_MASK)");
    ++scrVarPub.time;
    scrVarPub.time &= 0xFFFFFFu;
    scrVmPub.showError = scrVmPub.abort_on_error;
}

void __cdecl Scr_RunCurrentThreads()
{
    PROF_SCOPED("Scr_RunCurrentThreads");

    iassert(!scrVmPub.function_count);
    iassert(!scrVarPub.error_message);
    iassert(!scrVarPub.error_index);
    iassert(!scrVmPub.outparamcount);
    iassert(!scrVmPub.inparamcount);
    iassert(scrVmPub.top == scrVmPub.stack);

    VM_SetTime();
}

void VM_SetTime()
{
    unsigned int Object; // eax
    unsigned int id; // [esp+0h] [ebp-4h]

    iassert(!(scrVarPub.time & ~VAR_NAME_LOW_MASK));

    if (scrVarPub.timeArrayId)
    {
        id = FindVariable(scrVarPub.timeArrayId, scrVarPub.time);
        if (id)
        {
            iassert(logScriptTimes);
            if (logScriptTimes->current.enabled)
            {
                Com_Printf(23, "SET TIME: %d\n", Sys_Milliseconds());
            }
            Object = FindObject(id);
            VM_Resume(Object);
            SafeRemoveVariable(scrVarPub.timeArrayId, scrVarPub.time);
        }
    }
}

void __cdecl VM_Resume(unsigned int timeId)
{
    unsigned int v1; // eax
    unsigned int v2; // eax
    const char* pos; // [esp+18h] [ebp-14h]
    int time; // [esp+1Ch] [ebp-10h]
    VariableStackBuffer* stackValue; // [esp+20h] [ebp-Ch]
    unsigned int stackId; // [esp+24h] [ebp-8h]
    unsigned int startLocalId; // [esp+28h] [ebp-4h]
    function_stack_t stack;

    PROF_SCOPED("VM_Resume");

    iassert(scrVmPub.top == scrVmPub.stack);

    //Scr_ResetAbortDebugger(inst); // blops
    Scr_ResetTimeout();

    iassert(timeId);

    AddRefToObject(timeId);
    fs.startTop = scrVmPub.stack;
    thread_count = 0;

    while (1)
    {
        iassert(!scrVarPub.error_message);
        iassert(!scrVarPub.error_index);
        iassert(!scrVmPub.outparamcount);
        iassert(!scrVmPub.inparamcount);
        iassert(!scrVmPub.function_count);
        iassert(scrVmPub.localVars == scrVmGlob.localVarsStack - 1);
        iassert(fs.startTop == &scrVmPub.stack[0]);
        iassert(!thread_count);

        stackId = FindFirstSibling(timeId);
        if (!stackId)
            break;

        //stack.startTop = scrVmPub.stack;

        startLocalId = GetVariableKeyObject(stackId);
        iassert(startLocalId);
        iassert(GetValueType(stackId) == VAR_STACK);
        stackValue = GetVariableValueAddress(stackId)->u.stackValue;
        RemoveObjectVariable(timeId, startLocalId);
        VM_UnarchiveStack(startLocalId, stackValue);
        //VM_UnarchiveStack2(startLocalId, &stack, stackValue);
        if (scrVarPub.bScriptProfile)
        {
            scrVmDebugPub.builtInTime = 0;
            time = __rdtsc();
            pos = fs.pos;
            //v2 = VM_Execute_0();
            //v2 = VM_ExecuteInternal(stack.pos, stack.localId, stack.localVarCount, stack.top, stack.startTop);
            v2 = VM_ExecuteInternal();
            RemoveRefToObject(v2);
            RemoveRefToValue(scrVmPub.stack[1].type, scrVmPub.stack[1].u);
            Scr_AddProfileTime(pos, __rdtsc() - time, scrVmDebugPub.builtInTime);
        }
        else
        {
            //v1 = VM_Execute_0();
            //v1 = VM_ExecuteInternal(stack.pos, stack.localId, stack.localVarCount, stack.top, stack.startTop);
            v1 = VM_ExecuteInternal();
            RemoveRefToObject(v1);
            RemoveRefToValue(scrVmPub.stack[1].type, scrVmPub.stack[1].u);
        }
    }
    RemoveRefToObject(timeId);
    ClearVariableValue(scrVarPub.tempVariable);
    scrVmPub.top = scrVmPub.stack;
}

void __cdecl VM_UnarchiveStack(unsigned int startLocalId, VariableStackBuffer* stackValue)
{
    VariableValue* top; // [esp+0h] [ebp-14h]
    char* buf; // [esp+4h] [ebp-10h]
    unsigned int localId; // [esp+8h] [ebp-Ch]
    int function_count; // [esp+Ch] [ebp-8h]
    int size; // [esp+10h] [ebp-4h]

    iassert(!scrVmPub.function_count);
    iassert(stackValue->pos);
    iassert(fs.startTop == &scrVmPub.stack[0]);

    scrVmPub.function_frame->fs.pos = stackValue->pos;
    ++scrVmPub.function_count;
    ++scrVmPub.function_frame;
    size = stackValue->size;
    buf = stackValue->buf;
    top = scrVmPub.stack;

    while (size)
    {
        ++top;
        --size;
        top->type = (Vartype_t)*(unsigned char*)buf;
        buf += 1;

        if (top->type == VAR_CODEPOS)
        {
            iassert(scrVmPub.function_count < 32 /*MAX_VM_STACK_DEPTH*/);

            scrVmPub.function_frame->fs.pos = *(const char**)buf;
            ++scrVmPub.function_count;
            ++scrVmPub.function_frame;
        }
        else
        {
            top->u.codePosValue = *(const char**)buf;
        }

        buf += 4;
    }
    fs.pos = stackValue->pos;
    fs.top = top;
    localId = stackValue->localId;
    fs.localId = localId;
    Scr_ClearWaitTime(startLocalId);

    iassert(scrVmPub.function_count < 32 /*MAX_VM_STACK_DEPTH*/);

    function_count = scrVmPub.function_count;

    while (1)
    {
        scrVmPub.function_frame_start[function_count--].fs.localId = localId;

        if (!function_count)
            break;

        localId = GetParentLocalId(localId);
    }

    while (++function_count != scrVmPub.function_count)
    {
        //scrVmPub.stack[3 * function_count - 95].u.intValue = Scr_AddLocalVars(scrVmPub.function_frame_start[function_count].fs.localId);
        scrVmPub.function_frame_start[function_count].fs.localVarCount = Scr_AddLocalVars(scrVmPub.function_frame_start[function_count].fs.localId);
    }

    fs.localVarCount = Scr_AddLocalVars(fs.localId);

    if (stackValue->time != LOBYTE(scrVarPub.time))
        Scr_ResetTimeout();

    --scrVarPub.numScriptThreads;
    MT_Free((byte*)stackValue, stackValue->bufLen);

    iassert(scrVmPub.stack[0].type == VAR_CODEPOS);
}

void VM_UnarchiveStack2(unsigned int startLocalId, function_stack_t *stack, VariableStackBuffer *stackValue)
{
    int function_count;
    unsigned int localId;
    VariableValue *startTop;
    int size;
    const char *buf;
    const char *pos;

    iassert(!scrVmPub.function_count);
    iassert(stackValue->pos);
    iassert(fs.startTop == &scrVmPub.stack[0]);

    scrVmPub.function_frame->fs.pos = stackValue->pos;
    ++scrVmPub.function_count;
    ++scrVmPub.function_frame;
    size = stackValue->size;
    buf = stackValue->buf;
    startTop = stack->startTop;

    while (size)
    {
        ++startTop;
        --size;
        startTop->type = (Vartype_t)*(unsigned char *)buf;
        pos = buf + 1;

        if (startTop->type == VAR_CODEPOS)
        {
            iassert(scrVmPub.function_count < 32/*MAX_VM_STACK_DEPTH*/);
            scrVmPub.function_frame->fs.pos = *(const char **)pos;
            ++scrVmPub.function_count;
            ++scrVmPub.function_frame;
        }
        else
        {
            startTop->u.intValue = *(int *)pos;
        }

        buf = (pos + 4);
    }

    stack->pos = stackValue->pos;
    stack->top = startTop;
    localId = stackValue->localId;
    stack->localId = localId;
    Scr_ClearWaitTime(startLocalId);
    iassert(scrVmPub.function_count < 32/*MAX_VM_STACK_DEPTH*/);
    function_count = scrVmPub.function_count;

    while (1)
    {
        scrVmPub.function_frame_start[function_count--].fs.localId = localId;

        if (!function_count)
            break;

        localId = GetParentLocalId(localId);
    }

    while (++function_count != scrVmPub.function_count)
    {
        scrVmPub.function_frame_start[function_count].fs.localVarCount = Scr_AddLocalVars(scrVmPub.function_frame_start[function_count].fs.localId);
    }

    stack->localVarCount = Scr_AddLocalVars(stack->localId);

    if (stackValue->time != LOBYTE(scrVarPub.time))
        Scr_ResetTimeout();

    --scrVarPub.numScriptThreads;

    MT_Free((unsigned char*)stackValue, stackValue->bufLen);

    //iassert(scrVmPub.stack[0].type == VAR_CODEPOS);
    iassert(stack->startTop[0].type == VAR_CODEPOS);
}

int __cdecl Scr_AddLocalVars(unsigned int localId)
{
    int localVarCount; // [esp+0h] [ebp-8h]
    unsigned int fieldIndex; // [esp+4h] [ebp-4h]

    localVarCount = 0;
    for (fieldIndex = FindLastSibling(localId); fieldIndex; fieldIndex = FindPrevSibling(fieldIndex))
    {
        ++scrVmPub.localVars;
        *scrVmPub.localVars = Scr_GetVarId(fieldIndex);
        ++localVarCount;
    }
    return localVarCount;
}

void __cdecl Scr_ResetTimeout()
{
    DWORD v0; // eax

    scrVmGlob.starttime = Sys_Milliseconds();
    iassert(logScriptTimes);
    if (logScriptTimes->current.enabled)
    {
        v0 = Sys_Milliseconds();
        Com_Printf(23, "RESET TIME: %d\n", v0);
    }
    memset(scrVmDebugPub.jumpbackHistory, 0, sizeof(scrVmDebugPub.jumpbackHistory));
}

BOOL __cdecl Scr_IsStackClear()
{
    iassert(!Sys_IsRemoteDebugClient());
    return scrVmPub.top == scrVmPub.stack;
}

void __cdecl Scr_StackClear()
{
    scrVmPub.top = scrVmPub.stack;
}

void __cdecl Scr_ProfileUpdate()
{
#if 0
    int total; // [esp+4h] [ebp-Ch] BYREF
    int totalNonBuiltIn; // [esp+8h] [ebp-8h] BYREF
    const char* profileString; // [esp+Ch] [ebp-4h]

    profileString = Dvar_EnumToString(profile);
    if (scrVarPub.bScriptProfile)
    {
        if (I_stricmp(profileString, "ai"))
        {
            if (profile_script_by_file->current.enabled)
            {
                Scr_CalcScriptFileProfile();
            }
            else if (Scr_PrintProfileTimes(scrVarPub.scriptProfileMinTime))
            {
                scrVarPub.bScriptProfile = 0;
            }
        }
        else
        {
            Scr_CalcAnimscriptProfile(&total, &totalNonBuiltIn);
            Profile_SetTotal(236, total);
            Profile_SetTotal(237, totalNonBuiltIn);
        }
    }
    else if (!I_stricmp(profileString, "ai") || profile_script_by_file->current.enabled)
    {
        scrVarPub.bScriptProfile = 1;
    }
#endif
}

void __cdecl Scr_ProfileBuiltinUpdate()
{
#if 0
    int i; // [esp+4h] [ebp-4h]

    if (scrVarPub.bScriptProfileBuiltin)
    {
        if (Scr_PrintProfileBuiltinTimes(scrVarPub.scriptProfileBuiltinMinTime))
            scrVarPub.bScriptProfileBuiltin = 0;
    }
    else
    {
        for (i = 0; i < scrCompilePub.func_table_size; ++i)
        {
            scrVmDebugPub.func_table[i].usage = 0;
            scrVmDebugPub.func_table[i].prof = 0;
        }
    }
#endif
}

void __cdecl Scr_DoProfile(float minTime)
{
#if 0
    scrVarPub.bScriptProfile = 1;
    scrVarPub.scriptProfileMinTime = minTime;
#endif
}

void __cdecl Scr_DoProfileBuiltin(float minTime)
{
#if 0
    scrVarPub.bScriptProfileBuiltin = 1;
    scrVarPub.scriptProfileBuiltinMinTime = minTime;
#endif
}

char __cdecl Scr_PrintProfileBuiltinTimes(float minTime)
{
    return 0;
#if 0
    float v2; // [esp+10h] [ebp-20h]
    int j; // [esp+20h] [ebp-10h]
    int* order; // [esp+24h] [ebp-Ch]
    int time; // [esp+28h] [ebp-8h]
    int i; // [esp+2Ch] [ebp-4h]
    int ia; // [esp+2Ch] [ebp-4h]
    int ib; // [esp+2Ch] [ebp-4h]
    int ic; // [esp+2Ch] [ebp-4h]

    if (minTime <= 0.0)
        goto LABEL_10;
    time = 0;
    for (i = 0; i < scrCompilePub.func_table_size; time += scrVmDebugPub.func_table[i++].prof)
        ;
    if (minTime <= (double)time * *((float*)Sys_GetValue(0) + 20782))
    {
    LABEL_10:
        order = (int*)Z_VirtualAlloc(4 * scrCompilePub.func_table_size, "Scr_PrintProfileBuiltinTimes", 0);
        for (ib = 0; ib < scrCompilePub.func_table_size; ++ib)
            order[ib] = ib;
        qsort(order, scrCompilePub.func_table_size, 4u, (int(*)(const void*, const void*))Scr_BuiltinCompare);
        for (ic = 0; ic < scrCompilePub.func_table_size; ++ic)
        {
            j = order[ic];
            if (scrVmDebugPub.func_table[j].usage)
            {
                v2 = *((float*)Sys_GetValue(0) + 20782);
                Com_Printf(
                    23,
                    "time: %f, usage: %d, %s\n",
                    (double)scrVmDebugPub.func_table[j].prof * v2,
                    scrVmDebugPub.func_table[j].usage,
                    scrVmDebugPub.func_table[j].name);
                scrVmDebugPub.func_table[j].usage = 0;
                scrVmDebugPub.func_table[j].prof = 0;
            }
            else if (scrVmDebugPub.func_table[j].prof)
            {
                MyAssertHandler(".\\script\\scr_vm.cpp", 5421, 0, "%s", "!scrVmDebugPub.func_table[j].prof");
            }
        }
        Z_VirtualFree(order);
        return 1;
    }
    else
    {
        for (ia = 0; ia < scrCompilePub.func_table_size; ++ia)
        {
            scrVmDebugPub.func_table[ia].usage = 0;
            scrVmDebugPub.func_table[ia].prof = 0;
        }
        return 0;
    }
#endif
}

int __cdecl Scr_BuiltinCompare(_DWORD* a, _DWORD* b)
{
    return scrVmDebugPub.func_table[*a].prof - scrVmDebugPub.func_table[*b].prof;
}

void Scr_DecTime()
{
    unsigned int time; // r11

    time = scrVarPub.time;
    iassert(!(scrVarPub.time & ~VAR_NAME_LOW_MASK));
    scrVarPub.time = time - 1;
    scrVarPub.time &= 0x00FFFFFF; // Zero out highest byte to ensure the unsigned int didn't rollback to 4 billion
    //HIBYTE(scrVarPub.time) = 0; 
}


void Scr_AddExecEntThreadNum(int entnum, unsigned int classnum, int handle, unsigned int paramcount)
{
    int v9; // r3
    const char *varUsagePos; // r27
    unsigned int EntityId; // r28
    unsigned int v12; // r3
    unsigned int v13; // r3

    //_R29 = &scrVarPub.programBuffer[handle];
    const char *pos = &scrVarPub.programBuffer[handle];
    //__asm { dcbt      0, r29 } // prefetches into l1 data cache
    if (!scrVmPub.function_count)
    {
        if ((int *)scrVmPub.localVars != &scrVmGlob.starttime)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\script\\scr_vm.cpp",
                4192,
                0,
                "%s",
                "scrVmPub.localVars == scrVmGlob.localVarsStack - 1");
        //Profile_Begin(336);
        Scr_ResetTimeout();
    }
    iassert(scrVarPub.timeArrayId);
    iassert(handle);
    iassert(Scr_IsInOpcodeMemory(pos));

    v9 = entnum;
    varUsagePos = scrVarPub.varUsagePos;
    scrVarPub.varUsagePos = pos + 1;
    EntityId = Scr_GetEntityId(v9, classnum);
    AddRefToObject(EntityId);
    v12 = AllocThread(EntityId);
    v13 = VM_Execute(v12, pos, paramcount);
    RemoveRefToObject(v13);
    scrVarPub.varUsagePos = varUsagePos;
    ++scrVmPub.outparamcount;
    --scrVmPub.inparamcount;
    if (!scrVmPub.function_count)
    {
        //Profile_EndInternal(0);
        if ((int *)scrVmPub.localVars != &scrVmGlob.starttime)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\script\\scr_vm.cpp",
                4223,
                0,
                "%s",
                "scrVmPub.localVars == scrVmGlob.localVarsStack - 1");
    }
}

void Scr_ErrorWithDialogMessage(const char *error, const char *dialog_error)
{
    I_strncpyz(error_message, error, 1024);
    scrVmGlob.dialog_error_message = dialog_error;
    scrVarPub.error_message = error_message;
    Scr_ErrorInternal();
}

unsigned int Scr_GetFunc(unsigned int index)
{
    VariableValue *value; // r29

    if (index < scrVmPub.outparamcount)
    {
        value = &scrVmPub.top[-index];
        if (value->type == 9)
        {
            if (!Scr_IsInOpcodeMemory(value->u.codePosValue))
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\script\\scr_vm.cpp",
                    4746,
                    0,
                    "%s",
                    "Scr_IsInOpcodeMemory( value->u.codePosValue )");
            return value->u.intValue - (unsigned int)scrVarPub.programBuffer;
        }
        scrVarPub.error_index = index + 1;
        Scr_Error(va("type %s is not a function", var_typename[value->type]));
    }
    if (!scrVarPub.error_message)
    {
        I_strncpyz(error_message, va("parameter %d does not exist", index + 1), 1024);
        scrVarPub.error_message = error_message;
    }
    Scr_ErrorInternal();
    return 0;
}

void Scr_SetRecordScriptPlace(int on)
{
    bool v1; // r11

    if (on != 1 || (v1 = 1, !scrVarPub.developer))
        v1 = 0;
    scrVmGlob.recordPlace = v1;
}

void Scr_GetLastScriptPlace(int *line, const char **filename)
{
    const char *lastFileName; // r11

    *line = scrVmGlob.lastLine;
    lastFileName = scrVmGlob.lastFileName;
    if (!scrVmGlob.lastFileName)
        lastFileName = "";
    *filename = lastFileName;
}

XAnim_s * Scr_GetAnimTree(unsigned int index)
{
    VariableValue *v3; // r29
    int type; // r11
    VariableUnion *v5; // r11
    int v7; // r4
    int v8; // r3
    const char *v9; // r3
    const char *v10; // r4

    if (index < scrVmPub.outparamcount)
    {
        v3 = &scrVmPub.top[-index];
        type = v3->type;
        if (type == 6)
        {
            if (v3->u.intValue <= scrAnimPub.xanim_num[1])
            {
                v5 = (VariableUnion *)(4 * v3->u.intValue);
                if (*(unsigned int *)((char *)&scrAnimPub.xanim_num[-128] + (_DWORD)v5))
                    return *(XAnim_s **)((char *)&scrAnimPub.xanim_num[-128] + (_DWORD)v5);
            }
            scrVarPub.error_message = "bad anim tree";
        }
        else
        {
            scrVarPub.error_message = va("type %s is not an animtree", var_typename[type]);
        }
        RemoveRefToValue(v3);
        v3->type = VAR_UNDEFINED;
        scrVarPub.error_index = index + 1;
        Scr_ErrorInternal();
    }
    v9 = va("parameter %d does not exist", index + 1);
    v10 = v9;
    if (!scrVarPub.error_message)
    {
        I_strncpyz(error_message, v9, 1024);
        scrVarPub.error_message = error_message;
    }
    Scr_ErrorInternal();
    return scrAnimPub.xanim_lookup[1][0].anims;
}

#pragma warning(pop)