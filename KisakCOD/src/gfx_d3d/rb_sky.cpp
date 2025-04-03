#include "rb_sky.h"
#include <qcommon/mem_track.h>
#include "r_init.h"
#include "rb_backend.h"


SunFlareDynamic sunFlareArray[4];

void __cdecl TRACK_rb_sky()
{
    track_static_alloc_internal(sunFlareArray, 160, "sunFlareArray", 18);
}

unsigned int __cdecl RB_CalcSunSpriteSamples()
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    int v5; // [esp+38h] [ebp-1Ch]
    int v6; // [esp+3Ch] [ebp-18h]
    int v7; // [esp+40h] [ebp-14h]
    int v8; // [esp+44h] [ebp-10h]
    IDirect3DQuery9 *occlusionQuery; // [esp+48h] [ebp-Ch]
    HRESULT hr; // [esp+4Ch] [ebp-8h]
    unsigned int sampleCount; // [esp+50h] [ebp-4h] BYREF

    if (vidConfig.displayWidth < 0x10)
        MyAssertHandler(
            ".\\rb_sky.cpp",
            45,
            0,
            "%s\n\t(vidConfig.displayWidth) = %i",
            "(vidConfig.displayWidth >= 16)",
            vidConfig.displayWidth);
    if (vidConfig.displayHeight < 0x10)
        MyAssertHandler(
            ".\\rb_sky.cpp",
            46,
            0,
            "%s\n\t(vidConfig.displayHeight) = %i",
            "(vidConfig.displayHeight >= 16)",
            vidConfig.displayHeight);
    occlusionQuery = sunFlareArray[0].sunQuery[0];
    if (!sunFlareArray[0].sunQuery[0])
        return 0;
    RB_ResetStatTracking();
    if (tess.indexCount)
        RB_EndTessSurface();
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("dx.device->BeginScene()\n");
        v8 = dx.device->BeginScene(dx.device);
        if (v8 < 0)
        {
            do
            {
                ++g_disableRendering;
                v1 = R_ErrorDescription(v8);
                Com_Error(ERR_FATAL, ".\\rb_sky.cpp (%i) dx.device->BeginScene() failed: %s\n", 58, v1);
            } while (alwaysfails);
        }
    } while (alwaysfails);
    R_ClearScreenInternal(gfxCmdBufState.prim.device, 2u, colorWhite, 1.0, 0, 0);
    R_SetCodeConstantFromVec4(&gfxCmdBufSourceState, 0x28u, (float *)colorWhite);
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("occlusionQuery->Issue( (1 << 1) )\n");
        v7 = occlusionQuery->Issue(occlusionQuery, 2u);
        if (v7 < 0)
        {
            do
            {
                ++g_disableRendering;
                v2 = R_ErrorDescription(v7);
                Com_Error(ERR_FATAL, ".\\rb_sky.cpp (%i) occlusionQuery->Issue( (1 << 1) ) failed: %s\n", 62, v2);
            } while (alwaysfails);
        }
    } while (alwaysfails);
    R_Set2D(&gfxCmdBufSourceState);
    RB_DrawStretchPic(rgp.additiveMaterial, 0.0, 0.0, 16.0, 16.0, 0.0, 0.0, 1.0, 1.0, 0xFFFFFFFF, GFX_PRIM_STATS_CODE);
    if (gfxCmdBufSourceState.viewMode != VIEW_MODE_2D)
        MyAssertHandler(".\\rb_sky.cpp", 65, 1, "%s", "gfxCmdBufSourceState.viewMode == VIEW_MODE_2D");
    RB_EndTessSurface();
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("occlusionQuery->Issue( (1 << 0) )\n");
        v6 = ((int(__thiscall *)(IDirect3DQuery9 *, IDirect3DQuery9 *, int))occlusionQuery->Issue)(
            occlusionQuery,
            occlusionQuery,
            1);
        if (v6 < 0)
        {
            do
            {
                ++g_disableRendering;
                v3 = R_ErrorDescription(v6);
                Com_Error(ERR_FATAL, ".\\rb_sky.cpp (%i) occlusionQuery->Issue( (1 << 0) ) failed: %s\n", 67, v3);
            } while (alwaysfails);
        }
    } while (alwaysfails);
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("dx.device->EndScene()\n");
        v5 = dx.device->EndScene(dx.device);
        if (v5 < 0)
        {
            do
            {
                ++g_disableRendering;
                v4 = R_ErrorDescription(v5);
                Com_Error(ERR_FATAL, ".\\rb_sky.cpp (%i) dx.device->EndScene() failed: %s\n", 69, v4);
            } while (alwaysfails);
        }
    } while (alwaysfails);
    while (1)
    {
        hr = occlusionQuery->GetData(occlusionQuery, &sampleCount, 4u, 1u);
        if (hr != 1)
            break;
        Sleep(0);
    }
    if (hr)
        return 256;
    return sampleCount;
}

void __cdecl RB_DrawSun(unsigned int localClientNum)
{
    SunFlareDynamic *sunFlare; // [esp+0h] [ebp-4h]

    if (!rgp.world)
        MyAssertHandler(".\\rb_sky.cpp", 590, 0, "%s", "rgp.world");
    if (localClientNum >= gfxCfg.maxClientViews)
        MyAssertHandler(
            ".\\rb_sky.cpp",
            591,
            0,
            "localClientNum doesn't index gfxCfg.maxClientViews\n\t%i not in [0, %i)",
            localClientNum,
            gfxCfg.maxClientViews);
    if (r_drawSun->current.enabled && rgp.world->sun.hasValidData)
    {
        sunFlare = &sunFlareArray[localClientNum];
        RB_SetIdentity();
        RB_DrawSunQuerySprite(sunFlare);
        RB_AddSunEffects(sunFlare);
        if (tess.indexCount)
            RB_EndTessSurface();
    }
}

void __cdecl RB_DrawSunQuerySprite(SunFlareDynamic *sunFlare)
{
    float v2; // [esp+10h] [ebp-74h]
    float v3; // [esp+14h] [ebp-70h]
    float widthInClipSpace; // [esp+28h] [ebp-5Ch]
    float heightInClipSpace; // [esp+2Ch] [ebp-58h]
    unsigned int sunSpriteSamples; // [esp+70h] [ebp-14h]
    unsigned int drawnSampleCount; // [esp+74h] [ebp-10h]
    float lastVisibilitya; // [esp+7Ch] [ebp-8h]
    float lastVisibility; // [esp+7Ch] [ebp-8h]
    int queryIndex; // [esp+80h] [ebp-4h]

    if (!sunFlare)
        MyAssertHandler(".\\rb_sky.cpp", 316, 0, "%s", "sunFlare");
    queryIndex = r_glob.backEndFrameCount % 2;
    if ((unsigned int)(r_glob.backEndFrameCount % 2) >= 2)
        MyAssertHandler(
            ".\\rb_sky.cpp",
            319,
            0,
            "queryIndex doesn't index ARRAY_COUNT( sunFlare->sunQuery )\n\t%i not in [0, %i)",
            r_glob.backEndFrameCount % 2,
            2);
    if (sunFlare->sunQuery[queryIndex])
    {
        //Profile_Begin(120);
        RB_SetTessTechnique(rgp.additiveMaterial, TECHNIQUE_UNLIT);
        R_TrackPrims(&gfxCmdBufState, GFX_PRIM_STATS_FX);
        RB_SetIdentity();
        heightInClipSpace = 16.0 / (double)gfxCmdBufSourceState.renderTargetHeight;
        widthInClipSpace = 16.0 / (double)gfxCmdBufSourceState.renderTargetWidth;
        RB_TessSunBillboard(widthInClipSpace, heightInClipSpace, (GfxColor)-16777216);
        if (sunFlare->sunQueryIssued[queryIndex])
        {
            drawnSampleCount = RB_HW_ReadOcclusionQuery(sunFlare->sunQuery[queryIndex]);
            if (drawnSampleCount == -1)
                sunFlare->error = 1;
            sunSpriteSamples = dx.sunSpriteSamples;
            if (!dx.sunSpriteSamples)
                MyAssertHandler(".\\rb_sky.cpp", 356, 0, "%s\n\t(sunSpriteSamples) = %i", "(sunSpriteSamples > 0)", 0);
            if (drawnSampleCount > sunSpriteSamples)
                sunFlare->error = 1;
            lastVisibilitya = (double)drawnSampleCount / (double)sunSpriteSamples;
            v3 = 1.0 - lastVisibilitya;
            if (v3 < 0.0)
                v2 = 1.0;
            else
                v2 = (double)drawnSampleCount / (double)sunSpriteSamples;
            lastVisibility = v2;
        }
        else
        {
            lastVisibility = 0.0;
        }
        if (!sunFlare->error)
            sunFlare->lastVisibility = lastVisibility;
        sunFlare->error = 0;
        if (pixelCostMode == GFX_PIXEL_COST_MODE_MEASURE_COST || pixelCostMode == GFX_PIXEL_COST_MODE_MEASURE_MSEC)
        {
            RB_EndTessSurface();
        }
        else
        {
            RB_HW_BeginOcclusionQuery(sunFlare->sunQuery[queryIndex]);
            RB_EndTessSurface();
            sunFlare->sunQuery[queryIndex]->Issue(sunFlare->sunQuery[queryIndex], 1u);
            sunFlare->sunQueryIssued[queryIndex] = 1;
        }
        //Profile_EndInternal(0);
    }
    else
    {
        RB_UpdateSunVisibilityWithoutQuery(sunFlare);
    }
}

void __cdecl RB_HW_BeginOcclusionQuery(IDirect3DQuery9 *query)
{
    if (!query)
        MyAssertHandler("c:\\trees\\cod3\\src\\gfx_d3d\\rb_query_d3d.h", 26, 0, "%s", "query");
    query->Issue(query, 2u);
}

unsigned int __cdecl RB_HW_ReadOcclusionQuery(IDirect3DQuery9 *query)
{
    HRESULT hr; // [esp+4h] [ebp-8h]
    unsigned int pixelCount; // [esp+8h] [ebp-4h] BYREF

    while (1)
    {
        hr = query->GetData(query, &pixelCount, 4u, 1u);
        if (hr != 1)
            break;
        Sleep(0);
    }
    if (hr >= 0)
        return pixelCount;
    else
        return -1;
}

void __cdecl RB_TessSunBillboard(float widthInClipSpace, float heightInClipSpace, GfxColor color)
{
    GfxVertex *v3; // eax
    float v4; // [esp+Ch] [ebp-34h]
    float v5; // [esp+10h] [ebp-30h]
    float v6; // [esp+14h] [ebp-2Ch]
    GfxMatrix *transform; // [esp+18h] [ebp-28h]
    float sunTransformedPosition; // [esp+2Ch] [ebp-14h]
    float sunTransformedPosition_4; // [esp+30h] [ebp-10h]
    float sunTransformedPosition_8; // [esp+34h] [ebp-Ch]
    float sunTransformedPosition_12; // [esp+38h] [ebp-8h]

    transform = &gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix;
    if (!rgp.world)
        MyAssertHandler(".\\rb_sky.cpp", 198, 0, "%s", "rgp.world");
    sunTransformedPosition = rgp.world->sun.sunFxPosition[0] * transform->m[0][0]
        + rgp.world->sun.sunFxPosition[1] * transform->m[1][0]
        + rgp.world->sun.sunFxPosition[2] * transform->m[2][0];
    sunTransformedPosition_4 = rgp.world->sun.sunFxPosition[0] * transform->m[0][1]
        + rgp.world->sun.sunFxPosition[1] * transform->m[1][1]
        + rgp.world->sun.sunFxPosition[2] * transform->m[2][1];
    sunTransformedPosition_8 = rgp.world->sun.sunFxPosition[0] * transform->m[0][2]
        + rgp.world->sun.sunFxPosition[1] * transform->m[1][2]
        + rgp.world->sun.sunFxPosition[2] * transform->m[2][2];
    sunTransformedPosition_12 = rgp.world->sun.sunFxPosition[0] * transform->m[0][3]
        + rgp.world->sun.sunFxPosition[1] * transform->m[1][3]
        + rgp.world->sun.sunFxPosition[2] * transform->m[2][3];
    v3 = RB_SetTessQuad(color);
    v4 = sunTransformedPosition_12 * widthInClipSpace;
    v5 = sunTransformedPosition_12 * heightInClipSpace;
    v6 = sunTransformedPosition_12 * -0.0005000000237487257;
    v3->xyzw[0] = sunTransformedPosition;
    v3->xyzw[1] = sunTransformedPosition_4;
    v3->xyzw[2] = sunTransformedPosition_8;
    v3->xyzw[3] = sunTransformedPosition_12;
    v3[1].xyzw[0] = sunTransformedPosition;
    v3[1].xyzw[1] = sunTransformedPosition_4;
    v3[1].xyzw[2] = sunTransformedPosition_8;
    v3[1].xyzw[3] = sunTransformedPosition_12;
    v3[2].xyzw[0] = sunTransformedPosition;
    v3[2].xyzw[1] = sunTransformedPosition_4;
    v3[2].xyzw[2] = sunTransformedPosition_8;
    v3[2].xyzw[3] = sunTransformedPosition_12;
    v3[3].xyzw[0] = sunTransformedPosition;
    v3[3].xyzw[1] = sunTransformedPosition_4;
    v3[3].xyzw[2] = sunTransformedPosition_8;
    v3[3].xyzw[3] = sunTransformedPosition_12;
    v3->xyzw[0] = v3->xyzw[0] + v4;
    v3->xyzw[1] = v3->xyzw[1] + v5;
    v3->xyzw[2] = v3->xyzw[2] + v6;
    v3[1].xyzw[0] = v3[1].xyzw[0] + v4;
    v3[1].xyzw[1] = v3[1].xyzw[1] - v5;
    v3[1].xyzw[2] = v3[1].xyzw[2] + v6;
    v3[2].xyzw[0] = v3[2].xyzw[0] - v4;
    v3[2].xyzw[1] = v3[2].xyzw[1] - v5;
    v3[2].xyzw[2] = v3[2].xyzw[2] + v6;
    v3[3].xyzw[0] = v3[3].xyzw[0] - v4;
    v3[3].xyzw[1] = v3[3].xyzw[1] + v5;
    v3[3].xyzw[2] = v3[3].xyzw[2] + v6;
}

GfxVertex *__cdecl RB_SetTessQuad(GfxColor color)
{
    unsigned __int16 vertCount; // [esp+10h] [ebp-8h]
    GfxVertex *vert; // [esp+14h] [ebp-4h]

    if (tess.vertexCount)
        MyAssertHandler(".\\rb_sky.cpp", 152, 0, "%s", "tess.vertexCount == 0");
    if (tess.indexCount)
        MyAssertHandler(".\\rb_sky.cpp", 153, 0, "%s", "tess.indexCount == 0");
    vertCount = tess.vertexCount;
    tess.indices[tess.indexCount] = LOWORD(tess.vertexCount) + 3;
    tess.indices[tess.indexCount + 1] = vertCount;
    tess.indices[tess.indexCount + 2] = vertCount + 2;
    tess.indices[tess.indexCount + 3] = vertCount + 2;
    tess.indices[tess.indexCount + 4] = vertCount;
    tess.indices[tess.indexCount + 5] = vertCount + 1;
    vert = &tess.verts[vertCount];
    vert->normal.packed = 1073643391;
    vert->texCoord[0] = 0.0;
    vert->texCoord[1] = 0.0;
    vert->color = color;
    vert[1].normal.packed = 1073643391;
    vert[1].texCoord[0] = 1.0;
    vert[1].texCoord[1] = 0.0;
    vert[1].color = color;
    vert[2].normal.packed = 1073643391;
    vert[2].texCoord[0] = 1.0;
    vert[2].texCoord[1] = 1.0;
    vert[2].color = color;
    vert[3].normal.packed = 1073643391;
    vert[3].texCoord[0] = 0.0;
    vert[3].texCoord[1] = 1.0;
    vert[3].color = color;
    tess.vertexCount += 4;
    tess.indexCount += 6;
    return vert;
}

void __cdecl RB_UpdateSunVisibilityWithoutQuery(SunFlareDynamic *sunFlare)
{
    float sunTraceEnd[3]; // [esp+Ch] [ebp-Ch] BYREF

    if (!sunFlare)
        MyAssertHandler(".\\rb_sky.cpp", 292, 0, "%s", "sunFlare");
    sunFlare->lastVisibility = RB_GetSunSampleRectRelativeArea(16, 16);
    if (sunFlare->lastVisibility != 0.0)
    {
        Vec3Mad(gfxCmdBufSourceState.viewParms3D->origin, 262144.0, rgp.world->sun.sunFxPosition, sunTraceEnd);
        sunFlare->hitNum = CM_BoxSightTrace(
            sunFlare->hitNum,
            gfxCmdBufSourceState.viewParms3D->origin,
            sunTraceEnd,
            vec3_origin,
            vec3_origin,
            0,
            8195);
        if (sunFlare->hitNum)
            sunFlare->lastVisibility = 0.0;
    }
}

double __cdecl RB_GetSunSampleRectRelativeArea(int widthInPixels, int heightInPixels)
{
    float v4; // [esp+Ch] [ebp-50h]
    float v5; // [esp+1Ch] [ebp-40h]
    GfxMatrix *transform; // [esp+2Ch] [ebp-30h]
    int top; // [esp+30h] [ebp-2Ch]
    signed int right; // [esp+34h] [ebp-28h]
    int left; // [esp+3Ch] [ebp-20h]
    float sunTransformedPosition; // [esp+44h] [ebp-18h]
    float sunTransformedPosition_4; // [esp+48h] [ebp-14h]
    float sunTransformedPosition_12; // [esp+50h] [ebp-Ch]
    signed int bottom; // [esp+54h] [ebp-8h]

    transform = &gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix;
    if (!rgp.world)
        MyAssertHandler(".\\rb_sky.cpp", 254, 0, "%s", "rgp.world");
    sunTransformedPosition_12 = rgp.world->sun.sunFxPosition[0] * transform->m[0][3]
        + rgp.world->sun.sunFxPosition[1] * transform->m[1][3]
        + rgp.world->sun.sunFxPosition[2] * transform->m[2][3];
    if (sunTransformedPosition_12 <= 0.0)
        return 0.0;
    sunTransformedPosition = rgp.world->sun.sunFxPosition[0] * transform->m[0][0]
        + rgp.world->sun.sunFxPosition[1] * transform->m[1][0]
        + rgp.world->sun.sunFxPosition[2] * transform->m[2][0];
    sunTransformedPosition_4 = rgp.world->sun.sunFxPosition[0] * transform->m[0][1]
        + rgp.world->sun.sunFxPosition[1] * transform->m[1][1]
        + rgp.world->sun.sunFxPosition[2] * transform->m[2][1];
    v5 = ((double)(int)vidConfig.displayWidth * (sunTransformedPosition / sunTransformedPosition_12 + 1.0)
        - (double)widthInPixels)
        * 0.5;
    left = (int)(v5 + 9.313225746154785e-10);
    v4 = ((double)(int)vidConfig.displayHeight * (sunTransformedPosition_4 / sunTransformedPosition_12 + 1.0)
        - (double)heightInPixels)
        * 0.5;
    top = (int)(v4 + 9.313225746154785e-10);
    right = widthInPixels + left;
    bottom = heightInPixels + top;
    if (left < 0)
        left = 0;
    if (right > (int)vidConfig.displayWidth)
        right = vidConfig.displayWidth;
    if (top < 0)
        top = 0;
    if (bottom > (int)vidConfig.displayHeight)
        bottom = vidConfig.displayHeight;
    if (right <= left || bottom <= top)
        return 0.0;
    return (float)((double)((bottom - top) * (right - left)) / (double)(heightInPixels * widthInPixels));
}

void __cdecl RB_AddSunEffects(SunFlareDynamic *sunFlare)
{
    if (!rgp.world)
        MyAssertHandler(".\\rb_sky.cpp", 556, 0, "%s", "rgp.world");
    if (!sunFlare)
        MyAssertHandler(".\\rb_sky.cpp", 557, 0, "%s", "sunFlare");
    sunFlare->lastDot = Vec3Dot(rgp.world->sun.sunFxPosition, gfxCmdBufSourceState.viewParms3D->axis[0]);
    if (sunFlare->lastDot > 0.0)
        RB_DrawSunSprite();
}

void RB_DrawSunSprite()
{
    float v0; // [esp+18h] [ebp-50h]
    float perp[3]; // [esp+1Ch] [ebp-4Ch] BYREF
    float rightDown[3]; // [esp+28h] [ebp-40h] BYREF
    float right[3]; // [esp+34h] [ebp-34h] BYREF
    const float *sunPosition; // [esp+40h] [ebp-28h]
    float up[3]; // [esp+44h] [ebp-24h] BYREF
    float scale; // [esp+50h] [ebp-18h]
    GfxColor color; // [esp+54h] [ebp-14h]
    GfxVertex *vert; // [esp+58h] [ebp-10h]
    float rightUp[3]; // [esp+5Ch] [ebp-Ch] BYREF

    if (gfxCmdBufSourceState.viewMode != VIEW_MODE_IDENTITY)
        MyAssertHandler(".\\rb_sky.cpp", 406, 0, "%s", "gfxCmdBufSourceState.viewMode == VIEW_MODE_IDENTITY");
    if (!rgp.world)
        MyAssertHandler(".\\rb_sky.cpp", 408, 0, "%s", "rgp.world");
    sunPosition = rgp.world->sun.sunFxPosition;
    RB_SetTessTechnique(rgp.world->sun.spriteMaterial, TECHNIQUE_UNLIT);
    R_TrackPrims(&gfxCmdBufState, GFX_PRIM_STATS_FX);
    color.packed = -1;
    vert = RB_SetTessQuad((GfxColor)-1);
    scale = rgp.world->sun.spriteSize * 0.001311092986725271;
    if (sunPosition[2] * sunPosition[2] <= 0.9900000095367432)
    {
        v0 = -*sunPosition;
        perp[0] = sunPosition[1];
        perp[1] = v0;
    }
    else
    {
        perp[0] = 1.0;
        perp[1] = 0.0;
    }
    perp[2] = 0.0;
    Vec3Cross(sunPosition, perp, right);
    Vec3Normalize(right);
    Vec3Scale(right, scale, right);
    Vec3Cross(right, sunPosition, up);
    Vec3Add(right, up, rightUp);
    Vec3Sub(right, up, rightDown);
    Vec3Add(sunPosition, rightUp, vert->xyzw);
    vert->xyzw[3] = 0.0;
    Vec3Add(sunPosition, rightDown, vert[1].xyzw);
    vert[1].xyzw[3] = 0.0;
    Vec3Sub(sunPosition, rightUp, vert[2].xyzw);
    vert[2].xyzw[3] = 0.0;
    Vec3Sub(sunPosition, rightDown, vert[3].xyzw);
    vert[3].xyzw[3] = 0.0;
    RB_EndTessSurface();
}

void __cdecl RB_DrawSunPostEffects(unsigned int localClientNum)
{
    int frameTime; // [esp+0h] [ebp-8h]
    SunFlareDynamic *sunFlare; // [esp+4h] [ebp-4h]

    if (localClientNum >= 4)
        MyAssertHandler(
            ".\\rb_sky.cpp",
            618,
            0,
            "localClientNum doesn't index ARRAY_COUNT( sunFlareArray )\n\t%i not in [0, %i)",
            localClientNum,
            4);
    sunFlare = &sunFlareArray[localClientNum];
    if (sunFlare->lastTime && sunFlare->lastTime <= gfxCmdBufSourceState.sceneDef.time)
        frameTime = gfxCmdBufSourceState.sceneDef.time - sunFlare->lastTime;
    else
        frameTime = 10;
    sunFlare->lastTime = gfxCmdBufSourceState.sceneDef.time;
    if (r_drawSun->current.enabled)
    {
        if (!rgp.world)
            MyAssertHandler(".\\rb_sky.cpp", 634, 0, "%s", "rgp.world");
        if (rgp.world->sun.hasValidData)
        {
            RB_DrawSunFlare(sunFlare, frameTime);
            RB_DrawBlindAndGlare(sunFlare, frameTime);
        }
    }
}

void __cdecl RB_DrawSunFlare(SunFlareDynamic *sunFlare, int frameTime)
{
    float v2; // [esp+14h] [ebp-10h]
    float lerp; // [esp+18h] [ebp-Ch]
    float size; // [esp+1Ch] [ebp-8h]
    float alpha; // [esp+20h] [ebp-4h]

    if (!sunFlare)
        MyAssertHandler(".\\rb_sky.cpp", 470, 0, "%s", "sunFlare");
    if (rgp.world->sun.flareMaterial && rgp.world->sun.flareMinDot < (double)sunFlare->lastDot)
    {
        if (rgp.world->sun.flareMaxDot > (double)sunFlare->lastDot)
            lerp = (sunFlare->lastDot - rgp.world->sun.flareMinDot)
            / (rgp.world->sun.flareMaxDot - rgp.world->sun.flareMinDot);
        else
            lerp = 1.0;
        if (lerp < 0.0 || lerp > 1.0)
            MyAssertHandler(".\\rb_sky.cpp", 483, 1, "%s\n\t(lerp) = %g", "(lerp >= 0.0f && lerp <= 1.0f)", lerp);
        if (rgp.world->sun.flareMaxAlpha < 0.0)
            MyAssertHandler(
                ".\\rb_sky.cpp",
                484,
                1,
                "%s\n\t(rgp.world->sun.flareMaxAlpha) = %g",
                "(rgp.world->sun.flareMaxAlpha >= 0.0f)",
                rgp.world->sun.flareMaxAlpha);
        alpha = lerp * rgp.world->sun.flareMaxAlpha;
        size = lerp * rgp.world->sun.flareMaxSize + rgp.world->sun.flareMinSize;
        if (sunFlare->flareIntensity < 0.0)
            MyAssertHandler(
                ".\\rb_sky.cpp",
                489,
                0,
                "%s\n\t(sunFlare->flareIntensity) = %g",
                "(sunFlare->flareIntensity >= 0.0f)",
                sunFlare->flareIntensity);
        if (sunFlare->lastVisibility < 0.0)
            MyAssertHandler(
                ".\\rb_sky.cpp",
                490,
                0,
                "%s\n\t(sunFlare->lastVisibility) = %g",
                "(sunFlare->lastVisibility >= 0.0f)",
                sunFlare->lastVisibility);
        sunFlare->flareIntensity = R_UpdateOverTime(
            sunFlare->flareIntensity,
            sunFlare->lastVisibility,
            rgp.world->sun.flareFadeInTime,
            rgp.world->sun.flareFadeOutTime,
            frameTime);
        if (sunFlare->flareIntensity < 0.0)
            MyAssertHandler(
                ".\\rb_sky.cpp",
                494,
                0,
                "%s\n\t(sunFlare->flareIntensity) = %g",
                "(sunFlare->flareIntensity >= 0.0f)",
                sunFlare->flareIntensity);
        if (alpha < 0.0)
            MyAssertHandler(".\\rb_sky.cpp", 495, 0, "%s\n\t(alpha) = %g", "(alpha >= 0.0f)", alpha);
        v2 = sunFlare->flareIntensity * alpha;
        RB_DrawSunFlareCore(v2, size);
    }
}

double __cdecl R_UpdateOverTime(float fCurrent, float fGoal, int iFadeInTime, int iFadeOutTime, int frametime)
{
    if (frametime < 0)
        MyAssertHandler(".\\rb_sky.cpp", 120, 0, "%s\n\t(frametime) = %i", "(frametime >= 0)", frametime);
    if (iFadeInTime < 0)
        MyAssertHandler(".\\rb_sky.cpp", 121, 0, "%s\n\t(iFadeInTime) = %i", "(iFadeInTime >= 0)", iFadeInTime);
    if (iFadeOutTime < 0)
        MyAssertHandler(".\\rb_sky.cpp", 122, 0, "%s\n\t(iFadeOutTime) = %i", "(iFadeOutTime >= 0)", iFadeOutTime);
    if (fGoal <= (double)fCurrent)
    {
        if (fGoal < (double)fCurrent)
        {
            if (iFadeOutTime <= 0)
                return fGoal;
            fCurrent = fCurrent - (double)frametime * 1.0 / (double)iFadeOutTime;
            if (fGoal > (double)fCurrent)
                return fGoal;
        }
    }
    else
    {
        if (iFadeInTime <= 0)
            return fGoal;
        fCurrent = (double)frametime * 1.0 / (double)iFadeInTime + fCurrent;
        if (fGoal < (double)fCurrent)
            return fGoal;
    }
    return fCurrent;
}

void __cdecl RB_DrawSunFlareCore(float alpha, float sizeIn640x480)
{
    float widthInClipSpace; // [esp+Ch] [ebp-28h]
    float heightInClipSpace; // [esp+10h] [ebp-24h]
    float v4; // [esp+1Ch] [ebp-18h]

    RB_SetTessTechnique(rgp.world->sun.flareMaterial, TECHNIQUE_UNLIT);
    R_TrackPrims(&gfxCmdBufState, GFX_PRIM_STATS_FX);
    if (alpha < 0.0 || alpha > 1.0)
        MyAssertHandler(".\\rb_sky.cpp", 454, 0, "%s\n\t(alpha) = %g", "(alpha >= 0 && alpha <= 1)", alpha);
    v4 = alpha * 255.0;
    RB_SetIdentity();
    heightInClipSpace = sizeIn640x480 / 480.0;
    widthInClipSpace = sizeIn640x480 / 640.0;
    RB_TessSunBillboard(
        widthInClipSpace,
        heightInClipSpace,
        (GfxColor)((unsigned __int8)(int)(v4 + 9.313225746154785e-10)
            | ((unsigned __int8)(int)(v4 + 9.313225746154785e-10) << 8)
            | ((unsigned __int8)(int)(v4 + 9.313225746154785e-10) << 16)
            | 0xFF000000));
}

void __cdecl RB_DrawBlindAndGlare(SunFlareDynamic *sunFlare, int frameTime)
{
    float colorVec[4]; // [esp+14h] [ebp-1Ch] BYREF
    float blind; // [esp+24h] [ebp-Ch] BYREF
    float glare; // [esp+28h] [ebp-8h] BYREF
    GfxColor color; // [esp+2Ch] [ebp-4h] BYREF

    if (!sunFlare)
        MyAssertHandler(".\\rb_sky.cpp", 573, 0, "%s", "sunFlare");
    RB_CalcSunBlind(sunFlare, frameTime, &blind, &glare);
    colorVec[0] = glare;
    colorVec[1] = glare;
    colorVec[2] = glare;
    colorVec[3] = blind;
    R_ConvertColorToBytes(colorVec, (unsigned __int8 *)&color);
    RB_DrawFullScreenColoredQuad(rgp.glareBlindMaterial, 0.0, 0.0, 1.0, 1.0, color.packed);
}

void __cdecl RB_CalcSunBlind(SunFlareDynamic *sunFlare, int frameTime, float *blind, float *glare)
{
    float blindLerp; // [esp+14h] [ebp-Ch]
    float blindLerpa; // [esp+14h] [ebp-Ch]
    float glareLerp; // [esp+18h] [ebp-8h]
    float glareLerpa; // [esp+18h] [ebp-8h]
    float sunDot; // [esp+1Ch] [ebp-4h]

    if (!sunFlare)
        MyAssertHandler(".\\rb_sky.cpp", 507, 0, "%s", "sunFlare");
    if (!rgp.world)
        MyAssertHandler(".\\rb_sky.cpp", 508, 0, "%s", "rgp.world");
    sunDot = Vec3Dot(rgp.world->sun.sunFxPosition, gfxCmdBufSourceState.viewParms3D->axis[0]);
    if (rgp.world->sun.blindMaxDarken > 0.0)
    {
        if (rgp.world->sun.blindMinDot < (double)sunDot)
        {
            if (rgp.world->sun.blindMaxDot > (double)sunDot)
                blindLerp = (sunDot - rgp.world->sun.blindMinDot) / (rgp.world->sun.blindMaxDot - rgp.world->sun.blindMinDot);
            else
                blindLerp = 1.0;
        }
        else
        {
            blindLerp = 0.0;
        }
        if (blindLerp < 0.0)
            MyAssertHandler(".\\rb_sky.cpp", 524, 0, "%s\n\t(blindLerp) = %g", "(blindLerp >= 0)", blindLerp);
        if (blindLerp > 1.0)
            MyAssertHandler(".\\rb_sky.cpp", 525, 0, "%s\n\t(blindLerp) = %g", "(blindLerp <= 1)", blindLerp);
        blindLerpa = blindLerp * sunFlare->lastVisibility;
        if (blindLerpa < 0.0)
            MyAssertHandler(".\\rb_sky.cpp", 527, 0, "%s\n\t(blindLerp) = %g", "(blindLerp >= 0)", blindLerpa);
        if (blindLerpa > 1.0)
            MyAssertHandler(".\\rb_sky.cpp", 528, 0, "%s\n\t(blindLerp) = %g", "(blindLerp <= 1)", blindLerpa);
        sunFlare->currentBlind = R_UpdateOverTime(
            sunFlare->currentBlind,
            blindLerpa,
            rgp.world->sun.blindFadeInTime,
            rgp.world->sun.blindFadeOutTime,
            frameTime);
        if (sunFlare->currentBlind < 0.0)
            MyAssertHandler(
                ".\\rb_sky.cpp",
                530,
                0,
                "%s\n\t(sunFlare->currentBlind) = %g",
                "(sunFlare->currentBlind >= 0)",
                sunFlare->currentBlind);
        if (sunFlare->currentBlind > 1.0)
            MyAssertHandler(
                ".\\rb_sky.cpp",
                531,
                0,
                "%s\n\t(sunFlare->currentBlind) = %g",
                "(sunFlare->currentBlind <= 1)",
                sunFlare->currentBlind);
        *blind = sunFlare->currentBlind * rgp.world->sun.blindMaxDarken;
    }
    else
    {
        *blind = 0.0;
    }
    if (rgp.world->sun.glareMaxLighten > 0.0)
    {
        if (rgp.world->sun.glareMinDot < (double)sunDot)
        {
            if (rgp.world->sun.glareMaxDot > (double)sunDot)
                glareLerp = (sunDot - rgp.world->sun.glareMinDot) / (rgp.world->sun.glareMaxDot - rgp.world->sun.glareMinDot);
            else
                glareLerp = 1.0;
        }
        else
        {
            glareLerp = 0.0;
        }
        glareLerpa = glareLerp * sunFlare->lastVisibility;
        sunFlare->currentGlare = R_UpdateOverTime(
            sunFlare->currentGlare,
            glareLerpa,
            rgp.world->sun.glareFadeInTime,
            rgp.world->sun.glareFadeOutTime,
            frameTime);
        *glare = sunFlare->currentGlare * rgp.world->sun.glareMaxLighten;
    }
    else
    {
        *glare = 0.0;
    }
}

void __cdecl RB_AllocSunSpriteQueries()
{
    unsigned int viewIndex; // [esp+8h] [ebp-8h]
    unsigned int queryIndex; // [esp+Ch] [ebp-4h]

    for (viewIndex = 0; viewIndex < gfxCfg.maxClientViews; ++viewIndex)
    {
        for (queryIndex = 0; queryIndex < 2; ++queryIndex)
        {
            sunFlareArray[viewIndex].sunQuery[queryIndex] = RB_HW_AllocOcclusionQuery();
            sunFlareArray[viewIndex].sunQueryIssued[queryIndex] = 0;
        }
    }
}

