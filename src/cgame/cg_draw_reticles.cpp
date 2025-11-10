#include "cg_local.h"
#include "cg_public.h"

#include <database/database.h>

#include <client/client.h>
#include <universal/profile.h>

#ifdef KISAK_MP
#include <cgame_mp/cg_local_mp.h>
#include <client_mp/client_mp.h>
#elif KISAK_SP
#include <cgame/cg_main.h>
#endif

void __cdecl CG_CalcCrosshairPosition(const cg_s *cgameGlob, float *x, float *y)
{
    double v3; // st7
    double v4; // st7
    float gunYaw; // [esp+4h] [ebp-24h]
    float v6; // [esp+8h] [ebp-20h]
    float gunDir[3]; // [esp+Ch] [ebp-1Ch] BYREF
    float gunAng[3]; // [esp+18h] [ebp-10h] BYREF
    float dot; // [esp+24h] [ebp-4h]

    gunYaw = cgameGlob->gunYaw;
    v6 = cgameGlob->refdefViewAngles[2];
    gunAng[0] = cgameGlob->gunPitch;
    gunAng[1] = gunYaw;
    gunAng[2] = v6;
    AngleVectors(gunAng, gunDir, 0, 0);
    dot = Vec3Dot(cgameGlob->refdef.viewaxis[0], gunDir);
    if (dot > 0.0 && cgameGlob->refdef.tanHalfFovX > 0.0)
    {
        v3 = Vec3Dot(cgameGlob->refdef.viewaxis[1], gunDir);
        *x = v3 / (dot * cgameGlob->refdef.tanHalfFovX) * -320.0;
        v4 = Vec3Dot(cgameGlob->refdef.viewaxis[2], gunDir);
        *y = v4 / (dot * cgameGlob->refdef.tanHalfFovY) * -240.0;
    }
    else
    {
        *x = 0.0;
        *y = 0.0;
    }
}

char __cdecl CG_GetWeapReticleZoom(const cg_s *cgameGlob, float *zoom)
{
    int32_t weapIndex; // [esp+0h] [ebp-Ch]
    float fPosLerp; // [esp+4h] [ebp-8h]
    WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    weapIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
    weapDef = BG_GetWeaponDef(weapIndex);
    fPosLerp = cgameGlob->predictedPlayerState.fWeaponPosFrac;
    *zoom = 0.0;
    if (!weapDef)
        MyAssertHandler(".\\cgame\\cg_draw_reticles.cpp", 57, 0, "%s", "weapDef");
    if (!weapDef->overlayMaterial && weapDef->overlayReticle == WEAPOVERLAYRETICLE_NONE)
        return 0;
    if (fPosLerp == 0.0)
        return 0;
    if (cgameGlob->playerEntity.bPositionToADS)
    {
        *zoom = fPosLerp - (1.0 - weapDef->fAdsZoomInFrac);
        if (*zoom > 0.0)
            *zoom = *zoom / weapDef->fAdsZoomInFrac;
    }
    else
    {
        *zoom = fPosLerp - (1.0 - weapDef->fAdsZoomOutFrac);
        if (*zoom > 0.0)
            *zoom = *zoom / weapDef->fAdsZoomOutFrac;
    }
    if (*zoom <= 0.009999999776482582)
        return 0;
    if (*zoom > 1.0)
        *zoom = 1.0;
    return 1;
}

void __cdecl CG_DrawNightVisionOverlay(int32_t localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (CG_LookingThroughNightVision(localClientNum))
    {
        if (cgMedia.nightVisionOverlay)
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                0.0,
                0.0,
                640.0,
                480.0,
                4,
                4,
                0.0,
                0.0,
                1.0,
                1.0,
                colorWhite,
                cgMedia.nightVisionOverlay);
        else
            Com_PrintWarning(14, "CG_DrawNightVisionOverlay(): Nightvision Assets not Precached.\n");
    }
}

void __cdecl CG_DrawCrosshair(int32_t localClientNum)
{
    WeaponDef *weapDefTurret; // [esp+Ch] [ebp-44h]
    float posLerp; // [esp+10h] [ebp-40h]
    bool drawHud; // [esp+1Bh] [ebp-35h]
    float reticleAlpha; // [esp+20h] [ebp-30h]
    float centerY; // [esp+24h] [ebp-2Ch] BYREF
    int32_t weapIndex; // [esp+28h] [ebp-28h]
    const playerState_s *ps; // [esp+2Ch] [ebp-24h]
    WeaponDef *weapDef; // [esp+30h] [ebp-20h]
    float color[4]; // [esp+34h] [ebp-1Ch] BYREF
    float transShift; // [esp+44h] [ebp-Ch] BYREF
    float transScale; // [esp+48h] [ebp-8h] BYREF
    float centerX; // [esp+4Ch] [ebp-4h] BYREF
    const cg_s *cgameGlob;

    PROF_SCOPED("CG_DrawCrosshair");

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    ps = &cgameGlob->predictedPlayerState;

    iassert(cg_drawGun);
    iassert(cg_crosshairDynamic);

#ifdef KISAK_MP
    if (!cgameGlob->renderingThirdPerson)
#endif
    {
#ifdef KISAK_MP
        drawHud = CG_ShouldDrawHud(localClientNum);
#elif KISAK_SP
        iassert(cg_paused);
        drawHud = (cg_paused->current.integer == 0);
#endif
        posLerp = ps->fWeaponPosFrac;
        transScale = 1.0;
        transShift = 0.0;
        if ((ps->eFlags & 0x300) != 0)
        {
            weapIndex = CG_PlayerTurretWeaponIdx(localClientNum);
            if (weapIndex && (weapDefTurret = BG_GetWeaponDef(weapIndex), weapDefTurret->overlayMaterial))
            {
                CG_DrawAdsOverlay(localClientNum, weapDefTurret, colorWhite, vec2_origin);
            }
            else if (!CG_Flashbanged(localClientNum) && drawHud && ps->viewlocked_entNum != ENTITYNUM_NONE)
            {
                CG_DrawTurretCrossHair(localClientNum);
            }
        }
        else
        {
            weapIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
            if (weapIndex)
            {
                weapDef = BG_GetWeaponDef(weapIndex);
                reticleAlpha = CG_DrawWeapReticle(localClientNum);
                if (!CG_Flashbanged(localClientNum) && drawHud)
                {
                    if (localClientNum)
                        MyAssertHandler(
                            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                            1083,
                            0,
                            "%s\n\t(localClientNum) = %i",
                            "(localClientNum == 0)",
                            localClientNum);
                    CG_CalcCrosshairColor(localClientNum, reticleAlpha, color);
                    if (color[3] >= 0.009999999776482582
                        && (posLerp != 1.0 || !cg_drawGun->current.enabled)
                        && AllowedToDrawCrosshair(localClientNum, &cgameGlob->predictedPlayerState))
                    {
                        CG_CalcCrosshairPosition(cgameGlob, &centerX, &centerY);
                        if (posLerp != 0.0)
                        {
                            CG_TransitionToAds(cgameGlob, weapDef, posLerp, &transScale, &transShift);
                            CG_DrawAdsAimIndicator(localClientNum, weapDef, color, centerX, centerY, transScale);
                        }
                        if (posLerp != 1.0 || cg_drawGun->current.enabled)
                        {
                            if (!cg_crosshairDynamic->current.enabled)
                            {
                                centerX = 0.0;
                                centerY = transShift;
                            }
                            CG_DrawReticleCenter(localClientNum, weapDef, color, centerX, centerY, transScale);
                            CG_DrawReticleSides(localClientNum, weapDef, color, centerX, centerY, transScale);
                        }
                    }
                }
            }
        }
    }
}

void __cdecl CG_DrawAdsOverlay(
    int32_t localClientNum,
    const WeaponDef *weapDef,
    const float *color,
    const float *crosshairPos)
{
    float v4; // [esp+30h] [ebp-34h]
    float v5; // [esp+34h] [ebp-30h]
    float v6; // [esp+38h] [ebp-2Ch]
    float v7; // [esp+3Ch] [ebp-28h]
    ScreenPlacement *scrPlace; // [esp+40h] [ebp-24h]
    float drawPos[2]; // [esp+44h] [ebp-20h] BYREF
    Material *material; // [esp+4Ch] [ebp-18h]
    cg_s *cgameGlob; // [esp+50h] [ebp-14h]
    int32_t vertAlign; // [esp+54h] [ebp-10h]
    float drawSize[2]; // [esp+58h] [ebp-Ch] BYREF
    int32_t horzAlign; // [esp+60h] [ebp-4h]

    iassert(weapDef);

    if (CG_UsingLowResViewPort(localClientNum) && weapDef->overlayMaterialLowRes)
        material = weapDef->overlayMaterialLowRes;
    else
        material = weapDef->overlayMaterial;

    if (material)
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        horzAlign = 2;
        vertAlign = 2;
        drawSize[0] = weapDef->overlayWidth;
        drawSize[1] = weapDef->overlayHeight;
        scrPlace = &scrPlaceView[localClientNum];
        if (drawSize[0] <= 320.0 && drawSize[1] <= 240.0)
        {
            drawPos[0] = *crosshairPos - drawSize[0];
            drawPos[1] = crosshairPos[1] - drawSize[1];
            CL_DrawStretchPic(
                scrPlace,
                drawPos[0],
                drawPos[1],
                drawSize[0],
                drawSize[1],
                horzAlign,
                vertAlign,
                0.0,
                0.0,
                1.0,
                1.0,
                color,
                material);
            drawPos[0] = *crosshairPos;
            drawPos[1] = crosshairPos[1] - drawSize[1];
            CL_DrawStretchPic(
                scrPlace,
                drawPos[0],
                drawPos[1],
                drawSize[0],
                drawSize[1],
                horzAlign,
                vertAlign,
                1.0,
                0.0,
                0.0,
                1.0,
                color,
                material);
            drawPos[0] = *crosshairPos - drawSize[0];
            drawPos[1] = crosshairPos[1];
            CL_DrawStretchPic(
                scrPlace,
                drawPos[0],
                drawPos[1],
                drawSize[0],
                drawSize[1],
                horzAlign,
                vertAlign,
                0.0,
                1.0,
                1.0,
                0.0,
                color,
                material);
            drawPos[0] = *crosshairPos;
            drawPos[1] = crosshairPos[1];
            CL_DrawStretchPic(
                scrPlace,
                drawPos[0],
                drawPos[1],
                drawSize[0],
                drawSize[1],
                horzAlign,
                vertAlign,
                1.0,
                1.0,
                0.0,
                0.0,
                color,
                material);
            drawPos[0] = *crosshairPos - drawSize[0];
            drawPos[1] = crosshairPos[1] - drawSize[1];
            ScrPlace_ApplyRect(scrPlace, drawPos, &drawPos[1], drawSize, &drawSize[1], horzAlign, vertAlign);
            v5 = drawSize[1] + drawSize[1] + drawPos[1];
            v4 = drawSize[0] + drawSize[0] + drawPos[0];
            CG_DrawFrameOverlay(drawPos[0], v4, drawPos[1], v5, color, material);
        }
        else
        {
            drawPos[0] = *crosshairPos - drawSize[0] * 0.5;
            drawPos[1] = crosshairPos[1] - drawSize[1] * 0.5;
            if (!cg_debug_overlay_viewport->current.enabled)
                CL_DrawStretchPic(
                    scrPlace,
                    drawPos[0],
                    drawPos[1],
                    drawSize[0],
                    drawSize[1],
                    horzAlign,
                    vertAlign,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    color,
                    material);
            ScrPlace_ApplyRect(scrPlace, drawPos, &drawPos[1], drawSize, &drawSize[1], horzAlign, vertAlign);
            if (!cg_debug_overlay_viewport->current.enabled)
            {
                v7 = drawPos[1] + drawSize[1];
                v6 = drawPos[0] + drawSize[0];
                CG_DrawFrameOverlay(drawPos[0], v6, drawPos[1], v7, color, material);
            }
            if (color[3] > 0.9900000095367432)
                CG_UpdateScissorViewport(&cgameGlob->refdef, drawPos, drawSize);
        }
    }
}

void __cdecl CG_DrawFrameOverlay(
    float innerLeft,
    float innerRight,
    float innerTop,
    float innerBottom,
    const float *color,
    Material *material)
{
    float v6; // [esp+28h] [ebp-24h]
    float h; // [esp+2Ch] [ebp-20h]
    float v8; // [esp+30h] [ebp-1Ch]
    float w; // [esp+34h] [ebp-18h]
    float screenWidth; // [esp+38h] [ebp-14h]
    int32_t displayHeight; // [esp+3Ch] [ebp-10h] BYREF
    float displayAspect; // [esp+40h] [ebp-Ch] BYREF
    int32_t displayWidth; // [esp+44h] [ebp-8h] BYREF
    float screenHeight; // [esp+48h] [ebp-4h]

    CL_GetScreenDimensions(&displayWidth, &displayHeight, &displayAspect);
    screenWidth = (float)displayWidth;
    screenHeight = (float)displayHeight;
    if (innerLeft > 0.0)
        CL_DrawStretchPicPhysical(0.0, 0.0, innerLeft, screenHeight, 0.0, 0.0, 0.0, 1.0, color, material);
    if (screenWidth > (double)innerRight)
    {
        w = screenWidth - innerRight;
        CL_DrawStretchPicPhysical(innerRight, 0.0, w, screenHeight, 0.0, 0.0, 0.0, 1.0, color, material);
    }
    if (innerTop > 0.0)
    {
        v8 = innerRight - innerLeft;
        CL_DrawStretchPicPhysical(innerLeft, 0.0, v8, innerTop, 0.0, 0.0, 1.0, 0.0, color, material);
    }
    if (screenHeight > (double)innerBottom)
    {
        h = screenHeight - innerBottom;
        v6 = innerRight - innerLeft;
        CL_DrawStretchPicPhysical(innerLeft, innerBottom, v6, h, 0.0, 0.0, 1.0, 0.0, color, material);
    }
}

bool __cdecl CG_UsingLowResViewPort(int32_t localClientNum)
{
    return scrPlaceView[localClientNum].realViewportSize[1] <= 480.0;
}

void __cdecl CG_UpdateScissorViewport(refdef_s *refdef, float *drawPos, float *drawSize)
{
    uint32_t v3; // [esp+0h] [ebp-54h]
    uint32_t v4; // [esp+4h] [ebp-50h]
    uint32_t v5; // [esp+8h] [ebp-4Ch]
    uint32_t v6; // [esp+Ch] [ebp-48h]
    int32_t v7; // [esp+18h] [ebp-3Ch]
    int32_t v8; // [esp+24h] [ebp-30h]
    int32_t y; // [esp+34h] [ebp-20h]
    int32_t x; // [esp+44h] [ebp-10h]
    int32_t x1; // [esp+48h] [ebp-Ch]
    int32_t y1; // [esp+4Ch] [ebp-8h]

    if (!refdef)
        MyAssertHandler(".\\cgame\\cg_draw_reticles.cpp", 161, 0, "%s", "refdef");
    if (refdef->useScissorViewport)
        MyAssertHandler(".\\cgame\\cg_draw_reticles.cpp", 162, 0, "%s", "!refdef->useScissorViewport");
    refdef->useScissorViewport = 1;
    refdef->scissorViewport.x = refdef->x + SnapFloatToInt(drawPos[0]);
    refdef->scissorViewport.y = refdef->y + SnapFloatToInt(drawPos[1]);
    refdef->scissorViewport.width = SnapFloatToInt(drawSize[0]);
    refdef->scissorViewport.height = SnapFloatToInt(drawSize[1]);
    x1 = refdef->scissorViewport.width + refdef->scissorViewport.x;
    y1 = refdef->scissorViewport.height + refdef->scissorViewport.y;
    if (refdef->scissorViewport.x < (int32_t)(refdef->width + refdef->x))
        x = refdef->scissorViewport.x;
    else
        x = refdef->width + refdef->x;
    if ((int32_t)refdef->x < x)
        v6 = x;
    else
        v6 = refdef->x;
    refdef->scissorViewport.x = v6;
    if (refdef->scissorViewport.y < (int32_t)(refdef->height + refdef->y))
        y = refdef->scissorViewport.y;
    else
        y = refdef->height + refdef->y;
    if ((int32_t)refdef->y < y)
        v5 = y;
    else
        v5 = refdef->y;
    refdef->scissorViewport.y = v5;
    if (x1 < (int32_t)(refdef->width + refdef->x))
        v8 = x1;
    else
        v8 = refdef->width + refdef->x;
    if ((int32_t)refdef->x < v8)
        v4 = v8;
    else
        v4 = refdef->x;
    if (y1 < (int32_t)(refdef->height + refdef->y))
        v7 = y1;
    else
        v7 = refdef->height + refdef->y;
    if ((int32_t)refdef->y < v7)
        v3 = v7;
    else
        v3 = refdef->y;
    refdef->scissorViewport.width = v4 - refdef->scissorViewport.x;
    refdef->scissorViewport.height = v3 - refdef->scissorViewport.y;
}

double __cdecl CG_DrawWeapReticle(int32_t localClientNum)
{
    int32_t weapIndex; // [esp+14h] [ebp-28h]
    float crossHairAlpha; // [esp+18h] [ebp-24h]
    WeaponDef *weapDef; // [esp+1Ch] [ebp-20h]
    float color[4]; // [esp+20h] [ebp-1Ch] BYREF
    float zoomFrac; // [esp+30h] [ebp-Ch] BYREF
    float crosshairPos[2]; // [esp+34h] [ebp-8h] BYREF
    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (CG_GetWeapReticleZoom(cgameGlob, &zoomFrac))
    {
        weapIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
        weapDef = BG_GetWeaponDef(weapIndex);
        color[0] = 1.0f;
        color[1] = 1.0f;
        color[2] = 1.0f;
        color[3] = zoomFrac;
        CG_CalcCrosshairPosition(cgameGlob, crosshairPos, &crosshairPos[1]);
        CG_DrawAdsOverlay(localClientNum, weapDef, color, crosshairPos);
        crossHairAlpha = 1.0f - zoomFrac;
    }
    else
    {
        crossHairAlpha = 1.0f;
    }

    iassert(crossHairAlpha >= 0 && crossHairAlpha <= 1);
    return crossHairAlpha;
}

void __cdecl CG_CalcCrosshairColor(int32_t localClientNum, float alpha, float *color)
{
    WeaponDef *weapDef; // [esp+14h] [ebp-4h]
    cg_s *cgameGlob;
    cgs_t *cgsGlob;

    iassert(cg_crosshairAlpha);
    iassert(cg_crosshairEnemyColor);
    iassert(alpha >= 0.0f && alpha <= 1.0f);

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    //cgsGlob = CG_GetLocalClientStaticGlobals(localClientNum);

    weapDef = BG_GetWeaponDef(cgameGlob->predictedPlayerState.weapon);

    iassert(weapDef);

    if (!weapDef->crosshairColorChange)
    {
        goto LABEL_21;
    }

    if ((cgameGlob->predictedPlayerState.weapFlags & 8) == 0)
    {
        if ((cgameGlob->predictedPlayerState.weapFlags & 0x10) != 0
#ifdef KISAK_MP
            && cg_crosshairEnemyColor->current.enabled
            && cgameGlob->crosshairClientLastTime - cgameGlob->crosshairClientStartTime >= cg_enemyNameFadeIn->current.integer
#endif
            )
        {
#ifdef KISAK_MP
            Dvar_GetUnpackedColorByName("g_TeamColor_EnemyTeam", color);
#endif
            goto LABEL_22;
        }
    LABEL_21:
        color[0] = 1.0f;
        color[1] = 1.0f;
        color[2] = 1.0f;
        goto LABEL_22;
    }

#ifdef KISAK_MP
    if (cgameGlob->crosshairClientLastTime - cgameGlob->crosshairClientStartTime < cg_friendlyNameFadeIn->current.integer)
        goto LABEL_21;
    Dvar_GetUnpackedColorByName("g_TeamColor_MyTeam", color);
#endif
LABEL_22:
    color[3] = alpha * cg_crosshairAlpha->current.value;
}

void __cdecl CG_DrawTurretCrossHair(int32_t localClientNum)
{
    centity_s *cent; // [esp+38h] [ebp-38h]
    float drawSize; // [esp+44h] [ebp-2Ch]
    uint32_t weapIndex; // [esp+50h] [ebp-20h]
    WeaponDef *weapDef; // [esp+54h] [ebp-1Ch]
    float reticleColor[4]; // [esp+58h] [ebp-18h] BYREF
    float x; // [esp+68h] [ebp-8h]
    const cg_s *cgameGlob;

    iassert(cg_drawTurretCrosshair);
    iassert(cg_paused);
    iassert(cg_drawpaused);
    iassert(cg_crosshairAlpha);

    if (cg_drawTurretCrosshair->current.enabled && (!cg_paused->current.integer || !cg_drawpaused->current.enabled))
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);

        iassert(cgameGlob->predictedPlayerState.viewlocked_entNum != ENTITYNUM_NONE);

        cent = CG_GetEntity(localClientNum, cgameGlob->predictedPlayerState.viewlocked_entNum);
        iassert(cent->nextState.eType == ET_MG42);
        weapIndex = cent->nextState.weapon;
        if (weapIndex)
        {
            weapDef = BG_GetWeaponDef(weapIndex);
            iassert(weapDef->weapClass == WEAPCLASS_TURRET);
            if (weapDef->reticleCenter)
            {
                if (cg_crosshairAlpha->current.value >= 0.009999999776482582)
                {
                    CG_CalcCrosshairColor(localClientNum, 1.0f, reticleColor);
                    drawSize = (float)weapDef->iReticleCenterSize;
                    x = drawSize * -0.5f;
                    CL_DrawStretchPic(
                        &scrPlaceView[localClientNum],
                        x,
                        x,
                        drawSize,
                        drawSize,
                        2,
                        2,
                        0.0f,
                        0.0f,
                        1.0f,
                        1.0f,
                        reticleColor,
                        weapDef->reticleCenter);
                }
            }
        }
    }
}

char __cdecl AllowedToDrawCrosshair(int32_t localClientNum, const playerState_s *predictedPlayerState)
{
    if (!predictedPlayerState)
        MyAssertHandler(".\\cgame\\cg_draw_reticles.cpp", 487, 0, "%s", "predictedPlayerState");
    if (!cg_paused)
        MyAssertHandler(".\\cgame\\cg_draw_reticles.cpp", 488, 0, "%s", "cg_paused");
    if (cg_paused->current.integer && cg_drawpaused->current.enabled)
        return 0;
    if (CG_IsReticleTurnedOff())
        return 0;
    switch (predictedPlayerState->weaponstate)
    {
    case 0xC:
    case 0xD:
    case 0xE:
        return 0;
    case 7:
        return 0;
    case 1:
    case 2:
    case 3:
    case 4:
        return 0;
    }
    return 1;
}

bool __cdecl CG_IsReticleTurnedOff()
{
#ifdef KISAK_MP
    return !cg_drawCrosshair->current.enabled || !UI_ShouldDrawCrosshair();
#elif KISAK_SP
    return !cg_drawCrosshair->current.enabled;
#endif
}

void __cdecl CG_DrawAdsAimIndicator(
    int32_t localClientNum,
    const WeaponDef *weapDef,
    const float *color,
    float centerX,
    float centerY,
    float transScale)
{
    Material *material; // [esp+30h] [ebp-1Ch]
    float x; // [esp+3Ch] [ebp-10h]
    float y; // [esp+40h] [ebp-Ch]
    float w; // [esp+48h] [ebp-4h]
    float wa; // [esp+48h] [ebp-4h]

    if (!weapDef)
        MyAssertHandler(".\\cgame\\cg_draw_reticles.cpp", 526, 0, "%s", "weapDef");
    if (!cg_drawGun)
        MyAssertHandler(".\\cgame\\cg_draw_reticles.cpp", 527, 0, "%s", "cg_drawGun");
    if (!cg_drawGun->current.enabled && transScale < 1.0)
    {
        material = weapDef->reticleCenter;
        if (material)
        {
            w = (float)weapDef->iReticleCenterSize;
            wa = (1.5 - transScale) * w;
            x = centerX - wa * 0.5;
            y = centerY - wa * 0.5;
            CL_DrawStretchPic(&scrPlaceView[localClientNum], x, y, wa, wa, 2, 2, 0.0, 0.0, 1.0, 1.0, color, material);
        }
    }
}

void __cdecl CG_TransitionToAds(
    const cg_s *cgameGlob,
    const WeaponDef *weapDef,
    float posLerp,
    float *transScale,
    float *transShift)
{
    float v5; // [esp+8h] [ebp-10h]
    float v6; // [esp+10h] [ebp-8h]
    float f; // [esp+14h] [ebp-4h]
    float fa; // [esp+14h] [ebp-4h]
    float fb; // [esp+14h] [ebp-4h]

    if (!cgameGlob)
        MyAssertHandler(".\\cgame\\cg_draw_reticles.cpp", 560, 0, "%s", "cgameGlob");
    if (!weapDef)
        MyAssertHandler(".\\cgame\\cg_draw_reticles.cpp", 561, 0, "%s", "weapDef");
    if (!transScale)
        MyAssertHandler(".\\cgame\\cg_draw_reticles.cpp", 562, 0, "%s", "transScale");
    if (!transShift)
        MyAssertHandler(".\\cgame\\cg_draw_reticles.cpp", 563, 0, "%s", "transShift");
    if (cgameGlob->playerEntity.bPositionToADS)
    {
        f = posLerp - (1.0 - weapDef->fAdsCrosshairInFrac);
        if (f <= 0.0)
            return;
        if (weapDef->fAdsCrosshairInFrac == 0.0)
            MyAssertHandler(
                ".\\cgame\\cg_draw_reticles.cpp",
                573,
                0,
                "%s\n\t(weapDef->fAdsCrosshairInFrac) = %g",
                "(weapDef->fAdsCrosshairInFrac != 0.0f)",
                weapDef->fAdsCrosshairInFrac);
        fa = f / weapDef->fAdsCrosshairInFrac;
    }
    else
    {
        fb = posLerp - (1.0 - weapDef->fAdsCrosshairOutFrac);
        if (fb <= 0.0)
            return;
        if (weapDef->fAdsCrosshairOutFrac == 0.0)
            MyAssertHandler(
                ".\\cgame\\cg_draw_reticles.cpp",
                582,
                0,
                "%s\n\t(weapDef->fAdsCrosshairOutFrac) = %g",
                "(weapDef->fAdsCrosshairOutFrac != 0.0f)",
                weapDef->fAdsCrosshairOutFrac);
        fa = fb / weapDef->fAdsCrosshairOutFrac;
    }
    if (cgameGlob->refdef.tanHalfFovY == 0.0)
        MyAssertHandler(
            ".\\cgame\\cg_draw_reticles.cpp",
            586,
            0,
            "%s\n\t(cgameGlob->refdef.tanHalfFovY) = %g",
            "(cgameGlob->refdef.tanHalfFovY != 0.0f)",
            cgameGlob->refdef.tanHalfFovY);
    *transScale = 1.0 - fa * 0.5;
    v6 = weapDef->fAdsAimPitch * 0.01745329238474369;
    v5 = tan(v6);
    *transShift = fa * 240.0 / cgameGlob->refdef.tanHalfFovY * v5;
}

void __cdecl CG_DrawReticleCenter(
    int32_t localClientNum,
    const WeaponDef *weapDef,
    const float *color,
    float centerX,
    float centerY,
    float transScale)
{
    Material *material; // [esp+34h] [ebp-20h]
    float drawSize; // [esp+40h] [ebp-14h]
    float drawSizea; // [esp+40h] [ebp-14h]
    float x; // [esp+4Ch] [ebp-8h]
    float y; // [esp+50h] [ebp-4h]
    int32_t grenadeTimeLeft;

    iassert(weapDef);

    material = weapDef->reticleCenter;
    if (material)
    {
        drawSize = (float)weapDef->iReticleCenterSize;
        if (weapDef->weapType == WEAPTYPE_GRENADE && weapDef->bCookOffHold)
        {
            grenadeTimeLeft = CG_GetLocalClientGlobals(localClientNum)->predictedPlayerState.grenadeTimeLeft;
            if (grenadeTimeLeft)
                drawSize = (float)((float)(grenadeTimeLeft % 1000) / 100.0f) + drawSize;
        }

        drawSizea = drawSize * transScale;
        x = centerX - drawSizea * 0.5f;
        y = centerY - drawSizea * 0.5f;

        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            x,
            y,
            drawSizea,
            drawSizea,
            2,
            2,
            0.0,
            0.0,
            1.0,
            1.0,
            color,
            material);
    }
}

void __cdecl CG_DrawReticleSides(
    int32_t localClientNum,
    const WeaponDef *weapDef,
    const float *baseColor,
    float centerX,
    float centerY,
    float transScale)
{
    float spread[2]; // [esp+24h] [ebp-48h] BYREF
    const ScreenPlacement *scrPlace; // [esp+2Ch] [ebp-40h]
    const cg_s *cgameGlob; // [esp+30h] [ebp-3Ch]
    Material *material; // [esp+34h] [ebp-38h]
    float drawPos[2]; // [esp+38h] [ebp-34h]
    int32_t vertAlign; // [esp+40h] [ebp-2Ch]
    float imageTexelOffset[2]; // [esp+44h] [ebp-28h] BYREF
    float reticleAlpha; // [esp+4Ch] [ebp-20h]
    float drawSize[2]; // [esp+50h] [ebp-1Ch] BYREF
    int32_t horzAlign; // [esp+58h] [ebp-14h]
    float reticleColor[4]; // [esp+5Ch] [ebp-10h] BYREF

    iassert(weapDef);

    material = weapDef->reticleSide;

    if (material)
    {
        horzAlign = 2;
        vertAlign = 2;

        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        reticleAlpha = CG_DrawWeapReticle(localClientNum);
        CG_CalcReticleColor(baseColor, reticleAlpha, cgameGlob->predictedPlayerState.aimSpreadScale, reticleColor);
        if (reticleColor[3] >= 0.009999999776482582)
        {
            drawSize[0] = (double)weapDef->iReticleSideSize * transScale;
            drawSize[1] = drawSize[0];
            CG_CalcReticleSpread(cgameGlob, weapDef, drawSize, transScale, spread);
            scrPlace = &scrPlaceView[localClientNum];
            CG_CalcReticleImageOffset(drawSize, imageTexelOffset);
            drawPos[0] = centerX - drawSize[0] * 0.5;
            drawPos[1] = centerY - drawSize[1] - spread[1] - imageTexelOffset[1];
            CG_DrawRotatedPic(
                scrPlace,
                drawPos[0],
                drawPos[1],
                drawSize[0],
                drawSize[1],
                horzAlign,
                vertAlign,
                0.0,
                reticleColor,
                material);
            drawPos[0] = centerX + spread[0];
            drawPos[1] = centerY - drawSize[1] * 0.5;
            CG_DrawRotatedPic(
                scrPlace,
                drawPos[0],
                drawPos[1],
                drawSize[0],
                drawSize[1],
                horzAlign,
                vertAlign,
                90.0,
                reticleColor,
                material);
            drawPos[0] = centerX - drawSize[0] * 0.5 - imageTexelOffset[0];
            drawPos[1] = centerY + spread[1];
            CG_DrawRotatedPic(
                scrPlace,
                drawPos[0],
                drawPos[1],
                drawSize[0],
                drawSize[1],
                horzAlign,
                vertAlign,
                180.0,
                reticleColor,
                material);
            drawPos[0] = centerX - drawSize[0] - spread[0] - imageTexelOffset[0];
            drawPos[1] = centerY - drawSize[1] * 0.5 - imageTexelOffset[1];
            CG_DrawRotatedPic(
                scrPlace,
                drawPos[0],
                drawPos[1],
                drawSize[0],
                drawSize[1],
                horzAlign,
                vertAlign,
                270.0,
                reticleColor,
                material);
        }
    }
}

void __cdecl CG_CalcReticleSpread(
    const cg_s *cgameGlob,
    const WeaponDef *weapDef,
    const float *drawSize,
    float transScale,
    float *spread)
{
    float v5; // [esp+0h] [ebp-18h]
    float v6; // [esp+8h] [ebp-10h]
    float maxSpread; // [esp+Ch] [ebp-Ch] BYREF
    float f; // [esp+10h] [ebp-8h] BYREF
    float scale; // [esp+14h] [ebp-4h]

    if (!weapDef)
        MyAssertHandler(".\\cgame\\cg_draw_reticles.cpp", 641, 0, "%s", "weapDef");
    BG_GetSpreadForWeapon(&cgameGlob->predictedPlayerState, weapDef, &f, &maxSpread);
    f = ((maxSpread - f) * (cgameGlob->predictedPlayerState.aimSpreadScale / 255.0) + f) * transScale;
    v6 = f * 0.01745329238474369;
    v5 = tan(v6);
    scale = v5 * 240.0 / cgameGlob->refdef.tanHalfFovY;
    if (scale < (double)weapDef->iReticleMinOfs)
        scale = (float)weapDef->iReticleMinOfs;
    *spread = scale - weapDef->fHipReticleSidePos * *drawSize;
    spread[1] = scale - weapDef->fHipReticleSidePos * drawSize[1];
}

void __cdecl CG_CalcReticleColor(const float *baseColor, float alpha, float aimSpreadScale, float *reticleColor)
{
    if (!cg_crosshairAlpha)
        MyAssertHandler(".\\cgame\\cg_draw_reticles.cpp", 657, 0, "%s", "cg_crosshairAlpha");
    if (!cg_crosshairAlphaMin)
        MyAssertHandler(".\\cgame\\cg_draw_reticles.cpp", 658, 0, "%s", "cg_crosshairAlphaMin");
    if (alpha < 0.0 || alpha > 1.0)
        MyAssertHandler(
            ".\\cgame\\cg_draw_reticles.cpp",
            659,
            0,
            "%s\n\t(alpha) = %g",
            "(alpha >= 0 && alpha <= 1.0f)",
            alpha);
    *reticleColor = *baseColor;
    reticleColor[1] = baseColor[1];
    reticleColor[2] = baseColor[2];
    reticleColor[3] = alpha * cg_crosshairAlpha->current.value * (1.0 - aimSpreadScale / 255.0);
    if (cg_crosshairAlphaMin->current.value > (double)reticleColor[3])
        reticleColor[3] = cg_crosshairAlphaMin->current.value;
    if (reticleColor[3] < 0.0 || reticleColor[3] > 1.0)
        MyAssertHandler(
            ".\\cgame\\cg_draw_reticles.cpp",
            666,
            1,
            "%s\n\t(reticleColor[3]) = %g",
            "(reticleColor[3] >= 0 && reticleColor[3] <= 1.0f)",
            reticleColor[3]);
}

void __cdecl CG_CalcReticleImageOffset(const float *drawSize, float *imageTexelOffset)
{
    *imageTexelOffset = *drawSize / 8.0;
    imageTexelOffset[1] = drawSize[1] / 8.0;
}

