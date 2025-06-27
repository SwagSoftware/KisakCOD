#include "cg_local.h"
#include "cg_public.h"

#include <cgame_mp/cg_local_mp.h>
#include <ui/ui.h>
#include <client/client.h>
#include <client_mp/client_mp.h>
#include <gfx_d3d/r_rendercmds.h>

unsigned int g_hudGrenadeCount;
HudGrenade g_hudGrenades[32];

void __cdecl CG_DrawFlashDamage(const cg_s *cgameGlob)
{
    float y; // [esp+4h] [ebp-48h]
    float width; // [esp+14h] [ebp-38h]
    float height; // [esp+18h] [ebp-34h]
    float v4; // [esp+1Ch] [ebp-30h]
    float v5; // [esp+24h] [ebp-28h]
    int displayHeight; // [esp+28h] [ebp-24h] BYREF
    float sidebuffer; // [esp+2Ch] [ebp-20h]
    float displayAspect; // [esp+30h] [ebp-1Ch] BYREF
    int displayWidth; // [esp+34h] [ebp-18h] BYREF
    float redFlash; // [esp+38h] [ebp-14h]
    float col[4]; // [esp+3Ch] [ebp-10h] BYREF

    if (cgameGlob->nextSnap && cgameGlob->v_dmg_time > cgameGlob->time)
    {
        v5 = (float)(cgameGlob->v_dmg_time - cgameGlob->time) * cgameGlob->v_dmg_pitch / 500.0f;
        v4 = fabs(v5);
        redFlash = v4;
        if (v4 > 5.0f)
            redFlash = 5.0f;
        col[0] = 0.2f;
        col[1] = 0.0f;
        col[2] = 0.0f;
        col[3] = redFlash / 5.0f * 0.699999988079071f;
        sidebuffer = 10.0f;
        CL_GetScreenDimensions(&displayWidth, &displayHeight, &displayAspect);
        height = (double)displayHeight + sidebuffer;
        width = (double)displayWidth + sidebuffer;
        y = -sidebuffer;
        UI_FillRectPhysical(y, y, width, height, col);
    }
}

void __cdecl CG_DrawDamageDirectionIndicators(int localClientNum)
{
    float v1; // [esp+18h] [ebp-7Ch]
    float v2; // [esp+1Ch] [ebp-78h]
    float v3; // [esp+20h] [ebp-74h]
    float v4; // [esp+24h] [ebp-70h]
    int t; // [esp+34h] [ebp-60h]
    const ScreenPlacement *scrPlace; // [esp+38h] [ebp-5Ch]
    float xy[4][2]; // [esp+3Ch] [ebp-58h] BYREF
    const cg_s *cgameGlob; // [esp+5Ch] [ebp-38h]
    int slot; // [esp+60h] [ebp-34h]
    float halfWidth; // [esp+64h] [ebp-30h]
    float yaw; // [esp+68h] [ebp-2Ch]
    float height; // [esp+6Ch] [ebp-28h]
    float angle; // [esp+70h] [ebp-24h] BYREF
    float radius; // [esp+74h] [ebp-20h]
    int maxTime; // [esp+78h] [ebp-1Ch]
    float centerY; // [esp+7Ch] [ebp-18h] BYREF
    float color[4]; // [esp+80h] [ebp-14h] BYREF
    float centerX; // [esp+90h] [ebp-4h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgameGlob = cgArray;
    if (cgArray[0].nextSnap && !CG_Flashbanged(localClientNum))
    {
        scrPlace = &scrPlaceView[localClientNum];
        if (CG_GetWeapReticleZoom(cgameGlob, &angle))
        {
            if (!cg_hudDamageIconInScope->current.enabled)
                return;
            CG_CalcCrosshairPosition(cgameGlob, &centerX, &centerY);
            centerX = ScrPlace_ApplyX(scrPlace, centerX, 2);
            centerY = ScrPlace_ApplyY(scrPlace, centerY, 2);
        }
        else
        {
            centerX = ScrPlace_ApplyX(scrPlace, 0.0, 2);
            centerY = ScrPlace_ApplyY(scrPlace, 0.0, 2);
        }
        v3 = scrPlace->scaleVirtualToReal[0] * cg_hudDamageIconWidth->current.value;
        halfWidth = v3 * 0.5;
        height = scrPlace->scaleVirtualToReal[1] * cg_hudDamageIconHeight->current.value;
        radius = scrPlace->scaleVirtualToReal[1] * cg_hudDamageIconOffset->current.value;
        xy[0][0] = halfWidth;
        xy[0][1] = height + radius;
        xy[1][0] = -halfWidth;
        xy[1][1] = xy[0][1];
        xy[2][0] = xy[1][0];
        xy[2][1] = radius;
        xy[3][0] = halfWidth;
        xy[3][1] = radius;
        color[0] = 1.0;
        color[1] = 1.0;
        color[2] = 1.0;
        color[3] = 1.0;
        for (slot = 0; slot < 8; ++slot)
        {
            maxTime = cgameGlob->viewDamage[slot].duration;
            t = cgameGlob->time - cgameGlob->viewDamage[slot].time;
            if (t > 0 && t < maxTime)
            {
                yaw = vectoyaw(cgameGlob->refdef.viewaxis[0]);
                angle = yaw - cgameGlob->viewDamage[slot].yaw;
                v4 = 2.0 - ((double)t + (double)t) / (double)maxTime;
                v2 = v4 - 1.0;
                if (v2 < 0.0)
                    v1 = 2.0 - ((double)t + (double)t) / (double)maxTime;
                else
                    v1 = 1.0;
                color[3] = v1;
                CG_DrawRotatedQuadPic(scrPlace, centerX, centerY, xy, angle, color, cgMedia.damageMaterial);
            }
        }
    }
}

void __cdecl CG_ClearHudGrenades()
{
    g_hudGrenadeCount = 0;
}

char __cdecl CG_AddHudGrenade_PositionCheck(const cg_s *cgameGlob, const centity_s *grenadeEnt, WeaponDef *weapDef)
{
    float v4; // [esp+0h] [ebp-3Ch]
    float v5; // [esp+4h] [ebp-38h]
    float v6; // [esp+8h] [ebp-34h]
    float v7; // [esp+Ch] [ebp-30h]
    float heightToCeiling; // [esp+10h] [ebp-2Ch]
    float value; // [esp+14h] [ebp-28h]
    float iExplosionRadius; // [esp+18h] [ebp-24h]
    float grenadeOffset[3]; // [esp+1Ch] [ebp-20h] BYREF
    float maxOffsetSquared; // [esp+28h] [ebp-14h]
    float height; // [esp+2Ch] [ebp-10h]
    float maxHeight; // [esp+30h] [ebp-Ch]
    float grenadeOffsetSquared; // [esp+34h] [ebp-8h]
    float maxOffset; // [esp+38h] [ebp-4h]

    if (!grenadeEnt)
        MyAssertHandler(".\\cgame\\cg_draw_indicators.cpp", 206, 0, "%s", "grenadeEnt");
    Vec3Sub(grenadeEnt->pose.origin, cgameGlob->predictedPlayerState.origin, grenadeOffset);
    if (weapDef->projExplosion && weapDef->projExplosion != WEAPPROJEXP_HEAVY)
    {
        if (weapDef->projExplosion != WEAPPROJEXP_FLASHBANG)
            MyAssertHandler(
                ".\\cgame\\cg_draw_indicators.cpp",
                220,
                0,
                "%s",
                "weapDef->projExplosion == WEAPPROJEXP_FLASHBANG");
        maxOffset = cg_hudGrenadeIconMaxRangeFlash->current.value;
    }
    else if (cg_hudGrenadeIconMaxRangeFrag->current.value == 0.0)
    {
        maxOffset = 0.0;
    }
    else
    {
        value = cg_hudGrenadeIconMaxRangeFrag->current.value;
        iExplosionRadius = (float)weapDef->iExplosionRadius;
        v7 = value - iExplosionRadius;
        if (v7 < 0.0)
            v6 = (float)weapDef->iExplosionRadius;
        else
            v6 = value;
        maxOffset = v6;
    }
    maxOffsetSquared = maxOffset * maxOffset;
    grenadeOffsetSquared = Vec3LengthSq(grenadeOffset);
    if (maxOffsetSquared < (double)grenadeOffsetSquared)
        return 0;
    height = grenadeOffset[2];
    if (grenadeOffset[2] >= 0.0)
    {
        maxHeight = cg_hudGrenadeIconMaxHeight->current.value + cgameGlob->predictedPlayerState.viewHeightCurrent;
        heightToCeiling = cgameGlob->heightToCeiling;
        v5 = heightToCeiling - maxHeight;
        if (v5 < 0.0)
            v4 = heightToCeiling;
        else
            v4 = maxHeight;
        if (v4 < (double)height)
            return 0;
    }
    else if (height < -cg_hudGrenadeIconMaxHeight->current.value)
    {
        return 0;
    }
    return 1;
}

void __cdecl CG_AddHudGrenade(const cg_s *cgameGlob, const centity_s *grenadeEnt)
{
    HudGrenade *v2; // [esp+0h] [ebp-28h]
    Material *material; // [esp+1Ch] [ebp-Ch]
    float maxSpeedSq; // [esp+20h] [ebp-8h]
    WeaponDef *weapDef; // [esp+24h] [ebp-4h]

    if (grenadeEnt->nextState.eType != 4)
        MyAssertHandler(
            ".\\cgame\\cg_draw_indicators.cpp",
            262,
            0,
            "%s\n\t(state->eType) = %i",
            "(state->eType == ET_MISSILE)",
            grenadeEnt->nextState.eType);
    if ((COERCE_UNSIGNED_INT(grenadeEnt->nextState.lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(grenadeEnt->nextState.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(grenadeEnt->nextState.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\cgame\\cg_draw_indicators.cpp",
            264,
            0,
            "%s",
            "!IS_NAN((state->lerp.pos.trDelta)[0]) && !IS_NAN((state->lerp.pos.trDelta)[1]) && !IS_NAN((state->lerp.pos.trDelta)[2])");
    }
    weapDef = BG_GetWeaponDef(grenadeEnt->nextState.weapon);
    if (weapDef->weapType == WEAPTYPE_GRENADE
        && (weapDef->projExplosion == WEAPPROJEXP_GRENADE
            || weapDef->projExplosion == WEAPPROJEXP_FLASHBANG
            || weapDef->projExplosion == WEAPPROJEXP_HEAVY)
        && weapDef->timedDetonation
        && CG_AddHudGrenade_PositionCheck(cgameGlob, grenadeEnt, weapDef))
    {
        maxSpeedSq = weapDef->projExplosion == WEAPPROJEXP_GRENADE || weapDef->projExplosion == WEAPPROJEXP_HEAVY
            ? bg_maxGrenadeIndicatorSpeed->current.value * bg_maxGrenadeIndicatorSpeed->current.value
            : 1.0;
        if (maxSpeedSq >= Vec3LengthSq(grenadeEnt->nextState.lerp.pos.trDelta) && g_hudGrenadeCount < 0x20)
        {
            if (weapDef->projExplosion == WEAPPROJEXP_GRENADE || weapDef->projExplosion == WEAPPROJEXP_HEAVY)
            {
                if (cgameGlob->nextSnap->ps.cursorHint >= 5
                    && cgameGlob->nextSnap->ps.cursorHintEntIndex == grenadeEnt->nextState.number)
                {
                    material = cgMedia.grenadeIconThrowBack;
                }
                else
                {
                    material = cgMedia.grenadeIconFrag;
                }
                goto LABEL_29;
            }
            if (weapDef->projExplosion != WEAPPROJEXP_FLASHBANG)
                MyAssertHandler(
                    ".\\cgame\\cg_draw_indicators.cpp",
                    304,
                    0,
                    "%s\n\t(weapDef->projExplosion) = %i",
                    "(weapDef->projExplosion == WEAPPROJEXP_FLASHBANG)",
                    weapDef->projExplosion);
            if (cg_hudGrenadeIconEnabledFlash->current.enabled)
            {
                material = cgMedia.grenadeIconFlash;
            LABEL_29:
                v2 = &g_hudGrenades[g_hudGrenadeCount];
                v2->origin[0] = grenadeEnt->pose.origin[0];
                v2->origin[1] = grenadeEnt->pose.origin[1];
                v2->origin[2] = grenadeEnt->pose.origin[2];
                g_hudGrenades[g_hudGrenadeCount++].material = material;
            }
        }
    }
}

void __cdecl CG_DrawGrenadeIndicators(int localClientNum)
{
    float v1; // [esp+14h] [ebp-58h]
    float v2; // [esp+18h] [ebp-54h]
    float v3; // [esp+1Ch] [ebp-50h]
    float v4; // [esp+20h] [ebp-4Ch]
    float v5; // [esp+2Ch] [ebp-40h]
    float v6; // [esp+30h] [ebp-3Ch]
    float grenadeOffset[3]; // [esp+34h] [ebp-38h] BYREF
    unsigned int entityIndex; // [esp+40h] [ebp-2Ch]
    const cg_s *cgameGlob; // [esp+44h] [ebp-28h]
    float amplitude; // [esp+48h] [ebp-24h]
    float angle; // [esp+4Ch] [ebp-20h] BYREF
    float centerY; // [esp+50h] [ebp-1Ch] BYREF
    float bias; // [esp+54h] [ebp-18h]
    float color[4]; // [esp+58h] [ebp-14h] BYREF
    float centerX; // [esp+68h] [ebp-4h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    cgameGlob = cgArray;
    if (cgArray[0].nextSnap
        && cgameGlob->predictedPlayerState.pm_type != 4
        && (cgameGlob->predictedPlayerState.otherFlags & 2) == 0
        && g_hudGrenadeCount)
    {
        if (CG_GetWeapReticleZoom(cgameGlob, &angle))
        {
            if (!cg_hudGrenadeIconInScope->current.enabled)
                return;
            CG_CalcCrosshairPosition(cgameGlob, &centerX, &centerY);
            centerX = ScrPlace_ApplyX(&scrPlaceView[localClientNum], centerX, 2);
            centerY = ScrPlace_ApplyY(&scrPlaceView[localClientNum], centerY, 2);
        }
        else
        {
            centerX = ScrPlace_ApplyX(&scrPlaceView[localClientNum], 0.0, 2);
            centerY = ScrPlace_ApplyY(&scrPlaceView[localClientNum], 0.0, 2);
        }
        for (entityIndex = 0; entityIndex < g_hudGrenadeCount; ++entityIndex)
        {
            Vec3Sub(g_hudGrenades[entityIndex].origin, cgameGlob->predictedPlayerState.origin, grenadeOffset);
            color[0] = 1.0;
            color[1] = 1.0;
            color[2] = 1.0;
            color[3] = 1.0;
            amplitude = (cg_hudGrenadePointerPulseMax->current.value - cg_hudGrenadePointerPulseMin->current.value) * 0.5;
            bias = cg_hudGrenadePointerPulseMin->current.value + amplitude;
            v6 = cg_hudGrenadePointerPulseFreq->current.value * ((double)cgameGlob->time * 0.006283185444772243);
            v4 = sin(v6);
            color[3] = v4 * amplitude + bias;
            v3 = color[3] - 1.0;
            if (v3 < 0.0)
                v5 = color[3];
            else
                v5 = 1.0;
            v2 = 0.0 - color[3];
            if (v2 < 0.0)
                v1 = v5;
            else
                v1 = 0.0;
            color[3] = v1;
            CG_DrawGrenadePointer(localClientNum, centerX, centerY, grenadeOffset, color);
            CG_DrawGrenadeIcon(localClientNum, centerX, centerY, grenadeOffset, color, g_hudGrenades[entityIndex].material);
        }
    }
}

void __cdecl CG_DrawGrenadePointer(
    int localClientNum,
    float centerX,
    float centerY,
    const float *grenadeOffset,
    const float *color)
{
    float v5; // [esp+Ch] [ebp-84h]
    float v6; // [esp+18h] [ebp-78h]
    float v7; // [esp+1Ch] [ebp-74h]
    float v8; // [esp+20h] [ebp-70h]
    float v9; // [esp+24h] [ebp-6Ch]
    float v10; // [esp+28h] [ebp-68h]
    float v11; // [esp+2Ch] [ebp-64h]
    float angle; // [esp+30h] [ebp-60h]
    float v13; // [esp+3Ch] [ebp-54h]
    const ScreenPlacement *scrPlace; // [esp+40h] [ebp-50h]
    float radiusScale; // [esp+44h] [ebp-4Ch]
    float width; // [esp+4Ch] [ebp-44h]
    float height; // [esp+50h] [ebp-40h]
    float yaw; // [esp+54h] [ebp-3Ch]
    float sinYaw; // [esp+58h] [ebp-38h]
    float pointerPos; // [esp+5Ch] [ebp-34h]
    float pointerPos_4; // [esp+60h] [ebp-30h]
    float cosYaw; // [esp+64h] [ebp-2Ch]
    float pivot; // [esp+68h] [ebp-28h]
    float pivot_4; // [esp+6Ch] [ebp-24h]
    float grenade_vertices[4][2]; // [esp+70h] [ebp-20h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    scrPlace = &scrPlaceView[localClientNum];
    width = cg_hudGrenadePointerWidth->current.value;
    height = cg_hudGrenadePointerHeight->current.value;
    radiusScale = cg_hudGrenadeIconOffset->current.value;
    pivot = cg_hudGrenadePointerPivot->current.value;
    pivot_4 = cg_hudGrenadePointerPivot->current.vector[1];
    angle = vectoyaw(grenadeOffset) - cgArray[0].predictedPlayerState.viewangles[1];
    yaw = AngleNormalize360(angle);
    v13 = yaw * 0.01745329238474369;
    cosYaw = cos(v13);
    sinYaw = sin(v13);
    v11 = radiusScale * sinYaw;
    v10 = scrPlace->scaleVirtualToReal[0] * v11;
    pointerPos = centerX - v10;
    v9 = radiusScale * cosYaw;
    v8 = scrPlace->scaleVirtualToReal[1] * v9;
    pointerPos_4 = centerY - v8;
    grenade_vertices[3][0] = scrPlace->scaleVirtualToReal[0] * -pivot;
    grenade_vertices[0][0] = grenade_vertices[3][0];
    v7 = width - pivot;
    grenade_vertices[2][0] = scrPlace->scaleVirtualToReal[0] * v7;
    grenade_vertices[1][0] = grenade_vertices[2][0];
    grenade_vertices[1][1] = scrPlace->scaleVirtualToReal[1] * -pivot_4;
    grenade_vertices[0][1] = grenade_vertices[1][1];
    v6 = height - pivot_4;
    grenade_vertices[3][1] = scrPlace->scaleVirtualToReal[1] * v6;
    grenade_vertices[2][1] = grenade_vertices[3][1];
    v5 = -yaw;
    CG_DrawRotatedQuadPic(scrPlace, pointerPos, pointerPos_4, grenade_vertices, v5, color, cgMedia.grenadePointer);
}

void __cdecl CG_DrawGrenadeIcon(
    int localClientNum,
    float centerX,
    float centerY,
    const float *grenadeOffset,
    const float *color,
    Material *material)
{
    float v6; // [esp+4h] [ebp-70h]
    float v7; // [esp+8h] [ebp-6Ch]
    float v8; // [esp+Ch] [ebp-68h]
    float v9; // [esp+10h] [ebp-64h]
    float v10; // [esp+14h] [ebp-60h]
    float v11; // [esp+18h] [ebp-5Ch]
    float v12; // [esp+1Ch] [ebp-58h]
    float v13; // [esp+20h] [ebp-54h]
    float angle; // [esp+24h] [ebp-50h]
    float v15; // [esp+30h] [ebp-44h]
    const ScreenPlacement *scrPlace; // [esp+34h] [ebp-40h]
    float radiusScale; // [esp+38h] [ebp-3Ch]
    float width; // [esp+40h] [ebp-34h]
    float height; // [esp+44h] [ebp-30h]
    float yaw; // [esp+48h] [ebp-2Ch]
    float sinYaw; // [esp+4Ch] [ebp-28h]
    float cosYaw; // [esp+50h] [ebp-24h]
    float grenade_vertices[4][2]; // [esp+54h] [ebp-20h] BYREF

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    scrPlace = &scrPlaceView[localClientNum];
    width = cg_hudGrenadeIconWidth->current.value;
    height = cg_hudGrenadeIconHeight->current.value;
    radiusScale = cg_hudGrenadeIconOffset->current.value;
    angle = vectoyaw(grenadeOffset) - cgArray[0].predictedPlayerState.viewangles[1];
    yaw = AngleNormalize360(angle);
    v15 = yaw * 0.01745329238474369;
    cosYaw = cos(v15);
    sinYaw = sin(v15);
    v13 = -(width * 0.5) - radiusScale * sinYaw;
    v12 = scrPlace->scaleVirtualToReal[0] * v13;
    grenade_vertices[3][0] = centerX + v12;
    grenade_vertices[0][0] = grenade_vertices[3][0];
    v11 = width * 0.5 - radiusScale * sinYaw;
    v10 = scrPlace->scaleVirtualToReal[0] * v11;
    grenade_vertices[2][0] = centerX + v10;
    grenade_vertices[1][0] = grenade_vertices[2][0];
    v9 = -(height * 0.5) - radiusScale * cosYaw;
    v8 = scrPlace->scaleVirtualToReal[1] * v9;
    grenade_vertices[1][1] = centerY + v8;
    grenade_vertices[0][1] = grenade_vertices[1][1];
    v7 = height * 0.5 - radiusScale * cosYaw;
    v6 = scrPlace->scaleVirtualToReal[1] * v7;
    grenade_vertices[3][1] = centerY + v6;
    grenade_vertices[2][1] = grenade_vertices[3][1];
    R_AddCmdDrawQuadPic(grenade_vertices, color, material);
}

