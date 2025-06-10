#include "game_public.h"
#include <qcommon/mem_track.h>
#include <script/scr_vm.h>
#include <game_mp/g_utils_mp.h>
#include <game_mp/g_public_mp.h>
#include <server_mp/server.h>
#include <server/sv_game.h>

const char *g_he_font[6] = { "default", "bigfixed", "smallfixed", "objective", "big", "small" }; // idb
const char *g_he_alignx[3] = { "left", "center", "right" };
const char *g_he_aligny[3] = { "top", "middle", "bottom" };
const char *g_he_horzalign[8] = { "subleft", "left", "center", "right", "fullscreen", "noscale", "alignto640", "center_safearea" };
const char *g_he_vertalign[8] = { "subtop", "top", "middle", "bottom", "fullscreen", "noscale", "alignto480", "center_safearea" };


const game_hudelem_field_t fields_0[20] = // LWSS: should be called "fields"
{
  { "x", 4, F_FLOAT, 0, 0, NULL, NULL },
  { "y", 8, F_FLOAT, 0, 0, NULL, NULL },
  { "z", 12, F_FLOAT, 0, 0, NULL, NULL },
  { "fontscale", 20, F_FLOAT, -1, 0, &HudElem_SetFontScale, NULL },
  { "font", 24, F_INT, -1, 0, &HudElem_SetFont, &HudElem_GetFont },
  { "alignx", 28, F_INT, 3, 2, &HudElem_SetAlignX, &HudElem_GetAlignX },
  { "aligny", 28, F_INT, 3, 0, &HudElem_SetAlignY, &HudElem_GetAlignY },
  { "horzalign", 32, F_INT, 7, 3, &HudElem_SetHorzAlign, &HudElem_GetHorzAlign },
  { "vertalign", 32, F_INT, 7, 0, &HudElem_SetVertAlign, &HudElem_GetVertAlign },
  { "color", 36, F_INT, -1, 0, &HudElem_SetColor, &HudElem_GetColor },
  { "alpha", 36, F_INT, -1, 0, &HudElem_SetAlpha, &HudElem_GetAlpha },
  { "label", 52, F_INT, -1, 0, &HudElem_SetLocalizedString, NULL },
  { "sort", 128, F_FLOAT, 0, 0, NULL, NULL },
  {
    "foreground",
    156,
    F_INT,
    -1,
    0,
    &HudElem_SetFlagForeground,
    &HudElem_GetFlagForeground
  },
  {
    "hidewhendead",
    156,
    F_INT,
    -1,
    0,
    &HudElem_SetFlagHideWhenDead,
    &HudElem_GetFlagHideWhenDead
  },
  {
    "hidewheninmenu",
    156,
    F_INT,
    -1,
    0,
    &HudElem_SetFlagHideWhenInMenu,
    &HudElem_GetFlagHideWhenInMenu
  },
  {
    "glowcolor",
    132,
    F_INT,
    -1,
    0,
    &HudElem_SetGlowColor,
    &HudElem_GetGlowColor
  },
  {
    "glowalpha",
    132,
    F_INT,
    -1,
    0,
    &HudElem_SetGlowAlpha,
    &HudElem_GetGlowAlpha
  },
  { "archived", 168, F_INT, -1, 0, &HudElem_SetBoolean, NULL },
  { NULL, 0, F_INT, 0, 0, NULL, NULL }
}; // idb

//Line 53047:  0006 : 00514eb0       struct game_hudelem_s *g_hudelems 82cc4eb0     g_hudelem.obj

game_hudelem_s g_hudelems[1024];


void __cdecl TRACK_g_hudelem()
{
    track_static_alloc_internal(g_hudelems, 176128, "g_hudelems", 10);
}

game_hudelem_s *__cdecl HudElem_Alloc(int clientNum, int teamNum)
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 0x400; ++i)
    {
        if (g_hudelems[i].elem.type == HE_TYPE_FREE)
        {
            HudElem_SetDefaults(&g_hudelems[i]);
            g_hudelems[i].clientNum = clientNum;
            g_hudelems[i].team = teamNum;
            return &g_hudelems[i];
        }
    }
    return 0;
}

void __cdecl HudElem_SetDefaults(game_hudelem_s *hud)
{
    if (!hud)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 181, 0, "%s", "hud");
    if ((unsigned int)(hud - g_hudelems) >= 0x400)
        MyAssertHandler(
            ".\\game\\g_hudelem.cpp",
            182,
            0,
            "hud - g_hudelems doesn't index ARRAY_COUNT( g_hudelems )\n\t%i not in [0, %i)",
            hud - g_hudelems,
            1024);
    hud->elem.type = HE_TYPE_TEXT;
    hud->elem.x = 0.0;
    hud->elem.y = 0.0;
    hud->elem.z = 0.0;
    hud->elem.targetEntNum = 1023;
    hud->elem.font = 0;
    hud->elem.alignOrg = 0;
    hud->elem.alignScreen = 0;
    hud->elem.color.rgba = -1;
    hud->elem.glowColor.rgba = 0;
    hud->elem.fromColor.rgba = 0;
    hud->elem.fadeStartTime = 0;
    hud->elem.fadeTime = 0;
    hud->elem.label = 0;
    hud->elem.sort = 0.0;
    hud->elem.flags = 0;
    hud->elem.fxBirthTime = 0;
    hud->elem.fxLetterTime = 0;
    hud->elem.fxDecayStartTime = 0;
    hud->elem.fxDecayDuration = 0;
    hud->elem.soundID = 0;
    hud->elem.moveStartTime = 0;
    hud->elem.moveTime = 0;
    hud->elem.fontScale = 0.0;
    hud->archived = 1;
    HudElem_ClearTypeSettings(hud);
}

void __cdecl HudElem_ClearTypeSettings(game_hudelem_s *hud)
{
    hud->elem.width = 0;
    hud->elem.height = 0;
    hud->elem.materialIndex = 0;
    hud->elem.fromX = 0.0;
    hud->elem.fromY = 0.0;
    hud->elem.fromAlignOrg = 0;
    hud->elem.fromAlignScreen = 0;
    hud->elem.fromWidth = 0;
    hud->elem.fromHeight = 0;
    hud->elem.scaleStartTime = 0;
    hud->elem.scaleTime = 0;
    hud->elem.time = 0;
    hud->elem.duration = 0;
    hud->elem.value = 0.0;
    hud->elem.text = 0;
}

void __cdecl HudElem_Free(game_hudelem_s *hud)
{
    if (!hud)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 264, 0, "%s", "hud");
    if ((unsigned int)(hud - g_hudelems) >= 0x400)
        MyAssertHandler(
            ".\\game\\g_hudelem.cpp",
            265,
            0,
            "hud - g_hudelems doesn't index ARRAY_COUNT( g_hudelems )\n\t%i not in [0, %i)",
            hud - g_hudelems,
            1024);
    if (hud->elem.type <= HE_TYPE_FREE || hud->elem.type >= HE_TYPE_COUNT)
        MyAssertHandler(
            ".\\game\\g_hudelem.cpp",
            266,
            0,
            "%s\n\t(hud->elem.type) = %i",
            "(hud->elem.type > HE_TYPE_FREE && hud->elem.type < HE_TYPE_COUNT)",
            hud->elem.type);
    Scr_FreeHudElem(hud);
    hud->elem.type = HE_TYPE_FREE;
}

void __cdecl HudElem_ClientDisconnect(gentity_s *ent)
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 0x400; ++i)
    {
        if (g_hudelems[i].elem.type)
        {
            if (g_hudelems[i].clientNum == ent->s.number)
                HudElem_Free(&g_hudelems[i]);
        }
    }
}

void __cdecl HudElem_DestroyAll()
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 0x400; ++i)
    {
        if (g_hudelems[i].elem.type)
            HudElem_Free(&g_hudelems[i]);
    }
    memset((unsigned __int8 *)g_hudelems, 0, sizeof(g_hudelems));
}

void __cdecl HudElem_SetLocalizedString(game_hudelem_s *hud, int offset)
{
    char *string; // [esp+0h] [ebp-8h]

    string = Scr_GetIString(0);
    *(he_type_t *)((char *)&hud->elem.type + fields_0[offset].ofs) = (he_type_t)G_LocalizedStringIndex(string);
}

void __cdecl HudElem_SetFlagForeground(game_hudelem_s *hud, int offset)
{
    unsigned int v2; // ecx
    int *flags; // [esp+4h] [ebp-8h]

    flags = (int *)((char *)hud + fields_0[offset].ofs);
    if (Scr_GetInt(0))
        v2 = *flags | 1;
    else
        v2 = *flags & 0xFFFFFFFE;
    *flags = v2;
}

void __cdecl HudElem_GetFlagForeground(game_hudelem_s *hud, int offset)
{
    if (fields_0[offset].ofs != 156)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 411, 0, "%s", "fields[offset].ofs == HEOFS( elem.flags )");
    if ((hud->elem.flags & 1) != 0)
        Scr_AddBool(1u);
    else
        Scr_AddBool(0);
}

void __cdecl HudElem_SetFlagHideWhenDead(game_hudelem_s *hud, int offset)
{
    unsigned int v2; // ecx
    int *flags; // [esp+4h] [ebp-8h]

    flags = (int *)((char *)hud + fields_0[offset].ofs);
    if (Scr_GetInt(0))
        v2 = *flags | 2;
    else
        v2 = *flags & 0xFFFFFFFD;
    *flags = v2;
}

void __cdecl HudElem_GetFlagHideWhenDead(game_hudelem_s *hud, int offset)
{
    if (fields_0[offset].ofs != 156)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 439, 0, "%s", "fields[offset].ofs == HEOFS( elem.flags )");
    if ((hud->elem.flags & 2) != 0)
        Scr_AddBool(1u);
    else
        Scr_AddBool(0);
}

void __cdecl HudElem_SetFlagHideWhenInMenu(game_hudelem_s *hud, int offset)
{
    unsigned int v2; // ecx
    int *flags; // [esp+4h] [ebp-8h]

    flags = (int *)((char *)hud + fields_0[offset].ofs);
    if (Scr_GetInt(0))
        v2 = *flags | 4;
    else
        v2 = *flags & 0xFFFFFFFB;
    *flags = v2;
}

void __cdecl HudElem_GetFlagHideWhenInMenu(game_hudelem_s *hud, int offset)
{
    if (fields_0[offset].ofs != 156)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 467, 0, "%s", "fields[offset].ofs == HEOFS( elem.flags )");
    if ((hud->elem.flags & 4) != 0)
        Scr_AddBool(1u);
    else
        Scr_AddBool(0);
}

void __cdecl HudElem_SetBoolean(game_hudelem_s *hud, int offset)
{
    *(VariableUnion *)((char *)&hud->elem.type + fields_0[offset].ofs) = Scr_GetInt(0);
}

void __cdecl HudElem_SetColor(game_hudelem_s *hud, int offset)
{
    float v2; // [esp+0h] [ebp-78h]
    float v3; // [esp+4h] [ebp-74h]
    float v4; // [esp+8h] [ebp-70h]
    float v5; // [esp+Ch] [ebp-6Ch]
    float v6; // [esp+10h] [ebp-68h]
    float v7; // [esp+14h] [ebp-64h]
    float v8; // [esp+18h] [ebp-60h]
    float v9; // [esp+1Ch] [ebp-5Ch]
    float v10; // [esp+20h] [ebp-58h]
    float v11; // [esp+28h] [ebp-50h]
    float v12; // [esp+38h] [ebp-40h]
    float v13; // [esp+40h] [ebp-38h]
    float v14; // [esp+50h] [ebp-28h]
    float v15; // [esp+58h] [ebp-20h]
    float v16; // [esp+68h] [ebp-10h]
    float color[3]; // [esp+6Ch] [ebp-Ch] BYREF

    if (fields_0[offset].ofs != 36)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 495, 0, "%s", "fields[offset].ofs == HEOFS( elem.color )");
    Scr_GetVector(0, color);
    v10 = color[0] - 1.0;
    if (v10 < 0.0)
        v16 = color[0];
    else
        v16 = 1.0;
    v9 = 0.0 - v16;
    if (v9 < 0.0)
        v8 = v16;
    else
        v8 = 0.0;
    v15 = v8 * 255.0;
    hud->elem.color.r = (int)(v15 + 9.313225746154785e-10);
    v7 = color[1] - 1.0;
    if (v7 < 0.0)
        v14 = color[1];
    else
        v14 = 1.0;
    v6 = 0.0 - v14;
    if (v6 < 0.0)
        v5 = v14;
    else
        v5 = 0.0;
    v13 = v5 * 255.0;
    hud->elem.color.g = (int)(v13 + 9.313225746154785e-10);
    v4 = color[2] - 1.0;
    if (v4 < 0.0)
        v12 = color[2];
    else
        v12 = 1.0;
    v3 = 0.0 - v12;
    if (v3 < 0.0)
        v2 = v12;
    else
        v2 = 0.0;
    v11 = v2 * 255.0;
    hud->elem.color.b = (int)(v11 + 9.313225746154785e-10);
}

void __cdecl HudElem_GetColor(game_hudelem_s *hud, int offset)
{
    float color[3]; // [esp+Ch] [ebp-Ch] BYREF

    if (fields_0[offset].ofs != 36)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 508, 0, "%s", "fields[offset].ofs == HEOFS( elem.color )");
    color[0] = (double)hud->elem.color.r * 0.003921568859368563;
    color[1] = (double)hud->elem.color.g * 0.003921568859368563;
    color[2] = (double)hud->elem.color.b * 0.003921568859368563;
    Scr_AddVector(color);
}

void __cdecl HudElem_SetAlpha(game_hudelem_s *hud, int offset)
{
    float v2; // [esp+0h] [ebp-28h]
    float v3; // [esp+4h] [ebp-24h]
    float v4; // [esp+8h] [ebp-20h]
    float v5; // [esp+10h] [ebp-18h]
    float v6; // [esp+20h] [ebp-8h]
    float alpha; // [esp+24h] [ebp-4h]

    if (fields_0[offset].ofs != 36)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 521, 0, "%s", "fields[offset].ofs == HEOFS( elem.color )");
    alpha = Scr_GetFloat(0);
    v4 = alpha - 1.0;
    if (v4 < 0.0)
        v6 = alpha;
    else
        v6 = 1.0;
    v3 = 0.0 - v6;
    if (v3 < 0.0)
        v2 = v6;
    else
        v2 = 0.0;
    v5 = v2 * 255.0;
    hud->elem.color.a = (int)(v5 + 9.313225746154785e-10);
}

void __cdecl HudElem_GetAlpha(game_hudelem_s *hud, int offset)
{
    iassert(fields_0[offset].ofs == 36 /*HEOFS(elem.color)*/);
    Scr_AddFloat(hud->elem.color.a * 0.003921568859368563);
}

void __cdecl HudElem_SetGlowColor(game_hudelem_s *hud, int offset)
{
    float v2; // [esp+0h] [ebp-78h]
    float v3; // [esp+4h] [ebp-74h]
    float v4; // [esp+8h] [ebp-70h]
    float v5; // [esp+Ch] [ebp-6Ch]
    float v6; // [esp+10h] [ebp-68h]
    float v7; // [esp+14h] [ebp-64h]
    float v8; // [esp+18h] [ebp-60h]
    float v9; // [esp+1Ch] [ebp-5Ch]
    float v10; // [esp+20h] [ebp-58h]
    float v11; // [esp+28h] [ebp-50h]
    float v12; // [esp+38h] [ebp-40h]
    float v13; // [esp+40h] [ebp-38h]
    float v14; // [esp+50h] [ebp-28h]
    float v15; // [esp+58h] [ebp-20h]
    float v16; // [esp+68h] [ebp-10h]
    float glowColor[3]; // [esp+6Ch] [ebp-Ch] BYREF

    if (fields_0[offset].ofs != 132)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 540, 0, "%s", "fields[offset].ofs == HEOFS( elem.glowColor )");
    Scr_GetVector(0, glowColor);
    v10 = glowColor[0] - 1.0;
    if (v10 < 0.0)
        v16 = glowColor[0];
    else
        v16 = 1.0;
    v9 = 0.0 - v16;
    if (v9 < 0.0)
        v8 = v16;
    else
        v8 = 0.0;
    v15 = v8 * 255.0;
    hud->elem.glowColor.r = (int)(v15 + 9.313225746154785e-10);
    v7 = glowColor[1] - 1.0;
    if (v7 < 0.0)
        v14 = glowColor[1];
    else
        v14 = 1.0;
    v6 = 0.0 - v14;
    if (v6 < 0.0)
        v5 = v14;
    else
        v5 = 0.0;
    v13 = v5 * 255.0;
    hud->elem.glowColor.g = (int)(v13 + 9.313225746154785e-10);
    v4 = glowColor[2] - 1.0;
    if (v4 < 0.0)
        v12 = glowColor[2];
    else
        v12 = 1.0;
    v3 = 0.0 - v12;
    if (v3 < 0.0)
        v2 = v12;
    else
        v2 = 0.0;
    v11 = v2 * 255.0;
    hud->elem.glowColor.b = (int)(v11 + 9.313225746154785e-10);
}

void __cdecl HudElem_GetGlowColor(game_hudelem_s *hud, int offset)
{
    float glowColor[3]; // [esp+Ch] [ebp-Ch] BYREF

    if (fields_0[offset].ofs != 132)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 553, 0, "%s", "fields[offset].ofs == HEOFS( elem.glowColor )");
    glowColor[0] = (double)hud->elem.glowColor.r * 0.003921568859368563;
    glowColor[1] = (double)hud->elem.glowColor.g * 0.003921568859368563;
    glowColor[2] = (double)hud->elem.glowColor.b * 0.003921568859368563;
    Scr_AddVector(glowColor);
}

void __cdecl HudElem_SetGlowAlpha(game_hudelem_s *hud, int offset)
{
    float v2; // [esp+0h] [ebp-28h]
    float v3; // [esp+4h] [ebp-24h]
    float v4; // [esp+8h] [ebp-20h]
    float v5; // [esp+10h] [ebp-18h]
    float v6; // [esp+20h] [ebp-8h]
    float glowAlpha; // [esp+24h] [ebp-4h]

    if (fields_0[offset].ofs != 132)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 566, 0, "%s", "fields[offset].ofs == HEOFS( elem.glowColor )");
    glowAlpha = Scr_GetFloat(0);
    v4 = glowAlpha - 1.0;
    if (v4 < 0.0)
        v6 = glowAlpha;
    else
        v6 = 1.0;
    v3 = 0.0 - v6;
    if (v3 < 0.0)
        v2 = v6;
    else
        v2 = 0.0;
    v5 = v2 * 255.0;
    hud->elem.glowColor.a = (int)(v5 + 9.313225746154785e-10);
}

void __cdecl HudElem_GetGlowAlpha(game_hudelem_s *hud, int offset)
{
    iassert(fields_0[offset].ofs == 36 /*HEOFS(elem.color)*/);

    Scr_AddFloat(hud->elem.glowColor.a * 0.003921568859368563);
}

void __cdecl HudElem_SetFontScale(game_hudelem_s *hud, int offset)
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    float scale; // [esp+10h] [ebp-4h]

    if (fields_0[offset].ofs != 20)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 585, 0, "%s", "fields[offset].ofs == HEOFS( elem.fontScale )");
    scale = Scr_GetFloat(0);
    if (scale <= 0.0)
    {
        v2 = va("font scale was %g; should be > 0", scale);
        Scr_Error(v2);
    }
    if (scale >= 1.399999976158142)
    {
        if (scale > 4.599999904632568)
        {
            v4 = va("font scale %f is above the expected maximum %f", scale, 4.599999904632568);
            Scr_Error(v4);
        }
    }
    else
    {
        v3 = va("font scale %f is below the expected minimum %f", scale, 1.399999976158142);
        Scr_Error(v3);
    }
    hud->elem.fontScale = scale;
}

void __cdecl HudElem_SetFont(game_hudelem_s *hud, int offset)
{
    HudElem_SetEnumString(hud, &fields_0[offset], g_he_font, 6);
}

void __cdecl HudElem_SetEnumString(
    game_hudelem_s *hud,
    const game_hudelem_field_t *f,
    const char **names,
    int nameCount)
{
    const char *v4; // eax
    char *selectedName; // [esp+0h] [ebp-814h]
    char errormsg[2052]; // [esp+4h] [ebp-810h] BYREF
    int nameIndex; // [esp+80Ch] [ebp-8h]
    int *value; // [esp+810h] [ebp-4h]

    if (!hud)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 321, 0, "%s", "hud");
    if (!f)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 322, 0, "%s", "f");
    if (!names)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 323, 0, "%s", "names");
    if (nameCount <= 0)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 324, 0, "%s\n\t(nameCount) = %i", "(nameCount > 0)", nameCount);
    value = (int *)((char *)hud + f->ofs);
    selectedName = Scr_GetString(0);
    for (nameIndex = 0; nameIndex < nameCount; ++nameIndex)
    {
        if (!I_stricmp(selectedName, names[nameIndex]))
        {
            *value &= ~(f->mask << f->shift);
            *value |= nameIndex << f->shift;
            return;
        }
    }
    sprintf(errormsg, "\"%s\" is not a valid value for hudelem field \"%s\"\nShould be one of:", selectedName, f->name);
    for (nameIndex = 0; nameIndex < nameCount; ++nameIndex)
    {
        v4 = va(" %s", names[nameIndex]);
        strncat(errormsg, v4, 0x800u);
        errormsg[2047] = 0;
    }
    Scr_Error(errormsg);
}

void __cdecl HudElem_GetFont(game_hudelem_s *hud, int offset)
{
    HudElem_GetEnumString(hud, &fields_0[offset], g_he_font, 6);
}

void __cdecl HudElem_GetEnumString(
    game_hudelem_s *hud,
    const game_hudelem_field_t *f,
    const char **names,
    int nameCount)
{
    int index; // [esp+0h] [ebp-8h]

    if (!hud)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 353, 0, "%s", "hud");
    if (!f)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 354, 0, "%s", "f");
    if (!names)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 355, 0, "%s", "names");
    if (nameCount <= 0)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 356, 0, "%s\n\t(nameCount) = %i", "(nameCount > 0)", nameCount);
    index = f->mask & (*(int *)((char *)&hud->elem.type + f->ofs) >> f->shift);
    if (index < 0 || index >= nameCount)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 360, 0, "%s\n\t(index) = %i", "(index >= 0 && index < nameCount)", index);
    Scr_AddString((char *)names[index]);
}

void __cdecl HudElem_SetAlignX(game_hudelem_s *hud, int offset)
{
    HudElem_SetEnumString(hud, &fields_0[offset], g_he_alignx, 3);
}

void __cdecl HudElem_GetAlignX(game_hudelem_s *hud, int offset)
{
    HudElem_GetEnumString(hud, &fields_0[offset], g_he_alignx, 3);
}

void __cdecl HudElem_SetAlignY(game_hudelem_s *hud, int offset)
{
    HudElem_SetEnumString(hud, &fields_0[offset], g_he_aligny, 3);
}

void __cdecl HudElem_GetAlignY(game_hudelem_s *hud, int offset)
{
    HudElem_GetEnumString(hud, &fields_0[offset], g_he_aligny, 3);
}

void __cdecl HudElem_SetHorzAlign(game_hudelem_s *hud, int offset)
{
    HudElem_SetEnumString(hud, &fields_0[offset], g_he_horzalign, 8);
}

void __cdecl HudElem_GetHorzAlign(game_hudelem_s *hud, int offset)
{
    HudElem_GetEnumString(hud, &fields_0[offset], g_he_horzalign, 8);
}

void __cdecl HudElem_SetVertAlign(game_hudelem_s *hud, int offset)
{
    HudElem_SetEnumString(hud, &fields_0[offset], g_he_vertalign, 8);
}

void __cdecl HudElem_GetVertAlign(game_hudelem_s *hud, int offset)
{
    HudElem_GetEnumString(hud, &fields_0[offset], g_he_vertalign, 8);
}

void __cdecl Scr_GetHudElemField(unsigned int entnum, unsigned int offset)
{
    game_hudelem_s *hud; // [esp+0h] [ebp-8h]
    const game_hudelem_field_t *f; // [esp+4h] [ebp-4h]

    if (offset >= 0x13)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 667, 0, "%s", "(unsigned)offset < ARRAY_COUNT( fields ) - 1");
    if (entnum >= 0x400)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 668, 0, "%s", "(unsigned)entnum < ARRAY_COUNT( g_hudelems )");
    f = &fields_0[offset];
    hud = &g_hudelems[entnum];
    if (f->getter)
        f->getter(hud, offset);
    else
        Scr_GetGenericField((unsigned __int8 *)hud, f->type, f->ofs);
}

void __cdecl Scr_SetHudElemField(unsigned int entnum, unsigned int offset)
{
    game_hudelem_s *hud; // [esp+0h] [ebp-8h]
    const game_hudelem_field_t *f; // [esp+4h] [ebp-4h]

    if (offset >= 0x13)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 688, 0, "%s", "(unsigned)offset < ARRAY_COUNT( fields ) - 1");
    if (entnum >= 0x400)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 689, 0, "%s", "(unsigned)entnum < ARRAY_COUNT( g_hudelems )");
    f = &fields_0[offset];
    hud = &g_hudelems[entnum];
    if (f->setter)
        f->setter(hud, offset);
    else
        Scr_SetGenericField((unsigned __int8 *)hud, f->type, f->ofs);
}

void __cdecl Scr_FreeHudElemConstStrings(game_hudelem_s *hud)
{
    const game_hudelem_field_t *f; // [esp+0h] [ebp-4h]

    for (f = fields_0; f->name; ++f)
    {
        if (f->type == F_STRING)
            Scr_SetString((unsigned __int16 *)((char *)hud + f->ofs), 0);
    }
}

void __cdecl GScr_NewHudElem()
{
    game_hudelem_s *hud; // [esp+0h] [ebp-4h]

    hud = HudElem_Alloc(1023, 0);
    if (!hud)
        Scr_Error("out of hudelems");
    Scr_AddHudElem(hud);
}

void __cdecl GScr_NewClientHudElem()
{
    game_hudelem_s *hud; // [esp+0h] [ebp-8h]
    gentity_s *ent; // [esp+4h] [ebp-4h]

    ent = Scr_GetEntity(0);
    if (!ent)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 759, 0, "%s", "ent");
    if (!ent->r.inuse)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 760, 0, "%s", "ent->r.inuse");
    if (!ent->client)
        Scr_ParamError(0, "not a client");
    hud = HudElem_Alloc(ent->s.number, 0);
    if (!hud)
        Scr_Error("out of hudelems");
    Scr_AddHudElem(hud);
}

void __cdecl GScr_NewTeamHudElem()
{
    game_hudelem_s *v0; // eax
    char *String; // eax
    const char *v2; // eax
    unsigned __int16 teamName; // [esp+0h] [ebp-Ch]
    game_hudelem_s *hud; // [esp+8h] [ebp-4h]

    teamName = Scr_GetConstString(0);
    if (teamName == scr_const.allies)
    {
        v0 = HudElem_Alloc(1023, 2);
    }
    else if (teamName == scr_const.axis)
    {
        v0 = HudElem_Alloc(1023, 1);
    }
    else if (teamName == scr_const.spectator)
    {
        v0 = HudElem_Alloc(1023, 3);
    }
    else
    {
        String = Scr_GetString(0);
        v2 = va("team \"%s\" should be \"allies\", \"axis\", or \"spectator\"", String);
        Scr_ParamError(0, v2);
        v0 = HudElem_Alloc(1023, 0);
    }
    hud = v0;
    if (!v0)
        Scr_Error("out of hudelems");
    Scr_AddHudElem(hud);
}

void __cdecl GScr_AddFieldsForHudElems()
{
    const game_hudelem_field_t *f; // [esp+4h] [ebp-4h]

    for (f = fields_0; f->name; ++f)
    {
        if (((f - fields_0) & 0xC000) != 0)
            MyAssertHandler(".\\game\\g_hudelem.cpp", 821, 0, "%s", "((f - fields) & ENTFIELD_MASK) == ENTFIELD_ENTITY");
        if (f - fields_0 != (unsigned __int16)(f - fields_0))
            MyAssertHandler(".\\game\\g_hudelem.cpp", 822, 0, "%s", "(f - fields) == (unsigned short)( f - fields )");
        Scr_AddClassField(1u, (char *)f->name, (unsigned __int16)(f - fields_0));
    }
}

void __cdecl HECmd_SetText(scr_entref_t entref)
{
    int v1; // eax
    char string[1024]; // [esp+0h] [ebp-408h] BYREF
    game_hudelem_s *hud; // [esp+404h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    HudElem_ClearTypeSettings(hud);
    Scr_ConstructMessageString(0, 0, "Hud Elem String", string, 0x400u);
    hud->elem.type = HE_TYPE_TEXT;
    v1 = G_LocalizedStringIndex(string);
    hud->elem.text = v1;
}

game_hudelem_s *__cdecl HECmd_GetHudElem(scr_entref_t entref)
{
    if (entref.classnum == 1)
    {
        if (entref.entnum >= 0x400u)
            MyAssertHandler(".\\game\\g_hudelem.cpp", 832, 0, "%s", "entref.entnum < ARRAY_COUNT( g_hudelems )");
        return &g_hudelems[entref.entnum];
    }
    else
    {
        Scr_ObjectError("not a hud element");
        return 0;
    }
}

void __cdecl HECmd_ClearAllTextAfterHudElem(scr_entref_t entref)
{
    int configStringIndex; // [esp+0h] [ebp-8h]
    game_hudelem_s *hud; // [esp+4h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    if (!hud->elem.text)
        Scr_Error("Hud elem doesn't reference any text.  Make sure to call setText before using clearAllTextAfterHudElem.");
    for (configStringIndex = hud->elem.text + 1; configStringIndex < 512; ++configStringIndex)
        SV_SetConfigstring(configStringIndex + 309, (char *)"");
}

void __cdecl HECmd_SetMaterial(scr_entref_t entref)
{
    char *String; // eax
    const char *v2; // eax
    const char *v3; // eax
    int width; // [esp+0h] [ebp-14h]
    int height; // [esp+4h] [ebp-10h]
    int materialIndex; // [esp+8h] [ebp-Ch]
    int numParam; // [esp+Ch] [ebp-8h]
    game_hudelem_s *hud; // [esp+10h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    numParam = Scr_GetNumParam();
    if (numParam != 1 && numParam != 3)
        Scr_Error("USAGE: <hudelem> setShader(\"materialname\"[, optional_width, optional_height]);");
    String = Scr_GetString(0);
    materialIndex = G_MaterialIndex(String);
    if (numParam == 1)
    {
        width = 0;
        height = 0;
    }
    else
    {
        width = Scr_GetInt(1);
        if (width < 0)
        {
            v2 = va("width %i < 0", width);
            Scr_ParamError(1u, v2);
        }
        height = Scr_GetInt(2);
        if (height < 0)
        {
            v3 = va("height %i < 0", height);
            Scr_ParamError(2u, v3);
        }
    }
    HudElem_ClearTypeSettings(hud);
    hud->elem.type = HE_TYPE_MATERIAL;
    hud->elem.materialIndex = materialIndex;
    hud->elem.width = width;
    hud->elem.height = height;
}

void __cdecl HECmd_SetTargetEnt(scr_entref_t entref)
{
    game_hudelem_s *hud; // [esp+0h] [ebp-8h]
    gentity_s *ent; // [esp+4h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    ent = Scr_GetEntity(0);
    hud->elem.targetEntNum = ent->s.number;
    if ((ent->r.svFlags & 0x10) == 0)
        Com_PrintWarning(15, "SetTargetEnt() called on a non-broadcasting entity, may not show in client snapshots.");
}

void __cdecl HECmd_ClearTargetEnt(scr_entref_t entref)
{
    HECmd_GetHudElem(entref)->elem.targetEntNum = 1023;
}

void __cdecl HECmd_SetTimer(scr_entref_t entref)
{
    HECmd_SetTimer_Internal(entref, HE_TYPE_TIMER_DOWN, "setTimer");
}

void __cdecl HECmd_SetTimer_Internal(scr_entref_t entref, he_type_t type, const char *cmdName)
{
    const char *v3; // eax
    const char *v4; // eax
    float v5; // [esp+8h] [ebp-18h]
    game_hudelem_s *hud; // [esp+18h] [ebp-8h]
    int time; // [esp+1Ch] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    if (type != HE_TYPE_TIMER_DOWN
        && type != HE_TYPE_TIMER_UP
        && type != HE_TYPE_TENTHS_TIMER_DOWN
        && type != HE_TYPE_TENTHS_TIMER_UP)
    {
        MyAssertHandler(
            ".\\game\\g_hudelem.cpp",
            998,
            0,
            "%s\n\t(type) = %i",
            "(type == HE_TYPE_TIMER_DOWN || type == HE_TYPE_TIMER_UP || type == HE_TYPE_TENTHS_TIMER_DOWN || type == HE_TYPE_TENTHS_TIMER_UP)",
            type);
    }
    if (Scr_GetNumParam() != 1)
    {
        v3 = va("USAGE: <hudelem> %s(time_in_seconds);\n", cmdName);
        Scr_Error(v3);
    }
    v5 = Scr_GetFloat(0) * 1000.0;
    time = (int)(v5 + 0.4999999990686774);
    if (time <= 0 && type != HE_TYPE_TIMER_UP)
    {
        v4 = va("time %g should be > 0", (double)time * EQUAL_EPSILON);
        Scr_ParamError(0, v4);
    }
    HudElem_ClearTypeSettings(hud);
    hud->elem.type = type;
    hud->elem.time = time + level.time;
}

void __cdecl HECmd_SetTimerUp(scr_entref_t entref)
{
    HECmd_SetTimer_Internal(entref, HE_TYPE_TIMER_UP, "setTimerUp");
}

void __cdecl HECmd_SetTenthsTimer(scr_entref_t entref)
{
    HECmd_SetTimer_Internal(entref, HE_TYPE_TENTHS_TIMER_DOWN, "setTenthsTimer");
}

void __cdecl HECmd_SetTenthsTimerUp(scr_entref_t entref)
{
    HECmd_SetTimer_Internal(entref, HE_TYPE_TENTHS_TIMER_UP, "setTenthsTimerUp");
}

void __cdecl HECmd_SetClock(scr_entref_t entref)
{
    HECmd_SetClock_Internal(entref, HE_TYPE_CLOCK_DOWN, "setClock");
}

void __cdecl HECmd_SetClock_Internal(scr_entref_t entref, he_type_t type, const char *cmdName)
{
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    char *String; // eax
    const char *v7; // eax
    const char *v8; // eax
    float v9; // [esp+8h] [ebp-3Ch]
    float v10; // [esp+18h] [ebp-2Ch]
    int duration; // [esp+28h] [ebp-1Ch]
    int materialIndex; // [esp+2Ch] [ebp-18h]
    int width; // [esp+30h] [ebp-14h]
    int height; // [esp+34h] [ebp-10h]
    int numParam; // [esp+38h] [ebp-Ch]
    game_hudelem_s *hud; // [esp+3Ch] [ebp-8h]
    int time; // [esp+40h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    if (type != HE_TYPE_CLOCK_DOWN && type != HE_TYPE_CLOCK_UP)
        MyAssertHandler(
            ".\\game\\g_hudelem.cpp",
            1024,
            0,
            "%s\n\t(type) = %i",
            "(type == HE_TYPE_CLOCK_DOWN || type == HE_TYPE_CLOCK_UP)",
            type);
    numParam = Scr_GetNumParam();
    if (numParam != 3 && numParam != 5)
    {
        v3 = va(
            "USAGE: <hudelem> %s(time_in_seconds, total_clock_time_in_seconds, shadername[, width, height]);\n",
            cmdName);
        Scr_Error(v3);
    }
    v10 = Scr_GetFloat(0) * 1000.0;
    time = (int)(v10 + 0.4999999990686774);
    if (time <= 0 && type != HE_TYPE_CLOCK_UP)
    {
        v4 = va("time %g should be > 0", (double)time * EQUAL_EPSILON);
        Scr_ParamError(0, v4);
    }
    v9 = Scr_GetFloat(1u) * 1000.0;
    duration = (int)(v9 + 0.4999999990686774);
    if (duration <= 0)
    {
        v5 = va("duration %g should be > 0", (double)duration * EQUAL_EPSILON);
        Scr_ParamError(1u, v5);
    }
    String = Scr_GetString(2u);
    materialIndex = G_MaterialIndex(String);
    if (numParam == 3)
    {
        width = 0;
        height = 0;
    }
    else
    {
        width = Scr_GetInt(3);
        if (width < 0)
        {
            v7 = va("width %i < 0", width);
            Scr_ParamError(3u, v7);
        }
        height = Scr_GetInt(4);
        if (height < 0)
        {
            v8 = va("height %i < 0", height);
            Scr_ParamError(4u, v8);
        }
    }
    HudElem_ClearTypeSettings(hud);
    hud->elem.type = type;
    hud->elem.time = time + level.time;
    hud->elem.duration = duration;
    hud->elem.materialIndex = materialIndex;
    hud->elem.width = width;
    hud->elem.height = height;
}

void __cdecl HECmd_SetClockUp(scr_entref_t entref)
{
    HECmd_SetClock_Internal(entref, HE_TYPE_CLOCK_UP, "setClockUp");
}

void __cdecl HECmd_SetValue(scr_entref_t entref)
{
    game_hudelem_s *hud; // [esp+0h] [ebp-8h]
    float value; // [esp+4h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    value = Scr_GetFloat(0);
    HudElem_ClearTypeSettings(hud);
    hud->elem.type = HE_TYPE_VALUE;
    hud->elem.value = value;
}

void __cdecl HECmd_SetWaypoint(scr_entref_t entref)
{
    VariableUnion v1; // eax
    char *String; // eax
    int numParam; // [esp+4h] [ebp-8h]
    game_hudelem_s *hud; // [esp+8h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    numParam = Scr_GetNumParam();
    v1.intValue = Scr_GetInt(0);
    hud->elem.type = HE_TYPE_WAYPOINT;
    hud->elem.value = (float)v1.intValue;
    if (numParam == 1)
    {
        hud->elem.offscreenMaterialIdx = 0;
    }
    else
    {
        String = Scr_GetString(1u);
        hud->elem.offscreenMaterialIdx = G_MaterialIndex(String);
    }
}

void __cdecl HECmd_FadeOverTime(scr_entref_t entref)
{
    const char *v1; // eax
    const char *v2; // eax
    float v3; // [esp+Ch] [ebp-18h]
    float fadeTime; // [esp+1Ch] [ebp-8h]
    game_hudelem_s *hud; // [esp+20h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    fadeTime = Scr_GetFloat(0);
    if (fadeTime > 0.0)
    {
        if (fadeTime > 60.0)
        {
            v2 = va("fade time %g > 60", fadeTime);
            Scr_ParamError(0, v2);
        }
    }
    else
    {
        v1 = va("fade time %g <= 0", fadeTime);
        Scr_ParamError(0, v1);
    }
    BG_LerpHudColors(&hud->elem, level.time, &hud->elem.fromColor);
    hud->elem.fadeStartTime = level.time;
    v3 = fadeTime * 1000.0;
    hud->elem.fadeTime = (int)(v3 + 9.313225746154785e-10);
}

void __cdecl HECmd_ScaleOverTime(scr_entref_t entref)
{
    const char *v1; // eax
    const char *v2; // eax
    float v3; // [esp+Ch] [ebp-20h]
    int width; // [esp+1Ch] [ebp-10h]
    int height; // [esp+20h] [ebp-Ch]
    game_hudelem_s *hud; // [esp+24h] [ebp-8h]
    float scaleTime; // [esp+28h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    if (Scr_GetNumParam() != 3)
        Scr_Error("hudelem scaleOverTime(time_in_seconds, new_width, new_height)");
    scaleTime = Scr_GetFloat(0);
    if (scaleTime > 0.0)
    {
        if (scaleTime > 60.0)
        {
            v2 = va("scale time %g > 60", scaleTime);
            Scr_ParamError(0, v2);
        }
    }
    else
    {
        v1 = va("scale time %g <= 0", scaleTime);
        Scr_ParamError(0, v1);
    }
    width = Scr_GetInt(1);
    height = Scr_GetInt(2);
    hud->elem.scaleStartTime = level.time;
    v3 = scaleTime * 1000.0;
    hud->elem.scaleTime = (int)(v3 + 9.313225746154785e-10);
    hud->elem.fromWidth = hud->elem.width;
    hud->elem.fromHeight = hud->elem.height;
    hud->elem.width = width;
    hud->elem.height = height;
}

void __cdecl HECmd_MoveOverTime(scr_entref_t entref)
{
    const char *v1; // eax
    const char *v2; // eax
    float v3; // [esp+Ch] [ebp-18h]
    game_hudelem_s *hud; // [esp+1Ch] [ebp-8h]
    float moveTime; // [esp+20h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    moveTime = Scr_GetFloat(0);
    if (moveTime > 0.0)
    {
        if (moveTime > 60.0)
        {
            v2 = va("move time %g > 60", moveTime);
            Scr_ParamError(0, v2);
        }
    }
    else
    {
        v1 = va("move time %g <= 0", moveTime);
        Scr_ParamError(0, v1);
    }
    hud->elem.moveStartTime = level.time;
    v3 = moveTime * 1000.0;
    hud->elem.moveTime = (int)(v3 + 9.313225746154785e-10);
    hud->elem.fromX = hud->elem.x;
    hud->elem.fromY = hud->elem.y;
    hud->elem.fromAlignOrg = hud->elem.alignOrg;
    hud->elem.fromAlignScreen = hud->elem.alignScreen;
}

void __cdecl HECmd_Reset(scr_entref_t entref)
{
    game_hudelem_s *hud; // [esp+0h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    HudElem_SetDefaults(hud);
}

void __cdecl HECmd_Destroy(scr_entref_t entref)
{
    game_hudelem_s *hud; // [esp+0h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    HudElem_Free(hud);
}

void __cdecl HECmd_SetPlayerNameString(scr_entref_t entref)
{
    game_hudelem_s *hud; // [esp+0h] [ebp-8h]
    gentity_s *player; // [esp+4h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    player = Scr_GetEntity(0);
    if (player)
    {
        if (player->client)
        {
            HudElem_ClearTypeSettings(hud);
            hud->elem.type = HE_TYPE_PLAYERNAME;
            hud->elem.value = (float)player->s.number;
        }
        else
        {
            Com_Printf(23, "Invalid entity passed to hudelem setplayernamestring(), entity is not a client\n");
        }
    }
    else
    {
        Com_Printf(23, "Invalid entity passed to hudelem setplayernamestring()\n");
    }
}

void __cdecl HECmd_SetGameTypeString(scr_entref_t entref)
{
    char *gametype; // [esp+0h] [ebp-8h]
    game_hudelem_s *hud; // [esp+4h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    gametype = Scr_GetString(0);
    if (gametype)
    {
        if (Scr_GetGameTypeNameForScript(gametype))
        {
            SV_SetConfigstring(18, gametype);
            HudElem_ClearTypeSettings(hud);
            hud->elem.type = HE_TYPE_GAMETYPE;
            hud->elem.value = 18.0;
        }
        else
        {
            Com_Printf(23, "Invalid gametype '%s'\n", gametype);
        }
    }
    else
    {
        Com_Printf(23, "Invalid entity passed to hudelem setgametypestring()\n");
    }
}

void __cdecl HECmd_SetMapNameString(scr_entref_t entref)
{
    game_hudelem_s *hud; // [esp+0h] [ebp-8h]
    char *mapname; // [esp+4h] [ebp-4h]

    hud = HECmd_GetHudElem(entref);
    mapname = Scr_GetString(0);
    if (mapname)
    {
        if (SV_MapExists(mapname))
        {
            SV_SetConfigstring(17, mapname);
            HudElem_ClearTypeSettings(hud);
            hud->elem.type = HE_TYPE_MAPNAME;
        }
        else
        {
            Com_Printf(23, "Invalid map name passed to hudelem setmapnamestring(), map not found\n");
        }
    }
    else
    {
        Com_Printf(23, "Invalid mapname passed to hudelem setmapnamestring()\n");
    }
}

void __cdecl HECmd_SetPulseFX(scr_entref_t entref)
{
    VariableUnion *hud; // [esp+4h] [ebp-8h]
    gclient_s *ps; // [esp+8h] [ebp-4h]

    if (Scr_GetNumParam() != 3)
        Scr_Error("USAGE: <hudelem> SetPulseFX( <speed>, <decayStart>, <decayDuration> );\n");
    hud = (VariableUnion *)HECmd_GetHudElem(entref);
    hud[34].intValue = level.time;
    hud[35].intValue = GetIntGTZero(0).intValue;
    hud[36].intValue = GetIntGTZero(1u).intValue;
    hud[37].intValue = GetIntGTZero(2u).intValue;
    ps = &level.clients[hud[40].intValue];
    ++ps->ps.hudElemLastAssignedSoundID;
    ps->ps.hudElemLastAssignedSoundID %= 32;
    if (!ps->ps.hudElemLastAssignedSoundID)
        ++ps->ps.hudElemLastAssignedSoundID;
    hud[38].intValue = ps->ps.hudElemLastAssignedSoundID;
}

VariableUnion __cdecl GetIntGTZero(unsigned int index)
{
    const char *v1; // eax
    int number; // [esp+0h] [ebp-4h]

    number = Scr_GetInt(index);
    if (number < 0)
    {
        v1 = va("Time (%i) must be greater than zero.", number);
        Scr_ParamError(index, v1);
    }
    return (VariableUnion)number;
}

const BuiltinMethodDef methods_0[22] =
{
  { "settext", &HECmd_SetText, 0 },
  { "clearalltextafterhudelem", &HECmd_ClearAllTextAfterHudElem, 0 },
  { "setshader", &HECmd_SetMaterial, 0 },
  { "settargetent", &HECmd_SetTargetEnt, 0 },
  { "cleartargetent", &HECmd_ClearTargetEnt, 0 },
  { "settimer", &HECmd_SetTimer, 0 },
  { "settimerup", &HECmd_SetTimerUp, 0 },
  { "settenthstimer", &HECmd_SetTenthsTimer, 0 },
  { "settenthstimerup", &HECmd_SetTenthsTimerUp, 0 },
  { "setclock", &HECmd_SetClock, 0 },
  { "setclockup", &HECmd_SetClockUp, 0 },
  { "setvalue", &HECmd_SetValue, 0 },
  { "setwaypoint", &HECmd_SetWaypoint, 0 },
  { "fadeovertime", &HECmd_FadeOverTime, 0 },
  { "scaleovertime", &HECmd_ScaleOverTime, 0 },
  { "moveovertime", &HECmd_MoveOverTime, 0 },
  { "reset", &HECmd_Reset, 0 },
  { "destroy", &HECmd_Destroy, 0 },
  { "setpulsefx", &HECmd_SetPulseFX, 0 },
  { "setplayernamestring", &HECmd_SetPlayerNameString, 0 },
  { "setmapnamestring", &HECmd_SetMapNameString, 0 },
  { "setgametypestring", &HECmd_SetGameTypeString, 0 }
};

void(__cdecl *__cdecl HudElem_GetMethod(const char **pName))(scr_entref_t)
{
    unsigned int i; // [esp+18h] [ebp-4h]

    for (i = 0; i < 0x16; ++i)
    {
        if (!strcmp(*pName, methods_0[i].actionString))
        {
            *pName = methods_0[i].actionString;
            return methods_0[i].actionFunc;
        }
    }
    return 0;
}

hudelem_s g_dummyHudCurrent_0;
void __cdecl HudElem_UpdateClient(gclient_s *client, int clientNum, hudelem_update_t which)
{
    int archivalCount; // [esp+8h] [ebp-14h]
    int currentCount; // [esp+Ch] [ebp-10h]
    game_hudelem_s *hud; // [esp+10h] [ebp-Ch]
    unsigned int i; // [esp+14h] [ebp-8h]
    hudelem_s *elem; // [esp+18h] [ebp-4h]

    if (clientNum < 0 || clientNum >= level.maxclients)
        MyAssertHandler(
            ".\\game\\g_hudelem.cpp",
            1650,
            0,
            "%s\n\t(clientNum) = %i",
            "(clientNum >= 0 && clientNum < level.maxclients)",
            clientNum);
    if (!level.gentities[clientNum].r.inuse)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 1651, 0, "%s", "level.gentities[clientNum].r.inuse");
    if (!client)
        MyAssertHandler(".\\game\\g_hudelem.cpp", 1652, 0, "%s", "client");
    archivalCount = 0;
    currentCount = 0;
    hud = g_hudelems;
    for (i = 0; i < 0x400; ++i)
    {
        if (hud->elem.type
            && (!hud->team || hud->team == client->sess.cs.team)
            && (hud->clientNum == 1023 || hud->clientNum == clientNum))
        {
            if (hud->archived)
            {
                if ((which & 1) != 0)
                {
                    elem = &client->ps.hud.archival[archivalCount];
                    if (archivalCount < 31)
                    {
                        ++archivalCount;
                    LABEL_23:
                        memcpy(elem, hud, sizeof(hudelem_s));
                    }
                }
            }
            else if ((which & 2) != 0)
            {
                elem = &client->ps.hud.current[currentCount];
                if (currentCount < 31)
                {
                    ++currentCount;
                    goto LABEL_23;
                }
            }
        }
        ++hud;
    }
    if ((which & 1) != 0)
    {
        while (archivalCount < 31 && client->ps.hud.archival[archivalCount].type)
        {
            memset(
                (unsigned __int8 *)&client->ps.hud.archival[archivalCount],
                0,
                sizeof(client->ps.hud.archival[archivalCount]));
            if (client->ps.hud.archival[archivalCount].type)
                MyAssertHandler(
                    ".\\game\\g_hudelem.cpp",
                    1697,
                    0,
                    "%s",
                    "client->ps.hud.archival[archivalCount].type == HE_TYPE_FREE");
            ++archivalCount;
        }
        while (archivalCount < 31)
        {
            if (memcmp(&client->ps.hud.archival[archivalCount], &g_dummyHudCurrent_0, 0xA0u))
                MyAssertHandler(
                    ".\\game\\g_hudelem.cpp",
                    1704,
                    0,
                    "%s",
                    "!memcmp( &client->ps.hud.archival[archivalCount], &g_dummyHudCurrent, sizeof( g_dummyHudCurrent ) )");
            ++archivalCount;
        }
    }
    if ((which & 2) != 0)
    {
        while (currentCount < 31 && client->ps.hud.current[currentCount].type)
        {
            memset((unsigned __int8 *)&client->ps.hud.current[currentCount], 0, sizeof(client->ps.hud.current[currentCount]));
            if (client->ps.hud.current[currentCount].type)
                MyAssertHandler(
                    ".\\game\\g_hudelem.cpp",
                    1715,
                    0,
                    "%s",
                    "client->ps.hud.current[currentCount].type == HE_TYPE_FREE");
            ++currentCount;
        }
        while (currentCount < 31)
        {
            if (memcmp(&client->ps.hud.current[currentCount], &g_dummyHudCurrent_0, 0xA0u))
                MyAssertHandler(
                    ".\\game\\g_hudelem.cpp",
                    1722,
                    0,
                    "%s",
                    "!memcmp( &client->ps.hud.current[currentCount], &g_dummyHudCurrent, sizeof( g_dummyHudCurrent ) )");
            ++currentCount;
        }
    }
}

