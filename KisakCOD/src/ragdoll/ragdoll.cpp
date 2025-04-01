#include "ragdoll.h"

//  struct dvar_s const *const ragdoll_self_collision_scale 85801018     ragdoll.obj
//  struct dvar_s const *const ragdoll_bullet_force 8580101c     ragdoll.obj
//  struct dvar_s const *const ragdoll_jointlerp_time 85801020     ragdoll.obj
//  struct dvar_s const *const ragdoll_jitter_scale 85801024     ragdoll.obj
//  struct dvar_s const *const ragdoll_dump_anims 85801028     ragdoll.obj
//  struct dvar_s const *const ragdoll_max_simulating 8580102c     ragdoll.obj
//  struct dvar_s const *const ragdoll_baselerp_time 85801030     ragdoll.obj
//  struct dvar_s const *const ragdoll_fps 85801034     ragdoll.obj
//  struct dvar_s const *const ragdoll_rotvel_scale 8580103c     ragdoll.obj
//  struct dvar_s const *const ragdoll_bullet_upbias 85801040     ragdoll.obj
//  struct dvar_s const *const ragdoll_enable 85801044     ragdoll.obj
//  struct dvar_s const *const ragdoll_debug 85814ac8     ragdoll.obj
//  struct dvar_s const *const ragdoll_max_life 85814acc     ragdoll.obj
//  struct dvar_s const *const ragdoll_explode_force 85814ad0     ragdoll.obj
//  struct dvar_s const *const ragdoll_explode_upbias 85814ad4     ragdoll.obj

//  BOOL ragdollInited       85801038     ragdoll.obj
//  struct RagdollBody *ragdollBodies 85801048     ragdoll.obj
//  struct RagdollDef *ragdollDefs 85814ad8     ragdoll.obj




void __cdecl TRACK_ragdoll()
{
    track_static_alloc_internal(ragdollDefs, 7584, "ragdollDefs", 10);
    track_static_alloc_internal(ragdollBodies, 80512, "ragdollBodies", 10);
}

void __cdecl Ragdoll_DebugDraw()
{
    char *v0; // eax
    char *v1; // eax
    RagdollBody *body; // [esp+20h] [ebp-24h]
    int total; // [esp+24h] [ebp-20h]
    int i; // [esp+28h] [ebp-1Ch]
    int states[6]; // [esp+2Ch] [ebp-18h] BYREF

    if (ragdoll_debug->current.integer)
    {
        memset(states, 0, sizeof(states));
        total = 0;
        for (i = 0; i < 32; ++i)
        {
            body = &ragdollBodies[i];
            if (!body)
                MyAssertHandler("c:\\trees\\cod3\\src\\ragdoll\\ragdoll.h", 285, 0, "%s", "body");
            if (body->references > 0)
            {
                if (body->state >= (unsigned int)RAGDOLL_NUM_STATES)
                    MyAssertHandler(
                        ".\\ragdoll\\ragdoll.cpp",
                        107,
                        0,
                        "(int)body->state doesn't index RAGDOLL_NUM_STATES\n\t%i not in [0, %i)",
                        body->state,
                        6);
                ++states[body->state];
                ++total;
            }
        }
        v0 = va("RB Total: %d", total);
        CG_DrawStringExt(&scrPlaceFull, 0.0, 72.0, v0, colorGreen, 0, 1, 12.0);
        v1 = va("RB State: %d %d %d %d %d %d", states[0], states[1], states[2], states[3], states[4], states[5]);
        CG_DrawStringExt(&scrPlaceFull, 0.0, 84.0, v1, colorGreen, 0, 1, 12.0);
    }
}

RagdollDef *__cdecl Ragdoll_BodyDef(RagdollBody *body)
{
    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll.cpp", 182, 0, "%s", "body");
    if (body->ragdollDef >= 2u)
        MyAssertHandler(
            ".\\ragdoll\\ragdoll.cpp",
            183,
            0,
            "body->ragdollDef doesn't index RAGDOLL_MAX_DEFS\n\t%i not in [0, %i)",
            body->ragdollDef,
            2);
    return &ragdollDefs[body->ragdollDef];
}

DObj_s *__cdecl Ragdoll_BodyDObj(RagdollBody *body)
{
    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll.cpp", 191, 0, "%s", "body");
    if (body->obj)
        return body->obj;
    if (body->dobj == -1)
        MyAssertHandler(".\\ragdoll\\ragdoll.cpp", 199, 0, "%s", "body->dobj != DOBJ_HANDLE_NONE");
    return Com_GetClientDObj(body->dobj, body->localClientNum);
}

centity_s *__cdecl Ragdoll_BodyPose(RagdollBody *body)
{
    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll.cpp", 207, 0, "%s", "body");
    if (body->pose)
        return (centity_s *)body->pose;
    if (body->dobj == -1)
        MyAssertHandler(".\\ragdoll\\ragdoll.cpp", 215, 0, "%s", "body->dobj != DOBJ_HANDLE_NONE");
    return CG_GetPose(body->localClientNum, body->dobj);
}

void __cdecl Ragdoll_BodyRootOrigin(RagdollBody *body, float *origin)
{
    BoneOrientation *boneOrientation; // [esp+0h] [ebp-4h]

    if (!body)
        MyAssertHandler("c:\\trees\\cod3\\src\\ragdoll\\ragdoll.h", 271, 0, "%s", "body");
    if (body->state >= BS_TUNNEL_TEST)
    {
        boneOrientation = Ragdoll_BodyBoneOrientations(body);
        *origin = boneOrientation->origin[0];
        origin[1] = boneOrientation->origin[1];
        origin[2] = boneOrientation->origin[2];
    }
}

void __cdecl Ragdoll_GetRootOrigin(int ragdollHandle, float *origin)
{
    RagdollBody *body; // [esp+0h] [ebp-4h]

    body = Ragdoll_HandleBody(ragdollHandle);
    if (!body || body->references <= 0)
        MyAssertHandler(".\\ragdoll\\ragdoll.cpp", 244, 0, "%s", "body && Ragdoll_BodyInUse( body )");
    if (!body)
        MyAssertHandler("c:\\trees\\cod3\\src\\ragdoll\\ragdoll.h", 271, 0, "%s", "body");
    if (body->state >= BS_TUNNEL_TEST)
        Ragdoll_BodyRootOrigin(body, origin);
}

int __cdecl Ragdoll_CountPhysicsBodies()
{
    RagdollBody *body; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    int running; // [esp+8h] [ebp-4h]

    running = 0;
    for (i = 0; i < 32; ++i)
    {
        body = &ragdollBodies[i];
        if (!body)
            MyAssertHandler("c:\\trees\\cod3\\src\\ragdoll\\ragdoll.h", 285, 0, "%s", "body");
        if (body->references > 0 && Ragdoll_BodyHasPhysics(body))
            ++running;
    }
    return running;
}

bool __cdecl Ragdoll_BodyHasPhysics(RagdollBody *body)
{
    if (!body)
        MyAssertHandler("c:\\trees\\cod3\\src\\ragdoll\\ragdoll.h", 292, 0, "%s", "body");
    return body->state >= BS_TUNNEL_TEST && body->state <= BS_RUNNING;
}

int __cdecl Ragdoll_CreateRagdollForDObj(int localClientNum, int ragdollDef, int dobj, bool reset, bool share)
{
    int ragdoll; // [esp+0h] [ebp-8h]
    RagdollBody *body; // [esp+4h] [ebp-4h]

    if (dobj == -1)
        MyAssertHandler(".\\ragdoll\\ragdoll.cpp", 360, 0, "%s", "dobj != DOBJ_HANDLE_NONE");
    if (!Ragdoll_BindDef(1u))
        return 0;
    if (Ragdoll_CountPhysicsBodies() >= ragdoll_max_simulating->current.integer)
        return 0;
    if (share)
    {
        ragdoll = Ragdoll_ReferenceDObjBody(dobj);
        if (!ragdoll)
            ragdoll = Ragdoll_GetUnusedBody();
    }
    else
    {
        ragdoll = Ragdoll_GetUnusedBody();
    }
    if (ragdoll)
    {
        body = Ragdoll_HandleBody(ragdoll);
        if (reset)
            Ragdoll_BodyNewState(body, BS_DEAD);
        if (body->state == BS_DEAD)
        {
            body->dobj = dobj;
            body->localClientNum = localClientNum;
            body->ragdollDef = 1;
            Ragdoll_BodyNewState(body, BS_DOBJ_WAIT);
        }
        return ragdoll;
    }
    else
    {
        Com_Printf(20, "Ragdoll allocation failed, out of ragdoll bodies (obj %d)\n", dobj);
        return 0;
    }
}

int __cdecl Ragdoll_GetUnusedBody()
{
    RagdollBody *body; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    body = ragdollBodies;
    for (i = 0; i < 32; ++i)
    {
        if (!body)
            MyAssertHandler("c:\\trees\\cod3\\src\\ragdoll\\ragdoll.h", 285, 0, "%s", "body");
        if (body->references <= 0)
        {
            Ragdoll_InitBody(body);
            Ragdoll_BodyNewState(body, BS_DEAD);
            ++ragdollBodies[i].references;
            return i + 1;
        }
        ++body;
    }
    return 0;
}

void __cdecl Ragdoll_InitBody(RagdollBody *body)
{
    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll.cpp", 120, 0, "%s", "body");
    memset((unsigned __int8 *)body, 0, sizeof(RagdollBody));
}

int __cdecl Ragdoll_ReferenceDObjBody(int dobj)
{
    RagdollBody *body; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    body = ragdollBodies;
    for (i = 0; i < 32; ++i)
    {
        if (!body)
            MyAssertHandler("c:\\trees\\cod3\\src\\ragdoll\\ragdoll.h", 285, 0, "%s", "body");
        if (body->references > 0 && body->dobj == dobj)
        {
            ++body->references;
            return i + 1;
        }
        ++body;
    }
    return 0;
}

char __cdecl Ragdoll_BindDef(unsigned int ragdollDef)
{
    RagdollDef *def; // [esp+0h] [ebp-18h]
    int nameIdx; // [esp+4h] [ebp-14h]
    BoneDef *boneDef; // [esp+8h] [ebp-10h]
    BoneDef *boneDefa; // [esp+8h] [ebp-10h]
    int i; // [esp+Ch] [ebp-Ch]
    int ia; // [esp+Ch] [ebp-Ch]
    BaseLerpBoneDef *lerpBoneDef; // [esp+10h] [ebp-8h]
    int parentIdx; // [esp+14h] [ebp-4h]

    if (!Ragdoll_ValidateDef(ragdollDef))
        return 0;
    def = &ragdollDefs[ragdollDef];
    if (def->bound)
        return 1;
    boneDef = def->boneDefs;
    for (i = 0; i < def->numBones; ++i)
    {
        for (nameIdx = 0; nameIdx < 2; ++nameIdx)
        {
            if (I_stricmp(boneDef->animBoneTextNames[nameIdx], "none"))
            {
                boneDef->animBoneNames[nameIdx] = SL_FindString(boneDef->animBoneTextNames[nameIdx]).prev;
                if (!boneDef->animBoneNames[nameIdx])
                    return 0;
            }
            else
            {
                boneDef->animBoneNames[nameIdx] = 0;
            }
        }
        ++boneDef;
    }
    lerpBoneDef = def->baseLerpBoneDefs;
    for (ia = 0; ia < def->numBaseLerpBones; ++ia)
    {
        lerpBoneDef->animBoneName = SL_FindString(lerpBoneDef->animBoneTextName).prev;
        if (!lerpBoneDef->animBoneName)
            return 0;
        if (!lerpBoneDef->parentBoneIndex)
        {
            boneDefa = def->boneDefs;
            for (parentIdx = 0; parentIdx < def->numBones; ++parentIdx)
            {
                if (boneDefa->animBoneNames[1] == lerpBoneDef->animBoneName)
                {
                    lerpBoneDef->parentBoneIndex = parentIdx;
                    break;
                }
                ++boneDefa;
            }
            if (parentIdx == def->numBones)
                return 0;
        }
        ++lerpBoneDef;
    }
    def->bound = 1;
    return 1;
}

bool __cdecl Ragdoll_ValidateDef(unsigned int ragdollDef)
{
    if (ragdollDef >= 2)
        MyAssertHandler(
            ".\\ragdoll\\ragdoll.cpp",
            257,
            0,
            "ragdollDef doesn't index RAGDOLL_MAX_DEFS\n\t%i not in [0, %i)",
            ragdollDef,
            2);
    return ragdollDefs[ragdollDef].numBones != 0;
}

void __cdecl Ragdoll_Remove(int ragdoll)
{
    RagdollBody *body; // [esp+0h] [ebp-4h]

    body = Ragdoll_HandleBody(ragdoll);
    if (!body)
        MyAssertHandler("c:\\trees\\cod3\\src\\ragdoll\\ragdoll.h", 285, 0, "%s", "body");
    if (body->references <= 0)
        MyAssertHandler(".\\ragdoll\\ragdoll.cpp", 454, 0, "%s", "Ragdoll_BodyInUse( body )");
    if (body->references == 1)
    {
        Ragdoll_BodyNewState(body, BS_DEAD);
        Ragdoll_FreeBody(ragdoll);
    }
    else
    {
        --body->references;
    }
}

void __cdecl Ragdoll_FreeBody(int ragdollBody)
{
    RagdollBody *body; // [esp+0h] [ebp-4h]

    body = Ragdoll_HandleBody(ragdollBody);
    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll.cpp", 173, 0, "%s", "body");
    Ragdoll_BodyNewState(body, BS_DEAD);
    body->references = 0;
}

void __cdecl Ragdoll_InitDvars()
{
    DvarLimits min; // [esp+4h] [ebp-10h]
    DvarLimits mina; // [esp+4h] [ebp-10h]
    DvarLimits minb; // [esp+4h] [ebp-10h]
    DvarLimits minc; // [esp+4h] [ebp-10h]
    DvarLimits mind; // [esp+4h] [ebp-10h]
    DvarLimits mine; // [esp+4h] [ebp-10h]
    DvarLimits minf; // [esp+4h] [ebp-10h]

    ragdoll_enable = Dvar_RegisterBool("ragdoll_enable", 1, 1u, "Turn on ragdoll death animations");
    ragdoll_debug = Dvar_RegisterInt(
        "ragdoll_debug",
        0,
        (DvarLimits)0x7FFFFFFF00000000LL,
        0x80u,
        "Draw ragdoll debug info (bitflags)");
    ragdoll_fps = Dvar_RegisterInt(
        "ragdoll_fps",
        20,
        (DvarLimits)0x6400000000LL,
        0x80u,
        "Ragdoll update frames per second");
    ragdoll_max_life = Dvar_RegisterInt(
        "ragdoll_max_life",
        4500,
        (DvarLimits)0x7FFFFFFF00000000LL,
        0x80u,
        "Max lifetime of a ragdoll system in msec");
    ragdoll_max_simulating = Dvar_RegisterInt(
        "ragdoll_max_simulating",
        16,
        (DvarLimits)0x2000000000LL,
        1u,
        "Max number of simultaneous active ragdolls");
    min.value.max = 60000.0;
    min.value.min = 0.0;
    ragdoll_explode_force = Dvar_RegisterFloat(
        "ragdoll_explode_force",
        18000.0,
        min,
        0x80u,
        "Explosive force applied to ragdolls");
    mina.value.max = 2.0;
    mina.value.min = 0.0;
    ragdoll_explode_upbias = Dvar_RegisterFloat(
        "ragdoll_explode_upbias",
        0.80000001,
        mina,
        0x80u,
        "Upwards bias applied to ragdoll explosion effects");
    minb.value.max = 10000.0;
    minb.value.min = 0.0;
    ragdoll_bullet_force = Dvar_RegisterFloat(
        "ragdoll_bullet_force",
        500.0,
        minb,
        0x80u,
        "Bullet force applied to ragdolls");
    minc.value.max = 10000.0;
    minc.value.min = 0.0;
    ragdoll_bullet_upbias = Dvar_RegisterFloat(
        "ragdoll_bullet_upbias",
        0.5,
        minc,
        0x80u,
        "Upward bias applied to ragdoll bullet effects");
    ragdoll_baselerp_time = Dvar_RegisterInt(
        "ragdoll_baselerp_time",
        1000,
        (DvarLimits)0x177000000064LL,
        0x80u,
        "Default time ragdoll baselerp bones take to reach the base pose");
    ragdoll_jointlerp_time = Dvar_RegisterInt(
        "ragdoll_jointlerp_time",
        3000,
        (DvarLimits)0x177000000064LL,
        0x80u,
        "Default time taken to lerp down ragdoll joint friction");
    mind.value.max = 2000.0;
    mind.value.min = 0.0;
    ragdoll_rotvel_scale = Dvar_RegisterFloat(
        "ragdoll_rotvel_scale",
        1.0,
        mind,
        0x80u,
        "Ragdoll rotational velocity estimate scale");
    mine.value.max = 10.0;
    mine.value.min = 0.0;
    ragdoll_jitter_scale = Dvar_RegisterFloat(
        "ragdoll_jitter_scale",
        1.0,
        mine,
        0x80u,
        "Scale up or down the effect of physics jitter on ragdolls");
    minf.value.max = 10.0;
    minf.value.min = 0.1;
    ragdoll_self_collision_scale = Dvar_RegisterFloat(
        "ragdoll_self_collision_scale",
        1.2,
        minf,
        0x80u,
        "Scale the size of the collision capsules used to prevent ragdoll limbs from interpenetrating");
    ragdoll_dump_anims = Dvar_RegisterBool("ragdoll_dump_anims", 0, 0, "Dump animation data when ragdoll fails");
}

void __cdecl Ragdoll_Register()
{
    int v0; // eax

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\ragdoll\\ragdoll.cpp", 503, 0, "%s", "Sys_IsMainThread()");
    if (ragdollFirstInit)
        MyAssertHandler(".\\ragdoll\\ragdoll.cpp", 504, 0, "%s", "!ragdollFirstInit");
    Ragdoll_InitDvars();
    Ragdoll_InitCommands();
    v0 = CL_ControllerIndexFromClientNum(0);
    Cmd_ExecuteSingleCommand(0, v0, "exec ragdoll.cfg");
    ragdollFirstInit = 1;
}

void __cdecl Ragdoll_Init()
{
    int i; // [esp+0h] [ebp-4h]

    if (!Sys_IsMainThread())
        MyAssertHandler(".\\ragdoll\\ragdoll.cpp", 519, 0, "%s", "Sys_IsMainThread()");
    if (!ragdollFirstInit)
        MyAssertHandler(".\\ragdoll\\ragdoll.cpp", 520, 0, "%s", "ragdollFirstInit");
    if (!ragdollInited)
    {
        if (ragdoll_enable->current.enabled && ragdoll_max_simulating->current.integer < 8)
            Dvar_SetInt((dvar_s *)ragdoll_max_simulating, 8);
        memset((unsigned __int8 *)ragdollBodies, 0, 0x13A80u);
        for (i = 0; i < 2; ++i)
        {
            byte_E05CDF9[3792 * i] = 0;
            ragdollDefs[i].inUse = 0;
        }
        ragdollInited = 1;
    }
}

void __cdecl Ragdoll_Shutdown()
{
    if (!Sys_IsMainThread())
        MyAssertHandler(".\\ragdoll\\ragdoll.cpp", 546, 0, "%s", "Sys_IsMainThread()");
    ragdollInited = 0;
}

