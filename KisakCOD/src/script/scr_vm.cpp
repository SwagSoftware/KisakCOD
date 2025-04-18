#include "scr_vm.h"

#include "scr_animtree.h"
#include "scr_debugger.h"
#include "scr_parser.h"
#include "scr_main.h"
#include "scr_stringlist.h"

#include <database/database.h>

#include <bgame/bg_local.h>

#include <game_mp/g_public_mp.h>

#include <universal/com_memory.h>
#include <universal/com_files.h>
#include <win32/win_net_debug.h>
#include <ui/ui.h>
#include <win32/win_input.h>
#include <client/client.h>

scrVmPub_t scrVmPub;
scrVmGlob_t scrVmGlob;

function_stack_t pos;

int scr_initialized;

void __cdecl SCR_Init()
{
    scr_initialized = 1;
}

void GScr_GetAnimLength()
{
    float value; // [esp+0h] [ebp-10h]
    scr_anim_s anim; // [esp+8h] [ebp-8h]
    XAnim_s* anims; // [esp+Ch] [ebp-4h]

    anim = Scr_GetAnim(0, 0);
    anims = Scr_GetAnims(anim.tree);
    if (!XAnimIsPrimitive(anims, anim.index))
        Scr_ParamError(0, "non-primitive animation has no concept of length");
    value = XAnimGetLength(anims, anim.index);
    Scr_AddFloat(value);
}

void __cdecl Scr_ErrorOnDefaultAsset(XAssetType type, const char* assetName)
{
    const char* XAssetTypeName; // eax
    const char* v3; // eax

    DB_FindXAssetHeader(type, assetName);
    if (DB_IsXAssetDefault(type, assetName))
    {
        XAssetTypeName = DB_GetXAssetTypeName(type);
        v3 = va("precache %s '%s' failed", XAssetTypeName, assetName);
        Scr_NeverTerminalError(v3);
    }
}

void(__cdecl* __cdecl Scr_GetFunction(const char** pName, int* type))()
{
    unsigned int i; // [esp+18h] [ebp-4h]

    for (i = 0; i < 0xCD; ++i)
    {
        if (!strcmp(*pName, functions[i].actionString))
        {
            *pName = functions[i].actionString;
            *type = functions[i].type;
            return functions[i].actionFunc;
        }
    }
    return 0;
}

void __cdecl GScr_AddVector(const float* vVec)
{
    if (vVec)
        Scr_AddVector(vVec);
    else
        Scr_AddUndefined();
}

void __cdecl GScr_Shutdown()
{
    if (level.cachedTagMat.name)
        Scr_SetString(&level.cachedTagMat.name, 0);
}

void __cdecl Scr_GetObjectField(unsigned int classnum, int entnum, int offset)
{
    const char* v3; // eax

    if (classnum)
    {
        if (classnum == 1)
        {
            Scr_GetHudElemField(entnum, offset);
        }
        else if (!alwaysfails)
        {
            v3 = va("bad class num %u", classnum);
            MyAssertHandler(".\\game_mp\\g_spawn_mp.cpp", 760, 0, v3);
        }
    }
    else
    {
        Scr_GetEntityField(entnum, offset);
    }
}

void __cdecl GScr_SetDynamicEntityField(gentity_s* ent, unsigned int index)
{
    Scr_SetDynamicEntityField(ent->s.number, 0, index);
}

int __cdecl Scr_GetAnimsIndex(const XAnim_s* anims)
{
    int i; // [esp+4h] [ebp-4h]

    for (i = scrAnimPub.xanim_num[1]; i && (const XAnim_s*)scrAnimPub.xanim_num[i - 128] != anims; --i)
        ;
    return i;
}

XAnim_s* __cdecl Scr_GetAnims(unsigned int index)
{
    if (!index || index > scrAnimPub.xanim_num[1])
        MyAssertHandler(
            ".\\script\\scr_animtree.cpp",
            408,
            0,
            "index not in [1, scrAnimPub.xanim_num[SCR_XANIM_SERVER]]\n\t%i not in [%i, %i]",
            index,
            1,
            scrAnimPub.xanim_num[1]);
    return (XAnim_s*)scrAnimPub.xanim_num[index - 128];
}

int __cdecl Scr_GetAnimTreeSize(unsigned int parentNode)
{
    VariableValueInternal_u Object; // eax
    unsigned int node; // [esp+0h] [ebp-Ch]
    int size; // [esp+8h] [ebp-4h]

    size = 0;
    for (node = FindFirstSibling(parentNode); node; node = FindNextSibling(node))
    {
        if (GetVariableName(node) < 0x10000)
        {
            if (GetValueType(node) == 1)
            {
                Object = FindObject(node);
                size += Scr_GetAnimTreeSize(Object.u.stringValue);
            }
            else
            {
                ++size;
            }
        }
    }
    if (size)
        ++size;
    return size;
}

void __cdecl Scr_InitFromChildBlocks(scr_block_s** childBlocks, int childCount, scr_block_s* block)
{
    scr_block_s* childBlock; // [esp+0h] [ebp-14h]
    scr_block_s* childBlocka; // [esp+0h] [ebp-14h]
    int childIndex; // [esp+4h] [ebp-10h]
    int childIndexa; // [esp+4h] [ebp-10h]
    int localVarsCreateCount; // [esp+8h] [ebp-Ch]
    unsigned int name; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    if (childCount)
    {
        localVarsCreateCount = (*childBlocks)->localVarsPublicCount;
        for (childIndex = 1; childIndex < childCount; ++childIndex)
        {
            childBlock = childBlocks[childIndex];
            if (childBlock->localVarsPublicCount < localVarsCreateCount)
                localVarsCreateCount = childBlock->localVarsPublicCount;
        }
        if (block->localVarsCreateCount > localVarsCreateCount)
            MyAssertHandler(
                ".\\script\\scr_compiler.cpp",
                930,
                0,
                "%s",
                "block->localVarsCreateCount <= localVarsCreateCount");
        if (localVarsCreateCount > block->localVarsCount)
            MyAssertHandler(".\\script\\scr_compiler.cpp", 931, 0, "%s", "localVarsCreateCount <= block->localVarsCount");
        block->localVarsCreateCount = localVarsCreateCount;
        for (i = 0; i < localVarsCreateCount; ++i)
        {
            if (i >= block->localVarsCount)
                MyAssertHandler(".\\script\\scr_compiler.cpp", 936, 0, "%s", "i < block->localVarsCount");
            if (((1 << (i & 7)) & block->localVarsInitBits[i >> 3]) == 0)
            {
                name = block->localVars[i].name;
                for (childIndexa = 0; childIndexa < childCount; ++childIndexa)
                {
                    childBlocka = childBlocks[childIndexa];
                    if (localVarsCreateCount > childBlocka->localVarsPublicCount)
                        MyAssertHandler(
                            ".\\script\\scr_compiler.cpp",
                            943,
                            0,
                            "%s",
                            "localVarsCreateCount <= childBlock->localVarsPublicCount");
                    if (i >= childBlocka->localVarsPublicCount)
                        MyAssertHandler(".\\script\\scr_compiler.cpp", 944, 0, "%s", "i < childBlock->localVarsPublicCount");
                    if (childBlocka->localVars[i].name != name)
                        MyAssertHandler(".\\script\\scr_compiler.cpp", 945, 0, "%s", "childBlock->localVars[i].name == name");
                    if (((1 << (i & 7)) & childBlocka->localVarsInitBits[i >> 3]) == 0)
                        goto LABEL_12;
                }
                block->localVarsInitBits[i >> 3] |= 1 << (i & 7);
            }
        LABEL_12:
            ;
        }
    }
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

void __cdecl Scr_InitDebuggerMain()
{
    if (scrVarPub.developer)
    {
        if (scrDebuggerGlob.debugger_inited_main)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 7941, 0, "%s", "!scrDebuggerGlob.debugger_inited_main");
        if (!Sys_IsRemoteDebugClient())
        {
            scrDebuggerGlob.variableBreakpoints = (Scr_WatchElementDoubleNode_t**)Hunk_AllocDebugMem(
                393216,
                /*"scrDebuggerGlob.variableBreakpoints"*/);
            memset((unsigned __int8*)scrDebuggerGlob.variableBreakpoints, 0, 0x60000u);
            scrDebuggerGlob.assignHead = 0;
            scrDebuggerGlob.assignHeadCodePos = 0;
            scrDebuggerGlob.disableBreakpoints = 0;
        }
        UI_ScrollPane::Init(&scrDebuggerGlob.scriptScrollPane);
        UI_ScrollPane::Init(&scrDebuggerGlob.miscScrollPane);
        UI_VerticalDivider::Init(&scrDebuggerGlob.mainWindow);
        scrDebuggerGlob.debugger_inited_main = 1;
    }
}

void __cdecl Scr_ShutdownDebuggerMain()
{
    int j; // [esp+0h] [ebp-8h]
    Scr_OpcodeList_s* opcodeElement; // [esp+4h] [ebp-4h]

    if (scrVarPub.developer && scrDebuggerGlob.debugger_inited_main)
    {
        scrDebuggerGlob.debugger_inited_main = 0;
        if (!Sys_IsRemoteDebugClient())
        {
            if (scrDebuggerGlob.variableBreakpoints)
            {
                for (j = 0; j < 98304; ++j)
                {
                    if (scrDebuggerGlob.variableBreakpoints[j])
                        MyAssertHandler(".\\script\\scr_debugger.cpp", 8003, 0, "%s", "!scrDebuggerGlob.variableBreakpoints[j]");
                }
                Hunk_FreeDebugMem(scrDebuggerGlob.variableBreakpoints);
                scrDebuggerGlob.variableBreakpoints = 0;
            }
            while (scrDebuggerGlob.assignHead)
            {
                opcodeElement = scrDebuggerGlob.assignHead->next;
                Hunk_FreeDebugMem(scrDebuggerGlob.assignHead);
                scrDebuggerGlob.assignHead = opcodeElement;
            }
        }
    }
}

void __cdecl Scr_InitDebugger()
{
    if (scrVarPub.developer && scrCompilePub.script_loading)
    {
        if (scrDebuggerGlob.debugger_inited)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 8028, 0, "%s", "!scrDebuggerGlob.debugger_inited");
        if (!Sys_IsRemoteDebugClient())
        {
            scrDebuggerGlob.breakpoints = (char*)Hunk_AllocDebugMem(scrCompilePub.programLen, "scrDebuggerGlob.breakpoints");
            memset((unsigned __int8*)scrDebuggerGlob.breakpoints, 0x7Fu, scrCompilePub.programLen);
        }
        Scr_ScriptList::Init(&scrDebuggerGlob.scriptList);
        Scr_OpenScriptList::Init(&scrDebuggerGlob.openScriptList);
        scrDebuggerGlob.debugger_inited = 1;
        if (cls.uiStarted)
            UI_Component_Init();
    }
}

void __cdecl Scr_ShutdownDebugger()
{
    if (scrVarPub.developer && scrDebuggerGlob.debugger_inited)
    {
        scrDebuggerGlob.debugger_inited = 0;
        Scr_OpenScriptList::Shutdown(&scrDebuggerGlob.openScriptList);
        Scr_ScriptList::Shutdown(&scrDebuggerGlob.scriptList);
        if (!Sys_IsRemoteDebugClient())
        {
            if (scrDebuggerGlob.breakpoints)
            {
                Hunk_FreeDebugMem(scrDebuggerGlob.breakpoints);
                scrDebuggerGlob.breakpoints = 0;
            }
        }
        scrDebuggerGlob.debugger_inited = 0;
    }
}

void __cdecl Scr_InitDebuggerSystem()
{
    if (scrVarPub.developer)
    {
        if (scrDebuggerGlob.debugger_inited_system)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 8088, 0, "%s", "!scrDebuggerGlob.debugger_inited_system");
        Scr_InitBreakpoints();
        if (!Sys_IsRemoteDebugClient())
        {
            scrDebuggerGlob.nextBreakpointCodePos = 0;
            scrDebuggerGlob.killThreadCodePos = 0;
            scrDebuggerGlob.breakpointCount = 0;
            if (scrVarPub.evaluate)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 8103, 0, "%s", "!scrVarPub.evaluate");
            scrVarPub.evaluate = 1;
        }
        scrDebuggerGlob.assignBreakpointSet = 0;
        scrDebuggerGlob.breakpointPos.bufferIndex = -1;
        scrDebuggerGlob.atBreakpoint = 0;
        scrDebuggerGlob.run_debugger = 0;
        Scr_ScriptWatch::Init(&scrDebuggerGlob.scriptWatch);
        scrDebuggerGlob.gainFocusTime = 0;
        Scr_ScriptList::LoadScriptPos(&scrDebuggerGlob.scriptList);
        Scr_ScriptCallStack::Init(&scrDebuggerGlob.scriptCallStack);
        if (Sys_IsRemoteDebugClient())
        {
            scrDebuggerGlob.atBreakpoint = 1;
            scrDebuggerGlob.step_mode = 0;
            Scr_Step();
        }
        else
        {
            if (!scrVarPub.evaluate)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 8128, 0, "%s", "scrVarPub.evaluate");
            scrVarPub.evaluate = 0;
        }
        scrDebuggerGlob.debugger_inited_system = 1;
        if (scrDebuggerGlob.mainWindow.posY == 0.0)
            scrDebuggerGlob.mainWindow.posY = UI_Component::g.screenHeight - 120.0;
        scrDebuggerGlob.miscScrollPane.comp = &scrDebuggerGlob.scriptWatch;
        scrDebuggerGlob.scriptList.selectionParent = &scrDebuggerGlob.miscScrollPane;
        scrDebuggerGlob.openScriptList.selectionParent = &scrDebuggerGlob.miscScrollPane;
        scrDebuggerGlob.scriptWatch.selectionParent = &scrDebuggerGlob.miscScrollPane;
        scrDebuggerGlob.scriptCallStack.selectionParent = &scrDebuggerGlob.miscScrollPane;
        Scr_SetSelectionComp(&scrDebuggerGlob.miscScrollPane);
        if (!Sys_IsRemoteDebugClient())
            Scr_ScriptWatch::UpdateBreakpoints(&scrDebuggerGlob.scriptWatch, 1);
    }
}

void Scr_InitBreakpoints()
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 0x7F; ++i)
        g_breakpoints[i].next = &g_breakpoints[i + 1];
    if (g_breakpoints[127].next)
        MyAssertHandler(
            ".\\script\\scr_debugger.cpp",
            617,
            0,
            "%s",
            "!g_breakpoints[ARRAY_COUNT( g_breakpoints ) - 1].next");
    g_breakpointsHead = g_breakpoints;
}

void __cdecl Scr_ShutdownDebuggerSystem(int restart)
{
    if (scrVarPub.developer)
    {
        if (!restart && Key_IsCatcherActive(0, 2))
        {
            Key_RemoveCatcher(0, -3);
            IN_ActivateMouse(1);
        }
        if (scrDebuggerGlob.debugger_inited_system)
        {
            scrDebuggerGlob.debugger_inited_system = 0;
            scrVarPub.evaluate = 0;
            Scr_ScriptWatch::Shutdown(&scrDebuggerGlob.scriptWatch);
            if (!Sys_IsRemoteDebugClient())
            {
                if (scrDebuggerGlob.nextBreakpointCodePos)
                {
                    Scr_RemoveManualBreakpoint((unsigned __int8*)scrDebuggerGlob.nextBreakpointCodePos);
                    scrDebuggerGlob.nextBreakpointCodePos = 0;
                }
                if (scrDebuggerGlob.killThreadCodePos)
                {
                    Scr_RemoveManualBreakpoint((unsigned __int8*)scrDebuggerGlob.killThreadCodePos);
                    scrDebuggerGlob.killThreadCodePos = 0;
                }
                if (scrDebuggerGlob.breakpointCount)
                    MyAssertHandler(".\\script\\scr_debugger.cpp", 8219, 0, "%s", "!scrDebuggerGlob.breakpointCount");
            }
        }
    }
}

void __cdecl Scr_ShutdownRemoteClient(int restart)
{
    if (!Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 8244, 0, "%s", "Sys_IsRemoteDebugClient()");
    Scr_ShutdownDebuggerSystem(restart);
    Scr_ShutdownDebugger();
    Scr_ShutdownDebuggerMain();
    Scr_ShutdownOpcodeLookup();
}

void __cdecl Scr_InitEvaluate()
{
    scrEvaluateGlob.archivedCanonicalStringsBuf = 0;
    scrEvaluateGlob.archivedCanonicalStrings = 0;
    scrEvaluateGlob.canonicalStringLookup = 0;
    CL_ResetStats_f();
}

void __cdecl Scr_ShutdownEvaluate()
{
    if (scrEvaluateGlob.archivedCanonicalStringsBuf)
    {
        Hunk_FreeDebugMem(scrEvaluateGlob.archivedCanonicalStringsBuf);
        scrEvaluateGlob.archivedCanonicalStringsBuf = 0;
    }
    if (scrEvaluateGlob.archivedCanonicalStrings)
    {
        Hunk_FreeDebugMem(scrEvaluateGlob.archivedCanonicalStrings);
        scrEvaluateGlob.archivedCanonicalStrings = 0;
    }
    if (scrEvaluateGlob.canonicalStringLookup)
    {
        Hunk_FreeDebugMem(scrEvaluateGlob.canonicalStringLookup);
        scrEvaluateGlob.canonicalStringLookup = 0;
    }
    CL_ResetStats_f();
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
            "pos.type == VAR_CODEPOS || pos.type == VAR_DEVELOPER_CODEPOS");
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

void __cdecl Scr_InitOpcodeLookup()
{
    if (scrParserGlob.opcodeLookup)
        MyAssertHandler(".\\script\\scr_parser.cpp", 75, 0, "%s", "!scrParserGlob.opcodeLookup");
    if (scrParserGlob.sourcePosLookup)
        MyAssertHandler(".\\script\\scr_parser.cpp", 76, 0, "%s", "!scrParserGlob.sourcePosLookup");
    if (scrParserPub.sourceBufferLookup)
        MyAssertHandler(".\\script\\scr_parser.cpp", 77, 0, "%s", "!scrParserPub.sourceBufferLookup");
    if (scrVarPub.developer)
    {
        scrParserGlob.delayedSourceIndex = -1;
        scrParserGlob.opcodeLookupMaxLen = 0x40000;
        scrParserGlob.opcodeLookupLen = 0;
        scrParserGlob.opcodeLookup = (OpcodeLookup*)Hunk_AllocDebugMem(6291456, "Scr_InitOpcodeLookup");
        memset((unsigned __int8*)scrParserGlob.opcodeLookup, 0, 24 * scrParserGlob.opcodeLookupMaxLen);
        scrParserGlob.sourcePosLookupMaxLen = 0x40000;
        scrParserGlob.sourcePosLookupLen = 0;
        scrParserGlob.sourcePosLookup = (SourceLookup*)Hunk_AllocDebugMem(0x200000, "Scr_InitOpcodeLookup");
        scrParserGlob.currentCodePos = 0;
        scrParserGlob.currentSourcePosCount = 0;
        scrParserGlob.sourceBufferLookupMaxLen = 256;
        scrParserPub.sourceBufferLookupLen = 0;
        scrParserPub.sourceBufferLookup = (SourceBufferInfo*)Hunk_AllocDebugMem(11264, "Scr_InitOpcodeLookup");
    }
}

void __cdecl Scr_ShutdownOpcodeLookup()
{
    unsigned int i; // [esp+0h] [ebp-4h]
    unsigned int ia; // [esp+0h] [ebp-4h]

    if (scrParserGlob.opcodeLookup)
    {
        Hunk_FreeDebugMem(scrParserGlob.opcodeLookup);
        scrParserGlob.opcodeLookup = 0;
    }
    if (scrParserGlob.sourcePosLookup)
    {
        Hunk_FreeDebugMem(scrParserGlob.sourcePosLookup);
        scrParserGlob.sourcePosLookup = 0;
    }
    if (scrParserPub.sourceBufferLookup)
    {
        for (i = 0; i < scrParserPub.sourceBufferLookupLen; ++i)
            Hunk_FreeDebugMem(scrParserPub.sourceBufferLookup[i].buf);
        Hunk_FreeDebugMem(scrParserPub.sourceBufferLookup);
        scrParserPub.sourceBufferLookup = 0;
    }
    if (scrParserGlob.saveSourceBufferLookup)
    {
        for (ia = 0; ia < scrParserGlob.saveSourceBufferLookupLen; ++ia)
        {
            if (scrParserGlob.saveSourceBufferLookup[ia].sourceBuf)
                Hunk_FreeDebugMem(scrParserGlob.saveSourceBufferLookup[ia].sourceBuf);
        }
        Hunk_FreeDebugMem(scrParserGlob.saveSourceBufferLookup);
        scrParserGlob.saveSourceBufferLookup = 0;
    }
}

unsigned int __cdecl Scr_GetFunctionLineNumInternal(const char* buf, unsigned int sourcePos, const char** startLine)
{
    unsigned int lineNum; // [esp+0h] [ebp-Ch]
    unsigned int functionLine; // [esp+4h] [ebp-8h]
    const char* functionStartLine; // [esp+8h] [ebp-4h]

    if (!buf)
        MyAssertHandler(".\\script\\scr_parser.cpp", 497, 0, "%s", "buf");
    *startLine = buf;
    lineNum = 0;
    functionLine = 0;
    functionStartLine = buf;
    while (sourcePos)
    {
        if (!*buf)
        {
            if (buf[1] == 123)
            {
                functionLine = lineNum;
                functionStartLine = *startLine;
            }
            *startLine = buf + 1;
            ++lineNum;
        }
        ++buf;
        --sourcePos;
    }
    *startLine = functionStartLine;
    return functionLine;
}

unsigned int __cdecl Scr_GetFunctionInfo(const char* buf, unsigned int sourcePos, char* line)
{
    const char* startLine; // [esp+0h] [ebp-8h] BYREF
    unsigned int lineNum; // [esp+4h] [ebp-4h]

    lineNum = Scr_GetFunctionLineNumInternal(buf, sourcePos, &startLine);
    Scr_CopyFormattedLine(line, startLine);
    return lineNum;
}

void __cdecl Scr_InitAllocNode()
{
    if (g_allocNodeUser)
        MyAssertHandler(".\\script\\scr_parsetree.cpp", 66, 0, "%s", "!g_allocNodeUser");
    g_allocNodeUser = Hunk_UserCreate(0x10000, "Scr_InitAllocNode", 0, 1, 7);
}

void __cdecl Scr_ShutdownAllocNode()
{
    if (g_allocNodeUser)
    {
        Hunk_UserDestroy(g_allocNodeUser);
        g_allocNodeUser = 0;
    }
}

void __cdecl Scr_ShutdownGameStrings()
{
    SL_ShutdownSystem(1u);
}

void __cdecl Scr_InitVariables()
{
    if (!scrVarDebugPub)
        scrVarDebugPub = &scrVarDebugPubBuf;
    memset((unsigned __int8*)scrVarDebugPub->leakCount, 0, sizeof(scrVarDebugPub->leakCount));
    scrVarPub.totalObjectRefCount = 0;
    scrVarPub.totalVectorRefCount = 0;
    if (scrVarDebugPub)
        memset((unsigned __int8*)scrVarDebugPub->extRefCount, 0, sizeof(scrVarDebugPub->extRefCount));
    scrVarPub.numScriptValues = 0;
    scrVarPub.numScriptObjects = 0;
    if (scrVarDebugPub)
        memset((unsigned __int8*)scrVarDebugPub, 0, 0x60000u);
    Scr_InitVariableRange(1u, 0x8001u);
    Scr_InitVariableRange(0x8002u, 0x18000u);
}

void __cdecl Scr_InitVariableRange(unsigned int begin, unsigned int end)
{
    unsigned int index; // [esp+4h] [ebp-8h]
    VariableValueInternal* value; // [esp+8h] [ebp-4h]
    VariableValueInternal* valuea; // [esp+8h] [ebp-4h]

    for (index = begin + 1; index < end; ++index)
    {
        value = &scrVarGlob.variableList[index];
        value->w.status = 0;
        if ((value->w.status & 0x1F) != 0)
            MyAssertHandler(".\\script\\scr_variable.cpp", 801, 0, "%s", "!(value->w.type & VAR_MASK)");
        value->hash.id = index - begin;
        value->v.next = index - begin;
        value->u.next = index - begin + 1;
        value->hash.u.prev = index - begin - 1;
    }
    valuea = &scrVarGlob.variableList[begin];
    valuea->w.status = 0;
    if ((valuea->w.status & 0x1F) != 0)
        MyAssertHandler(".\\script\\scr_variable.cpp", 810, 0, "%s", "!(value->w.type & VAR_MASK)");
    valuea->w.status = valuea->w.status;
    valuea->hash.id = 0;
    valuea->v.next = 0;
    valuea->u.next = 1;
    scrVarGlob.variableList[begin + 1].hash.u.prev = 0;
    valuea->hash.u.prev = end - begin - 1;
    scrVarGlob.variableList[end - 1].u.next = 0;
}

void __cdecl Scr_InitClassMap()
{
    int classnum; // [esp+0h] [ebp-4h]

    for (classnum = 0; classnum < 4; ++classnum)
    {
        g_classMap[classnum].entArrayId = 0;
        g_classMap[classnum].id = 0;
    }
}

void __cdecl Scr_ShutdownVariables()
{
    if (scrVarPub.gameId)
    {
        FreeValue(scrVarPub.gameId);
        scrVarPub.gameId = 0;
    }
    if (!scrStringDebugGlob || !scrStringDebugGlob->ignoreLeaks)
    {
        if (scrVarPub.numScriptValues)
            MyAssertHandler(
                ".\\script\\scr_variable.cpp",
                884,
                0,
                "%s\n\t(scrVarPub.numScriptValues) = %i",
                "(!scrVarPub.numScriptValues)",
                scrVarPub.numScriptValues);
        if (scrVarPub.numScriptObjects)
            MyAssertHandler(
                ".\\script\\scr_variable.cpp",
                885,
                0,
                "%s\n\t(scrVarPub.numScriptObjects) = %i",
                "(!scrVarPub.numScriptObjects)",
                scrVarPub.numScriptObjects);
    }
    Scr_CheckLeaks();
}

void __cdecl Scr_AddArrayKeys(unsigned int parentId)
{
    VariableValue ArrayIndexValue; // rax
    VariableValueInternal* entryValue; // [esp+18h] [ebp-8h]
    unsigned int id; // [esp+1Ch] [ebp-4h]

    if (!parentId)
        MyAssertHandler(".\\script\\scr_variable.cpp", 4796, 0, "%s", "parentId");
    if (GetObjectType(parentId) != 21)
        MyAssertHandler(".\\script\\scr_variable.cpp", 4797, 0, "%s", "GetObjectType( parentId ) == VAR_ARRAY");
    Scr_MakeArray();
    for (id = FindFirstSibling(parentId); id; id = FindNextSibling(id))
    {
        entryValue = &scrVarGlob.variableList[id + 32770];
        if ((entryValue->w.status & 0x60) == 0 || (entryValue->w.status & 0x60) == 0x60)
            MyAssertHandler(
                ".\\script\\scr_variable.cpp",
                4805,
                0,
                "%s",
                "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE && (entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_EXTERNAL");
        if (IsObject(entryValue))
            MyAssertHandler(".\\script\\scr_variable.cpp", 4806, 0, "%s", "!IsObject( entryValue )");
        ArrayIndexValue = Scr_GetArrayIndexValue(entryValue->w.status >> 8);
        if (ArrayIndexValue.type == 2)
        {
            Scr_AddConstString(ArrayIndexValue.u.stringValue);
        }
        else if (ArrayIndexValue.type == 6)
        {
            Scr_AddInt(ArrayIndexValue.u.intValue);
        }
        else if (!alwaysfails)
        {
            MyAssertHandler(".\\script\\scr_variable.cpp", 4822, 1, "bad case");
        }
        Scr_AddArray();
    }
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
    scrVmPub.stack[0].type = 7;
    scrVmGlob.loading = 0;
    scrVmGlob.recordPlace = 0;
    scrVmGlob.lastFileName = 0;
    scrVmGlob.lastLine = 0;
    scrVarPub.ext_threadcount = 0;
    scrVarPub.numScriptThreads = 0;
    scrVarPub.varUsagePos = 0;
    result = Dvar_RegisterBool("logScriptTimes", 0, 0, "Log times for every print called from script");
    logScriptTimes = result;
    return result;
}

void __cdecl Scr_Settings(int developer, int developer_script, int abort_on_error)
{
    if (abort_on_error && !developer)
        MyAssertHandler(".\\script\\scr_vm.cpp", 188, 0, "%s", "!abort_on_error || developer");
    scrVarPub.developer = developer != 0;
    scrVarPub.developer_script = developer_script != 0;
    scrVmPub.abort_on_error = abort_on_error != 0;
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
        if (scrVarPub.error_message)
            MyAssertHandler(".\\script\\scr_vm.cpp", 2393, 0, "%s", "!scrVarPub.error_message");
        if (!top)
            MyAssertHandler(".\\script\\scr_vm.cpp", 2394, 0, "%s", "top");
        if (!pos)
            MyAssertHandler(".\\script\\scr_vm.cpp", 2395, 0, "%s", "pos");
        if (!scrVarPub.evaluate)
            MyAssertHandler(".\\script\\scr_vm.cpp", 2396, 0, "%s", "scrVarPub.evaluate");
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
                value.type = type;
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
                value.type = v7;
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
    notifyListId = FindObject(Variable).u.stringValue;
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

    if (stringValue != (unsigned __int16)Scr_GetThreadNotifyName(startLocalId))
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
            if (!u.intValue)
                MyAssertHandler(".\\script\\scr_vm.cpp", 3039, 0, "%s", "u.codePosValue");
            pos = (const char*)u.intValue;
        }
    }
    if (killThread)
        Scr_DebugKillThread(localId, pos);
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

    Profile_Begin(331);
    SL_CheckExists(stringValue);
    varUsagePos = scrVarPub.varUsagePos;
    if (!scrVarPub.varUsagePos)
        scrVarPub.varUsagePos = "<script notify variable>";
    if (!scrVarPub.timeArrayId)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3670, 0, "%s", "scrVarPub.timeArrayId");
    if (paramcount > scrVmPub.inparamcount)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3671, 0, "%s", "paramcount <= scrVmPub.inparamcount");
    Scr_ClearOutParams();
    startTop = &scrVmPub.top[-paramcount];
    paramcounta = scrVmPub.inparamcount - paramcount;
    id = FindEntityId(entnum, classnum).u.stringValue;
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
        startTop->type = 8;
        scrVmPub.inparamcount = 0;
        VM_Notify(id, stringValue, scrVmPub.top);
        startTop->type = type;
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
    Profile_EndInternal(0);
}

void __cdecl VM_Notify(unsigned int notifyListOwnerId, unsigned int stringValue, VariableValue* top)
{
    unsigned int ObjectVariable; // eax
    int type; // edx
    unsigned int Variable; // eax
    VariableValueInternal_u Array; // eax
    unsigned int v7; // [esp-4h] [ebp-64h]
    VariableUnion stackValue; // [esp+0h] [ebp-60h]
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
    VariableUnion* tempValue; // [esp+54h] [ebp-Ch]
    unsigned int selfId; // [esp+58h] [ebp-8h]
    unsigned int notifyListEntry; // [esp+5Ch] [ebp-4h]

    notifyListId = FindVariable(notifyListOwnerId, 0x18000u);
    if (notifyListId)
    {
        notifyListId = FindObject(notifyListId).u.stringValue;
        if (!notifyListId)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3446, 0, "%s", "notifyListId");
        notifyNameListId = FindVariable(notifyListId, stringValue);
        if (notifyNameListId)
        {
            notifyNameListId = FindObject(notifyNameListId).u.stringValue;
            if (!notifyNameListId)
                MyAssertHandler(".\\script\\scr_vm.cpp", 3453, 0, "%s", "notifyNameListId");
            AddRefToObject(notifyNameListId);
            if (scrVarPub.evaluate)
                MyAssertHandler(".\\script\\scr_vm.cpp", 3457, 0, "%s", "!scrVarPub.evaluate");
            scrVarPub.evaluate = 1;
            notifyListEntry = notifyNameListId;
            while (1)
            {
                notifyListIndex = FindLastSibling(notifyListEntry);
            next:
                if (!notifyListIndex)
                    break;
                notifyListEntry = Scr_GetVarId(notifyListIndex);
                if (!notifyListEntry)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 3471, 0, "%s", "notifyListEntry");
                startLocalId = GetVariableKeyObject(notifyListEntry);
                selfId = Scr_GetSelf(startLocalId);
                ObjectVariable = FindObjectVariable(scrVarPub.pauseArrayId, selfId);
                selfNameId = FindObject(ObjectVariable).u.stringValue;
                if (GetValueType(notifyListEntry))
                {
                    if (GetValueType(notifyListEntry) != 10)
                        MyAssertHandler(".\\script\\scr_vm.cpp", 3496, 0, "%s", "GetValueType( notifyListEntry ) == VAR_STACK");
                    tempValue = (VariableUnion*)GetVariableValueAddress(notifyListEntry);
                    stackValue.intValue = tempValue->intValue;
                    if (*(_BYTE*)(*(_DWORD*)tempValue->intValue - 1) == 119)
                    {
                        size = **(char**)stackValue.intValue;
                        if (size < 0)
                            MyAssertHandler(".\\script\\scr_vm.cpp", 3504, 0, "%s", "size >= 0");
                        if (size > *(unsigned __int16*)(stackValue.intValue + 4))
                            MyAssertHandler(
                                ".\\script\\scr_vm.cpp",
                                3505,
                                0,
                                "size <= stackValue->size\n\t%i, %i",
                                size,
                                *(unsigned __int16*)(stackValue.intValue + 4));
                        buf = (char*)(stackValue.intValue + 5 * (*(unsigned __int16*)(stackValue.intValue + 4) - size) + 11);
                        for (currentValue = top; size; --currentValue)
                        {
                            if (currentValue->type == 7)
                                MyAssertHandler(".\\script\\scr_vm.cpp", 3512, 0, "%s", "currentValue->type != VAR_CODEPOS");
                            if (currentValue->type == 8)
                            {
                            LABEL_30:
                                notifyListIndex = FindPrevSibling(notifyListIndex);
                                goto next;
                            }
                            --size;
                            tempValue3.type = (unsigned __int8)*buf;
                            if (tempValue3.type == 7)
                                MyAssertHandler(".\\script\\scr_vm.cpp", 3521, 0, "%s", "tempValue3.type != VAR_CODEPOS");
                            if (tempValue3.type == 8)
                                break;
                            tempValue3.u.intValue = *(_DWORD*)++buf;
                            buf += 4;
                            AddRefToValue(tempValue3.type, tempValue3.u);
                            type = currentValue->type;
                            tempValue2.u.intValue = currentValue->u.intValue;
                            tempValue2.type = type;
                            AddRefToValue(type, tempValue2.u);
                            Scr_EvalEquality(&tempValue3, &tempValue2);
                            if (scrVarPub.error_message)
                            {
                                RuntimeError(
                                    *(const char**)stackValue.intValue,
                                    **(char**)stackValue.intValue - size + 3,
                                    scrVarPub.error_message,
                                    scrVmGlob.dialog_error_message);
                                Scr_ClearErrorMessage();
                                notifyListIndex = FindPrevSibling(notifyListIndex);
                                goto next;
                            }
                            if (tempValue3.type != 6)
                                MyAssertHandler(".\\script\\scr_vm.cpp", 3537, 0, "%s", "tempValue3.type == VAR_INTEGER");
                            if (!tempValue3.u.intValue)
                                goto LABEL_30;
                        }
                        ++ * (_DWORD*)stackValue.intValue;
                        bNoStack = 1;
                    }
                    else
                    {
                        if (top->type == 7)
                            MyAssertHandler(".\\script\\scr_vm.cpp", 3559, 0, "%s", "top->type != VAR_CODEPOS");
                        bNoStack = top->type == 8;
                    }
                    tempValue5.type = 10;
                    tempValue5.u = stackValue;
                    v7 = startLocalId;
                    Variable = GetVariable(scrVarPub.timeArrayId, scrVarPub.time);
                    Array = GetArray(Variable);
                    stackId = GetNewObjectVariable(Array.u.stringValue, v7);
                    SetNewVariableValue(stackId, &tempValue5);
                    tempValue = (VariableUnion*)GetVariableValueAddress(stackId);
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
                        if (top->type == 8)
                            MyAssertHandler(
                                ".\\script\\scr_vm.cpp",
                                3585,
                                0,
                                "%s\n\t(top - scrVmPub.stack) = %i",
                                "(top->type != VAR_PRECODEPOS)",
                                top - scrVmPub.stack);
                        if (top->type == 7)
                            MyAssertHandler(
                                ".\\script\\scr_vm.cpp",
                                3586,
                                0,
                                "%s\n\t(top - scrVmPub.stack) = %i",
                                "(top->type != VAR_CODEPOS)",
                                top - scrVmPub.stack);
                        size = *(unsigned __int16*)(stackValue.intValue + 4);
                        newSize = size;
                        currentValue = top;
                        do
                        {
                            ++newSize;
                            --currentValue;
                            if (currentValue->type == 7)
                                MyAssertHandler(
                                    ".\\script\\scr_vm.cpp",
                                    3596,
                                    0,
                                    "%s\n\t(currentValue - scrVmPub.stack) = %i",
                                    "(currentValue->type != VAR_CODEPOS)",
                                    currentValue - scrVmPub.stack);
                        } while (currentValue->type != 8);
                        if ((unsigned int)newSize >= 0x10000)
                            MyAssertHandler(".\\script\\scr_vm.cpp", 3599, 0, "%s", "newSize >= 0 && newSize < (1 << 16)");
                        len = 5 * size;
                        bufLen = 5 * newSize + 11;
                        if (!MT_Realloc(*(unsigned __int16*)(stackValue.intValue + 6), bufLen))
                        {
                            newStackValue = (VariableStackBuffer*)MT_Alloc(bufLen, 1);
                            newStackValue->bufLen = bufLen;
                            newStackValue->pos = *(const char**)stackValue.intValue;
                            newStackValue->localId = *(_WORD*)(stackValue.intValue + 8);
                            memcpy((unsigned __int8*)newStackValue->buf, (unsigned __int8*)(stackValue.intValue + 11), len);
                            MT_Free((VariableUnion)stackValue.intValue, *(unsigned __int16*)(stackValue.intValue + 6));
                            stackValue.intValue = (int)newStackValue;
                            tempValue->intValue = (int)newStackValue;
                        }
                        *(_WORD*)(stackValue.intValue + 4) = newSize;
                        buf = (char*)(stackValue.intValue + len + 11);
                        newSize -= size;
                        if (!newSize)
                            MyAssertHandler(".\\script\\scr_vm.cpp", 3624, 0, "%s", "newSize");
                        do
                        {
                            ++currentValue;
                            AddRefToValue(currentValue->type, currentValue->u);
                            if (currentValue->type >= 0x17u)
                                MyAssertHandler(".\\script\\scr_vm.cpp", 3630, 0, "%s", "(unsigned)currentValue->type < VAR_COUNT");
                            *buf++ = currentValue->type;
                            *(_DWORD*)buf = currentValue->u.intValue;
                            buf += 4;
                            --newSize;
                        } while (newSize);
                        if (&buf[-stackValue.intValue] != (char*)bufLen)
                            MyAssertHandler(".\\script\\scr_vm.cpp", 3638, 0, "%s", "buf - (const char *)stackValue == bufLen");
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
            if (!scrVarPub.evaluate)
                MyAssertHandler(".\\script\\scr_vm.cpp", 3644, 0, "%s", "scrVarPub.evaluate");
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

    if (!scrVmPub.function_count)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3139, 0, "%s", "scrVmPub.function_count");
    function_count = scrVmPub.function_count;
    topThread = scrVmPub.function_count;
    while (1)
    {
        if (!function_count)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3144, 0, "%s", "function_count");
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
            Scr_DebugTerminateThread(topThread);
        else
            scrVmPub.stack[3 * topThread - 96].u.intValue = (int)&g_EndPos;
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
    id = FindObject(Variable).u.stringValue;
    stackId = FindObjectVariable(id, startLocalId);
    if (!stackId)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3208, 0, "%s", "stackId");
    if (GetValueType(stackId) != 10)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3209, 0, "%s", "GetValueType( stackId ) == VAR_STACK");
    stackValue = (VariableStackBuffer*)GetVariableValueAddress(stackId)->u.intValue;
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
    const char* buf; // [esp+8h] [ebp-1Ch]
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
        buf = bufa - 1;
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
                tempValue.type = 10;
                tempValue.u.intValue = (int)stackValue;
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
    MT_Free(stackValue, stackValue->bufLen);
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
        selfNameId = FindObject(ObjectVariable).u.stringValue;
        v4 = FindObjectVariable(selfNameId, startLocalId);
        notifyListOwnerId.intValue = (int)GetVariableValueAddress(v4)->u;
        Variable = FindVariable(notifyListOwnerId.stringValue, 0x18000u);
        notifyListId = FindObject(Variable).u.stringValue;
        v6 = FindVariable(notifyListId, stringValue);
        notifyNameListId = FindObject(v6).u.stringValue;
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
    VariableStackBuffer* stackValuea; // [esp+0h] [ebp-1Ch]
    VariableStackBuffer* stackValue; // [esp+0h] [ebp-1Ch]
    unsigned int stackId; // [esp+4h] [ebp-18h]
    unsigned int stackIda; // [esp+4h] [ebp-18h]
    unsigned int startLocalId; // [esp+8h] [ebp-14h]
    unsigned int selfStartLocalId; // [esp+Ch] [ebp-10h]
    unsigned int notifyListId; // [esp+10h] [ebp-Ch]
    unsigned int notifyListIda; // [esp+10h] [ebp-Ch]
    unsigned int notifyNameListId; // [esp+14h] [ebp-8h]
    unsigned int notifyNameListIda; // [esp+14h] [ebp-8h]
    unsigned int selfLocalId; // [esp+18h] [ebp-4h]

    while (1)
    {
        notifyListId = FindVariable(notifyListOwnerId, 0x18000u);
        if (!notifyListId)
            break;
        notifyListIda = FindObject(notifyListId).u.stringValue;
        if (!notifyListIda)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3739, 0, "%s", "notifyListId");
        notifyNameListId = FindFirstSibling(notifyListIda);
        if (!notifyNameListId)
            break;
        notifyNameListIda = FindObject(notifyNameListId).u.stringValue;
        if (!notifyNameListIda)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3746, 0, "%s", "notifyNameListId");
        stackId = FindFirstSibling(notifyNameListIda);
        if (!stackId)
            break;
        startLocalId = GetVariableKeyObject(stackId);
        if (!startLocalId)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3753, 0, "%s", "startLocalId");
        if (GetValueType(stackId) == 10)
        {
            stackValuea = (VariableStackBuffer*)GetVariableValueAddress(stackId)->u.intValue;
            Scr_CancelWaittill(startLocalId);
            VM_TrimStack(startLocalId, stackValuea, 0);
        }
        else
        {
            AddRefToObject(startLocalId);
            Scr_CancelWaittill(startLocalId);
            selfLocalId = Scr_GetSelf(startLocalId);
            selfStartLocalId = GetStartLocalId(selfLocalId);
            stackIda = FindVariable(selfStartLocalId, 0x18001u);
            if (stackIda)
            {
                if ((unsigned __int16)Scr_GetThreadNotifyName(selfStartLocalId))
                    MyAssertHandler(".\\script\\scr_vm.cpp", 3772, 0, "%s", "!Scr_GetThreadNotifyName( selfStartLocalId )");
                if (GetValueType(stackIda) != 10)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 3773, 0, "%s", "GetValueType( stackId ) == VAR_STACK");
                VariableValueAddress = GetVariableValueAddress(stackIda);
                stackValue = (VariableStackBuffer*)VariableValueAddress->u.intValue;
                if (*(_DWORD*)VariableValueAddress->u.intValue)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 3775, 0, "%s", "!stackValue->pos");
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

    if (!startLocalId)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3065, 0, "%s", "startLocalId");
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
                if (startLocalId == localId)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 3089, 0, "%s", "startLocalId != localId");
                stackValue->localId = localId;
                stackValue->size = size + 1;
                Scr_StopThread(localId);
                if (!fromEndon)
                {
                    Scr_SetThreadNotifyName(startLocalId, 0);
                    stackValue->pos = 0;
                    tempValue.type = 10;
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
    if (startLocalId != localId)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3118, 0, "%s", "startLocalId == localId");
    if (fromEndon)
        RemoveVariable(startLocalId, 0x18001u);
    Scr_KillThread(startLocalId);
    RemoveRefToObject(startLocalId);
    --scrVarPub.numScriptThreads;
    MT_Free(stackValue, stackValue->bufLen);
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

unsigned __int16 __cdecl Scr_ExecThread(int handle, unsigned int paramcount)
{
    unsigned int v2; // eax
    const char* pos; // [esp+34h] [ebp-Ch]
    const char* varUsagePos; // [esp+38h] [ebp-8h]
    unsigned int id; // [esp+3Ch] [ebp-4h]

    pos = &scrVarPub.programBuffer[handle];
    if (!scrVmPub.function_count)
    {
        if ((int*)scrVmPub.localVars != &scrVmGlob.starttime)
            MyAssertHandler(".\\script\\scr_vm.cpp", 4024, 0, "%s", "scrVmPub.localVars == scrVmGlob.localVarsStack - 1");
        Profile_Begin(332);
        Scr_ResetTimeout();
    }
    if (!scrVarPub.timeArrayId)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4030, 0, "%s", "scrVarPub.timeArrayId");
    if (!handle)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4031, 0, "%s", "handle");
    ++scrVarPub.ext_threadcount;
    Scr_IsInOpcodeMemory(pos);
    varUsagePos = scrVarPub.varUsagePos;
    scrVarPub.varUsagePos = pos + 1;
    AddRefToObject(scrVarPub.levelId);
    v2 = AllocThread((ObjectInfo_u)scrVarPub.levelId);
    id = VM_Execute(v2, pos, paramcount);
    scrVarPub.varUsagePos = varUsagePos;
    if (scrVarDebugPub)
        ++scrVarDebugPub->extRefCount[id];
    RemoveRefToValue(scrVmPub.top->type, scrVmPub.top->u);
    scrVmPub.top->type = 0;
    --scrVmPub.top;
    --scrVmPub.inparamcount;
    if (!scrVmPub.function_count)
    {
        Profile_EndInternal(0);
        if ((int*)scrVmPub.localVars != &scrVmGlob.starttime)
            MyAssertHandler(".\\script\\scr_vm.cpp", 4065, 0, "%s", "scrVmPub.localVars == scrVmGlob.localVarsStack - 1");
    }
    return id;
}

unsigned int __cdecl VM_Execute(unsigned int localId, const char* pos, unsigned int paramcount)
{
    int time; // [esp+14h] [ebp-24h]
    function_stack_t fs_backup; // [esp+18h] [ebp-20h]
    VariableValue* startTop; // [esp+2Ch] [ebp-Ch]
    int type; // [esp+30h] [ebp-8h]
    int thread_count_backup; // [esp+34h] [ebp-4h]
    unsigned int localIda; // [esp+40h] [ebp+8h]
    unsigned int paramcounta; // [esp+48h] [ebp+10h]

    if (paramcount > scrVmPub.inparamcount)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3915, 0, "%s", "paramcount <= scrVmPub.inparamcount");
    Scr_ClearOutParams();
    startTop = &scrVmPub.top[-paramcount];
    paramcounta = scrVmPub.inparamcount - paramcount;
    if (scrVmPub.function_count >= 30)
    {
        Scr_KillThread(localId);
        scrVmPub.inparamcount = paramcounta + 1;
        if (scrVmPub.outparamcount)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3993, 0, "%s", "!scrVmPub.outparamcount");
        while (paramcounta)
        {
            RemoveRefToValue(scrVmPub.top->type, scrVmPub.top->u);
            --scrVmPub.top;
            --paramcounta;
        }
        ++scrVmPub.top;
        scrVmPub.top->type = 0;
        RuntimeError(pos, 0, "script stack overflow (too many embedded function calls)", 0);
        return localId;
    }
    else
    {
        fs_backup = ::pos;
        thread_count_backup = thread_count;
        ::pos.localId = localId;
        ::pos.startTop = startTop;
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
        startTop->type = 8;
        scrVmPub.inparamcount = 0;
        ::pos.top = scrVmPub.top;
        ::pos.pos = pos;
        ::pos.localVarCount = 0;
        thread_count = 0;
        scrVmDebugPub.profileEnable[localId] = 0;
        if (scrVarPub.bScriptProfile)
        {
            scrVmDebugPub.builtInTime = 0;
            time = __rdtsc();
            localIda = VM_Execute_0();
            if (!scrVmPub.function_count)
                Scr_AddProfileTime(pos, __rdtsc() - time, scrVmDebugPub.builtInTime);
        }
        else
        {
            localIda = VM_Execute_0();
        }
        ::pos = fs_backup;
        thread_count = thread_count_backup;
        startTop->type = type;
        scrVmPub.top = startTop + 1;
        scrVmPub.inparamcount = paramcounta + 1;
        if (scrVmPub.outparamcount)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3978, 0, "%s", "!scrVmPub.outparamcount");
        ClearVariableValue(scrVarPub.tempVariable);
        if (scrVmPub.function_count)
        {
            --scrVmPub.function_count;
            --scrVmPub.function_frame;
        }
        return localIda;
    }
}

unsigned int __cdecl VM_Execute_0()
{
    int v0; // edx
    const char* v1; // eax
    const char* v2; // eax
    unsigned int Variable; // eax
    unsigned int Variable; // eax
    unsigned int v5; // eax
    const char* v6; // eax
    const char* v7; // eax
    const char* v8; // eax
    unsigned int v9; // eax
    unsigned int v10; // eax
    const char* v11; // eax
    const char* v12; // eax
    const char* v13; // eax
    const char* v14; // eax
    DWORD v15; // eax
    const char* v16; // eax
    const char* v17; // eax
    unsigned int v18; // eax
    VariableValueInternal_u Array; // eax
    unsigned int v20; // eax
    unsigned int Self; // eax
    unsigned int ObjectVariable; // eax
    VariableValueInternal_u v23; // eax
    unsigned int NewObjectVariable; // eax
    unsigned int v25; // eax
    VariableValueInternal_u v26; // eax
    unsigned int v27; // eax
    VariableValueInternal_u v28; // eax
    unsigned int v29; // eax
    VariableValueInternal_u v30; // eax
    unsigned int v31; // eax
    const char* v32; // eax
    const char* v33; // eax
    unsigned int v35; // eax
    const char* v36; // eax
    const char* v37; // eax
    unsigned int localId; // [esp-8h] [ebp-390h]
    unsigned int v39; // [esp-8h] [ebp-390h]
    unsigned int v40; // [esp-8h] [ebp-390h]
    VariableValue* top; // [esp-4h] [ebp-38Ch]
    VariableValue* v42; // [esp-4h] [ebp-38Ch]
    unsigned int v43; // [esp-4h] [ebp-38Ch]
    unsigned int v44; // [esp-4h] [ebp-38Ch]
    int v45; // [esp+0h] [ebp-388h]
    int v46; // [esp+4h] [ebp-384h]
    int v47; // [esp+8h] [ebp-380h]
    const char* v48; // [esp+10h] [ebp-378h]
    unsigned int v49; // [esp+18h] [ebp-370h]
    unsigned int v50; // [esp+28h] [ebp-360h]
    unsigned __int16 v51; // [esp+54h] [ebp-334h]
    unsigned __int16 v52; // [esp+80h] [ebp-308h]
    unsigned __int16 v53; // [esp+84h] [ebp-304h]
    unsigned __int16 v54; // [esp+88h] [ebp-300h]
    unsigned int v55; // [esp+8Ch] [ebp-2FCh]
    unsigned int v56; // [esp+90h] [ebp-2F8h]
    unsigned __int16 v57; // [esp+98h] [ebp-2F0h]
    const char* v58; // [esp+9Ch] [ebp-2ECh]
    unsigned int v59; // [esp+A4h] [ebp-2E4h]
    unsigned __int16 v60; // [esp+A8h] [ebp-2E0h]
    int v61; // [esp+ACh] [ebp-2DCh]
    unsigned __int16 v62; // [esp+B4h] [ebp-2D4h]
    unsigned __int16 v63; // [esp+B8h] [ebp-2D0h]
    int v64; // [esp+BCh] [ebp-2CCh]
    unsigned __int16 v65; // [esp+C0h] [ebp-2C8h]
    unsigned __int16 v66; // [esp+C4h] [ebp-2C4h]
    unsigned __int16 v67; // [esp+C8h] [ebp-2C0h]
    unsigned __int16 v68; // [esp+CCh] [ebp-2BCh]
    int v69; // [esp+E0h] [ebp-2A8h]
    int v70; // [esp+E8h] [ebp-2A0h]
    const char* v71; // [esp+F0h] [ebp-298h]
    int v72; // [esp+FCh] [ebp-28Ch]
    int v73; // [esp+104h] [ebp-284h]
    const char* v74; // [esp+10Ch] [ebp-27Ch]
    const char* v75; // [esp+11Ch] [ebp-26Ch]
    const char* v76; // [esp+128h] [ebp-260h]
    float v77; // [esp+130h] [ebp-258h]
    unsigned __int16 v78; // [esp+154h] [ebp-234h]
    unsigned __int16 v79; // [esp+16Ch] [ebp-21Ch]
    unsigned __int16 v80; // [esp+174h] [ebp-214h]
    unsigned __int16 v81; // [esp+178h] [ebp-210h]
    unsigned __int16 v82; // [esp+17Ch] [ebp-20Ch]
    unsigned __int16 v83; // [esp+194h] [ebp-1F4h]
    unsigned __int16 v84; // [esp+198h] [ebp-1F0h]
    unsigned __int16 v85; // [esp+19Ch] [ebp-1ECh]
    unsigned __int16 v86; // [esp+1A0h] [ebp-1E8h]
    unsigned __int16 v87; // [esp+1A8h] [ebp-1E0h]
    unsigned __int16 v88; // [esp+1CCh] [ebp-1BCh]
    VariableUnion v89; // [esp+1D0h] [ebp-1B8h]
    VariableUnion v90; // [esp+1D8h] [ebp-1B0h]
    const char* v91; // [esp+1DCh] [ebp-1ACh]
    unsigned __int16 v92; // [esp+1E0h] [ebp-1A8h]
    unsigned __int16 v93; // [esp+1E4h] [ebp-1A4h]
    VariableUnion v94; // [esp+1E8h] [ebp-1A0h]
    VariableUnion v95; // [esp+1ECh] [ebp-19Ch]
    unsigned __int16 v96; // [esp+1F0h] [ebp-198h]
    unsigned __int16 v97; // [esp+1F4h] [ebp-194h]
    unsigned int v98; // [esp+274h] [ebp-114h]
    unsigned int outparamcount; // [esp+278h] [ebp-110h]
    scr_entref_t entref; // [esp+27Ch] [ebp-10Ch]
    VariableValue stackValue; // [esp+284h] [ebp-104h] BYREF
    unsigned int builtinIndex; // [esp+28Ch] [ebp-FCh]
    const char* pos; // [esp+290h] [ebp-F8h]
    int* profileEnablePos; // [esp+294h] [ebp-F4h]
    unsigned int stringValue; // [esp+298h] [ebp-F0h]
    const char* currentCodePos; // [esp+29Ch] [ebp-ECh]
    unsigned int profileEnable[33]; // [esp+2A0h] [ebp-E8h] BYREF
    const char* tempCodePos; // [esp+324h] [ebp-64h]
    unsigned int stackId; // [esp+328h] [ebp-60h]
    unsigned int fieldValueId; // [esp+32Ch] [ebp-5Ch]
    int builtInTime; // [esp+330h] [ebp-58h]
    int waitTime; // [esp+334h] [ebp-54h]
    int jumpOffset; // [esp+338h] [ebp-50h]
    unsigned int objectId; // [esp+33Ch] [ebp-4Ch]
    unsigned int caseValue; // [esp+340h] [ebp-48h]
    unsigned int fieldName; // [esp+344h] [ebp-44h]
    unsigned int threadId; // [esp+348h] [ebp-40h]
    unsigned int currentCaseValue; // [esp+34Ch] [ebp-3Ch]
    int profileBit; // [esp+350h] [ebp-38h]
    int time; // [esp+354h] [ebp-34h]
    unsigned int parentLocalId; // [esp+358h] [ebp-30h]
    unsigned int paramcount; // [esp+35Ch] [ebp-2Ch]
    int type; // [esp+360h] [ebp-28h]
    int entnum; // [esp+364h] [ebp-24h]
    int timeSpent; // [esp+368h] [ebp-20h]
    unsigned int fieldValueIndex; // [esp+36Ch] [ebp-1Ch]
    unsigned int selfId; // [esp+370h] [ebp-18h]
    VariableValue tempValue; // [esp+374h] [ebp-14h] BYREF
    unsigned int id; // [esp+37Ch] [ebp-Ch]
    unsigned __int8 removeCount; // [esp+383h] [ebp-5h]
    int profileIndex; // [esp+384h] [ebp-4h]

    profileEnable[0] = scrVmDebugPub.profileEnable[::pos.localId];
    if (profileEnable[0])
        Profile_BeginScripts(profileEnable[0]);
    profileEnablePos = (int*)profileEnable;
    ++g_script_error_level;
    while (1)
    {
        if ((unsigned int)g_script_error_level >= 0x21)
            MyAssertHandler(
                ".\\script\\scr_vm.cpp",
                396,
                0,
                "g_script_error_level doesn't index ARRAY_COUNT( g_script_error )\n\t%i not in [0, %i)",
                g_script_error_level,
                33);
        if (!_setjmp3(g_script_error[g_script_error_level], 0))
            break;
    error_1:
        Com_PrintMessage(6, "caught script exception\n", 0);
        v46 = opcode - 33;
        switch (opcode)
        {
        case 33:
        case 34:
        case 35:
        case 36:
        case 132:
            if (scrVarPub.error_index < -1)
                MyAssertHandler(".\\script\\scr_vm.cpp", 2058, 0, "%s", "scrVarPub.error_index >= -1");
            if (scrVarPub.error_index < 0)
                scrVarPub.error_index = 1;
            break;
        case 41:
        case 42:
        case 47:
        case 57:
        case 59:
        case 100:
        case 101:
            scrVarPub.error_index = 0;
            break;
        case 62:
        case 63:
        case 64:
        case 65:
        case 66:
        case 67:
        case 68:
            if (scrVarPub.error_index < 0)
                MyAssertHandler(".\\script\\scr_vm.cpp", 2080, 0, "%s", "scrVarPub.error_index >= 0");
            if (scrVarPub.error_index > 0)
                scrVarPub.error_index = scrVmPub.outparamcount + 1 - scrVarPub.error_index;
            break;
        case 69:
        case 70:
        case 71:
        case 72:
        case 73:
        case 74:
        case 75:
            if (scrVarPub.error_index < -1)
                MyAssertHandler(".\\script\\scr_vm.cpp", 2094, 0, "%s", "scrVarPub.error_index >= -1");
            if (scrVarPub.error_index <= 0)
            {
                if (scrVarPub.error_index < 0)
                    scrVarPub.error_index = 1;
            }
            else
            {
                scrVarPub.error_index = scrVmPub.outparamcount + 2 - scrVarPub.error_index;
            }
            break;
        default:
            break;
        }
        RuntimeError(::pos.pos, scrVarPub.error_index, scrVarPub.error_message, scrVmGlob.dialog_error_message);
        Scr_ClearErrorMessage();
        v45 = opcode - 31;
        switch (opcode)
        {
        case 31:
        case 32:
            RemoveRefToValue(::pos.top->type, ::pos.top->u);
            --::pos.top;
            RemoveRefToValue(::pos.top->type, ::pos.top->u);
            ::pos.top->type = 0;
            break;
        case 33:
        case 34:
        case 35:
        case 132:
            fieldValueIndex = 0;
            fieldValueId = GetDummyFieldValue();
            goto $LN43_3;
        case 36:
        case 76:
            goto $LN43_3;
        case 38:
            goto $LN64_0;
        case 41:
        case 42:
            ::pos.top->type = 0;
            break;
        case 45:
        case 46:
            fieldValueIndex = 0;
            fieldValueId = GetDummyFieldValue();
            break;
        case 47:
            if (scrVmPub.outparamcount)
            {
                if (scrVmPub.outparamcount != 1)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 2146, 0, "%s", "scrVmPub.outparamcount == 1");
                if (scrVmPub.top->type)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 2148, 0, "%s", "scrVmPub.top->type == VAR_UNDEFINED");
                scrVmPub.outparamcount = 0;
            }
            break;
        case 53:
            if (::pos.top->type == 7)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    2238,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top->type != VAR_CODEPOS)",
                    ::pos.top - scrVmPub.stack);
            while (::pos.top->type != 8)
            {
                RemoveRefToValue(::pos.top->type, ::pos.top->u);
                --::pos.top;
                if (::pos.top->type == 7)
                    MyAssertHandler(
                        ".\\script\\scr_vm.cpp",
                        2243,
                        0,
                        "%s\n\t(fs.top - scrVmPub.stack) = %i",
                        "(fs.top->type != VAR_CODEPOS)",
                        ::pos.top - scrVmPub.stack);
            }
            ::pos.top->type = 7;
            break;
        case 57:
            if (scrVmPub.outparamcount)
            {
                if (scrVmPub.outparamcount != 1)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 2161, 0, "%s", "scrVmPub.outparamcount == 1");
                if (scrVmPub.top != ::pos.top)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 2163, 0, "%s", "scrVmPub.top == fs.top");
                RemoveRefToValue(::pos.top->type, ::pos.top->u);
                scrVmPub.outparamcount = 0;
            }
            goto error_dec_top;
        case 59:
            RemoveRefToValue(::pos.top->type, ::pos.top->u);
            scrVmPub.outparamcount = 0;
            goto error_dec_top;
        case 62:
        case 63:
        case 64:
        case 65:
        case 66:
        case 67:
        case 68:
        case 69:
        case 70:
        case 71:
        case 72:
        case 73:
        case 74:
        case 75:
            Scr_ClearOutParams();
            ::pos.top = scrVmPub.top + 1;
            scrVmPub.top[1].type = 0;
            break;
        case 79:
        case 80:
        case 82:
            ::pos.pos += 4;
            goto $LN931;
        case 81:
        case 83:
        $LN931:
            if (::pos.top->type == 7)
                MyAssertHandler(".\\script\\scr_vm.cpp", 2256, 0, "%s", "fs.top->type != VAR_CODEPOS");
            while (::pos.top->type != 8)
            {
                RemoveRefToValue(::pos.top->type, ::pos.top->u);
                --::pos.top;
                if (::pos.top->type == 7)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 2261, 0, "%s", "fs.top->type != VAR_CODEPOS");
            }
            ::pos.top->type = 0;
            break;
        case 84:
        case 86:
            ::pos.pos += 4;
            goto $LN937;
        case 85:
        case 87:
        $LN937:
            v50 = *(_DWORD*)::pos.pos;
            ::pos.pos += 4;
            for (paramcount = v50; paramcount; --paramcount)
            {
                RemoveRefToValue(::pos.top->type, ::pos.top->u);
                --::pos.top;
            }
            ++::pos.top;
            ::pos.top->type = 0;
            break;
        case 89:
            objectId = GetDummyObject();
            goto error_dec_top;
        case 90:
            ++::pos.pos;
        $LN64_0:
            objectId = GetDummyObject();
            break;
        case 94:
        case 95:
        case 96:
        case 97:
            ::pos.pos += 2;
            goto error_dec_top;
        case 99:
            v51 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            jumpOffset = v51;
            ::pos.pos -= v51;
            break;
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
            goto error_dec_top;
        case 119:
            ++::pos.pos;
            goto $LN44_2;
        case 120:
        case 122:
        $LN44_2:
            RemoveRefToValue(::pos.top->type, ::pos.top->u);
            --::pos.top;
            goto $LN43_3;
        case 121:
            if (::pos.top->type == 7)
                MyAssertHandler(".\\script\\scr_vm.cpp", 2292, 0, "%s", "fs.top->type != VAR_CODEPOS");
            while (::pos.top->type != 8)
            {
                RemoveRefToValue(::pos.top->type, ::pos.top->u);
                --::pos.top;
                if (::pos.top->type == 7)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 2297, 0, "%s", "fs.top->type != VAR_CODEPOS");
            }
            goto $LN43_3;
        case 124:
            if (caseCount)
            {
                do
                {
                    v49 = *(_DWORD*)::pos.pos;
                    ::pos.pos += 4;
                    currentCaseValue = v49;
                    v48 = *(const char**)::pos.pos;
                    ::pos.pos += 4;
                    currentCodePos = v48;
                    --caseCount;
                } while (caseCount);
                if (!currentCaseValue)
                {
                    ::pos.pos = currentCodePos;
                    if (!currentCodePos)
                        MyAssertHandler(".\\script\\scr_vm.cpp", 2315, 0, "%s", "fs.pos");
                }
            }
        $LN43_3:
            RemoveRefToValue(::pos.top->type, ::pos.top->u);
        error_dec_top:
            --::pos.top;
            break;
        default:
            break;
        }
        if (scrVmPub.inparamcount)
            MyAssertHandler(".\\script\\scr_vm.cpp", 2322, 0, "%s", "!scrVmPub.inparamcount");
        if (scrVmPub.outparamcount)
            MyAssertHandler(".\\script\\scr_vm.cpp", 2323, 0, "%s", "!scrVmPub.outparamcount");
        if (scrVmPub.showError && !scrVmPub.debugCode && !Scr_IgnoreErrors())
        {
            scrVmPub.showError = 0;
            Scr_ShowConsole();
            Scr_HitBreakpoint(::pos.top, ::pos.pos, ::pos.localId, 0);
        }
    }
    while (2)
    {
        if (scrVarPub.error_message)
            MyAssertHandler(
                ".\\script\\scr_vm.cpp",
                405,
                0,
                "%s\n\t(scrVarPub.error_message) = %s",
                "(!scrVarPub.error_message)",
                scrVarPub.error_message);
        if (scrVarPub.error_index)
            MyAssertHandler(".\\script\\scr_vm.cpp", 406, 0, "%s", "!scrVarPub.error_index");
        if (scrVmPub.outparamcount)
            MyAssertHandler(".\\script\\scr_vm.cpp", 407, 0, "%s", "!scrVmPub.outparamcount");
        if (scrVmPub.inparamcount)
            MyAssertHandler(".\\script\\scr_vm.cpp", 408, 0, "%s", "!scrVmPub.inparamcount");
        opcode = (unsigned __int8)*::pos.pos++;
    interrupt_return:
        scrVarPub.varUsagePos = ::pos.pos;
        switch (opcode)
        {
        case 0:
            parentLocalId = GetSafeParentLocalId(::pos.localId);
            Scr_KillThread(::pos.localId);
            scrVmPub.localVars -= ::pos.localVarCount;
            if (::pos.top->type == 8)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    430,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top->type != VAR_PRECODEPOS)",
                    ::pos.top - scrVmPub.stack);
            while (::pos.top->type != 7)
            {
                RemoveRefToValue(::pos.top->type, ::pos.top->u);
                --::pos.top;
                if (::pos.top->type == 8)
                    MyAssertHandler(
                        ".\\script\\scr_vm.cpp",
                        435,
                        0,
                        "%s\n\t(fs.top - scrVmPub.stack) = %i",
                        "(fs.top->type != VAR_PRECODEPOS)",
                        ::pos.top - scrVmPub.stack);
            }
            --scrVmPub.function_count;
            --scrVmPub.function_frame;
            if (!parentLocalId)
            {
                if (::pos.top != ::pos.startTop)
                    MyAssertHandler(
                        ".\\script\\scr_vm.cpp",
                        442,
                        0,
                        "%s\n\t(fs.top - fs.startTop) = %i",
                        "(fs.top == fs.startTop)",
                        ::pos.top - ::pos.startTop);
            thread_end:
                ::pos.startTop[1].type = 0;
                goto thread_return;
            }
            if (::pos.top->type != 7)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    446,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top->type == VAR_CODEPOS)",
                    ::pos.top - scrVmPub.stack);
            ::pos.top->type = 0;
            goto end_0;
        case 1:
            parentLocalId = GetSafeParentLocalId(::pos.localId);
            Scr_KillThread(::pos.localId);
            scrVmPub.localVars -= ::pos.localVarCount;
            v0 = ::pos.top->type;
            tempValue.u.intValue = ::pos.top->u.intValue;
            tempValue.type = v0;
            --::pos.top;
            if (::pos.top->type == 8)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    466,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top->type != VAR_PRECODEPOS)",
                    ::pos.top - scrVmPub.stack);
            while (::pos.top->type != 7)
            {
                RemoveRefToValue(::pos.top->type, ::pos.top->u);
                --::pos.top;
                if (::pos.top->type == 8)
                    MyAssertHandler(
                        ".\\script\\scr_vm.cpp",
                        471,
                        0,
                        "%s\n\t(fs.top - scrVmPub.stack) = %i",
                        "(fs.top->type != VAR_PRECODEPOS)",
                        ::pos.top - scrVmPub.stack);
            }
            --scrVmPub.function_count;
            --scrVmPub.function_frame;
            if (parentLocalId)
            {
                if (::pos.top->type != 7)
                    MyAssertHandler(
                        ".\\script\\scr_vm.cpp",
                        483,
                        0,
                        "%s\n\t(fs.top - scrVmPub.stack) = %i",
                        "(fs.top->type == VAR_CODEPOS)",
                        ::pos.top - scrVmPub.stack);
                *::pos.top = tempValue;
            end_0:
                if (::pos.top == ::pos.startTop)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 449, 0, "%s", "fs.top != fs.startTop");
                RemoveRefToObject(::pos.localId);
                ::pos.pos = scrVmPub.function_frame->fs.pos;
                if (!::pos.pos)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 454, 0, "%s", "fs.pos");
                ::pos.localVarCount = scrVmPub.function_frame->fs.localVarCount;
                ::pos.localId = parentLocalId;
                continue;
            }
            if (::pos.top != ::pos.startTop)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    478,
                    0,
                    "%s\n\t(fs.top - fs.startTop) = %i",
                    "(fs.top == fs.startTop)",
                    ::pos.top - ::pos.startTop);
            ::pos.top[1] = tempValue;
        thread_return:
            if (*profileEnablePos)
                Profile_EndScripts(*profileEnablePos);
            if (thread_count)
            {
                --profileEnablePos;
                --thread_count;
                RemoveRefToObject(::pos.localId);
                ::pos = scrVmPub.function_frame->fs;
                ::pos.top->type = scrVmPub.function_frame->topType;
                ++::pos.top;
                continue;
            }
            if (g_script_error_level < 0)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    1963,
                    0,
                    "%s\n\t(g_script_error_level) = %i",
                    "(g_script_error_level >= 0)",
                    g_script_error_level);
            --g_script_error_level;
            return ::pos.localId;
        case 2:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    488,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    490,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            ::pos.top->type = 0;
            continue;
        case 3:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    495,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    497,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            ::pos.top->type = 6;
            ::pos.top->u.intValue = 0;
            continue;
        case 4:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    503,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    505,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            ::pos.top->type = 6;
            ::pos.top->u.intValue = (unsigned __int8)*::pos.pos++;
            continue;
        case 5:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    512,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    514,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            ::pos.top->type = 6;
            ::pos.top->u.intValue = -(unsigned __int8)*::pos.pos++;
            continue;
        case 6:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    521,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    523,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            ::pos.top->type = 6;
            v97 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            ::pos.top->u.intValue = v97;
            continue;
        case 7:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    529,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    531,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            ::pos.top->type = 6;
            v96 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            ::pos.top->u.intValue = -v96;
            continue;
        case 8:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    537,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    539,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            ::pos.top->type = 6;
            v95.intValue = *(int*)::pos.pos;
            ::pos.pos += 4;
            ::pos.top->u.intValue = v95.intValue;
            continue;
        case 9:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    545,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    547,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            ::pos.top->type = 5;
            v94.intValue = *(int*)::pos.pos;
            ::pos.pos += 4;
            *(float*)&::pos.top->u.intValue = v94.floatValue;
            continue;
        case 10:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    553,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    555,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            ::pos.top->type = 2;
            v93 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            ::pos.top->u.intValue = v93;
            SL_AddRefToString(::pos.top->u.stringValue);
            continue;
        case 11:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    562,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    564,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            ::pos.top->type = 3;
            v92 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            ::pos.top->u.intValue = v92;
            SL_AddRefToString(::pos.top->u.stringValue);
            continue;
        case 12:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    571,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    573,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            ::pos.top->type = 4;
            v91 = ::pos.pos;
            ::pos.pos += 12;
            ::pos.top->u.intValue = (int)v91;
            continue;
        case 13:
            objectId = scrVarPub.levelId;
            continue;
        case 14:
            objectId = scrVarPub.animId;
            continue;
        case 15:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    587,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    589,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            ::pos.top->type = 1;
            ::pos.top->u.intValue = Scr_GetSelf(::pos.localId);
            AddRefToObject(::pos.top->u.stringValue);
            continue;
        case 16:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    596,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    598,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            ::pos.top->type = 1;
            ::pos.top->u.intValue = scrVarPub.levelId;
            AddRefToObject(scrVarPub.levelId);
            continue;
        case 17:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    605,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    607,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            *::pos.top = Scr_EvalVariable(scrVarPub.gameId);
            continue;
        case 18:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    612,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    614,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            ::pos.top->type = 1;
            ::pos.top->u.intValue = scrVarPub.animId;
            AddRefToObject(scrVarPub.animId);
            continue;
        case 19:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    621,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    623,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            ::pos.top->type = 11;
            v90.intValue = *(int*)::pos.pos;
            ::pos.pos += 4;
            ::pos.top->u.intValue = v90.intValue;
            continue;
        case 20:
            fieldValueIndex = 0;
            fieldValueId = scrVarPub.gameId;
            continue;
        case 21:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    634,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    636,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            ::pos.top->type = 9;
            v89.intValue = *(int*)::pos.pos;
            ::pos.pos += 4;
            ::pos.top->u.intValue = v89.intValue;
            continue;
        case 22:
            ++scrVmPub.localVars;
            ++::pos.localVarCount;
            v88 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            *scrVmPub.localVars = GetNewVariable(::pos.localId, v88);
            continue;
        case 23:
            removeCount = *::pos.pos++;
            scrVmPub.localVars -= removeCount;
            ::pos.localVarCount -= removeCount;
            while (removeCount)
            {
                RemoveNextVariable(::pos.localId);
                --removeCount;
            }
            continue;
        case 24:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    660,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    662,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            *::pos.top = Scr_EvalVariable(*scrVmPub.localVars);
            continue;
        case 25:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    667,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    669,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            *::pos.top = Scr_EvalVariable(*(scrVmPub.localVars - 1));
            continue;
        case 26:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    674,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    676,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            *::pos.top = Scr_EvalVariable(*(scrVmPub.localVars - 2));
            continue;
        case 27:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    681,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    683,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            *::pos.top = Scr_EvalVariable(*(scrVmPub.localVars - 3));
            continue;
        case 28:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    688,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    690,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            *::pos.top = Scr_EvalVariable(*(scrVmPub.localVars - 4));
            continue;
        case 29:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    695,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    697,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            *::pos.top = Scr_EvalVariable(*(scrVmPub.localVars - 5));
            continue;
        case 30:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    702,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    704,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            *::pos.top = Scr_EvalVariable(scrVmPub.localVars[-(unsigned __int8)*::pos.pos]);
            ++::pos.pos;
            continue;
        case 31:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    710,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top >= scrVmPub.stack)",
                    ::pos.top - scrVmPub.stack);
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    712,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top <= scrVmPub.maxstack)",
                    ::pos.top - scrVmPub.stack);
            *::pos.top = Scr_EvalVariable(scrVmPub.localVars[-(unsigned __int8)*::pos.pos]);
            ++::pos.pos;
            goto $LN476;
        case 32:
        $LN476:
            Scr_EvalArray(::pos.top, ::pos.top - 1);
            goto loop_dec_top;
        case 33:
            fieldValueId = *scrVmPub.localVars;
            goto $LN472;
        case 34:
            fieldValueId = scrVmPub.localVars[-(unsigned __int8)*::pos.pos++];
            goto $LN472;
        case 35:
        $LN472:
            objectId = Scr_EvalArrayRef(fieldValueId).stringValue;
            if (::pos.top->type == 6)
            {
                if (!IsValidArrayIndex(::pos.top->u.stringValue))
                {
                    v1 = va("array index %d out of range", ::pos.top->u.intValue);
                    Scr_Error(v1);
                }
                fieldValueIndex = GetArrayVariableIndex(objectId, ::pos.top->u.stringValue);
            }
            else if (::pos.top->type == 2)
            {
                fieldValueIndex = GetVariableIndexInternal(objectId, ::pos.top->u.stringValue);
                _SL_RemoveRefToString(::pos.top->u.stringValue);
            }
            else
            {
                v2 = va("%s is not an array index", var_typename[::pos.top->type]);
                Scr_Error(v2);
            }
            fieldValueId = Scr_GetVarId(fieldValueIndex);
            if (!fieldValueId)
                MyAssertHandler(".\\script\\scr_vm.cpp", 750, 0, "%s", "fieldValueId");
            goto loop_dec_top;
        case 36:
            ClearArray(fieldValueId, ::pos.top);
            goto loop_dec_top;
        case 37:
            ++::pos.top;
            ::pos.top->type = 1;
            ::pos.top->u.intValue = Scr_AllocArray();
            continue;
        case 38:
            objectId = Scr_GetSelf(::pos.localId);
            if (IsFieldObject(objectId))
                continue;
            goto not_an_object;
        case 39:
            objectId = scrVarPub.levelId;
            goto EvalFieldVariable;
        case 40:
            objectId = scrVarPub.animId;
        EvalFieldVariable:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(".\\script\\scr_vm.cpp", 774, 0, "%s", "fs.top >= scrVmPub.stack");
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(".\\script\\scr_vm.cpp", 776, 0, "%s", "fs.top <= scrVmPub.maxstack");
            v87 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            Variable = FindVariable(objectId, v87);
            *::pos.top = Scr_EvalVariable(Variable);
            continue;
        case 41:
            objectId = Scr_GetSelf(::pos.localId);
            if (IsFieldObject(objectId))
                goto $LN452;
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(".\\script\\scr_vm.cpp", 788, 0, "%s", "fs.top >= scrVmPub.stack");
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(".\\script\\scr_vm.cpp", 790, 0, "%s", "fs.top <= scrVmPub.maxstack");
            ::pos.pos += 2;
        not_an_object:
            type = GetObjectType(objectId);
            goto not_an_object_error;
        case 42:
        $LN452:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(".\\script\\scr_vm.cpp", 797, 0, "%s", "fs.top >= scrVmPub.stack");
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(".\\script\\scr_vm.cpp", 799, 0, "%s", "fs.top <= scrVmPub.maxstack");
            v86 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            *::pos.top = Scr_FindVariableField(objectId, v86);
            continue;
        case 43:
            objectId = scrVarPub.levelId;
            goto EvalFieldVariableRef;
        case 44:
            objectId = scrVarPub.animId;
            goto EvalFieldVariableRef;
        case 45:
            objectId = Scr_GetSelf(::pos.localId);
            goto EvalFieldVariableRef;
        case 46:
        EvalFieldVariableRef:
            v85 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            fieldValueIndex = Scr_GetVariableFieldIndex(objectId, v85);
            fieldValueId = Scr_GetVarId(fieldValueIndex);
            continue;
        case 47:
            v84 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            ClearVariableField(objectId, v84, ::pos.top);
            continue;
        case 48:
            ++scrVmPub.localVars;
            ++::pos.localVarCount;
            v83 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            *scrVmPub.localVars = GetNewVariable(::pos.localId, v83);
            goto $LN433;
        case 49:
        $LN433:
            if (::pos.top->type == 7)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    832,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top->type != VAR_CODEPOS)",
                    ::pos.top - scrVmPub.stack);
            if (::pos.top->type != 8)
                goto LABEL_227;
            continue;
        case 50:
            if (::pos.top->type == 7)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    841,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top->type != VAR_CODEPOS)",
                    ::pos.top - scrVmPub.stack);
            if (::pos.top->type != 8)
                goto LABEL_236;
            ++::pos.pos;
            continue;
        case 51:
            if (::pos.top->type == 8)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    852,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top->type != VAR_PRECODEPOS)",
                    ::pos.top - scrVmPub.stack);
            if (::pos.top->type != 7)
                goto LABEL_236;
            ClearVariableValue(scrVmPub.localVars[-(unsigned __int8)*::pos.pos]);
            ++::pos.pos;
            continue;
        case 52:
            if (::pos.top->type == 8)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    864,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top->type != VAR_PRECODEPOS)",
                    ::pos.top - scrVmPub.stack);
            while (::pos.top->type != 7)
            {
                RemoveRefToValue(::pos.top->type, ::pos.top->u);
                --::pos.top;
                if (::pos.top->type == 8)
                    MyAssertHandler(
                        ".\\script\\scr_vm.cpp",
                        869,
                        0,
                        "%s\n\t(fs.top - scrVmPub.stack) = %i",
                        "(fs.top->type != VAR_PRECODEPOS)",
                        ::pos.top - scrVmPub.stack);
            }
            continue;
        case 53:
            if (::pos.top->type == 7)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    874,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top->type != VAR_CODEPOS)",
                    ::pos.top - scrVmPub.stack);
            if (::pos.top->type == 8)
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
                ::pos.top->type = 7;
            }
            else
            {
                Scr_Error("function called with too many parameters");
            $LN407:
                fieldValueIndex = 0;
                fieldValueId = *scrVmPub.localVars;
            }
            continue;
        case 54:
            goto $LN407;
        case 55:
            fieldValueIndex = 0;
            fieldValueId = scrVmPub.localVars[-(unsigned __int8)*::pos.pos++];
            continue;
        case 56:
            v82 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            top = ::pos.top;
            Variable = GetVariable(scrVarPub.levelId, v82);
            SetVariableValue(Variable, top);
            goto loop_dec_top;
        case 57:
            goto $LN403;
        case 58:
            v80 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            v42 = ::pos.top;
            v5 = GetVariable(scrVarPub.animId, v80);
            SetVariableValue(v5, v42);
            goto loop_dec_top;
        case 59:
            v81 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            fieldName = v81;
            objectId = Scr_GetSelf(::pos.localId);
            fieldValueIndex = Scr_GetVariableFieldIndex(objectId, fieldName);
            fieldValueId = Scr_GetVarId(fieldValueIndex);
        $LN403:
            if (fieldValueIndex)
            {
                if (!fieldValueId)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 922, 0, "%s", "fieldValueId");
                if (::pos.top->type)
                    SetVariableValue(fieldValueId, ::pos.top);
                else
                    RemoveVariableValue(objectId, fieldValueIndex);
            }
            else
            {
                SetVariableFieldValue(fieldValueId, ::pos.top);
            }
            goto loop_dec_top;
        case 60:
        LABEL_227:
            SetVariableValue(*scrVmPub.localVars, ::pos.top);
            goto loop_dec_top;
        case 61:
        LABEL_236:
            SetVariableValue(scrVmPub.localVars[-(unsigned __int8)*::pos.pos], ::pos.top);
            ++::pos.pos;
            goto loop_dec_top;
        case 62:
        case 63:
        case 64:
        case 65:
        case 66:
        case 67:
            if (scrVmPub.outparamcount)
                MyAssertHandler(".\\script\\scr_vm.cpp", 956, 0, "%s", "!scrVmPub.outparamcount");
            scrVmPub.outparamcount = opcode - 62;
            goto CallBuiltin;
        case 68:
            if (scrVmPub.outparamcount)
                MyAssertHandler(".\\script\\scr_vm.cpp", 961, 0, "%s", "!scrVmPub.outparamcount");
            scrVmPub.outparamcount = (unsigned __int8)*::pos.pos++;
        CallBuiltin:
            if (scrVmPub.inparamcount)
                MyAssertHandler(".\\script\\scr_vm.cpp", 968, 0, "%s", "!scrVmPub.inparamcount");
            pos = ::pos.pos;
            v79 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            builtinIndex = v79;
            scrVmPub.function_frame->fs.pos = ::pos.pos;
            if (scrVmDebugPub.func_table[builtinIndex].breakpointCount)
            {
                outparamcount = scrVmPub.outparamcount;
                Scr_HitBuiltinBreakpoint(::pos.top, pos, ::pos.localId, opcode, builtinIndex, scrVmPub.outparamcount);
                scrVmPub.outparamcount = outparamcount;
            }
            scrVmPub.top = ::pos.top;
            builtInTime = scrVmDebugPub.builtInTime;
            time = __rdtsc();
            ((void (*)(void))scrCompilePub.func_table[builtinIndex])();
            timeSpent = __rdtsc() - time;
            scrVmDebugPub.builtInTime = timeSpent + builtInTime;
            scrVmDebugPub.func_table[builtinIndex].prof += timeSpent;
            ++scrVmDebugPub.func_table[builtinIndex].usage;
            goto post_builtin;
        case 69:
        case 70:
        case 71:
        case 72:
        case 73:
        case 74:
            if (scrVmPub.outparamcount)
                MyAssertHandler(".\\script\\scr_vm.cpp", 1042, 0, "%s", "!scrVmPub.outparamcount");
            scrVmPub.outparamcount = opcode - 69;
            goto CallBuiltinMethod;
        case 75:
            if (scrVmPub.outparamcount)
                MyAssertHandler(".\\script\\scr_vm.cpp", 1047, 0, "%s", "!scrVmPub.outparamcount");
            scrVmPub.outparamcount = (unsigned __int8)*::pos.pos++;
        CallBuiltinMethod:
            if (scrVmPub.inparamcount)
                MyAssertHandler(".\\script\\scr_vm.cpp", 1054, 0, "%s", "!scrVmPub.inparamcount");
            pos = ::pos.pos;
            scrVmPub.top = ::pos.top - 1;
            v78 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            builtinIndex = v78;
            if (::pos.top->type != 1)
                goto LABEL_308;
            objectId = ::pos.top->u.stringValue;
            if (GetObjectType(objectId) != 20)
            {
                type = GetObjectType(objectId);
                RemoveRefToObject(objectId);
                scrVarPub.error_index = -1;
                v6 = va("%s is not an entity", var_typename[type]);
                Scr_Error(v6);
            LABEL_308:
                type = ::pos.top->type;
                RemoveRefToValue(::pos.top->type, ::pos.top->u);
                scrVarPub.error_index = -1;
                v7 = va("%s is not an entity", var_typename[type]);
                Scr_Error(v7);
            $LN359:
                if (!Scr_IsInOpcodeMemory(::pos.pos))
                    MyAssertHandler(".\\script\\scr_vm.cpp", 1122, 0, "%s", "Scr_IsInOpcodeMemory( fs.pos )");
                if (::pos.top->type == 5)
                {
                    if (*(float*)&::pos.top->u.intValue < 0.0)
                        goto negWait;
                    v77 = *(float*)&::pos.top->u.intValue * 20.0;
                    waitTime = (int)(v77 + 9.313225746154785e-10);
                    if (!waitTime)
                        waitTime = *(float*)&::pos.top->u.intValue != 0.0;
                }
                else if (::pos.top->type == 6)
                {
                    waitTime = 20 * ::pos.top->u.intValue;
                }
                else
                {
                    scrVarPub.error_index = 2;
                    v8 = va("type %s is not a float", var_typename[::pos.top->type]);
                    Scr_Error(v8);
                }
                if (waitTime < (unsigned int)&clients[0].parseClients[238].attachTagIndex[4] + 3)
                {
                    if (waitTime)
                        Scr_ResetTimeout();
                    waitTime = ((unsigned int)&clients[0].parseClients[238].attachTagIndex[4] + 3) & (waitTime + scrVarPub.time);
                    --::pos.top;
                    scrVmDebugPub.profileEnable[::pos.localId] = *profileEnablePos;
                    stackValue.type = 10;
                    stackValue.u.intValue = (int)VM_ArchiveStack();
                    v9 = GetVariable(scrVarPub.timeArrayId, waitTime);
                    id = GetArray(v9).u.stringValue;
                    stackId = GetNewObjectVariable(id, ::pos.localId);
                    SetNewVariableValue(stackId, &stackValue);
                    Scr_SetThreadWaitTime(::pos.localId, waitTime);
                    goto thread_end;
                }
                scrVarPub.error_index = 2;
                if (waitTime >= 0)
                    Scr_Error("wait is too long");
            negWait:
                Scr_Error("negative wait is not allowed");
            $LN345:
                if (!Scr_IsInOpcodeMemory(::pos.pos))
                    MyAssertHandler(".\\script\\scr_vm.cpp", 1173, 0, "%s", "Scr_IsInOpcodeMemory( fs.pos )");
                if ((scrVarPub.time & 0xFF000000) != 0)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 1174, 0, "%s", "!(scrVarPub.time & ~VAR_NAME_LOW_MASK)");
                scrVmDebugPub.profileEnable[::pos.localId] = *profileEnablePos;
                stackValue.type = 10;
                stackValue.u.intValue = (int)VM_ArchiveStack();
                v10 = GetVariable(scrVarPub.timeArrayId, scrVarPub.time);
                id = GetArray(v10).u.stringValue;
                stackId = GetNewObjectVariableReverse(id, ::pos.localId);
                SetNewVariableValue(stackId, &stackValue);
                Scr_SetThreadWaitTime(::pos.localId, scrVarPub.time);
                goto thread_end;
            }
            entref = Scr_GetEntityIdRef(objectId);
            RemoveRefToObject(objectId);
            scrVmPub.function_frame->fs.pos = ::pos.pos;
            if (scrVmGlob.recordPlace)
                Scr_GetFileAndLine(::pos.pos, &scrVmGlob.lastFileName, &scrVmGlob.lastLine);
            if (scrVmDebugPub.func_table[builtinIndex].breakpointCount)
            {
                if (scrVmPub.top != ::pos.top - 1)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 1084, 0, "%s", "scrVmPub.top == fs.top - 1");
                v98 = scrVmPub.outparamcount;
                Scr_HitBuiltinBreakpoint(::pos.top, pos, ::pos.localId, opcode, builtinIndex, scrVmPub.outparamcount + 1);
                scrVmPub.outparamcount = v98;
                scrVmPub.top = ::pos.top - 1;
            }
            builtInTime = scrVmDebugPub.builtInTime;
            time = __rdtsc();
            ((void(__cdecl*)(_DWORD))scrCompilePub.func_table[builtinIndex])(entref);
            timeSpent = __rdtsc() - time;
            scrVmDebugPub.builtInTime = timeSpent + builtInTime;
            scrVmDebugPub.func_table[builtinIndex].prof += timeSpent;
            ++scrVmDebugPub.func_table[builtinIndex].usage;
        post_builtin:
            ::pos.top = scrVmPub.top;
            ::pos.pos = scrVmPub.function_frame->fs.pos;
            if (scrVmPub.outparamcount)
            {
                paramcount = scrVmPub.outparamcount;
                scrVmPub.outparamcount = 0;
                scrVmPub.top -= paramcount;
                do
                {
                    RemoveRefToValue(::pos.top->type, ::pos.top->u);
                    --::pos.top;
                    --paramcount;
                } while (paramcount);
            }
            if (scrVmPub.inparamcount)
            {
                if (scrVmPub.inparamcount != 1)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 1022, 0, "%s", "scrVmPub.inparamcount == 1");
                scrVmPub.inparamcount = 0;
                if (::pos.top != scrVmPub.top)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 1024, 0, "%s", "fs.top == scrVmPub.top");
            }
            else
            {
                if (::pos.top != scrVmPub.top)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 1029, 0, "%s", "fs.top == scrVmPub.top");
                ++::pos.top;
                ::pos.top->type = 0;
            }
            continue;
        case 76:
            goto $LN359;
        case 77:
            goto $LN345;
        case 78:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(".\\script\\scr_vm.cpp", 1192, 0, "%s", "fs.top >= scrVmPub.stack");
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(".\\script\\scr_vm.cpp", 1194, 0, "%s", "fs.top <= scrVmPub.maxstack");
            ::pos.top->type = 8;
            continue;
        case 79:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(".\\script\\scr_vm.cpp", 1199, 0, "%s", "fs.top >= scrVmPub.stack");
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(".\\script\\scr_vm.cpp", 1201, 0, "%s", "fs.top <= scrVmPub.maxstack");
            ::pos.top->type = 8;
            goto $LN329;
        case 80:
        $LN329:
            if (scrVmPub.function_count < 31)
            {
                selfId = Scr_GetSelf(::pos.localId);
                AddRefToObject(selfId);
                ::pos.localId = AllocChildThread((ObjectInfo_u)selfId, ::pos.localId);
                scrVmPub.function_frame->fs.pos = ::pos.pos;
                v76 = *(const char**)scrVmPub.function_frame->fs.pos;
                scrVmPub.function_frame->fs.pos += 4;
                ::pos.pos = v76;
                goto function_call;
            }
            Scr_Error("script stack overflow (too many embedded function calls)");
        $LN326:
            if (::pos.top->type != 9)
                goto LABEL_348;
            if (scrVmPub.function_count < 31)
            {
                selfId = Scr_GetSelf(::pos.localId);
                AddRefToObject(selfId);
                ::pos.localId = AllocChildThread((ObjectInfo_u)selfId, ::pos.localId);
                scrVmPub.function_frame->fs.pos = ::pos.pos;
                ::pos.pos = ::pos.top->u.codePosValue;
                --::pos.top;
                goto function_call;
            }
            scrVarPub.error_index = 1;
            Scr_Error("script stack overflow (too many embedded function calls)");
        LABEL_348:
            v11 = va("%s is not a function pointer", var_typename[::pos.top->type]);
            Scr_Error(v11);
        $LN322:
            if (::pos.top->type != 1)
                goto not_an_object1;
            if (scrVmPub.function_count < 31)
            {
                ::pos.localId = AllocChildThread((ObjectInfo_u)::pos.top->u.intValue, ::pos.localId);
                --::pos.top;
                scrVmPub.function_frame->fs.pos = ::pos.pos;
                v75 = *(const char**)scrVmPub.function_frame->fs.pos;
                scrVmPub.function_frame->fs.pos += 4;
                ::pos.pos = v75;
                goto function_call;
            }
            Scr_Error("script stack overflow (too many embedded function calls)");
        not_an_object1:
            type = ::pos.top->type;
            goto not_an_object_error1;
        case 81:
            goto $LN326;
        case 82:
            goto $LN322;
        case 83:
            if (::pos.top->type == 9)
            {
                tempCodePos = ::pos.top->u.codePosValue;
                --::pos.top;
                if (::pos.top->type != 1)
                    goto not_an_object2;
                if (scrVmPub.function_count < 31)
                {
                    ::pos.localId = AllocChildThread((ObjectInfo_u)::pos.top->u.intValue, ::pos.localId);
                    --::pos.top;
                    scrVmPub.function_frame->fs.pos = ::pos.pos;
                    ::pos.pos = tempCodePos;
                    goto function_call;
                }
                scrVarPub.error_index = 1;
                Scr_Error("script stack overflow (too many embedded function calls)");
            }
            else
            {
                RemoveRefToValue(::pos.top->type, ::pos.top->u);
                --::pos.top;
                v12 = va("%s is not a function pointer", var_typename[::pos.top[1].type]);
                Scr_Error(v12);
            $LN311:
                if (scrVmPub.function_count < 31)
                {
                    selfId = Scr_GetSelf(::pos.localId);
                    AddRefToObject(selfId);
                    ::pos.localId = AllocThread((ObjectInfo_u)selfId);
                    scrVmPub.function_frame->fs.pos = ::pos.pos;
                    scrVmPub.function_frame->fs.startTop = ::pos.startTop;
                    v74 = *(const char**)scrVmPub.function_frame->fs.pos;
                    scrVmPub.function_frame->fs.pos += 4;
                    ::pos.pos = v74;
                    v73 = *(_DWORD*)scrVmPub.function_frame->fs.pos;
                    scrVmPub.function_frame->fs.pos += 4;
                    ::pos.startTop = &::pos.top[-v73];
                    goto thread_call;
                }
                scrVarPub.error_index = 1;
                Scr_Error("script stack overflow (too many embedded function calls)");
            $LN308:
                if (::pos.top->type == 9)
                {
                    if (scrVmPub.function_count < 31)
                    {
                        tempCodePos = ::pos.top->u.codePosValue;
                        --::pos.top;
                        selfId = Scr_GetSelf(::pos.localId);
                        AddRefToObject(selfId);
                        ::pos.localId = AllocThread((ObjectInfo_u)selfId);
                        scrVmPub.function_frame->fs.pos = ::pos.pos;
                        scrVmPub.function_frame->fs.startTop = ::pos.startTop;
                        ::pos.pos = tempCodePos;
                        v72 = *(_DWORD*)scrVmPub.function_frame->fs.pos;
                        scrVmPub.function_frame->fs.pos += 4;
                        ::pos.startTop = &::pos.top[-v72];
                        goto thread_call;
                    }
                    scrVarPub.error_index = 1;
                    Scr_Error("script stack overflow (too many embedded function calls)");
                }
                v13 = va("%s is not a function pointer", var_typename[::pos.top->type]);
                Scr_Error(v13);
            $LN304:
                if (::pos.top->type == 1)
                {
                    if (scrVmPub.function_count < 31)
                    {
                        ::pos.localId = AllocThread((ObjectInfo_u)::pos.top->u.intValue);
                        --::pos.top;
                        scrVmPub.function_frame->fs.pos = ::pos.pos;
                        scrVmPub.function_frame->fs.startTop = ::pos.startTop;
                        v71 = *(const char**)scrVmPub.function_frame->fs.pos;
                        scrVmPub.function_frame->fs.pos += 4;
                        ::pos.pos = v71;
                        v70 = *(_DWORD*)scrVmPub.function_frame->fs.pos;
                        scrVmPub.function_frame->fs.pos += 4;
                        ::pos.startTop = &::pos.top[-v70];
                        goto thread_call;
                    }
                    scrVarPub.error_index = 1;
                    Scr_Error("script stack overflow (too many embedded function calls)");
                }
            }
            goto not_an_object2;
        case 84:
            goto $LN311;
        case 85:
            goto $LN308;
        case 86:
            goto $LN304;
        case 87:
            if (::pos.top->type != 9)
            {
                RemoveRefToValue(::pos.top->type, ::pos.top->u);
                --::pos.top;
                v14 = va("%s is not a function pointer", var_typename[::pos.top[1].type]);
                Scr_Error(v14);
            $LN293:
                RemoveRefToValue(::pos.top->type, ::pos.top->u);
                goto loop_dec_top;
            }
            tempCodePos = ::pos.top->u.codePosValue;
            --::pos.top;
            if (::pos.top->type != 1)
                goto not_an_object2;
            if (scrVmPub.function_count >= 31)
            {
                scrVarPub.error_index = 1;
                Scr_Error("script stack overflow (too many embedded function calls)");
            not_an_object2:
                type = ::pos.top->type;
                goto not_an_object_error2;
            }
            ::pos.localId = AllocThread((ObjectInfo_u)::pos.top->u.intValue);
            --::pos.top;
            scrVmPub.function_frame->fs.pos = ::pos.pos;
            scrVmPub.function_frame->fs.startTop = ::pos.startTop;
            ::pos.pos = tempCodePos;
            v69 = *(_DWORD*)scrVmPub.function_frame->fs.pos;
            scrVmPub.function_frame->fs.pos += 4;
            ::pos.startTop = &::pos.top[-v69];
        thread_call:
            scrVmPub.function_frame->fs.top = ::pos.startTop;
            scrVmPub.function_frame->topType = ::pos.startTop->type;
            ::pos.startTop->type = 8;
            ++thread_count;
            *++profileEnablePos = 0;
        function_call:
            scrVmPub.function_frame->fs.localVarCount = ::pos.localVarCount;
            ::pos.localVarCount = 0;
            ++scrVmPub.function_count;
            ++scrVmPub.function_frame;
            scrVmPub.function_frame->fs.localId = ::pos.localId;
            if (!::pos.pos)
                MyAssertHandler(".\\script\\scr_vm.cpp", 1949, 0, "%s", "fs.pos");
            continue;
        case 88:
            goto $LN293;
        case 89:
            objectId = Scr_EvalFieldObject(scrVarPub.tempVariable, ::pos.top).stringValue;
            goto loop_dec_top;
        case 90:
            objectId = Scr_EvalVariableObject(scrVmPub.localVars[-(unsigned __int8)*::pos.pos]).u.stringValue;
            ++::pos.pos;
            continue;
        case 91:
            Scr_CastBool(::pos.top);
            continue;
        case 92:
            Scr_EvalBoolNot(::pos.top);
            continue;
        case 93:
            Scr_EvalBoolComplement(::pos.top);
            continue;
        case 94:
            Scr_CastBool(::pos.top);
            if (::pos.top->type != 6)
                MyAssertHandler(".\\script\\scr_vm.cpp", 1397, 0, "%s", "fs.top->type == VAR_INTEGER");
            v68 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            jumpOffset = v68;
            if (!::pos.top->u.intValue)
                ::pos.pos += jumpOffset;
            goto loop_dec_top;
        case 95:
            Scr_CastBool(::pos.top);
            if (::pos.top->type != 6)
                MyAssertHandler(".\\script\\scr_vm.cpp", 1406, 0, "%s", "fs.top->type == VAR_INTEGER");
            v67 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            jumpOffset = v67;
            if (::pos.top->u.intValue)
                ::pos.pos += jumpOffset;
            goto loop_dec_top;
        case 96:
            Scr_CastBool(::pos.top);
            if (::pos.top->type != 6)
                MyAssertHandler(".\\script\\scr_vm.cpp", 1415, 0, "%s", "fs.top->type == VAR_INTEGER");
            v66 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            jumpOffset = v66;
            if (::pos.top->u.intValue)
                goto loop_dec_top;
            ::pos.pos += jumpOffset;
            continue;
        case 97:
            Scr_CastBool(::pos.top);
            if (::pos.top->type != 6)
                MyAssertHandler(".\\script\\scr_vm.cpp", 1424, 0, "%s", "fs.top->type == VAR_INTEGER");
            v65 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            jumpOffset = v65;
            if (!::pos.top->u.intValue)
                goto loop_dec_top;
            ::pos.pos += jumpOffset;
            continue;
        case 98:
            v64 = *(_DWORD*)::pos.pos;
            ::pos.pos += 4;
            jumpOffset = v64;
            ::pos.pos += v64;
            continue;
        case 99:
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
            if ((int)(Sys_Milliseconds() - scrVmGlob.starttime) >= 2500)
            {
                if (!logScriptTimes)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 1465, 0, "%s", "logScriptTimes");
                if (logScriptTimes->current.enabled)
                {
                    v15 = Sys_Milliseconds();
                    Com_Printf(23, "EXCEED TIME: %d\n", v15);
                }
                if (!scrVmGlob.loading)
                {
                    VM_PrintJumpHistory(v45, v46);
                    if (scrVmPub.showError)
                    {
                        if (scrVmPub.debugCode)
                            MyAssertHandler(".\\script\\scr_vm.cpp", 1497, 0, "%s", "!scrVmPub.debugCode");
                        Scr_DumpScriptThreads();
                        Scr_DumpScriptVariablesDefault();
                        Scr_Error("potential infinite loop in script");
                    }
                    if (!scrVmPub.abort_on_error)
                    {
                        Com_PrintError(23, "script runtime error: potential infinite loop in script - killing thread.\n");
                        Scr_PrintPrevCodePos(23, ::pos.pos, 0);
                        Scr_ResetTimeout();
                        while (1)
                        {
                            parentLocalId = GetSafeParentLocalId(::pos.localId);
                            Scr_KillThread(::pos.localId);
                            scrVmPub.localVars -= ::pos.localVarCount;
                            if (::pos.top->type == 8)
                                MyAssertHandler(
                                    ".\\script\\scr_vm.cpp",
                                    1986,
                                    0,
                                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                                    "(fs.top->type != VAR_PRECODEPOS)",
                                    ::pos.top - scrVmPub.stack);
                            while (::pos.top->type != 7)
                            {
                                RemoveRefToValue(::pos.top->type, ::pos.top->u);
                                --::pos.top;
                                if (::pos.top->type == 8)
                                    MyAssertHandler(
                                        ".\\script\\scr_vm.cpp",
                                        1991,
                                        0,
                                        "%s\n\t(fs.top - scrVmPub.stack) = %i",
                                        "(fs.top->type != VAR_PRECODEPOS)",
                                        ::pos.top - scrVmPub.stack);
                            }
                            --scrVmPub.function_count;
                            --scrVmPub.function_frame;
                            if (!parentLocalId)
                                break;
                            if (::pos.top == ::pos.startTop)
                                MyAssertHandler(".\\script\\scr_vm.cpp", 2002, 0, "%s", "fs.top != fs.startTop");
                            RemoveRefToObject(::pos.localId);
                            if (::pos.top->type != 7)
                                MyAssertHandler(
                                    ".\\script\\scr_vm.cpp",
                                    2006,
                                    0,
                                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                                    "(fs.top->type == VAR_CODEPOS)",
                                    ::pos.top - scrVmPub.stack);
                            ::pos.pos = scrVmPub.function_frame->fs.pos;
                            if (!::pos.pos)
                                MyAssertHandler(".\\script\\scr_vm.cpp", 2008, 0, "%s", "fs.pos");
                            ::pos.localVarCount = scrVmPub.function_frame->fs.localVarCount;
                            ::pos.localId = parentLocalId;
                            --::pos.top;
                        }
                        if (::pos.top != ::pos.startTop)
                            MyAssertHandler(
                                ".\\script\\scr_vm.cpp",
                                1998,
                                0,
                                "%s\n\t(fs.top - fs.startTop) = %i",
                                "(fs.top == fs.startTop)",
                                ::pos.top - ::pos.startTop);
                        goto thread_end;
                    }
                    Scr_TerminalError("potential infinite loop in script");
                $LN255:
                    if (::pos.top < scrVmPub.stack)
                        MyAssertHandler(".\\script\\scr_vm.cpp", 1518, 0, "%s", "fs.top >= scrVmPub.stack");
                    if (++::pos.top > scrVmPub.maxstack)
                        MyAssertHandler(".\\script\\scr_vm.cpp", 1520, 0, "%s", "fs.top <= scrVmPub.maxstack");
                    *::pos.top = Scr_EvalVariableField(fieldValueId);
                    if (::pos.top->type == 6)
                    {
                        ++::pos.top->u.intValue;
                        if (*::pos.pos != 57)
                            MyAssertHandler(".\\script\\scr_vm.cpp", 1525, 0, "%s", "*fs.pos == OP_SetVariableField");
                    }
                    else
                    {
                        v16 = va("++ must be applied to an int (applied to %s)", var_typename[::pos.top->type]);
                        Scr_Error(v16);
                    $LN247:
                        if (::pos.top < scrVmPub.stack)
                            MyAssertHandler(".\\script\\scr_vm.cpp", 1533, 0, "%s", "fs.top >= scrVmPub.stack");
                        if (++::pos.top > scrVmPub.maxstack)
                            MyAssertHandler(".\\script\\scr_vm.cpp", 1535, 0, "%s", "fs.top <= scrVmPub.maxstack");
                        *::pos.top = Scr_EvalVariableField(fieldValueId);
                        if (::pos.top->type != 6)
                        {
                            v17 = va("-- must be applied to an int (applied to %s)", var_typename[::pos.top->type]);
                            Scr_Error(v17);
                        $LN239:
                            Scr_EvalOr(::pos.top - 1, ::pos.top);
                            goto loop_dec_top;
                        }
                        --::pos.top->u.intValue;
                        if (*::pos.pos != 57)
                            MyAssertHandler(".\\script\\scr_vm.cpp", 1540, 0, "%s", "*fs.pos == OP_SetVariableField");
                    }
                    ++::pos.pos;
                    SetVariableFieldValue(fieldValueId, ::pos.top);
                loop_dec_top:
                    --::pos.top;
                    continue;
                }
                if (scrVmPub.abort_on_error)
                    scrVmPub.showError = 1;
                Com_PrintWarning(23, "script runtime warning: potential infinite loop in script.\n");
                Scr_PrintPrevCodePos(23, ::pos.pos, 0);
                v62 = *(_WORD*)::pos.pos;
                ::pos.pos += 2;
                jumpOffset = v62;
                ::pos.pos -= v62;
                Scr_ResetTimeout();
            }
            else
            {
                scrVmDebugPub.jumpbackHistory[scrVmDebugPub.jumpbackHistoryIndex] = ::pos.pos;
                scrVmDebugPub.jumpbackHistoryIndex = (scrVmDebugPub.jumpbackHistoryIndex + 1) % 0x80u;
                v63 = *(_WORD*)::pos.pos;
                ::pos.pos += 2;
                jumpOffset = v63;
                ::pos.pos -= v63;
            }
            continue;
        case 100:
            goto $LN255;
        case 101:
            goto $LN247;
        case 102:
            goto $LN239;
        case 103:
            Scr_EvalExOr(::pos.top - 1, ::pos.top);
            goto loop_dec_top;
        case 104:
            Scr_EvalAnd(::pos.top - 1, ::pos.top);
            goto loop_dec_top;
        case 105:
            Scr_EvalEquality(::pos.top - 1, ::pos.top);
            goto loop_dec_top;
        case 106:
            Scr_EvalInequality(::pos.top - 1, ::pos.top);
            goto loop_dec_top;
        case 107:
            Scr_EvalLess(::pos.top - 1, ::pos.top);
            goto loop_dec_top;
        case 108:
            Scr_EvalGreater(::pos.top - 1, ::pos.top);
            goto loop_dec_top;
        case 109:
            Scr_EvalLessEqual(::pos.top - 1, ::pos.top);
            goto loop_dec_top;
        case 110:
            Scr_EvalGreaterEqual(::pos.top - 1, ::pos.top);
            goto loop_dec_top;
        case 111:
            Scr_EvalShiftLeft(::pos.top - 1, ::pos.top);
            goto loop_dec_top;
        case 112:
            Scr_EvalShiftRight(::pos.top - 1, ::pos.top);
            goto loop_dec_top;
        case 113:
            Scr_EvalPlus(::pos.top - 1, ::pos.top);
            goto loop_dec_top;
        case 114:
            Scr_EvalMinus(::pos.top - 1, ::pos.top);
            goto loop_dec_top;
        case 115:
            Scr_EvalMultiply(::pos.top - 1, ::pos.top);
            goto loop_dec_top;
        case 116:
            Scr_EvalDivide(::pos.top - 1, ::pos.top);
            goto loop_dec_top;
        case 117:
            Scr_EvalMod(::pos.top - 1, ::pos.top);
            goto loop_dec_top;
        case 118:
            Scr_EvalSizeValue(::pos.top);
            continue;
        case 119:
        case 120:
            if (!Scr_IsInOpcodeMemory(::pos.pos))
                MyAssertHandler(".\\script\\scr_vm.cpp", 1617, 0, "%s", "Scr_IsInOpcodeMemory( fs.pos )");
            if (::pos.top->type != 1)
                goto not_an_object2;
            if (!IsFieldObject(::pos.top->u.stringValue))
                goto not_an_object2a;
            tempValue.u.intValue = ::pos.top->u.intValue;
            --::pos.top;
            if (::pos.top->type != 2)
            {
                ++::pos.top;
                scrVarPub.error_index = 3;
                Scr_Error("first parameter of waittill must evaluate to a string");
            not_an_object2a:
                type = GetObjectType(::pos.top->u.stringValue);
            not_an_object_error2:
                scrVarPub.error_index = 2;
            not_an_object_error:
                v37 = va("%s is not an object", var_typename[type]);
                Scr_Error(v37);
                goto error_1;
            }
            stringValue = ::pos.top->u.stringValue;
            --::pos.top;
            if (GetObjectType(tempValue.u.stringValue) == 14)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    1631,
                    0,
                    "%s",
                    "GetObjectType( tempValue.u.pointerValue ) != VAR_THREAD");
            if (GetObjectType(tempValue.u.stringValue) == 15)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    1632,
                    0,
                    "%s",
                    "GetObjectType( tempValue.u.pointerValue ) != VAR_NOTIFY_THREAD");
            if (GetObjectType(tempValue.u.stringValue) == 16)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    1633,
                    0,
                    "%s",
                    "GetObjectType( tempValue.u.pointerValue ) != VAR_TIME_THREAD");
            if (GetObjectType(tempValue.u.stringValue) == 17)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    1634,
                    0,
                    "%s",
                    "GetObjectType( tempValue.u.pointerValue ) != VAR_CHILD_THREAD");
            if (GetObjectType(tempValue.u.stringValue) == 22)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    1635,
                    0,
                    "%s",
                    "GetObjectType( tempValue.u.pointerValue ) != VAR_DEAD_THREAD");
            scrVmDebugPub.profileEnable[::pos.localId] = *profileEnablePos;
            stackValue.type = 10;
            stackValue.u.intValue = (int)VM_ArchiveStack();
            v43 = stringValue;
            v18 = GetVariable(tempValue.u.stringValue, 0x18000u);
            Array = GetArray(v18);
            v20 = GetVariable(Array.u.stringValue, v43);
            id = GetArray(v20).u.stringValue;
            stackId = GetNewObjectVariable(id, ::pos.localId);
            SetNewVariableValue(stackId, &stackValue);
            tempValue.type = 1;
            localId = ::pos.localId;
            Self = Scr_GetSelf(::pos.localId);
            ObjectVariable = GetObjectVariable(scrVarPub.pauseArrayId, Self);
            v23 = GetArray(ObjectVariable);
            NewObjectVariable = GetNewObjectVariable(v23.u.stringValue, localId);
            SetNewVariableValue(NewObjectVariable, &tempValue);
            Scr_SetThreadNotifyName(::pos.localId, stringValue);
            goto thread_end;
        case 121:
            if (::pos.top->type != 1)
                goto not_an_object2;
            id = ::pos.top->u.stringValue;
            if (!IsFieldObject(id))
                goto not_an_object2a;
            --::pos.top;
            if (::pos.top->type != 2)
            {
                ++::pos.top;
                scrVarPub.error_index = 1;
                Scr_Error("first parameter of notify must evaluate to a string");
                goto not_an_object2a;
            }
            stringValue = ::pos.top->u.stringValue;
            --::pos.top;
            if (scrVmDebugPub.checkBreakon)
                Scr_CheckBreakonNotify(id, stringValue, ::pos.top, ::pos.pos, ::pos.localId);
            scrVmPub.function_frame->fs.pos = ::pos.pos;
            VM_Notify(id, stringValue, ::pos.top);
            ::pos.pos = scrVmPub.function_frame->fs.pos;
            RemoveRefToObject(id);
            _SL_RemoveRefToString(stringValue);
            if (::pos.top->type == 7)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    1694,
                    0,
                    "%s\n\t(fs.top - scrVmPub.stack) = %i",
                    "(fs.top->type != VAR_CODEPOS)",
                    ::pos.top - scrVmPub.stack);
            while (::pos.top->type != 8)
            {
                RemoveRefToValue(::pos.top->type, ::pos.top->u);
                --::pos.top;
                if (::pos.top->type == 7)
                    MyAssertHandler(
                        ".\\script\\scr_vm.cpp",
                        1699,
                        0,
                        "%s\n\t(fs.top - scrVmPub.stack) = %i",
                        "(fs.top->type != VAR_CODEPOS)",
                        ::pos.top - scrVmPub.stack);
            }
            goto loop_dec_top;
        case 122:
            if (::pos.top->type != 1)
                goto not_an_object1;
            if (!IsFieldObject(::pos.top->u.stringValue))
                goto LABEL_510;
            if (::pos.top[-1].type == 2)
            {
                stringValue = ::pos.top[-1].u.stringValue;
                AddRefToObject(::pos.localId);
                threadId = AllocThread((ObjectInfo_u)::pos.localId);
                if (GetObjectType(::pos.top->u.stringValue) == 14)
                    MyAssertHandler(
                        ".\\script\\scr_vm.cpp",
                        1727,
                        0,
                        "%s",
                        "GetObjectType( fs.top->u.pointerValue ) != VAR_THREAD");
                if (GetObjectType(::pos.top->u.stringValue) == 15)
                    MyAssertHandler(
                        ".\\script\\scr_vm.cpp",
                        1728,
                        0,
                        "%s",
                        "GetObjectType( fs.top->u.pointerValue ) != VAR_NOTIFY_THREAD");
                if (GetObjectType(::pos.top->u.stringValue) == 16)
                    MyAssertHandler(
                        ".\\script\\scr_vm.cpp",
                        1729,
                        0,
                        "%s",
                        "GetObjectType( fs.top->u.pointerValue ) != VAR_TIME_THREAD");
                if (GetObjectType(::pos.top->u.stringValue) == 17)
                    MyAssertHandler(
                        ".\\script\\scr_vm.cpp",
                        1730,
                        0,
                        "%s",
                        "GetObjectType( fs.top->u.pointerValue ) != VAR_CHILD_THREAD");
                if (GetObjectType(::pos.top->u.stringValue) == 22)
                    MyAssertHandler(
                        ".\\script\\scr_vm.cpp",
                        1731,
                        0,
                        "%s",
                        "GetObjectType( fs.top->u.pointerValue ) != VAR_DEAD_THREAD");
                v44 = threadId;
                v39 = stringValue;
                v25 = GetVariable(::pos.top->u.stringValue, 0x18000u);
                v26 = GetArray(v25);
                v27 = GetVariable(v26.u.stringValue, v39);
                v28 = GetArray(v27);
                GetObjectVariable(v28.u.stringValue, v44);
                RemoveRefToObject(threadId);
                tempValue.type = 1;
                tempValue.u.intValue = ::pos.top->u.intValue;
                v40 = threadId;
                v29 = GetObjectVariable(scrVarPub.pauseArrayId, ::pos.localId);
                v30 = GetArray(v29);
                v31 = GetNewObjectVariable(v30.u.stringValue, v40);
                SetNewVariableValue(v31, &tempValue);
                Scr_SetThreadNotifyName(threadId, stringValue);
                ::pos.top -= 2;
                continue;
            }
            Scr_Error("first parameter of endon must evaluate to a string");
        LABEL_510:
            type = GetObjectType(::pos.top->u.stringValue);
        not_an_object_error1:
            scrVarPub.error_index = 1;
            goto not_an_object_error;
        case 123:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(".\\script\\scr_vm.cpp", 1760, 0, "%s", "fs.top >= scrVmPub.stack");
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(".\\script\\scr_vm.cpp", 1762, 0, "%s", "fs.top <= scrVmPub.maxstack");
            ::pos.top->type = 8;
            continue;
        case 124:
            v61 = *(_DWORD*)::pos.pos;
            ::pos.pos += 4;
            jumpOffset = v61;
            ::pos.pos += v61;
            v60 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            caseCount = v60;
            v47 = ::pos.top->type;
            if (v47 == 2)
                goto LABEL_522;
            if (v47 == 6)
            {
                if (IsValidArrayIndex(::pos.top->u.stringValue))
                {
                    caseValue = GetInternalVariableIndex(::pos.top->u.stringValue);
                }
                else
                {
                    v32 = va("switch index %d out of range", ::pos.top->u.intValue);
                    Scr_Error(v32);
                LABEL_522:
                    caseValue = ::pos.top->u.stringValue;
                    _SL_RemoveRefToString(::pos.top->u.stringValue);
                }
            }
            else
            {
                v33 = va("cannot switch on %s", var_typename[::pos.top->type]);
                Scr_Error(v33);
            }
            if (!caseCount)
                goto loop_dec_top;
            if (!caseValue)
                MyAssertHandler(".\\script\\scr_vm.cpp", 1796, 0, "%s", "caseValue");
            do
            {
                v59 = *(_DWORD*)::pos.pos;
                ::pos.pos += 4;
                currentCaseValue = v59;
                v58 = *(const char**)::pos.pos;
                ::pos.pos += 4;
                currentCodePos = v58;
                if (v59 == caseValue)
                {
                    ::pos.pos = currentCodePos;
                    if (!currentCodePos)
                        MyAssertHandler(".\\script\\scr_vm.cpp", 1805, 0, "%s", "fs.pos");
                    goto loop_dec_top;
                }
                --caseCount;
            } while (caseCount);
            if (!currentCaseValue)
            {
                ::pos.pos = currentCodePos;
                if (!currentCodePos)
                    MyAssertHandler(".\\script\\scr_vm.cpp", 1815, 0, "%s", "fs.pos");
            }
            goto loop_dec_top;
        case 125:
            v57 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            caseCount = v57;
            ::pos.pos += 8 * v57;
            continue;
        case 126:
            ::pos.top -= 2;
            Scr_CastVector(::pos.top);
            continue;
        case 127:
            continue;
        case 128:
            if (g_script_error_level < 0)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    1831,
                    0,
                    "%s\n\t(g_script_error_level) = %i",
                    "(g_script_error_level >= 0)",
                    g_script_error_level);
            --g_script_error_level;
            return 0;
        case 129:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(".\\script\\scr_vm.cpp", 1837, 0, "%s", "fs.top >= scrVmPub.stack");
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(".\\script\\scr_vm.cpp", 1839, 0, "%s", "fs.top <= scrVmPub.maxstack");
            v56 = *(_DWORD*)::pos.pos;
            ::pos.pos += 4;
            v55 = *(_DWORD*)::pos.pos;
            ::pos.pos += 4;
            entnum = v55;
            ::pos.top->u.intValue = FindEntityId(v55, v56).u.intValue;
            if (!::pos.top->u.intValue)
            {
                ::pos.top->type = 0;
                Scr_Error("unknown object");
            }
            goto object;
        case 130:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(".\\script\\scr_vm.cpp", 1858, 0, "%s", "fs.top >= scrVmPub.stack");
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(".\\script\\scr_vm.cpp", 1860, 0, "%s", "fs.top <= scrVmPub.maxstack");
            v54 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            ::pos.top->u.intValue = v54;
        object:
            ::pos.top->type = 1;
            AddRefToObject(::pos.top->u.stringValue);
            continue;
        case 131:
            if (::pos.top < scrVmPub.stack)
                MyAssertHandler(".\\script\\scr_vm.cpp", 1865, 0, "%s", "fs.top >= scrVmPub.stack");
            if (++::pos.top > scrVmPub.maxstack)
                MyAssertHandler(".\\script\\scr_vm.cpp", 1867, 0, "%s", "fs.top <= scrVmPub.maxstack");
            v53 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            v35 = FindVariable(::pos.localId, v53);
            *::pos.top = Scr_EvalVariable(v35);
            continue;
        case 132:
            fieldValueIndex = 0;
            v52 = *(_WORD*)::pos.pos;
            ::pos.pos += 2;
            fieldValueId = FindVariable(::pos.localId, v52);
            if (fieldValueId)
                continue;
            Scr_Error("cannot create a new local variable in the debugger");
            goto $LN125;
        case 133:
        $LN125:
            profileIndex = *::pos.pos++;
            profileBit = 1 << profileIndex;
            if (((1 << profileIndex) & *profileEnablePos) == 0)
            {
                *profileEnablePos |= profileBit;
                Profile_BeginScript(profileIndex);
            }
            continue;
        case 134:
            profileIndex = *::pos.pos++;
            profileBit = 1 << profileIndex;
            if (((1 << profileIndex) & *profileEnablePos) != 0)
            {
                *profileEnablePos &= ~profileBit;
                Profile_EndScript(profileIndex);
            }
            continue;
        case 135:
            if (!scrVarPub.developer)
                continue;
            opcode = Scr_HitBreakpoint(::pos.top, ::pos.pos, ::pos.localId, 0);
            goto interrupt_return;
        case 136:
            opcode = Scr_HitAssignmentBreakpoint(::pos.top, ::pos.pos, ::pos.localId, 0);
            goto interrupt_return;
        case 137:
            opcode = Scr_HitAssignmentBreakpoint(::pos.top, ::pos.pos, ::pos.localId, 1);
            goto interrupt_return;
        default:
            scrVmPub.terminal_error = 1;
            v36 = va("CODE ERROR: unknown opcode %d", opcode);
            RuntimeError(::pos.pos, 0, v36, 0);
            continue;
        }
    }
}

VariableValueInternal_u __cdecl GetDummyObject()
{
    ClearVariableValue(scrVarPub.tempVariable);
    return GetObject(scrVarPub.tempVariable);
}

unsigned int __cdecl GetDummyFieldValue()
{
    ClearVariableValue(scrVarPub.tempVariable);
    return scrVarPub.tempVariable;
}

void VM_PrintJumpHistory()
{
    const char* pos; // [esp+0h] [ebp-8h]
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
        Scr_PrintPrevCodePos(23, pos, 0);
    } while (index != scrVmDebugPub.jumpbackHistoryIndex);
    Com_Printf(23, "********************************\n");
}

VariableStackBuffer* __cdecl VM_ArchiveStack()
{
    char* stackValue; // [esp+0h] [ebp-18h]
    VariableValue* top; // [esp+4h] [ebp-14h]
    char* buf; // [esp+8h] [ebp-10h]
    char* bufa; // [esp+8h] [ebp-10h]
    unsigned int localId; // [esp+Ch] [ebp-Ch]
    int size; // [esp+10h] [ebp-8h]
    int bufLen; // [esp+14h] [ebp-4h]

    top = pos.top;
    size = pos.top - pos.startTop;
    if (size != (unsigned __int16)size)
        MyAssertHandler(".\\script\\scr_vm.cpp", 2768, 0, "%s", "size == (unsigned short)size");
    bufLen = 5 * size + 11;
    if (bufLen != (unsigned __int16)bufLen)
        MyAssertHandler(".\\script\\scr_vm.cpp", 2770, 0, "%s", "bufLen == (unsigned short)bufLen");
    stackValue = MT_Alloc(bufLen, 1);
    ++scrVarPub.numScriptThreads;
    localId = pos.localId;
    *((_WORD*)stackValue + 4) = pos.localId;
    *((_WORD*)stackValue + 2) = size;
    *((_WORD*)stackValue + 3) = bufLen;
    *(_DWORD*)stackValue = pos.pos;
    stackValue[10] = scrVarPub.time;
    scrVmPub.localVars -= pos.localVarCount;
    buf = &stackValue[5 * size + 11];
    while (size)
    {
        bufa = buf - 4;
        if (top->type == 7)
        {
            --scrVmPub.function_count;
            --scrVmPub.function_frame;
            *(_DWORD*)bufa = scrVmPub.function_frame->fs.pos;
            scrVmPub.localVars -= scrVmPub.function_frame->fs.localVarCount;
            localId = GetParentLocalId(localId);
        }
        else
        {
            *(_DWORD*)bufa = top->u.intValue;
        }
        buf = bufa - 1;
        if (top->type >= 0x100u)
            MyAssertHandler(".\\script\\scr_vm.cpp", 2805, 0, "%s", "top->type >= 0 && top->type < (1 << 8)");
        *buf = top->type;
        --top;
        --size;
    }
    --scrVmPub.function_count;
    --scrVmPub.function_frame;
    AddRefToObject(localId);
    pos.localId = localId;
    return (VariableStackBuffer*)stackValue;
}

unsigned __int16 __cdecl Scr_ExecEntThreadNum(
    unsigned int entnum,
    unsigned int classnum,
    int handle,
    unsigned int paramcount)
{
    unsigned int v4; // eax
    const char* pos; // [esp+34h] [ebp-10h]
    const char* varUsagePos; // [esp+38h] [ebp-Ch]
    unsigned int objId; // [esp+3Ch] [ebp-8h]
    unsigned int id; // [esp+40h] [ebp-4h]

    pos = &scrVarPub.programBuffer[handle];
    if (!scrVmPub.function_count)
    {
        if ((int*)scrVmPub.localVars != &scrVmGlob.starttime)
            MyAssertHandler(".\\script\\scr_vm.cpp", 4087, 0, "%s", "scrVmPub.localVars == scrVmGlob.localVarsStack - 1");
        Profile_Begin(332);
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
    v4 = AllocThread((ObjectInfo_u)objId);
    id = VM_Execute(v4, pos, paramcount);
    scrVarPub.varUsagePos = varUsagePos;
    if (scrVarDebugPub)
        ++scrVarDebugPub->extRefCount[id];
    RemoveRefToValue(scrVmPub.top->type, scrVmPub.top->u);
    scrVmPub.top->type = 0;
    --scrVmPub.top;
    --scrVmPub.inparamcount;
    if (!scrVmPub.function_count)
    {
        Profile_EndInternal(0);
        if ((int*)scrVmPub.localVars != &scrVmGlob.starttime)
            MyAssertHandler(".\\script\\scr_vm.cpp", 4129, 0, "%s", "scrVmPub.localVars == scrVmGlob.localVarsStack - 1");
    }
    return id;
}

void __cdecl Scr_AddExecThread(int handle, unsigned int paramcount)
{
    unsigned int v2; // eax
    unsigned int v3; // eax
    const char* pos; // [esp+30h] [ebp-8h]
    const char* varUsagePos; // [esp+34h] [ebp-4h]

    pos = &scrVarPub.programBuffer[handle];
    if (!scrVmPub.function_count)
    {
        if ((int*)scrVmPub.localVars != &scrVmGlob.starttime)
            MyAssertHandler(".\\script\\scr_vm.cpp", 4149, 0, "%s", "scrVmPub.localVars == scrVmGlob.localVarsStack - 1");
        Profile_Begin(332);
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
    v2 = AllocThread((ObjectInfo_u)scrVarPub.levelId);
    v3 = VM_Execute(v2, pos, paramcount);
    RemoveRefToObject(v3);
    scrVarPub.varUsagePos = varUsagePos;
    ++scrVmPub.outparamcount;
    --scrVmPub.inparamcount;
    if (!scrVmPub.function_count)
    {
        Profile_EndInternal(0);
        if ((int*)scrVmPub.localVars != &scrVmGlob.starttime)
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

void __cdecl Scr_ExecCode(const char* pos, unsigned int localId)
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
        localIda = AllocThread((ObjectInfo_u)scrVarPub.levelId);
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

    if (sys != 1)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4383, 0, "%s", "sys == SCR_SYS_GAME");
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
            parentId.intValue = (int)GetVariableValueAddress(idb)->u;
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

VariableUnion __cdecl Scr_GetInt(unsigned int index)
{
    const char* v2; // eax
    const char* v3; // eax
    VariableValue* value; // [esp+0h] [ebp-4h]

    if (index < scrVmPub.outparamcount)
    {
        value = &scrVmPub.top[-index];
        if (value->type == 6)
            return value->u;
        scrVarPub.error_index = index + 1;
        v2 = va("type %s is not an int", var_typename[value->type]);
        Scr_Error(v2);
    }
    v3 = va("parameter %d does not exist", index + 1);
    Scr_Error(v3);
    return 0;
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
        value = &scrVmPub.top[-index];
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
        value->type = 0;
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

    if (!scrVarPub.error_message)
        MyAssertHandler(".\\script\\scr_vm.cpp", 215, 0, "%s", "scrVarPub.error_message");
    result = scrVarPub.evaluate;
    if (!scrVarPub.evaluate && !scrCompilePub.script_loading)
    {
        if (scrVmPub.function_count || scrVmPub.debugCode)
        {
            Com_PrintMessage(6, "throwing script exception: ", 0);
            Com_PrintMessage(6, (char*)scrVarPub.error_message, 0);
            Com_PrintMessage(6, "\n", 0);
            if ((unsigned int)g_script_error_level >= 0x21)
                MyAssertHandler(
                    ".\\script\\scr_vm.cpp",
                    252,
                    0,
                    "g_script_error_level doesn't index ARRAY_COUNT( g_script_error )\n\t%i not in [0, %i)",
                    g_script_error_level,
                    33);
            longjmp(g_script_error[g_script_error_level], -1);
        }
    error_2:
        Sys_Error("%s", scrVarPub.error_message);
    }
    if (scrVmPub.terminal_error)
        goto error_2;
    return result;
}

double __cdecl Scr_GetFloat(unsigned int index)
{
    const char* v2; // eax
    const char* v3; // eax
    VariableValue* value; // [esp+0h] [ebp-4h]

    if (index < scrVmPub.outparamcount)
    {
        value = &scrVmPub.top[-index];
        if (value->type == 5)
            return value->u.floatValue;
        if (value->type == 6)
            return (double)value->u.intValue;
        scrVarPub.error_index = index + 1;
        v2 = va("type %s is not a float", var_typename[value->type]);
        Scr_Error(v2);
    }
    v3 = va("parameter %d does not exist", index + 1);
    Scr_Error(v3);
    return 0.0;
}

VariableUnion __cdecl Scr_GetConstString(unsigned int index)
{
    const char* v2; // eax
    VariableValue* value; // [esp+0h] [ebp-4h]

    if (index >= scrVmPub.outparamcount)
        goto LABEL_7;
    value = &scrVmPub.top[-index];
    if (!Scr_CastString(value))
    {
        scrVarPub.error_index = index + 1;
        Scr_ErrorInternal();
    LABEL_7:
        v2 = va("parameter %d does not exist", index + 1);
        Scr_Error(v2);
        return 0;
    }
    if (value->type != 2)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4605, 0, "%s", "value->type == VAR_STRING");
    SL_CheckExists(value->u.intValue);
    return value->u;
}

VariableUnion __cdecl Scr_GetConstLowercaseString(unsigned int index)
{
    HashEntry_unnamed_type_u v1; // eax
    const char* v3; // eax
    char* v4; // [esp+0h] [ebp-2018h]
    VariableUnion stringValue; // [esp+4h] [ebp-2014h]
    char str[8196]; // [esp+8h] [ebp-2010h] BYREF
    int i; // [esp+2010h] [ebp-8h]
    VariableValue* value; // [esp+2014h] [ebp-4h]

    if (index >= scrVmPub.outparamcount)
        goto LABEL_12;
    value = &scrVmPub.top[-index];
    if (!Scr_CastString(value))
    {
        scrVarPub.error_index = index + 1;
        Scr_ErrorInternal();
    LABEL_12:
        v3 = va("parameter %d does not exist", index + 1);
        Scr_Error(v3);
        return 0;
    }
    if (value->type != 2)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4635, 0, "%s", "value->type == VAR_STRING");
    stringValue.intValue = (int)value->u;
    v4 = _SL_ConvertToString(value->u.intValue);
    for (i = 0; ; ++i)
    {
        str[i] = tolower(v4[i]);
        if (!v4[i])
            break;
    }
    if (value->type != 2)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4646, 0, "%s", "value->type == VAR_STRING");
    v1.prev = SL_GetString(str, 0).prev;
    value->u.stringValue = v1.prev;
    _SL_RemoveRefToString(stringValue.stringValue);
    SL_CheckExists(value->u.intValue);
    return value->u;
}

char* __cdecl Scr_GetString(unsigned int index)
{
    VariableUnion v1; // eax

    v1.intValue = Scr_GetConstString(index).intValue;
    return _SL_ConvertToString(v1.stringValue);
}

VariableUnion __cdecl Scr_GetConstStringIncludeNull(unsigned int index)
{
    if (index >= scrVmPub.outparamcount || scrVmPub.top[-index].type)
        return Scr_GetConstString(index);
    return 0;
}

char* __cdecl Scr_GetDebugString(unsigned int index)
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
        value = &scrVmPub.top[-index];
        Scr_CastDebugString(value);
        if (value->type != 2)
            MyAssertHandler(".\\script\\scr_vm.cpp", 4693, 0, "%s", "value->type == VAR_STRING");
        return _SL_ConvertToString(value->u.intValue);
    }
}

VariableUnion __cdecl Scr_GetConstIString(unsigned int index)
{
    const char* v2; // eax
    const char* v3; // eax
    VariableValue* value; // [esp+0h] [ebp-4h]

    if (index < scrVmPub.outparamcount)
    {
        value = &scrVmPub.top[-index];
        if (value->type == 3)
            return value->u;
        scrVarPub.error_index = index + 1;
        v2 = va("type %s is not a localized string", var_typename[value->type]);
        Scr_Error(v2);
    }
    v3 = va("parameter %d does not exist", index + 1);
    Scr_Error(v3);
    return 0;
}

char* __cdecl Scr_GetIString(unsigned int index)
{
    VariableUnion v1; // eax

    v1.intValue = Scr_GetConstIString(index).intValue;
    return _SL_ConvertToString(v1.stringValue);
}

void __cdecl Scr_GetVector(unsigned int index, float* vectorValue)
{
    const char* v2; // eax
    const char* v3; // eax
    float* intValue; // [esp+0h] [ebp-8h]
    VariableValue* value; // [esp+4h] [ebp-4h]

    if (index < scrVmPub.outparamcount)
    {
        value = &scrVmPub.top[-index];
        if (value->type == 4)
        {
            intValue = (float*)value->u.intValue;
            *vectorValue = *(float*)value->u.intValue;
            vectorValue[1] = intValue[1];
            vectorValue[2] = intValue[2];
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
    const char* v3; // eax
    const char* v4; // eax
    const char* v5; // eax
    VariableValue* value; // [esp+8h] [ebp-8h]
    unsigned int id; // [esp+Ch] [ebp-4h]

    if (index < scrVmPub.outparamcount)
    {
        value = &scrVmPub.top[-index];
        if (value->type == 1)
        {
            id = value->u.intValue;
            if (GetObjectType(value->u.intValue) == 20)
                return Scr_GetEntityIdRef(id);
            scrVarPub.error_index = index + 1;
            ObjectType = GetObjectType(id);
            v3 = va("type %s is not an entity", var_typename[ObjectType]);
            Scr_Error(v3);
        }
        scrVarPub.error_index = index + 1;
        v4 = va("type %s is not an entity", var_typename[value->type]);
        Scr_Error(v4);
    }
    v5 = va("parameter %d does not exist", index + 1);
    Scr_Error(v5);
    return 0;
}

VariableUnion __cdecl Scr_GetObject(unsigned int index)
{
    const char* v2; // eax
    const char* v3; // eax
    VariableValue* value; // [esp+0h] [ebp-4h]

    if (index < scrVmPub.outparamcount)
    {
        value = &scrVmPub.top[-index];
        if (value->type == 1)
            return value->u;
        scrVarPub.error_index = index + 1;
        v2 = va("type %s is not an object", var_typename[value->type]);
        Scr_Error(v2);
    }
    v3 = va("parameter %d does not exist", index + 1);
    Scr_Error(v3);
    return 0;
}

int __cdecl Scr_GetType(unsigned int index)
{
    const char* v2; // eax

    if (index < scrVmPub.outparamcount)
        return scrVmPub.top[-index].type;
    v2 = va("parameter %d does not exist", index + 1);
    Scr_Error(v2);
    return 0;
}

const char* __cdecl Scr_GetTypeName(unsigned int index)
{
    const char* v2; // eax

    if (index < scrVmPub.outparamcount)
        return var_typename[scrVmPub.top[-index].type];
    v2 = va("parameter %d does not exist", index + 1);
    Scr_Error(v2);
    return 0;
}

unsigned int __cdecl Scr_GetPointerType(unsigned int index)
{
    const char* v2; // eax
    const char* v3; // eax

    if (index < scrVmPub.outparamcount)
    {
        if (scrVmPub.top[-index].type == 1)
            return GetObjectType(scrVmPub.top[-index].u.stringValue);
        v2 = va("type %s is not an object", var_typename[scrVmPub.top[-index].type]);
        Scr_Error(v2);
    }
    v3 = va("parameter %d does not exist", index + 1);
    Scr_Error(v3);
    return 0;
}

unsigned int __cdecl Scr_GetNumParam()
{
    return scrVmPub.outparamcount;
}

void __cdecl Scr_AddBool(unsigned int value)
{
    if (value > 1)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4850, 0, "%s", "value == 0 || value == 1");
    IncInParam();
    scrVmPub.top->type = 6;
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
    scrVmPub.top->type = 6;
    scrVmPub.top->u.intValue = value;
}

void __cdecl Scr_AddFloat(VariableUnion value)
{
    IncInParam();
    scrVmPub.top->type = 5;
    *(float*)&scrVmPub.top->u.intValue = value.floatValue;
}

void __cdecl Scr_AddAnim(scr_anim_s value)
{
    IncInParam();
    scrVmPub.top->type = 11;
    scrVmPub.top->u.intValue = (int)value.linkPointer;
}

void __cdecl Scr_AddUndefined()
{
    IncInParam();
    scrVmPub.top->type = 0;
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
    scrVmPub.top->type = 1;
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
    scrVmPub.top->type = 2;
    scrVmPub.top->u.intValue = SL_GetString(value, 0);
}

void __cdecl Scr_AddIString(const char* value)
{
    if (!value)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4956, 0, "%s", "value");
    IncInParam();
    scrVmPub.top->type = 3;
    scrVmPub.top->u.intValue = SL_GetString(value, 0);
}

void __cdecl Scr_AddConstString(unsigned int value)
{
    if (!value)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4966, 0, "%s", "value");
    IncInParam();
    scrVmPub.top->type = 2;
    scrVmPub.top->u.intValue = value;
    SL_AddRefToString(value);
}

void __cdecl Scr_AddVector(const float* value)
{
    IncInParam();
    scrVmPub.top->type = 4;
    scrVmPub.top->u.intValue = (int)Scr_AllocVector(value);
}

void __cdecl Scr_MakeArray()
{
    IncInParam();
    scrVmPub.top->type = 1;
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
    if (!scrVmPub.inparamcount)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5002, 0, "%s", "scrVmPub.inparamcount");
    --scrVmPub.top;
    --scrVmPub.inparamcount;
    if (scrVmPub.top->type != 1)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5005, 0, "%s", "scrVmPub.top->type == VAR_POINTER");
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
        longjmp(g_script_error[g_script_error_level], -1);
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
    if (scrVmPub.inparamcount)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5125, 0, "%s", "!scrVmPub.inparamcount");
    if (scrVmPub.outparamcount)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5126, 0, "%s", "!scrVmPub.outparamcount");
    scrVmPub.top = (VariableValue*)((char*)&scrVmGlob - 8);
    scrVmGlob.eval_stack[0].type = 0;
    Scr_GetObjectField(classnum, entnum, offset);
    if (scrVmPub.inparamcount > 1)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5132, 0, "%s", "!scrVmPub.inparamcount || scrVmPub.inparamcount == 1");
    if (scrVmPub.outparamcount)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5133, 0, "%s", "!scrVmPub.outparamcount");
    if (&scrVmPub.top[-scrVmPub.inparamcount] != (VariableValue*)((char*)&scrVmGlob - 8))
        MyAssertHandler(
            ".\\script\\scr_vm.cpp",
            5134,
            0,
            "%s",
            "scrVmPub.top - scrVmPub.inparamcount == scrVmGlob.eval_stack - 1");
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
    scrVarPub.time &= (unsigned int)&clients[0].parseClients[238].attachTagIndex[4] + 3;
    scrVmPub.showError = scrVmPub.abort_on_error;
}

void __cdecl Scr_RunCurrentThreads()
{
    Profile_Begin(332);
    if (scrVmPub.function_count)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5220, 0, "%s", "!scrVmPub.function_count");
    if (scrVarPub.error_message)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5221, 0, "%s", "!scrVarPub.error_message");
    if (scrVarPub.error_index)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5222, 0, "%s", "!scrVarPub.error_index");
    if (scrVmPub.outparamcount)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5223, 0, "%s", "!scrVmPub.outparamcount");
    if (scrVmPub.inparamcount)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5224, 0, "%s", "!scrVmPub.inparamcount");
    if (scrVmPub.top != scrVmPub.stack)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5225, 0, "%s", "scrVmPub.top == scrVmPub.stack");
    VM_SetTime();
    Profile_EndInternal(0);
}

void VM_SetTime()
{
    DWORD v0; // eax
    VariableValueInternal_u Object; // eax
    unsigned int id; // [esp+0h] [ebp-4h]

    if ((scrVarPub.time & 0xFF000000) != 0)
        MyAssertHandler(".\\script\\scr_vm.cpp", 4299, 0, "%s", "!(scrVarPub.time & ~VAR_NAME_LOW_MASK)");
    if (scrVarPub.timeArrayId)
    {
        id = FindVariable(scrVarPub.timeArrayId, scrVarPub.time);
        if (id)
        {
            if (!logScriptTimes)
                MyAssertHandler(".\\script\\scr_vm.cpp", 4307, 0, "%s", "logScriptTimes");
            if (logScriptTimes->current.enabled)
            {
                v0 = Sys_Milliseconds();
                Com_Printf(23, "SET TIME: %d\n", v0);
            }
            Object = FindObject(id);
            VM_Resume(Object.u.stringValue);
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

    if (scrVmPub.top != scrVmPub.stack)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3825, 0, "%s", "scrVmPub.top == scrVmPub.stack");
    Scr_ResetTimeout();
    if (!timeId)
        MyAssertHandler(".\\script\\scr_vm.cpp", 3833, 0, "%s", "timeId");
    AddRefToObject(timeId);
    ::pos.startTop = scrVmPub.stack;
    thread_count = 0;
    while (1)
    {
        if (scrVarPub.error_message)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3841, 0, "%s", "!scrVarPub.error_message");
        if (scrVarPub.error_index)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3842, 0, "%s", "!scrVarPub.error_index");
        if (scrVmPub.outparamcount)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3843, 0, "%s", "!scrVmPub.outparamcount");
        if (scrVmPub.inparamcount)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3844, 0, "%s", "!scrVmPub.inparamcount");
        if (scrVmPub.function_count)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3845, 0, "%s", "!scrVmPub.function_count");
        if ((int*)scrVmPub.localVars != &scrVmGlob.starttime)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3846, 0, "%s", "scrVmPub.localVars == scrVmGlob.localVarsStack - 1");
        if (::pos.startTop != scrVmPub.stack)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3847, 0, "%s", "fs.startTop == &scrVmPub.stack[0]");
        if (thread_count)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3848, 0, "%s", "!thread_count");
        stackId = FindFirstSibling(timeId);
        if (!stackId)
            break;
        startLocalId = GetVariableKeyObject(stackId);
        if (!startLocalId)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3855, 0, "%s", "startLocalId");
        if (GetValueType(stackId) != 10)
            MyAssertHandler(".\\script\\scr_vm.cpp", 3857, 0, "%s", "GetValueType( stackId ) == VAR_STACK");
        stackValue = (VariableStackBuffer*)GetVariableValueAddress(stackId)->u.intValue;
        RemoveObjectVariable(timeId, startLocalId);
        VM_UnarchiveStack(startLocalId, stackValue);
        if (scrVarPub.bScriptProfile)
        {
            scrVmDebugPub.builtInTime = 0;
            time = __rdtsc();
            pos = ::pos.pos;
            v2 = VM_Execute_0();
            RemoveRefToObject(v2);
            RemoveRefToValue(scrVmPub.stack[1].type, scrVmPub.stack[1].u);
            Scr_AddProfileTime(pos, __rdtsc() - time, scrVmDebugPub.builtInTime);
        }
        else
        {
            v1 = VM_Execute_0();
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
    const char** bufa; // [esp+4h] [ebp-10h]
    unsigned int localId; // [esp+8h] [ebp-Ch]
    int function_count; // [esp+Ch] [ebp-8h]
    int size; // [esp+10h] [ebp-4h]

    if (scrVmPub.function_count)
        MyAssertHandler(".\\script\\scr_vm.cpp", 2846, 0, "%s", "!scrVmPub.function_count");
    if (!stackValue->pos)
        MyAssertHandler(".\\script\\scr_vm.cpp", 2847, 0, "%s", "stackValue->pos");
    if (pos.startTop != scrVmPub.stack)
        MyAssertHandler(".\\script\\scr_vm.cpp", 2848, 0, "%s", "fs.startTop == &scrVmPub.stack[0]");
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
        top->type = (unsigned __int8)*buf;
        bufa = (const char**)(buf + 1);
        if (top->type == 7)
        {
            if (scrVmPub.function_count >= 32)
                MyAssertHandler(".\\script\\scr_vm.cpp", 2868, 0, "%s", "scrVmPub.function_count < MAX_VM_STACK_DEPTH");
            scrVmPub.function_frame->fs.pos = *bufa;
            ++scrVmPub.function_count;
            ++scrVmPub.function_frame;
        }
        else
        {
            top->u.intValue = (int)*bufa;
        }
        buf = (char*)(bufa + 1);
    }
    pos.pos = stackValue->pos;
    pos.top = top;
    localId = stackValue->localId;
    pos.localId = localId;
    Scr_ClearWaitTime(startLocalId);
    if (scrVmPub.function_count >= 32)
        MyAssertHandler(".\\script\\scr_vm.cpp", 2888, 0, "%s", "scrVmPub.function_count < MAX_VM_STACK_DEPTH");
    function_count = scrVmPub.function_count;
    while (1)
    {
        scrVmPub.function_frame_start[function_count--].fs.localId = localId;
        if (!function_count)
            break;
        localId = GetParentLocalId(localId);
    }
    while (++function_count != scrVmPub.function_count)
        scrVmPub.stack[3 * function_count - 95].u.intValue = Scr_AddLocalVars(scrVmPub.function_frame_start[function_count].fs.localId);
    pos.localVarCount = Scr_AddLocalVars(pos.localId);
    if (stackValue->time != LOBYTE(scrVarPub.time))
        Scr_ResetTimeout();
    --scrVarPub.numScriptThreads;
    MT_Free(stackValue, stackValue->bufLen);
    if (scrVmPub.stack[0].type != 7)
        MyAssertHandler(".\\script\\scr_vm.cpp", 2917, 0, "%s", "scrVmPub.stack[0].type == VAR_CODEPOS");
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
    if (!logScriptTimes)
        MyAssertHandler(".\\script\\scr_vm.cpp", 5247, 0, "%s", "logScriptTimes");
    if (logScriptTimes->current.enabled)
    {
        v0 = Sys_Milliseconds();
        Com_Printf(23, "RESET TIME: %d\n", v0);
    }
    memset((unsigned __int8*)scrVmDebugPub.jumpbackHistory, 0, sizeof(scrVmDebugPub.jumpbackHistory));
}

BOOL __cdecl Scr_IsStackClear()
{
    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_vm.cpp", 5262, 0, "%s", "!Sys_IsRemoteDebugClient()");
    return scrVmPub.top == scrVmPub.stack;
}

void __cdecl Scr_StackClear()
{
    scrVmPub.top = scrVmPub.stack;
}

void __cdecl Scr_ProfileUpdate()
{
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
}

void __cdecl Scr_ProfileBuiltinUpdate()
{
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
}

void __cdecl Scr_DoProfile(float minTime)
{
    scrVarPub.bScriptProfile = 1;
    scrVarPub.scriptProfileMinTime = minTime;
}

void __cdecl Scr_DoProfileBuiltin(float minTime)
{
    scrVarPub.bScriptProfileBuiltin = 1;
    scrVarPub.scriptProfileBuiltinMinTime = minTime;
}

char __cdecl Scr_PrintProfileBuiltinTimes(float minTime)
{
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
        qsort(order, scrCompilePub.func_table_size, 4u, Scr_BuiltinCompare);
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
}

int __cdecl Scr_BuiltinCompare(_DWORD* a, _DWORD* b)
{
    return scrVmDebugPub.func_table[*a].prof - scrVmDebugPub.func_table[*b].prof;
}

