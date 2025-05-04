#include "scr_main.h"
#include "scr_stringlist.h"

#include <universal/assertive.h>
#include <universal/q_shared.h>
#include "scr_compiler.h"
#include <win32/win_net_debug.h>
#include "scr_vm.h"
#include <universal/profile.h>
#include "scr_evaluate.h"
#include "scr_animtree.h"
#include "scr_parsetree.h"
#include "scr_yacc.h"

#undef GetObject

bool Scr_IsInOpcodeMemory(char const* pos)
{
    iassert(scrVarPub.programBuffer);
    iassert(pos);
    return pos - scrVarPub.programBuffer < scrCompilePub.programLen;
}

bool Scr_IsIdentifier(char const* token)
{
    while (*token)
    {
        if (!I_iscsym(*token))
            return 0;
        ++token;
    }
    return 1;
}

unsigned int SL_TransferToCanonicalString(unsigned int stringValue)
{
	iassert(stringValue);
	SL_TransferRefToUser(stringValue, 2u);

	if (scrCompilePub.canonicalStrings[stringValue])
		return scrCompilePub.canonicalStrings[stringValue];

	scrCompilePub.canonicalStrings[stringValue] = ++scrVarPub.canonicalStrCount;

	return scrVarPub.canonicalStrCount;
}

unsigned int __cdecl SL_GetCanonicalString(char* str)
{
    unsigned int v1; // eax
    unsigned int v3; // eax

    v1 = SL_FindString(str);
    if (scrCompilePub.canonicalStrings[v1])
        return scrCompilePub.canonicalStrings[v1];
    v3 = SL_GetString_(str, 0, 16);
    return SL_TransferToCanonicalString(v3);
}

void SL_BeginLoadScripts()
{
    memset((unsigned __int8*)scrCompilePub.canonicalStrings, 0, sizeof(scrCompilePub.canonicalStrings));
    scrVarPub.canonicalStrCount = 0;
}

int __cdecl Scr_ScanFile(char *buf, int max_size)
{
    char c; // [esp+3h] [ebp-5h]
    int n; // [esp+4h] [ebp-4h]

    c = 42;
    for (n = 0; n < max_size; ++n)
    {
        c = *scrCompilePub.in_ptr++;
        if (!c || c == 10)
            break;
        buf[n] = c;
    }
    if (c == 10)
    {
        buf[n++] = c;
    }
    else if (!c)
    {
        if (scrCompilePub.parseBuf)
        {
            scrCompilePub.in_ptr = scrCompilePub.parseBuf;
            scrCompilePub.parseBuf = 0;
        }
        else
        {
            --scrCompilePub.in_ptr;
        }
    }
    return n;
}

void __cdecl Scr_SetLoadedImpureScript(bool loadedImpureScript)
{
    g_loadedImpureScript = loadedImpureScript;
}

void __cdecl Scr_BeginLoadScripts()
{
    scrVarPub.varUsagePos = "<script compile variable>";
    if (scrCompilePub.script_loading)
        MyAssertHandler(".\\script\\scr_main.cpp", 156, 0, "%s", "!scrCompilePub.script_loading");
    scrCompilePub.script_loading = 1;
    Scr_InitOpcodeLookup();
    Scr_InitDebuggerMain();
    if (scrCompilePub.loadedscripts)
        MyAssertHandler(".\\script\\scr_main.cpp", 164, 0, "%s", "!scrCompilePub.loadedscripts");
    scrCompilePub.loadedscripts = Scr_AllocArray();
    if (scrVarDebugPub)
        ++scrVarDebugPub->extRefCount[scrCompilePub.loadedscripts];
    if (!Sys_IsRemoteDebugClient())
    {
        memset(profileScript.profileScriptNames, 0, sizeof(profileScript.profileScriptNames));
        scrVmPub.showError = scrVarPub.developer;
        if (scrCompilePub.scripts)
            MyAssertHandler(".\\script\\scr_main.cpp", 184, 0, "%s", "!scrCompilePub.scripts");
        scrCompilePub.scripts = Scr_AllocArray();
        if (scrVarDebugPub)
            ++scrVarDebugPub->extRefCount[scrCompilePub.scripts];
        if (scrCompilePub.builtinFunc)
            MyAssertHandler(".\\script\\scr_main.cpp", 191, 0, "%s", "!scrCompilePub.builtinFunc");
        scrCompilePub.builtinFunc = Scr_AllocArray();
        if (scrVarDebugPub)
            ++scrVarDebugPub->extRefCount[scrCompilePub.builtinFunc];
        if (scrCompilePub.builtinMeth)
            MyAssertHandler(".\\script\\scr_main.cpp", 198, 0, "%s", "!scrCompilePub.builtinMeth");
        scrCompilePub.builtinMeth = Scr_AllocArray();
        if (scrVarDebugPub)
            ++scrVarDebugPub->extRefCount[scrCompilePub.builtinMeth];
        scrVarPub.programHunkUser = Hunk_UserCreate(0x100000, "Scr_BeginLoadScripts", 1, 0, 7);
        TempMemoryReset(scrVarPub.programHunkUser);
        scrVarPub.programBuffer = TempMalloc(0);
        if (((int)scrVarPub.programBuffer & 0x1F) != 0)
            MyAssertHandler(
                ".\\script\\scr_main.cpp",
                209,
                0,
                "%s\n\t((int)scrVarPub.programBuffer) = %i",
                "(!((int)scrVarPub.programBuffer & 31))",
                scrVarPub.programBuffer);
        scrCompilePub.programLen = 0;
        scrVarPub.endScriptBuffer = 0;
        SL_BeginLoadScripts();
        Scr_InitEvaluate();
        scrVarPub.fieldBuffer = 0;
        scrCompilePub.value_count = 0;
        Scr_ClearErrorMessage();
        scrCompilePub.func_table_size = 0;
        scrAnimPub.animTreeNames = 0;
        Scr_SetLoadedImpureScript(0);
        Scr_BeginLoadAnimTrees(1);
    }
}

void __cdecl Scr_EndLoadScripts()
{
    if (!Sys_IsRemoteDebugClient())
    {
        Scr_EndLoadEvaluate();
        CL_ResetStats_f();
        SL_ShutdownSystem(2u);
    }
    Scr_InitDebugger();
    scrCompilePub.script_loading = 0;
    if (!scrCompilePub.loadedscripts)
        MyAssertHandler(".\\script\\scr_main.cpp", 415, 0, "%s", "scrCompilePub.loadedscripts");
    ClearObject(scrCompilePub.loadedscripts);
    if (scrVarDebugPub)
        --scrVarDebugPub->extRefCount[scrCompilePub.loadedscripts];
    RemoveRefToObject(scrCompilePub.loadedscripts);
    scrCompilePub.loadedscripts = 0;
    if (!Sys_IsRemoteDebugClient())
    {
        if (!scrCompilePub.scripts)
            MyAssertHandler(".\\script\\scr_main.cpp", 429, 0, "%s", "scrCompilePub.scripts");
        ClearObject(scrCompilePub.scripts);
        if (scrVarDebugPub)
            --scrVarDebugPub->extRefCount[scrCompilePub.scripts];
        RemoveRefToObject(scrCompilePub.scripts);
        scrCompilePub.scripts = 0;
        if (!scrCompilePub.builtinFunc)
            MyAssertHandler(".\\script\\scr_main.cpp", 438, 0, "%s", "scrCompilePub.builtinFunc");
        ClearObject(scrCompilePub.builtinFunc);
        if (scrVarDebugPub)
            --scrVarDebugPub->extRefCount[scrCompilePub.builtinFunc];
        RemoveRefToObject(scrCompilePub.builtinFunc);
        scrCompilePub.builtinFunc = 0;
        if (!scrCompilePub.builtinMeth)
            MyAssertHandler(".\\script\\scr_main.cpp", 447, 0, "%s", "scrCompilePub.builtinMeth");
        ClearObject(scrCompilePub.builtinMeth);
        if (scrVarDebugPub)
            --scrVarDebugPub->extRefCount[scrCompilePub.builtinMeth];
        RemoveRefToObject(scrCompilePub.builtinMeth);
        scrCompilePub.builtinMeth = 0;
    }
}

void __cdecl Scr_BeginLoadAnimTrees(int user)
{
    scrVarPub.varUsagePos = "<script compile variable>";
    if (scrAnimPub.animtree_loading)
        MyAssertHandler(".\\script\\scr_main.cpp", 261, 0, "%s", "!scrAnimPub.animtree_loading");
    scrAnimPub.animtree_loading = 1;
    scrAnimPub.xanim_num[user] = 0;
    scrAnimPub.xanim_lookup[user][0].anims = 0;
    if (scrAnimPub.animtrees)
        MyAssertHandler(".\\script\\scr_main.cpp", 267, 0, "%s", "!scrAnimPub.animtrees");
    scrAnimPub.animtrees = Scr_AllocArray();
    if (scrVarDebugPub)
        ++scrVarDebugPub->extRefCount[scrAnimPub.animtrees];
    scrAnimPub.animtree_node = 0;
    scrCompilePub.developer_statement = 0;
}

unsigned int __cdecl Scr_LoadScriptInternal(const char *filename, PrecacheEntry *entries, int entriesCount)
{
    char *v4; // eax
    char *v5; // eax
    unsigned int Variable; // eax
    char *v7; // [esp-8h] [ebp-84h]
    VariableValueInternal_u Object; // [esp+0h] [ebp-7Ch]
    char extFilename[64]; // [esp+14h] [ebp-68h] BYREF
    unsigned int filePtr; // [esp+58h] [ebp-24h]
    char *sourceBuffer; // [esp+5Ch] [ebp-20h]
    const char *oldFilename; // [esp+60h] [ebp-1Ch]
    unsigned int name; // [esp+64h] [ebp-18h]
    unsigned int oldAnimTreeNames; // [esp+68h] [ebp-14h]
    const char *oldSourceBuf; // [esp+6Ch] [ebp-10h]
    unsigned int scriptId; // [esp+70h] [ebp-Ch]
    sval_u parseData; // [esp+74h] [ebp-8h] BYREF
    unsigned int fileId; // [esp+78h] [ebp-4h]

    if (!scrCompilePub.script_loading)
        MyAssertHandler(".\\script\\scr_main.cpp", 322, 0, "%s", "scrCompilePub.script_loading");
    if (strlen(filename) >= 0x40)
        MyAssertHandler(".\\script\\scr_main.cpp", 323, 0, "%s", "strlen( filename ) < MAX_QPATH");
    Hunk_CheckTempMemoryHighClear();
    name = Scr_CreateCanonicalFilename(filename).prev;
    if (FindVariable(scrCompilePub.loadedscripts, name))
    {
        SL_RemoveRefToString(name);
        filePtr = FindVariable(scrCompilePub.scripts, name);
        if (!filePtr)
            return 0;
        Object = FindObject(filePtr);
        return Object.u.intValue;
    }
    else
    {
        scriptId = GetNewVariable(scrCompilePub.loadedscripts, name);
        SL_RemoveRefToString(name);
        v4 = SL_ConvertToString(name);
        Com_sprintf(extFilename, 0x40u, "%s.gsc", v4);
        oldSourceBuf = scrParserPub.sourceBuf;
        ProfLoad_Begin("Scr_AddSourceBuffer");
        v7 = TempMalloc(0);
        v5 = SL_ConvertToString(name);
        sourceBuffer = Scr_AddSourceBuffer(v5, extFilename, v7, 1);
        ProfLoad_End();
        if (sourceBuffer)
        {
            oldAnimTreeNames = scrAnimPub.animTreeNames;
            scrAnimPub.animTreeNames = 0;
            scrCompilePub.far_function_count = 0;
            Scr_InitAllocNode();
            oldFilename = scrParserPub.scriptfilename;
            scrParserPub.scriptfilename = extFilename;
            scrCompilePub.in_ptr = "+";
            scrCompilePub.parseBuf = sourceBuffer;
            ScriptParse(&parseData, 0);
            Variable = GetVariable(scrCompilePub.scripts, name);
            fileId = GetObject(Variable).u.stringValue;
            ProfLoad_Begin("ScriptCompile");
            ScriptCompile(parseData, fileId, scriptId, entries, entriesCount);
            ProfLoad_End();
            scrParserPub.scriptfilename = oldFilename;
            scrParserPub.sourceBuf = oldSourceBuf;
            scrAnimPub.animTreeNames = oldAnimTreeNames;
            Hunk_CheckTempMemoryHighClear();
            return fileId;
        }
        else
        {
            return 0;
        }
    }
}

unsigned int __cdecl Scr_LoadScript(char *filename)
{
    PrecacheEntry entries; // [esp+0h] [ebp-2000h] BYREF

    return Scr_LoadScriptInternal(filename, &entries, 0);
}

void __cdecl Scr_PostCompileScripts()
{
    if (scrCompilePub.programLen != TempMalloc(0) - scrVarPub.programBuffer)
        MyAssertHandler(
            ".\\script\\scr_main.cpp",
            390,
            0,
            "%s",
            "scrCompilePub.programLen == static_cast< uint >( TempMalloc( 0 ) - scrVarPub.programBuffer )");
    if (scrAnimPub.animTreeNames)
        MyAssertHandler(".\\script\\scr_main.cpp", 391, 0, "%s", "!scrAnimPub.animTreeNames");
}

void __cdecl Scr_PrecacheAnimTrees(void *(__cdecl *Alloc)(int), int user)
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for (i = 1; i <= scrAnimPub.xanim_num[user]; ++i)
        Scr_LoadAnimTreeAtIndex(i, Alloc, user);
}

void __cdecl Scr_EndLoadAnimTrees()
{
    if (!scrAnimPub.animtrees)
        MyAssertHandler(".\\script\\scr_main.cpp", 471, 0, "%s", "scrAnimPub.animtrees");
    ClearObject(scrAnimPub.animtrees);
    if (scrVarDebugPub)
        --scrVarDebugPub->extRefCount[scrAnimPub.animtrees];
    RemoveRefToObject(scrAnimPub.animtrees);
    scrAnimPub.animtrees = 0;
    if (scrAnimPub.animtree_node)
        RemoveRefToObject(scrAnimPub.animtree_node);
    SL_ShutdownSystem(2u);
    if (scrVarPub.programBuffer && !scrVarPub.endScriptBuffer)
        scrVarPub.endScriptBuffer = TempMalloc(0);
    scrAnimPub.animtree_loading = 0;
    scrVarPub.varUsagePos = 0;
}

void __cdecl Scr_FreeScripts(unsigned __int8 sys)
{
    if (sys != 1)
        MyAssertHandler(".\\script\\scr_main.cpp", 498, 0, "%s", "sys == SCR_SYS_GAME");
    Hunk_CheckTempMemoryClear();
    if (scrCompilePub.script_loading)
    {
        scrCompilePub.script_loading = 0;
        Scr_EndLoadScripts();
    }
    if (scrAnimPub.animtree_loading)
    {
        scrAnimPub.animtree_loading = 0;
        Scr_EndLoadAnimTrees();
    }
    Scr_ShutdownDebugger();
    Scr_ShutdownDebuggerMain();
    Scr_ShutdownEvaluate();
    SL_ShutdownSystem(1u);
    Scr_ShutdownOpcodeLookup();
    if (scrVarPub.programHunkUser)
    {
        Hunk_UserDestroy(scrVarPub.programHunkUser);
        scrVarPub.programHunkUser = 0;
    }
    scrVarPub.programBuffer = 0;
    scrCompilePub.programLen = 0;
    scrVarPub.endScriptBuffer = 0;
    scrVarPub.checksum = 0;
}

