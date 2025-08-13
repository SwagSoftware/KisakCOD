#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "server_mp.h"
#include <qcommon/qcommon.h>
#include <server/sv_game.h>
#include <database/database.h>
#include <game_mp/g_public_mp.h>
#include <gfx_d3d/r_rendercmds.h>
#include <universal/com_files.h>
#include <win32/win_net.h>
#include <universal/com_constantconfigstrings.h>
#include <qcommon/threads.h>
#include <qcommon/com_bsp.h>
#include <universal/com_sndalias.h>
#include <qcommon/cmd.h>
#include <stringed/stringed_hooks.h>
#include <client/client.h>


//Line 53332:  0006 : 00cf4d5c       int sv_serverId_value    834a4d5c     sv_init_mp.obj

const dvar_t *sv_punkbuster;
const dvar_t *sv_allowAnonymous;
const dvar_t *sv_privatePassword;
const dvar_t *sv_allowDownload;
const dvar_t *sv_iwds;
const dvar_t *sv_iwdNames;
const dvar_t *sv_referencedIwds;
const dvar_t *sv_referencedIwdNames;
const dvar_t *sv_FFCheckSums;
const dvar_t *sv_FFNames;
const dvar_t *sv_referencedFFCheckSums;
const dvar_t *sv_referencedFFNames;
const dvar_t *sv_voice;
const dvar_t *sv_voiceQuality;
const dvar_t *sv_pure;
const dvar_t *rcon_password;
const dvar_t *sv_wwwDownload;
const dvar_t *sv_wwwBaseURL;
const dvar_t *sv_wwwDlDisconnected;
const dvar_t *sv_loadMyChanges;
const dvar_t *sv_clientArchive;

void __cdecl SV_SetConfigstring(int index, const char *val)
{
    unsigned __int16 v2; // [esp+20h] [ebp-444h]
    client_t *client; // [esp+38h] [ebp-42Ch]
    int maxChunk; // [esp+3Ch] [ebp-428h]
    int remaining; // [esp+40h] [ebp-424h]
    char buf[1028]; // [esp+44h] [ebp-420h] BYREF
    int len; // [esp+44Ch] [ebp-18h]
    int overhead; // [esp+450h] [ebp-14h]
    int sent; // [esp+454h] [ebp-10h]
    int caseSensitive; // [esp+458h] [ebp-Ch]
    int i; // [esp+45Ch] [ebp-8h]
    char cmd; // [esp+463h] [ebp-1h]

    if ((unsigned int)index >= 0x98A)
        Com_Error(ERR_DROP, "SV_SetConfigstring: bad index %i", index);

    if (sv.configstrings[index])
    {
        if (!val)
            val = (char *)"";
        if (strcmp(val, SL_ConvertToString(sv.configstrings[index])))
        {
            SL_RemoveRefToString(sv.configstrings[index]);
            caseSensitive = index < 821;
            v2 = index < 821 ? SL_GetString_(val, 0, 19) : SL_GetLowercaseString_(val, 0, 19);
            sv.configstrings[index] = v2;
            if (SV_Loaded() || sv.restarting)
            {
                len = strlen(val);
                sprintf(buf, "%i", index);
                overhead = &buf[strlen(buf) + 1] - &buf[1] + 4;
                maxChunk = 1024 - overhead;
                i = 0;
                client = svs.clients;
                while (i < sv_maxclients->current.integer)
                {
                    if (client->header.state >= 3)
                    {
                        if (len <= maxChunk)
                        {
                            SV_SendServerCommand(client, SV_CMD_RELIABLE, "%c %i %s", 100, index, val);
                        }
                        else
                        {
                            sent = 0;
                            for (remaining = len; remaining > 0; remaining -= maxChunk)
                            {
                                if (sent)
                                {
                                    if (remaining > maxChunk)
                                        cmd = 121;
                                    else
                                        cmd = 122;
                                }
                                else
                                {
                                    cmd = 120;
                                }
                                I_strncpyz(buf, &val[sent], maxChunk + 1);
                                SV_SendServerCommand(client, SV_CMD_RELIABLE, "%c %i %s", cmd, index, buf);
                                sent += maxChunk;
                            }
                        }
                    }
                    ++i;
                    ++client;
                }
            }
        }
    }
    else if (val)
    {
        MyAssertHandler(".\\server_mp\\sv_init_mp.cpp", 114, 0, "%s", "!val");
    }
}

void __cdecl SV_GetConfigstring(unsigned int index, char *buffer, int bufferSize)
{
    if (bufferSize < 1)
        Com_Error(ERR_DROP, "SV_GetConfigstring: bufferSize == %i", bufferSize);
    if (index >= ARRAY_COUNT(sv.configstrings))
        Com_Error(ERR_DROP, "SV_GetConfigstring: bad index %i", index);

    iassert(sv.configstrings[index]);
    I_strncpyz(buffer, SL_ConvertToString(sv.configstrings[index]), bufferSize);
}

unsigned int __cdecl SV_GetConfigstringConst(unsigned int index)
{
    iassert((unsigned)index < MAX_CONFIGSTRINGS);
    iassert(sv.configstrings[index]);
    
    return sv.configstrings[index];
}

void __cdecl SV_SetConfigValueForKey(int start, int max, char *key, char *value)
{
    HashEntry_unnamed_type_u v5; // [esp+0h] [ebp-14h]
    unsigned int name; // [esp+4h] [ebp-10h]
    signed int i; // [esp+10h] [ebp-4h]
    int ia; // [esp+10h] [ebp-4h]

    if (start < 821)
        v5.prev = SL_FindString(key);
    else
        v5.prev = SL_FindLowercaseString(key).prev;
    i = CCS_GetConstConfigStringIndex(value);
    if (i < 0)
    {
        for (ia = 0; ia < max; ++ia)
        {
            name = sv.configstrings[ia + start];
            if (name == sv.emptyConfigString)
            {
                SV_SetConfigstring(ia + start, key);
                break;
            }
            if (v5.prev == name)
                break;
        }
    }
    else
    {
        ia = CCS_GetConfigStringNumForConstIndex(i);
        if (ia >= start && ia < max + start)
            SV_SetConfigstring(ia + start, key);
    }
    if (ia == max)
    {
        Com_Printf(15, "Overflow at config string start value of %i: key values printed below\n", start);
        for (ia = 0; ia < max; ++ia)
        {
            Com_Printf(15, "%i: %i ( %s )\n", ia + start, sv.configstrings[ia + start], SL_ConvertToString(sv.configstrings[ia + start]));
        }
        Com_Error(ERR_DROP, "SV_SetConfigValueForKey: overflow'");
    }
    SV_SetConfigstring(ia + max + start, value);
}

void __cdecl SV_SetUserinfo(int index, char *val)
{
    const char *v2; // eax

    if (index < 0 || index >= sv_maxclients->current.integer)
        Com_Error(ERR_DROP, "SV_SetUserinfo: bad index %i", index);
    if (!val)
        val = (char *)"";
    I_strncpyz(svs.clients[index].userinfo, val, 1024);
    v2 = Info_ValueForKey(val, "name");
    I_strncpyz(svs.clients[index].name, v2, 16);
}

void __cdecl SV_GetUserinfo(int index, char *buffer, int bufferSize)
{
    if (bufferSize < 1)
        Com_Error(ERR_DROP, "SV_GetUserinfo: bufferSize == %i", bufferSize);
    if (index < 0 || index >= sv_maxclients->current.integer)
        Com_Error(ERR_DROP, "SV_GetUserinfo: bad index %i", index);
    I_strncpyz(buffer, svs.clients[index].userinfo, bufferSize);
}

void __cdecl SV_CreateBaseline()
{
    float *absmax; // [esp+8h] [ebp-18h]
    float *absmin; // [esp+10h] [ebp-10h]
    gentity_s *svent; // [esp+18h] [ebp-8h]
    int entnum; // [esp+1Ch] [ebp-4h]

    for (entnum = 1; entnum < sv.num_entities; ++entnum)
    {
        svent = SV_GentityNum(entnum);
        if (svent->r.linked)
        {
            svent->s.number = entnum;
            memcpy(&sv.svEntities[entnum].baseline, svent, 0xF4u);
            sv.svEntities[entnum].baseline.r.svFlags = svent->r.svFlags;
            sv.svEntities[entnum].baseline.r.clientMask[0] = svent->r.clientMask[0];
            sv.svEntities[entnum].baseline.r.clientMask[1] = svent->r.clientMask[1];
            absmin = sv.svEntities[entnum].baseline.r.absmin;
            *absmin = svent->r.absmin[0];
            absmin[1] = svent->r.absmin[1];
            absmin[2] = svent->r.absmin[2];
            absmax = sv.svEntities[entnum].baseline.r.absmax;
            *absmax = svent->r.absmax[0];
            absmax[1] = svent->r.absmax[1];
            absmax[2] = svent->r.absmax[2];
            if (svent->s.clientNum >= 64)
                svent->s.clientNum = 64;
        }
    }
}

void __cdecl SV_BoundMaxClients(int minimum)
{
    DvarLimits v1; // [esp-10h] [ebp-14h]

    v1.integer.max = Dvar_RegisterInt(
        "ui_maxclients",
        32,
        (DvarLimits)0x4000000001LL,
        DVAR_ARCHIVE | DVAR_SERVERINFO | DVAR_LATCH,
        "The maximum number of clients that can connect to a server")->current.integer;
    v1.enumeration.stringCount = 1;
    sv_maxclients = Dvar_RegisterInt(
        "sv_maxclients",
        32,
        v1,
        DVAR_ARCHIVE | DVAR_SERVERINFO | DVAR_LATCH,
        "The maximum number of clients that can connect to a server");
    Dvar_ClearModified((dvar_s*)sv_maxclients);
    if (sv_maxclients->current.integer < minimum)
        Dvar_SetInt((dvar_s *)sv_maxclients, minimum);
}

void __cdecl SV_Startup()
{
    iassert(!svs.initialized);
    SV_BoundMaxClients(1);
    iassert(sv_maxclients->current.integer <= 64);
    svs.numSnapshotEntities = 172032;
    svs.numSnapshotClients = sv_maxclients->current.integer * 32 * sv_maxclients->current.integer;
    svs.sv_lastTimeMasterServerCommunicated = svs.time;
    svs.initialized = 1;
    Dvar_SetBool((dvar_s *)com_sv_running, 1);
}

void __cdecl SV_ClearServer()
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 2442; ++i)
    {
        if (sv.configstrings[i])
            SL_RemoveRefToString(sv.configstrings[i]);
    }
    if (sv.emptyConfigString)
        SL_RemoveRefToString(sv.emptyConfigString);
    Com_Memset((unsigned int *)&sv, 0, 392288);
    com_inServerFrame = 0;
}

void __cdecl SV_InitArchivedSnapshot()
{
    svs.nextArchivedSnapshotFrames = 0;
    svs.nextArchivedSnapshotBuffer = 0;
    svs.nextCachedSnapshotEntities = 0;
    svs.nextCachedSnapshotClients = 0;
    svs.nextCachedSnapshotFrames = 0;
}

void __cdecl SV_InitDvar()
{
    Dvar_ResetScriptInfo();
}

void __cdecl SV_ChangeMaxClients()
{
    int oldMaxClients; // [esp+0h] [ebp-10h]
    client_t *oldClients; // [esp+4h] [ebp-Ch]
    int i; // [esp+8h] [ebp-8h]
    int ia; // [esp+8h] [ebp-8h]
    int ib; // [esp+8h] [ebp-8h]
    int count; // [esp+Ch] [ebp-4h]
    int counta; // [esp+Ch] [ebp-4h]

    count = 0;
    for (i = 0; i < sv_maxclients->current.integer; ++i)
    {
        if (svs.clients[i].header.state >= 2 && i > count)
            count = i;
    }
    counta = count + 1;
    oldMaxClients = sv_maxclients->current.integer;
    SV_BoundMaxClients(counta);
    if (sv_maxclients->current.integer != oldMaxClients)
    {
        oldClients = (client_t *)Hunk_AllocateTempMemory(677432 * counta, "SV_ChangeMaxClients");
        for (ia = 0; ia < counta; ++ia)
        {
            if (svs.clients[ia].header.state < 2)
                Com_Memset(&oldClients[ia], 0, 677432);
            else
                memcpy(&oldClients[ia], &svs.clients[ia], sizeof(client_t));
        }
        Com_Memset(svs.clients, 0, 677432 * sv_maxclients->current.integer);
        for (ib = 0; ib < counta; ++ib)
        {
            if (oldClients[ib].header.state >= 2)
                memcpy(&svs.clients[ib], &oldClients[ib], sizeof(svs.clients[ib]));
        }
        Hunk_FreeTempMemory((char*)oldClients);
        svs.numSnapshotEntities = 172032;
        svs.numSnapshotClients = sv_maxclients->current.integer * 32 * sv_maxclients->current.integer;
    }
}

void __cdecl SV_SetExpectedHunkUsage(char *mapname)
{
    int handle[2]; // [esp+0h] [ebp-18h] BYREF
    char *buf; // [esp+8h] [ebp-10h]
    int len; // [esp+Ch] [ebp-Ch]
    const char *token; // [esp+10h] [ebp-8h]
    const char *buftrav; // [esp+14h] [ebp-4h] BYREF

    handle[1] = (int)"hunkusage.dat"; // KISAKTODO: handle fubar?
    len = FS_FOpenFileByMode((char*)"hunkusage.dat", handle, FS_READ);
    if (len >= 0)
    {
        buf = (char*)Z_Malloc(len + 1, "SV_SetExpectedHunkUsage", 10);
        memset(buf, 0, len + 1);
        FS_Read((unsigned char*)buf, len, handle[0]);
        FS_FCloseFile(handle[0]);
        buftrav = buf;
        while (1)
        {
            token = Com_Parse(&buftrav)->token;
            if (!token)
                MyAssertHandler(".\\server_mp\\sv_init_mp.cpp", 573, 0, "%s", "token");
            if (!*token)
                break;
            if (!I_stricmp(token, mapname))
            {
                token = Com_Parse(&buftrav)->token;
                if (token)
                {
                    if (*token)
                    {
                        com_expectedHunkUsage = atoi(token);
                        Z_Free(buf, 10);
                        return;
                    }
                }
            }
        }
        Z_Free(buf, 10);
    }
}

void __cdecl SV_SpawnServer(char *server)
{
    char* v1; // eax
    unsigned int v2; // eax
    int v3; // esi
    int v4; // esi
    int v5; // eax
    unsigned __int16 prev; // ax
    const char *denied; // [esp+18h] [ebp-B0h]
    client_t *client; // [esp+1Ch] [ebp-ACh]
    client_t *clienta; // [esp+1Ch] [ebp-ACh]
    char filename[68]; // [esp+20h] [ebp-A8h] BYREF
    int checksum; // [esp+64h] [ebp-64h] BYREF
    XZoneInfo zoneInfo[1]; // [esp+68h] [ebp-60h] BYREF
    int savepersist; // [esp+74h] [ebp-54h]
    char zoneName[64]; // [esp+78h] [ebp-50h] BYREF
    int i; // [esp+BCh] [ebp-Ch]
    bool mapIsPreloaded; // [esp+C3h] [ebp-5h]
    const char *p; // [esp+C4h] [ebp-4h]

    Com_SyncThreads();
    Sys_BeginLoadThreadPriorities();
    mapIsPreloaded = 0;
    if (useFastFile->current.enabled && !com_dedicated->current.integer)
    {
        DB_ResetZoneSize(0);
        Com_sprintf(zoneName, 0x40u, "%s_load", server);
        zoneInfo[0].name = zoneName;
        zoneInfo[0].allocFlags = 32;
        zoneInfo[0].freeFlags = 96;
        DB_LoadXAssets(zoneInfo, 1u, 0);
    }
    Scr_ParseGameTypeList();
    SV_SetGametype();
    if (!mapIsPreloaded)
        CL_InitLoad(server, sv_gametype->current.string);
    if (useFastFile->current.enabled && !mapIsPreloaded)
    {
        DB_SyncXAssets();
        DB_UpdateDebugZone();
    }
    R_BeginRemoteScreenUpdate();
    if (fs_debug->current.integer == 2)
        Dvar_SetInt((dvar_s *)fs_debug, 0);
    ProfLoad_Activate();
    if (com_sv_running->current.enabled)
    {
        savepersist = G_GetSavePersist();
        i = 0;
        client = svs.clients;
        while (i < sv_maxclients->current.integer)
        {
            if (client->header.state >= 3)
            {
                Com_sprintf(filename, 0x40u, "loadingnewmap\n%s\n%s", server, sv_gametype->current.string);
                NET_OutOfBandPrint(NS_SERVER, client->header.netchan.remoteAddress, filename);
            }
            ++i;
            ++client;
        }
        NET_Sleep(0xFAu);
    }
    else
    {
        savepersist = 0;
    }

    iassert(!strstr(server, "\\"));
    Dvar_SetStringByName("mapname", server);
    R_EndRemoteScreenUpdate();
    if (!mapIsPreloaded)
    {
        CL_MapLoading(server);
        R_BeginRemoteScreenUpdate();
        R_EndRemoteScreenUpdate();
        CL_ShutdownAll();
    }
    SV_ShutdownGameProgs();
    Com_Printf(15, "------ Server Initialization ------\n");
    Com_Printf(15, "Server: %s\n", server);
    SV_ClearServer();
    if (!useFastFile->current.enabled)
    {
        FS_Shutdown();
        FS_ClearIwdReferences();
    }
    if (!mapIsPreloaded)
        Com_Restart();
    if (com_sv_running->current.enabled)
        SV_ChangeMaxClients();
    else
        SV_Startup();
    I_strncpyz(sv.gametype, (char *)sv_gametype->current.integer, 64);
    v2 = Sys_MillisecondsRaw();
    srand(v2);
    v3 = rand() << 16;
    v4 = rand() ^ v3;
    sv.checksumFeed = Sys_Milliseconds() ^ v4;
    FS_Restart(0, sv.checksumFeed);
    if (!useFastFile->current.enabled)
    {
        Com_GetBspFilename(filename, 0x40u, server);
        SV_SetExpectedHunkUsage(filename);
    }
    if (!mapIsPreloaded)
    {
        ProfLoad_Begin("start loading client");
        CL_StartLoading();
        ProfLoad_End();
        if (useFastFile->current.enabled)
        {
            zoneInfo[0].name = server;
            zoneInfo[0].allocFlags = 8;
            zoneInfo[0].freeFlags = 8;
            DB_LoadXAssets(zoneInfo, 1u, 0);
            iassert(sv_loadMyChanges);
            if (sv_loadMyChanges->current.enabled)
            {
                Cbuf_ExecuteBuffer(0, CL_ControllerIndexFromClientNum(0), "loadzone mychanges\n");
            }
        }
    }
    R_BeginRemoteScreenUpdate();
    sv.emptyConfigString = SL_GetString_((char *)"", 0, 19);
    for (i = 0; i < 2442; ++i)
    {
        prev = SL_GetString_((char *)"", 0, 19);
        sv.configstrings[i] = prev;
    }
    SV_InitDvar();
    svs.nextSnapshotEntities = 0;
    svs.nextSnapshotClients = 0;
    SV_InitArchivedSnapshot();
    SV_InitSnapshot();
    svs.snapFlagServerBit ^= 4u;
    Dvar_SetString((dvar_s *)nextmap, (char*)"map_restart");
    Dvar_SetInt((dvar_s *)cl_paused, 0);
    Com_GetBspFilename(filename, 0x40u, server);
    if (!useFastFile->current.enabled)
        Com_LoadBsp(filename);
    CM_LoadMap(filename, &checksum);
    Com_LoadWorld(filename);
    if (!useFastFile->current.enabled)
        Com_UnloadBsp();
    CM_LinkWorld();
    sv_serverId_value = (unsigned __int8)(sv_serverId_value + 16);
    if ((sv_serverId_value & 0xF0) == 0)
        sv_serverId_value += 16;
    Dvar_SetInt((dvar_s *)sv_serverid, sv_serverId_value);
    sv.start_frameTime = com_frameTime;
    sv.state = SS_LOADING;
    if (!useFastFile->current.enabled)
    {
        Com_GetBspFilename(filename, 0x40u, server);
        Com_LoadSoundAliases(filename, "all_mp", SASYS_GAME);
    }
    ProfLoad_Begin("Init game");
    SV_InitGameProgs(savepersist);
    ProfLoad_End();
    for (i = 0; i < 3; ++i)
    {
        svs.time += 100;
        SV_RunFrame();
    }
    SV_CreateBaseline();
    for (i = 0; i < sv_maxclients->current.integer; ++i)
    {
        if (svs.clients[i].header.state >= 2)
        {
            denied = ClientConnect(i, svs.clients[i].scriptId);
            if (denied)
                SV_DropClient(&svs.clients[i], denied, 1);
            else
                svs.clients[i].header.state = 2;
        }
    }
    if (com_sv_running->current.enabled)
    {
        i = 0;
        clienta = svs.clients;
        while (i < sv_maxclients->current.integer)
        {
            if (clienta->header.state >= 2)
            {
                clienta->statPacketsReceived = 0;
                NET_OutOfBandPrint(NS_SERVER, clienta->header.netchan.remoteAddress, "requeststats\n");
            }
            ++i;
            ++clienta;
        }
    }
    if (sv_pure->current.enabled)
    {
        p = FS_LoadedIwdChecksums();
        Dvar_SetString((dvar_s *)sv_iwds, (char *)p);
        if (!strlen(p))
            Com_PrintWarning(15, "WARNING: sv_pure set but no IWD files loaded\n");
        p = FS_LoadedIwdNames();
        Dvar_SetString((dvar_s *)sv_iwdNames, (char *)p);
    }
    else
    {
        Dvar_SetString((dvar_s *)sv_iwds, (char *)"");
        Dvar_SetString((dvar_s *)sv_iwdNames, (char *)"");
    }
    p = FS_ReferencedIwdChecksums();
    Dvar_SetString((dvar_s *)sv_referencedIwds, (char *)p);
    p = FS_ReferencedIwdNames();
    Dvar_SetString((dvar_s *)sv_referencedIwdNames, (char *)p);
    p = DB_ReferencedFFChecksums();
    Dvar_SetString((dvar_s *)sv_referencedFFCheckSums, (char *)p);
    p = DB_ReferencedFFNameList();
    Dvar_SetString((dvar_s *)sv_referencedFFNames, (char *)p);
    SV_SaveSystemInfo();
    sv.state = SS_GAME;
    SV_Heartbeat_f();
    ProfLoad_Deactivate();
    Com_Printf(15, "-----------------------------------\n");
    // LWSS: Remove punkbuster junk
    //if (Dvar_GetBool("sv_punkbuster"))
    //    EnablePbSv();
    //else
    //    DisablePbSv();
    R_EndRemoteScreenUpdate();
    Sys_EndLoadThreadPriorities();
}

void SV_SaveSystemInfo()
{
    char *v0; // eax

    SV_SetSystemInfoConfig();
    v0 = Dvar_InfoString(0, 4);
    SV_SetConfigstring(0, v0);
    dvar_modifiedFlags &= 0xFFFFFBFB;
    SV_SetConfig(20, 128, 256);
    dvar_modifiedFlags &= ~0x100u;
}

bool __cdecl SV_Loaded()
{
    return sv.state == SS_GAME;
}

void __cdecl SV_Init()
{
    DvarLimits min; // [esp+4h] [ebp-18h]
    DvarLimits mina; // [esp+4h] [ebp-18h]

    SV_AddOperatorCommands();
    sv_gametype = Dvar_RegisterString("g_gametype", "war", DVAR_SERVERINFO | DVAR_LATCH, "Current game type");
    Dvar_RegisterString("sv_keywords", (char *)"", DVAR_SERVERINFO, "Server keywords");
    Dvar_RegisterInt("protocol", 1, (DvarLimits)0x100000001LL, DVAR_SERVERINFO | DVAR_ROM, "Protocol version");
    sv_mapname = Dvar_RegisterString("mapname", (char *)"", DVAR_SERVERINFO | DVAR_ROM, "Current map name");
    sv_privateClients = Dvar_RegisterInt(
        "sv_privateClients",
        0,
        (DvarLimits)0x4000000000LL,
        DVAR_SERVERINFO,
        "Maximum number of private clients allowed on the server");
    min.integer.max = Dvar_RegisterInt(
        "ui_maxclients",
        32,
        (DvarLimits)0x4000000001LL,
        DVAR_ARCHIVE | DVAR_SERVERINFO | DVAR_LATCH,
        "The maximum number of clients that can connect to a server")->current.integer;
    min.enumeration.stringCount = 1;
    sv_maxclients = Dvar_RegisterInt(
        "sv_maxclients",
        32,
        min,
        DVAR_ARCHIVE | DVAR_SERVERINFO | DVAR_LATCH,
        "The maximum number of clients that can connect to a server");
    sv_hostname = Dvar_RegisterString("sv_hostname", "CoD4Host", 5u, "Host name of the server");
    sv_clientSideBullets = Dvar_RegisterBool(
        "sv_clientSideBullets",
        1,
        DVAR_SYSTEMINFO,
        "If true, clients will synthesize tracers and bullet impacts");
    sv_punkbuster = Dvar_RegisterBool("sv_punkbuster", 1, 0x15u, "Enable PunkBuster on this server");
    sv_maxRate = Dvar_RegisterInt("sv_maxRate", 5000, (DvarLimits)0x61A800000000LL, 5u, "Maximum bit rate");
    sv_minPing = Dvar_RegisterInt("sv_minPing", 0, (DvarLimits)0x3E700000000LL, 5u, "Minimum ping allowed on the server");
    sv_maxPing = Dvar_RegisterInt("sv_maxPing", 0, (DvarLimits)0x3E700000000LL, 5u, "Maximum ping allowed on the server");
    sv_floodProtect = Dvar_RegisterBool(
        "sv_floodProtect",
        1,
        DVAR_SERVERINFO | DVAR_ARCHIVE,
        "Prevent malicious lagging by flooding the server with commands");
    sv_showCommands = Dvar_RegisterBool("sv_showCommands", 0, 0, "Print client commands in the log file");
    sv_allowAnonymous = Dvar_RegisterBool("sv_allowAnonymous", 0, 4u, "Allow anonymous access");
    sv_disableClientConsole = Dvar_RegisterBool(
        "sv_disableClientConsole",
        0,
        DVAR_SERVERINFO,
        "Disallow remote clients from accessing the console");
    sv_privatePassword = Dvar_RegisterString(
        "sv_privatePassword",
        (char *)"",
        DVAR_NOFLAG,
        "password for the privateClient slots");
    sv_allowDownload = Dvar_RegisterBool("sv_allowDownload", 1, DVAR_ARCHIVE, "Allow auto download of files");
    sv_iwds = Dvar_RegisterString("sv_iwds", (char *)"", DVAR_SYSTEMINFO | DVAR_ROM, "IWD server checksums");
    sv_iwdNames = Dvar_RegisterString("sv_iwdNames", (char *)"", DVAR_SYSTEMINFO | DVAR_ROM, "Names of IWD files used by the server");
    sv_referencedIwds = Dvar_RegisterString(
        "sv_referencedIwds",
        (char *)"",
        DVAR_SYSTEMINFO | DVAR_ROM,
        "Checksum of all referenced IWD files");
    sv_referencedIwdNames = Dvar_RegisterString(
        "sv_referencedIwdNames",
        (char *)"",
        DVAR_SYSTEMINFO | DVAR_ROM,
        "Names of all referenced IWD files");
    sv_FFCheckSums = Dvar_RegisterString("sv_FFCheckSums", (char *)"", DVAR_SYSTEMINFO | DVAR_ROM, "Fast File server checksums");
    sv_FFNames = Dvar_RegisterString("sv_FFNames", (char *)"", DVAR_SYSTEMINFO | DVAR_ROM, "Names of Fast Files used by the server");
    sv_referencedFFCheckSums = Dvar_RegisterString(
        "sv_referencedFFCheckSums",
        (char *)"",
        DVAR_SYSTEMINFO | DVAR_ROM,
        "Checksum of all referenced Fast Files");
    sv_referencedFFNames = Dvar_RegisterString(
        "sv_referencedFFNames",
        (char *)"",
        DVAR_SYSTEMINFO | DVAR_ROM,
        "Names of all referenced Fast Files");
    sv_voice = Dvar_RegisterBool("sv_voice", 0, DVAR_ARCHIVE | DVAR_SERVERINFO | DVAR_SYSTEMINFO, "Use server side voice communications");
    sv_voiceQuality = Dvar_RegisterInt("sv_voiceQuality", 3, (DvarLimits)0x900000000LL, DVAR_SYSTEMINFO, "Voice quality");
    sv_cheats = Dvar_RegisterBool("sv_cheats", 1, DVAR_SYSTEMINFO | DVAR_INIT, "Enable cheats on the server");
    sv_serverid = Dvar_RegisterInt("sv_serverid", 0, (DvarLimits)0x7FFFFFFF80000000LL, DVAR_SYSTEMINFO | DVAR_ROM, "Server identification");
    sv_pure = Dvar_RegisterBool("sv_pure", 0, DVAR_SERVERINFO | DVAR_SYSTEMINFO, "Cannot use modified IWD files");
    rcon_password = Dvar_RegisterString("rcon_password", (char *)"", DVAR_NOFLAG, "Password for the rcon command");
    sv_fps = Dvar_RegisterInt("sv_fps", 20, (DvarLimits)0x3E80000000ALL, DVAR_NOFLAG, "Server frames per second");
    sv_timeout = Dvar_RegisterInt("sv_timeout", 240, (DvarLimits)0x70800000000LL, DVAR_NOFLAG, "seconds without any message");
    sv_connectTimeout = Dvar_RegisterInt(
        "sv_connectTimeout",
        45,
        (DvarLimits)0x70800000000LL,
        DVAR_NOFLAG,
        "seconds without any message when a client is loading");
    sv_zombietime = Dvar_RegisterInt(
        "sv_zombietime",
        2,
        (DvarLimits)0x70800000000LL,
        DVAR_NOFLAG,
        "seconds to sync messages after disconnect");
    sv_reconnectlimit = Dvar_RegisterInt(
        "sv_reconnectlimit",
        3,
        (DvarLimits)0x70800000000LL,
        DVAR_ARCHIVE,
        "minimum seconds between connect messages");
    sv_padPackets = Dvar_RegisterInt("sv_padPackets", 0, (DvarLimits)0x7FFFFFFF00000000LL, DVAR_NOFLAG, "add nop bytes to messages");
    sv_allowedClan1 = Dvar_RegisterString("sv_allowedClan1", (char *)"", DVAR_NOFLAG, "Allow this clan to join the server");
    sv_allowedClan2 = Dvar_RegisterString("sv_allowedClan2", (char *)"", DVAR_NOFLAG, "Allow this clan to join the server");
    sv_packet_info = Dvar_RegisterBool("sv_packet_info", 0, DVAR_NOFLAG, "Enable packet info debugging information");
    sv_showAverageBPS = Dvar_RegisterBool("sv_showAverageBPS", 0, DVAR_NOFLAG, "Show average bytes per second for net debugging");
    mina.value.max = 3600.0;
    mina.value.min = 0.0;
    sv_kickBanTime = Dvar_RegisterFloat(
        "sv_kickBanTime",
        300.0,
        mina,
        DVAR_NOFLAG,
        "Time in seconds for a player to be banned from the server after being kicked");
    sv_botsPressAttackBtn = Dvar_RegisterBool("sv_botsPressAttackBtn", 1, DVAR_NOFLAG, "Allow testclients to press attack button");
    sv_debugMessageKey = Dvar_RegisterBool("sv_debugMessageKey", 0, DVAR_NOFLAG, "net message key generation debugging");
    sv_debugPacketContents = Dvar_RegisterBool(
        "sv_debugPacketContents",
        0,
        DVAR_NOFLAG,
        "print out the contents of every snapshot (VERY SLOW)");
    sv_debugPacketContentsForClientThisFrame = Dvar_RegisterBool(
        "sv_debugPacketContentsForClientThisFrame",
        0,
        DVAR_NOFLAG,
        "set to true to get the next snapshot for this client");
    sv_mapRotation = Dvar_RegisterString("sv_mapRotation", (char *)"", 0, "List of maps for the server to play");
    sv_mapRotationCurrent = Dvar_RegisterString(
        "sv_mapRotationCurrent",
        (char *)"",
        DVAR_NOFLAG,
        "Current map in the map rotation");
    sv_debugRate = Dvar_RegisterBool("sv_debugRate", 0, 0, "Enable snapshot rate debugging info");
    sv_debugReliableCmds = Dvar_RegisterBool(
        "sv_debugReliableCmds",
        0,
        DVAR_NOFLAG,
        "Enable debugging information for 'reliable' commands");
    nextmap = Dvar_RegisterString("nextmap", (char *)"", DVAR_NOFLAG, "Next map to play");
    sv_wwwDownload = Dvar_RegisterBool("sv_wwwDownload", 0, DVAR_ARCHIVE, "Enable http downloads");
    sv_wwwBaseURL = Dvar_RegisterString(
        "sv_wwwBaseURL",
        (char *)"",
        DVAR_ARCHIVE,
        "The base url for files downloaded via http");
    sv_wwwDlDisconnected = Dvar_RegisterBool(
        "sv_wwwDlDisconnected",
        0,
        DVAR_ARCHIVE,
        "Should clients stay connected while downloading?");
    sv_loadMyChanges = Dvar_RegisterBool("sv_loadMyChanges", 0, 0, "Load my changes fast file on devmap.");
    sv_debugPlayerstate = Dvar_RegisterBool(
        "sv_debugPlayerstate",
        0,
        DVAR_NOFLAG,
        "Print out what fields are changing in the playerstate");
    sv_clientArchive = Dvar_RegisterBool(
        "sv_clientArchive",
        1,
        DVAR_NOFLAG,
        "Have the clients archive data to save bandwidth on the server");
    Dvar_RegisterBool("clientSideEffects", 1, DVAR_CHEAT, "Enable loading _fx.gsc files on the client");
}

void __cdecl SV_DropAllClients()
{
    client_t *drop; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    i = 0;
    drop = svs.clients;
    while (i < sv_maxclients->current.integer)
    {
        if (drop->header.state >= 2)
            SV_DropClient(drop, "EXE_DISCONNECTED", 1);
        ++i;
        ++drop;
    }
}

void __cdecl SV_Shutdown(const char *finalmsg)
{
    int client; // [esp+0h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\server_mp\\sv_init_mp.cpp", 1554, 0, "%s", "Sys_IsMainThread()");
    if (com_sv_running && com_sv_running->current.enabled)
    {
        Com_SyncThreads();
        Com_Printf(15, "----- Server Shutdown -----\n");
        SV_FinalMessage(finalmsg);
        KISAK_NULLSUB();
        SV_MasterShutdown();
        SV_ShutdownGameProgs();
        SV_DropAllClients();
        SV_WriteEntityFieldNumbers();
        SV_FreeClients();
        SV_ClearServer();
        Dvar_SetBool((dvar_s *)com_sv_running, 0);
        for (client = 0; client < 1; ++client)
        {
            if (CL_IsLocalClientActive(client))
                CL_Disconnect(client);
        }
        memset(&svs, 0, sizeof(svs));
        bgs = 0;
        Com_Printf(15, "---------------------------\n");
    }
}

void __cdecl SV_FinalMessage(const char *message)
{
    int j; // [esp+0h] [ebp-38h]
    client_t *client; // [esp+4h] [ebp-34h]
    bool translationForReason; // [esp+Bh] [ebp-2Dh]
    msg_t msg; // [esp+Ch] [ebp-2Ch] BYREF
    int i; // [esp+34h] [ebp-4h]

    translationForReason = SEH_StringEd_GetString(message) != 0;
    for (j = 0; j < 2; ++j)
    {
        i = 0;
        client = svs.clients;
        while (i < sv_maxclients->current.integer)
        {
            if (client->header.state >= 2)
            {
                if (client->header.netchan.remoteAddress.type != NA_LOOPBACK)
                    SV_SendDisconnect(client, client->header.state, message, translationForReason, client->name);
                client->nextSnapshotTime = -1;
                SV_SetServerStaticHeader();
                SV_BeginClientSnapshot(client, &msg);
                if (client->header.state == 4 || client->header.state == 1)
                    SV_WriteSnapshotToClient(client, &msg);
                SV_EndClientSnapshot(client, &msg);
                SV_GetServerStaticHeader();
            }
            ++i;
            ++client;
        }
    }
}

void __cdecl SV_CheckThread()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\server_mp\\sv_init_mp.cpp", 1663, 0, "%s", "Sys_IsMainThread()");
}


