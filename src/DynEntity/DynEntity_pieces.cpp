#include "DynEntity_client.h"
#include <win32/win_local.h>
#include <gfx_d3d/r_dpvs.h>
#include <universal/profile.h>

const dvar_t *dynEntPieces_velocity;
const dvar_t *dynEntPieces_angularVelocity;
const dvar_t *dynEntPieces_impactForce;

int numPieces;
BreakablePiece g_breakablePieces[100];

void __cdecl DynEntPieces_RegisterDvars()
{
    DvarLimits min; // [esp+Ch] [ebp-10h]
    DvarLimits mina; // [esp+Ch] [ebp-10h]
    DvarLimits minb; // [esp+Ch] [ebp-10h]

    min.value.max = 1000.0;
    min.value.min = -1000.0;
    dynEntPieces_velocity = Dvar_RegisterVec3(
        "dynEntPieces_velocity",
        0.0,
        0.0,
        0.0,
        min,
        0x80u,
        "Initial breakable pieces velocity");
    mina.value.max = 180.0;
    mina.value.min = -180.0;
    dynEntPieces_angularVelocity = Dvar_RegisterVec3(
        "dynEntPieces_angularVelocity",
        0.0,
        0.0,
        0.0,
        mina,
        0x80u,
        "Initial breakable pieces angular velocity");
    minb.value.max = 1000000.0;
    minb.value.min = 0.0;
    dynEntPieces_impactForce = Dvar_RegisterFloat(
        "dynEntPieces_impactForce",
        1000.0,
        minb,
        0x80u,
        "Force applied when breakable is destroyed");
}

void __cdecl DynEntPieces_AddDrawSurfs()
{
    GfxScaledPlacement placement; // [esp+30h] [ebp-24h] BYREF
    int i; // [esp+50h] [ebp-4h]

    PROF_SCOPED("DynEntCl_AddBreakableDrawSurfs");
    for (i = 0; i < numPieces; ++i)
    {
        if (g_breakablePieces[i].active)
        {
            Sys_EnterCriticalSection(CRITSECT_PHYSICS);
            Phys_ObjGetInterpolatedState(
                PHYS_WORLD_FX,
                (dxBody *)g_breakablePieces[i].physObjId,
                placement.base.origin,
                placement.base.quat);
            placement.scale = 1.0;
            Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
            R_FilterXModelIntoScene(g_breakablePieces[i].model, &placement, 0, &g_breakablePieces[i].lightingHandle);
        }
    }
}

void __cdecl DynEntPieces_SpawnPieces(
    int localClientNum,
    const XModelPieces *pieces,
    const float *origin,
    const float (*axis)[3],
    const float *hitPos,
    const float *hitDir)
{
    int pieceIndex; // [esp+0h] [ebp-4h]

    if (!pieces)
        MyAssertHandler(".\\DynEntity\\DynEntity_pieces.cpp", 184, 0, "%s", "pieces");
    for (pieceIndex = 0; pieceIndex < pieces->numpieces; ++pieceIndex)
        DynEntPieces_SpawnPhysicsModel(
            localClientNum,
            pieces->pieces[pieceIndex].model,
            pieces->pieces[pieceIndex].offset,
            origin,
            axis,
            hitPos,
            hitDir);
}

bool __cdecl DynEntPieces_SpawnPhysicsModel(
    int localClientNum,
    const XModel *model,
    const float *offset,
    const float *origin,
    const float (*axis)[3],
    const float *hitPos,
    const float *hitDir)
{
    bool result; // al
    float forceDir[3]; // [esp+10h] [ebp-5Ch] BYREF
    float velocity[3]; // [esp+1Ch] [ebp-50h] BYREF
    float angularVelocity[3]; // [esp+28h] [ebp-44h] BYREF
    int physObjId; // [esp+34h] [ebp-38h]
    float mins[3]; // [esp+38h] [ebp-34h] BYREF
    float quat[4]; // [esp+44h] [ebp-28h] BYREF
    float maxs[3]; // [esp+54h] [ebp-18h] BYREF
    float worldOffset[3]; // [esp+60h] [ebp-Ch] BYREF

    if (!model)
        MyAssertHandler(".\\DynEntity\\DynEntity_pieces.cpp", 131, 0, "%s", "model");
    XModelGetBounds(model, mins, maxs);
    if (maxs[0] == mins[0] || maxs[1] == mins[1] || maxs[2] == mins[2])
    {
        Com_PrintWarning(1, "Failed to spawn pieces model '%s'.  No bounds.\n", model->name);
        return 0;
    }
    else
    {
        MatrixTransformVector(offset, *(const mat3x3*)axis, worldOffset);
        Vec3Add(worldOffset, origin, worldOffset);
        AxisToQuat(axis, quat);
        velocity[0] = dynEntPieces_velocity->current.value;
        velocity[1] = dynEntPieces_velocity->current.vector[1];
        velocity[2] = dynEntPieces_velocity->current.vector[2];
        angularVelocity[0] = dynEntPieces_angularVelocity->current.value;
        angularVelocity[1] = dynEntPieces_angularVelocity->current.vector[1];
        angularVelocity[2] = dynEntPieces_angularVelocity->current.vector[2];
        velocity[2] = velocity[2] + model->physPreset->piecesUpwardVelocity;
        Sys_EnterCriticalSection(CRITSECT_PHYSICS);
        physObjId = (int)DynEntPieces_SpawnPhysObj(
            model->name,
            mins,
            maxs,
            worldOffset,
            quat,
            velocity,
            angularVelocity,
            model->physPreset);
        Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
        if (physObjId)
        {
            DynEntPieces_CalcForceDir(hitDir, model->physPreset->piecesSpreadFraction, forceDir);
            Phys_ObjBulletImpact(
                PHYS_WORLD_DYNENT,
                (dxBody *)physObjId,
                hitPos,
                forceDir,
                dynEntPieces_impactForce->current.value,
                model->physPreset->bulletForceScale);
            g_breakablePieces[numPieces].physObjId = physObjId;
            g_breakablePieces[numPieces].model = model;
            result = 1;
            g_breakablePieces[numPieces].lightingHandle = 0;
            g_breakablePieces[numPieces++].active = 1;
        }
        else
        {
            return 0;
        }
    }
    return result;
}

dxBody *__cdecl DynEntPieces_SpawnPhysObj(
    const char *modelName,
    const float *mins,
    const float *maxs,
    float *position,
    float *quat,
    float *velocity,
    float *angularVelocity,
    const PhysPreset *physPreset)
{
    dxBody *physObjId; // [esp+0h] [ebp-4h]

    if (numPieces < 100 && physPreset)
    {
        physObjId = Phys_ObjCreate(PHYS_WORLD_FX, position, quat, velocity, physPreset);
        if (physObjId)
        {
            Phys_ObjAddGeomBox(PHYS_WORLD_FX, physObjId, mins, maxs);
            Phys_ObjSetAngularVelocity(physObjId, angularVelocity);
            return physObjId;
        }
        else
        {
            Com_PrintWarning(1, "Failed to create physics object for '%s'.\n", modelName);
            return 0;
        }
    }
    else
    {
        Com_PrintWarning(1, "Failed to spawn pieces model '%s'.  It is missing physics preset.\n", modelName);
        return 0;
    }
}

void __cdecl DynEntPieces_CalcForceDir(const float *hitDir, float spreadFraction, float *forceDir)
{
    int v3; // [esp+8h] [ebp-18h]
    int v4; // [esp+Ch] [ebp-14h]
    int v5; // [esp+10h] [ebp-10h]
    float outDir[3]; // [esp+14h] [ebp-Ch] BYREF

    v5 = rand();
    outDir[0] = (double)v5 / 32767.0 + (double)v5 / 32767.0 - 1.0;
    v4 = rand();
    outDir[1] = (double)v4 / 32767.0 + (double)v4 / 32767.0 - 1.0;
    v3 = rand();
    outDir[2] = (double)v3 / 32767.0 + (double)v3 / 32767.0 - 1.0;
    Vec3Lerp(hitDir, outDir, spreadFraction, forceDir);
}

