#include <qcommon/qcommon.h>

#include "dynentity_client.h"

const dvar_t *dynEnt_active;
const dvar_t *dynEnt_bulletForce;
const dvar_t *dynEnt_explodeForce;
const dvar_t *dynEnt_explodeUpbias;
const dvar_t *dynEnt_explodeSpinScale;
const dvar_t *dynEnt_explodeMinForce;
const dvar_t *dynEnt_explodeMaxEnts;

void __cdecl DynEntCl_RegisterDvars()
{
    DvarLimits min; // [esp+4h] [ebp-10h]
    DvarLimits mina; // [esp+4h] [ebp-10h]
    DvarLimits minb; // [esp+4h] [ebp-10h]
    DvarLimits minc; // [esp+4h] [ebp-10h]
    DvarLimits mind; // [esp+4h] [ebp-10h]

    dynEnt_active = Dvar_RegisterBool("dynEnt_active", 1, 1u, "Disable/enable dynent reactions");
    min.value.max = 1000000.0;
    min.value.min = 0.0;
    dynEnt_bulletForce = Dvar_RegisterFloat("dynEnt_bulletForce", 1000.0, min, 0x80u, "Force applied from bullet hit");
    mina.value.max = 1000000.0;
    mina.value.min = 0.0;
    dynEnt_explodeForce = Dvar_RegisterFloat(
        "dynEnt_explodeForce",
        12500.0,
        mina,
        0x80u,
        "Force applied from explosion hit");
    minb.value.max = 2.0;
    minb.value.min = 0.0;
    dynEnt_explodeUpbias = Dvar_RegisterFloat(
        "dynEnt_explodeUpbias",
        0.5,
        minb,
        0x80u,
        "Upward bias applied to force directions from explosion hits");
    minc.value.max = 100.0;
    minc.value.min = 0.0;
    dynEnt_explodeSpinScale = Dvar_RegisterFloat(
        "dynEnt_explodeSpinScale",
        3.0,
        minc,
        0x80u,
        "Scale of the random offset from the center of mass for explosion forces.");
    mind.value.max = 3.4028235e38;
    mind.value.min = 0.0;
    dynEnt_explodeMinForce = Dvar_RegisterFloat(
        "dynEnt_explodeMinForce",
        40.0,
        mind,
        0x80u,
        "Force below which dynents won't even bother waking up");
    dynEnt_explodeMaxEnts = Dvar_RegisterInt(
        "dynEnt_explodeMaxEnts",
        20,
        (DvarLimits)0x100000000000LL,
        0x80u,
        "The maximum number of dynents that can be awakened by one explosion");
    DynEntPieces_RegisterDvars();
}

void __cdecl DynEntCl_InitEntities(int localClientNum)
{
    DynEntityPose *dynEntPose; // [esp+8h] [ebp-24h]
    DynEntityDrawType drawType; // [esp+Ch] [ebp-20h]
    DynEntityClient *dynEntClient; // [esp+18h] [ebp-14h]
    const DynEntityDef *dynEntDef; // [esp+1Ch] [ebp-10h]
    unsigned __int16 dynEntCount; // [esp+20h] [ebp-Ch]
    unsigned __int16 dynEntId; // [esp+24h] [ebp-8h]
    DynEntityColl *dynEntColl; // [esp+28h] [ebp-4h]

    if (localClientNum == jpeg_mem_init())
    {
        for (drawType = DYNENT_DRAW_MODEL; (unsigned int)drawType < DYNENT_DRAW_COUNT; ++drawType)
        {
            DynEnt_ClearCollWorld((DynEntityCollType)drawType);
            dynEntCount = DynEnt_GetEntityCount((DynEntityCollType)drawType);
            for (dynEntId = 0; dynEntId < (int)dynEntCount; ++dynEntId)
            {
                dynEntDef = DynEnt_GetEntityDef(dynEntId, drawType);
                dynEntPose = DynEnt_GetClientPose(dynEntId, drawType);
                dynEntClient = DynEnt_GetClientEntity(dynEntId, drawType);
                dynEntColl = DynEnt_GetEntityColl((DynEntityCollType)drawType, dynEntId);
                memcpy(dynEntPose, &dynEntDef->pose, 0x1Cu);
                dynEntClient->physObjId = 0;
                dynEntClient->flags = 2;
                dynEntClient->health = dynEntDef->health;
                if (DynEnt_GetEntityProps(dynEntDef->type)->clientOnly)
                    dynEntClient->flags |= 1u;
                dynEntColl->nextEntInSector = 0;
                dynEntColl->sector = 0;
                if (drawType)
                    DynEntCl_LinkBrush(dynEntId);
                else
                    DynEntCl_LinkModel(dynEntId);
            }
        }
    }
}

void __cdecl DynEntCl_LinkModel(unsigned __int16 dynEntId)
{
    int v1; // [esp+0h] [ebp-204h]
    int v2; // [esp+4h] [ebp-200h]
    int v3; // [esp+8h] [ebp-1FCh]
    int v4; // [esp+Ch] [ebp-1F8h]
    int v5; // [esp+10h] [ebp-1F4h]
    int v6; // [esp+14h] [ebp-1F0h]
    int v7; // [esp+18h] [ebp-1ECh]
    int v8; // [esp+1Ch] [ebp-1E8h]
    int v9; // [esp+20h] [ebp-1E4h]
    int v10; // [esp+24h] [ebp-1E0h]
    int v11; // [esp+28h] [ebp-1DCh]
    int v12; // [esp+2Ch] [ebp-1D8h]
    float v13; // [esp+90h] [ebp-174h]
    float v14; // [esp+E0h] [ebp-124h]
    float v15; // [esp+F0h] [ebp-114h]
    float v16; // [esp+134h] [ebp-D0h]
    float v17; // [esp+138h] [ebp-CCh]
    float v18; // [esp+13Ch] [ebp-C8h]
    float v19; // [esp+140h] [ebp-C4h]
    DynEntityPose *dynEntPose; // [esp+160h] [ebp-A4h]
    float modelBoundsVec3[2][3]; // [esp+164h] [ebp-A0h] BYREF
    float4 worldBoundsFloat4[2]; // [esp+17Ch] [ebp-88h]
    XModel *model; // [esp+19Ch] [ebp-68h]
    const DynEntityDef *dynEntDef; // [esp+1A0h] [ebp-64h]
    float modelAxis[3][3]; // [esp+1A4h] [ebp-60h] BYREF
    float worldBoundsVec3[2][3]; // [esp+1C8h] [ebp-3Ch] BYREF
    float4 modelBoundsFloat4[2]; // [esp+1E0h] [ebp-24h]

    dynEntDef = DynEnt_GetEntityDef(dynEntId, DYNENT_DRAW_MODEL);
    dynEntPose = DynEnt_GetClientPose(dynEntId, DYNENT_DRAW_MODEL);
    model = dynEntDef->xModel;
    if (!model)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 126, 0, "%s", "model");
    dynEntPose->radius = XModelGetRadius(model);
    XModelGetBounds(model, modelBoundsVec3[0], modelBoundsVec3[1]);
    modelBoundsFloat4[0].v[0] = modelBoundsVec3[0][0];
    modelBoundsFloat4[0].v[1] = modelBoundsVec3[0][1];
    modelBoundsFloat4[0].v[2] = modelBoundsVec3[0][2];
    modelBoundsFloat4[0].v[3] = 0.0;
    modelBoundsFloat4[1].v[0] = modelBoundsVec3[1][0];
    modelBoundsFloat4[1].v[1] = modelBoundsVec3[1][1];
    modelBoundsFloat4[1].v[2] = modelBoundsVec3[1][2];
    modelBoundsFloat4[1].v[3] = 0.0;
    UnitQuatToAxis(dynEntPose->pose.quat, modelAxis);
    v14 = 0.0;
    v13 = 0.0;
    v15 = 0.0;
    v16 = dynEntPose->pose.origin[0];
    v17 = dynEntPose->pose.origin[1];
    v18 = dynEntPose->pose.origin[2];
    v19 = 0.0;
    if (modelAxis[0][0] >= 0.0)
        v12 = 0;
    else
        v12 = -1;
    if (modelAxis[0][1] >= 0.0)
        v11 = 0;
    else
        v11 = -1;
    if (modelAxis[0][2] >= 0.0)
        v10 = 0;
    else
        v10 = -1;
    if (v14 >= 0.0)
        v9 = 0;
    else
        v9 = -1;
    if (modelAxis[1][0] >= 0.0)
        v8 = 0;
    else
        v8 = -1;
    if (modelAxis[1][1] >= 0.0)
        v7 = 0;
    else
        v7 = -1;
    if (modelAxis[1][2] >= 0.0)
        v6 = 0;
    else
        v6 = -1;
    if (v13 >= 0.0)
        v5 = 0;
    else
        v5 = -1;
    if (modelAxis[2][0] >= 0.0)
        v4 = 0;
    else
        v4 = -1;
    if (modelAxis[2][1] >= 0.0)
        v3 = 0;
    else
        v3 = -1;
    if (modelAxis[2][2] >= 0.0)
        v2 = 0;
    else
        v2 = -1;
    if (v15 >= 0.0)
        v1 = 0;
    else
        v1 = -1;
    worldBoundsFloat4[0].v[0] = COERCE_FLOAT(modelBoundsFloat4[1].u[0] & v12 | modelBoundsFloat4[0].u[0] & ~v12)
        * modelAxis[0][0]
        + v16;
    worldBoundsFloat4[0].v[1] = COERCE_FLOAT(modelBoundsFloat4[1].u[0] & v11 | modelBoundsFloat4[0].u[0] & ~v11)
        * modelAxis[0][1]
        + v17;
    worldBoundsFloat4[0].v[2] = COERCE_FLOAT(modelBoundsFloat4[1].u[0] & v10 | modelBoundsFloat4[0].u[0] & ~v10)
        * modelAxis[0][2]
        + v18;
    worldBoundsFloat4[0].v[3] = COERCE_FLOAT(modelBoundsFloat4[1].u[0] & v9 | modelBoundsFloat4[0].u[0] & ~v9) * v14 + v19;
    worldBoundsFloat4[0].v[0] = COERCE_FLOAT(modelBoundsFloat4[1].u[1] & v8 | modelBoundsFloat4[0].u[1] & ~v8)
        * modelAxis[1][0]
        + worldBoundsFloat4[0].v[0];
    worldBoundsFloat4[0].v[1] = COERCE_FLOAT(modelBoundsFloat4[1].u[1] & v7 | modelBoundsFloat4[0].u[1] & ~v7)
        * modelAxis[1][1]
        + worldBoundsFloat4[0].v[1];
    worldBoundsFloat4[0].v[2] = COERCE_FLOAT(modelBoundsFloat4[1].u[1] & v6 | modelBoundsFloat4[0].u[1] & ~v6)
        * modelAxis[1][2]
        + worldBoundsFloat4[0].v[2];
    worldBoundsFloat4[0].v[3] = COERCE_FLOAT(modelBoundsFloat4[1].u[1] & v5 | modelBoundsFloat4[0].u[1] & ~v5) * v13
        + worldBoundsFloat4[0].v[3];
    worldBoundsFloat4[0].v[0] = COERCE_FLOAT(modelBoundsFloat4[1].u[2] & v4 | modelBoundsFloat4[0].u[2] & ~v4)
        * modelAxis[2][0]
        + worldBoundsFloat4[0].v[0];
    worldBoundsFloat4[0].v[1] = COERCE_FLOAT(modelBoundsFloat4[1].u[2] & v3 | modelBoundsFloat4[0].u[2] & ~v3)
        * modelAxis[2][1]
        + worldBoundsFloat4[0].v[1];
    worldBoundsFloat4[0].v[2] = COERCE_FLOAT(modelBoundsFloat4[1].u[2] & v2 | modelBoundsFloat4[0].u[2] & ~v2)
        * modelAxis[2][2]
        + worldBoundsFloat4[0].v[2];
    worldBoundsFloat4[0].v[3] = COERCE_FLOAT(modelBoundsFloat4[1].u[2] & v1 | modelBoundsFloat4[0].u[2] & ~v1) * v15
        + worldBoundsFloat4[0].v[3];
    worldBoundsFloat4[1].v[0] = COERCE_FLOAT(modelBoundsFloat4[0].u[0] & v12 | modelBoundsFloat4[1].u[0] & ~v12)
        * modelAxis[0][0]
        + v16;
    worldBoundsFloat4[1].v[1] = COERCE_FLOAT(modelBoundsFloat4[0].u[0] & v11 | modelBoundsFloat4[1].u[0] & ~v11)
        * modelAxis[0][1]
        + v17;
    worldBoundsFloat4[1].v[2] = COERCE_FLOAT(modelBoundsFloat4[0].u[0] & v10 | modelBoundsFloat4[1].u[0] & ~v10)
        * modelAxis[0][2]
        + v18;
    worldBoundsFloat4[1].v[3] = COERCE_FLOAT(modelBoundsFloat4[0].u[0] & v9 | modelBoundsFloat4[1].u[0] & ~v9) * v14 + v19;
    worldBoundsFloat4[1].v[0] = COERCE_FLOAT(modelBoundsFloat4[0].u[1] & v8 | modelBoundsFloat4[1].u[1] & ~v8)
        * modelAxis[1][0]
        + worldBoundsFloat4[1].v[0];
    worldBoundsFloat4[1].v[1] = COERCE_FLOAT(modelBoundsFloat4[0].u[1] & v7 | modelBoundsFloat4[1].u[1] & ~v7)
        * modelAxis[1][1]
        + worldBoundsFloat4[1].v[1];
    worldBoundsFloat4[1].v[2] = COERCE_FLOAT(modelBoundsFloat4[0].u[1] & v6 | modelBoundsFloat4[1].u[1] & ~v6)
        * modelAxis[1][2]
        + worldBoundsFloat4[1].v[2];
    worldBoundsFloat4[1].v[3] = COERCE_FLOAT(modelBoundsFloat4[0].u[1] & v5 | modelBoundsFloat4[1].u[1] & ~v5) * v13
        + worldBoundsFloat4[1].v[3];
    worldBoundsFloat4[1].v[0] = COERCE_FLOAT(modelBoundsFloat4[0].u[2] & v4 | modelBoundsFloat4[1].u[2] & ~v4)
        * modelAxis[2][0]
        + worldBoundsFloat4[1].v[0];
    worldBoundsFloat4[1].v[1] = COERCE_FLOAT(modelBoundsFloat4[0].u[2] & v3 | modelBoundsFloat4[1].u[2] & ~v3)
        * modelAxis[2][1]
        + worldBoundsFloat4[1].v[1];
    worldBoundsFloat4[1].v[2] = COERCE_FLOAT(modelBoundsFloat4[0].u[2] & v2 | modelBoundsFloat4[1].u[2] & ~v2)
        * modelAxis[2][2]
        + worldBoundsFloat4[1].v[2];
    worldBoundsFloat4[1].v[3] = COERCE_FLOAT(modelBoundsFloat4[0].u[2] & v1 | modelBoundsFloat4[1].u[2] & ~v1) * v15
        + worldBoundsFloat4[1].v[3];
    worldBoundsVec3[0][0] = worldBoundsFloat4[0].v[0];
    worldBoundsVec3[0][1] = worldBoundsFloat4[0].v[1];
    worldBoundsVec3[0][2] = worldBoundsFloat4[0].v[2];
    worldBoundsVec3[1][0] = worldBoundsFloat4[1].v[0];
    worldBoundsVec3[1][1] = worldBoundsFloat4[1].v[1];
    worldBoundsVec3[1][2] = worldBoundsFloat4[1].v[2];
    DynEnt_LinkEntity(DYNENT_COLL_CLIENT_FIRST, dynEntId, worldBoundsVec3[0], worldBoundsVec3[1]);
    R_LinkDynEnt(dynEntId, DYNENT_DRAW_MODEL, worldBoundsVec3[0], worldBoundsVec3[1]);
}

void __cdecl DynEntCl_LinkBrush(unsigned __int16 dynEntId)
{
    DynEntityPose *dynEntPose; // [esp+4h] [ebp-24h]
    float absMaxs[3]; // [esp+8h] [ebp-20h] BYREF
    const DynEntityDef *dynEntDef; // [esp+14h] [ebp-14h]
    GfxBrushModel *bmodel; // [esp+18h] [ebp-10h]
    float absMins[3]; // [esp+1Ch] [ebp-Ch] BYREF

    dynEntDef = DynEnt_GetEntityDef(dynEntId, DYNENT_DRAW_BRUSH);
    dynEntPose = DynEnt_GetClientPose(dynEntId, DYNENT_DRAW_BRUSH);
    if (dynEntDef->xModel)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 160, 0, "%s", "!dynEntDef->xModel");
    dynEntPose->radius = DynEntCl_UpdateBModelWorldBounds(dynEntDef, &dynEntPose->pose);
    bmodel = R_GetBrushModel(dynEntDef->brushModel);
    absMins[0] = bmodel->writable.mins[0];
    absMins[1] = bmodel->writable.mins[1];
    absMins[2] = bmodel->writable.mins[2];
    absMaxs[0] = bmodel->writable.maxs[0];
    absMaxs[1] = bmodel->writable.maxs[1];
    absMaxs[2] = bmodel->writable.maxs[2];
    DynEnt_LinkEntity(DYNENT_COLL_CLIENT_BRUSH, dynEntId, absMins, absMaxs);
    R_LinkDynEnt(dynEntId, DYNENT_DRAW_BRUSH, absMins, absMaxs);
}

double __cdecl DynEntCl_UpdateBModelWorldBounds(const DynEntityDef *dynEntDef, const GfxPlacement *pose)
{
    int v3; // [esp+4h] [ebp-1C0h]
    int v4; // [esp+8h] [ebp-1BCh]
    int v5; // [esp+Ch] [ebp-1B8h]
    int v6; // [esp+14h] [ebp-1B0h]
    int v7; // [esp+18h] [ebp-1ACh]
    int v8; // [esp+1Ch] [ebp-1A8h]
    int v9; // [esp+24h] [ebp-1A0h]
    int v10; // [esp+28h] [ebp-19Ch]
    int v11; // [esp+2Ch] [ebp-198h]
    float *maxs; // [esp+30h] [ebp-194h]
    float v13; // [esp+134h] [ebp-90h]
    float v14; // [esp+138h] [ebp-8Ch]
    float v15; // [esp+13Ch] [ebp-88h]
    float rotatedBounds; // [esp+158h] [ebp-6Ch]
    float rotatedBoundsa; // [esp+158h] [ebp-6Ch]
    float rotatedBoundsb; // [esp+158h] [ebp-6Ch]
    float rotatedBounds_4; // [esp+15Ch] [ebp-68h]
    float rotatedBounds_4a; // [esp+15Ch] [ebp-68h]
    float rotatedBounds_4b; // [esp+15Ch] [ebp-68h]
    float rotatedBounds_8; // [esp+160h] [ebp-64h]
    float rotatedBounds_8a; // [esp+160h] [ebp-64h]
    float rotatedBounds_8b; // [esp+160h] [ebp-64h]
    float rotatedBounds_16; // [esp+168h] [ebp-5Ch]
    float rotatedBounds_16a; // [esp+168h] [ebp-5Ch]
    float rotatedBounds_16b; // [esp+168h] [ebp-5Ch]
    float rotatedBounds_20; // [esp+16Ch] [ebp-58h]
    float rotatedBounds_20a; // [esp+16Ch] [ebp-58h]
    float rotatedBounds_20b; // [esp+16Ch] [ebp-58h]
    float rotatedBounds_24; // [esp+170h] [ebp-54h]
    float rotatedBounds_24a; // [esp+170h] [ebp-54h]
    float rotatedBounds_24b; // [esp+170h] [ebp-54h]
    __int64 bounds; // [esp+178h] [ebp-4Ch]
    __int64 bounds_8; // [esp+180h] [ebp-44h]
    __int64 bounds_16; // [esp+188h] [ebp-3Ch]
    __int64 bounds_24; // [esp+190h] [ebp-34h]
    float axis[3][3]; // [esp+19Ch] [ebp-28h] BYREF
    GfxBrushModel *bmodel; // [esp+1C0h] [ebp-4h]

    bmodel = R_GetBrushModel(dynEntDef->brushModel);
    bounds = *(_QWORD *)&bmodel->bounds[0][0];
    bounds_8 = *(_QWORD *)&bmodel->bounds[0][2];
    bounds_16 = *(_QWORD *)&bmodel->bounds[1][0];
    bounds_24 = *(_QWORD *)&bmodel->bounds[1][2];
    UnitQuatToAxis(pose->quat, axis);
    v13 = pose->origin[0];
    v14 = pose->origin[1];
    v15 = pose->origin[2];
    if (axis[0][0] >= 0.0)
        v11 = 0;
    else
        v11 = -1;
    if (axis[0][1] >= 0.0)
        v10 = 0;
    else
        v10 = -1;
    if (axis[0][2] >= 0.0)
        v9 = 0;
    else
        v9 = -1;
    if (axis[1][0] >= 0.0)
        v8 = 0;
    else
        v8 = -1;
    if (axis[1][1] >= 0.0)
        v7 = 0;
    else
        v7 = -1;
    if (axis[1][2] >= 0.0)
        v6 = 0;
    else
        v6 = -1;
    if (axis[2][0] >= 0.0)
        v5 = 0;
    else
        v5 = -1;
    if (axis[2][1] >= 0.0)
        v4 = 0;
    else
        v4 = -1;
    if (axis[2][2] >= 0.0)
        v3 = 0;
    else
        v3 = -1;
    rotatedBounds = COERCE_FLOAT(bounds_16 & v11 | bounds & ~v11) * axis[0][0] + v13;
    rotatedBounds_4 = COERCE_FLOAT(bounds_16 & v10 | bounds & ~v10) * axis[0][1] + v14;
    rotatedBounds_8 = COERCE_FLOAT(bounds_16 & v9 | bounds & ~v9) * axis[0][2] + v15;
    rotatedBoundsa = COERCE_FLOAT(HIDWORD(bounds_16) & v8 | HIDWORD(bounds) & ~v8) * axis[1][0] + rotatedBounds;
    rotatedBounds_4a = COERCE_FLOAT(HIDWORD(bounds_16) & v7 | HIDWORD(bounds) & ~v7) * axis[1][1] + rotatedBounds_4;
    rotatedBounds_8a = COERCE_FLOAT(HIDWORD(bounds_16) & v6 | HIDWORD(bounds) & ~v6) * axis[1][2] + rotatedBounds_8;
    rotatedBoundsb = COERCE_FLOAT(bounds_24 & v5 | bounds_8 & ~v5) * axis[2][0] + rotatedBoundsa;
    rotatedBounds_4b = COERCE_FLOAT(bounds_24 & v4 | bounds_8 & ~v4) * axis[2][1] + rotatedBounds_4a;
    rotatedBounds_8b = COERCE_FLOAT(bounds_24 & v3 | bounds_8 & ~v3) * axis[2][2] + rotatedBounds_8a;
    rotatedBounds_16 = COERCE_FLOAT(bounds & v11 | bounds_16 & ~v11) * axis[0][0] + v13;
    rotatedBounds_20 = COERCE_FLOAT(bounds & v10 | bounds_16 & ~v10) * axis[0][1] + v14;
    rotatedBounds_24 = COERCE_FLOAT(bounds & v9 | bounds_16 & ~v9) * axis[0][2] + v15;
    rotatedBounds_16a = COERCE_FLOAT(HIDWORD(bounds) & v8 | HIDWORD(bounds_16) & ~v8) * axis[1][0] + rotatedBounds_16;
    rotatedBounds_20a = COERCE_FLOAT(HIDWORD(bounds) & v7 | HIDWORD(bounds_16) & ~v7) * axis[1][1] + rotatedBounds_20;
    rotatedBounds_24a = COERCE_FLOAT(HIDWORD(bounds) & v6 | HIDWORD(bounds_16) & ~v6) * axis[1][2] + rotatedBounds_24;
    rotatedBounds_16b = COERCE_FLOAT(bounds_8 & v5 | bounds_24 & ~v5) * axis[2][0] + rotatedBounds_16a;
    rotatedBounds_20b = COERCE_FLOAT(bounds_8 & v4 | bounds_24 & ~v4) * axis[2][1] + rotatedBounds_20a;
    rotatedBounds_24b = COERCE_FLOAT(bounds_8 & v3 | bounds_24 & ~v3) * axis[2][2] + rotatedBounds_24a;
    bmodel->writable.mins[0] = rotatedBoundsb;
    bmodel->writable.mins[1] = rotatedBounds_4b;
    bmodel->writable.mins[2] = rotatedBounds_8b;
    maxs = bmodel->writable.maxs;
    bmodel->writable.maxs[0] = rotatedBounds_16b;
    maxs[1] = rotatedBounds_20b;
    maxs[2] = rotatedBounds_24b;
    return RadiusFromBounds(bmodel->bounds[0], bmodel->bounds[1]);
}

void __cdecl DynEntCl_ProcessEntities(int localClientNum)
{
    DynEntityPose *dynEntPose; // [esp+38h] [ebp-20h]
    DynEntityPose *dynEntPosea; // [esp+38h] [ebp-20h]
    float origin[3]; // [esp+3Ch] [ebp-1Ch] BYREF
    cg_s *cgameGlob; // [esp+48h] [ebp-10h]
    DynEntityClient *dynEntClient; // [esp+4Ch] [ebp-Ch]
    unsigned __int16 dynEntCount; // [esp+50h] [ebp-8h]
    unsigned __int16 dynEntId; // [esp+54h] [ebp-4h]

    if (localClientNum == jpeg_mem_init())
    {
        CL_ResetStats_f();
        //Profile_Begin(386);
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\dynentity\\../cgame_mp/cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        cgameGlob = cgArray;
        dynEntCount = DynEnt_GetEntityCount(DYNENT_COLL_CLIENT_FIRST);
        for (dynEntId = 0; dynEntId < (int)dynEntCount; ++dynEntId)
        {
            dynEntClient = DynEnt_GetClientEntity(dynEntId, DYNENT_DRAW_MODEL);
            if ((dynEntClient->flags & 1) != 0 && dynEntClient->physObjId)
            {
                dynEntPose = DynEnt_GetClientPose(dynEntId, DYNENT_DRAW_MODEL);
                Phys_ObjGetInterpolatedState(
                    PHYS_WORLD_DYNENT,
                    (dxBody *)dynEntClient->physObjId,
                    origin,
                    dynEntPose->pose.quat);
                if (Phys_ObjIsAsleep((dxBody *)dynEntClient->physObjId))
                {
                    Phys_ObjDestroy(PHYS_WORLD_DYNENT, (dxBody *)dynEntClient->physObjId);
                    dynEntClient->physObjId = 0;
                }
                if (!VecNCompareCustomEpsilon(origin, dynEntPose->pose.origin, 0.0099999998, 3))
                {
                    dynEntPose->pose.origin[0] = origin[0];
                    dynEntPose->pose.origin[1] = origin[1];
                    dynEntPose->pose.origin[2] = origin[2];
                    DynEntCl_LinkModel(dynEntId);
                }
            }
        }
        dynEntCount = DynEnt_GetEntityCount(DYNENT_COLL_CLIENT_BRUSH);
        for (dynEntId = 0; dynEntId < (int)dynEntCount; ++dynEntId)
        {
            dynEntClient = DynEnt_GetClientEntity(dynEntId, DYNENT_DRAW_BRUSH);
            if ((dynEntClient->flags & 1) != 0 && dynEntClient->physObjId)
            {
                dynEntPosea = DynEnt_GetClientPose(dynEntId, DYNENT_DRAW_BRUSH);
                Phys_ObjGetInterpolatedState(
                    PHYS_WORLD_DYNENT,
                    (dxBody *)dynEntClient->physObjId,
                    origin,
                    dynEntPosea->pose.quat);
                if (Phys_ObjIsAsleep((dxBody *)dynEntClient->physObjId))
                {
                    Phys_ObjDestroy(PHYS_WORLD_DYNENT, (dxBody *)dynEntClient->physObjId);
                    dynEntClient->physObjId = 0;
                }
                if (!VecNCompareCustomEpsilon(origin, dynEntPosea->pose.origin, 0.0099999998, 3))
                {
                    dynEntPosea->pose.origin[0] = origin[0];
                    dynEntPosea->pose.origin[1] = origin[1];
                    dynEntPosea->pose.origin[2] = origin[2];
                    DynEntCl_LinkBrush(dynEntId);
                }
            }
        }
        //Profile_Begin(387);
        Phys_RunToTime(localClientNum, PHYS_WORLD_DYNENT, (dxSpace *)cgameGlob->time);
        //Profile_EndInternal(0);
        //Profile_EndInternal(0);
    }
}

void __cdecl DynEntCl_Shutdown(int localClientNum)
{
    DynEntityClient *dynEntClient; // [esp+0h] [ebp-Ch]
    DynEntityClient *dynEntClienta; // [esp+0h] [ebp-Ch]
    unsigned __int16 dynEntCount; // [esp+4h] [ebp-8h]
    unsigned __int16 dynEntCounta; // [esp+4h] [ebp-8h]
    unsigned __int16 dynEntId; // [esp+8h] [ebp-4h]
    unsigned __int16 dynEntIda; // [esp+8h] [ebp-4h]

    if (CL_GetLocalClientActiveCount() && localClientNum == jpeg_mem_init())
    {
        dynEntCount = DynEnt_GetEntityCount(DYNENT_COLL_CLIENT_FIRST);
        for (dynEntId = 0; dynEntId < (int)dynEntCount; ++dynEntId)
        {
            dynEntClient = DynEnt_GetClientEntity(dynEntId, DYNENT_DRAW_MODEL);
            if ((dynEntClient->flags & 1) != 0)
            {
                if (dynEntClient->physObjId)
                {
                    Phys_ObjDestroy(PHYS_WORLD_DYNENT, (dxBody *)dynEntClient->physObjId);
                    dynEntClient->physObjId = 0;
                    dynEntClient->flags &= ~1u;
                }
            }
        }
        dynEntCounta = DynEnt_GetEntityCount(DYNENT_COLL_CLIENT_BRUSH);
        for (dynEntIda = 0; dynEntIda < (int)dynEntCounta; ++dynEntIda)
        {
            dynEntClienta = DynEnt_GetClientEntity(dynEntIda, DYNENT_DRAW_BRUSH);
            if ((dynEntClienta->flags & 1) != 0 && dynEntClienta->physObjId)
            {
                Phys_ObjDestroy(PHYS_WORLD_DYNENT, (dxBody *)dynEntClienta->physObjId);
                dynEntClienta->physObjId = 0;
                dynEntClienta->flags &= ~1u;
            }
        }
    }
}

void __cdecl DynEntCl_UnlinkEntity(unsigned __int16 dynEntId, DynEntityCollType drawType)
{
    DynEnt_UnlinkEntity(drawType, dynEntId);
    R_UnlinkDynEnt(dynEntId, (DynEntityDrawType)drawType);
}

void __cdecl DynEntCl_PointTrace(const pointtrace_t *clip, trace_t *results)
{
    float start[4]; // [esp+3Ch] [ebp-20h] BYREF
    float end[4]; // [esp+4Ch] [ebp-10h] BYREF

    //Profile_Begin(391);
    if (!clip)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 493, 0, "%s", "clip");
    if (!results)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 494, 0, "%s", "results");
    if (results->fraction > 1.0)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_client.cpp",
            495,
            0,
            "%s\n\t(results->fraction) = %g",
            "(results->fraction <= 1.0f)",
            results->fraction);
    if (results->fraction == 0.0)
    {
        //Profile_EndInternal(0);
    }
    else
    {
        start[0] = clip->extents.start[0];
        start[1] = clip->extents.start[1];
        start[2] = clip->extents.start[2];
        end[0] = clip->extents.end[0];
        end[1] = clip->extents.end[1];
        end[2] = clip->extents.end[2];
        start[3] = 0.0;
        end[3] = results->fraction;
        CL_ResetStats_f();
        DynEntCl_PointTrace_r(DYNENT_COLL_CLIENT_BRUSH, clip, 1u, start, end, results);
        if (results->fraction != 0.0)
        {
            CL_ResetStats_f();
            DynEntCl_PointTrace_r(DYNENT_COLL_CLIENT_FIRST, clip, 1u, start, end, results);
        }
        //Profile_EndInternal(0);
    }
}

void __cdecl DynEntCl_PointTrace_r(
    DynEntityCollType drawType,
    const pointtrace_t *clip,
    unsigned int sectorIndex,
    float *p1,
    float *p2,
    trace_t *results)
{
    float v6; // [esp+10h] [ebp-58h]
    float v7; // [esp+14h] [ebp-54h]
    DynEntityPose *dynEntPose; // [esp+18h] [ebp-50h]
    DynEntityCollSector *sector; // [esp+20h] [ebp-48h]
    unsigned __int16 listIndex; // [esp+24h] [ebp-44h]
    float t1; // [esp+28h] [ebp-40h]
    float frac; // [esp+2Ch] [ebp-3Ch]
    DynEntityClient *dynEntClient; // [esp+34h] [ebp-34h]
    const DynEntityDef *dynEntDef; // [esp+38h] [ebp-30h]
    float t2; // [esp+3Ch] [ebp-2Ch]
    int contentmask; // [esp+40h] [ebp-28h]
    DynEntityColl *dynEntColl; // [esp+44h] [ebp-24h]
    float p[4]; // [esp+48h] [ebp-20h] BYREF
    float mid[4]; // [esp+58h] [ebp-10h] BYREF

    if (!clip)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 399, 0, "%s", "clip");
    if (!p1)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 400, 0, "%s", "p1");
    if (!p2)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 401, 0, "%s", "p2");
    if (!results)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 402, 0, "%s", "results");
    if (results->fraction > 1.0)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_client.cpp",
            403,
            0,
            "%s\n\t(results->fraction) = %g",
            "(results->fraction <= 1.0f)",
            results->fraction);
    contentmask = clip->contentmask;
    p[0] = *p1;
    p[1] = p1[1];
    p[2] = p1[2];
    p[3] = p1[3];
    while (sectorIndex)
    {
        sector = DynEnt_GetCollSector(drawType, sectorIndex);
        if ((clip->contentmask & sector->contents) == 0)
            break;
        for (listIndex = sector->entListHead; listIndex; listIndex = dynEntColl->nextEntInSector)
        {
            dynEntColl = DynEnt_GetEntityColl(drawType, listIndex - 1);
            dynEntClient = DynEnt_GetClientEntity(listIndex - 1, (DynEntityDrawType)drawType);
            if ((dynEntClient->flags & 2) == 0)
                MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 421, 0, "%s", "dynEntClient->flags & DYNENT_CL_VISIBLE");
            if ((dynEntClient->flags & 1) != 0)
            {
                dynEntDef = DynEnt_GetEntityDef(listIndex - 1, (DynEntityDrawType)drawType);
                if ((contentmask & DynEnt_GetContents(dynEntDef)) != 0)
                {
                    dynEntPose = DynEnt_GetClientPose(listIndex - 1, (DynEntityDrawType)drawType);
                    if (!CM_TraceSphere(&clip->extents, dynEntPose->pose.origin, dynEntPose->radius, results->fraction))
                    {
                        if (drawType)
                        {
                            if (drawType != DYNENT_COLL_CLIENT_BRUSH)
                                MyAssertHandler(
                                    ".\\DynEntity\\DynEntity_client.cpp",
                                    442,
                                    0,
                                    "%s\n\t(drawType) = %i",
                                    "(drawType == DYNENT_DRAW_BRUSH)",
                                    drawType);
                            DynEnt_PointTraceToBrush(dynEntDef, dynEntPose, clip, results);
                        }
                        else
                        {
                            DynEnt_PointTraceToModel(dynEntDef, dynEntPose, clip, results);
                        }
                        if (results->fraction == 0.0)
                            return;
                    }
                }
            }
        }
        t1 = p[sector->tree.axis] - sector->tree.dist;
        t2 = p2[sector->tree.axis] - sector->tree.dist;
        if (t1 * t2 < 0.0)
        {
            if (p[3] >= (double)results->fraction)
                return;
            frac = t1 / (t1 - t2);
            if (frac < 0.0)
                MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 463, 0, "%s", "frac >= 0.0f");
            if (frac > 1.0)
                MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 464, 0, "%s", "frac <= 1.0f");
            mid[0] = (*p2 - p[0]) * frac + p[0];
            mid[1] = (p2[1] - p[1]) * frac + p[1];
            mid[2] = (p2[2] - p[2]) * frac + p[2];
            mid[3] = (p2[3] - p[3]) * frac + p[3];
            DynEntCl_PointTrace_r(drawType, clip, sector->tree.child[t2 >= 0.0], p, mid, results);
            if (results->fraction == 0.0)
                return;
            sectorIndex = sector->tree.child[t2 < 0.0];
            p[0] = mid[0];
            p[1] = mid[1];
            p[2] = mid[2];
            p[3] = mid[3];
        }
        else
        {
            v6 = t2 - t1;
            if (v6 < 0.0)
                v7 = p2[sector->tree.axis] - sector->tree.dist;
            else
                v7 = p[sector->tree.axis] - sector->tree.dist;
            sectorIndex = sector->tree.child[v7 < 0.0];
        }
    }
}

void __cdecl DynEntCl_ClipMoveTrace(const moveclip_t *clip, trace_t *results)
{
    float start[4]; // [esp+40h] [ebp-20h] BYREF
    float end[4]; // [esp+50h] [ebp-10h] BYREF

    //Profile_Begin(392);
    if (!clip)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 660, 0, "%s", "clip");
    if (!results)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 661, 0, "%s", "results");
    if (results->fraction > 1.0)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_client.cpp",
            662,
            0,
            "%s\n\t(results->fraction) = %g",
            "(results->fraction <= 1.0f)",
            results->fraction);
    if (results->fraction == 0.0)
    {
        //Profile_EndInternal(0);
    }
    else
    {
        start[0] = clip->extents.start[0];
        start[1] = clip->extents.start[1];
        start[2] = clip->extents.start[2];
        end[0] = clip->extents.end[0];
        end[1] = clip->extents.end[1];
        end[2] = clip->extents.end[2];
        start[3] = 0.0;
        end[3] = results->fraction;
        DynEntCl_ClipMoveTrace_r(clip, 1u, start, end, results);
        //Profile_EndInternal(0);
    }
}

void __cdecl DynEntCl_ClipMoveTrace_r(
    const moveclip_t *clip,
    unsigned int sectorIndex,
    float *p1,
    float *p2,
    trace_t *results)
{
    float v5; // [esp+8h] [ebp-8Ch]
    float v6; // [esp+Ch] [ebp-88h]
    float v7; // [esp+10h] [ebp-84h]
    float v8; // [esp+14h] [ebp-80h]
    float v9; // [esp+1Ch] [ebp-78h]
    float v10; // [esp+20h] [ebp-74h]
    float v11; // [esp+24h] [ebp-70h]
    float v12; // [esp+28h] [ebp-6Ch]
    float v13; // [esp+2Ch] [ebp-68h]
    float v14; // [esp+30h] [ebp-64h]
    DynEntityPose *dynEntPose; // [esp+34h] [ebp-60h]
    bool side; // [esp+38h] [ebp-5Ch]
    float diff; // [esp+3Ch] [ebp-58h]
    DynEntityCollSector *sector; // [esp+44h] [ebp-50h]
    unsigned __int16 listIndex; // [esp+48h] [ebp-4Ch]
    float t1; // [esp+4Ch] [ebp-48h]
    float frac; // [esp+50h] [ebp-44h]
    const DynEntityDef *dynEntDef; // [esp+58h] [ebp-3Ch]
    float offset; // [esp+5Ch] [ebp-38h]
    float t2; // [esp+60h] [ebp-34h]
    float frac2; // [esp+64h] [ebp-30h]
    float invDist; // [esp+6Ch] [ebp-28h]
    DynEntityColl *dynEntColl; // [esp+70h] [ebp-24h]
    float p[4]; // [esp+74h] [ebp-20h] BYREF
    float mid[4]; // [esp+84h] [ebp-10h] BYREF

    if (!clip)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 556, 0, "%s", "clip");
    if (!p1)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 557, 0, "%s", "p1");
    if (!p2)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 558, 0, "%s", "p2");
    if (!results)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 559, 0, "%s", "results");
    if (results->fraction > 1.0)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_client.cpp",
            560,
            0,
            "%s\n\t(results->fraction) = %g",
            "(results->fraction <= 1.0f)",
            results->fraction);
    p[0] = *p1;
    p[1] = p1[1];
    p[2] = p1[2];
    p[3] = p1[3];
    while (sectorIndex)
    {
        sector = DynEnt_GetCollSector(DYNENT_COLL_CLIENT_BRUSH, sectorIndex);
        if ((clip->contentmask & sector->contents) == 0)
            break;
        for (listIndex = sector->entListHead; listIndex; listIndex = dynEntColl->nextEntInSector)
        {
            dynEntColl = DynEnt_GetEntityColl(DYNENT_COLL_CLIENT_BRUSH, listIndex - 1);
            if ((DynEnt_GetClientEntity(listIndex - 1, DYNENT_DRAW_BRUSH)->flags & 2) == 0)
                MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 576, 0, "%s", "dynEntClient->flags & DYNENT_CL_VISIBLE");
            dynEntDef = DynEnt_GetEntityDef(listIndex - 1, DYNENT_DRAW_BRUSH);
            dynEntPose = DynEnt_GetClientPose(listIndex - 1, DYNENT_DRAW_BRUSH);
            DynEnt_ClipMoveTraceToBrush(dynEntDef, dynEntPose, clip, results);
            if (results->fraction == 0.0)
                return;
        }
        t1 = p[sector->tree.axis] - sector->tree.dist;
        t2 = p2[sector->tree.axis] - sector->tree.dist;
        offset = clip->outerSize[sector->tree.axis];
        v14 = t2 - t1;
        if (v14 < 0.0)
            v13 = p2[sector->tree.axis] - sector->tree.dist;
        else
            v13 = p[sector->tree.axis] - sector->tree.dist;
        if (offset > (double)v13)
        {
            v12 = t1 - t2;
            if (v12 < 0.0)
                v11 = p2[sector->tree.axis] - sector->tree.dist;
            else
                v11 = p[sector->tree.axis] - sector->tree.dist;
            if (v11 > -offset)
            {
                if (p[3] >= (double)results->fraction)
                    return;
                diff = t2 - t1;
                if (diff == 0.0)
                {
                    frac = 1.0;
                    frac2 = 0.0;
                    side = 0;
                }
                else
                {
                    v10 = fabs(diff);
                    if (diff < 0.0)
                        v9 = p[sector->tree.axis] - sector->tree.dist;
                    else
                        v9 = -t1;
                    invDist = 1.0 / v10;
                    frac = (v9 + offset) * invDist;
                    frac2 = (v9 - offset) * invDist;
                    side = diff >= 0.0;
                }
                if (frac < 0.0)
                    MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 625, 0, "%s", "frac >= 0.0f");
                v8 = 1.0 - frac;
                v7 = v8 < 0.0 ? 1.0 : frac;
                mid[0] = (*p2 - p[0]) * v7 + p[0];
                mid[1] = (p2[1] - p[1]) * v7 + p[1];
                mid[2] = (p2[2] - p[2]) * v7 + p[2];
                mid[3] = (p2[3] - p[3]) * v7 + p[3];
                DynEntCl_ClipMoveTrace_r(clip, sector->tree.child[side], p, mid, results);
                if (results->fraction == 0.0)
                    return;
                if (frac2 > 1.0)
                    MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 637, 0, "%s", "frac2 <= 1.0f");
                v6 = frac2 - 0.0;
                if (v6 < 0.0)
                    v5 = 0.0;
                else
                    v5 = frac2;
                p[0] = (*p2 - p[0]) * v5 + p[0];
                p[1] = (p2[1] - p[1]) * v5 + p[1];
                p[2] = (p2[2] - p[2]) * v5 + p[2];
                p[3] = (p2[3] - p[3]) * v5 + p[3];
                sectorIndex = sector->tree.child[1 - side];
            }
            else
            {
                sectorIndex = sector->tree.child[1];
            }
        }
        else
        {
            sectorIndex = sector->tree.child[0];
        }
    }
}

unsigned __int16 __cdecl DynEntCl_AreaEntities(
    DynEntityDrawType drawType,
    const float *mins,
    const float *maxs,
    int contentMask,
    unsigned __int16 dynEntMaxCount,
    unsigned __int16 *dynEntList)
{
    DynEntityAreaParms areaParms; // [esp+30h] [ebp-14h] BYREF

    //Profile_Begin(393);
    if (!mins)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 764, 0, "%s", "mins");
    if (!maxs)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 765, 0, "%s", "maxs");
    if (!dynEntMaxCount)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_client.cpp",
            766,
            0,
            "%s\n\t(dynEntMaxCount) = %i",
            "(dynEntMaxCount > 0)",
            0);
    if (!dynEntList)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 767, 0, "%s", "dynEntList");
    areaParms.mins = mins;
    areaParms.maxs = maxs;
    areaParms.contentMask = contentMask;
    areaParms.list = dynEntList;
    areaParms.maxCount = dynEntMaxCount;
    areaParms.count = 0;
    DynEntCl_AreaEntities_r((DynEntityCollType)drawType, 1u, &areaParms);
    //Profile_EndInternal(0);
    return areaParms.count;
}

void __cdecl DynEntCl_AreaEntities_r(
    DynEntityCollType drawType,
    unsigned int sectorIndex,
    DynEntityAreaParms *areaParms)
{
    DynEntityPose *dynEntPose; // [esp+0h] [ebp-20h]
    DynEntityCollSector *sector; // [esp+4h] [ebp-1Ch]
    unsigned __int16 listIndex; // [esp+8h] [ebp-18h]
    DynEntityClient *dynEntClient; // [esp+10h] [ebp-10h]
    const DynEntityDef *dynEntDef; // [esp+14h] [ebp-Ch]
    DynEntityColl *dynEntColl; // [esp+18h] [ebp-8h]
    unsigned int nextSectorIndex; // [esp+1Ch] [ebp-4h]

    if (!areaParms)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 695, 0, "%s", "areaParms");
    while (sectorIndex)
    {
        sector = DynEnt_GetCollSector(drawType, sectorIndex);
        if ((areaParms->contentMask & sector->contents) == 0)
            break;
        for (listIndex = sector->entListHead; listIndex; listIndex = dynEntColl->nextEntInSector)
        {
            dynEntColl = DynEnt_GetEntityColl(drawType, listIndex - 1);
            dynEntClient = DynEnt_GetClientEntity(listIndex - 1, (DynEntityDrawType)drawType);
            if ((dynEntClient->flags & 2) == 0)
                MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 710, 0, "%s", "dynEntClient->flags & DYNENT_CL_VISIBLE");
            if ((dynEntClient->flags & 1) != 0)
            {
                dynEntDef = DynEnt_GetEntityDef(listIndex - 1, (DynEntityDrawType)drawType);
                dynEntPose = DynEnt_GetClientPose(listIndex - 1, (DynEntityDrawType)drawType);
                if (DynEnt_EntityInArea(dynEntDef, dynEntPose, areaParms->mins, areaParms->maxs, areaParms->contentMask))
                {
                    if (areaParms->count == areaParms->maxCount)
                    {
                        Com_PrintWarning(16, "DynEntCl_AreaEntities_r: Hit max count!\n");
                        return;
                    }
                    areaParms->list[areaParms->count++] = listIndex - 1;
                }
            }
        }
        if (sector->tree.dist >= (double)areaParms->maxs[sector->tree.axis])
        {
            if (sector->tree.dist <= (double)areaParms->mins[sector->tree.axis])
                return;
            sectorIndex = sector->tree.child[1];
        }
        else if (sector->tree.dist <= (double)areaParms->mins[sector->tree.axis])
        {
            sectorIndex = sector->tree.child[0];
        }
        else
        {
            nextSectorIndex = sector->tree.child[1];
            DynEntCl_AreaEntities_r((DynEntityDrawType)drawType, sector->tree.child[0], areaParms);
            sectorIndex = nextSectorIndex;
        }
    }
}

void __cdecl DynEntCl_EntityImpactEvent(
    const trace_t *trace,
    int localClientNum,
    int sourceEntityNum,
    const float *start,
    const float *hitPos,
    bool isMelee)
{
    float hitDir[3]; // [esp+14h] [ebp-18h] BYREF
    DObj_s *obj; // [esp+20h] [ebp-Ch]
    PhysPreset *physPreset; // [esp+24h] [ebp-8h]
    centity_s *cent; // [esp+28h] [ebp-4h]

    if (trace->hitType == TRACE_HITTYPE_ENTITY && DynEntCl_EventNeedsProcessed(localClientNum, sourceEntityNum))
    {
        cent = CG_GetEntity(localClientNum, trace->hitId);
        if (!cent)
            MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 996, 0, "%s", "cent");
        if (cent->pose.physObjId != -1 && cent->pose.physObjId)
        {
            Vec3Sub(hitPos, start, hitDir);
            Vec3Normalize(hitDir);
            if (isMelee)
                CG_PlaySoundAlias(localClientNum, 1022, hitPos, cgMedia.meleeKnifeHitOther);
            else
                DynEntCl_PlayImpactEffects(
                    localClientNum,
                    sourceEntityNum,
                    (trace->surfaceFlags & 0x1F00000) >> 20,
                    hitPos,
                    trace->normal);
            obj = Com_GetClientDObj(cent->nextState.number, localClientNum);
            if (!obj)
                MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1010, 0, "%s", "obj");
            physPreset = DObjGetPhysPreset(obj);
            if (!physPreset)
                MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1013, 0, "%s", "physPreset");
            Phys_ObjBulletImpact(
                PHYS_WORLD_DYNENT,
                (dxBody *)cent->pose.physObjId,
                hitPos,
                hitDir,
                dynEnt_bulletForce->current.value,
                physPreset->bulletForceScale);
        }
    }
}

void __cdecl DynEntCl_PlayImpactEffects(
    int localClientNum,
    unsigned int sourceEntityNum,
    unsigned int surfType,
    const float *hitPos,
    const float *hitNormal)
{
    float axis[3][3]; // [esp+4h] [ebp-34h] BYREF
    const centity_s *attacker; // [esp+28h] [ebp-10h]
    snd_alias_list_t *hitSound; // [esp+2Ch] [ebp-Ch]
    const WeaponDef *weaponDef; // [esp+30h] [ebp-8h]
    const FxEffectDef *hitFx; // [esp+34h] [ebp-4h]

    if (surfType >= 0x1D)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_client.cpp",
            845,
            0,
            "surfType doesn't index SURF_TYPECOUNT\n\t%i not in [0, %i)",
            surfType,
            29);
    if (!hitPos)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 846, 0, "%s", "hitPos");
    if (!hitNormal)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 847, 0, "%s", "hitNormal");
    attacker = CG_GetEntity(localClientNum, sourceEntityNum);
    weaponDef = BG_GetWeaponDef(attacker->nextState.weapon);
    if (!weaponDef)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 852, 0, "%s", "weaponDef");
    hitFx = 0;
    hitSound = 0;
    switch (weaponDef->impactType)
    {
    case IMPACT_TYPE_BULLET_SMALL:
        hitFx = cgMedia.fx->table->nonflesh[surfType];
        hitSound = cgMedia.bulletHitSmallSound[surfType];
        break;
    case IMPACT_TYPE_BULLET_LARGE:
        hitFx = cgMedia.fx->table[2].nonflesh[surfType];
        hitSound = cgMedia.bulletHitLargeSound[surfType];
        break;
    case IMPACT_TYPE_BULLET_AP:
        hitFx = cgMedia.fx->table[6].nonflesh[surfType];
        hitSound = cgMedia.bulletHitAPSound[surfType];
        break;
    case IMPACT_TYPE_SHOTGUN:
        hitFx = cgMedia.fx->table[4].nonflesh[surfType];
        hitSound = cgMedia.shotgunHitSound[surfType];
        break;
    default:
        break;
    }
    if (hitFx && (*hitNormal != 0.0 || hitNormal[1] != 0.0 || hitNormal[2] != 0.0))
    {
        axis[0][0] = *hitNormal;
        axis[0][1] = hitNormal[1];
        axis[0][2] = hitNormal[2];
        CG_RandomEffectAxis(axis[0], axis[1], axis[2]);
        DynEntCl_PlayEventFx(hitFx, hitPos, axis);
    }
    if (hitSound)
        CG_PlaySoundAlias(localClientNum, 1022, hitPos, hitSound);
}

void __cdecl DynEntCl_PlayEventFx(const FxEffectDef *def, const float *origin, const float (*axis)[3])
{
    int clientIndex; // [esp+4h] [ebp-4h]

    for (clientIndex = 0; clientIndex < 1; ++clientIndex)
    {
        if (CL_IsLocalClientActive(clientIndex))
        {
            if (clientIndex)
                MyAssertHandler(
                    "c:\\trees\\cod3\\src\\dynentity\\../cgame_mp/cg_local_mp.h",
                    1071,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum == 0)",
                    clientIndex);
            FX_PlayOrientedEffect(clientIndex, def, time, origin, axis);
        }
    }
}

char __cdecl DynEntCl_EventNeedsProcessed(int localClientNum, int sourceEntityNum)
{
    if (!dynEnt_active->current.enabled)
        return 0;
    if (CG_GetLocalClientGlobalsForEnt(localClientNum, sourceEntityNum))
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\dynentity\\../cgame_mp/cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        if ((*(unsigned int *)(dword_98F45C + 32) & 6) == 0 || sourceEntityNum != *(unsigned int *)(dword_98F45C + 232))
            return 0;
    }
    else if (localClientNum != jpeg_mem_init())
    {
        return 0;
    }
    return 1;
}

char __cdecl DynEntCl_DynEntImpactEvent(
    int localClientNum,
    int sourceEntityNum,
    float *start,
    float *end,
    int damage,
    bool isMelee)
{
    dxBody *PhysObj; // eax
    DynEntityPose *dynEntPose; // [esp+14h] [ebp-8Ch]
    pointtrace_t clip; // [esp+18h] [ebp-88h] BYREF
    float hitDir[3]; // [esp+4Ch] [ebp-54h] BYREF
    DynEntityDrawType drawType; // [esp+58h] [ebp-48h] BYREF
    DynEntityClient *dynEntClient; // [esp+5Ch] [ebp-44h]
    const DynEntityDef *dynEntDef; // [esp+60h] [ebp-40h]
    trace_t trace; // [esp+64h] [ebp-3Ch] BYREF
    float hitPos[3]; // [esp+90h] [ebp-10h] BYREF
    unsigned __int16 dynEntId; // [esp+9Ch] [ebp-4h]

    if (!start)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1058, 0, "%s", "start");
    if (!end)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1059, 0, "%s", "end");
    if (damage < 0)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1060, 0, "%s\n\t(damage) = %i", "((damage >= 0))", damage);
    if (!DynEntCl_EventNeedsProcessed(localClientNum, sourceEntityNum))
        return 0;
    if (!sv_clientSideBullets->current.enabled)
        DynEntCl_TestPhysicsEntities(localClientNum, sourceEntityNum, start, end, isMelee);
    memset((unsigned __int8 *)&trace, 0, sizeof(trace));
    trace.fraction = 1.0;
    clip.extents.start[0] = *start;
    clip.extents.start[1] = start[1];
    clip.extents.start[2] = start[2];
    clip.extents.end[0] = *end;
    clip.extents.end[1] = end[1];
    clip.extents.end[2] = end[2];
    CM_CalcTraceExtents(&clip.extents);
    clip.ignoreEntParams = 0;
    clip.contentmask = (int)&loc_802012 + 1;
    clip.bLocational = 1;
    clip.priorityMap = 0;
    DynEntCl_PointTrace(&clip, &trace);
    dynEntId = Trace_GetDynEntHitId(&trace, &drawType);
    if (dynEntId == 0xFFFF)
        return 0;
    dynEntDef = DynEnt_GetEntityDef(dynEntId, drawType);
    dynEntClient = DynEnt_GetClientEntity(dynEntId, drawType);
    if ((dynEntClient->flags & 1) == 0)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1090, 0, "%s", "dynEntClient->flags & DYNENT_CL_ACTIVE");
    Vec3Lerp(start, end, trace.fraction, hitPos);
    Vec3Sub(end, start, hitDir);
    Vec3Normalize(hitDir);
    if (isMelee)
        CG_PlaySoundAlias(localClientNum, 1022, hitPos, cgMedia.meleeKnifeHitOther);
    else
        DynEntCl_PlayImpactEffects(
            localClientNum,
            sourceEntityNum,
            (trace.surfaceFlags & 0x1F00000) >> 20,
            hitPos,
            trace.normal);
    if (DynEnt_GetEntityProps(dynEntDef->type)->usePhysics)
    {
        dynEntPose = DynEnt_GetClientPose(dynEntId, drawType);
        if (!dynEntClient->physObjId)
        {
            PhysObj = DynEntCl_CreatePhysObj(dynEntDef, &dynEntPose->pose);
            dynEntClient->physObjId = (int)PhysObj;
        }
        if (dynEntClient->physObjId)
            Phys_ObjBulletImpact(
                PHYS_WORLD_DYNENT,
                (dxBody *)dynEntClient->physObjId,
                hitPos,
                hitDir,
                dynEnt_bulletForce->current.value,
                dynEntDef->physPreset->bulletForceScale);
    }
    if (DynEnt_GetEntityProps(dynEntDef->type)->destroyable)
    {
        if (damage)
            DynEntCl_Damage(localClientNum, dynEntId, (DynEntityCollType)drawType, hitPos, hitDir, damage);
    }
    return 1;
}

dxBody *__cdecl DynEntCl_CreatePhysObj(const DynEntityDef *dynEntDef, const GfxPlacement *pose)
{
    dxBody *physId; // [esp+0h] [ebp-4h]

    if (!dynEntDef)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 793, 0, "%s", "dynEntDef");
    if (!pose)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 794, 0, "%s", "pose");
    if (!DynEnt_GetEntityProps(dynEntDef->type)->usePhysics)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_client.cpp",
            795,
            0,
            "%s",
            "DynEnt_GetEntityProps( dynEntDef->type )->usePhysics");
    if (!dynEntDef->physPreset)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 796, 0, "%s", "dynEntDef->physPreset");
    if (!dynEnt_active->current.enabled)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 799, 0, "%s", "dynEnt_active->current.enabled");
    physId = Phys_ObjCreate(PHYS_WORLD_DYNENT, pose->origin, pose->quat, (float *)vec3_origin, dynEntDef->physPreset);
    if (physId)
    {
        DynEnt_SetPhysObjCollision(dynEntDef, physId);
        return physId;
    }
    else
    {
        Com_PrintWarning(1, "DynEntCl_CreatePhysObj: Unable to create physic object.");
        return 0;
    }
}

void __cdecl DynEntCl_Damage(
    int localClientNum,
    unsigned __int16 dynEntId,
    DynEntityCollType drawType,
    const float *hitPos,
    const float *hitDir,
    int damage)
{
    DynEntityPose *dynEntPose; // [esp+0h] [ebp-34h]
    DynEntityClient *dynEntClient; // [esp+8h] [ebp-2Ch]
    const DynEntityDef *dynEntDef; // [esp+Ch] [ebp-28h]
    float axis[3][3]; // [esp+10h] [ebp-24h] BYREF

    if (dynEntId == 0xFFFF)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 906, 0, "%s", "dynEntId != DYNENT_INVALID_ID");
    if (!hitPos)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 907, 0, "%s", "hitPos");
    if (!hitDir)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 908, 0, "%s", "hitDir");
    if (damage <= 0)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 909, 0, "%s\n\t(damage) = %i", "((damage > 0))", damage);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\dynentity\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    dynEntDef = DynEnt_GetEntityDef(dynEntId, (DynEntityDrawType)drawType);
    dynEntClient = DynEnt_GetClientEntity(dynEntId, (DynEntityDrawType)drawType);
    if (!DynEnt_GetEntityProps(dynEntDef->type)->destroyable)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_client.cpp",
            916,
            0,
            "%s",
            "DynEnt_GetEntityProps( dynEntDef->type )->destroyable");
    if ((dynEntClient->flags & 3) == 0)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_client.cpp",
            917,
            0,
            "%s",
            "dynEntClient->flags & (DYNENT_CL_ACTIVE | DYNENT_CL_VISIBLE)");
    if (dynEntClient->health <= 0)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 919, 0, "%s", "dynEntClient->health > 0");
    dynEntClient->health -= damage;
    if (dynEntClient->health <= 0)
    {
        dynEntClient->flags &= 0xFFFCu;
        if (dynEntClient->physObjId)
        {
            Phys_ObjDestroy(PHYS_WORLD_DYNENT, (dxBody *)dynEntClient->physObjId);
            dynEntClient->physObjId = 0;
        }
        DynEntCl_UnlinkEntity(dynEntId, drawType);
        if (dynEntDef->destroyFx || dynEntDef->destroyPieces)
        {
            dynEntPose = DynEnt_GetClientPose(dynEntId, (DynEntityDrawType)drawType);
            UnitQuatToAxis(dynEntPose->pose.quat, axis);
            if (dynEntDef->destroyFx)
                DynEntCl_PlayEventFx(dynEntDef->destroyFx, dynEntPose->pose.origin, axis);
            if (dynEntDef->destroyPieces)
                DynEntPieces_SpawnPieces(
                    localClientNum,
                    dynEntDef->destroyPieces,
                    dynEntPose->pose.origin,
                    axis,
                    hitPos,
                    hitDir);
        }
    }
}

void __cdecl DynEntCl_TestPhysicsEntities(
    int localClientNum,
    int sourceEntityNum,
    float *start,
    float *end,
    bool isMelee)
{
    trace_t trace; // [esp+8h] [ebp-38h] BYREF
    float hitPos[3]; // [esp+34h] [ebp-Ch] BYREF

    memset((unsigned __int8 *)&trace, 0, sizeof(trace));
    trace.fraction = 1.0;
    CG_LocationalTraceEntitiesOnly(&trace, start, end, sourceEntityNum, (int)&sv.svEntities[342].clusternums[2] + 1);
    if (trace.hitType)
    {
        Vec3Lerp(start, end, trace.fraction, hitPos);
        DynEntCl_EntityImpactEvent(&trace, localClientNum, sourceEntityNum, start, hitPos, isMelee);
    }
}

void __cdecl DynEntCl_MeleeEvent(int localClientNum, int sourceEntityNum)
{
    float v2; // [esp+Ch] [ebp-58h]
    float scale; // [esp+10h] [ebp-54h]
    centity_s *attacker; // [esp+14h] [ebp-50h]
    int damage; // [esp+18h] [ebp-4Ch]
    float right[3]; // [esp+20h] [ebp-44h] BYREF
    float end[3]; // [esp+2Ch] [ebp-38h] BYREF
    float forward[3]; // [esp+38h] [ebp-2Ch] BYREF
    float up[3]; // [esp+44h] [ebp-20h] BYREF
    float eyePos[3]; // [esp+50h] [ebp-14h] BYREF
    unsigned int traceIndex; // [esp+5Ch] [ebp-8h]
    const WeaponDef *weapDef; // [esp+60h] [ebp-4h]

    if (DynEntCl_EventNeedsProcessed(localClientNum, sourceEntityNum))
    {
        attacker = CG_GetEntity(localClientNum, sourceEntityNum);
        if (attacker->nextState.weapon)
        {
            weapDef = BG_GetWeaponDef(attacker->nextState.weapon);
            damage = weapDef->iMeleeDamage;
            CG_CalcEyePoint(localClientNum, sourceEntityNum, eyePos);
            CG_GetViewDirection(localClientNum, sourceEntityNum, forward, right, up);
            for (traceIndex = 0; traceIndex < 5; ++traceIndex)
            {
                if (!player_meleeRange)
                    MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1154, 0, "%s", "player_meleeRange");
                if (!player_meleeWidth)
                    MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1155, 0, "%s", "player_meleeWidth");
                if (!player_meleeHeight)
                    MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1156, 0, "%s", "player_meleeHeight");
                Vec3Mad(eyePos, player_meleeRange->current.value, forward, end);
                scale = player_meleeWidth->current.value * (float)traceOffsets[traceIndex][0];
                Vec3Mad(end, scale, right, end);
                v2 = player_meleeHeight->current.value * flt_89265C[2 * traceIndex];
                Vec3Mad(end, v2, up, end);
                if (DynEntCl_DynEntImpactEvent(localClientNum, attacker->nextState.number, eyePos, end, damage, 1))
                    break;
            }
        }
    }
}

void __cdecl DynEntCl_ExplosionEvent(
    int localClientNum,
    bool isCylinder,
    float *origin,
    float innerRadius,
    float outerRadius,
    float *impulse,
    float inScale,
    int innerDamage,
    int outerDamage)
{
    dxBody *PhysObj; // eax
    double v10; // st7
    double v11; // st7
    double v12; // st7
    float v14; // [esp+10h] [ebp-209Ch]
    float v15; // [esp+14h] [ebp-2098h]
    float s; // [esp+18h] [ebp-2094h]
    int damage; // [esp+34h] [ebp-2078h]
    DynEntityPose *dynEntPose; // [esp+38h] [ebp-2074h]
    float CylindricalRadiusDistSqr; // [esp+3Ch] [ebp-2070h]
    unsigned int ClosestEntities; // [esp+40h] [ebp-206Ch]
    float radiusMaxs[3]; // [esp+44h] [ebp-2068h] BYREF
    float diff[2]; // [esp+50h] [ebp-205Ch] BYREF
    float v23; // [esp+58h] [ebp-2054h]
    DynEntityDrawType drawType; // [esp+5Ch] [ebp-2050h]
    DynEntityClient *ClientEntity; // [esp+60h] [ebp-204Ch]
    float result[3]; // [esp+64h] [ebp-2048h] BYREF
    DynEntityDef *dynEntDef; // [esp+70h] [ebp-203Ch]
    float v28; // [esp+74h] [ebp-2038h]
    float outPosition; // [esp+78h] [ebp-2034h] BYREF
    float v30; // [esp+7Ch] [ebp-2030h]
    float v31; // [esp+80h] [ebp-202Ch]
    unsigned __int16 hitEnts[4098]; // [esp+84h] [ebp-2028h] BYREF
    float v33; // [esp+208Ch] [ebp-20h]
    unsigned int i; // [esp+2090h] [ebp-1Ch]
    float sum[3]; // [esp+2094h] [ebp-18h] BYREF
    float v36; // [esp+20A0h] [ebp-Ch]
    float v37; // [esp+20A4h] [ebp-8h]
    float v38; // [esp+20A8h] [ebp-4h]

    if (!origin)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1247, 0, "%s", "origin");
    if (innerRadius < 0.0)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1248, 0, "%s", "innerRadius >= 0.0f");
    if (innerRadius > (double)outerRadius)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1249, 0, "%s", "outerRadius >= innerRadius");
    if (innerDamage < 0)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_client.cpp",
            1250,
            0,
            "%s\n\t(innerDamage) = %i",
            "((innerDamage >= 0))",
            innerDamage);
    if (outerDamage < 0)
        MyAssertHandler(
            ".\\DynEntity\\DynEntity_client.cpp",
            1251,
            0,
            "%s\n\t(outerDamage) = %i",
            "((outerDamage >= 0))",
            outerDamage);
    if (DynEntCl_EventNeedsProcessed(localClientNum, 1023) && outerRadius != 0.0)
    {
        v38 = outerRadius * outerRadius;
        v36 = innerRadius * innerRadius;
        v28 = 0.0;
        if (v36 < (double)v38)
            v28 = 1.0 / (innerRadius - outerRadius);
        s = -(outerRadius * 1.414213538169861);
        Vec3AddScalar(origin, s, sum);
        v15 = outerRadius * 1.414213538169861;
        Vec3AddScalar(origin, v15, radiusMaxs);
        if (isCylinder)
        {
            sum[2] = -3.4028235e38;
            radiusMaxs[2] = 3.4028235e38;
        }
        drawType = DYNENT_DRAW_MODEL;
    LABEL_18:
        if ((unsigned int)drawType < DYNENT_DRAW_COUNT)
        {
            ClosestEntities = DynEntCl_GetClosestEntities(drawType, sum, radiusMaxs, origin, hitEnts, isCylinder);
            for (i = 0; ; ++i)
            {
                if (i >= ClosestEntities)
                {
                    ++drawType;
                    goto LABEL_18;
                }
                ClientEntity = DynEnt_GetClientEntity(hitEnts[i], drawType);
                if ((ClientEntity->flags & 1) == 0)
                    MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1283, 0, "%s", "dynEntClient->flags & DYNENT_CL_ACTIVE");
                dynEntPose = DynEnt_GetClientPose(hitEnts[i], drawType);
                if (isCylinder)
                    CylindricalRadiusDistSqr = DynEnt_GetCylindricalRadiusDistSqr(dynEntPose, origin);
                else
                    CylindricalRadiusDistSqr = DynEnt_GetRadiusDistSqr(dynEntPose, origin);
                if (v38 > (double)CylindricalRadiusDistSqr)
                {
                    v37 = inScale;
                    if (v36 < (double)CylindricalRadiusDistSqr)
                    {
                        if (v36 >= (double)v38)
                            MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1298, 1, "%s", "outerRadiusSqr > innerRadiusSqr");
                        v14 = sqrt(CylindricalRadiusDistSqr);
                        v37 = (v14 - outerRadius) * v28 * v37;
                    }
                    if (v37 < 0.0)
                        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1301, 1, "%s", "scale >= 0");
                    dynEntDef = (DynEntityDef *)DynEnt_GetEntityDef(hitEnts[i], drawType);
                    v33 = v37 * dynEntDef->physPreset->explosiveForceScale * dynEnt_explodeForce->current.value;
                    if (*impulse == 0.0 && impulse[1] == 0.0 && impulse[2] == 0.0)
                    {
                        if (dynEnt_explodeMinForce->current.value > (double)v33)
                            continue;
                        Vec3Sub(dynEntPose->pose.origin, origin, diff);
                        if (isCylinder)
                            v23 = 0.0;
                        Vec3Normalize(diff);
                        v23 = v23 + dynEnt_explodeUpbias->current.value;
                        Vec3Normalize(diff);
                    }
                    else
                    {
                        diff[0] = *impulse;
                        diff[1] = impulse[1];
                        v23 = impulse[2];
                    }
                    Vec3Scale(diff, v33, result);
                    if (DynEnt_GetEntityProps(dynEntDef->type)->usePhysics)
                    {
                        if (!ClientEntity->physObjId)
                        {
                            PhysObj = DynEntCl_CreatePhysObj(dynEntDef, &dynEntPose->pose);
                            ClientEntity->physObjId = (int)PhysObj;
                        }
                        if (ClientEntity->physObjId)
                        {
                            Phys_ObjGetCenterOfMass((dxBody *)ClientEntity->physObjId, &outPosition);
                            v10 = flrand(-1.0, 1.0);
                            outPosition = v10 * dynEnt_explodeSpinScale->current.value + outPosition;
                            v11 = flrand(-1.0, 1.0);
                            v30 = v11 * dynEnt_explodeSpinScale->current.value + v30;
                            v12 = flrand(-1.0, 1.0);
                            v31 = v12 * dynEnt_explodeSpinScale->current.value + v31;
                            Phys_ObjAddForce(PHYS_WORLD_DYNENT, (dxBody *)ClientEntity->physObjId, &outPosition, result);
                        }
                    }
                    if (DynEnt_GetEntityProps(dynEntDef->type)->destroyable)
                    {
                        damage = (int)((double)(innerDamage - outerDamage) * v37 + (double)outerDamage);
                        if (damage)
                            DynEntCl_Damage(localClientNum, hitEnts[i], (DynEntityCollType)drawType, &outPosition, diff, damage);
                    }
                }
            }
        }
    }
}

void __cdecl Vec3AddScalar(const float *a, float s, float *sum)
{
    *sum = *a + s;
    sum[1] = a[1] + s;
    sum[2] = a[2] + s;
}

unsigned int __cdecl DynEntCl_GetClosestEntities(
    DynEntityDrawType drawType,
    float *radiusMins,
    float *radiusMaxs,
    float *origin,
    unsigned __int16 *hitEnts,
    bool isCylinder)
{
    double CylindricalRadiusDistSqr; // st7
    DynEntityPose *dynEntPose; // [esp+12Ch] [ebp-8010h]
    unsigned int unsignedInt_low; // [esp+130h] [ebp-800Ch]
    ShadowCandidate v10[4096]; // [esp+134h] [ebp-8008h] BYREF
    DynEntityClient *ClientEntity; // [esp+8134h] [ebp-8h]
    unsigned int i; // [esp+8138h] [ebp-4h]

    unsignedInt_low = DynEntCl_AreaEntities(drawType, radiusMins, radiusMaxs, (int)&loc_802012 + 1, 0x1000u, hitEnts);
    if (unsignedInt_low > dynEnt_explodeMaxEnts->current.integer)
    {
        for (i = 0; i < unsignedInt_low; ++i)
        {
            LOWORD(v10[i].weight) = hitEnts[i];
            ClientEntity = DynEnt_GetClientEntity(hitEnts[i], drawType);
            if ((ClientEntity->flags & 1) == 0)
                MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1201, 0, "%s", "dynEntClient->flags & DYNENT_CL_ACTIVE");
            dynEntPose = DynEnt_GetClientPose(hitEnts[i], drawType);
            if (isCylinder)
                CylindricalRadiusDistSqr = DynEnt_GetCylindricalRadiusDistSqr(dynEntPose, origin);
            else
                CylindricalRadiusDistSqr = DynEnt_GetRadiusDistSqr(dynEntPose, origin);
            *(float *)&v10[i].sceneEntIndex = CylindricalRadiusDistSqr;
        }
        std::_Sort<ShadowCandidate *, int, bool(__cdecl *)(ShadowCandidate const &, ShadowCandidate const &)>(
            v10,
            &v10[unsignedInt_low],
            (int)(8 * unsignedInt_low) >> 3,
            (bool(__cdecl *)(const ShadowCandidate *, const ShadowCandidate *))DynEntCl_CompareDynEntsForExplosion);
        unsignedInt_low = LOWORD(dynEnt_explodeMaxEnts->current.unsignedInt);
        if (unsignedInt_low != dynEnt_explodeMaxEnts->current.integer)
            MyAssertHandler(
                ".\\DynEntity\\DynEntity_client.cpp",
                1213,
                0,
                "%s",
                "hitCount == (uint)dynEnt_explodeMaxEnts->current.integer");
        for (i = 0; i < unsignedInt_low; ++i)
            hitEnts[i] = LOWORD(v10[i].weight);
    }
    return unsignedInt_low;
}

bool __cdecl DynEntCl_CompareDynEntsForExplosion(const DynEntSortStruct *ent1, const DynEntSortStruct *ent2)
{
    return ent2->distSq > (double)ent1->distSq;
}

void __cdecl DynEntCl_JitterEvent(
    int localClientNum,
    float *origin,
    float innerRadius,
    float outerRadius,
    float minDisplacement,
    float maxDisplacement)
{
    double CylindricalRadiusDistSqr; // st7
    dxBody *PhysObj; // eax
    float v8; // [esp+10h] [ebp-A170h]
    float s; // [esp+14h] [ebp-A16Ch]
    DynEntityPose *dynEntPose; // [esp+148h] [ebp-A038h]
    DynEntityPose *dynEntPosea; // [esp+148h] [ebp-A038h]
    unsigned __int16 unsignedInt; // [esp+14Ch] [ebp-A034h]
    float maxs[3]; // [esp+150h] [ebp-A030h] BYREF
    DynEntityDrawType drawType; // [esp+15Ch] [ebp-A024h]
    ShadowCandidate v15[4096]; // [esp+160h] [ebp-A020h] BYREF
    DynEntityClient *ClientEntity; // [esp+8160h] [ebp-2020h]
    DynEntityDef *dynEntDef; // [esp+8164h] [ebp-201Ch]
    unsigned __int16 dynEntList[4098]; // [esp+8168h] [ebp-2018h] BYREF
    unsigned __int16 i; // [esp+A170h] [ebp-10h]
    float sum[3]; // [esp+A174h] [ebp-Ch] BYREF

    if (!origin)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1372, 0, "%s", "origin");
    if (innerRadius < 0.0)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1373, 0, "%s", "innerRadius >= 0.0f");
    if (innerRadius > (double)outerRadius)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1374, 0, "%s", "outerRadius >= innerRadius");
    if (DynEntCl_EventNeedsProcessed(localClientNum, 1023) && outerRadius != 0.0)
    {
        s = -(outerRadius * 1.414213538169861);
        Vec3AddScalar(origin, s, sum);
        v8 = outerRadius * 1.414213538169861;
        Vec3AddScalar(origin, v8, maxs);
        sum[2] = -3.4028235e38;
        maxs[2] = 3.4028235e38;
        for (drawType = DYNENT_DRAW_MODEL; (unsigned int)drawType < DYNENT_DRAW_COUNT; ++drawType)
        {
            unsignedInt = DynEntCl_AreaEntities(drawType, sum, maxs, (int)&loc_802012 + 1, 0x1000u, dynEntList);
            if (unsignedInt > dynEnt_explodeMaxEnts->current.integer)
            {
                for (i = 0; i < (int)unsignedInt; ++i)
                {
                    LOWORD(v15[i].weight) = dynEntList[i];
                    ClientEntity = DynEnt_GetClientEntity(dynEntList[i], drawType);
                    if ((ClientEntity->flags & 1) == 0)
                        MyAssertHandler(
                            ".\\DynEntity\\DynEntity_client.cpp",
                            1401,
                            0,
                            "%s",
                            "dynEntClient->flags & DYNENT_CL_ACTIVE");
                    dynEntPose = DynEnt_GetClientPose(dynEntList[i], drawType);
                    CylindricalRadiusDistSqr = DynEnt_GetCylindricalRadiusDistSqr(dynEntPose, origin);
                    *(float *)&v15[i].sceneEntIndex = CylindricalRadiusDistSqr;
                }
                std::_Sort<ShadowCandidate *, int, bool(__cdecl *)(ShadowCandidate const &, ShadowCandidate const &)>(
                    v15,
                    &v15[unsignedInt],
                    (8 * unsignedInt) >> 3,
                    (bool(__cdecl *)(const ShadowCandidate *, const ShadowCandidate *))DynEntCl_CompareDynEntsForExplosion);
                unsignedInt = dynEnt_explodeMaxEnts->current.unsignedInt;
                if (unsignedInt != dynEnt_explodeMaxEnts->current.integer)
                    MyAssertHandler(
                        ".\\DynEntity\\DynEntity_client.cpp",
                        1410,
                        0,
                        "%s",
                        "hitCount == dynEnt_explodeMaxEnts->current.integer");
                for (i = 0; i < (int)unsignedInt; ++i)
                    dynEntList[i] = LOWORD(v15[i].weight);
            }
            for (i = 0; i < (int)unsignedInt; ++i)
            {
                dynEntDef = (DynEntityDef *)DynEnt_GetEntityDef(dynEntList[i], drawType);
                ClientEntity = DynEnt_GetClientEntity(dynEntList[i], drawType);
                if ((ClientEntity->flags & 1) == 0)
                    MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1421, 0, "%s", "dynEntClient->flags & DYNENT_CL_ACTIVE");
                if (DynEnt_GetEntityProps(dynEntDef->type)->usePhysics && !ClientEntity->physObjId)
                {
                    dynEntPosea = DynEnt_GetClientPose(dynEntList[i], drawType);
                    PhysObj = DynEntCl_CreatePhysObj(dynEntDef, &dynEntPosea->pose);
                    ClientEntity->physObjId = (int)PhysObj;
                }
            }
        }
        Phys_AddJitterRegion(PHYS_WORLD_DYNENT, origin, innerRadius, outerRadius, minDisplacement, maxDisplacement);
    }
}

void __cdecl DynEntCl_DestroyEvent(
    int localClientNum,
    unsigned __int16 dynEntId,
    DynEntityCollType drawType,
    const float *hitPos,
    const float *hitDir)
{
    DynEntityPose *dynEntPose; // [esp+0h] [ebp-34h]
    DynEntityClient *dynEntClient; // [esp+8h] [ebp-2Ch]
    const DynEntityDef *dynEntDef; // [esp+Ch] [ebp-28h]
    float axis[3][3]; // [esp+10h] [ebp-24h] BYREF

    if (dynEntId == 0xFFFF)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1454, 0, "%s", "dynEntId != DYNENT_INVALID_ID");
    if (!hitPos)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1455, 0, "%s", "hitPos");
    if (!hitDir)
        MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1456, 0, "%s", "hitDir");
    if (DynEntCl_EventNeedsProcessed(localClientNum, 1023))
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\dynentity\\../cgame_mp/cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        dynEntDef = DynEnt_GetEntityDef(dynEntId, (DynEntityDrawType)drawType);
        dynEntClient = DynEnt_GetClientEntity(dynEntId, (DynEntityDrawType)drawType);
        if ((dynEntClient->flags & 1) != 0)
            MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1466, 0, "%s", "!(dynEntClient->flags & DYNENT_CL_ACTIVE)");
        if ((dynEntClient->flags & 2) == 0)
            MyAssertHandler(".\\DynEntity\\DynEntity_client.cpp", 1467, 0, "%s", "dynEntClient->flags & DYNENT_CL_VISIBLE");
        dynEntClient->flags &= ~2u;
        DynEntCl_UnlinkEntity(dynEntId, drawType);
        dynEntPose = DynEnt_GetClientPose(dynEntId, (DynEntityDrawType)drawType);
        if (dynEntDef->destroyFx || dynEntDef->destroyPieces)
            UnitQuatToAxis(dynEntPose->pose.quat, axis);
        if (dynEntDef->destroyFx)
            DynEntCl_PlayEventFx(dynEntDef->destroyFx, dynEntPose->pose.origin, axis);
        if (dynEntDef->destroyPieces)
            DynEntPieces_SpawnPieces(localClientNum, dynEntDef->destroyPieces, dynEntPose->pose.origin, axis, hitPos, hitDir);
    }
}

