#include "ragdoll.h"
#include <qcommon/threads.h>
#include <qcommon/mem_track.h>
#include <cgame/cg_local.h>
#include <client/client.h>

#include <qcommon/cmd.h>

#ifdef KISAK_MP
#include <cgame_mp/cg_local_mp.h>
#include <client_mp/client_mp.h>
#elif KISAK_SP
#include <cgame/cg_ents.h>
#endif

const dvar_t *ragdoll_self_collision_scale;
const dvar_t *ragdoll_bullet_force;
const dvar_t *ragdoll_jointlerp_time;
const dvar_t *ragdoll_jitter_scale;
const dvar_t *ragdoll_dump_anims;
const dvar_t *ragdoll_max_simulating;
const dvar_t *ragdoll_baselerp_time;
const dvar_t *ragdoll_fps;
const dvar_t *ragdoll_rotvel_scale;
const dvar_t *ragdoll_bullet_upbias;
const dvar_t *ragdoll_enable;
const dvar_t *ragdoll_debug;
const dvar_t *ragdoll_max_life;
const dvar_t *ragdoll_explode_force;
const dvar_t *ragdoll_explode_upbias;

BOOL ragdollInited;
RagdollDef ragdollDefs[2];
RagdollBody ragdollBodies[32];

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

const cpose_t *__cdecl Ragdoll_BodyPose(RagdollBody *body)
{
    if (!body)
        MyAssertHandler(".\\ragdoll\\ragdoll.cpp", 207, 0, "%s", "body");
    if (body->pose)
        return body->pose;
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
                boneDef->animBoneNames[nameIdx] = SL_FindString(boneDef->animBoneTextNames[nameIdx]);
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
        lerpBoneDef->animBoneName = SL_FindString(lerpBoneDef->animBoneTextName);
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

    ragdoll_enable = Dvar_RegisterBool("ragdoll_enable", 1, DVAR_ARCHIVE, "Turn on ragdoll death animations");
    ragdoll_debug = Dvar_RegisterInt(
        "ragdoll_debug",
        0,
        (DvarLimits)0x7FFFFFFF00000000LL,
        DVAR_CHEAT,
        "Draw ragdoll debug info (bitflags)");
    ragdoll_fps = Dvar_RegisterInt(
        "ragdoll_fps",
        20,
        (DvarLimits)0x6400000000LL,
        DVAR_CHEAT,
        "Ragdoll update frames per second");
    ragdoll_max_life = Dvar_RegisterInt(
        "ragdoll_max_life",
        4500,
        (DvarLimits)0x7FFFFFFF00000000LL,
        DVAR_CHEAT,
        "Max lifetime of a ragdoll system in msec");
    ragdoll_max_simulating = Dvar_RegisterInt(
        "ragdoll_max_simulating",
        16,
        (DvarLimits)0x2000000000LL,
        DVAR_ARCHIVE,
        "Max number of simultaneous active ragdolls");
    min.value.max = 60000.0f;
    min.value.min = 0.0f;
    ragdoll_explode_force = Dvar_RegisterFloat(
        "ragdoll_explode_force",
        18000.0f,
        min,
        DVAR_CHEAT,
        "Explosive force applied to ragdolls");
    mina.value.max = 2.0f;
    mina.value.min = 0.0f;
    ragdoll_explode_upbias = Dvar_RegisterFloat(
        "ragdoll_explode_upbias",
        0.80000001f,
        mina,
        DVAR_CHEAT,
        "Upwards bias applied to ragdoll explosion effects");
    minb.value.max = 10000.0f;
    minb.value.min = 0.0f;
    ragdoll_bullet_force = Dvar_RegisterFloat(
        "ragdoll_bullet_force",
        500.0f,
        minb,
        DVAR_CHEAT,
        "Bullet force applied to ragdolls");
    minc.value.max = 10000.0f;
    minc.value.min = 0.0f;
    ragdoll_bullet_upbias = Dvar_RegisterFloat(
        "ragdoll_bullet_upbias",
        0.5,
        minc,
        DVAR_CHEAT,
        "Upward bias applied to ragdoll bullet effects");
    ragdoll_baselerp_time = Dvar_RegisterInt(
        "ragdoll_baselerp_time",
        1000,
        (DvarLimits)0x177000000064LL,
        DVAR_CHEAT,
        "Default time ragdoll baselerp bones take to reach the base pose");
    ragdoll_jointlerp_time = Dvar_RegisterInt(
        "ragdoll_jointlerp_time",
        3000,
        (DvarLimits)0x177000000064LL,
        DVAR_CHEAT,
        "Default time taken to lerp down ragdoll joint friction");
    mind.value.max = 2000.0f;
    mind.value.min = 0.0f;
    ragdoll_rotvel_scale = Dvar_RegisterFloat(
        "ragdoll_rotvel_scale",
        1.0f,
        mind,
        DVAR_CHEAT,
        "Ragdoll rotational velocity estimate scale");
    mine.value.max = 10.0f;
    mine.value.min = 0.0f;
    ragdoll_jitter_scale = Dvar_RegisterFloat(
        "ragdoll_jitter_scale",
        1.0f,
        mine,
        DVAR_CHEAT,
        "Scale up or down the effect of physics jitter on ragdolls");
    minf.value.max = 10.0f;
    minf.value.min = 0.1f;
    ragdoll_self_collision_scale = Dvar_RegisterFloat(
        "ragdoll_self_collision_scale",
        1.2f,
        minf,
        DVAR_CHEAT,
        "Scale the size of the collision capsules used to prevent ragdoll limbs from interpenetrating");
    ragdoll_dump_anims = Dvar_RegisterBool("ragdoll_dump_anims", 0, DVAR_NOFLAG, "Dump animation data when ragdoll fails");
}

bool ragdollFirstInit = false;

void __cdecl Ragdoll_ResetBodiesUsingDef()
{
    RagdollBody *body; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    for (i = 0; i < 32; ++i)
    {
        body = &ragdollBodies[i];
        if (!body)
            MyAssertHandler("c:\\trees\\cod3\\src\\ragdoll\\ragdoll.h", 278, 0, "%s", "body");
        if (body->state >= BS_VELOCITY_CAPTURE)
            Ragdoll_BodyNewState(body, BS_DOBJ_WAIT);
    }
}

char __cdecl Ragdoll_ReadGeomType(int arg, BoneDef *bone)
{
    const char *name; // [esp+14h] [ebp-8h]
    int idx; // [esp+18h] [ebp-4h]

    if (Cmd_Argc() >= arg)
    {
        name = Cmd_Argv(arg);
        for (idx = (int)PHYS_GEOM_NONE; idx < (int)PHYS_GEOM_COUNT; ++idx)
        {
            if (!I_strnicmp(geomNames[idx], name, strlen(geomNames[idx])))
            {
                bone->geomType = (PhysicsGeomType)idx;
                return 1;
            }
        }
        Com_Printf(14, "Ragdoll: Unknown bone geom type %s\n", name);
        return 0;
    }
    else
    {
        Com_Printf(14, "Ragdoll: Missing geom type arg %d\n", arg);
        return 0;
    }
}

char __cdecl Ragdoll_ReadJointType(int arg, JointDef *joint)
{
    const char *name; // [esp+0h] [ebp-8h]
    int idx; // [esp+4h] [ebp-4h]

    if (Cmd_Argc() >= arg)
    {
        name = Cmd_Argv(arg);
        for (idx = (int)RAGDOLL_JOINT_NONE; (int)idx < (RAGDOLL_JOINT_SWIVEL | RAGDOLL_JOINT_HINGE); ++idx)
        {
            if (!I_stricmp(name, jointNames[idx]))
            {
                joint->type = (JointType)idx;
                return 1;
            }
        }
        Com_Printf(14, "Ragdoll: Unknown joint type %s\n", name);
        return 0;
    }
    else
    {
        Com_Printf(14, "Ragdoll: Missing joint type arg %d\n", arg);
        return 0;
    }
}

struct AxisTable // sizeof=0x10
{                                       // ...
    const char *name;                   // ...
    float axis[3];                      // ...
};

AxisTable axisTable[4] =
{
  { "x", { 1.0, 0.0, 0.0 } },
  { "y", { 0.0, 1.0, 0.0 } },
  { "z", { 0.0, 0.0, 1.0 } },
  { "n", { 0.0, 0.0, 0.0 } }
}; // idb

char __cdecl Ragdoll_ReadAxis(int arg, float *dest)
{
    float *axis; // [esp+0h] [ebp-10h]
    bool negate; // [esp+7h] [ebp-9h]
    unsigned int idx; // [esp+8h] [ebp-8h]
    const char *argv; // [esp+Ch] [ebp-4h]

    if (Cmd_Argc() >= arg)
    {
        negate = 0;
        argv = Cmd_Argv(arg);
        if (*argv == 45)
        {
            negate = 1;
            ++argv;
        }
        for (idx = 0; ; ++idx)
        {
            if (idx >= 4)
            {
                Com_Printf(14, "Ragdoll: Unknown bone axis %s\n", argv);
                return 0;
            }
            if (!I_stricmp(argv, axisTable[idx].name))
                break;
        }
        axis = axisTable[idx].axis;
        *dest = *axis;
        dest[1] = axis[1];
        dest[2] = axis[2];
        if (negate)
        {
            *dest = -*dest;
            dest[1] = -dest[1];
            dest[2] = -dest[2];
        }
        return 1;
    }
    else
    {
        Com_Printf(14, "Ragdoll: Missing axis arg %d\n", arg);
        *dest = 0.0;
        dest[1] = 0.0;
        dest[2] = 0.0;
        return 0;
    }
}

void __cdecl Ragdoll_Clear_f()
{
    const char *v0; // eax
    unsigned int ragdoll; // [esp+0h] [ebp-4h]

    if (Cmd_Argc() >= 2)
    {
        v0 = Cmd_Argv(1);
        ragdoll = atoi(v0);
        if (ragdoll < 2)
        {
            memset((unsigned __int8 *)&ragdollDefs[ragdoll], 0, sizeof(RagdollDef));
            Ragdoll_ResetBodiesUsingDef();
        }
    }
}

void __cdecl Ragdoll_Bone_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    unsigned int ragdoll; // [esp+0h] [ebp-18h]
    RagdollDef *def; // [esp+4h] [ebp-14h]
    int parentBone; // [esp+8h] [ebp-10h]
    char *name; // [esp+Ch] [ebp-Ch]
    int i; // [esp+10h] [ebp-8h]
    BoneDef *bone; // [esp+14h] [ebp-4h]

    if (Cmd_Argc() >= 11)
    {
        v0 = Cmd_Argv(1);
        ragdoll = atoi(v0);
        if (ragdoll < 2)
        {
            def = &ragdollDefs[ragdoll];
            if (def->numBones >= 14)
            {
                Com_Printf(14, "Ragdoll: Too many ragdoll bones, max %d\n", 14);
                return;
            }
            bone = &def->boneDefs[def->numBones];
            for (i = 0; i < 2; ++i)
            {
                bone->animBoneNames[i] = 0;
                name = (char *)Cmd_Argv(i + 2);
                I_strncpyz(bone->animBoneTextNames[i], name, 20);
            }
            v1 = Cmd_Argv(4);
            bone->radius = atof(v1);
            v2 = Cmd_Argv(5);
            bone->percent = atof(v2);
            v3 = Cmd_Argv(6);
            bone->mass = atof(v3);
            v4 = Cmd_Argv(7);
            bone->friction = atof(v4);
            v5 = Cmd_Argv(8);
            parentBone = atoi(v5);
            if (parentBone == -1)
            {
                bone->parentBone = -1;
            }
            else
            {
                if (parentBone >= def->numBones)
                {
                    Com_Printf(14, "Ragdoll: Child bones must come after parent bones: %d\n", parentBone);
                    return;
                }
                bone->parentBone = parentBone;
            }
            v6 = Cmd_Argv(9);
            bone->mirror = atoi(v6) != 0;
            if (Ragdoll_ReadGeomType(10, bone))
            {
                if (Ragdoll_ValidatePrecalcBoneDef(def, bone))
                {
                    ++def->numBones;
                    Ragdoll_ResetBodiesUsingDef();
                }
                else
                {
                    Com_Printf(14, "Ragdoll: Bone %d validation failed\n", def->numBones);
                }
            }
        }
    }
}

void __cdecl Ragdoll_BaseLerpBone_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    int v3; // [esp+0h] [ebp-1Ch]
    int lerpTime; // [esp+8h] [ebp-14h]
    unsigned int ragdoll; // [esp+Ch] [ebp-10h]
    RagdollDef *def; // [esp+10h] [ebp-Ch]
    char *name; // [esp+14h] [ebp-8h]
    BaseLerpBoneDef *bone; // [esp+18h] [ebp-4h]

    if (Cmd_Argc() >= 3)
    {
        v0 = Cmd_Argv(1);
        ragdoll = atoi(v0);
        if (ragdoll < 2)
        {
            def = &ragdollDefs[ragdoll];
            if (def->numBaseLerpBones < 9)
            {
                bone = &def->baseLerpBoneDefs[def->numBaseLerpBones];
                name = (char *)Cmd_Argv(2);
                I_strncpyz(bone->animBoneTextName, name, 20);
                bone->animBoneName = 0;
                if (Cmd_Argc() <= 2)
                {
                    bone->lerpTime = ragdoll_baselerp_time->current.integer;
                }
                else
                {
                    v1 = Cmd_Argv(3);
                    bone->lerpTime = atoi(v1);
                    if (bone->lerpTime < 6000)
                        lerpTime = bone->lerpTime;
                    else
                        lerpTime = 6000;
                    if (lerpTime > 100)
                        v3 = lerpTime;
                    else
                        v3 = 100;
                    bone->lerpTime = v3;
                }
                if (Cmd_Argc() <= 3)
                {
                    bone->parentBoneIndex = 0;
                }
                else
                {
                    v2 = Cmd_Argv(4);
                    bone->parentBoneIndex = atoi(v2);
                }
                ++def->numBaseLerpBones;
            }
            else
            {
                Com_Printf(14, "Ragdoll: Too many base pose lerping bones, max %d\n", 9);
            }
        }
    }
}
void __cdecl Ragdoll_PinBone_f()
{
    const char *v0; // eax
    const char *v1; // eax
    unsigned int ragdoll; // [esp+0h] [ebp-10h]
    RagdollDef *def; // [esp+4h] [ebp-Ch]
    const char *name; // [esp+8h] [ebp-8h]
    HashEntry_unnamed_type_u *bone; // [esp+Ch] [ebp-4h]

    if (Cmd_Argc() >= 3)
    {
        v0 = Cmd_Argv(1);
        ragdoll = atoi(v0);
        if (ragdoll < 2)
        {
            def = &ragdollDefs[ragdoll];
            if (def->numBaseLerpBones < 9)
            {
                bone = (HashEntry_unnamed_type_u *)&def->baseLerpBoneDefs[def->numBaseLerpBones];
                name = Cmd_Argv(2);
                bone[5].prev = SL_FindString(name);
                if (bone[5].prev)
                {
                    v1 = Cmd_Argv(3);
                    bone[6].prev = atoi(v1);
                    if (bone[6].prev < def->numBones)
                    {
                        bone[7].prev = 0;
                        ++def->numBaseLerpBones;
                    }
                    else
                    {
                        Com_Printf(14, "Ragdoll: Pinned bone has invalid parent index %d\n", bone[6].prev);
                    }
                }
                else
                {
                    Com_Printf(14, "Ragdoll: Couldn't find pinned bone named %s\n", name);
                }
            }
            else
            {
                Com_Printf(14, "Ragdoll: Too many base pose lerping bones, max %d\n", 9);
            }
        }
    }
}
void __cdecl Ragdoll_Joint_f()
{
    const char *v0; // eax
    const char *v1; // eax
    unsigned int ragdoll; // [esp+0h] [ebp-Ch]
    RagdollDef *def; // [esp+4h] [ebp-8h]
    JointDef *joint; // [esp+8h] [ebp-4h]

    if (Cmd_Argc() >= 4)
    {
        v0 = Cmd_Argv(1);
        ragdoll = atoi(v0);
        if (ragdoll < 2)
        {
            def = &ragdollDefs[ragdoll];
            if (def->numJoints < 28)
            {
                joint = &def->jointDefs[def->numJoints];
                v1 = Cmd_Argv(2);
                joint->bone = atoi(v1);
                if (joint->bone < def->numBones)
                {
                    if (joint->bone)
                    {
                        if (Ragdoll_ReadJointType(3, joint))
                        {
                            ++def->numJoints;
                            Ragdoll_ResetBodiesUsingDef();
                        }
                    }
                    else
                    {
                        Com_Printf(14, "Ragdoll: Joint referenced bone with no parent (0)\n");
                    }
                }
                else
                {
                    Com_Printf(14, "Ragdoll: Joint referenced nonexistent bone\n");
                }
            }
            else
            {
                Com_Printf(14, "Ragdoll: Too many ragdoll joints, max %d\n", 28);
            }
        }
    }
}

void __cdecl Ragdoll_Limit_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    float v5; // [esp+0h] [ebp-44h]
    float v6; // [esp+4h] [ebp-40h]
    float v7; // [esp+8h] [ebp-3Ch]
    float v8; // [esp+Ch] [ebp-38h]
    float v9; // [esp+10h] [ebp-34h]
    float v10; // [esp+14h] [ebp-30h]
    float v11; // [esp+18h] [ebp-2Ch]
    float v12; // [esp+1Ch] [ebp-28h]
    float v13; // [esp+20h] [ebp-24h]
    float v14; // [esp+24h] [ebp-20h]
    float v15; // [esp+28h] [ebp-1Ch]
    float v16; // [esp+2Ch] [ebp-18h]
    float v17; // [esp+30h] [ebp-14h]
    unsigned int ragdoll; // [esp+34h] [ebp-10h]
    RagdollDef *def; // [esp+38h] [ebp-Ch]
    int jointNum; // [esp+40h] [ebp-4h]

    if (Cmd_Argc() >= 7)
    {
        v0 = Cmd_Argv(1);
        ragdoll = atoi(v0);
        if (ragdoll < 2)
        {
            def = &ragdollDefs[ragdoll];
            if (def->numJoints < 28)
            {
                v1 = Cmd_Argv(2);
                jointNum = atoi(v1);
                if (jointNum < def->numJoints)
                {
                    if (def->jointDefs[jointNum].numLimitAxes < 3)
                    {
                        if (Ragdoll_ReadAxis(3, def->jointDefs[jointNum].limitAxes[def->jointDefs[jointNum].numLimitAxes]))
                        {
                            v2 = Cmd_Argv(4);
                            v13 = atof(v2);
                            def->jointDefs[jointNum].axisFriction[def->jointDefs[jointNum].numLimitAxes] = v13;
                            v3 = Cmd_Argv(5);
                            v12 = atof(v3);
                            def->jointDefs[jointNum].minAngles[def->jointDefs[jointNum].numLimitAxes] = v12 * 0.01745329238474369f;
                            v4 = Cmd_Argv(6);
                            v11 = atof(v4);
                            def->jointDefs[jointNum].maxAngles[def->jointDefs[jointNum].numLimitAxes] = v11 * 0.01745329238474369f;
                            v16 = def->jointDefs[jointNum].minAngles[def->jointDefs[jointNum].numLimitAxes];
                            v10 = v16 - 3.1415927f;
                            if (v10 < 0.0)
                                v17 = v16;
                            else
                                v17 = 3.1415927f;
                            v9 = -3.1415927f - v16;
                            if (v9 < 0.0)
                                v8 = v17;
                            else
                                v8 = -3.1415927f;
                            def->jointDefs[jointNum].minAngles[def->jointDefs[jointNum].numLimitAxes] = v8;
                            v14 = def->jointDefs[jointNum].maxAngles[def->jointDefs[jointNum].numLimitAxes];
                            v7 = v14 - 3.1415927f;
                            if (v7 < 0.0)
                                v15 = v14;
                            else
                                v15 = 3.1415927f;
                            v6 = -3.1415927f - v14;
                            if (v6 < 0.0f)
                                v5 = v15;
                            else
                                v5 = -3.1415927f;
                            def->jointDefs[jointNum].maxAngles[def->jointDefs[jointNum].numLimitAxes++] = v5;
                            Ragdoll_ResetBodiesUsingDef();
                        }
                    }
                    else
                    {
                        Com_Printf(14, "Ragdoll: Too many limit axes for joint %d, max %d\n", jointNum, 3);
                    }
                }
                else
                {
                    Com_Printf(14, "Ragdoll: Angular limit added to nonexistent joint %d\n", jointNum);
                }
            }
            else
            {
                Com_Printf(14, "Ragdoll: Too many ragdoll joints, max %d\n", 28);
            }
        }
    }
}
void __cdecl Ragdoll_Selfpair_f()
{
    const char *v0; // eax
    const char *v1; // eax
    unsigned int ragdoll; // [esp+0h] [ebp-10h]
    RagdollDef *def; // [esp+4h] [ebp-Ch]
    SelfPairDef *pair; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]

    if (Cmd_Argc() >= 4)
    {
        v0 = Cmd_Argv(1);
        ragdoll = atoi(v0);
        if (ragdoll < 2)
        {
            def = &ragdollDefs[ragdoll];
            if (def->numSelfPairs < 33)
            {
                pair = &def->selfPairDefs[def->numSelfPairs];
                for (i = 0; i < 2; ++i)
                {
                    v1 = Cmd_Argv(i + 2);
                    pair->bones[i] = atoi(v1);
                    if (pair->bones[i] > def->numBones)
                    {
                        Com_Printf(14, "Ragdoll: Bad self collision pair bone %d\n", pair->bones[i]);
                        return;
                    }
                }
                ++def->numSelfPairs;
                Ragdoll_ResetBodiesUsingDef();
            }
            else
            {
                Com_Printf(14, "Ragdoll: Too many ragdoll self collision pairs, max %d\n", 33);
            }
        }
    }
}

cmd_function_s Ragdoll_Clear_f_VAR;
cmd_function_s Ragdoll_Bone_f_VAR;
cmd_function_s Ragdoll_BaseLerpBone_f_VAR;
cmd_function_s Ragdoll_PinBone_f_VAR;
cmd_function_s Ragdoll_Joint_f_VAR;
cmd_function_s Ragdoll_Limit_f_VAR;
cmd_function_s Ragdoll_Selfpair_f_VAR;

static void __cdecl Ragdoll_InitCommands()
{
    Cmd_AddCommandInternal("ragdoll_clear", Ragdoll_Clear_f, &Ragdoll_Clear_f_VAR);
    Cmd_AddCommandInternal("ragdoll_bone", Ragdoll_Bone_f, &Ragdoll_Bone_f_VAR);
    Cmd_AddCommandInternal("ragdoll_baselerp_bone", Ragdoll_BaseLerpBone_f, &Ragdoll_BaseLerpBone_f_VAR);
    Cmd_AddCommandInternal("ragdoll_pin_bone", Ragdoll_PinBone_f, &Ragdoll_PinBone_f_VAR);
    Cmd_AddCommandInternal("ragdoll_joint", Ragdoll_Joint_f, &Ragdoll_Joint_f_VAR);
    Cmd_AddCommandInternal("ragdoll_limit", Ragdoll_Limit_f, &Ragdoll_Limit_f_VAR);
    Cmd_AddCommandInternal("ragdoll_selfpair", Ragdoll_Selfpair_f, &Ragdoll_Selfpair_f_VAR);
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
    Cmd_ExecuteSingleCommand(0, v0, (char*)"exec ragdoll.cfg");
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
            ragdollDefs[i].bound = 0;
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

int Ragdoll_CreateRagdollForDObjRaw(int localClientNum, int ragdollDef, const cpose_t *pose, DObj_s *dobj)
{
    int UnusedBody; // r3
    int v10; // r31
    RagdollBody *v11; // r3

    if (!dobj)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\ragdoll\\ragdoll.cpp", 424, 0, "%s", "dobj");
    if (!pose)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\ragdoll\\ragdoll.cpp", 425, 0, "%s", "pose");
    if (!Ragdoll_BindDef(ragdollDef))
        return 0;
    UnusedBody = Ragdoll_GetUnusedBody();
    v10 = UnusedBody;
    if (!UnusedBody)
        return 0;
    v11 = Ragdoll_HandleBody(UnusedBody);
    v11->obj = dobj;
    v11->pose = pose;
    v11->localClientNum = localClientNum;
    v11->ragdollDef = ragdollDef;
    v11->dobj = -1;
    Ragdoll_BodyNewState(v11, BS_DOBJ_WAIT);
    return v10;
}