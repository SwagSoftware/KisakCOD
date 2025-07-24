#include "fx_system.h"

int32_t warnCount_1;

void __cdecl FX_OffsetSpawnOrigin(
    const FxSpatialFrame* effectFrame,
    const FxElemDef* elemDef,
    int32_t randomSeed,
    float* spawnOrigin)
{
    float scale0; // [esp+14h] [ebp-74h]
    float v5; // [esp+18h] [ebp-70h]
    int32_t v6; // [esp+1Ch] [ebp-6Ch]
    float dir[3]; // [esp+44h] [ebp-44h] BYREF
    float height; // [esp+50h] [ebp-38h]
    float yaw; // [esp+54h] [ebp-34h]
    float radius; // [esp+58h] [ebp-30h]
    float sinYaw; // [esp+5Ch] [ebp-2Ch]
    float cosYaw; // [esp+60h] [ebp-28h]
    float axis[3][3]; // [esp+64h] [ebp-24h] BYREF

    v6 = elemDef->flags & 0x30;
    if (v6)
    {
        if (v6 == 16)
        {
            FX_RandomDir(randomSeed, dir);
            radius = elemDef->spawnOffsetRadius.amplitude * fx_randomTable[randomSeed + 11] + elemDef->spawnOffsetRadius.base;
            Vec3Mad(spawnOrigin, radius, dir, spawnOrigin);
        }
        else
        {
            if ((elemDef->flags & 0x30) != 0x20)
                MyAssertHandler(
                    ".\\EffectsCore\\fx_update_util.cpp",
                    119,
                    0,
                    "%s\n\t(elemDef->flags & FX_ELEM_SPAWN_OFFSET_MASK) = %i",
                    "((elemDef->flags & FX_ELEM_SPAWN_OFFSET_MASK) == FX_ELEM_SPAWN_OFFSET_CYLINDER)",
                    elemDef->flags & 0x30);
            radius = elemDef->spawnOffsetRadius.amplitude * fx_randomTable[randomSeed + 11] + elemDef->spawnOffsetRadius.base;
            yaw = fx_randomTable[randomSeed + 9] * 6.283185482025146;
            UnitQuatToAxis(effectFrame->quat, axis);
            cosYaw = cos(yaw);
            sinYaw = sin(yaw);
            v5 = radius * sinYaw;
            scale0 = radius * cosYaw;
            Vec3MadMad(spawnOrigin, scale0, axis[1], v5, axis[2], spawnOrigin);
            height = elemDef->spawnOffsetHeight.amplitude * fx_randomTable[randomSeed + 10] + elemDef->spawnOffsetHeight.base;
            Vec3Mad(spawnOrigin, height, axis[0], spawnOrigin);
        }
    }
}

void __cdecl FX_GetOriginForTrailElem(
    FxEffect *effect,
    const FxElemDef *elemDef,
    const FxSpatialFrame *effectFrameWhenPlayed,
    int32_t randomSeed,
    float *outOrigin,
    float *outRight,
    float *outUp)
{
    float effectFrameAxis[3][3]; // [esp+8h] [ebp-24h] BYREF

    if (!outRight || !outUp)
        MyAssertHandler(".\\EffectsCore\\fx_update_util.cpp", 143, 0, "%s", "(outRight != NULL) && (outUp != NULL)");
    if ((elemDef->flags & 0xC0) != 0)
        MyAssertHandler(
            ".\\EffectsCore\\fx_update_util.cpp",
            144,
            0,
            "%s\n\t((elemDef->flags & FX_ELEM_RUN_MASK)) = %i",
            "((elemDef->flags & FX_ELEM_RUN_MASK) == FX_ELEM_RUN_RELATIVE_TO_WORLD)",
            elemDef->flags & 0xC0);
    UnitQuatToAxis(effectFrameWhenPlayed->quat, effectFrameAxis);
    *outRight = effectFrameAxis[1][0];
    outRight[1] = effectFrameAxis[1][1];
    outRight[2] = effectFrameAxis[1][2];
    *outUp = effectFrameAxis[2][0];
    outUp[1] = effectFrameAxis[2][1];
    outUp[2] = effectFrameAxis[2][2];
    FX_GetSpawnOrigin(effectFrameWhenPlayed, elemDef, randomSeed, outOrigin);
    FX_OffsetSpawnOrigin(effectFrameWhenPlayed, elemDef, randomSeed, outOrigin);
}

void __cdecl FX_GetSpawnOrigin(
    const FxSpatialFrame *effectFrame,
    const FxElemDef *elemDef,
    int32_t randomSeed,
    float *spawnOrigin)
{
    float offset[3]; // [esp+18h] [ebp-Ch] BYREF

    offset[0] = elemDef->spawnOrigin[0].amplitude * fx_randomTable[randomSeed + 6] + elemDef->spawnOrigin[0].base;
    offset[1] = elemDef->spawnOrigin[1].amplitude * fx_randomTable[randomSeed + 7] + elemDef->spawnOrigin[1].base;
    offset[2] = elemDef->spawnOrigin[2].amplitude * fx_randomTable[randomSeed + 8] + elemDef->spawnOrigin[2].base;
    if ((elemDef->flags & 2) != 0)
        FX_TransformPosFromLocalToWorld(effectFrame, offset, spawnOrigin);
    else
        Vec3Add(offset, effectFrame->origin, spawnOrigin);
}

void __cdecl FX_TransformPosFromLocalToWorld(const FxSpatialFrame *frame, float *posLocal, float *posWorld)
{
    float axis[3][3]; // [esp+Ch] [ebp-24h] BYREF

    UnitQuatToAxis(frame->quat, axis);
    Vec3Mad(frame->origin, *posLocal, axis[0], posWorld);
    Vec3Mad(posWorld, posLocal[1], axis[1], posWorld);
    Vec3Mad(posWorld, posLocal[2], axis[2], posWorld);
}

void __cdecl FX_SpatialFrameToOrientation(const FxSpatialFrame *frame, orientation_t *orient)
{
    orient->origin[0] = frame->origin[0];
    orient->origin[1] = frame->origin[1];
    orient->origin[2] = frame->origin[2];
    UnitQuatToAxis(frame->quat, orient->axis);
}

void __cdecl FX_OrientationDirToWorldDir(const orientation_t *orient, const float *dir, float *out)
{
    if (dir == out)
        MyAssertHandler(".\\EffectsCore\\fx_update_util.cpp", 184, 0, "%s", "dir != out");
    *out = *dir * orient->axis[0][0] + dir[1] * orient->axis[1][0] + dir[2] * orient->axis[2][0];
    out[1] = *dir * orient->axis[0][1] + dir[1] * orient->axis[1][1] + dir[2] * orient->axis[2][1];
    out[2] = *dir * orient->axis[0][2] + dir[1] * orient->axis[1][2] + dir[2] * orient->axis[2][2];
}

void __cdecl FX_GetOrientation(
    const FxElemDef* elemDef,
    const FxSpatialFrame* frameAtSpawn,
    const FxSpatialFrame* frameNow,
    int32_t randomSeed,
    orientation_t* orient)
{
    const char* v5; // eax
    const char* v6; // eax
    float v7; // [esp+20h] [ebp-4Ch]
    float up[3]; // [esp+5Ch] [ebp-10h] BYREF
    int32_t runFlags; // [esp+68h] [ebp-4h]

    if (!Vec4IsNormalized(frameAtSpawn->quat))
    {
        v5 = va("%g %g %g %g", frameAtSpawn->quat[0], frameAtSpawn->quat[1], frameAtSpawn->quat[2], frameAtSpawn->quat[3]);
        MyAssertHandler(
            ".\\EffectsCore\\fx_update_util.cpp",
            196,
            0,
            "%s\n\t%s",
            "Vec4IsNormalized( frameAtSpawn->quat )",
            v5);
    }
    if (!Vec4IsNormalized(frameNow->quat))
    {
        v6 = va("%g %g %g %g", frameNow->quat[0], frameNow->quat[1], frameNow->quat[2], frameNow->quat[3]);
        MyAssertHandler(".\\EffectsCore\\fx_update_util.cpp", 197, 0, "%s\n\t%s", "Vec4IsNormalized( frameNow->quat )", v6);
    }
    runFlags = elemDef->flags & 0xC0;
    if (runFlags)
    {
        if (runFlags == 64)
        {
            FX_SpatialFrameToOrientation(frameAtSpawn, orient);
        }
        else if (runFlags == 128)
        {
            FX_SpatialFrameToOrientation(frameNow, orient);
        }
        else
        {
            if (runFlags != 192)
                MyAssertHandler(
                    ".\\EffectsCore\\fx_update_util.cpp",
                    221,
                    0,
                    "%s\n\t(runFlags) = %i",
                    "(runFlags == FX_ELEM_RUN_RELATIVE_TO_OFFSET)",
                    runFlags);
            if ((elemDef->flags & 0x30) == 0)
                MyAssertHandler(
                    ".\\EffectsCore\\fx_update_util.cpp",
                    222,
                    0,
                    "%s\n\t(elemDef->flags & FX_ELEM_SPAWN_OFFSET_MASK) = %i",
                    "((elemDef->flags & FX_ELEM_SPAWN_OFFSET_MASK) != FX_ELEM_SPAWN_OFFSET_NONE)",
                    elemDef->flags & 0x30);
            FX_GetSpawnOrigin(frameAtSpawn, elemDef, randomSeed, orient->origin);
            orient->axis[0][0] = 0.0;
            orient->axis[0][1] = 0.0;
            orient->axis[0][2] = 0.0;
            FX_OffsetSpawnOrigin(frameAtSpawn, elemDef, randomSeed, orient->axis[0]);
            Vec3Add(orient->axis[0], orient->origin, orient->origin);
            if (Vec3Normalize(orient->axis[0]) == 0.0)
            {
                orient->axis[0][0] = 1.0;
                orient->axis[0][1] = 0.0;
                orient->axis[0][2] = 0.0;
            }
            if ((elemDef->flags & 0x30) == 0x10)
            {
                Vec3Basis_RightHanded(orient->axis[0], orient->axis[1], orient->axis[2]);
            }
            else
            {
                if ((elemDef->flags & 0x30) != 0x20)
                    MyAssertHandler(
                        ".\\EffectsCore\\fx_update_util.cpp",
                        239,
                        0,
                        "%s\n\t(elemDef->flags & FX_ELEM_SPAWN_OFFSET_MASK) = %i",
                        "((elemDef->flags & FX_ELEM_SPAWN_OFFSET_MASK) == FX_ELEM_SPAWN_OFFSET_CYLINDER)",
                        elemDef->flags & 0x30);
                v7 = I_fabs(orient->axis[0][2]);
                up[0] = 0.0;
                if (v7 < 0.9990000128746033)
                {
                    up[1] = 0.0;
                    up[2] = 1.0;
                }
                else
                {
                    up[1] = 1.0;
                    up[2] = 0.0;
                }
                Vec3Cross(up, orient->axis[0], orient->axis[1]);
                Vec3Normalize(orient->axis[1]);
                Vec3Cross(orient->axis[0], orient->axis[1], orient->axis[2]);
            }
        }
    }
    else
    {
        orient->origin[0] = 0.0;
        orient->origin[1] = 0.0;
        orient->origin[2] = 0.0;
        orient->axis[0][0] = 1.0;
        orient->axis[0][1] = 0.0;
        orient->axis[0][2] = 0.0;
        orient->axis[1][0] = 0.0;
        orient->axis[1][1] = 1.0;
        orient->axis[1][2] = 0.0;
        orient->axis[2][0] = 0.0;
        orient->axis[2][1] = 0.0;
        orient->axis[2][2] = 1.0;
    }
};

void __cdecl FX_GetVelocityAtTime(
    const FxElemDef* elemDef,
    int32_t randomSeed,
    float msecLifeSpan,
    float msecElapsed,
    const orientation_t* orient,
    const float* baseVel,
    float* velocity)
{
    const char* v7; // eax
    int32_t v8; // eax
    char* v9; // eax
    double v10; // [esp+18h] [ebp-58h]
    float v11; // [esp+20h] [ebp-50h]
    float v12; // [esp+24h] [ebp-4Ch]
    float velocityWorld[3]; // [esp+28h] [ebp-48h] BYREF
    float sampleTime; // [esp+34h] [ebp-3Ch]
    float samplePoint; // [esp+38h] [ebp-38h]
    float velocityScale; // [esp+3Ch] [ebp-34h]
    int32_t sampleIndex; // [esp+40h] [ebp-30h]
    const FxElemVelStateSample* samples; // [esp+44h] [ebp-2Ch]
    int32_t intervalCount; // [esp+48h] [ebp-28h]
    float velocityLocal[3]; // [esp+4Ch] [ebp-24h] BYREF
    float weight[2]; // [esp+58h] [ebp-18h] BYREF
    float rangeLerp[3]; // [esp+60h] [ebp-10h] BYREF
    float sampleLerp; // [esp+6Ch] [ebp-4h]

    if (!elemDef)
        MyAssertHandler(".\\EffectsCore\\fx_update_util.cpp", 281, 0, "%s", "elemDef");
    if (!elemDef->velSamples)
        MyAssertHandler(".\\EffectsCore\\fx_update_util.cpp", 282, 0, "%s", "elemDef->velSamples");
    if (!elemDef->velIntervalCount)
        MyAssertHandler(
            ".\\EffectsCore\\fx_update_util.cpp",
            283,
            0,
            "%s\n\t(elemDef->velIntervalCount) = %i",
            "(elemDef->velIntervalCount >= 1)",
            elemDef->velIntervalCount);
    sampleTime = msecElapsed / msecLifeSpan;
    if (sampleTime < 0.0 || sampleTime >= 1.0)
    {
        v7 = va("%g: %g, %g", sampleTime, msecElapsed, msecLifeSpan);
        MyAssertHandler(
            ".\\EffectsCore\\fx_update_util.cpp",
            286,
            0,
            "%s\n\t%s",
            "0.0f <= sampleTime && sampleTime < 1.0f",
            v7);
    }
    v12 = fx_randomTable[randomSeed];
    rangeLerp[0] = v12;
    v11 = fx_randomTable[randomSeed + 1];
    rangeLerp[1] = v11;
    *((float*)&v10 + 1) = fx_randomTable[randomSeed + 2];
    rangeLerp[2] = *((float*)&v10 + 1);
    intervalCount = elemDef->velIntervalCount;
    samplePoint = (double)intervalCount * sampleTime;
    *(float*)&v10 = floor(samplePoint);
    v8 = (int)*(float*)&v10;
    sampleIndex = v8;
    sampleLerp = samplePoint - (double)v8;
    if (v8 < 0 || sampleIndex >= intervalCount)
    {
        v9 = va("%i for %g on %i intervals", sampleIndex, sampleTime, intervalCount);
        MyAssertHandler(
            ".\\EffectsCore\\fx_update_util.cpp",
            296,
            1,
            "%s\n\t(va( \"%i for %g on %i intervals\", sampleIndex, sampleTime, intervalCount )) = %i",
            "(sampleIndex >= 0 && sampleIndex < intervalCount)",
            v9,
            v10,
            v11,
            v12);
    }
    weight[1] = (double)intervalCount * sampleLerp;
    weight[0] = (double)intervalCount - weight[1];
    samples = &elemDef->velSamples[sampleIndex];
    *velocity = *baseVel;
    velocity[1] = baseVel[1];
    velocity[2] = baseVel[2];
    velocityScale = 1000.0;
    if ((elemDef->flags & 0x2000000) != 0)
    {
        FX_GetVelocityAtTimeInFrame(&samples->world, &samples[1].world, rangeLerp, weight, velocityWorld);
        Vec3Mad(velocity, velocityScale, velocityWorld, velocity);
    }
    if ((elemDef->flags & 0x1000000) != 0)
    {
        FX_GetVelocityAtTimeInFrame(&samples->local, &samples[1].local, rangeLerp, weight, velocityLocal);
        FX_OrientationDirToWorldDir(orient, velocityLocal, velocityWorld);
        Vec3Mad(velocity, velocityScale, velocityWorld, velocity);
    }
}

void __cdecl FX_GetVelocityAtTimeInFrame(
    const FxElemVelStateInFrame *statePrev,
    const FxElemVelStateInFrame *stateNext,
    const float *rangeLerp,
    const float *weight,
    float *velocity)
{
    *velocity = (*rangeLerp * statePrev->velocity.amplitude[0] + statePrev->velocity.base[0]) * *weight;
    *velocity = (*rangeLerp * stateNext->velocity.amplitude[0] + stateNext->velocity.base[0]) * weight[1] + *velocity;
    velocity[1] = (rangeLerp[1] * statePrev->velocity.amplitude[1] + statePrev->velocity.base[1]) * *weight;
    velocity[1] = (rangeLerp[1] * stateNext->velocity.amplitude[1] + stateNext->velocity.base[1]) * weight[1]
        + velocity[1];
    velocity[2] = (rangeLerp[2] * statePrev->velocity.amplitude[2] + statePrev->velocity.base[2]) * *weight;
    velocity[2] = (rangeLerp[2] * stateNext->velocity.amplitude[2] + stateNext->velocity.base[2]) * weight[1]
        + velocity[2];
}

void __cdecl FX_OrientationPosToWorldPos(const orientation_t *orient, const float *pos, float *out)
{
    if (pos == out)
        MyAssertHandler(".\\EffectsCore\\fx_update_util.cpp", 323, 0, "%s", "pos != out");
    *out = *pos * orient->axis[0][0] + orient->origin[0] + pos[1] * orient->axis[1][0] + pos[2] * orient->axis[2][0];
    out[1] = *pos * orient->axis[0][1] + orient->origin[1] + pos[1] * orient->axis[1][1] + pos[2] * orient->axis[2][1];
    out[2] = *pos * orient->axis[0][2] + orient->origin[2] + pos[1] * orient->axis[1][2] + pos[2] * orient->axis[2][2];
}

void __cdecl FX_OrientationPosFromWorldPos(const orientation_t *orient, const float *pos, float *out)
{
    float dir; // [esp+0h] [ebp-Ch]
    float dir_4; // [esp+4h] [ebp-8h]
    float dir_8; // [esp+8h] [ebp-4h]

    if (pos == out)
        MyAssertHandler(".\\EffectsCore\\fx_update_util.cpp", 334, 0, "%s", "pos != out");
    dir = *pos - orient->origin[0];
    dir_4 = pos[1] - orient->origin[1];
    dir_8 = pos[2] - orient->origin[2];
    *out = dir * orient->axis[0][0] + dir_4 * orient->axis[0][1] + dir_8 * orient->axis[0][2];
    out[1] = dir * orient->axis[1][0] + dir_4 * orient->axis[1][1] + dir_8 * orient->axis[1][2];
    out[2] = dir * orient->axis[2][0] + dir_4 * orient->axis[2][1] + dir_8 * orient->axis[2][2];
}

void __cdecl FX_AddVisBlocker(FxSystem *system, const float *posWorld, float radius, float opacity)
{
    FxVisState *visState; // [esp+1Ch] [ebp-Ch]
    int32_t blockerIndex; // [esp+20h] [ebp-8h]
    FxVisBlocker *localVisBlocker; // [esp+24h] [ebp-4h]

    visState = system->visStateBufferWrite;
    blockerIndex = visState->blockerCount + 1;
    if (blockerIndex < 256)
    {
        localVisBlocker = &visState->blocker[blockerIndex];
        localVisBlocker->origin[0] = *posWorld;
        localVisBlocker->origin[1] = posWorld[1];
        localVisBlocker->origin[2] = posWorld[2];
        if (radius >= 4096.0)
            MyAssertHandler(
                ".\\EffectsCore\\fx_update_util.cpp",
                383,
                0,
                "%s",
                "radius < 65536.0f * FX_VIS_BLOCKER_RADIUS_INV_SCALE");
        if (opacity < 0.0 || opacity >= 1.0)
            MyAssertHandler(
                ".\\EffectsCore\\fx_update_util.cpp",
                384,
                0,
                "%s",
                "opacity >= 0.0f && opacity < 65536.0f * FX_VIS_BLOCKER_VISIBILITY_INV_SCALE");
        localVisBlocker->radius = (int)(radius * 16.0);
        localVisBlocker->visibility = (int)((1.0 - opacity) * 65536.0);
        InterlockedIncrement(&visState->blockerCount);
    }
    else if (warnCount_1 != system->frameCount)
    {
        warnCount_1 = system->frameCount;
        Com_PrintWarning(21, "More than %i visibility blocking particles exist concurrently\n", 256);
    }
}

void __cdecl FX_ToggleVisBlockerFrame(FxSystem *system)
{
    FxVisState *visStateSwapCache; // [esp+0h] [ebp-4h]

    if (!system)
        MyAssertHandler(".\\EffectsCore\\fx_update_util.cpp", 400, 0, "%s", "system");
    visStateSwapCache = (FxVisState *)system->visStateBufferRead;
    system->visStateBufferRead = system->visStateBufferWrite;
    system->visStateBufferWrite = visStateSwapCache;
    system->visStateBufferWrite->blockerCount = 0;
    fx_serverVisClient = system->localClientNum;
}

char __cdecl FX_CullSphere(const FxCamera *camera, uint32_t frustumPlaneCount, const float *posWorld, float radius)
{
    const char *v4; // eax
    const char *v5; // eax
    double v7; // [esp+18h] [ebp-18h]
    float pointToPlaneDist; // [esp+28h] [ebp-8h]
    uint32_t planeIndex; // [esp+2Ch] [ebp-4h]

    if (!camera->isValid)
        MyAssertHandler(".\\EffectsCore\\fx_update_util.cpp", 439, 0, "%s", "camera->isValid");
    if (frustumPlaneCount != camera->frustumPlaneCount && frustumPlaneCount != 5)
    {
        v4 = va("%i, %i", frustumPlaneCount, camera->frustumPlaneCount);
        MyAssertHandler(
            ".\\EffectsCore\\fx_update_util.cpp",
            440,
            0,
            "%s\n\t%s",
            "frustumPlaneCount == camera->frustumPlaneCount || frustumPlaneCount == 5",
            v4);
    }
    for (planeIndex = 0; planeIndex < frustumPlaneCount; ++planeIndex)
    {
        if (!Vec3IsNormalized(camera->frustum[planeIndex]))
        {
            v7 = Vec3Length(camera->frustum[planeIndex]);
            v5 = va(
                "(%g %g %g) len %g",
                camera->frustum[planeIndex][0],
                camera->frustum[planeIndex][1],
                camera->frustum[planeIndex][2],
                v7);
            MyAssertHandler(
                ".\\EffectsCore\\fx_update_util.cpp",
                444,
                0,
                "%s\n\t%s",
                "Vec3IsNormalized( camera->frustum[planeIndex] )",
                v5);
        }
        pointToPlaneDist = Vec3Dot(camera->frustum[planeIndex], posWorld) - camera->frustum[planeIndex][3];
        if (pointToPlaneDist <= -radius)
            return 1;
    }
    return 0;
}

void __cdecl FX_GetElemAxis(
    const FxElemDef* elemDef,
    int32_t randomSeed,
    const orientation_t* orient,
    float msecElapsed,
    mat3x3 &axis)
{
    float v5; // [esp+0h] [ebp-90h]
    float v6; // [esp+4h] [ebp-8Ch]
    float v7; // [esp+8h] [ebp-88h]
    float angles[3]; // [esp+84h] [ebp-Ch] BYREF

    angles[0] = elemDef->spawnAngles[0].amplitude * fx_randomTable[randomSeed + 12] + elemDef->spawnAngles[0].base;
    angles[1] = elemDef->spawnAngles[1].amplitude * fx_randomTable[randomSeed + 13] + elemDef->spawnAngles[1].base;
    angles[2] = elemDef->spawnAngles[2].amplitude * fx_randomTable[randomSeed + 14] + elemDef->spawnAngles[2].base;
    v7 = elemDef->angularVelocity[0].amplitude * fx_randomTable[randomSeed + 3] + elemDef->angularVelocity[0].base;
    angles[0] = msecElapsed * v7 + angles[0];
    v6 = elemDef->angularVelocity[1].amplitude * fx_randomTable[randomSeed + 4] + elemDef->angularVelocity[1].base;
    angles[1] = msecElapsed * v6 + angles[1];
    v5 = elemDef->angularVelocity[2].amplitude * fx_randomTable[randomSeed + 5] + elemDef->angularVelocity[2].base;
    angles[2] = msecElapsed * v5 + angles[2];
    FX_AnglesToOrientedAxis(angles, orient, (float(*)[3][3])&axis);
}

void __cdecl FX_AnglesToOrientedAxis(const float* anglesInRad, const orientation_t* orient, float (*axisOut)[3][3])
{
    float v3; // [esp+8h] [ebp-40h]
    float v4; // [esp+14h] [ebp-34h]
    float v5; // [esp+20h] [ebp-28h]
    float cy; // [esp+24h] [ebp-24h]
    float sr; // [esp+28h] [ebp-20h]
    float localDir[3]; // [esp+2Ch] [ebp-1Ch] BYREF
    float v9; // [esp+38h] [ebp-10h]
    float cr; // [esp+3Ch] [ebp-Ch]
    float cp; // [esp+40h] [ebp-8h]
    float sy; // [esp+44h] [ebp-4h]

    v5 = anglesInRad[1];
    cy = cos(v5);
    sy = sin(v5);
    v4 = *anglesInRad;
    cp = cos(v4);
    v9 = sin(v4);
    v3 = anglesInRad[2];
    cr = cos(v3);
    sr = sin(v3);
    localDir[0] = cp * cy;
    localDir[1] = cp * sy;
    localDir[2] = -v9;
    FX_OrientationDirToWorldDir(orient, localDir, (float*)axisOut);
    localDir[0] = sr * v9 * cy - cr * sy;
    localDir[1] = sr * v9 * sy + cr * cy;
    localDir[2] = sr * cp;
    FX_OrientationDirToWorldDir(orient, localDir, (*axisOut)[1]);
    localDir[0] = cr * v9 * cy + sr * sy;
    localDir[1] = cr * v9 * sy - sr * cy;
    localDir[2] = cr * cp;
    FX_OrientationDirToWorldDir(orient, localDir, (*axisOut)[2]);
}