#include "cg_local.h"
#include "cg_public.h"

#include <client/client.h>
#include <client_mp/client_mp.h>
#include <cgame_mp/cg_local_mp.h>

#include <gfx_d3d/r_rendercmds.h>
#include <universal/q_parse.h>

const float sign[4][2] =
{
    { -1.0f, -1.0f },
    { 1.0f, -1.0f },
    { 1.0f, 1.0f },
    {-1.0f, 1.0f }
};

float color_0[4];

void __cdecl CG_DrawRotatedPicPhysical(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    float width,
    float height,
    float angle,
    const float *color,
    Material *material)
{
    float v8; // [esp+0h] [ebp-70h]
    float v9; // [esp+4h] [ebp-6Ch]
    float v10; // [esp+8h] [ebp-68h]
    float v11; // [esp+Ch] [ebp-64h]
    float v12; // [esp+10h] [ebp-60h]
    float v13; // [esp+14h] [ebp-5Ch]
    float v14; // [esp+20h] [ebp-50h]
    float cos; // [esp+24h] [ebp-4Ch]
    float halfWidth; // [esp+28h] [ebp-48h]
    float verts[4][2]; // [esp+2Ch] [ebp-44h] BYREF
    int32_t i; // [esp+4Ch] [ebp-24h]
    float scale[2][2]; // [esp+50h] [ebp-20h]
    float halfHeight; // [esp+60h] [ebp-10h]
    float center[2]; // [esp+64h] [ebp-Ch]
    float sin; // [esp+6Ch] [ebp-4h]

    v14 = angle * 0.01745329238474369;
    cos = cosf(v14);
    sin = sinf(v14);
    
    v13 = width * 0.5;
    halfWidth = scrPlace->scaleRealToVirtual[0] * v13;
    v12 = height * 0.5;
    halfHeight = scrPlace->scaleRealToVirtual[1] * v12;
    v11 = scrPlace->scaleRealToVirtual[0] * x;
    center[0] = v11 + halfWidth;
    v10 = scrPlace->scaleRealToVirtual[1] * y;
    center[1] = v10 + halfHeight;
    scale[0][0] = cos * halfWidth;
    scale[0][1] = sin * halfWidth;
    scale[1][0] = sin * halfHeight;
    scale[1][1] = cos * halfHeight;
    for (i = 0; i < 4; ++i)
    {
        v9 = scale[0][0] * (float)sign[i][0] + center[0] - scale[1][0] * (float)sign[i][1];
        verts[i][0] = scrPlace->scaleVirtualToReal[0] * v9;
        v8 = scale[1][1] * (float)sign[i][1] + scale[0][1] * (float)sign[i][0] + center[1];
        verts[i][1] = scrPlace->scaleVirtualToReal[1] * v8;
    }
    R_AddCmdDrawQuadPic(verts, color, material);
}

void __cdecl CG_DrawRotatedPic(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    float width,
    float height,
    int32_t horzAlign,
    int32_t vertAlign,
    float angle,
    const float *color,
    Material *material)
{
    ScrPlace_ApplyRect(scrPlace, &x, &y, &width, &height, horzAlign, vertAlign);
    CG_DrawRotatedPicPhysical(scrPlace, x, y, width, height, angle, color, material);
}

void __cdecl CG_DrawRotatedQuadPic(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    const float (*verts)[2],
    float angle,
    const float *color,
    Material *material)
{
    float v7; // [esp+0h] [ebp-58h]
    float v8; // [esp+4h] [ebp-54h]
    float v9; // [esp+8h] [ebp-50h]
    float v10; // [esp+Ch] [ebp-4Ch]
    float v11; // [esp+10h] [ebp-48h]
    float v12; // [esp+14h] [ebp-44h]
    float v13; // [esp+18h] [ebp-40h]
    float v14; // [esp+1Ch] [ebp-3Ch]
    float v15; // [esp+28h] [ebp-30h]
    float c; // [esp+2Ch] [ebp-2Ch]
    float xy[4][2]; // [esp+30h] [ebp-28h] BYREF
    float s; // [esp+50h] [ebp-8h]
    int32_t i; // [esp+54h] [ebp-4h]

    v15 = angle * 0.01745329238474369;
    c = cos(v15);
    s = sin(v15);
    for (i = 0; i < 4; ++i)
    {
        v14 = x * scrPlace->scaleRealToVirtual[0];
        v13 = (float)(*verts)[2 * i] * scrPlace->scaleRealToVirtual[0];
        v12 = (float)(*verts)[2 * i + 1] * scrPlace->scaleRealToVirtual[1];
        v8 = c * v13 + v14 - s * v12;
        xy[i][0] = scrPlace->scaleVirtualToReal[0] * v8;
        v11 = y * scrPlace->scaleRealToVirtual[1];
        v10 = (float)(*verts)[2 * i] * scrPlace->scaleRealToVirtual[0];
        v9 = (float)(*verts)[2 * i + 1] * scrPlace->scaleRealToVirtual[1];
        v7 = c * v9 + s * v10 + v11;
        xy[i][1] = scrPlace->scaleVirtualToReal[1] * v7;
    }
    R_AddCmdDrawQuadPic(xy, color, material);
}

void __cdecl CG_DrawVLine(
    const ScreenPlacement *scrPlace,
    float x,
    float top,
    float lineWidth,
    float height,
    int32_t horzAlign,
    int32_t vertAlign,
    const float *color,
    Material *material)
{
    float halfWidth; // [esp+0h] [ebp-34h]
    float verts[4][2]; // [esp+4h] [ebp-30h] BYREF
    int32_t i; // [esp+24h] [ebp-10h]
    float halfHeight; // [esp+28h] [ebp-Ch]
    float center[2]; // [esp+2Ch] [ebp-8h]

    ScrPlace_ApplyRect(scrPlace, &x, &top, &lineWidth, &height, horzAlign, vertAlign);
    halfWidth = lineWidth * 0.5;
    halfHeight = height * 0.5;
    center[0] = x;
    center[1] = top + halfHeight;
    for (i = 0; i < 4; ++i)
    {
        verts[i][0] = (float)sign[i + 4][0] * halfWidth + center[0];
        verts[i][1] = (float)sign[i + 4][1] * halfHeight + center[1];
    }
    R_AddCmdDrawQuadPic(verts, color, material);
}

void __cdecl CG_DrawStringExt(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    char *string,
    const float *setColor,
    int32_t forceColor,
    int32_t shadow,
    float charHeight)
{
    Font_s *font; // [esp+1Ch] [ebp-8h]
    float fontScale; // [esp+20h] [ebp-4h]
    float ya; // [esp+34h] [ebp+10h]

    ya = charHeight * 0.800000011920929 + y;
    fontScale = charHeight / 48.0;
    if (!setColor)
        setColor = colorWhite;
    font = UI_GetFontHandle(scrPlace, 5, fontScale);
    UI_DrawText(scrPlace, string, 0x7FFFFFFF, font, x, ya, 1, 1, fontScale, setColor, shadow != 0 ? 3 : 0);
}

int32_t __cdecl CG_DrawDevString(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    float xScale,
    float yScale,
    char *s,
    const float *color,
    char align,
    Font_s *font)
{
    float width; // [esp+28h] [ebp-Ch]
    float widtha; // [esp+28h] [ebp-Ch]
    float height; // [esp+2Ch] [ebp-8h]
    float heighta; // [esp+2Ch] [ebp-8h]
    int32_t step; // [esp+30h] [ebp-4h]

    if ((align & 3) != 1 && (align & 3) != 2 && (align & 3) != 3)
        MyAssertHandler(
            ".\\cgame\\cg_drawtools.cpp",
            216,
            0,
            "%s",
            "(align & CG_ALIGN_X) == CG_ALIGN_LEFT || (align & CG_ALIGN_X) == CG_ALIGN_RIGHT || (align & CG_ALIGN_X) == CG_ALIGN_CENTER");
    if ((align & 3) == 2)
    {
        width = (double)R_TextWidth(s, 0, font) * xScale;
        x = x - width;
    }
    else if ((align & 3) == 3)
    {
        widtha = (double)R_TextWidth(s, 0, font) * xScale;
        x = x - widtha * 0.5;
    }
    if ((align & 0xC) != 4 && (align & 0xC) != 8 && (align & 0xC) != 0xC)
        MyAssertHandler(
            ".\\cgame\\cg_drawtools.cpp",
            228,
            0,
            "%s",
            "(align & CG_ALIGN_Y) == CG_ALIGN_TOP || (align & CG_ALIGN_Y) == CG_ALIGN_BOTTOM || (align & CG_ALIGN_Y) == CG_ALIGN_MIDDLE");
    step = R_TextHeight(font);
    if ((align & 0xC) == 4)
    {
        height = (double)step * yScale;
        y = y + height;
    }
    else if ((align & 0xC) == 0xC)
    {
        heighta = (double)step * yScale;
        y = heighta * 0.5 + y;
    }
    CL_DrawText(scrPlace, s, 0x7FFFFFFF, font, x, y, 1, 1, xScale, yScale, color, 0);
    return step;
}

int32_t __cdecl CG_DrawBigDevString(const ScreenPlacement *scrPlace, float x, float y, char *s, float alpha, char align)
{
    float color[4]; // [esp+14h] [ebp-10h] BYREF

    color[0] = 1.0;
    color[1] = 1.0;
    color[2] = 1.0;
    color[3] = alpha;
    return CG_DrawBigDevStringColor(scrPlace, x, y, s, color, align);
}

int32_t __cdecl CG_DrawBigDevStringColor(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    char *s,
    const float *color,
    char align)
{
    return CG_DrawDevString(scrPlace, x, y, 1.0, 1.0, s, color, align, cgMedia.bigDevFont);
}

int32_t __cdecl CG_DrawSmallDevStringColor(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    char *s,
    const float *color,
    char align)
{
    return CG_DrawDevString(scrPlace, x, y, 1.0, 1.0, s, color, align, cgMedia.smallDevFont);
}

double __cdecl CG_FadeAlpha(int32_t timeNow, int32_t startMsec, int32_t totalMsec, int32_t fadeMsec)
{
    int32_t t; // [esp+8h] [ebp-4h]

    t = timeNow - startMsec;
    if (fadeMsec <= 0 || totalMsec - t >= fadeMsec)
        return 1.0;
    return (float)((double)(totalMsec - t) * 1.0 / (double)fadeMsec);
}

float *__cdecl CG_FadeColor(int32_t timeNow, int32_t startMsec, int32_t totalMsec, int32_t fadeMsec)
{
    if (!startMsec)
        return 0;
    if (timeNow - startMsec >= totalMsec)
        return 0;
    color_0[3] = CG_FadeAlpha(timeNow, startMsec, totalMsec, fadeMsec);
    color_0[2] = 1.0;
    color_0[1] = 1.0;
    color_0[0] = 1.0;
    return color_0;
}

void __cdecl CG_MiniMapChanged(int32_t localClientNum)
{
    parseInfo_t* v1; // eax
    parseInfo_t* v2; // eax
    parseInfo_t* v3; // eax
    parseInfo_t* v4; // eax
    const char* string; // [esp+8h] [ebp-28h] BYREF
    const char* material; // [esp+Ch] [ebp-24h]
    float toLR[2]; // [esp+10h] [ebp-20h]
    float south[2]; // [esp+18h] [ebp-18h]
    float east[2]; // [esp+20h] [ebp-10h]
    float lowerRight[2]; // [esp+28h] [ebp-8h]
    cg_s *cgameGlob;

    string = CL_GetConfigString(localClientNum, 0x337u);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    material = (const char*)Com_Parse(&string);
    cgameGlob->compassMapMaterial = Material_RegisterHandle((char*)material, 7);
    v1 = Com_Parse(&string);
    cgameGlob->compassMapUpperLeft[0] = atof(v1->token);
    v2 = Com_Parse(&string);
    cgameGlob->compassMapUpperLeft[1] = atof(v2->token);
    v3 = Com_Parse(&string);
    lowerRight[0] = atof(v3->token);
    v4 = Com_Parse(&string);
    lowerRight[1] = atof(v4->token);
    east[0] = cgameGlob->compassNorth[1];
    east[1] = -cgameGlob->compassNorth[0];
    south[0] = -cgameGlob->compassNorth[0];
    south[1] = -cgameGlob->compassNorth[1];
    toLR[0] = lowerRight[0] - cgameGlob->compassMapUpperLeft[0];
    toLR[1] = lowerRight[1] - cgameGlob->compassMapUpperLeft[1];
    cgameGlob->compassMapWorldSize[0] = east[1] * toLR[1] + east[0] * toLR[0];
    cgameGlob->compassMapWorldSize[1] = south[1] * toLR[1] + south[0] * toLR[0];
    if (cgameGlob->compassMapWorldSize[0] == 0.0)
        cgameGlob->compassMapWorldSize[0] = 1000.0;
    if (cgameGlob->compassMapWorldSize[1] == 0.0)
        cgameGlob->compassMapWorldSize[1] = 1000.0;
}

void __cdecl CG_NorthDirectionChanged(int32_t localClientNum)
{
    float v1; // [esp+8h] [ebp-Ch]
    const char *pszString; // [esp+10h] [ebp-4h]
    cg_s *cgameGlob;

    pszString = CL_GetConfigString(localClientNum, 0x336u);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgameGlob->compassNorthYaw = atof(pszString);
    v1 = cgameGlob->compassNorthYaw * 0.01745329238474369;
    cgameGlob->compassNorth[0] = cos(v1);
    cgameGlob->compassNorth[1] = sin(v1);
    CG_MiniMapChanged(localClientNum);
}

void __cdecl CG_DebugLine(const float *start, const float *end, const float *color, int32_t depthTest, int32_t duration)
{
    CL_AddDebugLine(start, end, color, depthTest, duration, 0);
}

void __cdecl CG_DebugStar(const float *point, const float *color, int32_t duration)
{
    CL_AddDebugStar(point, color, duration, 0);
}

void __cdecl CG_DebugStarWithText(
    const float *point,
    const float *starColor,
    const float *textColor,
    char *string,
    float fontsize,
    int32_t duration)
{
    CL_AddDebugStarWithText(point, starColor, textColor, string, fontsize, duration, 0);
}

void __cdecl CG_DebugBox(
    const float *origin,
    const float *mins,
    const float *maxs,
    float yaw,
    const float *color,
    int32_t depthTest,
    int32_t duration)
{
    float v7; // [esp+0h] [ebp-94h]
    float v8; // [esp+10h] [ebp-84h]
    uint32_t j; // [esp+14h] [ebp-80h]
    float rotated; // [esp+18h] [ebp-7Ch]
    float rotated_4; // [esp+1Ch] [ebp-78h]
    uint32_t i; // [esp+24h] [ebp-70h]
    uint32_t ia; // [esp+24h] [ebp-70h]
    float fCos; // [esp+28h] [ebp-6Ch]
    float v[25]; // [esp+2Ch] [ebp-68h] BYREF
    float fSin; // [esp+90h] [ebp-4h]

    v8 = yaw * 0.01745329238474369;
    fCos = cos(v8);
    fSin = sin(v8);
    for (i = 0; i < 8; ++i)
    {
        for (j = 0; j < 3; ++j)
        {
            if ((i & (1 << j)) != 0)
                v7 = maxs[j];
            else
                v7 = mins[j];
            v[3 * i + j] = v7;
        }
        rotated = v[3 * i] * fCos - v[3 * i + 1] * fSin;
        rotated_4 = v[3 * i] * fSin + v[3 * i + 1] * fCos;
        v[3 * i] = rotated;
        v[3 * i + 1] = rotated_4;
        Vec3Add(&v[3 * i], origin, &v[3 * i]);
    }
    for (ia = 0; ia < 0xC; ++ia)
        CG_DebugLine(&v[3 * iEdgePairs[ia][0]], &v[3 * iEdgePairs[ia][1]], color, depthTest, duration);
}

void __cdecl CG_DebugBoxOriented(
    const float *origin,
    const float *mins,
    const float *maxs,
    const mat3x3 &rotation,
    const float *color,
    int32_t depthTest,
    int32_t duration)
{
    float v7; // [esp+0h] [ebp-7Ch]
    float *v8; // [esp+4h] [ebp-78h]
    uint32_t j; // [esp+8h] [ebp-74h]
    float rotated[3]; // [esp+Ch] [ebp-70h] BYREF
    uint32_t i; // [esp+18h] [ebp-64h]
    float v[8][3]; // [esp+1Ch] [ebp-60h] BYREF

    for (i = 0; i < 8; ++i)
    {
        for (j = 0; j < 3; ++j)
        {
            if ((i & (1 << j)) != 0)
                v7 = maxs[j];
            else
                v7 = mins[j];
            v[i][j] = v7;
        }
        MatrixTransformVector(v[i], rotation, rotated);
        v8 = v[i];
        *v8 = rotated[0];
        v8[1] = rotated[1];
        v8[2] = rotated[2];
        Vec3Add(v[i], origin, v[i]);
    }
    for (i = 0; i < 0xC; ++i)
        CG_DebugLine(v[iEdgePairs[i][0]], v[iEdgePairs[i][1]], color, depthTest, duration);
}

void __cdecl CG_DebugCircle(
    const float *center,
    float radius,
    const float *dir,
    const float *color,
    int32_t depthTest,
    int32_t duration)
{
    float fAngle; // [esp+1Ch] [ebp-F4h]
    float fCos; // [esp+20h] [ebp-F0h]
    float fCosa; // [esp+20h] [ebp-F0h]
    float fSin; // [esp+24h] [ebp-ECh]
    float fSina; // [esp+24h] [ebp-ECh]
    float normal[3]; // [esp+28h] [ebp-E8h] BYREF
    float right[3]; // [esp+34h] [ebp-DCh] BYREF
    float up[3]; // [esp+40h] [ebp-D0h] BYREF
    uint32_t i; // [esp+4Ch] [ebp-C4h]
    float v[16][3]; // [esp+50h] [ebp-C0h] BYREF

    Vec3NormalizeTo(dir, normal);
    PerpendicularVector(normal, right);
    Vec3Cross(normal, right, up);
    for (i = 0; i < 0x10; ++i)
    {
        fAngle = (double)i * 0.3926990926265717;
        fCos = cos(fAngle);
        fSin = sin(fAngle);
        fSina = fSin * radius;
        fCosa = fCos * radius;
        Vec3Mad(center, fSina, up, v[i]);
        Vec3Mad(v[i], fCosa, right, v[i]);
    }
    for (i = 0; i < 0x10; ++i)
        CG_DebugLine(v[i], v[(i + 1) % 0x10], color, depthTest, duration);
}

void __cdecl CG_TeamColor(int32_t team, const char *prefix, float *color)
{
    const char *v3; // eax
    char dvarName[32]; // [esp+4h] [ebp-24h] BYREF

    switch (team)
    {
    case 0:
        Com_sprintf(dvarName, 0x20u, "%s_Free", prefix);
        goto LABEL_9;
    case 1:
        Com_sprintf(dvarName, 0x20u, "%s_Axis", prefix);
        goto LABEL_9;
    case 2:
        Com_sprintf(dvarName, 0x20u, "%s_Allies", prefix);
        goto LABEL_9;
    case 3:
        Com_sprintf(dvarName, 0x20u, "%s_Spectator", prefix);
    LABEL_9:
        Dvar_GetUnpackedColorByName(dvarName, color);
        break;
    default:
        if (!alwaysfails)
        {
            v3 = va("Unknown team: %i", team);
            MyAssertHandler(".\\cgame\\cg_drawtools.cpp", 604, 0, v3);
        }
        break;
    }
}

void __cdecl CG_RelativeTeamColor(int32_t clientNum, const char *prefix, float *color, int32_t localClientNum)
{
    char dvarName[32]; // [esp+Ch] [ebp-28h] BYREF
    float savedAlpha; // [esp+30h] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    bcassert(cgameGlob->clientNum, MAX_CLIENTS);
    savedAlpha = color[3];
    if (cgameGlob->bgs.clientinfo[clientNum].team == TEAM_SPECTATOR)
    {
        Com_sprintf(dvarName, 0x20u, "%s_Spectator", prefix);
    }
    else if (clientNum == cgameGlob->clientNum
        || cgameGlob->bgs.clientinfo[clientNum].team
        && cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team == cgameGlob->bgs.clientinfo[clientNum].team)
    {
        Com_sprintf(dvarName, 0x20u, "%s_MyTeam", prefix);
    }
    else
    {
        Com_sprintf(dvarName, 0x20u, "%s_EnemyTeam", prefix);
    }
    Dvar_GetUnpackedColorByName(dvarName, color);
    color[3] = savedAlpha;
}

