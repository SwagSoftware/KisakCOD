#include "sv_game.h"
#include <qcommon/mem_track.h>

#include <xanim/dobj.h>
#include <xanim/dobj_utils.h>
#include "sv_world.h"
#include <client/client.h>
#include <universal/com_files.h>
#include <universal/com_sndalias.h>
#include <qcommon/com_bsp.h>
#include <qcommon/threads.h>

#ifdef KISAK_MP
#include <game_mp/g_main_mp.h>
#include <game_mp/g_public_mp.h>
#elif KISAK_SP
#include <game/game_public.h>
#endif

#ifdef KISAK_MP
#define SKEL_MEMORY_SIZE 0x40000
#elif KISAK_SP
#define SKEL_MEMORY_SIZE 0x80000
#endif

#define SKEL_MEM_ALIGNMENT 16

char g_sv_skel_memory[SKEL_MEMORY_SIZE];
char *g_sv_skel_memory_start;
int gameInitialized;


void __cdecl TRACK_sv_game()
{
    track_static_alloc_internal(g_sv_skel_memory, SKEL_MEMORY_SIZE, "g_sv_skel_memory", 11);
}

gentity_s *__cdecl SV_GentityNum(int num)
{
    return (gentity_s *)((char *)sv.gentities + num * sv.gentitySize);
}

playerState_s *__cdecl SV_GameClientNum(int num)
{
    return (playerState_s *)((char *)sv.gameClients + num * sv.gameClientSize);
}

svEntity_s *__cdecl SV_SvEntityForGentity(const gentity_s *gEnt)
{
    if (!gEnt || gEnt->s.number >= ARRAY_COUNT(sv.svEntities))
        Com_Error(ERR_DROP, "SV_SvEntityForGentity: bad gEnt");
    return &sv.svEntities[gEnt->s.number];
}

gentity_s *__cdecl SV_GEntityForSvEntity(svEntity_s *svEnt)
{
    return SV_GentityNum(svEnt - sv.svEntities);
}

bool __cdecl SV_EntityContact(const float *mins, const float *maxs, const gentity_s *gEnt)
{
    unsigned int model; // [esp+8h] [ebp-40h]
    float dist; // [esp+Ch] [ebp-3Ch]
    float dista; // [esp+Ch] [ebp-3Ch]
    float distSqrd; // [esp+10h] [ebp-38h]
    float distSqrda; // [esp+10h] [ebp-38h]
    trace_t trace; // [esp+14h] [ebp-34h] BYREF
    float center[2]; // [esp+40h] [ebp-8h] BYREF

    if ((gEnt->r.svFlags & 0x60) != 0)
    {
        if ((gEnt->r.svFlags & 0x20) != 0)
        {
            if (gEnt->r.mins[2] != 0.0)
                MyAssertHandler(".\\server\\sv_game.cpp", 337, 0, "%s", "!gEnt->r.mins[2]");
            if (gEnt->r.currentOrigin[2] < (double)maxs[2])
            {
                if (mins[2] < gEnt->r.currentOrigin[2] + gEnt->r.maxs[2])
                {
                    center[0] = *mins + *maxs;
                    center[1] = mins[1] + maxs[1];
                    center[0] = 0.5 * center[0];
                    center[1] = 0.5 * center[1];
                    dist = *maxs - center[0] + gEnt->r.maxs[0];
                    distSqrd = dist * dist;
                    return distSqrd > Vec2DistanceSq(gEnt->r.currentOrigin, center);
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                return 0;
            }
        }
        else
        {
            if ((gEnt->r.svFlags & 0x40) == 0)
                MyAssertHandler(".\\server\\sv_game.cpp", 350, 0, "%s", "gEnt->r.svFlags & SVF_DISK");
            center[0] = *mins + *maxs;
            center[1] = mins[1] + maxs[1];
            center[0] = 0.5 * center[0];
            center[1] = 0.5 * center[1];
            dista = *maxs - center[0] + gEnt->r.maxs[0] - 64.0;
            distSqrda = dista * dista;
            return distSqrda <= Vec2DistanceSq(gEnt->r.currentOrigin, center);
        }
    }
    else
    {
        model = SV_ClipHandleForEntity(gEnt);
        CM_TransformedBoxTraceExternal(
            &trace,
            vec3_origin,
            vec3_origin,
            mins,
            maxs,
            model | 0xFFFFFFFF00000000uLL,
            gEnt->r.currentOrigin,
            gEnt->r.currentAngles);
        return trace.startsolid;
    }
}

void __cdecl SV_GetServerinfo(char *buffer, int bufferSize)
{
    if (bufferSize < 1)
        Com_Error(ERR_DROP, "SV_GetServerinfo: bufferSize == %i", bufferSize);

    I_strncpyz(buffer, Dvar_InfoString(0, 4), bufferSize);
}

void __cdecl SV_LocateGameData(
    gentity_s *gEnts,
    int numGEntities,
    int sizeofGEntity_t,
    playerState_s *clients,
    int sizeofGameClient)
{
    sv.gentities = gEnts;
    sv.gentitySize = sizeofGEntity_t;
    sv.num_entities = numGEntities;
    sv.gameClients = clients;
    sv.gameClientSize = sizeofGameClient;
}

void __cdecl SV_GetUsercmd(int clientNum, usercmd_s *cmd)
{
    iassert(clientNum >= 0);
#ifdef KISAK_MP
    iassert(sv_maxclients->current.integer >= 1 && sv_maxclients->current.integer <= 64);
    iassert(clientNum < sv_maxclients->current.integer);
#endif
    memcpy(cmd, &svs.clients[clientNum].lastUsercmd, sizeof(usercmd_s));
}

XModel *__cdecl SV_XModelGet(char *name)
{
    return XModelPrecache(
        name,
        (void *(__cdecl *)(int))SV_AllocXModelPrecache,
        (void *(__cdecl *)(int))SV_AllocXModelPrecacheColl);
}

unsigned __int8 *__cdecl SV_AllocXModelPrecache(unsigned int size)
{
    return Hunk_Alloc(size, "SV_AllocXModelPrecache", 21);
}

unsigned __int8 *__cdecl SV_AllocXModelPrecacheColl(unsigned int size)
{
    return Hunk_Alloc(size, "SV_AllocXModelPrecacheColl", 27);
}

void __cdecl SV_DObjDumpInfo(gentity_s *ent)
{
    const DObj_s *obj; // [esp+0h] [ebp-4h]

    if (com_developer->current.integer)
    {
        obj = Com_GetServerDObj(ent->s.number);
        if (obj)
            DObjDumpInfo(obj);
        else
            Com_Printf(15, "no model.\n");
    }
}

void __cdecl SV_ResetSkeletonCache()
{
    if (!++sv.skelTimeStamp)
        sv.skelTimeStamp = 1;
    g_sv_skel_memory_start = g_sv_skel_memory;
    sv.skelMemPos = 0;
}

bool __cdecl SV_DObjCreateSkelForBone(DObj_s *obj, int boneIndex)
{
    char *buf; // [esp+0h] [ebp-8h]
    unsigned int len; // [esp+4h] [ebp-4h]

    if (DObjSkelExists(obj, sv.skelTimeStamp))
        return DObjSkelIsBoneUpToDate(obj, boneIndex);
    len = DObjGetAllocSkelSize(obj);
    buf = SV_AllocSkelMemory(len);
    DObjCreateSkel(obj, buf, sv.skelTimeStamp);
    return 0;
}

int warnCount_2;
char *__cdecl SV_AllocSkelMemory(unsigned int size)
{
    char *result; // [esp+0h] [ebp-4h]
    unsigned int sizea; // [esp+Ch] [ebp+8h]

    iassert(size);
    sizea = (size + 15) & 0xFFFFFFF0;
    iassert(size <= sizeof(g_sv_skel_memory) - SKEL_MEM_ALIGNMENT);
    iassert(g_sv_skel_memory_start);

    while (1)
    {
        result = &g_sv_skel_memory_start[sv.skelMemPos];
        sv.skelMemPos += sizea;
        if (sv.skelMemPos <= (sizeof(g_sv_skel_memory) - SKEL_MEM_ALIGNMENT))
            break;
        if (warnCount_2 != sv.skelTimeStamp)
        {
            warnCount_2 = sv.skelTimeStamp;
            Com_PrintWarning(15, "WARNING: SV_SKEL_MEMORY_SIZE exceeded\n");
        }
        SV_ResetSkeletonCache();
    }

    iassert(result);
    return result;
}

int __cdecl SV_DObjCreateSkelForBones(DObj_s *obj, int *partBits)
{
    char *buf; // [esp+0h] [ebp-8h]
    unsigned int len; // [esp+4h] [ebp-4h]

    if (DObjSkelExists(obj, sv.skelTimeStamp))
        return DObjSkelAreBonesUpToDate(obj, partBits);
    len = DObjGetAllocSkelSize(obj);
    buf = SV_AllocSkelMemory(len);
    DObjCreateSkel(obj, buf, sv.skelTimeStamp);
    return 0;
}

int __cdecl SV_DObjUpdateServerTime(gentity_s *ent, float dtime, int bNotify)
{
    DObj_s *obj; // [esp+8h] [ebp-4h]

    obj = Com_GetServerDObj(ent->s.number);
    if (obj)
        return DObjUpdateServerInfo(obj, dtime, bNotify);
    else
        return 0;
}

void __cdecl SV_DObjInitServerTime(gentity_s *ent, float dtime)
{
    DObj_s *obj; // [esp+4h] [ebp-4h]

    obj = Com_GetServerDObj(ent->s.number);
    if (obj)
        DObjInitServerTime(obj, dtime);
}

int __cdecl SV_DObjGetBoneIndex(const gentity_s *ent, unsigned int boneName)
{
    const DObj_s *obj; // [esp+0h] [ebp-8h]
    unsigned __int8 index; // [esp+7h] [ebp-1h] BYREF

    obj = Com_GetServerDObj(ent->s.number);
    if (!obj)
        return -1;
    index = -2;
    if (DObjGetBoneIndex(obj, boneName, &index))
        return index;
    else
        return -1;
}

DObjAnimMat *__cdecl SV_DObjGetMatrixArray(const gentity_s *ent)
{
    const DObj_s *obj; // [esp+0h] [ebp-4h]

    obj = Com_GetServerDObj(ent->s.number);
    iassert(obj);
    return DObjGetRotTransArray(obj);
}
void __cdecl SV_DObjDisplayAnim(gentity_s *ent, const char *header)
{
    DObj_s *obj; // [esp+0h] [ebp-4h]

    obj = Com_GetServerDObj(ent->s.number);
    if (obj)
        DObjDisplayAnim(obj, header);
}
void __cdecl SV_DObjGetBounds(gentity_s *ent, float *mins, float *maxs)
{
    const DObj_s *obj; // [esp+0h] [ebp-4h]

    obj = Com_GetServerDObj(ent->s.number);
    iassert(obj);
    DObjGetBounds(obj, mins, maxs);
}

XAnimTree_s *__cdecl SV_DObjGetTree(gentity_s *ent)
{
    const DObj_s *obj; // [esp+4h] [ebp-4h]

    obj = Com_GetServerDObj(ent->s.number);
    if (obj)
        return DObjGetTree(obj);
    else
        return 0;
}

int boxVerts_0[24][3] =
{
  { 0, 0, 0 },
  { 1, 0, 0 },
  { 0, 0, 0 },
  { 0, 1, 0 },
  { 1, 1, 0 },
  { 1, 0, 0 },
  { 1, 1, 0 },
  { 0, 1, 0 },
  { 0, 0, 1 },
  { 1, 0, 1 },
  { 0, 0, 1 },
  { 0, 1, 1 },
  { 1, 1, 1 },
  { 1, 0, 1 },
  { 1, 1, 1 },
  { 0, 1, 1 },
  { 0, 0, 0 },
  { 0, 0, 1 },
  { 1, 0, 0 },
  { 1, 0, 1 },
  { 0, 1, 0 },
  { 0, 1, 1 },
  { 1, 1, 0 },
  { 1, 1, 1 }
}; // idb

void __cdecl SV_XModelDebugBoxes(gentity_s *ent)
{
    const XModel *Model; // eax
    float v2; // [esp+24h] [ebp-2F0h]
    float v3; // [esp+28h] [ebp-2ECh]
    float v4; // [esp+2Ch] [ebp-2E8h]
    float v5; // [esp+30h] [ebp-2E4h]
    float result[3]; // [esp+34h] [ebp-2E0h] BYREF
    float v7; // [esp+40h] [ebp-2D4h]
    float v8; // [esp+44h] [ebp-2D0h]
    float v9; // [esp+48h] [ebp-2CCh]
    float v10; // [esp+4Ch] [ebp-2C8h]
    float v11; // [esp+50h] [ebp-2C4h]
    DObjAnimMat *boneMatrix; // [esp+54h] [ebp-2C0h]
    unsigned int j; // [esp+58h] [ebp-2BCh]
    XBoneInfo *boneInfoArray[128]; // [esp+5Ch] [ebp-2B8h] BYREF
    int numBones; // [esp+260h] [ebp-B4h]
    DObj_s *obj; // [esp+264h] [ebp-B0h]
    float start[3]; // [esp+268h] [ebp-ACh] BYREF
    float end[3]; // [esp+274h] [ebp-A0h] BYREF
    int size; // [esp+280h] [ebp-94h]
    float boneAxis[4][3]; // [esp+284h] [ebp-90h] BYREF
    int localBoneIndex; // [esp+2B4h] [ebp-60h]
    int (*tempBoxVerts)[3]; // [esp+2B8h] [ebp-5Ch]
    float org[3]; // [esp+2BCh] [ebp-58h] BYREF
    XBoneInfo *boneInfo; // [esp+2C8h] [ebp-4Ch]
    float color[4]; // [esp+2CCh] [ebp-48h] BYREF
    float axis[3][3]; // [esp+2DCh] [ebp-38h] BYREF
    float vec[3]; // [esp+300h] [ebp-14h] BYREF
    int modelCount; // [esp+30Ch] [ebp-8h]
    int modelIndex; // [esp+310h] [ebp-4h]

    obj = Com_GetServerDObj(ent->s.number);
    if (!obj)
        MyAssertHandler(".\\server\\sv_game.cpp", 869, 0, "%s", "obj");
    numBones = DObjNumBones(obj);
    if (numBones > 128)
        MyAssertHandler(".\\server\\sv_game.cpp", 872, 0, "%s", "numBones <= DOBJ_MAX_PARTS");
    DObjGetBoneInfo(obj, boneInfoArray);
    boneMatrix = DObjGetRotTransArray(obj);
    color[0] = 1.0;
    color[1] = 1.0;
    color[2] = 1.0;
    color[3] = 0.0;
    AnglesToAxis(ent->r.currentAngles, axis);
    modelCount = DObjGetNumModels(obj);
    for (modelIndex = 0; modelIndex < modelCount; ++modelIndex)
    {
        if (!DObjIgnoreCollision(obj, modelIndex))
        {
            Model = DObjGetModel(obj, modelIndex);
            size = XModelNumBones(Model);
            localBoneIndex = 0;
            while (localBoneIndex < size)
            {
                boneInfo = boneInfoArray[localBoneIndex];
                tempBoxVerts = boxVerts_0;
                if ((COERCE_UNSIGNED_INT(boneMatrix->quat[0]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(boneMatrix->quat[1]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(boneMatrix->quat[2]) & 0x7F800000) == 0x7F800000
                    || (COERCE_UNSIGNED_INT(boneMatrix->quat[3]) & 0x7F800000) == 0x7F800000)
                {
                    MyAssertHandler(
                        "c:\\trees\\cod3\\src\\bgame\\../xanim/xanim_public.h",
                        432,
                        0,
                        "%s",
                        "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])");
                }
                if ((COERCE_UNSIGNED_INT(boneMatrix->transWeight) & 0x7F800000) == 0x7F800000)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\src\\bgame\\../xanim/xanim_public.h",
                        433,
                        0,
                        "%s",
                        "!IS_NAN(mat->transWeight)");
                Vec3Scale(boneMatrix->quat, boneMatrix->transWeight, result);
                v10 = result[0] * boneMatrix->quat[0];
                v3 = result[0] * boneMatrix->quat[1];
                v8 = result[0] * boneMatrix->quat[2];
                v11 = result[0] * boneMatrix->quat[3];
                v2 = result[1] * boneMatrix->quat[1];
                v9 = result[1] * boneMatrix->quat[2];
                v7 = result[1] * boneMatrix->quat[3];
                v4 = result[2] * boneMatrix->quat[2];
                v5 = result[2] * boneMatrix->quat[3];
                boneAxis[0][0] = 1.0 - (v2 + v4);
                boneAxis[0][1] = v3 + v5;
                boneAxis[0][2] = v8 - v7;
                boneAxis[1][0] = v3 - v5;
                boneAxis[1][1] = 1.0 - (v10 + v4);
                boneAxis[1][2] = v9 + v11;
                boneAxis[2][0] = v8 + v7;
                boneAxis[2][1] = v9 - v11;
                boneAxis[2][2] = 1.0 - (v10 + v2);
                boneAxis[3][0] = boneMatrix->trans[0];
                boneAxis[3][1] = boneMatrix->trans[1];
                boneAxis[3][2] = boneMatrix->trans[2];
                for (j = 0; j < 0xC; ++j)
                {
                    org[0] = boneInfo->bounds[(*tempBoxVerts)[0]][0];
                    org[1] = boneInfo->bounds[(*tempBoxVerts)[1]][1];
                    org[2] = boneInfo->bounds[(*tempBoxVerts)[2]][2];
                    MatrixTransformVector43(org, boneAxis, vec);
                    MatrixTransformVector(vec, axis, start);
                    Vec3Add(start, ent->r.currentOrigin, start);
                    org[0] = boneInfo->bounds[(*++tempBoxVerts)[0]][0];
                    org[1] = boneInfo->bounds[(*tempBoxVerts)[1]][1];
                    org[2] = boneInfo->bounds[(*tempBoxVerts)[2]][2];
                    MatrixTransformVector43(org, boneAxis, vec);
                    MatrixTransformVector(vec, axis, end);
                    Vec3Add(end, ent->r.currentOrigin, end);
                    ++tempBoxVerts;
                    CL_AddDebugLine(start, end, color, 0, 0, 1);
                }
                ++localBoneIndex;
                ++boneMatrix;
            }
        }
    }
}
bool __cdecl SV_DObjExists(gentity_s *ent)
{
    return Com_GetServerDObj(ent->s.number) != 0;
}
void __cdecl SV_track_shutdown()
{
    track_shutdown(2);
}

int __cdecl SV_GameCommand()
{
    if (sv.state == SS_GAME)
        return ConsoleCommand();
    else
        return 0;
}

#ifdef KISAK_SP
#include <game/g_local.h>
#include <client/cl_demo.h>
#include "sv_public.h"
#include <win32/win_local.h>
#include <client/cl_input.h>

void __cdecl SV_CheckLoadLevel(SaveGame *save)
{
    G_CheckLoadGame(sv.checksum, save);
    com_time = G_GetTime();
    sv.timeResidual = 49;
    if (CL_DemoPlaying())
        CL_ReadDemoMessagesUntilNextSnap();
    else
        SV_SendClientMessages();
    Hunk_CheckTempMemoryClear();
    Hunk_CheckTempMemoryHighClear();
}

static void SV_FreeReliableCommandsForClient(client_t *cl)
{
    Com_Memset(&cl->reliableCommands, 0, 12);
}
static void SV_ShutdownGameVM(int clearScripts)
{
    iassert(Sys_IsMainThread());
    SV_AutoSaveDemo("autosave/autoreplay", 0, 50, 0);
    if (!sv.demo.nextLevelplaying)
        SV_ShutdownDemo();
    sv.state = SS_DEAD;
    G_ShutdownGame(clearScripts);
    svs.clients->gentity = 0;
    SV_FreeReliableCommandsForClient(svs.clients);
}

void __cdecl SV_ShutdownGameProgs()
{
    iassert(Sys_IsMainThread());

    Com_SyncThreads();
    if (gameInitialized)
    {
        SV_ShutdownGameVM(1);
        SV_ShutdownDemo();
        gameInitialized = 0;
    }
    else
    {
        iassert(!sv.state);
    }
}

void __cdecl SV_InitGameVM(unsigned int randomSeed, int restart, int savegame, SaveGame **save, int loadScripts)
{
    iassert(save);
    ProfLoad_Begin("Start map");
    SV_StartMap(randomSeed);
    ProfLoad_End();
    G_ResetEntityParsePoint();
    if (!++sv.skelTimeStamp)
        sv.skelTimeStamp = 1;
    sv.skelMemPos = 0;
    g_sv_skel_memory_start = (char *)((unsigned int)&g_sv_skel_memory[15] & 0xFFFFFFF0);
    SND_ErrorCleanup();
    ProfLoad_Begin("Init game");
    Sys_LoadingKeepAlive();
    G_InitGame(randomSeed, restart, sv.checksum, loadScripts, savegame, save);
    Sys_LoadingKeepAlive();
    ProfLoad_End();
    ProfLoad_Begin("Settle game");
    SV_Settle();
    ProfLoad_End();
    ProfLoad_Begin("Connecting");
    SV_DirectConnect();
    CL_ConnectResponse();
    SV_ClientEnterWorld(svs.clients);
    ProfLoad_End();
    if (!restart || !*save)
    {
        R_BeginRemoteScreenUpdate();
        sv.demo.forwardMsec -= 50;
        if (sv.demo.forwardMsec < 0)
            sv.demo.forwardMsec = 0;
        ProfLoad_Begin("Server pre-frame");
        SV_PreFrame();
        ProfLoad_End();
        ProfLoad_Begin("Load game level");
        G_LoadLevel();
        ProfLoad_End();
        R_EndRemoteScreenUpdate();
    }
}

void __cdecl SV_RestartGameProgs(unsigned int randomSeed, int savegame, SaveGame **save, int loadScripts)
{
    iassert(Sys_IsMainThread());
    iassert(gameInitialized);
    R_BeginRemoteScreenUpdate();
    SV_ShutdownGameVM(loadScripts);
    iassert(save);

    if (loadScripts)
        Com_SetScriptSettings();

    com_fixedConsolePosition = 0;
    R_EndRemoteScreenUpdate();
    SV_InitGameVM(randomSeed, 1, savegame, save, loadScripts);
}

void __cdecl SV_InitGameProgs(unsigned int randomSeed, int savegame, SaveGame **save)
{
    iassert(save);
    gameInitialized = 1;
    SV_InitGameVM(randomSeed, 0, savegame, save, 1);
}


bool SV_SetBrushModel(gentity_s *ent)
{
    unsigned int index; // r3
    float mins[4]; // [sp+50h] [-30h] BYREF
    float maxs[4]; // [sp+60h] [-20h] BYREF

    iassert(ent->r.inuse);

    if (!CM_ClipHandleIsValid(ent->s.index.item))
        return false;

    CM_ModelBounds(ent->s.index.item, mins, maxs);

    ent->r.mins[0] = mins[0];
    ent->r.mins[1] = mins[1];
    ent->r.mins[2] = mins[2];

    ent->r.maxs[0] = maxs[0];
    ent->r.maxs[1] = maxs[1];
    ent->r.maxs[2] = maxs[2];

    index = ent->s.index.item;
    ent->r.bmodel = 1;

    ent->r.contents = CM_ContentsOfModel(index);

    SV_LinkEntity(ent);

    return true;
}

void SV_SetCheckSum(int checksum)
{
    sv.checksum = checksum;
}

int SV_DObjSetRotTransIndex(const gentity_s *ent, int *partBits, int boneIndex)
{
    const DObj_s *obj; // r31

    obj = Com_GetServerDObj(ent->s.number);

    iassert(obj);

    return DObjSetRotTransIndex((DObj_s*)obj, partBits, boneIndex);
}

DObjAnimMat *SV_DObjGetRotTransArray(const gentity_s *ent)
{
    const DObj_s *obj; // r31

    obj = Com_GetServerDObj(ent->s.number);

    iassert(obj);

    return DObjGetRotTransArray(obj);
}

const char *SV_Archived_Dvar_GetVariantString(const char *dvarName)
{
    const char *VariantString; // r31

    if (sv.demo.playing)
        return SV_Demo_Dvar_GetVariantString();
    VariantString = Dvar_GetVariantString(dvarName);
    SV_Record_Dvar_GetVariantString(VariantString);
    return VariantString;
}

void SV_SetUsercmdButtonsWeapons(int buttons, int weapon, int offhand)
{
    CL_SetUsercmdButtonsWeapons(buttons, weapon, offhand);
}

#endif // KISAK_SP

#ifdef KISAK_MP
bool __cdecl SV_inSnapshot(const float *origin, int iEntityNum)
{
    int clientcluster; // [esp+4h] [ebp-24h]
    float fogOpaqueDistSqrd; // [esp+8h] [ebp-20h]
    svEntity_s *svEnt; // [esp+Ch] [ebp-1Ch]
    int l; // [esp+10h] [ebp-18h]
    unsigned int leafnum; // [esp+14h] [ebp-14h]
    gentity_s *ent; // [esp+18h] [ebp-10h]
    int i; // [esp+1Ch] [ebp-Ch]
    unsigned __int8 *bitvector; // [esp+20h] [ebp-8h]

    ent = SV_GentityNum(iEntityNum);
    if (!ent->r.linked)
        return 0;
    if (ent->r.broadcastTime)
        return 1;
    if ((ent->r.svFlags & 1) != 0)
        return 0;
    if ((ent->r.svFlags & 0x18) != 0)
        return 1;
    svEnt = SV_SvEntityForGentity(ent);
    leafnum = CM_PointLeafnum(origin);
    if (!svEnt->numClusters)
        return 0;
    clientcluster = CM_LeafCluster(leafnum);
    bitvector = CM_ClusterPVS(clientcluster);
    l = 0;
    for (i = 0; i < svEnt->numClusters; ++i)
    {
        l = svEnt->clusternums[i];
        if (((1 << (l & 7)) & bitvector[l >> 3]) != 0)
            break;
    }
    if (i == svEnt->numClusters)
    {
        if (!svEnt->lastCluster)
            return 0;
        while (l <= svEnt->lastCluster && ((1 << (l & 7)) & bitvector[l >> 3]) == 0)
            ++l;
        if (l == svEnt->lastCluster)
            return 0;
    }
    fogOpaqueDistSqrd = G_GetFogOpaqueDistSqrd();
    return fogOpaqueDistSqrd == 3.402823466385289e38
        || !BoxDistSqrdExceeds(ent->r.absmin, ent->r.absmax, origin, fogOpaqueDistSqrd);
}

void __cdecl SV_SetGameEndTime(int gameEndTime)
{
    char *v1; // eax
    char lastGameEndTime[12]; // [esp+0h] [ebp-10h] BYREF

    SV_GetConfigstring(0xBu, lastGameEndTime, 12);
    if ((int)abs(atoi(lastGameEndTime) - gameEndTime) > 500)
    {
        v1 = va("%i", gameEndTime);
        SV_SetConfigstring(11, v1);
    }
}

void __cdecl SV_SetMapCenter(float *mapCenter)
{
    char *v1; // eax

    svs.mapCenter[0] = *mapCenter;
    svs.mapCenter[1] = mapCenter[1];
    svs.mapCenter[2] = mapCenter[2];
    v1 = va("%f %f %f", *mapCenter, mapCenter[1], mapCenter[2]);
    SV_SetConfigstring(12, v1);
}

void __cdecl SV_GameDropClient(int clientNum, const char *reason)
{
    if (sv_maxclients->current.integer < 1 || sv_maxclients->current.integer > 64)
        MyAssertHandler(
            ".\\server\\sv_game.cpp",
            184,
            0,
            "%s\n\t(sv_maxclients->current.integer) = %i",
            "(sv_maxclients->current.integer >= 1 && sv_maxclients->current.integer <= 64)",
            sv_maxclients->current.integer);
    if (clientNum >= 0 && clientNum < sv_maxclients->current.integer)
        SV_DropClient(&svs.clients[clientNum], reason, 1);
}

bool __cdecl SV_MapExists(char *name)
{
    char fullpath[256]; // [esp+0h] [ebp-108h] BYREF
    const char *basename; // [esp+104h] [ebp-4h]

    basename = SV_GetMapBaseName(name);
    Com_GetBspFilename(fullpath, 0x100u, basename);
    return FS_ReadFile(fullpath, 0) >= 0;
}

char *__cdecl SV_GetGuid(int clientNum)
{
    if (!sv_maxclients)
        MyAssertHandler(".\\server\\sv_game.cpp", 981, 0, "%s", "sv_maxclients");
    if (clientNum < 0 || clientNum >= sv_maxclients->current.integer)
        return (char *)"";
    else
        return svs.clients[clientNum].cdkeyHash;
}

int __cdecl SV_GetClientPing(int clientNum)
{
    return svs.clients[clientNum].ping;
}

bool __cdecl SV_IsLocalClient(int clientNum)
{
    return NET_IsLocalAddress(svs.clients[clientNum].header.netchan.remoteAddress);
}

void __cdecl SV_SetGametype()
{
    char gametype[64]; // [esp+0h] [ebp-48h] BYREF
    char *s; // [esp+44h] [ebp-4h]

    Dvar_RegisterString("g_gametype", "war", DVAR_SERVERINFO | DVAR_LATCH, "Game Type");
    if (com_sv_running->current.enabled && G_GetSavePersist())
        I_strncpyz(gametype, sv.gametype, 64);
    else
        I_strncpyz(gametype, (char *)sv_gametype->current.integer, 64);
    for (s = gametype; *s; ++s)
        *s = tolower(*s);
    if (!Scr_IsValidGameType(gametype))
    {
        Com_Printf(15, "g_gametype %s is not a valid gametype, defaulting to dm\n", gametype);
        strcpy(gametype, "dm");
    }
    Dvar_SetString((dvar_s *)sv_gametype, gametype);
}

void __cdecl SV_InitGameVM(int restart, int savepersist)
{
    unsigned int v2; // eax
    int i; // [esp+0h] [ebp-4h]

    G_ResetEntityParsePoint();
    SV_ResetSkeletonCache();
    v2 = Sys_MillisecondsRaw();
    G_InitGame(svs.time, v2, restart, savepersist);
    if (sv_maxclients->current.integer < 1 || sv_maxclients->current.integer > 64)
        MyAssertHandler(
            ".\\server\\sv_game.cpp",
            1112,
            0,
            "%s\n\t(sv_maxclients->current.integer) = %i",
            "(sv_maxclients->current.integer >= 1 && sv_maxclients->current.integer <= 64)",
            sv_maxclients->current.integer);
    for (i = 0; i < sv_maxclients->current.integer; ++i)
        svs.clients[i].gentity = 0;
}

void __cdecl SV_GameSendServerCommand(int clientNum, svscmd_type type, const char *text)
{
    if (clientNum == -1)
    {
        SV_SendServerCommand(0, type, "%s", text);
    }
    else
    {
        iassert(sv_maxclients->current.integer >= 1 && sv_maxclients->current.integer <= 64);
        if (clientNum >= 0 && clientNum < sv_maxclients->current.integer)
            SV_SendServerCommand(&svs.clients[clientNum], type, "%s", text);
    }
}

bool __cdecl SV_SetBrushModel(gentity_s *ent)
{
    float mins[3]; // [esp+8h] [ebp-18h] BYREF
    float maxs[3]; // [esp+14h] [ebp-Ch] BYREF

    iassert(ent->r.inuse);

    if (!CM_ClipHandleIsValid(ent->s.index.brushmodel))
        return 0;

    CM_ModelBounds(ent->s.index.brushmodel, mins, maxs);

    ent->r.mins[0] = mins[0];
    ent->r.mins[1] = mins[1];
    ent->r.mins[2] = mins[2];

    ent->r.maxs[0] = maxs[0];
    ent->r.maxs[1] = maxs[1];
    ent->r.maxs[2] = maxs[2];

    ent->r.bmodel = 1;

    ent->r.contents = CM_ContentsOfModel(ent->s.index.brushmodel);

    SV_LinkEntity(ent);

    return 1;
}

void __cdecl SV_ShutdownGameProgs()
{
    Com_SyncThreads();
    sv.state = SS_DEAD;
    Com_UnloadSoundAliases(SASYS_GAME);
    if (gameInitialized)
    {
        G_ShutdownGame(1);
        gameInitialized = 0;
    }
}

void __cdecl SV_InitGameProgs(int savepersist)
{
    gameInitialized = 1;
    SV_InitGameVM(0, savepersist);
}

void __cdecl SV_RestartGameProgs(int savepersist)
{
    Com_SyncThreads();
    G_ShutdownGame(0);
    com_fixedConsolePosition = 0;
    SV_InitGameVM(1, savepersist);
}

#endif // KISAK_MP

#ifdef KISAK_SP
void SV_GameSendServerCommand(int clientNum, const char *text)
{
    if (clientNum == -1)
    {
        SV_SendServerCommand(0, "%s", text);
    }
    else if (!clientNum)
    {
        SV_SendServerCommand(svs.clients, "%s", text);
    }
}
#endif // KISAK_SP