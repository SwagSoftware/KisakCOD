#include "cg_local.h"
#include "cg_public.h"

#include <qcommon/mem_track.h>

#ifdef KISAK_MP
#include <cgame_mp/cg_local_mp.h>
#elif KISAK_SP
#include "cg_main.h"
#endif


CameraShakeSet s_cameraShakeSet[1];

void __cdecl TRACK_cg_camerashake()
{
    track_static_alloc_internal(s_cameraShakeSet, 148, "s_cameraShakeSet", 9);
}

void __cdecl CG_StartShakeCamera(int32_t localClientNum, float p, int32_t duration, float *src, float radius)
{
    int32_t j; // [esp+8h] [ebp-38h]
    float minsize; // [esp+10h] [ebp-30h]
    int32_t i; // [esp+14h] [ebp-2Ch]
    CameraShakeSet *cameraShakeArray; // [esp+18h] [ebp-28h]
    CameraShake buildShake; // [esp+1Ch] [ebp-24h] BYREF
    const cg_s *cgameGlob;

    iassert(p > 0.0f);
    iassert(duration > 0);
    iassert(radius > 0.0f);

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    buildShake.size = 0.0f;
    buildShake.rumbleScale = 0.0f;
    buildShake.scale = p;
    buildShake.length = (float)duration;
    buildShake.time = cgameGlob->time;
    buildShake.src[0] = src[0];
    buildShake.src[1] = src[1];
    buildShake.src[2] = src[2];
    buildShake.radius = radius;
    CG_UpdateCameraShake(cgameGlob, &buildShake);
    cameraShakeArray = &s_cameraShakeSet[localClientNum];
    for (i = 0; i < 4; ++i)
    {
        if (cameraShakeArray->shakes[i].time > cgameGlob->time
            || (double)cgameGlob->time >= (double)cameraShakeArray->shakes[i].time + cameraShakeArray->shakes[i].length)
        {
            goto LABEL_23;
        }
    }
    minsize = buildShake.size;
    if (i != 4)
        MyAssertHandler(".\\cgame\\cg_camerashake.cpp", 118, 0, "%s", "i == MAX_CAMERA_SHAKE");
    for (j = 0; j < 4; ++j)
    {
        if (minsize > (double)cameraShakeArray->shakes[j].size)
        {
            minsize = cameraShakeArray->shakes[j].size;
            i = j;
        }
    }
    if (i != 4)
        LABEL_23:
    memcpy(&cameraShakeArray->shakes[i], &buildShake, sizeof(cameraShakeArray->shakes[i]));
}

int32_t __cdecl CG_UpdateCameraShake(const cg_s *cgameGlob, CameraShake *shake)
{
    double v3; // st7
    float diff[3]; // [esp+Ch] [ebp-2Ch] BYREF
    float scale; // [esp+18h] [ebp-20h]
    float radius; // [esp+1Ch] [ebp-1Ch]
    float length; // [esp+20h] [ebp-18h]
    int32_t dtime; // [esp+24h] [ebp-14h]
    float dist; // [esp+28h] [ebp-10h]
    float val; // [esp+2Ch] [ebp-Ch]
    float timePercent; // [esp+30h] [ebp-8h]
    float x; // [esp+34h] [ebp-4h]

    length = shake->length;
    if ((LODWORD(length) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\cgame\\cg_camerashake.cpp", 46, 0, "%s", "!IS_NAN(shake->length)");
    radius = shake->radius;
    if ((LODWORD(radius) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\cgame\\cg_camerashake.cpp", 47, 0, "%s", "!IS_NAN(shake->radius)");
    scale = shake->scale;
    if ((LODWORD(scale) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\cgame\\cg_camerashake.cpp", 48, 0, "%s", "!IS_NAN(shake->scale)");
    dtime = cgameGlob->time - shake->time;
    if (dtime < 0 || shake->length <= (double)dtime)
        return 0;
    if (shake->length <= 0.0)
        MyAssertHandler(
            ".\\cgame\\cg_camerashake.cpp",
            54,
            0,
            "%s\n\t(shake->length) = %g",
            "(shake->length > 0.0f)",
            shake->length);
    if (shake->radius <= 0.0)
        MyAssertHandler(
            ".\\cgame\\cg_camerashake.cpp",
            55,
            0,
            "%s\n\t(shake->radius) = %g",
            "(shake->radius > 0.0f)",
            shake->radius);
    if (shake->scale <= 0.0)
        MyAssertHandler(
            ".\\cgame\\cg_camerashake.cpp",
            56,
            0,
            "%s\n\t(shake->scale) = %g",
            "(shake->scale > 0.0f)",
            shake->scale);
    Vec3Sub(cgameGlob->refdef.vieworg, shake->src, diff);
    dist = Vec3Length(diff);
    val = 1.0 - dist / shake->radius;
    timePercent = (double)dtime / shake->length;
    if (timePercent < 0.0 || timePercent >= 1.0)
        MyAssertHandler(
            ".\\cgame\\cg_camerashake.cpp",
            62,
            0,
            "%s\n\t(timePercent) = %g",
            "(timePercent >= 0.0f && timePercent < 1.0f)",
            timePercent);
    x = (1.0 - timePercent) * shake->scale;
    if (x <= 0.0)
        return 0;
    if (val < 0.0)
        v3 = val / x;
    else
        v3 = val * x;
    val = v3;
    shake->size = val;
    shake->rumbleScale = x;
    return 1;
}

void __cdecl CG_ShakeCamera(int32_t localClientNum)
{
    float v1; // [esp+0h] [ebp-38h]
    float v2; // [esp+4h] [ebp-34h]
    float v3; // [esp+8h] [ebp-30h]
    float v4; // [esp+Ch] [ebp-2Ch]
    float v5; // [esp+10h] [ebp-28h]
    float v6; // [esp+14h] [ebp-24h]
    CameraShakeSet* camShakeSet; // [esp+18h] [ebp-20h]
    CameraShake* cameraShake; // [esp+1Ch] [ebp-1Ch]
    float val; // [esp+24h] [ebp-14h]
    float vala; // [esp+24h] [ebp-14h]
    float valb; // [esp+24h] [ebp-14h]
    float rumbleScale; // [esp+28h] [ebp-10h]
    float scale; // [esp+2Ch] [ebp-Ch]
    int32_t i; // [esp+30h] [ebp-8h]
    float sx; // [esp+34h] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    camShakeSet = &s_cameraShakeSet[localClientNum];
    scale = 0.0f;
    rumbleScale = 0.0f;
    sx = cgameGlob->time / 600.0f;

    for (i = 0; i < 4; ++i)
    {
        cameraShake = &camShakeSet->shakes[i];
        if (CG_UpdateCameraShake(cgameGlob, cameraShake))
        {
            if (scale < (double)cameraShake->size)
            {
                scale = cameraShake->size;
                rumbleScale = cameraShake->rumbleScale;
            }
        }
    }

    if (scale < cgameGlob->rumbleScale)
    {
        scale = cgameGlob->rumbleScale;
        rumbleScale = scale;
    }

    if (scale > 0.0f)
    {
        if (scale > 1.0f)
            scale = 1.0f;

        v3 = sin(camShakeSet->phase + sx * 25.13274192810059);
        val = v3 * rumbleScale * 18.0 * scale;
        cgameGlob->refdefViewAngles[0] = cgameGlob->refdefViewAngles[0] + val;
        v2 = sin(camShakeSet->phase + sx * 47.1238899230957);
        vala = v2 * rumbleScale * 16.0 * scale;
        cgameGlob->refdefViewAngles[1] = cgameGlob->refdefViewAngles[1] + vala;
        v1 = sin(camShakeSet->phase + sx * 37.69911193847656);
        valb = v1 * rumbleScale * 10.0 * scale;
        cgameGlob->refdefViewAngles[2] = cgameGlob->refdefViewAngles[2] + valb;
    }
    else
    {
        camShakeSet->phase = crandom() * 3.141592741012573;
    }
}

void __cdecl CG_ClearCameraShakes(int32_t localClientNum)
{
    memset((uint8_t *)&s_cameraShakeSet[localClientNum], 0, 0x90u);
}

