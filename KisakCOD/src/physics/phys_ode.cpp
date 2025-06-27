#include "phys_local.h"
#include <DynEntity/DynEntity_client.h>
#include <qcommon/mem_track.h>
#include <aim_assist/aim_assist.h>
#include <qcommon/cmd.h>
#include <xanim/xmodel.h>

#include <ode/objects.h>
#include <physics/ode/collision_kernel.h>
#include <win32/win_local.h>
#include <gfx_d3d/r_dpvs.h>
#include "ode/odeext.h"
#include <cgame_mp/cg_local_mp.h>
#include <universal/profile.h>
#include "ode/collision_transform.h"
//int *g_phys_msecStep    827c0304     phys_ode.obj
//struct PhysGlob physGlob   85513d50     phys_ode.obj

PhysGlob physGlob;

bool physInited;

const dvar_t *phys_contact_erp;
const dvar_t *phys_autoDisableAngular;
const dvar_t *phys_drawcontacts;
const dvar_t *phys_bulletUpBias;
const dvar_t *phys_joint_stop_erp;
const dvar_t *phys_noIslands;
const dvar_t *phys_dragLinear;
const dvar_t *phys_joint_cfm;
const dvar_t *phys_gravityChangeWakeupRadius;
const dvar_t *phys_drawAwake;
const dvar_t *phys_contact_cfm;
const dvar_t *phys_narrowObjMaxLength;
const dvar_t *phys_minImpactMomentum;
const dvar_t *phys_autoDisableTime;
const dvar_t *phys_bulletSpinScale;
const dvar_t *phys_drawAwakeTooLong;
const dvar_t *phys_cfm  ;
const dvar_t *phys_drawCollisionWorld;
const dvar_t *phys_visibleTris;
const dvar_t *phys_gravity;
const dvar_t *phys_collUseEntities;
const dvar_t *phys_autoDisableLinear;
const dvar_t *phys_contact_cfm_ragdoll;
const dvar_t *phys_drawCollisionObj;
const dvar_t *phys_joint_stop_cfm;
const dvar_t *phys_erp  ;
const dvar_t *phys_dragAngular;
const dvar_t *phys_frictionScale;
const dvar_t *phys_dumpcontacts;
const dvar_t *phys_mcv_ragdoll;
const dvar_t *phys_csl  ;
const dvar_t *phys_contact_erp_ragdoll;
const dvar_t *phys_reorderConst;
const dvar_t *phys_interBodyCollision;
const dvar_t *phys_drawDebugInfo;
const dvar_t *phys_qsi  ;
const dvar_t *phys_jitterMaxMass;
const dvar_t *phys_mcv;

struct FrameInfo // sizeof=0x8
{                                       // ...
    int localClientNum;                 // ...
    int worldIndex;                     // ...
};

template <typename T>
void __cdecl ODE_ForEachBody(dxWorld *world, T func)
{
    dxBody *bodyIter; // [esp+0h] [ebp-4h]

    for (bodyIter = world->firstbody; bodyIter; bodyIter = (dxBody *)bodyIter->next)
        func(bodyIter);
}

void __cdecl TRACK_phys()
{
    track_static_alloc_internal(&physGlob, 156936, "physGlob", 9);
}

cmd_function_s Phys_Stop_f_VAR;
cmd_function_s Phys_Go_f_VAR;

void __cdecl Phys_Init()
{
    DvarLimits min; // [esp+4h] [ebp-14h]
    DvarLimits mina; // [esp+4h] [ebp-14h]
    DvarLimits minb; // [esp+4h] [ebp-14h]
    DvarLimits minc; // [esp+4h] [ebp-14h]
    DvarLimits mind; // [esp+4h] [ebp-14h]
    DvarLimits mine; // [esp+4h] [ebp-14h]
    DvarLimits minf; // [esp+4h] [ebp-14h]
    DvarLimits ming; // [esp+4h] [ebp-14h]
    DvarLimits minh; // [esp+4h] [ebp-14h]
    DvarLimits mini; // [esp+4h] [ebp-14h]
    DvarLimits minj; // [esp+4h] [ebp-14h]
    DvarLimits mink; // [esp+4h] [ebp-14h]
    DvarLimits minl; // [esp+4h] [ebp-14h]
    DvarLimits minm; // [esp+4h] [ebp-14h]
    DvarLimits minn; // [esp+4h] [ebp-14h]
    DvarLimits mino; // [esp+4h] [ebp-14h]
    DvarLimits minp; // [esp+4h] [ebp-14h]
    DvarLimits minq; // [esp+4h] [ebp-14h]
    DvarLimits minr; // [esp+4h] [ebp-14h]
    DvarLimits mins; // [esp+4h] [ebp-14h]
    DvarLimits mint; // [esp+4h] [ebp-14h]
    DvarLimits minu; // [esp+4h] [ebp-14h]
    DvarLimits minv; // [esp+4h] [ebp-14h]
    DvarLimits minw; // [esp+4h] [ebp-14h]
    DvarLimits minx; // [esp+4h] [ebp-14h]
    PhysWorld worldIndex; // [esp+14h] [ebp-4h]

    if (!physInited)
    {
        memset((unsigned __int8 *)&physGlob, 0, sizeof(physGlob));
        Pool_Init((char *)physGlob.userData, &physGlob.userDataPool, 0x70u, 0x200u);
        ODE_Init();
        for (worldIndex = PHYS_WORLD_DYNENT; worldIndex < PHYS_WORLD_COUNT; ++worldIndex)
        {
            physGlob.world[worldIndex] = dWorldCreate(worldIndex);
            if (!physGlob.world[worldIndex])
                MyAssertHandler(".\\physics\\phys_ode.cpp", 316, 0, "%s", "physGlob.world[worldIndex]");
            physGlob.worldData[worldIndex].timeLastSnapshot = 0;
            physGlob.worldData[worldIndex].timeLastUpdate = 0;
            physGlob.space[worldIndex] = dGetSimpleSpace(worldIndex);
            if (!physGlob.space[worldIndex])
                MyAssertHandler(".\\physics\\phys_ode.cpp", 320, 0, "%s", "physGlob.space[worldIndex]");
            physGlob.contactgroup[worldIndex] = dGetContactJointGroup(worldIndex);
            if (!physGlob.contactgroup[worldIndex])
                MyAssertHandler(".\\physics\\phys_ode.cpp", 322, 0, "%s", "physGlob.contactgroup[worldIndex]");
            dWorldSetAutoDisableFlag(physGlob.world[worldIndex], 1);
            dWorldSetAutoDisableSteps(physGlob.world[worldIndex], 0);
            physGlob.worldData[worldIndex].collisionCallback = nullptr;
        }
        physGlob.dumpContacts = 0;
        min.value.max = 1.0f;
        min.value.min = 0.0f;
        phys_cfm = Dvar_RegisterFloat(
            "phys_cfm",
            0.000099999997f,
            min,
            0,
            "Physics constraint force mixing magic parameter.");
        mina.value.max = 1.0f;
        mina.value.min = 0.0f;
        phys_erp = Dvar_RegisterFloat("phys_erp", 0.80000001f, mina, 0, "Physics error reduction magic parameter.");
        minb.value.max = FLT_MAX;
        minb.value.min = -FLT_MAX;
        phys_mcv = Dvar_RegisterFloat("phys_mcv", 20.0f, minb, 0, "Physics maximum correcting velocity magic parameter.");
        minc.value.max = FLT_MAX;
        minc.value.min = -FLT_MAX;
        phys_mcv_ragdoll = Dvar_RegisterFloat(
            "phys_mcv_ragdoll",
            1000.0f,
            minc,
            0,
            "Physics maximum correcting velocity magic parameter (for ragdoll).");
        mind.value.max = FLT_MAX;
        mind.value.min = -FLT_MAX;
        phys_csl = Dvar_RegisterFloat("phys_csl", 1.0f, mind, 0, "Physics contact surface level magic parameter.");
        mine.value.max = FLT_MAX;
        mine.value.min = -FLT_MAX;
        phys_gravity = Dvar_RegisterFloat("phys_gravity", -800.0f, mine, 0, "Physics gravity in units/sec^2.");
        minf.value.max = 2.0f;
        minf.value.min = 0.0f;
        phys_bulletUpBias = Dvar_RegisterFloat(
            "phys_bulletUpBias",
            0.5f,
            minf,
            0,
            "Up Bias for the direction of the bullet impact.");
        ming.value.max = 100.0f;
        ming.value.min = -1.0f;
        phys_bulletSpinScale = Dvar_RegisterFloat(
            "phys_bulletSpinScale",
            3.0f,
            ming,
            0x1000u,
            "Scale of the effective offset from the center of mass for the bullet impacts.");
        phys_dumpcontacts = Dvar_RegisterBool(
            "phys_dumpcontacts",
            0,
            0,
            "Set to true to dump all constraints in next physics frame.");
        phys_qsi = Dvar_RegisterInt(
            "phys_qsi",
            15,
            (DvarLimits)0x7FFFFFFF00000001LL,
            0,
            "Number of iterations that QuickStep performs per step.");
        phys_drawcontacts = Dvar_RegisterBool("phys_drawcontacts", 0, 0, "Debug draw contact points");
        phys_drawCollisionWorld = Dvar_RegisterBool(
            "phys_drawCollisionWorld",
            0,
            0,
            "Debug draw collision brushes and terrain triangles");
        phys_drawCollisionObj = Dvar_RegisterBool(
            "phys_drawCollisionObj",
            0,
            0,
            "Debug draw collision geometry for each physics object");
        phys_drawAwake = Dvar_RegisterBool("phys_drawAwake", 0, 0, "Debug draw a box indicating which bodies are disabled");
        phys_drawAwakeTooLong = Dvar_RegisterBool(
            "phys_drawAwakeTooLong",
            0,
            0,
            "Draw an indicator showing where the objects are that have been awake too long.");
        phys_drawDebugInfo = Dvar_RegisterBool("phys_drawDebugInfo", 0, 0, "Print info about the physics objects");
        phys_visibleTris = Dvar_RegisterBool("phys_visibleTris", 0, 0, "Visible triangles are used for collision");
        phys_reorderConst = Dvar_RegisterBool("phys_reorderConst", 1, 0, "ODE solver reorder constraints");
        phys_noIslands = Dvar_RegisterBool(
            "phys_noIslands",
            0,
            0,
            "Make all contacts joints between an object and the world: no object-object contacts");
        phys_interBodyCollision = Dvar_RegisterBool(
            "phys_interBodyCollision",
            0,
            0,
            "Disable to turn off all inter-body collisions");
        phys_collUseEntities = Dvar_RegisterBool(
            "phys_collUseEntities",
            0,
            0,
            "Disable to turn off testing for collision against entities");
        minh.value.max = FLT_MAX;
        minh.value.min = 0.0f;
        phys_autoDisableLinear = Dvar_RegisterFloat(
            "phys_autoDisableLinear",
            20.0f,
            minh,
            0,
            "A body must have linear velocity less than this to be considered idle.");
        mini.value.max = FLT_MAX;
        mini.value.min = 0.0f;
        phys_autoDisableAngular = Dvar_RegisterFloat(
            "phys_autoDisableAngular",
            1.0f,
            mini,
            0,
            "A body must have angular velocity less than this to be considered idle.");
        minj.value.max = FLT_MAX;
        minj.value.min = 0.0f;
        phys_autoDisableTime = Dvar_RegisterFloat(
            "phys_autoDisableTime",
            0.89999998f,
            minj,
            0,
            "The amount of time a body must be idle for it to go to sleep.");
        mink.value.max = 1.0f;
        mink.value.min = 0.0f;
        phys_contact_cfm = Dvar_RegisterFloat(
            "phys_contact_cfm",
            0.0000099999997f,
            mink,
            0,
            "Physics constraint force mixing magic parameter for contacts.");
        minl.value.max = 1.0f;
        minl.value.min = 0.0f;
        phys_contact_erp = Dvar_RegisterFloat(
            "phys_contact_erp",
            0.80000001f,
            minl,
            0,
            "Physics error reduction magic parameter for contacts.");
        minm.value.max = 1.0f;
        minm.value.min = 0.0f;
        phys_contact_cfm_ragdoll = Dvar_RegisterFloat(
            "phys_contact_cfm_ragdoll",
            0.001f,
            minm,
            0,
            "Physics constraint force mixing magic parameter for contacts.");
        minn.value.max = 1.0f;
        minn.value.min = 0.0f;
        phys_contact_erp_ragdoll = Dvar_RegisterFloat(
            "phys_contact_erp_ragdoll",
            0.30000001f,
            minn,
            0,
            "Physics error reduction magic parameter for contacts.");
        mino.value.max = 1.0f;
        mino.value.min = 0.0f;
        phys_joint_cfm = Dvar_RegisterFloat(
            "phys_joint_cfm",
            0.000099999997f,
            mino,
            0,
            "Physics constraint force mixing magic parameter for joints.");
        minp.value.max = 1.0f;
        minp.value.min = 0.0f;
        phys_joint_stop_cfm = Dvar_RegisterFloat(
            "phys_joint_stop_cfm",
            0.000099999997f,
            minp,
            0,
            "Physics constraint force mixing magic parameter for joints at their limits.");
        minq.value.max = 1.0f;
        minq.value.min = 0.0f;
        phys_joint_stop_erp = Dvar_RegisterFloat(
            "phys_joint_stop_erp",
            0.80000001f,
            minq,
            0,
            "Physics error reduction magic parameter for joints at their limits.");
        minr.value.max = FLT_MAX;
        minr.value.min = 0.0f;
        phys_frictionScale = Dvar_RegisterFloat(
            "phys_frictionScale",
            1.0f,
            minr,
            0,
            "Scales the amount of physics friction globally.");
        mins.value.max = FLT_MAX;
        mins.value.min = 0.0f;
        phys_dragLinear = Dvar_RegisterFloat(
            "phys_dragLinear",
            0.029999999f,
            mins,
            0,
            "The amount of linear drag, applied globally");
        mint.value.max = FLT_MAX;
        mint.value.min = 0.0f;
        phys_dragAngular = Dvar_RegisterFloat(
            "phys_dragAngular",
            0.5f,
            mint,
            0,
            "The amount of angular drag, applied globally");
        minu.value.max = FLT_MAX;
        minu.value.min = 0.0f;
        phys_minImpactMomentum = Dvar_RegisterFloat(
            "phys_minImpactMomentum",
            250.0f,
            minu,
            0,
            "The minimum momentum required to trigger impact sounds");
        minv.value.max = FLT_MAX;
        minv.value.min = 0.1f;
        phys_jitterMaxMass = Dvar_RegisterFloat(
            "phys_jitterMaxMass",
            200.0f,
            minv,
            0,
            "Maximum mass to jitter - jitter will fall off up to this mass");
        minw.value.max = FLT_MAX;
        minw.value.min = 0.0f;
        phys_gravityChangeWakeupRadius = Dvar_RegisterFloat(
            "phys_gravityChangeWakeupRadius",
            120.0f,
            minw,
            0x1000u,
            "The radius around the player within which objects get awakened when gravity changes");
        minx.value.max = FLT_MAX;
        minx.value.min = 0.0f;
        phys_narrowObjMaxLength = Dvar_RegisterFloat(
            "phys_narrowObjMaxLength",
            4.0f,
            minx,
            0,
            "If a geom has a dimension less than this, then extra work will be done to prevent it fro"
            "m falling into cracks (like between the wall and the floor)");
        Cmd_AddCommandInternal("phys_stop", Phys_Stop_f, &Phys_Stop_f_VAR);
        Cmd_AddCommandInternal("phys_go", Phys_Go_f, &Phys_Go_f_VAR);
        Phys_InitBrushmodelGeomClass();
        Phys_InitBrushGeomClass();
        Phys_InitCylinderGeomClass();
        Phys_InitCapsuleGeomClass();
        Phys_InitWorldCollision();
        Phys_InitJoints();
        physGlob.gravityDirection[0] = 0.0f;
        physGlob.gravityDirection[1] = 0.0f;
        physGlob.gravityDirection[2] = -1.0f;
        physInited = 1;
    }
}

void __cdecl Phys_Go_f()
{
    ODE_ForEachBody<void(__cdecl *)(dxBody *)>(physGlob.world[0], Phys_EnableGeom);
}

void __cdecl Phys_EnableGeom(dxBody *body)
{
    dxGeom *geom; // [esp+4h] [ebp-4h]

    dBodyGetData(body);
    for (geom = ODE_BodyGetFirstGeom(body); geom; geom = dGeomGetBodyNext(geom))
        dGeomEnable(geom);
}

void __cdecl Phys_Stop_f()
{
    ODE_ForEachBody<void(__cdecl *)(dxBody *)>(physGlob.world[0], Phys_DisableBodyAndGeom);
}

void __cdecl Phys_DisableBodyAndGeom(dxBody *body)
{
    PhysObjUserData *userData; // [esp+0h] [ebp-8h]
    dxGeom *geom; // [esp+4h] [ebp-4h]

    userData = (PhysObjUserData *)dBodyGetData(body);
    dBodyDisable(userData->body);
    for (geom = ODE_BodyGetFirstGeom(body); geom; geom = dGeomGetBodyNext(geom))
        dGeomDisable(geom);
}

dxBody *__cdecl Phys_ObjCreateAxis(
    PhysWorld worldIndex,
    float *position,
    const float (*axis)[3],
    float *velocity,
    const PhysPreset *physPreset)
{
    BodyState state; // [esp+18h] [ebp-78h] BYREF

    iassert(!IS_NAN(position[0]) && !IS_NAN(position[1]) && !IS_NAN(position[2]));
    iassert(!IS_NAN(velocity[0]) && !IS_NAN(velocity[1]) && !IS_NAN(velocity[2]));

    iassert(physInited);
    iassert(physPreset);

    AxisCopy(*(const mat3x3*)axis, state.rotation);

    state.position[0] = position[0];
    state.position[1] = position[1];
    state.position[2] = position[2];

    state.velocity[0] = velocity[0];
    state.velocity[1] = velocity[1];
    state.velocity[2] = velocity[2];

    state.angVelocity[0] = 0.0;
    state.angVelocity[1] = 0.0;
    state.angVelocity[2] = 0.0;

    state.centerOfMassOffset[0] = 0.0;
    state.centerOfMassOffset[1] = 0.0;
    state.centerOfMassOffset[2] = 0.0;

    state.mass = physPreset->mass;
    state.bounce = physPreset->bounce;
    state.friction = physPreset->friction;
    state.state = 0;
    state.timeLastAsleep = physGlob.worldData[worldIndex].timeLastSnapshot; // LWSS: might wanna use timeLastUpdate instead here?
    state.type = physPreset->type;
    LOBYTE(state.underwater) = 1;

    return Phys_CreateBodyFromState(worldIndex, &state);
}

dxBody *__cdecl Phys_CreateBodyFromState(PhysWorld worldIndex, const BodyState *state)
{
    float *savedPos; // [esp+14h] [ebp-A4h]
    GeomState geomState; // [esp+28h] [ebp-90h] BYREF
    float rotMatrix[12]; // [esp+74h] [ebp-44h] BYREF
    PhysObjUserData *userData; // [esp+A4h] [ebp-14h]
    dxBody *body; // [esp+A8h] [ebp-10h]
    float centerOfMass[3]; // [esp+ACh] [ebp-Ch] BYREF

    iassert(state);
    iassert(physGlob.world[worldIndex]);
    iassert(physGlob.space[worldIndex]);

    dWorldSetAutoDisableLinearThreshold(physGlob.world[worldIndex], phys_autoDisableLinear->current.value);
    dWorldSetAutoDisableAngularThreshold(physGlob.world[worldIndex], phys_autoDisableAngular->current.value);
    dWorldSetAutoDisableTime(physGlob.world[worldIndex], phys_autoDisableTime->current.value);
    body = dBodyCreate(physGlob.world[worldIndex]);

    if (body)
    {
        if (worldIndex == PHYS_WORLD_RAGDOLL)
            dBodySetFiniteRotationMode(body, 1);
#ifdef USE_POOL_ALLOCATOR
        Sys_EnterCriticalSection(CRITSECT_PHYSICS);
        userData = (PhysObjUserData *)Pool_Alloc(&physGlob.userDataPool);
        Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
#else
        userData = (PhysObjUserData *)malloc(sizeof(PhysObjUserData));
#endif
        iassert(userData);
        memset((unsigned __int8 *)userData, 0, sizeof(PhysObjUserData));
        dBodySetData(body, userData);
        dBodySetPosition(body, state->position[0], state->position[1], state->position[2]);
        dBodySetLinearVel(body, state->velocity[0], state->velocity[1], state->velocity[2]);
        dBodySetAngularVel(body, state->angVelocity[0], state->angVelocity[1], state->angVelocity[2]);
        Phys_AxisToOdeMatrix3(state->rotation, rotMatrix);
        dBodySetRotation(body, rotMatrix);
        centerOfMass[0] = -state->centerOfMassOffset[0];
        centerOfMass[1] = -state->centerOfMassOffset[1];
        centerOfMass[2] = -state->centerOfMassOffset[2];
        geomState.isOriented = 0;
        geomState.type = PHYS_GEOM_NONE;
        userData->translation[0] = state->centerOfMassOffset[0];
        userData->translation[1] = state->centerOfMassOffset[1];
        userData->translation[2] = state->centerOfMassOffset[2];
        Phys_BodyAddGeomAndSetMass(worldIndex, body, state->mass, &geomState, centerOfMass);
        userData->body = body;
        savedPos = userData->savedPos;
        userData->savedPos[0] = state->position[0];
        savedPos[1] = state->position[1];
        savedPos[2] = state->position[2];
        qmemcpy(userData->savedRot, state->rotation, sizeof(userData->savedRot));
        userData->bounce = state->bounce;
        userData->friction = state->friction;
        userData->state = (physStuckState_t)state->state;
        userData->timeLastAsleep = state->timeLastAsleep;
        Phys_BodyGetCenterOfMass(body, userData->awakeTooLongLastPos);
        userData->sndClass = state->type;
        if (!LOBYTE(state->underwater))
            dBodyDisable(body);
        return body;
    }
    else
    {
        Com_PrintWarning(20, "Maximum number of physics bodies exceeded (more than %i)\n", 512);
        return 0;
    }
}

void __cdecl Phys_BodyGetCenterOfMass(dxBody *body, float *outPosition)
{
    const float *bodyPosition; // [esp+0h] [ebp-4h]

    bodyPosition = dBodyGetPosition(body);
    *outPosition = *bodyPosition;
    outPosition[1] = bodyPosition[1];
    outPosition[2] = bodyPosition[2];
}

void __cdecl Phys_BodyAddGeomAndSetMass(
    PhysWorld worldIndex,
    dxBody *body,
    float totalMass,
    GeomState *geomState,
    const float *centerOfMass)
{
    dxGeom *geom; // [esp+18h] [ebp-54h]
    dMass mass; // [esp+1Ch] [ebp-50h] BYREF
    dxGeom *geomTransform; // [esp+68h] [ebp-4h]

    dMassSetZero(&mass);
    if (totalMass <= 0.0)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 498, 0, "%s", "totalMass > 0");
    if (!dBodyGetData(body))
        MyAssertHandler(".\\physics\\phys_ode.cpp", 501, 0, "%s", "userData");
    geom = 0;
    Phys_AdjustForNewCenterOfMass(body, centerOfMass);
    switch (geomState->type)
    {
    case PHYS_GEOM_NONE:
        dMassSetSphereTotal(&mass, totalMass, 1.0);
        break;
    case PHYS_GEOM_BOX:
        dMassSetBoxTotal(
            &mass,
            totalMass,
            geomState->u.boxState.extent[0],
            geomState->u.cylinderState.radius,
            geomState->u.cylinderState.halfHeight);
        geom = dCreateBox(
            physGlob.space[worldIndex],
            body,
            geomState->u.boxState.extent[0],
            geomState->u.cylinderState.radius,
            geomState->u.cylinderState.halfHeight);
        if (!geom)
            Com_PrintWarning(20, "Maximum number of physics geoms exceeded\n");
        break;
    case PHYS_GEOM_BRUSHMODEL:
        Phys_MassSetBrushTotal(
            &mass,
            totalMass,
            geomState->u.brushState.momentsOfInertia,
            geomState->u.brushState.productsOfInertia);
        geom = Phys_CreateBrushmodelGeom(
            physGlob.space[worldIndex],
            body,
            geomState->u.brushState.u.brushModel,
            centerOfMass);
        if (!geom)
            Com_PrintWarning(20, "Maximum number of physics geoms exceeded\n");
        break;
    case PHYS_GEOM_BRUSH:
        Phys_MassSetBrushTotal(
            &mass,
            totalMass,
            geomState->u.brushState.momentsOfInertia,
            geomState->u.brushState.productsOfInertia);
        geom = Phys_CreateBrushGeom(physGlob.space[worldIndex], body, geomState->u.brushState.u.brush, centerOfMass);
        if (!geom)
            Com_PrintWarning(20, "Maximum number of physics geoms exceeded\n");
        break;
    case PHYS_GEOM_CYLINDER:
        dMassSetCylinderTotal(
            &mass,
            totalMass,
            geomState->u.cylinderState.direction,
            geomState->u.cylinderState.radius,
            geomState->u.cylinderState.halfHeight);
        geom = Phys_CreateCylinderGeom(physGlob.space[worldIndex], body, &geomState->u.cylinderState);
        if (!geom)
            Com_PrintWarning(20, "Maximum number of physics geoms exceeded\n");
        break;
    case PHYS_GEOM_CAPSULE:
        dMassSetCappedCylinderTotal(
            &mass,
            totalMass,
            geomState->u.cylinderState.direction,
            geomState->u.cylinderState.radius,
            geomState->u.cylinderState.halfHeight);
        geom = Phys_CreateCapsuleGeom(physGlob.space[worldIndex], body, &geomState->u.cylinderState);
        if (!geom)
            Com_PrintWarning(20, "Maximum number of physics geoms exceeded\n");
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler(".\\physics\\phys_ode.cpp", 548, 0, "invalid geometry type");
        break;
    }
    if (geomState->isOriented && geom)
    {
        geomTransform = dCreateGeomTransform(physGlob.space[worldIndex], body);
        if (geomTransform)
        {
            dGeomTransformSetGeom(geomTransform, geom);
            ODE_GeomTransformSetRotation(geomTransform, vec3_origin, geomState->orientation);
        }
        else
        {
            Com_PrintError(20, "Maximum number of physics geoms exceeded\n");
        }
    }
    dBodySetMass(body, &mass);
}

void __cdecl Phys_AdjustForNewCenterOfMass(dxBody *body, const float *newRelCenterOfMass)
{
    PhysObjUserData *bodyUserData; // [esp+10h] [ebp-74h]
    float geomOffset[3]; // [esp+14h] [ebp-70h] BYREF
    dxGeom *geom; // [esp+20h] [ebp-64h]
    float oldAbsCenterOfMass[3]; // [esp+24h] [ebp-60h] BYREF
    float rotation[3][3]; // [esp+30h] [ebp-54h] BYREF
    float objPosition[3]; // [esp+54h] [ebp-30h] BYREF
    float newAbsCenterOfMass[3]; // [esp+60h] [ebp-24h] BYREF
    float oldRelCenterOfMass[3]; // [esp+6Ch] [ebp-18h] BYREF
    float rotatedRelCenterOfMass[3]; // [esp+78h] [ebp-Ch] BYREF

    bodyUserData = (PhysObjUserData *)dBodyGetData(body);
    if (!bodyUserData)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 458, 0, "%s", "bodyUserData");
    Phys_BodyGetRotation(body, rotation);
    Phys_BodyGetCenterOfMass(body, oldAbsCenterOfMass);
    Phys_ObjGetPositionFromCenterOfMass(body, rotation, oldAbsCenterOfMass, objPosition);
    oldRelCenterOfMass[0] = -bodyUserData->translation[0];
    oldRelCenterOfMass[1] = -bodyUserData->translation[1];
    oldRelCenterOfMass[2] = -bodyUserData->translation[2];
    bodyUserData->translation[0] = -*newRelCenterOfMass;
    bodyUserData->translation[1] = -newRelCenterOfMass[1];
    bodyUserData->translation[2] = -newRelCenterOfMass[2];
    AxisTransformVec3(rotation, newRelCenterOfMass, rotatedRelCenterOfMass);
    Vec3Add(objPosition, rotatedRelCenterOfMass, newAbsCenterOfMass);
    dBodySetPosition(body, newAbsCenterOfMass[0], newAbsCenterOfMass[1], newAbsCenterOfMass[2]);
    for (geom = ODE_BodyGetFirstGeom(body); geom; geom = dGeomGetBodyNext(geom))
    {
        if (dGeomGetClass(geom) == dGeomTransformClass)
        {
            ODE_GeomTransformGetOffset(geom, geomOffset);
            Vec3Add(geomOffset, oldRelCenterOfMass, geomOffset);
            Vec3Sub(geomOffset, newRelCenterOfMass, geomOffset);
            ODE_GeomTransformSetOffset(geom, geomOffset);
        }
    }
    bodyUserData->savedPos[0] = newAbsCenterOfMass[0];
    bodyUserData->savedPos[1] = newAbsCenterOfMass[1];
    bodyUserData->savedPos[2] = newAbsCenterOfMass[2];
}

void __cdecl Phys_BodyGetRotation(dxBody *body, float (*outRotation)[3])
{
    const float *bodyRotation; // [esp+8h] [ebp-4h]

    if (!body)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 246, 0, "%s", "body");
    bodyRotation = dBodyGetRotation(body);
    Phys_OdeMatrix3ToAxis(bodyRotation, outRotation);
}

void __cdecl Phys_OdeMatrix3ToAxis(const float *inMatrix, float (*outAxis)[3])
{
    int r; // [esp+4h] [ebp-8h]
    int c; // [esp+8h] [ebp-4h]

    for (r = 0; r < 3; ++r)
    {
        for (c = 0; c < 3; ++c)
            (*outAxis)[3 * c + r] = inMatrix[4 * r + c];
    }
}

void __cdecl Phys_ObjGetPositionFromCenterOfMass(
    dxBody *body,
    const float (*rotation)[3],
    const float *centerOfGravity,
    float *objPos)
{
    PhysObjUserData *userData; // [esp+0h] [ebp-10h]
    float rotatedTrans[3]; // [esp+4h] [ebp-Ch] BYREF

    userData = (PhysObjUserData *)dBodyGetData(body);
    if (!userData)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 437, 0, "%s", "userData");
    AxisTransformVec3(*(const mat3x3*)rotation, userData->translation, rotatedTrans);
    Vec3Add(rotatedTrans, centerOfGravity, objPos);
}

void __cdecl Phys_MassSetBrushTotal(dMass *m, float totalMass, float *momentsOfInertia, const float *productsOfInertia)
{
    float I11; // [esp+28h] [ebp-24h]
    float I22; // [esp+2Ch] [ebp-20h]
    float I33; // [esp+30h] [ebp-1Ch]
    float I12; // [esp+34h] [ebp-18h]
    float I13; // [esp+38h] [ebp-14h]
    float I23; // [esp+3Ch] [ebp-10h]
    float v10; // [esp+40h] [ebp-Ch]
    float v11; // [esp+44h] [ebp-8h]
    float v12; // [esp+48h] [ebp-4h]

    if (momentsOfInertia[2] <= 0.0)
        v12 = 100.0;
    else
        v12 = momentsOfInertia[2];
    if (momentsOfInertia[1] <= 0.0)
        v11 = 100.0;
    else
        v11 = momentsOfInertia[1];
    if (*momentsOfInertia <= 0.0)
        v10 = 100.0;
    else
        v10 = *momentsOfInertia;
    I23 = totalMass * productsOfInertia[2];
    I13 = totalMass * productsOfInertia[1];
    I12 = totalMass * *productsOfInertia;
    I33 = totalMass * v12;
    I22 = totalMass * v11;
    I11 = totalMass * v10;
    dMassSetParameters(m, totalMass, 0.0, 0.0, 0.0, I11, I22, I33, I12, I13, I23);
}

dxBody *__cdecl Phys_ObjCreate(
    PhysWorld worldIndex,
    float *position,
    float *quat,
    float *velocity,
    const PhysPreset *physPreset)
{
    float axis[3][3]; // [esp+24h] [ebp-24h] BYREF

    if ((COERCE_UNSIGNED_INT(*position) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(position[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(position[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\physics\\phys_ode.cpp",
            677,
            0,
            "%s",
            "!IS_NAN((position)[0]) && !IS_NAN((position)[1]) && !IS_NAN((position)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*quat) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(quat[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(quat[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\physics\\phys_ode.cpp",
            678,
            0,
            "%s",
            "!IS_NAN((quat)[0]) && !IS_NAN((quat)[1]) && !IS_NAN((quat)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*velocity) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(velocity[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(velocity[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\physics\\phys_ode.cpp",
            679,
            0,
            "%s",
            "!IS_NAN((velocity)[0]) && !IS_NAN((velocity)[1]) && !IS_NAN((velocity)[2])");
    }
    if (!physInited)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 681, 0, "%s", "physInited");
    if (!physPreset)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 682, 0, "%s", "physPreset");
    QuatToAxis(quat, axis);
    return Phys_ObjCreateAxis(worldIndex, position, axis, velocity, physPreset);
}

void __cdecl Phys_ObjSetOrientation(
    PhysWorld worldIndex,
    dxBody *id,
    const float *newPosition,
    const float *newOrientation)
{
    PhysObjUserData *bodyUserData; // [esp+18h] [ebp-A4h]
    float newRotation[3][3]; // [esp+1Ch] [ebp-A0h] BYREF
    float newCenterOfGravity[3]; // [esp+40h] [ebp-7Ch] BYREF
    float rotatedCGOffset[3]; // [esp+4Ch] [ebp-70h] BYREF
    float newOdeRotation[12]; // [esp+58h] [ebp-64h] BYREF
    float oldRotation[3][3]; // [esp+88h] [ebp-34h] BYREF
    dxBody *body; // [esp+ACh] [ebp-10h]
    float oldPosition[3]; // [esp+B0h] [ebp-Ch] BYREF

    body = id;
    if (!id)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 702, 0, "%s", "body");
    bodyUserData = (PhysObjUserData *)dBodyGetData(body);
    if (!bodyUserData)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 705, 0, "%s", "bodyUserData");
    Phys_ObjGetPosition(id, oldPosition, oldRotation);
    Phys_BodyGetRotation(body, oldRotation);
    QuatToAxis(newOrientation, newRotation);
    AxisTransformVec3(newRotation, bodyUserData->translation, rotatedCGOffset);
    Vec3Sub(newPosition, rotatedCGOffset, newCenterOfGravity);
    Phys_AxisToOdeMatrix3(newRotation, newOdeRotation);
    bodyUserData->savedPos[0] = newCenterOfGravity[0];
    bodyUserData->savedPos[1] = newCenterOfGravity[1];
    bodyUserData->savedPos[2] = newCenterOfGravity[2];
    AxisCopy(newRotation, bodyUserData->savedRot);
    dBodySetPosition(body, newCenterOfGravity[0], newCenterOfGravity[1], newCenterOfGravity[2]);
    dBodySetRotation(body, newOdeRotation);
}

void __cdecl Phys_ObjAddGeomBox(PhysWorld worldIndex, dxBody *id, const float *boxMin, const float *boxMax)
{
    GeomState geomState; // [esp+Ch] [ebp-A0h] BYREF
    dxBody *body; // [esp+54h] [ebp-58h]
    float centerOfMass[3]; // [esp+58h] [ebp-54h] BYREF
    dMass mass; // [esp+64h] [ebp-48h] BYREF

    dMassSetZero(&mass);
    if (!id)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 731, 0, "%s", "id");
    body = id;
    geomState.type = PHYS_GEOM_BOX;
    Vec3Avg(boxMin, boxMax, centerOfMass);
    Vec3Sub(boxMax, boxMin, geomState.u.boxState.extent);
    geomState.isOriented = 0;
    dBodyGetMass(body, &mass);
    Phys_BodyAddGeomAndSetMass(worldIndex, body, mass.mass, &geomState, centerOfMass);
}

void __cdecl Phys_ObjAddGeomBoxRotated(
    PhysWorld worldIndex,
    dxBody *id,
    const float *center,
    const float *halfLengths,
    const float (*orientation)[3])
{
    GeomState geomState; // [esp+1Ch] [ebp-98h] BYREF
    dxBody *body; // [esp+68h] [ebp-4Ch]
    dMass mass; // [esp+6Ch] [ebp-48h] BYREF

    dMassSetZero(&mass);
    if (!id)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 753, 0, "%s", "id");
    body = id;
    geomState.type = PHYS_GEOM_BOX;
    Vec3Scale(halfLengths, 2.0, geomState.u.boxState.extent);
    geomState.isOriented = 1;
    geomState.orientation[0][0] = (*orientation)[0];
    geomState.orientation[0][1] = (*orientation)[1];
    geomState.orientation[0][2] = (*orientation)[2];
    geomState.orientation[1][0] = (*orientation)[3];
    geomState.orientation[1][1] = (*orientation)[4];
    geomState.orientation[1][2] = (*orientation)[5];
    geomState.orientation[2][0] = (*orientation)[6];
    geomState.orientation[2][1] = (*orientation)[7];
    geomState.orientation[2][2] = (*orientation)[8];
    dBodyGetMass(body, &mass);
    Phys_BodyAddGeomAndSetMass(worldIndex, body, mass.mass, &geomState, center);
}

void __cdecl Phys_ObjAddGeomBrushModel(
    PhysWorld worldIndex,
    dxBody *id,
    unsigned __int16 brushModel,
    const PhysMass *physMass)
{
    GeomState geomState; // [esp+14h] [ebp-98h] BYREF
    dxBody *body; // [esp+60h] [ebp-4Ch]
    dMass mass; // [esp+64h] [ebp-48h] BYREF

    dMassSetZero(&mass);
    if (!id)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 777, 0, "%s", "id");
    body = id;
    geomState.type = PHYS_GEOM_BRUSHMODEL;
    geomState.u.brushState.u.brushModel = brushModel;
    geomState.u.cylinderState.radius = physMass->momentsOfInertia[0];
    geomState.u.cylinderState.halfHeight = physMass->momentsOfInertia[1];
    geomState.u.brushState.momentsOfInertia[2] = physMass->momentsOfInertia[2];
    geomState.u.brushState.productsOfInertia[0] = physMass->productsOfInertia[0];
    geomState.u.brushState.productsOfInertia[1] = physMass->productsOfInertia[1];
    geomState.u.brushState.productsOfInertia[2] = physMass->productsOfInertia[2];
    geomState.isOriented = 0;
    dBodyGetMass(id, &mass);
    Phys_BodyAddGeomAndSetMass(worldIndex, id, mass.mass, &geomState, physMass->centerOfMass);
}

void __cdecl Phys_ObjAddGeomBrush(PhysWorld worldIndex, dxBody *id, const cbrush_t *brush, const PhysMass *physMass)
{
    GeomState geomState; // [esp+14h] [ebp-98h] BYREF
    dxBody *body; // [esp+60h] [ebp-4Ch]
    dMass mass; // [esp+64h] [ebp-48h] BYREF

    dMassSetZero(&mass);
    if (!id)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 798, 0, "%s", "id");
    body = id;
    geomState.type = PHYS_GEOM_BRUSH;
    geomState.u.cylinderState.direction = (int)brush;
    geomState.u.cylinderState.radius = physMass->momentsOfInertia[0];
    geomState.u.cylinderState.halfHeight = physMass->momentsOfInertia[1];
    geomState.u.brushState.momentsOfInertia[2] = physMass->momentsOfInertia[2];
    geomState.u.brushState.productsOfInertia[0] = physMass->productsOfInertia[0];
    geomState.u.brushState.productsOfInertia[1] = physMass->productsOfInertia[1];
    geomState.u.brushState.productsOfInertia[2] = physMass->productsOfInertia[2];
    geomState.isOriented = 0;
    dBodyGetMass(id, &mass);
    Phys_BodyAddGeomAndSetMass(worldIndex, id, mass.mass, &geomState, physMass->centerOfMass);
}

void __cdecl Phys_ObjAddGeomCylinder(PhysWorld worldIndex, dxBody *id, const float *boxMin, const float *boxMax)
{
    float v4; // [esp+Ch] [ebp-C0h]
    float v5; // [esp+10h] [ebp-BCh]
    GeomState geomState; // [esp+14h] [ebp-B8h] BYREF
    dxBody *body; // [esp+60h] [ebp-6Ch]
    float centerOfMass[3]; // [esp+64h] [ebp-68h] BYREF
    GeomStateCylinder *cyl; // [esp+70h] [ebp-5Ch]
    dMass mass; // [esp+74h] [ebp-58h] BYREF
    float extent[3]; // [esp+C0h] [ebp-Ch] BYREF

    dMassSetZero(&mass);
    if (!id)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 822, 0, "%s", "id");
    body = id;
    geomState.type = PHYS_GEOM_CYLINDER;
    cyl = &geomState.u.cylinderState;
    Vec3Avg(boxMin, boxMax, centerOfMass);
    Vec3Sub(boxMax, boxMin, extent);
    cyl->direction = 3;
    v5 = extent[0] - extent[1];
    if (v5 < 0.0)
        v4 = extent[1];
    else
        v4 = extent[0];
    cyl->radius = v4 * 0.5;
    cyl->halfHeight = extent[2] * 0.5;
    geomState.isOriented = 0;
    dBodyGetMass(body, &mass);
    Phys_BodyAddGeomAndSetMass(worldIndex, body, mass.mass, &geomState, centerOfMass);
}

void __cdecl Phys_ObjAddGeomCylinderDirection(
    PhysWorld worldIndex,
    dxBody *id,
    int direction,
    float radius,
    float halfHeight,
    const float *centerOfMass)
{
    GeomState geomState; // [esp+Ch] [ebp-98h] BYREF
    dxBody *body; // [esp+54h] [ebp-50h]
    GeomStateCylinder *cyl; // [esp+58h] [ebp-4Ch]
    dMass mass; // [esp+5Ch] [ebp-48h] BYREF

    dMassSetZero(&mass);
    if (!id)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 850, 0, "%s", "id");
    body = id;
    geomState.type = PHYS_GEOM_CYLINDER;
    cyl = &geomState.u.cylinderState;
    geomState.u.cylinderState.direction = direction + 1;
    if (direction + 1 < 1 || cyl->direction > 3)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 858, 0, "%s", "cyl->direction >= 1 && cyl->direction <= 3");
    cyl->radius = radius;
    cyl->halfHeight = halfHeight;
    geomState.isOriented = 0;
    dBodyGetMass(body, &mass);
    Phys_BodyAddGeomAndSetMass(worldIndex, body, mass.mass, &geomState, centerOfMass);
}

void __cdecl Phys_ObjAddGeomCylinderRotated(
    PhysWorld worldIndex,
    dxBody *id,
    int direction,
    float radius,
    float halfHeight,
    const float *center,
    const float (*orientation)[3])
{
    GeomState geomState; // [esp+1Ch] [ebp-98h] BYREF
    dxBody *body; // [esp+64h] [ebp-50h]
    GeomStateCylinder *cyl; // [esp+68h] [ebp-4Ch]
    dMass mass; // [esp+6Ch] [ebp-48h] BYREF

    dMassSetZero(&mass);
    if (!id)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 877, 0, "%s", "id");
    body = id;
    geomState.type = PHYS_GEOM_CYLINDER;
    cyl = &geomState.u.cylinderState;
    geomState.u.cylinderState.direction = direction + 1;
    if (direction + 1 < 1 || cyl->direction > 3)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 885, 0, "%s", "cyl->direction >= 1 && cyl->direction <= 3");
    cyl->radius = radius;
    cyl->halfHeight = halfHeight;
    geomState.isOriented = 1;
    geomState.orientation[0][0] = (*orientation)[0];
    geomState.orientation[0][1] = (*orientation)[1];
    geomState.orientation[0][2] = (*orientation)[2];
    geomState.orientation[1][0] = (*orientation)[3];
    geomState.orientation[1][1] = (*orientation)[4];
    geomState.orientation[1][2] = (*orientation)[5];
    geomState.orientation[2][0] = (*orientation)[6];
    geomState.orientation[2][1] = (*orientation)[7];
    geomState.orientation[2][2] = (*orientation)[8];
    dBodyGetMass(body, &mass);
    Phys_BodyAddGeomAndSetMass(worldIndex, body, mass.mass, &geomState, center);
}

void __cdecl Phys_ObjAddGeomCapsule(
    PhysWorld worldIndex,
    dxBody *id,
    int direction,
    float radius,
    float halfHeight,
    const float *centerOfMass)
{
    GeomState geomState; // [esp+Ch] [ebp-98h] BYREF
    dxBody *body; // [esp+54h] [ebp-50h]
    GeomStateCylinder *cyl; // [esp+58h] [ebp-4Ch]
    dMass mass; // [esp+5Ch] [ebp-48h] BYREF

    dMassSetZero(&mass);
    if (!id)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 907, 0, "%s", "id");
    body = id;
    geomState.type = PHYS_GEOM_CAPSULE;
    cyl = &geomState.u.cylinderState;
    geomState.u.cylinderState.direction = direction + 1;
    if (direction + 1 < 1 || cyl->direction > 3)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 915, 0, "%s", "cyl->direction >= 1 && cyl->direction <= 3");
    cyl->radius = radius;
    cyl->halfHeight = halfHeight;
    geomState.isOriented = 0;
    dBodyGetMass(body, &mass);
    Phys_BodyAddGeomAndSetMass(worldIndex, body, mass.mass, &geomState, centerOfMass);
}

void __cdecl Phys_ObjSetCollisionFromXModel(const XModel *model, PhysWorld worldIndex, dxBody *physId)
{
    float mins[3]; // [esp+10h] [ebp-24h] BYREF
    PhysGeomInfo *geom; // [esp+1Ch] [ebp-18h]
    float maxs[3]; // [esp+20h] [ebp-14h] BYREF
    PhysGeomList *geomList; // [esp+2Ch] [ebp-8h]
    unsigned int geomIndex; // [esp+30h] [ebp-4h]

    if (model->physGeoms)
    {
        geomList = model->physGeoms;
        geomIndex = 0;
        geom = geomList->geoms;
        while (geomIndex < geomList->count)
        {
            if (geom->brush)
            {
                Phys_ObjAddGeomBrush(worldIndex, physId, (const cbrush_t *)geom->brush, &geomList->mass);
            }
            else if (geom->type == 1)
            {
                Phys_ObjAddGeomBoxRotated(worldIndex, physId, geom->offset, geom->halfLengths, geom->orientation);
            }
            else
            {
                Phys_ObjAddGeomCylinderRotated(
                    worldIndex,
                    physId,
                    0,
                    geom->halfLengths[1],
                    geom->halfLengths[0],
                    geom->offset,
                    geom->orientation);
            }
            ++geomIndex;
            ++geom;
        }
        Phys_ObjSetInertialTensor(physId, &geomList->mass);
    }
    else
    {
        XModelGetBounds(model, mins, maxs);
        if (maxs[0] == mins[0] || maxs[1] == mins[1] || maxs[2] == mins[2])
        {
            mins[0] = -50.0;
            mins[1] = -50.0;
            mins[2] = -50.0;
            maxs[0] = 50.0;
            maxs[1] = 50.0;
            maxs[2] = 50.0;
        }
        Phys_ObjAddGeomBox(worldIndex, physId, mins, maxs);
    }
}

void __cdecl Phys_ObjSetAngularVelocity(dxBody *id, float *angularVel)
{
    if (!id)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 973, 0, "%s", "id");
    dBodySetAngularVel(id, angularVel[2], *angularVel, angularVel[1]);
}

void __cdecl Phys_ObjSetAngularVelocityRaw(dxBody *id, float *angularVel)
{
    if (!id)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 985, 0, "%s", "id");
    dBodySetAngularVel(id, *angularVel, angularVel[1], angularVel[2]);
}

void __cdecl Phys_ObjSetVelocity(dxBody *id, float *velocity)
{
    if (!id)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 997, 0, "%s", "id");
    dBodySetLinearVel(id, *velocity, velocity[1], velocity[2]);
}

void __cdecl Phys_ObjGetPosition(dxBody *id, float *outPosition, float (*outRotation)[3])
{
    const float *bodyRotation; // [esp+8h] [ebp-Ch]
    const float *bodyPosition; // [esp+Ch] [ebp-8h]

    if (!physInited)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 1012, 0, "%s", "physInited");
    if (!id)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 1013, 0, "%s", "id");
    bodyPosition = dBodyGetPosition(id);
    bodyRotation = dBodyGetRotation(id);
    Phys_OdeMatrix3ToAxis(bodyRotation, outRotation);
    *outPosition = *bodyPosition;
    outPosition[1] = bodyPosition[1];
    outPosition[2] = bodyPosition[2];
    Phys_ObjGetPositionFromCenterOfMass(id, outRotation, outPosition, outPosition);
}

void __cdecl Phys_ObjGetCenterOfMass(dxBody *id, float *outPosition)
{
    if (!id)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 1031, 0, "%s", "id");
    Phys_BodyGetCenterOfMass(id, outPosition);
}

void __cdecl Phys_ObjDestroy(PhysWorld worldIndex, dxBody *id)
{
    PhysObjUserData *userData; // [esp+0h] [ebp-8h]

    if (!physInited)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 1044, 0, "%s", "physInited");
    if (!id)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 1045, 0, "%s", "id");
    if (id->world != physGlob.world[worldIndex])
        MyAssertHandler(".\\physics\\phys_ode.cpp", 1048, 0, "%s", "body->world == physGlob.world[worldIndex]");
    userData = (PhysObjUserData *)dBodyGetData(id);
    dBodyDestroy(id);
#ifdef USE_POOL_ALLOCATOR
    Sys_EnterCriticalSection(CRITSECT_PHYSICS);
    Pool_Free((freenode *)userData, &physGlob.userDataPool);
    Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
#else
    free(userData);
#endif
}

void __cdecl Phys_ObjAddForce(PhysWorld worldIndex, dxBody *id, float *worldPos, const float *impulse)
{
    float fx; // [esp+18h] [ebp-24h]
    float fy; // [esp+1Ch] [ebp-20h]
    float fz; // [esp+20h] [ebp-1Ch]
    PhysObjUserData *userData; // [esp+28h] [ebp-14h]
    float SCALE; // [esp+30h] [ebp-Ch]
    dxWorld *odeWorld; // [esp+38h] [ebp-4h]

    SCALE = 1000.0 / (double)g_phys_msecStep[worldIndex];
    if (!physInited)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 1068, 0, "%s", "physInited");
    if (!id)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 1069, 0, "%s", "id");
    fz = impulse[2] * SCALE;
    fy = impulse[1] * SCALE;
    fx = *impulse * SCALE;
    dBodyAddForceAtPos(id, fx, fy, fz, *worldPos, worldPos[1], worldPos[2]);
    dBodyEnable(id);
    userData = (PhysObjUserData *)dBodyGetData(id);
    odeWorld = ODE_BodyGetWorld(id);
    userData->timeLastAsleep = (int)physGlob.space[51 * Phys_IndexFromODEWorld(odeWorld) - 152];
}

int __cdecl Phys_IndexFromODEWorld(dxWorld *world)
{
    int worldIndex; // [esp+0h] [ebp-4h]

    for (worldIndex = 0; worldIndex < 3; ++worldIndex)
    {
        if (world == physGlob.world[worldIndex])
            return worldIndex;
    }
    if (!alwaysfails)
        MyAssertHandler("c:\\trees\\cod3\\src\\physics\\phys_local.h", 269, 0, "Invalid ODE world");
    return 3;
}

void __cdecl Phys_ObjBulletImpact(
    PhysWorld worldIndex,
    dxBody *id,
    const float *worldPosRaw,
    const float *bulletDirRaw,
    float bulletSpeed,
    float scale)
{
    float v6; // [esp+8h] [ebp-14Ch]
    const float *AngularVel; // [esp+Ch] [ebp-148h]
    const float *LinearVel; // [esp+24h] [ebp-130h]
    float relativeBulletSpeed; // [esp+28h] [ebp-12Ch]
    float bodyAngularVelAroundAxis; // [esp+2Ch] [ebp-128h]
    float torqueAxisRelativeToBody[3]; // [esp+30h] [ebp-124h] BYREF
    float impactRelativeToBody[3]; // [esp+3Ch] [ebp-118h] BYREF
    float R; // [esp+48h] [ebp-10Ch]
    dxBody *body; // [esp+4Ch] [ebp-108h]
    float bodyVel[3]; // [esp+50h] [ebp-104h] BYREF
    float rotation[3][3]; // [esp+5Ch] [ebp-F8h] BYREF
    float centerOfMass[3]; // [esp+80h] [ebp-D4h] BYREF
    float denominator; // [esp+8Ch] [ebp-C8h]
    float changeInBodyMomentum[3]; // [esp+90h] [ebp-C4h] BYREF
    float bulletDir[3]; // [esp+9Ch] [ebp-B8h] BYREF
    float doubleDotTemp[3]; // [esp+A8h] [ebp-ACh] BYREF
    float momentOfInertia; // [esp+B4h] [ebp-A0h]
    float worldPos[3]; // [esp+B8h] [ebp-9Ch] BYREF
    dMass mass; // [esp+C4h] [ebp-90h] BYREF
    float ITensor[3][3]; // [esp+110h] [ebp-44h] BYREF
    float bodyAngularVel[3]; // [esp+134h] [ebp-20h] BYREF
    float BULLET_MASS; // [esp+140h] [ebp-14h]
    float numerator; // [esp+144h] [ebp-10h]
    float torqueAxis[3]; // [esp+148h] [ebp-Ch] BYREF

    BULLET_MASS = 0.5;
    dMassSetZero(&mass);
    if (!physInited)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 1132, 0, "%s", "physInited");
    if (!id)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 1133, 0, "%s", "id");
    body = id;
    Phys_BodyGetCenterOfMass(id, centerOfMass);
    bulletDir[0] = *bulletDirRaw;
    bulletDir[1] = bulletDirRaw[1];
    bulletDir[2] = bulletDirRaw[2];
    worldPos[0] = *worldPosRaw;
    worldPos[1] = worldPosRaw[1];
    worldPos[2] = worldPosRaw[2];
    Phys_TweakBulletImpact(worldPos, bulletDir, centerOfMass);
    LinearVel = dBodyGetLinearVel(id);
    bodyVel[0] = *LinearVel;
    bodyVel[1] = LinearVel[1];
    bodyVel[2] = LinearVel[2];
    relativeBulletSpeed = bulletSpeed - Vec3Dot(bodyVel, bulletDir);
    dBodyGetMass(id, &mass);
    Vec3Sub(worldPos, centerOfMass, impactRelativeToBody);
    Vec3Cross(impactRelativeToBody, bulletDir, torqueAxis);
    R = Vec3Normalize(torqueAxis);
    Phys_OdeMatrix3ToAxis(mass.I, ITensor);
    Phys_BodyGetRotation(body, rotation);
    MatrixTransformVector(torqueAxis, rotation, torqueAxisRelativeToBody);
    MatrixTransformVector(torqueAxisRelativeToBody, ITensor, doubleDotTemp);
    momentOfInertia = Vec3Dot(doubleDotTemp, torqueAxisRelativeToBody);
    if (momentOfInertia <= 0.0 && R != 0.0)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 1162, 0, "%s", "momentOfInertia > 0 || R == 0");
    AngularVel = dBodyGetAngularVel(body);
    bodyAngularVel[0] = *AngularVel;
    bodyAngularVel[1] = AngularVel[1];
    bodyAngularVel[2] = AngularVel[2];
    bodyAngularVelAroundAxis = Vec3Dot(bodyAngularVel, torqueAxis);
    numerator = (relativeBulletSpeed - bodyAngularVelAroundAxis * R) * 2.0 * BULLET_MASS;
    if (numerator > 0.0)
    {
        denominator = mass.mass + BULLET_MASS;
        if (R != 0.0)
            denominator = R * R * mass.mass * BULLET_MASS / momentOfInertia + denominator;
        if (denominator <= 0.0)
            MyAssertHandler(".\\physics\\phys_ode.cpp", 1257, 0, "%s", "denominator > 0");
        v6 = scale * mass.mass * numerator / denominator;
        Vec3Scale(bulletDir, v6, changeInBodyMomentum);
        Phys_ObjAddForce(worldIndex, id, worldPos, changeInBodyMomentum);
    }
}

void __cdecl Phys_TweakBulletImpact(float *worldPos, float *bulletDir, const float *centerOfMass)
{
    float offset[3]; // [esp+14h] [ebp-Ch] BYREF

    if (!phys_bulletUpBias)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 1095, 0, "%s", "phys_bulletUpBias");
    if (!phys_bulletSpinScale)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 1096, 0, "%s", "phys_bulletSpinScale");
    bulletDir[2] = bulletDir[2] + phys_bulletUpBias->current.value;
    Vec3Normalize(bulletDir);
    Vec3Sub(worldPos, centerOfMass, offset);
    Vec3Scale(offset, phys_bulletSpinScale->current.value, offset);
    Vec3Add(worldPos, offset, worldPos);
}

void __cdecl Phys_PlayCollisionSound(int localClientNum, dxBody *body, unsigned int sndClass, ContactList *contactList)
{
    double v4; // st7
    float scale; // [esp+8h] [ebp-74h]
    float velocity[3]; // [esp+Ch] [ebp-70h] BYREF
    float pos[3]; // [esp+18h] [ebp-64h] BYREF
    float impactVelocity; // [esp+24h] [ebp-58h]
    snd_alias_list_t *sound; // [esp+28h] [ebp-54h]
    dMass mass; // [esp+2Ch] [ebp-50h] BYREF
    int i; // [esp+78h] [ebp-4h]

    dMassSetZero(&mass);
    if (contactList->contactCount <= 0)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 1274, 0, "%s", "contactList->contactCount > 0");
    impactVelocity = 0.0;
    pos[0] = 0.0;
    pos[1] = 0.0;
    pos[2] = 0.0;
    dBodyGetMass(body, &mass);
    for (i = 0; i < contactList->contactCount; ++i)
    {
        Phys_BodyGetPointVelocity(body, contactList->contacts[i].contact.pos, velocity);
        v4 = Vec3Dot(velocity, contactList->contacts[i].contact.normal);
        impactVelocity = v4 + impactVelocity;
        Vec3Add(pos, contactList->contacts[i].contact.pos, pos);
    }
    impactVelocity = impactVelocity / (double)contactList->contactCount;
    scale = 1.0 / (double)contactList->contactCount;
    Vec3Scale(pos, scale, pos);
    if (-phys_minImpactMomentum->current.value > impactVelocity * mass.mass)
    {
        if (sndClass >= 0x32)
            MyAssertHandler(
                ".\\physics\\phys_ode.cpp",
                1293,
                0,
                "sndClass doesn't index AUDIOPHYS_CLASSMAX\n\t%i not in [0, %i)",
                sndClass,
                50);
        if (((contactList->contacts[0].surfFlags & 0x1F00000) >> 20) >= 0x1Du)
            MyAssertHandler(
                ".\\physics\\phys_ode.cpp",
                1294,
                0,
                "SURF_TYPEINDEX( contactList->contacts[0].surfFlags ) doesn't index SURF_TYPECOUNT\n\t%i not in [0, %i)",
                (contactList->contacts[0].surfFlags & 0x1F00000) >> 20,
                29);
        sound = cgMedia.physCollisionSound[sndClass][(contactList->contacts[0].surfFlags & 0x1F00000) >> 20];
        if (sound)
            SND_AddPhysicsSound(sound, pos);
    }
}

void __cdecl Phys_BodyGetPointVelocity(dxBody *body, float *point, float *outVelocity)
{
    float bodyVelocity[4]; // [esp+10h] [ebp-10h] BYREF

    dBodyGetPointVel(body, *point, point[1], point[2], bodyVelocity);
    *outVelocity = bodyVelocity[0];
    outVelocity[1] = bodyVelocity[1];
    outVelocity[2] = bodyVelocity[2];
}

void __cdecl Phys_DrawDebugText(const ScreenPlacement *scrPlace)
{
    int v1; // eax
    unsigned int totalBodiesAwake; // [esp+20h] [ebp-14h]
    unsigned int totalBodiesAwakea; // [esp+20h] [ebp-14h]
    float x; // [esp+24h] [ebp-10h] BYREF
    float y; // [esp+28h] [ebp-Ch] BYREF
    float charHeight; // [esp+2Ch] [ebp-8h]
    const char *text; // [esp+30h] [ebp-4h]

    x = 0.0;
    y = 72.0;
    charHeight = 12.0;
    totalBodiesAwake = Phys_DrawDebugTextForWorld(0, (char*)"Dynent Objects", &x, &y, 12.0, scrPlace);
    totalBodiesAwakea = totalBodiesAwake + Phys_DrawDebugTextForWorld(1u, (char *)"Fx Objects", &x, &y, charHeight, scrPlace);
    v1 = Phys_DrawDebugTextForWorld(2u, (char *)"Ragdoll Objects", &x, &y, charHeight, scrPlace);
    text = va("Total Objects Awake: %i", totalBodiesAwakea + v1);
    CG_DrawStringExt(scrPlace, x, y, (char *)text, colorGreen, 0, 1, charHeight);
}

int __cdecl Phys_DrawDebugTextForWorld(
    unsigned int worldIndex,
    char *worldText,
    float *x,
    float *y,
    float charHeight,
    const ScreenPlacement *scrPlace)
{
    char *v6; // eax
    char *text; // [esp+20h] [ebp-8h]
    int bodyCount; // [esp+24h] [ebp-4h]

    physGlob.debugActiveObjCount = 0;
    //bodyCount = XModelBoneNames((XModel *)physGlob.world[worldIndex]);
    bodyCount = physGlob.world[worldIndex]->nb;
    ODE_ForEachBody<void(__cdecl *)(dxBody *)>(physGlob.world[worldIndex], Phys_ObjCountIfActive);
    CG_DrawStringExt(scrPlace, *x, *y, worldText, colorGreen, 0, 1, charHeight);
    *y = *y + charHeight;
    v6 = va("   Awake: %i", physGlob.debugActiveObjCount);
    CG_DrawStringExt(scrPlace, *x, *y, v6, colorGreen, 0, 1, charHeight);
    *y = *y + charHeight;
    text = va("   Asleep: %i", (char *)bodyCount - physGlob.debugActiveObjCount);
    CG_DrawStringExt(scrPlace, *x, *y, text, colorGreen, 0, 1, charHeight);
    *y = *y + charHeight;
    return physGlob.debugActiveObjCount;
}

void __cdecl Phys_ObjCountIfActive(dxBody *body)
{
    if (dBodyIsEnabled(body))
        ++physGlob.debugActiveObjCount;
}

void __cdecl dxPostProcessIslands(PhysWorld worldIndex)
{
    float v1; // [esp+8h] [ebp-6Ch]
    float v2; // [esp+Ch] [ebp-68h]
    float v3; // [esp+10h] [ebp-64h]
    float v4; // [esp+34h] [ebp-40h]
    float v5; // [esp+38h] [ebp-3Ch]
    dxSpace *geom; // [esp+58h] [ebp-1Ch]
    dxBody *b; // [esp+5Ch] [ebp-18h]
    float seconds; // [esp+60h] [ebp-14h]
    dxBody *bodyIter; // [esp+68h] [ebp-Ch]
    dxWorld *world; // [esp+6Ch] [ebp-8h]
    int bodyEnableCount; // [esp+70h] [ebp-4h]

    PROF_SCOPED("Phys_PostStep");

    world = physGlob.world[worldIndex];
    for (b = world->firstbody; b; b = (dxBody *)b->next)
    {
        if ((b->flags & 4) == 0)
        {
            b->facc[0] = 0.0;
            b->facc[1] = 0.0;
            b->facc[2] = 0.0;
            b->tacc[0] = 0.0;
            b->tacc[1] = 0.0;
            b->tacc[2] = 0.0;
            for (geom = (dxSpace *)b->geom; geom; geom = (dxSpace *)dGeomGetBodyNext(geom))
                dGeomMoved(geom);
        }
    }
    dJointGroupEmpty(physGlob.contactgroup[worldIndex]);
    physGlob.space[51 * worldIndex - 149] = 0;
    seconds = world->seconds;
    bodyEnableCount = 0;
    for (bodyIter = world->firstbody; bodyIter; bodyIter = (dxBody *)bodyIter->next)
    {
        if (Phys_DoBodyOncePerFrame(worldIndex, bodyIter, seconds))
            ++bodyEnableCount;
    }
    v4 = (double)(bodyEnableCount - 32) / 18.0;
    v3 = v4 - 1.0;
    if (v3 < 0.0)
        v5 = (double)(bodyEnableCount - 32) / 18.0;
    else
        v5 = 1.0;
    v2 = 0.0 - v4;
    if (v2 < 0.0)
        v1 = v5;
    else
        v1 = 0.0;
    g_phys_msecStep[worldIndex] = g_phys_minMsecStep[worldIndex]
        + (int)((double)(g_phys_maxMsecStep[worldIndex] - g_phys_minMsecStep[worldIndex]) * v1);
        ODE_ForEachBody(world, Phys_CheckIfAliveTooLong);
}

void __cdecl Phys_CheckIfAliveTooLong(dxBody *body)
{
    float *awakeTooLongLastPos; // [esp+20h] [ebp-50h]
    float mins[3]; // [esp+28h] [ebp-48h] BYREF
    float size; // [esp+34h] [ebp-3Ch]
    float maxs[3]; // [esp+38h] [ebp-38h] BYREF
    float newPos[3]; // [esp+44h] [ebp-2Ch] BYREF
    float delta[3]; // [esp+50h] [ebp-20h] BYREF
    PhysObjUserData *userData; // [esp+5Ch] [ebp-14h]
    dxGeom *geom; // [esp+60h] [ebp-10h]
    unsigned int timeNow; // [esp+64h] [ebp-Ch]
    dxWorld *odeWorld; // [esp+68h] [ebp-8h]
    int type; // [esp+6Ch] [ebp-4h]

    userData = (PhysObjUserData *)dBodyGetData(body);
    if (!userData)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 1484, 0, "%s", "userData");
    geom = ODE_BodyGetFirstGeom(body);
    if (geom)
    {
        odeWorld = ODE_BodyGetWorld(body);
        timeNow = physGlob.worldData[Phys_IndexFromODEWorld(odeWorld)].timeLastUpdate; //  (unsigned int)physGlob.space[51 * Phys_IndexFromODEWorld(odeWorld) - 152];
        if (dBodyIsEnabled(body))
        {
            Phys_BodyGetCenterOfMass(body, newPos);
            if (phys_drawAwakeTooLong->current.enabled && userData->hasDisplayedAwakeTooLongWarning)
            {
                size = (double)(timeNow % 0x1F4) * 30.0 / 500.0 + 1.0;
                mins[0] = -size;
                mins[1] = mins[0];
                mins[2] = mins[0];
                maxs[0] = size;
                maxs[1] = size;
                maxs[2] = size;
                CG_DebugBox(newPos, mins, maxs, 0.0, colorRed, 0, 10);
            }
            type = dGeomGetClass(geom);
            if (type == 6)
            {
                geom = (dxGeom *)dGeomTransformGetGeom(geom);
                type = dGeomGetClass(geom);
            }
            if ((type == 13 || type == 14)
                && (Vec3Sub(newPos, userData->awakeTooLongLastPos, delta), Vec3LengthSq(delta) > 25.0))
            {
                awakeTooLongLastPos = userData->awakeTooLongLastPos;
                userData->awakeTooLongLastPos[0] = newPos[0];
                awakeTooLongLastPos[1] = newPos[1];
                awakeTooLongLastPos[2] = newPos[2];
                userData->timeLastAsleep = timeNow;
            }
            else if (timeNow - userData->timeLastAsleep > 0x2710)
            {
                userData->timeLastAsleep = timeNow - 5000;
                Com_PrintWarning(20, "Physics body awake too long (%.0f, %.0f, %.0f)\n", newPos[0], newPos[1], newPos[2]);
                userData->hasDisplayedAwakeTooLongWarning = 1;
            }
        }
        else
        {
            userData->timeLastAsleep = timeNow;
            Phys_BodyGetCenterOfMass(body, userData->awakeTooLongLastPos);
        }
    }
}

int __cdecl Phys_DoBodyOncePerFrame(unsigned int worldIndex, dxBody *body, float deltaT)
{
    float v4; // [esp+8h] [ebp-20h]
    float v5; // [esp+Ch] [ebp-1Ch]
    float scale; // [esp+10h] [ebp-18h]
    float v7; // [esp+14h] [ebp-14h]
    float angDamp[3]; // [esp+18h] [ebp-10h] BYREF
    float drag; // [esp+24h] [ebp-4h]

    if (!dBodyIsEnabled(body))
        return 0;
    if (worldIndex == 2)
    {
        Vec3Scale(body->info.avel, -0.0099999998f, angDamp);
        Vec3Add(angDamp, body->info.avel, body->info.avel);
    }
    else
    {
        drag = 1.0f - phys_dragLinear->current.value * deltaT;
        v7 = drag - 0.0f;
        if (v7 < 0.0f)
            scale = 0.0f;
        else
            scale = drag;
        drag = scale;
        Vec3Scale(body->info.lvel, scale, body->info.lvel);
        drag = 1.0f - phys_dragAngular->current.value * deltaT;
        v5 = drag - 0.0f;
        if (v5 < 0.0f)
            v4 = 0.0f;
        else
            v4 = drag;
        drag = v4;
        Vec3Scale(body->info.avel, v4, body->info.avel);
    }
    return 1;
}

void __cdecl Phys_GeomUserGetAAContainedBox(dxGeom *geom, float *mins, float *maxs)
{
    unsigned __int16 *ClassData; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    dxBody *Body; // eax
    dxWorld *World; // eax
    PhysWorld v9; // eax
    const char *v10; // eax
    const cbrush_t *brush; // [esp+38h] [ebp-20h]
    const cmodel_t *cmod; // [esp+3Ch] [ebp-1Ch]
    GeomStateCylinder *cyl; // [esp+40h] [ebp-18h]
    GeomStateCylinder *cyla; // [esp+40h] [ebp-18h]
    float lengths; // [esp+44h] [ebp-14h]
    float lengthsa; // [esp+44h] [ebp-14h]
    float minlength; // [esp+54h] [ebp-4h]

    if (!geom)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 1804, 0, "%s", "geom");
    switch (dGeomGetClass(geom))
    {
    case 11:
        ClassData = (unsigned __int16 *)dGeomGetClassData(geom);
        cmod = CM_ClipHandleToModel(*ClassData);
        Vec3Scale(cmod->mins, 0.0099999998f, mins);
        Vec3Scale(cmod->maxs, 0.0099999998f, maxs);
        ShrinkBoundsToHeight(mins, maxs);
        break;
    case 12:
        brush = *(const cbrush_t **)dGeomGetClassData(geom);
        Vec3Scale(brush->mins, 0.0099999998f, mins);
        Vec3Scale(brush->maxs, 0.0099999998f, maxs);
        if (*mins > *maxs || mins[1] > maxs[1] || mins[2] > maxs[2])
        {
            Com_PrintError(20, "Assert Info\n");
            v4 = va("brush: 0x%x, %i, 0x%x, 0x%x\n", brush, brush->numsides, brush->sides, brush->baseAdjacentSide);
            Com_PrintError(20, v4);
            v5 = va(
                "brush->edgeCount: %i %i %i, %i %i %i\n",
                brush->edgeCount[0][0],
                brush->edgeCount[0][1],
                brush->edgeCount[0][2],
                brush->edgeCount[1][0],
                brush->edgeCount[1][1],
                brush->edgeCount[1][2]);
            Com_PrintError(20, v5);
            v6 = va("mins/maxs: (%f %f %f), (%f %f %f)\n", *mins, mins[1], mins[2], *maxs, maxs[1], maxs[2]);
            Com_PrintError(20, v6);
            Body = dGeomGetBody(geom);
            World = ODE_BodyGetWorld(Body);
            v9 = (PhysWorld)Phys_IndexFromODEWorld(World);
            v10 = va("Physics world: %i\n", v9);
            Com_PrintError(20, v10);
            if (!alwaysfails)
                MyAssertHandler(
                    ".\\physics\\phys_ode.cpp",
                    1835,
                    0,
                    "physics bad-data assert.  (copy and paste the above assert info into the bug report with this assert (and/or"
                    " attatch the log file))");
        }
        ShrinkBoundsToHeight(mins, maxs);
        break;
    case 13:
        cyl = (GeomStateCylinder *)dGeomGetClassData(geom);
        lengths = cyl->radius * 0.7071067690849304;
        minlength = lengths;
        if (cyl->halfHeight < (double)lengths)
            minlength = cyl->halfHeight;
        if (minlength <= 0.0)
            MyAssertHandler(".\\physics\\phys_ode.cpp", 1849, 0, "%s", "minlength > 0");
        goto LABEL_15;
    case 14:
        cyla = (GeomStateCylinder *)dGeomGetClassData(geom);
        lengthsa = cyla->radius * 0.7071067690849304;
        minlength = lengthsa;
        if (lengthsa > cyla->radius + cyla->radius + cyla->halfHeight)
            minlength = cyla->radius + cyla->radius + cyla->halfHeight;
        if (minlength <= 0.0)
            MyAssertHandler(".\\physics\\phys_ode.cpp", 1860, 0, "%s", "minlength > 0");
    LABEL_15:
        *mins = -minlength;
        mins[1] = -minlength;
        mins[2] = -minlength;
        *maxs = minlength;
        maxs[1] = minlength;
        maxs[2] = minlength;
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler(".\\physics\\phys_ode.cpp", 1866, 0, "Invalid geom");
        break;
    }
}

int __cdecl Phys_ObjGetSnapshot(PhysWorld worldIndex, dxBody *id, float *outPos, float (*outMat)[3])
{
    PhysObjUserData *userData; // [esp+8h] [ebp-8h]

    userData = (PhysObjUserData *)dBodyGetData(id);
    Phys_ObjGetPositionFromCenterOfMass(id, userData->savedRot, userData->savedPos, outPos);
    memcpy(outMat, userData->savedRot, 0x24u);
    return physGlob.worldData[worldIndex].timeLastSnapshot;
}

void __cdecl Phys_RewindCurrentTime(PhysWorld worldIndex, int timeNow)
{
    float newFrac; // [esp+8h] [ebp-8h]

    if (physGlob.worldData[worldIndex].timeLastUpdate <= physGlob.worldData[worldIndex].timeLastSnapshot)
        goto LABEL_6;
    newFrac = (double)((int)timeNow - physGlob.worldData[worldIndex].timeLastSnapshot)
        / (double)(physGlob.worldData[worldIndex].timeLastUpdate
            - physGlob.worldData[worldIndex].timeLastSnapshot);
    if (physGlob.worldData[worldIndex].timeNowLerpFrac <= newFrac)
        return;
    if (newFrac < 0.0 || newFrac > 1.0)
    {
    LABEL_6:
        physGlob.worldData[worldIndex].timeLastSnapshot = timeNow;
        physGlob.worldData[worldIndex].timeLastUpdate = timeNow;
        physGlob.worldData[worldIndex].timeNowLerpFrac = 1.0;
    }
    else
    {
        physGlob.worldData[worldIndex].timeNowLerpFrac = newFrac;
    }
}

void __cdecl Phys_GetPerformance(float *average, int *mintime, int *maxtime)
{
    Sys_EnterCriticalSection(CRITSECT_PHYSICS);
    *average = physGlob.performanceAverage;
    *mintime = physGlob.performanceMintime;
    *maxtime = physGlob.performanceMaxtime;
    Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
}

void __cdecl Phys_PerformanceEndFrame()
{
    unsigned int total; // [esp+8h] [ebp-8h]
    unsigned int frameIndex; // [esp+Ch] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_PHYSICS);
    ++physGlob.physPerformanceFrame;
    physGlob.performanceAverage = 0.0;
    physGlob.performanceMintime = 0;
    physGlob.performanceMaxtime = 0;
    if (physGlob.physPerformanceFrame > 0xA)
    {
        total = 0;
        physGlob.performanceMintime = 0x7FFFFFFF;
        physGlob.performanceMaxtime = 0;
        for (frameIndex = 0; frameIndex < 0xA; ++frameIndex)
        {
            total += physGlob.physPreviousFrameTimes[frameIndex];
            if (physGlob.physPreviousFrameTimes[frameIndex] > physGlob.performanceMaxtime)
                physGlob.performanceMaxtime = physGlob.physPreviousFrameTimes[frameIndex];
            if (physGlob.physPreviousFrameTimes[frameIndex] < physGlob.performanceMintime)
                physGlob.performanceMintime = physGlob.physPreviousFrameTimes[frameIndex];
        }
        physGlob.performanceAverage = (double)total / 10.0;
    }
    physGlob.physPreviousFrameTimes[physGlob.physPerformanceFrame % 0xA] = 0;
    Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
}

void __cdecl Phys_RunToTime(int localClientNum, PhysWorld worldIndex, int timeNow)
{
    DWORD v3; // eax
    float seconds; // [esp+20h] [ebp-5Ch]
    unsigned int v5; // [esp+2Ch] [ebp-50h]
    PhysWorldData *data; // [esp+6Ch] [ebp-10h]
    DWORD time; // [esp+70h] [ebp-Ch]
    dxWorld *world; // [esp+74h] [ebp-8h]
    unsigned int maxIter; // [esp+78h] [ebp-4h]

    data = &physGlob.worldData[worldIndex];

    iassert(physInited);

    PROF_SCOPED("Phys_RunToTime");
    KISAK_NULLSUB();
    time = Sys_Milliseconds();
    if (timeNow < data->timeLastSnapshot)
        Phys_RewindCurrentTime(worldIndex, timeNow);
    world = physGlob.world[worldIndex];
    if (data->timeLastUpdate < timeNow)
    {
        data->timeLastSnapshot = data->timeLastUpdate;
        ODE_ForEachBody(world, Phys_BodyGrabSnapshot);
        maxIter = 2;
        do
        {
            if (!maxIter)
                MyAssertHandler(".\\physics\\phys_ode.cpp", 2173, 0, "%s", "maxIter");
            if (g_phys_msecStep[worldIndex] < ((timeNow - data->timeLastUpdate) / maxIter))
                v5 = (timeNow - data->timeLastUpdate) / maxIter;
            else
                v5 = g_phys_msecStep[worldIndex];
            --maxIter;
            seconds = v5 * EQUAL_EPSILON;
            Phys_RunFrame(localClientNum, worldIndex, seconds);
            data->timeLastUpdate += v5;
            dxPostProcessIslands(worldIndex);
        } while (data->timeLastUpdate < timeNow);
        ODE_ForEachBody(world, Phys_DoBodyOncePerRun);
    }
    if (phys_drawAwake->current.enabled || phys_drawCollisionObj->current.enabled)
        ODE_ForEachBody(world, Phys_ObjDraw);
    if (data->timeLastSnapshot > timeNow || timeNow > data->timeLastUpdate)
        MyAssertHandler(
            ".\\physics\\phys_ode.cpp",
            2201,
            0,
            "timeNow not in [data.timeLastSnapshot, data.timeLastUpdate]\n\t%i not in [%i, %i]",
            timeNow,
            data->timeLastSnapshot,
            data->timeLastUpdate);
    if (data->timeLastUpdate <= data->timeLastSnapshot)
    {
        if (data->timeLastUpdate != data->timeLastSnapshot)
            MyAssertHandler(".\\physics\\phys_ode.cpp", 2209, 0, "%s", "data.timeLastUpdate == data.timeLastSnapshot");
        data->timeNowLerpFrac = 1.0;
    }
    else
    {
        data->timeNowLerpFrac = (timeNow - data->timeLastSnapshot) / (data->timeLastUpdate - data->timeLastSnapshot);
        if (data->timeNowLerpFrac < 0.0 || data->timeNowLerpFrac > 1.0)
            MyAssertHandler(
                ".\\physics\\phys_ode.cpp",
                2205,
                0,
                "data.timeNowLerpFrac not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
                data->timeNowLerpFrac,
                0.0,
                1.0);
    }
    v3 = Sys_Milliseconds();
    Phys_PerformanceAddTime(v3 - time);
}

void __cdecl Phys_ObjDraw(dxBody *body)
{
    const float *v1; // eax
    float v2; // [esp+8h] [ebp-A8h]
    const float *v3; // [esp+18h] [ebp-98h]
    const dReal *Position; // [esp+24h] [ebp-8Ch]
    float pos[3]; // [esp+28h] [ebp-88h] BYREF
    dxGeom *geomIter; // [esp+34h] [ebp-7Ch]
    float mins[3]; // [esp+38h] [ebp-78h] BYREF
    PhysObjUserData *userData; // [esp+44h] [ebp-6Ch]
    dxGeom *geom; // [esp+48h] [ebp-68h]
    float rotation[3][3]; // [esp+4Ch] [ebp-64h] BYREF
    int cylAxis; // [esp+70h] [ebp-40h]
    float maxs[3]; // [esp+74h] [ebp-3Ch] BYREF
    GeomStateCylinder *cyl; // [esp+80h] [ebp-30h]
    float endpos[2][3]; // [esp+84h] [ebp-2Ch] BYREF
    float lengths[4]; // [esp+9Ch] [ebp-14h] BYREF
    int type; // [esp+ACh] [ebp-4h]

    Position = dBodyGetPosition(body);
    pos[0] = *Position;
    pos[1] = Position[1];
    pos[2] = Position[2];
    if (phys_drawAwake->current.enabled && dBodyIsEnabled(body))
    {
        mins[0] = -4.0;
        mins[1] = -4.0;
        mins[2] = -4.0;
        maxs[0] = 4.0;
        maxs[1] = 4.0;
        maxs[2] = 4.0;
        CG_DebugBox(pos, mins, maxs, 0.0, colorLtBlue, 0, 3);
    }

    if (phys_drawCollisionObj->current.enabled)
    {
        userData = (PhysObjUserData*)dBodyGetData(body);
        for (geomIter = ODE_BodyGetFirstGeom(body); ; geomIter = dGeomGetBodyNext(geomIter))
        {
            if (!geomIter)
                return;
            geom = geomIter;
            type = dGeomGetClass(geomIter);
            if (type == 6)
            {
                geom = ODE_GeomTransformUpdateGeomOrientation((dxGeomTransform*)geom);
                type = dGeomGetClass(geom);
            }
            v1 = dGeomGetRotation(geom);
            Phys_OdeMatrix3ToAxis(v1, rotation);
            v3 = dGeomGetPosition(geom);
            pos[0] = *v3;
            pos[1] = v3[1];
            pos[2] = v3[2];
            if (type == 1)
            {
                dGeomBoxGetLengths(geom, lengths);
                Vec3Scale(lengths, 0.5, maxs);
                mins[0] = -maxs[0];
                mins[1] = -maxs[1];
                mins[2] = -maxs[2];
                CG_DebugBoxOriented(pos, mins, maxs, rotation, colorLtGreen, 0, 3);
                continue;
            }
            if (type == 13)
                break;
            if (type == 14)
            {
                cyl = (GeomStateCylinder*)dGeomGetClassData(geom);
                if (cyl->direction < 1 || cyl->direction > 3)
                    MyAssertHandler(".\\physics\\phys_ode.cpp", 1459, 0, "%s", "cyl->direction >= 1 && cyl->direction <= 3");
            LABEL_22:
                cylAxis = cyl->direction - 1;
                Vec3Mad(pos, cyl->halfHeight, rotation[cylAxis], endpos[0]);
                v2 = -cyl->halfHeight;
                Vec3Mad(pos, v2, rotation[cylAxis], endpos[1]);
                CG_DebugLine(endpos[0], endpos[1], colorLtGreen, 0, 3);
                CG_DebugCircle(endpos[0], cyl->radius, rotation[cylAxis], colorLtGreen, 0, 2);
                CG_DebugCircle(endpos[1], cyl->radius, rotation[cylAxis], colorLtGreen, 0, 2);
                continue;
            }
        }
        cyl = (GeomStateCylinder*)dGeomGetClassData(geom);
        if (cyl->direction < 1 || cyl->direction > 3)
            MyAssertHandler(".\\physics\\phys_ode.cpp", 1448, 0, "%s", "cyl->direction >= 1 && cyl->direction <= 3");
        goto LABEL_22;
    }
}

void __cdecl Phys_NearCallback(void *userData, dxGeom *geom1, dxGeom *geom2)
{
    float v3; // [esp+8h] [ebp-30B4h]
    float value; // [esp+Ch] [ebp-30B0h]
    float v5; // [esp+10h] [ebp-30ACh]
    float v6; // [esp+14h] [ebp-30A8h]
    float v7; // [esp+18h] [ebp-30A4h]
    float v8; // [esp+1Ch] [ebp-30A0h]
    dxBody *v9; // [esp+20h] [ebp-309Ch]
    float v10; // [esp+40h] [ebp-307Ch]
    float bounce; // [esp+44h] [ebp-3078h]
    PhysObjUserData *v12; // [esp+60h] [ebp-305Ch]
    PhysObjUserData *Data; // [esp+60h] [ebp-305Ch]
    ContactList out; // [esp+64h] [ebp-3058h] BYREF
    float v15; // [esp+1870h] [ebp-184Ch]
    dxBody *b1; // [esp+1874h] [ebp-1848h]
    dSurfaceParameters surfParms; // [esp+1878h] [ebp-1844h] BYREF
    ContactList contactArray; // [esp+18A4h] [ebp-1818h] BYREF
    PhysWorld worldIndex; // [esp+30ACh] [ebp-10h]
    float v20; // [esp+30B0h] [ebp-Ch]
    dxBody *b2; // [esp+30B4h] [ebp-8h]
    FrameInfo *v22; // [esp+30B8h] [ebp-4h]

    PROF_SCOPED("Phys_NearCallback");

    v12 = 0;
    v22 = (FrameInfo*)userData;
    worldIndex = (PhysWorld)v22->worldIndex;
    b1 = dGeomGetBody(geom1);
    b2 = dGeomGetBody(geom2);
    if (b1 && b2 && dAreConnectedExcluding(b1, b2, 4))
    {
        return;
    }
    else
    {
        contactArray.contactCount = dCollide(geom1, geom2, 128, (dContactGeom *)&contactArray.contacts[0], sizeof(contactArray.contacts[0]));
        if (b1)
            v9 = b2 == 0 ? b1 : 0;
        else
            v9 = b2 != 0 ? b2 : 0;
        if (contactArray.contactCount <= 0)
        {
            if (v9)
            {
                Data = (PhysObjUserData *)dBodyGetData(v9);
                iassert(Data);
                Data->state = PHYS_OBJ_STATE_FREE;
            }
        }
        else
        {
            v20 = 0.0;
            v15 = 1.0;
            if (b1)
            {
                v12 = (PhysObjUserData *)dBodyGetData(b1);
                bounce = v12->bounce;
                v8 = v15 - bounce;
                if (v8 < 0.0)
                    v7 = v15;
                else
                    v7 = bounce;
                v15 = v7;
                v20 = v20 + v12->friction;
            }
            if (b2)
            {
                v12 = (PhysObjUserData *)dBodyGetData(b2);
                v10 = v12->bounce;
                v6 = v15 - v10;
                if (v6 < 0.0)
                    v5 = v15;
                else
                    v5 = v10;
                v15 = v5;
                v20 = v20 + v12->friction;
            }
            surfParms.mode = 0x301C;
            iassert(phys_contact_cfm);
            iassert(phys_contact_erp);
            if (worldIndex == PHYS_WORLD_RAGDOLL)
                value = phys_contact_cfm_ragdoll->current.value;
            else
                value = phys_contact_cfm->current.value;
            surfParms.soft_cfm = value;
            if (worldIndex == PHYS_WORLD_RAGDOLL)
                v3 = phys_contact_erp_ragdoll->current.value;
            else
                v3 = phys_contact_erp->current.value;
            surfParms.soft_erp = v3;
            surfParms.mu = v20 * phys_frictionScale->current.value;
            surfParms.mu2 = 0.0f;
            surfParms.bounce = v15;
            surfParms.bounce_vel = 0.1f;
            if (contactArray.contactCount >= 5)
            {
                Phys_ReduceContacts(v9, &contactArray, &out);
                Phys_CreateJointForEachContact(&out, b1, b2, &surfParms, worldIndex);
                if (v9)
                    Phys_PlayCollisionSound(v22->localClientNum, v9, v12->sndClass, &out);
                else
                    Phys_PlayCollisionSound(v22->localClientNum, b1, v12->sndClass, &out);
            }
            else
            {
                Phys_CreateJointForEachContact(&contactArray, b1, b2, &surfParms, worldIndex);
                if (v9)
                    Phys_PlayCollisionSound(v22->localClientNum, v9, v12->sndClass, &contactArray);
                else
                    Phys_PlayCollisionSound(v22->localClientNum, b1, v12->sndClass, &contactArray);
            }
        }
    }
}

void __cdecl ODE_CollideSimpleSpaceWithGeomNoAABBTest(dxSpace *space, dxGeom *geom, void *data);

void __cdecl Phys_RunFrame(int localClientNum, PhysWorld worldIndex, float seconds)
{
    float scale; // [esp+4h] [ebp-80h]
    float value; // [esp+Ch] [ebp-78h]
    dxWorld *world; // [esp+6Ch] [ebp-18h]
    float down[3]; // [esp+70h] [ebp-14h] BYREF
    FrameInfo frameInfo; // [esp+7Ch] [ebp-8h] BYREF

    if (!physInited)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 1653, 0, "%s", "physInited");
    world = physGlob.world[worldIndex];
    world->seconds = seconds;

    PROF_SCOPED("Phys_RunServerFrame");

    if (phys_dumpcontacts->current.enabled)
    {
        physGlob.dumpContacts = 1;
        Dvar_SetBool((dvar_s *)phys_dumpcontacts, 0);
    }
    dWorldSetCFM(world, phys_cfm->current.value);
    dWorldSetERP(world, phys_erp->current.value);
    if (worldIndex == PHYS_WORLD_RAGDOLL)
        value = phys_mcv_ragdoll->current.value;
    else
        value = phys_mcv->current.value;
    dWorldSetContactMaxCorrectingVel(world, value);
    dWorldSetContactSurfaceLayer(world, phys_csl->current.value);
    dWorldSetQuickStepNumIterations(world, phys_qsi->current.integer);
    dWorldSetAutoDisableLinearThreshold(world, phys_autoDisableLinear->current.value);
    dWorldSetAutoDisableAngularThreshold(world, phys_autoDisableAngular->current.value);
    dWorldSetAutoDisableTime(world, phys_autoDisableTime->current.value);
    if (phys_visibleTris->current.enabled)
    {
        if (physGlob.visTrisGeom)
            dGeomEnable(physGlob.visTrisGeom);
        if (!physGlob.worldGeom)
            MyAssertHandler(".\\physics\\phys_ode.cpp", 1683, 0, "%s", "physGlob.worldGeom");
        dGeomDisable(physGlob.worldGeom);
    }
    else
    {
        if (physGlob.visTrisGeom)
            dGeomDisable(physGlob.visTrisGeom);
        if (!physGlob.worldGeom)
            MyAssertHandler(".\\physics\\phys_ode.cpp", 1690, 0, "%s", "physGlob.worldGeom");
        dGeomEnable(physGlob.worldGeom);
    }
    scale = -phys_gravity->current.value;
    Vec3Scale(physGlob.gravityDirection, scale, down);
    dWorldSetGravity(world, down[0], down[1], down[2]);
    {
        PROF_SCOPED("Phys_Collide");
        if (phys_interBodyCollision->current.enabled)
            dSpaceCollide(physGlob.space[worldIndex], &worldIndex, Phys_NearCallback);
        if (physGlob.worldData[worldIndex].collisionCallback)
            physGlob.worldData[worldIndex].collisionCallback();
        frameInfo.localClientNum = localClientNum;
        frameInfo.worldIndex = worldIndex;
        ODE_CollideSimpleSpaceWithGeomNoAABBTest(physGlob.space[worldIndex], physGlob.worldGeom, &frameInfo);
    }
    {
        PROF_SCOPED("Phys_Step");
        dWorldQuickStep(world, seconds);
    }
    physGlob.dumpContacts = 0;
}

void __cdecl Phys_BodyGrabSnapshot(dxBody *body)
{
    PhysObjUserData *userData; // [esp+30h] [ebp-4h]

    if (!body)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 1783, 0, "%s", "body");
    userData = (PhysObjUserData *)dBodyGetData(body);
    if (!userData)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 1786, 0, "%s", "userData");
    Phys_BodyGetCenterOfMass(body, userData->savedPos);
    if ((COERCE_UNSIGNED_INT(userData->savedPos[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(userData->savedPos[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(userData->savedPos[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\physics\\phys_ode.cpp",
            1789,
            0,
            "%s",
            "!IS_NAN((userData->savedPos)[0]) && !IS_NAN((userData->savedPos)[1]) && !IS_NAN((userData->savedPos)[2])");
    }
    Phys_BodyGetRotation(body, userData->savedRot);
    if ((COERCE_UNSIGNED_INT(userData->savedRot[0][0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(userData->savedRot[0][1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(userData->savedRot[0][2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\physics\\phys_ode.cpp",
            1791,
            0,
            "%s",
            "!IS_NAN((userData->savedRot[0])[0]) && !IS_NAN((userData->savedRot[0])[1]) && !IS_NAN((userData->savedRot[0])[2])");
    }
    if ((COERCE_UNSIGNED_INT(userData->savedRot[1][0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(userData->savedRot[1][1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(userData->savedRot[1][2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\physics\\phys_ode.cpp",
            1792,
            0,
            "%s",
            "!IS_NAN((userData->savedRot[1])[0]) && !IS_NAN((userData->savedRot[1])[1]) && !IS_NAN((userData->savedRot[1])[2])");
    }
    if ((COERCE_UNSIGNED_INT(userData->savedRot[2][0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(userData->savedRot[2][1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(userData->savedRot[2][2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\physics\\phys_ode.cpp",
            1793,
            0,
            "%s",
            "!IS_NAN((userData->savedRot[2])[0]) && !IS_NAN((userData->savedRot[2])[1]) && !IS_NAN((userData->savedRot[2])[2])");
    }
}

void __cdecl Phys_DoBodyOncePerRun(dxBody *body)
{
    float mins[3]; // [esp+30h] [ebp-1Ch] BYREF
    float maxs[3]; // [esp+3Ch] [ebp-10h] BYREF
    int dim; // [esp+48h] [ebp-4h]

    if (dBodyIsEnabled(body))
    {
        {
            PROF_SCOPED("Phys_Traces");
            Phys_ObjTraceNewPos(body);
        }
        for (dim = 0; dim != 3; ++dim)
        {
            CM_ModelBounds(0, mins, maxs);
            if (maxs[dim] < (double)body->info.pos[dim] || mins[dim] > (double)body->info.pos[dim])
            {
                dBodyDisable(body);
                return;
            }
        }
    }
}

void __cdecl Phys_ObjTraceNewPos(dxBody *body)
{
    float number; // [esp+8h] [ebp-E0h]
    float fraction; // [esp+Ch] [ebp-DCh]
    float v4; // [esp+10h] [ebp-D8h]
    bool v5; // [esp+14h] [ebp-D4h]
    float v6; // [esp+18h] [ebp-D0h]
    float v7; // [esp+1Ch] [ebp-CCh]
    float v8; // [esp+20h] [ebp-C8h]
    float v9; // [esp+24h] [ebp-C4h]
    float delta[3]; // [esp+58h] [ebp-90h] BYREF
    float invDist; // [esp+64h] [ebp-84h]
    float value; // [esp+68h] [ebp-80h]
    float startMaxs[3]; // [esp+6Ch] [ebp-7Ch] BYREF
    float newPos[3]; // [esp+78h] [ebp-70h] BYREF
    PhysObjUserData *userData; // [esp+84h] [ebp-64h]
    float mins[3]; // [esp+88h] [ebp-60h] BYREF
    dxGeom *geom; // [esp+94h] [ebp-54h]
    float PHYS_TRACE_BOX_MIN; // [esp+98h] [ebp-50h]
    float startMins[3]; // [esp+9Ch] [ebp-4Ch] BYREF
    float PHYS_TRACE_BOX_SCALE; // [esp+A8h] [ebp-40h]
    float maxs[3]; // [esp+ACh] [ebp-3Ch] BYREF
    trace_t trace; // [esp+B8h] [ebp-30h] BYREF
    bool isTooNarrow; // [esp+E7h] [ebp-1h]

    PHYS_TRACE_BOX_SCALE = 0.2f;
    PHYS_TRACE_BOX_MIN = 0.001f;
    if (dBodyIsEnabled(body))
    {
        userData = (PhysObjUserData *)dBodyGetData(body);
        if (!userData)
            MyAssertHandler(".\\physics\\phys_ode.cpp", 1890, 0, "%s", "userData");
        iassert(!IS_NAN((userData->savedPos)[0]) && !IS_NAN((userData->savedPos)[1]) && !IS_NAN((userData->savedPos)[2]));
        geom = ODE_BodyGetFirstGeom(body);
        if (geom)
        {
            isTooNarrow = 0;
            ODE_GeomGetAAContainedBox((dxGeomTransform *)geom, startMins, startMaxs);
            v9 = phys_csl->current.value + phys_csl->current.value;
            if (startMaxs[0] >= (double)v9)
            {
                Vec3Scale(startMins, PHYS_TRACE_BOX_SCALE, mins);
                Vec3Scale(startMaxs, PHYS_TRACE_BOX_SCALE, maxs);
                v8 = phys_csl->current.value + phys_csl->current.value;
                if (v8 > startMaxs[0] - maxs[0])
                {
                    value = startMaxs[0] - (phys_csl->current.value + phys_csl->current.value);
                    v7 = value - PHYS_TRACE_BOX_MIN;
                    if (v7 < 0.0)
                        v6 = PHYS_TRACE_BOX_MIN;
                    else
                        v6 = value;
                    value = v6;
                    maxs[0] = v6;
                    maxs[1] = v6;
                    maxs[2] = v6;
                    mins[0] = -v6;
                    mins[1] = mins[0];
                    mins[2] = mins[0];
                }
            }
            else
            {
                mins[0] = -PHYS_TRACE_BOX_MIN;
                mins[1] = mins[0];
                mins[2] = mins[0];
                maxs[0] = PHYS_TRACE_BOX_MIN;
                maxs[1] = PHYS_TRACE_BOX_MIN;
                maxs[2] = PHYS_TRACE_BOX_MIN;
                isTooNarrow = 1;
            }
            Phys_BodyGetCenterOfMass(body, newPos);
            
            iassert(!IS_NAN((newPos)[0]) && !IS_NAN((newPos)[1]) && !IS_NAN((newPos)[2]));

            v5 = newPos[0] == userData->savedPos[0]
                && newPos[1] == userData->savedPos[1]
                && newPos[2] == userData->savedPos[2];
            if (!v5 || userData->state <= (unsigned int)PHYS_OBJ_STATE_STUCK)
            {
                CM_BoxTrace(&trace, userData->savedPos, newPos, mins, maxs, 0, 0x2806C91);
                userData->state = trace.startsolid ? PHYS_OBJ_STATE_STUCK : PHYS_OBJ_STATE_FREE;
                if (trace.fraction < 1.0 && !trace.startsolid)
                {
                    if (isTooNarrow)
                    {
                        Vec3Sub(newPos, userData->savedPos, delta);
                        number = Vec3LengthSq(delta);
                        invDist = Q_rsqrt(number);
                        trace.fraction = trace.fraction - startMaxs[0] * 0.5 * invDist;
                        v4 = 0.0 - trace.fraction;
                        if (v4 < 0.0)
                            fraction = trace.fraction;
                        else
                            fraction = 0.0;
                        trace.fraction = fraction;
                    }
                    Vec3Lerp(userData->savedPos, newPos, trace.fraction, newPos);
                    dBodySetPosition(body, newPos[0], newPos[1], newPos[2]);
                    if (trace.fraction < EQUAL_EPSILON)
                    {
                        dBodySetLinearVel(body, 0.0, 0.0, 0.0);
                        dBodySetAngularVel(body, 0.0, 0.0, 0.0);
                    }
                }
            }
        }
    }
}

void __cdecl Phys_PerformanceAddTime(int time)
{
    Sys_EnterCriticalSection(CRITSECT_PHYSICS);
    physGlob.physPreviousFrameTimes[physGlob.physPerformanceFrame % 0xA] += time;
    Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
}

void __cdecl Phys_ObjGetInterpolatedState(PhysWorld worldIndex, dxBody *id, float *outPos, float *outQuat)
{
    float oldMat[3][3]; // [esp+8h] [ebp-84h] BYREF
    float newPos[3]; // [esp+2Ch] [ebp-60h] BYREF
    const float *frac; // [esp+38h] [ebp-54h]
    float newMat[3][3]; // [esp+3Ch] [ebp-50h] BYREF
    float oldPos[3]; // [esp+60h] [ebp-2Ch] BYREF
    float newQuat[4]; // [esp+6Ch] [ebp-20h] BYREF
    float oldQuat[4]; // [esp+7Ch] [ebp-10h] BYREF

    frac = &physGlob.worldData[worldIndex].timeNowLerpFrac;
    Phys_ObjGetSnapshot(worldIndex, id, oldPos, oldMat);
    Phys_ObjGetPosition(id, newPos, newMat);
    Vec3Lerp(oldPos, newPos, *frac, outPos);
    AxisToQuat(oldMat, oldQuat);
    AxisToQuat(newMat, newQuat);
    QuatLerp(oldQuat, newQuat, *frac, outQuat);
    Vec4Normalize(outQuat);
}

void __cdecl Phys_ObjSetInertialTensor(dxBody *id, const PhysMass *physMass)
{
    dMass mass; // [esp+10h] [ebp-48h] BYREF

    dMassSetZero(&mass);
    if (!id)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 2302, 0, "%s", "id");
    if (!physMass)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 2303, 0, "%s", "physMass");
    dBodyGetMass(id, &mass);
    Phys_MassSetBrushTotal(&mass, mass.mass, (float*)physMass->momentsOfInertia, physMass->productsOfInertia);
    dBodySetMass(id, &mass);
    Phys_AdjustForNewCenterOfMass(id, physMass->centerOfMass);
}

bool __cdecl Phys_ObjIsAsleep(dxBody *id)
{
    return !dBodyIsEnabled(id);
}

void __cdecl Phys_Shutdown()
{
    unsigned int v0; // eax
    int worldIndex; // [esp+0h] [ebp-4h]

    if (physInited)
    {
        vassert(physGlob.world[PHYS_WORLD_DYNENT]->nb == 0, "physGlob.world[PHYS_WORLD_DYNENT]->nb = %d", physGlob.world[PHYS_WORLD_DYNENT]->nb);
        vassert(physGlob.world[PHYS_WORLD_FX]->nb == 0, "physGlob.world[PHYS_WORLD_FX]->nb = %d", physGlob.world[PHYS_WORLD_FX]->nb);
        vassert(physGlob.world[PHYS_WORLD_RAGDOLL]->nb == 0, "physGlob.world[PHYS_WORLD_RAGDOLL]->nb = %d", physGlob.world[PHYS_WORLD_RAGDOLL]->nb);

        int freeCount = Pool_FreeCount(&physGlob.userDataPool);
        vassert(freeCount == ARRAY_COUNT(physGlob.userData), "userdata physobj free count = %d", freeCount);

        ODE_LeakCheck();
        Cmd_RemoveCommand("phys_stop");
        Cmd_RemoveCommand("phys_go");
        for (worldIndex = 0; worldIndex < 3; ++worldIndex)
        {
            if (physGlob.contactgroup[worldIndex])
            {
                dJointGroupDestroy(physGlob.contactgroup[worldIndex]);
                physGlob.contactgroup[worldIndex] = 0;
            }
            if (physGlob.space[worldIndex])
            {
                dSpaceDestroy(physGlob.space[worldIndex]);
                physGlob.space[worldIndex] = 0;
            }
            if (physGlob.world[worldIndex])
            {
                dWorldDestroy(physGlob.world[worldIndex]);
                physGlob.world[worldIndex] = 0;
            }
        }
        dCloseODE();
        physGlob.triMeshInfo.verts = 0;
        physInited = 0;
    }
}

void __cdecl Phys_ObjSave(dxBody *id, MemoryFile *memFile)
{
    BodyState state; // [esp+0h] [ebp-70h] BYREF

    if (!physInited)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 2439, 0, "%s", "physInited");
    if (!id)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 2440, 0, "%s", "id");
    Phys_GetStateFromBody(id, &state);
    MemFile_WriteData(memFile, 112, &state);
}

void __cdecl Phys_GetStateFromBody(dxBody *body, BodyState *state)
{
    float *Rotation; // eax
    float *AngularVel; // [esp+8h] [ebp-64h]
    float *LinearVel; // [esp+10h] [ebp-5Ch]
    const dReal *Position; // [esp+1Ch] [ebp-50h]
    PhysObjUserData *userData; // [esp+20h] [ebp-4Ch]
    dMass mass; // [esp+24h] [ebp-48h] BYREF

    dMassSetZero(&mass);
    iassert(body);
    iassert(state);
    userData = (PhysObjUserData *)dBodyGetData(body);
    iassert(userData);

    Position = dBodyGetPosition(body);
    state->position[0] = Position[0];
    state->position[1] = Position[1];
    state->position[2] = Position[2];

    Rotation = (float*)dBodyGetRotation(body);
    Phys_OdeMatrix3ToAxis(Rotation, state->rotation);

    LinearVel = (float *)dBodyGetLinearVel(body);
    state->velocity[0] = LinearVel[0];
    state->velocity[1] = LinearVel[1];
    state->velocity[2] = LinearVel[2];

    AngularVel = (float *)dBodyGetAngularVel(body);
    state->angVelocity[0] = AngularVel[0];
    state->angVelocity[1] = AngularVel[1];
    state->angVelocity[2] = AngularVel[2];

    state->centerOfMassOffset[0] = userData->translation[0];
    state->centerOfMassOffset[1] = userData->translation[1];
    state->centerOfMassOffset[2] = userData->translation[2];

    dBodyGetMass(body, &mass);
    iassert(mass.mass != 0.0f);
    state->mass = mass.mass;

    state->bounce = userData->bounce;
    state->friction = userData->friction;
    state->state = userData->state;
    state->timeLastAsleep = userData->timeLastAsleep;
    state->type = userData->sndClass;
    LOBYTE(state->underwater) = dBodyIsEnabled(body);
}

dxBody *__cdecl Phys_ObjLoad(PhysWorld worldIndex, MemoryFile *memFile)
{
    BodyState state;

    iassert(physInited);
    MemFile_ReadData(memFile, sizeof(BodyState), (unsigned __int8 *)&state);
    return Phys_CreateBodyFromState(worldIndex, &state);
}

void __cdecl Phys_InitJoints()
{
    //PhysStaticArray<dxJointHinge, 192>::init(&physGlob.hingeArray);
    //PhysStaticArray<dxJointBall, 160>::init(&physGlob.ballArray);
    //PhysStaticArray<dxJointAMotor, 160>::init(&physGlob.aMotorArray);
    physGlob.hingeArray.init();
    physGlob.ballArray.init();
    physGlob.aMotorArray.init();
}

void __cdecl Phys_SetHingeParams(
    PhysWorld worldIndex,
    dxJointHinge *id,
    float motorSpeed,
    float motorMaxForce,
    float lowStop,
    float highStop)
{
    iassert(id);
    iassert(id->typenum == dJointTypeHinge);

    dJointSetHingeParam(id, 2, motorSpeed);
    dJointSetHingeParam(id, 3, motorMaxForce);
    dJointSetHingeParam(id, 4, 0.89999998f);
    dJointSetHingeParam(id, 6, phys_joint_cfm->current.value);
    dJointSetHingeParam(id, 8, phys_joint_stop_cfm->current.value);
    dJointSetHingeParam(id, 7, phys_joint_stop_erp->current.value);
    dJointSetHingeParam(id, 0, lowStop);
    dJointSetHingeParam(id, 1, highStop);
}

dxJointHinge *__cdecl Phys_CreateHinge(
    PhysWorld worldIndex,
    dxBody *obj1,
    dxBody *obj2,
    float *anchor,
    float *axis,
    float motorSpeed,
    float motorMaxForce,
    float lowStop,
    float highStop)
{
    dxJointHinge *joint; // [esp+10h] [ebp-10h]
    dxJointHinge *jointa; // [esp+10h] [ebp-10h]

    //joint = (dxJointBall *)PhysStaticArray<dxJointHinge, 192>::allocate(&physGlob.hingeArray);
    joint = physGlob.hingeArray.allocate();
    if (joint)
    {
        jointa = (dxJointHinge*)dJointCreateHinge(physGlob.world[worldIndex], joint);
        if (jointa)
        {
            dJointAttach(jointa, obj1, obj2);
            dJointSetHingeAnchor(jointa, *anchor, anchor[1], anchor[2]);
            dJointSetHingeAxis(jointa, *axis, axis[1], axis[2]);
            Phys_SetHingeParams(worldIndex, jointa, motorSpeed, motorMaxForce, lowStop, highStop);
            return jointa;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_PrintWarning(20, "Physics: Out of hinge joints (%d max)\n", 192);
        return 0;
    }
}

dxJointBall *__cdecl Phys_CreateBallAndSocket(PhysWorld worldIndex, dxBody *obj1, dxBody *obj2, float *anchor)
{
    dxJointBall *joint; // [esp+Ch] [ebp-Ch]
    dxJointBall *jointa; // [esp+Ch] [ebp-Ch]

    //joint = PhysStaticArray<dxJointBall, 160>::allocate(&physGlob.ballArray);
    joint = physGlob.ballArray.allocate();
    if (joint)
    {
        jointa = (dxJointBall*)dJointCreateBall(physGlob.world[worldIndex], joint);
        dJointAttach(jointa, obj1, obj2);
        dJointSetBallAnchor(jointa, *anchor, anchor[1], anchor[2]);
        return jointa;
    }
    else
    {
        Com_PrintWarning(20, "Physics: Out of ball and socket joints (%d max)\n", 160);
        return 0;
    }
}

void __cdecl Phys_SetAngularMotorParams(
    PhysWorld worldIndex,
    dxJointAMotor *id,
    const float *motorSpeeds,
    const float *motorFMaxs,
    const float *lowStops,
    const float *highStops)
{
    int group; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]

    if (!id)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 2551, 0, "%s", "id");
    if (id->typenum != 8)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 2555, 0, "%s", "joint->typenum == dJointTypeAMotor");
    i = 0;
    group = 0;
    while (i < 3)
    {
        dJointSetAMotorParam(id, group, lowStops[i]);
        dJointSetAMotorParam(id, group + 1, highStops[i]);
        dJointSetAMotorParam(id, group + 2, motorSpeeds[i]);
        dJointSetAMotorParam(id, group + 3, motorFMaxs[i]);
        dJointSetAMotorParam(id, group + 4, 0.89999998f);
        dJointSetAMotorParam(id, group + 6, phys_joint_cfm->current.value);
        dJointSetAMotorParam(id, group + 8, phys_joint_stop_cfm->current.value);
        dJointSetAMotorParam(id, group + 7, phys_joint_stop_erp->current.value);
        ++i;
        group += 256;
    }
}

dxJointAMotor *__cdecl Phys_CreateAngularMotor(
    PhysWorld worldIndex,
    dxBody *obj1,
    dxBody *obj2,
    unsigned int numAxes,
    const float (*axes)[3],
    const float *motorSpeeds,
    const float *motorFMaxs,
    const float *lowStops,
    const float *highStops)
{
    dxJointAMotor *joint; // [esp+Ch] [ebp-10h]
    dxJointAMotor *jointa; // [esp+Ch] [ebp-10h]

    if (numAxes >= 4)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 2578, 0, "%s", "numAxes >= 0 && numAxes <= 3");
    //joint = (dxJointBall *)PhysStaticArray<dxJointAMotor, 160>::allocate(&physGlob.aMotorArray);
    joint = physGlob.aMotorArray.allocate();
    if (joint)
    {
        jointa = (dxJointAMotor *)dJointCreateAMotor(physGlob.world[worldIndex], (dxJointAMotor *)joint);
        if (jointa)
        {
            dJointAttach(jointa, obj1, obj2);
            dJointSetAMotorMode(jointa, 1);
            dJointSetAMotorNumAxes(jointa, 3);
            if (obj2)
                dJointSetAMotorAxis(jointa, 2, 2, (*axes)[6], (*axes)[7], (*axes)[8]);
            else
                dJointSetAMotorAxis(jointa, 2, 0, (*axes)[6], (*axes)[7], (*axes)[8]);
            if (obj1)
                dJointSetAMotorAxis(jointa, 0, 1, (*axes)[0], (*axes)[1], (*axes)[2]);
            else
                dJointSetAMotorAxis(jointa, 0, 0, (*axes)[0], (*axes)[1], (*axes)[2]);
            Phys_SetAngularMotorParams(worldIndex, jointa, motorSpeeds, motorFMaxs, lowStops, highStops);
            return jointa;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_PrintWarning(20, "Physics: Out of angular motor joints (%d max)\n", 160);
        return 0;
    }
}

void __cdecl Phys_JointDestroy(PhysWorld worldIndex, dxJointHinge *id)
{
    if (!id)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 2631, 0, "%s", "id");

    dJointDestroy(id);

    if (physGlob.hingeArray.isMember(id))
    {
        physGlob.hingeArray.release(id);
    }
    else if (physGlob.ballArray.isMember(id))
    {
        physGlob.ballArray.release((dxJointBall*)id);
    }
    else if (physGlob.aMotorArray.isMember(id))
    {
        physGlob.aMotorArray.release((dxJointAMotor*)id);
    }
    else if (!alwaysfails)
    {
        MyAssertHandler(".\\physics\\phys_ode.cpp", 2642, 0, "Phys_JointDestroy: Tried to destroy joint of unknown type");
    }
}

void __cdecl Phys_SetCollisionCallback(PhysWorld worldIndex, void(__cdecl *callback)())
{
    if ((unsigned int)worldIndex >= PHYS_WORLD_COUNT)
        MyAssertHandler(
            ".\\physics\\phys_ode.cpp",
            2648,
            0,
            "worldIndex doesn't index PHYS_WORLD_COUNT\n\t%i not in [0, %i)",
            worldIndex,
            3);
    physGlob.worldData[worldIndex].collisionCallback = callback;
}

void __cdecl Phys_AddJitterRegion(
    PhysWorld worldIndex,
    const float *origin,
    float innerRadius,
    float outerRadius,
    float minDisplacement,
    float maxDisplacement)
{
    double v6; // st7
    PhysWorldData *worldData; // [esp+0h] [ebp-24h]
    float pos[3]; // [esp+4h] [ebp-20h] BYREF
    float delta[3]; // [esp+10h] [ebp-14h] BYREF
    Jitter *jitter; // [esp+1Ch] [ebp-8h]
    dxBody *body; // [esp+20h] [ebp-4h]

    if ((unsigned int)worldIndex >= PHYS_WORLD_COUNT)
        MyAssertHandler(
            ".\\physics\\phys_ode.cpp",
            2688,
            0,
            "worldIndex doesn't index PHYS_WORLD_COUNT\n\t%i not in [0, %i)",
            worldIndex,
            3);
    worldData = &physGlob.worldData[worldIndex];
    if (worldData->numJitterRegions < 5)
        jitter = &worldData->jitterRegions[worldData->numJitterRegions++];
    else
        jitter = &worldData->jitterRegions[4];
    jitter->origin[0] = *origin;
    jitter->origin[1] = origin[1];
    jitter->origin[2] = origin[2];
    jitter->innerRadius = innerRadius;
    jitter->outerRadius = outerRadius;
    jitter->innerRadiusSq = innerRadius * innerRadius;
    jitter->outerRadiusSq = outerRadius * outerRadius;
    jitter->minDisplacement = minDisplacement;
    jitter->maxDisplacement = maxDisplacement;
    for (body = physGlob.world[worldIndex]->firstbody; body; body = (dxBody *)body->next)
    {
        Phys_ObjGetCenterOfMass(body, pos);
        Vec3Sub(pos, origin, delta);
        delta[2] = 0.0;
        v6 = Vec3LengthSq(delta);
        if (jitter->outerRadiusSq >= v6)
            dBodyEnable(body);
    }
}

void __cdecl Phys_ObjSetContactCentroid(dxBody *id, const float *worldPos)
{
    PhysObjUserData *userData; // [esp+4h] [ebp-8h]

    if (!id)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 2727, 0, "%s", "id");
    userData = (PhysObjUserData *)dBodyGetData(id);
    if (!userData)
        MyAssertHandler(".\\physics\\phys_ode.cpp", 2731, 0, "%s", "userData");
    userData->contactCentroid[0] = *worldPos;
    userData->contactCentroid[1] = worldPos[1];
    userData->contactCentroid[2] = worldPos[2];
}

