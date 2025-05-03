#include "mem_track.h"

#include <universal/assertive.h>
#include <qcommon/qcommon.h>

#include <Windows.h>
#include "threads.h"
#include <xanim/xanim.h>

const char* g_mem_track_filename;

static _RTL_CRITICAL_SECTION g_crit;

static mem_track_t g_mem_track[0x400];
static int g_mem_track_count;
static meminfo_t g_info;
static TempMemInfo g_mallocMemInfoArray[0x400];
static int g_mallocMemInfoCount;
static mem_track_t g_hunk_track[0x80000];
static int g_hunk_track_count;
static mem_track_t g_hunklow_track[0x10000];
static int g_hunklow_track_count;
static TempMemInfo g_tempMemInfoArray[0x400];
static int g_tempMemInfoCount;
static TempMemInfo g_tempHighMemInfoArray[0x400];
static int g_tempHighMemInfoCount;
static TempMemInfo g_combinedMemInfoArray[0x400];
static TempMemInfo g_physicalMemInfoArray[0x400];
static int g_physicalMemInfoCount;
static meminfo_t g_virtualMemInfo;
static int g_malloc_mem_size;
static int g_malloc_mem_high;

static mem_track_node_s* g_head;

static const char* projName[4] =
{
    "exe",
    "cgame",
    "game",
    "renderer"
};

void __cdecl track_static_alloc_internal(void* ptr, int size, const char* name, int type)
{
    mem_track_t* mem_track; // [esp+0h] [ebp-8h]

    EnterCriticalSection(&g_crit);

    if (g_mem_track_count >= 1024)
        MyAssertHandler(".\\qcommon\\mem_track.cpp", 269, 0, "%s", "g_mem_track_count < MAX_MEM_TRACK");

    mem_track = &g_mem_track[g_mem_track_count];
    mem_track->size = size;
    if (!*name)
        MyAssertHandler(".\\qcommon\\mem_track.cpp", 273, 0, "%s", "name[0]");
    mem_track->name = name;
    mem_track->type = type;
    mem_track->usageType = 0;
    mem_track->filename = g_mem_track_filename;
    track_addbasicinfo(&g_info, type, size);
    ++g_mem_track_count;
    LeaveCriticalSection(&g_crit);
}

void __cdecl CG_track_init()
{
    g_mem_track_filename = "cg_memtrack";
    TRACK_g_memtrack();
    g_mem_track_filename = "cg_main";
    TRACK_cg_main();
    g_mem_track_filename = "cg_camerashake";
    TRACK_cg_camerashake();
    g_mem_track_filename = "cg_draw";
    TRACK_cg_draw();
    g_mem_track_filename = "cg_compassfriendlies";
    TRACK_cg_compassfriendlies();
    g_mem_track_filename = "cg_predict";
    TRACK_cg_predict();
    g_mem_track_filename = "CG_CollWorld";
    TRACK_CG_CollWorld();
    g_mem_track_filename = "cg_view";
    TRACK_cg_view();
    g_mem_track_filename = "cg_localents";
    TRACK_cg_localents();
    g_mem_track_filename = "cg_players";
    CL_ResetStats_f();
    g_mem_track_filename = "aim_assist";
    TRACK_aim_assist();
    g_mem_track_filename = "aim_target";
    TRACK_aim_target();
}

void __cdecl TRACK_g_memtrack()
{
    g_mem_track_filename = "";
}


void __cdecl G_track_init()
{
    g_mem_track_filename = "g_memtrack";
    TRACK_g_memtrack();
    g_mem_track_filename = "g_main";
    TRACK_g_main();
    g_mem_track_filename = "g_svcmds";
    CL_ResetStats_f();
    g_mem_track_filename = "g_utils";
    TRACK_g_utils();
    g_mem_track_filename = "bg_misctables";
    TRACK_bg_misctables();
    g_mem_track_filename = "bg_weapons";
    TRACK_bg_weapons();
    g_mem_track_filename = "bg_weapons_load_obj";
    TRACK_bg_weapons_load_obj();
    g_mem_track_filename = "g_combat";
    TRACK_g_combat();
    g_mem_track_filename = "g_scr_main";
    TRACK_g_scr_main();
    g_mem_track_filename = "g_mover";
    TRACK_g_mover();
    g_mem_track_filename = "g_hudelem";
    TRACK_g_hudelem();
    g_mem_track_filename = "bg_animation_mp";
    TRACK_bg_animation_mp();
}

void __cdecl TRACK_memtrack()
{
    track_static_alloc_internal(g_mem_track, (sizeof(mem_track_t) * 0x400)/*20480*/, "g_mem_track", 0);
    track_static_alloc_internal(&g_crit, sizeof(_RTL_CRITICAL_SECTION)/*24*/, "g_crit", 0);
    track_static_alloc_internal(g_mallocMemInfoArray, (sizeof(TempMemInfo) * 0x400) /*40960*/, "g_mallocMemInfoArray", 0);
    track_static_alloc_internal(g_hunk_track, sizeof(mem_track_t) * 0x80000, "g_hunk_track", 0);
    track_static_alloc_internal(g_hunklow_track, sizeof(mem_track_t) * 0x10000 /*1310720*/, "g_hunklow_track", 0);
    track_static_alloc_internal(g_tempMemInfoArray, sizeof(TempMemInfo) * 0x400 /*40960*/, "g_tempMemInfoArray", 0);
    track_static_alloc_internal(g_tempHighMemInfoArray, sizeof(TempMemInfo) * 0x400 /*40960*/, "g_tempHighMemInfoArray", 0);
    track_static_alloc_internal(g_combinedMemInfoArray, sizeof(TempMemInfo) * 0x400 /*40960*/, "g_combinedMemInfoArray", 0);
    track_static_alloc_internal(g_physicalMemInfoArray, sizeof(TempMemInfo) * 0x400 /*40960*/, "g_physicalMemInfoArray", 0);
    g_mem_track_filename = "";
}

void __cdecl track_addbasicinfo(meminfo_t* info, int type, int size)
{
    int v3; // [esp+8h] [ebp-8h]
    int v4; // [esp+Ch] [ebp-4h]

    info->typeTotal[type] += size;
    info->total += size;
    switch (type)
    {
    case 0:
    case 1:
    case 3:
    case 14:
    case 16:
    case 34:
    case 35:
        v4 = 0;
        break;
    default:
        v4 = 1;
        break;
    }
    if (v4)
        info->nonSwapTotal += size;
    switch (type)
    {
    case 0:
    case 1:
    case 3:
    case 13:
    case 15:
    case 34:
    case 35:
        v3 = 0;
        break;
    default:
        v3 = 1;
        break;
    }
    if (v3)
        info->nonSwapMinSpecTotal += size;
}

void __cdecl track_z_commit(int size, int type)
{
    EnterCriticalSection(&g_crit);
    track_addbasicinfo(&g_virtualMemInfo, type, size);
    track_addbasicinfo(&g_info, type, size);
    LeaveCriticalSection(&g_crit);
}

TempMemInfo* __cdecl GetTempMemInfo(
    int permanent,
    const char* name,
    int type,
    int usageType,
    TempMemInfo* tempMemInfoArray,
    int* tempMemInfoCount)
{
    TempMemInfo foundTempMemInfo; // [esp+8h] [ebp-34h] BYREF
    int i; // [esp+30h] [ebp-Ch]
    TempMemInfo* tempMemInfo; // [esp+34h] [ebp-8h]
    int count; // [esp+38h] [ebp-4h]

    count = *tempMemInfoCount;
    i = 0;
    for (tempMemInfo = tempMemInfoArray; ; ++tempMemInfo)
    {
        if (i >= count)
        {
            if (!name)
                MyAssertHandler(".\\qcommon\\mem_track.cpp", 318, 0, "%s", "name");
            if (++*tempMemInfoCount >= 1024)
                MyAssertHandler(
                    ".\\qcommon\\mem_track.cpp",
                    320,
                    0,
                    "%s\n\t(*tempMemInfoCount) = %i",
                    "((*tempMemInfoCount) < 1024)",
                    *tempMemInfoCount);
            tempMemInfo->permanent = permanent;
            tempMemInfo->data.name = name;
            tempMemInfo->data.type = type;
            tempMemInfo->data.usageType = usageType;
            tempMemInfo->data.size = 0;
            tempMemInfo->data.filename = "";
            tempMemInfo->high = 0;
            tempMemInfo->low = 0;
            tempMemInfo->hunkSize = 0;
            tempMemInfo->highExtra = 0;
            goto LABEL_17;
        }
        tempMemInfo = &tempMemInfoArray[i];
        if ((!name || tempMemInfo->data.name == name)
            && tempMemInfo->permanent == permanent
            && tempMemInfo->data.type == type
            && tempMemInfo->data.usageType == usageType)
        {
            break;
        }
        ++i;
    }
    if (!i)
        return tempMemInfo;
LABEL_17:
    memcpy(&foundTempMemInfo, tempMemInfo, sizeof(foundTempMemInfo));
    while (i > 0)
    {
        memcpy(&tempMemInfoArray[i], &tempMemInfoArray[i - 1], sizeof(TempMemInfo));
        --i;
    }
    memcpy(tempMemInfoArray, &foundTempMemInfo, sizeof(TempMemInfo));
    return tempMemInfoArray;
}

static void __cdecl CheckHighMemInfo(TempMemInfo* tempMemInfo, int hunkSize)
{
    int size; // [esp+0h] [ebp-4h]

    size = tempMemInfo->data.size;
    if (tempMemInfo->high < size)
        tempMemInfo->high = size;
    if (tempMemInfo->hunkSize < hunkSize)
    {
        tempMemInfo->hunkSize = hunkSize;
        tempMemInfo->highExtra = size;
    }
}

static void __cdecl CheckLowMemInfo(TempMemInfo* tempMemInfo)
{
    if (tempMemInfo->low > tempMemInfo->data.size)
        tempMemInfo->low = tempMemInfo->data.size;
}

static void __cdecl AddTempMemInfo(
    int size,
    int hunkSize,
    int permanent,
    const char* name,
    int type,
    int usageType,
    TempMemInfo* tempMemInfoArray,
    int* tempMemInfoCount)
{
    TempMemInfo* TempMemInfo; // eax

    TempMemInfo = GetTempMemInfo(permanent, name, type, usageType, tempMemInfoArray, tempMemInfoCount);
    TempMemInfo->data.size += size;
    CheckHighMemInfo(TempMemInfo, hunkSize);
}

static void __cdecl RemoveTempMemInfo(
    int size,
    int permanent,
    const char* name,
    int type,
    int usageType,
    TempMemInfo* tempMemInfoArray,
    int* tempMemInfoCount)
{
    TempMemInfo* TempMemInfo; // eax

    TempMemInfo = GetTempMemInfo(permanent, name, type, usageType, tempMemInfoArray, tempMemInfoCount);
    TempMemInfo->data.size -= size;
    CheckLowMemInfo(TempMemInfo);
}

void __cdecl track_physical_alloc(int size, const char* name, int type)
{
    if (size)
    {
        EnterCriticalSection(&g_crit);
        if (size <= 0)
            RemoveTempMemInfo(-size, 0, name, type, 7, g_physicalMemInfoArray, &g_physicalMemInfoCount);
        else
            AddTempMemInfo(size, 0, 0, name, type, 7, g_physicalMemInfoArray, &g_physicalMemInfoCount);
        track_addbasicinfo(&g_info, type, size);
        LeaveCriticalSection(&g_crit);
    }
}

void __cdecl track_hunk_alloc(int size, int pos, const char* name, int type)
{
    mem_track_t* mem_track; // [esp+8h] [ebp-4h]

    EnterCriticalSection(&g_crit);
    if (type != 14 && type != 16)
    {
        g_mem_track[1].size -= size;
        track_addbasicinfo(&g_info, 1, -size);
        if (g_mem_track[1].size < 0)
            MyAssertHandler(".\\qcommon\\mem_track.cpp", 631, 0, "%s", "g_mem_track[TRACK_FREE].size >= 0");
    }
    if (g_hunk_track_count >= 0x80000)
        MyAssertHandler(".\\qcommon\\mem_track.cpp", 634, 0, "%s", "g_hunk_track_count < MAX_HUNK_TRACK");
    mem_track = &g_hunk_track[g_hunk_track_count];
    mem_track->size = size;
    if (!*name)
        MyAssertHandler(".\\qcommon\\mem_track.cpp", 638, 0, "%s", "name[0]");
    mem_track->name = name;
    mem_track->type = type;
    mem_track->usageType = 3;
    mem_track->filename = "";
    mem_track->pos = pos;
    track_addbasicinfo(&g_info, type, size);
    ++g_hunk_track_count;
    LeaveCriticalSection(&g_crit);
}

void __cdecl track_hunk_allocLow(int size, int pos, const char* name, int type)
{
    mem_track_t* mem_track; // [esp+8h] [ebp-4h]

    EnterCriticalSection(&g_crit);
    if (type != 14 && type != 16)
    {
        g_mem_track[1].size -= size;
        track_addbasicinfo(&g_info, 1, -size);
        if (g_mem_track[1].size < 0)
            MyAssertHandler(".\\qcommon\\mem_track.cpp", 665, 0, "%s", "g_mem_track[TRACK_FREE].size >= 0");
    }
    if (g_hunklow_track_count >= 0x10000)
        MyAssertHandler(".\\qcommon\\mem_track.cpp", 668, 0, "%s", "g_hunklow_track_count < MAX_HUNKLOW_TRACK");
    mem_track = &g_hunklow_track[g_hunklow_track_count];
    mem_track->size = size;
    if (!*name)
        MyAssertHandler(".\\qcommon\\mem_track.cpp", 672, 0, "%s", "name[0]");
    mem_track->name = name;
    mem_track->type = type;
    mem_track->usageType = 3;
    mem_track->filename = "";
    mem_track->pos = pos;
    track_addbasicinfo(&g_info, type, size);
    ++g_hunklow_track_count;
    LeaveCriticalSection(&g_crit);
}

void __cdecl track_set_hunk_size(int size)
{
    EnterCriticalSection(&g_crit);
    if (g_mem_track[1].size)
        MyAssertHandler(".\\qcommon\\mem_track.cpp", 692, 0, "%s", "!g_mem_track[TRACK_FREE].size");
    g_mem_track[1].size = size;
    track_addbasicinfo(&g_info, 1, size);
    LeaveCriticalSection(&g_crit);
}

void __cdecl track_hunk_ClearToMarkHigh(int mark)
{
    mem_track_t* info; // [esp+8h] [ebp-Ch]
    int size; // [esp+Ch] [ebp-8h]
    int type; // [esp+10h] [ebp-4h]

    EnterCriticalSection(&g_crit);
    while (g_hunk_track_count)
    {
        info = &g_hunk_track[g_hunk_track_count - 1];
        if (info->pos <= mark)
            break;
        --g_hunk_track_count;
        size = info->size;
        type = info->type;
        track_addbasicinfo(&g_info, type, -size);
        if (type != 14 && type != 16)
        {
            track_addbasicinfo(&g_info, 1, size);
            g_mem_track[1].size += size;
        }
    }
    LeaveCriticalSection(&g_crit);
}

void __cdecl track_hunk_ClearToMarkLow(int mark)
{
    mem_track_t* info; // [esp+8h] [ebp-Ch]
    int size; // [esp+Ch] [ebp-8h]
    int type; // [esp+10h] [ebp-4h]

    EnterCriticalSection(&g_crit);
    while (g_hunklow_track_count)
    {
        info = &g_hunklow_track[g_hunklow_track_count - 1];
        if (info->pos <= mark)
            break;
        --g_hunklow_track_count;
        size = info->size;
        type = info->type;
        track_addbasicinfo(&g_info, type, -size);
        if (type != 14 && type != 16)
        {
            track_addbasicinfo(&g_info, 1, size);
            g_mem_track[1].size += size;
        }
    }
    LeaveCriticalSection(&g_crit);
}

static void track_temp_reset()
{
    g_tempMemInfoCount = 0;
}
static void track_temp_high_reset()
{
    g_tempHighMemInfoCount = 0;
}

void __cdecl track_hunk_ClearToStart()
{
    EnterCriticalSection(&g_crit);
    track_temp_reset();
    track_temp_high_reset();
    LeaveCriticalSection(&g_crit);
}

void track_init()
{
    mem_track_t* mem_track; // [esp+0h] [ebp-4h]

    InitializeCriticalSection(&g_crit);
    memset((unsigned __int8*)&g_info, 0, sizeof(g_info));
    memset((unsigned __int8*)&g_virtualMemInfo, 0, sizeof(g_virtualMemInfo));
    for (g_mem_track_count = 0; g_mem_track_count < 37; ++g_mem_track_count)
    {
        mem_track = &g_mem_track[g_mem_track_count];
        mem_track->size = 0;
        mem_track->name = "internal";
        mem_track->type = g_mem_track_count;
        mem_track->usageType = 4;
        mem_track->filename = "";
    }
    g_mem_track[1].name = "(hunk memory)";
    g_mem_track[22].name = "(agp memory)";
    g_hunk_track_count = 0;
    g_hunklow_track_count = 0;
    g_mem_track_filename = "scr_memorytree";
    TRACK_scr_memorytree();
    g_mem_track_filename = "scr_stringlist";
    TRACK_scr_stringlist();
    g_mem_track_filename = "scr_variable";
    TRACK_scr_variable();
    g_mem_track_filename = "scr_vm";
    TRACK_scr_vm();
    g_mem_track_filename = "scr_debugger";
    TRACK_scr_debugger();
    g_mem_track_filename = "scr_evaluate";
    TRACK_scr_evaluate();
    g_mem_track_filename = "scr_compiler";
    TRACK_scr_compiler();
    g_mem_track_filename = "scr_parser";
    TRACK_scr_parser();
    g_mem_track_filename = "xanim";
    TRACK_xanim();
    g_mem_track_filename = "xmodel";
    TRACK_xmodel();
    g_mem_track_filename = "sv_main";
    TRACK_sv_main();
    g_mem_track_filename = "cm_load";
    TRACK_cm_load();
    g_mem_track_filename = "cm_showcollision";
    TRACK_cm_showcollision();
    g_mem_track_filename = "cm_world";
    TRACK_cm_world();
    g_mem_track_filename = "cmd";
    TRACK_cmd();
    g_mem_track_filename = "com_files";
    TRACK_com_files();
    g_mem_track_filename = "com_math";
    TRACK_com_math();
    g_mem_track_filename = "com_sndalias";
    TRACK_com_sndalias();
    g_mem_track_filename = "fx_draw";
    CL_ResetStats_f();
    g_mem_track_filename = "fx_random";
    TRACK_fx_random();
    g_mem_track_filename = "fx_system";
    TRACK_fx_system();
    g_mem_track_filename = "fx_marks";
    TRACK_fx_marks();
    g_mem_track_filename = "dvar";
    TRACK_dvar();
    g_mem_track_filename = "dvar_cmds";
    TRACK_dvar_cmds();
    g_mem_track_filename = "msg";
    TRACK_msg();
    g_mem_track_filename = "profile";
    TRACK_profile();
    g_mem_track_filename = "profile_display";
    TRACK_profile_display();
    g_mem_track_filename = "q_parse";
    TRACK_q_parse();
    g_mem_track_filename = "win_common";
    TRACK_win_common();
    g_mem_track_filename = "win_syscon";
    TRACK_win_syscon();
    g_mem_track_filename = "win_net";
    TRACK_win_net();
    g_mem_track_filename = "q_shared";
    TRACK_q_shared();
    g_mem_track_filename = "common";
    TRACK_common();
    g_mem_track_filename = "dobj_management";
    TRACK_dobj_management();
    g_mem_track_filename = "files";
    TRACK_files();
    g_mem_track_filename = "memtrack";
    TRACK_memtrack();
    g_mem_track_filename = "scr_animtree";
    TRACK_scr_animtree();
    g_mem_track_filename = "com_memory";
    TRACK_com_memory();
    g_mem_track_filename = "sv_game";
    TRACK_sv_game();
    g_mem_track_filename = "sv_init";
    CL_ResetStats_f();
    g_mem_track_filename = "zutil";
    CL_ResetStats_f();
    g_mem_track_filename = "memfile";
    TRACK_memfile();
    g_mem_track_filename = "cl_main";
    TRACK_cl_main();
    g_mem_track_filename = "cl_console";
    TRACK_cl_console();
    g_mem_track_filename = "cl_input";
    TRACK_cl_input();
    g_mem_track_filename = "cl_keys";
    TRACK_cl_keys();
    g_mem_track_filename = "cl_parse";
    TRACK_cl_parse();
    g_mem_track_filename = "win_input";
    TRACK_win_input();
    g_mem_track_filename = "cl_srcn";
    CL_ResetStats_f();
    g_mem_track_filename = "cl_cgame";
    TRACK_cl_cgame();
    g_mem_track_filename = "statmonitor";
    TRACK_statmonitor();
    g_mem_track_filename = "snd";
    TRACK_snd();
    g_mem_track_filename = "snd_driver";
    TRACK_snd_driver();
    g_mem_track_filename = "stringed_hooks";
    TRACK_stringed_hooks();
    g_mem_track_filename = "devgui";
    TRACK_devgui();
    g_mem_track_filename = "assertive";
    TRACK_assertive();
    g_mem_track_filename = "db_registry";
    TRACK_db_registry();
    g_mem_track_filename = "com_profilemapload";
    TRACK_com_profilemapload();
    g_mem_track_filename = "record_dsound";
    TRACK_record_dsound();
    g_mem_track_filename = "phys";
    TRACK_phys();
    g_mem_track_filename = "phys_ode";
    TRACK_phys_ode();
    g_mem_track_filename = "profileMem";
    TRACK_profileMem();
    g_mem_track_filename = "missile_attractors";
    TRACK_missile_attractors();
    g_mem_track_filename = "DynEntityCollWorld";
    TRACK_DynEntityCollWorld();
    g_mem_track_filename = "ragdoll";
    TRACK_ragdoll();
    CG_track_init();
    G_track_init();
    UI_track_init();
    R_Track_Init();
    g_mem_track_filename = "";
}

void __cdecl track_shutdown(int project)
{
    const char* v1; // eax
    mem_track_node_s* node; // [esp+0h] [ebp-8h]
    int leak_size; // [esp+4h] [ebp-4h]

    EnterCriticalSection(&g_crit);
    leak_size = 0;
    for (node = g_head; node; node = node->next)
    {
        if (node->project == project)
        {
            if (project || I_stricmp(node->data.name, "MSS_MallocCallback"))
            {
                if (!alwaysfails)
                {
                    v1 = va("memory leak of '%s' in %s", node->data.name, projName[project]);
                    MyAssertHandler(".\\qcommon\\mem_track.cpp", 1211, 0, v1);
                }
            }
            else
            {
                leak_size += node->data.size;
            }
        }
    }
    LeaveCriticalSection(&g_crit);
}

void __cdecl track_getbasicinfo(meminfo_t* info)
{
    int MinSpecImageMemory; // eax

    if (!info)
        MyAssertHandler(".\\qcommon\\mem_track.cpp", 1243, 0, "%s", "info");
    EnterCriticalSection(&g_crit);
    memset((unsigned __int8*)info, 0, sizeof(meminfo_t));
    //MinSpecImageMemory = R_GetMinSpecImageMemory(); // KISAKTODO
    MinSpecImageMemory = 1337;
    track_addbasicinfo(info, 35, MinSpecImageMemory);
    track_addbasicmeminfo(info, &g_info);
    LeaveCriticalSection(&g_crit);
}

void __cdecl track_addbasicmeminfo(meminfo_t* sum, meminfo_t* in)
{
    int type; // [esp+0h] [ebp-4h]

    for (type = 0; type < 37; ++type)
        sum->typeTotal[type] += in->typeTotal[type];
    sum->total += in->total;
    sum->nonSwapTotal += in->nonSwapTotal;
    sum->nonSwapMinSpecTotal += in->nonSwapMinSpecTotal;
}

void __cdecl track_PrintInfo()
{
#if 0
    mem_track_t* p_data; // edx
    mem_track_t* v1; // edx
    int TreeMemUsage; // eax
    double v3; // st7
    double v4; // st7
    double v5; // st7
    int v6; // [esp+0h] [ebp-110h]
    int v7; // [esp+4h] [ebp-10Ch]
    int v8; // [esp+38h] [ebp-D8h]
    int v9; // [esp+3Ch] [ebp-D4h]
    int j; // [esp+40h] [ebp-D0h]
    mem_track_t* mem_trackd; // [esp+44h] [ebp-CCh]
    mem_track_t* mem_track; // [esp+44h] [ebp-CCh]
    mem_track_t* mem_tracka; // [esp+44h] [ebp-CCh]
    mem_track_t* mem_trackb; // [esp+44h] [ebp-CCh]
    mem_track_t* mem_trackc; // [esp+44h] [ebp-CCh]
    mem_track_node_s* node; // [esp+48h] [ebp-C8h]
    mem_track_node_s* nodea; // [esp+48h] [ebp-C8h]
    mem_track_t* mem_track2; // [esp+4Ch] [ebp-C4h]
    meminfo_t info; // [esp+50h] [ebp-C0h] BYREF
    int len2; // [esp+F0h] [ebp-20h]
    int len; // [esp+F4h] [ebp-1Ch]
    mem_track_t* sorted_mem_track; // [esp+F8h] [ebp-18h]
    bool addedLine; // [esp+FFh] [ebp-11h]
    int nodeCount; // [esp+100h] [ebp-10h]
    int type; // [esp+104h] [ebp-Ch]
    int i; // [esp+108h] [ebp-8h]
    int minSpecImageMemory; // [esp+10Ch] [ebp-4h]

    EnterCriticalSection(&g_crit);
    memset((unsigned __int8*)&info, 0, sizeof(info));
    minSpecImageMemory = 0;
    minSpecImageMemory = R_GetMinSpecImageMemory();
    len2 = g_hunklow_track_count + g_hunk_track_count + g_mem_track_count;
    if (minSpecImageMemory)
        ++len2;
    nodeCount = 0;
    for (node = g_head; node; node = node->next)
        ++nodeCount;
    nodeCount += g_physicalMemInfoCount;
    len2 += nodeCount;
    sorted_mem_track = (mem_track_t*)malloc(20 * len2);
    if (sorted_mem_track)
    {
        len = 0;
        Com_Memcpy((char*)sorted_mem_track, (char*)g_mem_track, 20 * g_mem_track_count);
        len += g_mem_track_count;
        Com_Memcpy((char*)&sorted_mem_track[len], (char*)g_hunk_track, 20 * g_hunk_track_count);
        len += g_hunk_track_count;
        Com_Memcpy((char*)&sorted_mem_track[len], (char*)g_hunklow_track, 20 * g_hunklow_track_count);
        len += g_hunklow_track_count;
        if (minSpecImageMemory)
        {
            mem_trackd = &sorted_mem_track[len];
            mem_trackd->size = minSpecImageMemory;
            mem_trackd->name = "min spec tex";
            mem_trackd->type = 35;
            mem_trackd->usageType = 5;
            mem_trackd->filename = "";
            ++len;
        }
        mem_track = &sorted_mem_track[len];
        nodea = g_head;
        while (nodea)
        {
            mem_track->name = nodea->data.name;
            mem_track->filename = nodea->data.filename;
            mem_track->size = nodea->data.size;
            mem_track->pos = nodea->data.pos;
            *(unsigned int*)&mem_track->type = *(unsigned int*)&nodea->data.type;
            nodea = nodea->next;
            ++mem_track;
        }
        i = 0;
        while (i < g_physicalMemInfoCount)
        {
            p_data = &g_physicalMemInfoArray[i].data;
            mem_track->name = p_data->name;
            mem_track->filename = p_data->filename;
            mem_track->size = p_data->size;
            mem_track->pos = p_data->pos;
            *(unsigned int*)&mem_track->type = *(unsigned int*)&p_data->type;
            ++i;
            ++mem_track;
        }
        len += nodeCount;
        if (len != len2)
            MyAssertHandler(".\\qcommon\\mem_track.cpp", 1423, 0, "%s", "len == len2");
        for (i = len - 1; i >= 0; --i)
        {
            for (j = i - 1; j >= 0; --j)
            {
                mem_tracka = &sorted_mem_track[i];
                mem_track2 = &sorted_mem_track[j];
                if (mem_tracka->type == mem_track2->type
                    && !strcmp(mem_tracka->name, mem_track2->name)
                    && !strcmp(mem_tracka->filename, mem_track2->filename))
                {
                    mem_track2->size += mem_tracka->size;
                    v1 = &sorted_mem_track[--len];
                    mem_tracka->name = v1->name;
                    mem_tracka->filename = v1->filename;
                    mem_tracka->size = v1->size;
                    mem_tracka->pos = v1->pos;
                    *(unsigned int*)&mem_tracka->type = *(unsigned int*)&v1->type;
                    break;
                }
            }
        }
        for (i = 0; i < len; ++i)
        {
            mem_trackb = &sorted_mem_track[i];
            type = mem_trackb->type;
            if (mem_trackb->size < 0)
                MyAssertHandler(
                    ".\\qcommon\\mem_track.cpp",
                    1446,
                    0,
                    "%s\n\t(mem_track->size) = %i",
                    "(mem_track->size >= 0)",
                    mem_trackb->size);
            info.typeTotal[type] += mem_trackb->size;
            info.total += mem_trackb->size;
            switch (type)
            {
            case 0:
            case 1:
            case 3:
            case 14:
            case 16:
            case 34:
            case 35:
                v9 = 0;
                break;
            default:
                v9 = 1;
                break;
            }
            if (v9)
                info.nonSwapTotal += mem_trackb->size;
            switch (type)
            {
            case 0:
            case 1:
            case 3:
            case 13:
            case 15:
            case 34:
            case 35:
                v8 = 0;
                break;
            default:
                v8 = 1;
                break;
            }
            if (v8)
                info.nonSwapMinSpecTotal += mem_trackb->size;
        }
        qsort(sorted_mem_track, len, 0x14u, (int(__cdecl*)(const void*, const void*))mem_track_compare);
        info.typeTotal[23] = info.typeTotal[19]
            + info.typeTotal[22]
            + info.typeTotal[21]
            + info.typeTotal[20]
            + info.typeTotal[18];
        info.typeTotal[29] = info.typeTotal[27] + info.typeTotal[28] + info.typeTotal[26] + info.typeTotal[25];
        info.typeTotal[4] = -1;
        info.typeTotal[17] = -1;
        info.typeTotal[24] = -1;
        info.typeTotal[32] = -1;
        info.typeTotal[36] = -1;
        for (i = 0; i < len; ++i)
        {
            mem_trackc = &sorted_mem_track[i];
            type = mem_trackc->type;
            if (mem_trackc->size >= 0x2000)
            {
                if (!*mem_trackc->name)
                    MyAssertHandler(".\\qcommon\\mem_track.cpp", 1485, 0, "%s", "mem_track->name[0]");
                Com_Printf(
                    0,
                    "%s %s %7i  %-24s %s\n",
                    typeName[type],
                    usageTypeName[mem_trackc->usageType],
                    mem_trackc->size / 1024,
                    mem_trackc->name,
                    mem_trackc->filename);
            }
        }
        Com_Printf(0, "\n");
        v7 = XAnimGetTreeMaxMemUsage() / 1024;
        v6 = XAnimGetTreeHighMemUsage() / 1024;
        TreeMemUsage = XAnimGetTreeMemUsage();
        Com_Printf(0, "anim tree      %7i (high: %7i, max: %7i)\n", TreeMemUsage / 1024, v6, v7);
        Com_Printf(0, "\ntotals:\n");
        Com_Printf(0, "-type-             -MB--\n");
        addedLine = 0;
        for (i = 0; i < 37; ++i)
        {
            if (info.typeTotal[i] < 0)
            {
                if (addedLine)
                    Com_Printf(0, "------------------------\n");
            }
            else if (info.typeTotal[i] > 51200)
            {
                v3 = ConvertToMB(info.typeTotal[i]);
                Com_Printf(0, "%s %5.1f\n", typeName[i], v3);
                addedLine = 1;
            }
        }
        v4 = ConvertToMB(info.nonSwapTotal);
        Com_Printf(0, "current total      %5.1f\n", v4);
        v5 = ConvertToMB(info.nonSwapMinSpecTotal);
        Com_Printf(0, "min pc total       %5.1f\n", v5);
        free(sorted_mem_track);
        LeaveCriticalSection(&g_crit);
    }
    else
    {
        Com_Printf(0, "track_PrintInfo: out of memory\n");
        LeaveCriticalSection(&g_crit);
    }
#endif
}

int __cdecl mem_track_compare(unsigned int* elem1, unsigned int* elem2)
{
    if (*((unsigned __int8*)elem1 + 16) < (int)*((unsigned __int8*)elem2 + 16))
        return -1;
    if (*((unsigned __int8*)elem1 + 16) <= (int)*((unsigned __int8*)elem2 + 16))
        return elem1[2] - elem2[2];
    return 1;
}

void __cdecl track_PrintAllInfo()
{
    if (Sys_IsMainThread())
    {
        track_PrintInfo();
        //track_PrintTempInfo();
    }
}

void __cdecl UI_track_init()
{
    g_mem_track_filename = "ui_main";
    TRACK_ui_main();
    g_mem_track_filename = "ui_shared";
    TRACK_ui_shared();
    g_mem_track_filename = "ui_utils";
    TRACK_ui_utils();
    g_mem_track_filename = "ui_atoms";
    CL_ResetStats_f();
    g_mem_track_filename = "ui_memtrack";
    TRACK_g_memtrack();
    g_mem_track_filename = "ui_shared_obj";
    TRACK_ui_shared_obj();
}

double __cdecl ConvertToMB(int bytes)
{
    return (float)((double)bytes / 1048576.0);
}

// str8 from the linker error
void __cdecl TRACK_r_bsp_load_obj() { /* THUNK */ }
void __cdecl TRACK_rb_debug() { /* THUNK */ }
void __cdecl TRACK_ui_shared_obj() { /* THUNK */ }
void __cdecl TRACK_scr_memorytree() { /* THUNK */ }
void __cdecl TRACK_scr_stringlist() { /* THUNK */ }
void __cdecl TRACK_scr_variable() { /* THUNK */ }
void __cdecl TRACK_scr_compiler() { /* THUNK */ }
void __cdecl TRACK_com_files() { /* THUNK */ }
void __cdecl TRACK_com_sndalias() { /* THUNK */ }
void __cdecl TRACK_dvar() { /* THUNK */ }
void __cdecl TRACK_profile() { /* THUNK */ }
void __cdecl TRACK_profile_display() { /* THUNK */ }
void __cdecl TRACK_win_common() { /* THUNK */ }
void __cdecl TRACK_win_syscon() { /* THUNK */ }
void __cdecl TRACK_common() { /* THUNK */ }
void __cdecl TRACK_files() { /* THUNK */ }
void __cdecl TRACK_com_memory() { /* THUNK */ }
void __cdecl TRACK_memfile() { /* THUNK */ }
void __cdecl TRACK_win_input() { /* THUNK */ }
void __cdecl TRACK_assertive() { /* THUNK */ }
void __cdecl TRACK_record_dsound() { /* THUNK */ }
void __cdecl TRACK_phys_ode() { /* THUNK */ }
void __cdecl TRACK_profileMem() { /* THUNK */ }

void __cdecl R_Track_Init()
{
    g_mem_track_filename = "r_bsp_load_obj";
    TRACK_r_bsp_load_obj();
    g_mem_track_filename = "rb_sky";
    TRACK_rb_sky();
    g_mem_track_filename = "r_buffers";
    TRACK_r_buffers();
    g_mem_track_filename = "r_debug";
    TRACK_r_debug();
    g_mem_track_filename = "r_dpvs";
    TRACK_r_dpvs();
    g_mem_track_filename = "r_fog";
    CL_ResetStats_f();
    g_mem_track_filename = "r_font";
    TRACK_r_font();
    g_mem_track_filename = "r_image";
    TRACK_r_image();
    g_mem_track_filename = "r_image_wavelet";
    TRACK_r_image_wavelet();
    g_mem_track_filename = "r_init";
    TRACK_r_init();
    g_mem_track_filename = "r_marks";
    CL_ResetStats_f();
    g_mem_track_filename = "r_material";
    TRACK_r_material();
    g_mem_track_filename = "r_model";
    TRACK_r_model();
    g_mem_track_filename = "r_rendercmds";
    TRACK_r_rendercmds();
    g_mem_track_filename = "r_scene";
    TRACK_r_scene();
    g_mem_track_filename = "r_screenshot";
    TRACK_r_screenshot();
    g_mem_track_filename = "r_staticmodel";
    CL_ResetStats_f();
    g_mem_track_filename = "r_staticmodelcache";
    TRACK_r_staticmodelcache();
    g_mem_track_filename = "r_utils";
    CL_ResetStats_f();
    g_mem_track_filename = "r_xsurface";
    CL_ResetStats_f();
    g_mem_track_filename = "r_water";
    TRACK_r_water();
    g_mem_track_filename = "r_workercmds";
    TRACK_r_workercmds();
    g_mem_track_filename = "r_light";
    CL_ResetStats_f();
    g_mem_track_filename = "rb_debug";
    TRACK_rb_debug();
    g_mem_track_filename = "rb_backend";
    TRACK_rb_backend();
    g_mem_track_filename = "rb_drawprofile";
    TRACK_rb_drawprofile();
    g_mem_track_filename = "rb_shade";
    CL_ResetStats_f();
    g_mem_track_filename = "rb_showcollision";
    TRACK_rb_showcollision();
    g_mem_track_filename = "rb_state";
    TRACK_rb_state();
    g_mem_track_filename = "rb_stats";
    TRACK_rb_stats();
    g_mem_track_filename = "rb_sunshadow";
    TRACK_rb_sunshadow();
    g_mem_track_filename = "rb_tess";
    CL_ResetStats_f();
}

void __cdecl track_z_alloc(int size, const char* name, int type, char* pos, int project, int overhead)
{
    mem_track_node_s* node; // [esp+8h] [ebp-4h]

    EnterCriticalSection(&g_crit);
    g_mem_track[0].size += overhead;
    track_addbasicinfo(&g_info, 0, overhead);
    if (type != 14 && type != 16 && type != 35)
    {
        AddTempMemInfo(size, 0, 0, name, type, 2, g_mallocMemInfoArray, &g_mallocMemInfoCount);
        g_malloc_mem_size += size;
        if (g_malloc_mem_high < g_malloc_mem_size)
            g_malloc_mem_high = g_malloc_mem_size;
    }
    node = (mem_track_node_s*)(pos - 32);
    if (!*name)
        MyAssertHandler(".\\qcommon\\mem_track.cpp", 411, 0, "%s", "name[0]");
    node->data.name = name;
    node->data.filename = "";
    node->data.size = size;
    node->data.type = type;
    node->data.usageType = 2;
    node->next = g_head;
    node->prev = 0;
    node->project = project;
    if (g_head)
        g_head->prev = node;
    g_head = (mem_track_node_s*)(pos - 32);
    track_addbasicinfo(&g_info, type, size);
    LeaveCriticalSection(&g_crit);
}

void __cdecl track_temp_free(int size, int permanent, const char* name)
{
    EnterCriticalSection(&g_crit);
    RemoveTempMemInfo(size, permanent, name, 10, 6, g_tempMemInfoArray, &g_tempMemInfoCount);
    LeaveCriticalSection(&g_crit);
}

void __cdecl track_temp_alloc(int size, int hunkSize, int permanent, const char* name)
{
    EnterCriticalSection(&g_crit);
    AddTempMemInfo(size, hunkSize, permanent, name, 10, 6, g_tempMemInfoArray, &g_tempMemInfoCount);
    LeaveCriticalSection(&g_crit);
}

void __cdecl track_temp_high_clear(int permanent)
{
    int i; // [esp+0h] [ebp-8h]
    TempMemInfo* tempMemInfo; // [esp+4h] [ebp-4h]

    EnterCriticalSection(&g_crit);
    i = 0;
    tempMemInfo = g_tempHighMemInfoArray;
    while (i < g_tempHighMemInfoCount)
    {
        if (tempMemInfo->permanent == permanent)
            tempMemInfo->data.size = 0;
        ++i;
        ++tempMemInfo;
    }
    LeaveCriticalSection(&g_crit);
}

void __cdecl track_temp_high_alloc_DONE(int size, int hunkSize, int permanent, const char* name)
{
    EnterCriticalSection(&g_crit);
    AddTempMemInfo(size, hunkSize, permanent, name, 10, 6, g_tempHighMemInfoArray, &g_tempHighMemInfoCount);
    LeaveCriticalSection(&g_crit);
}