#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "g_local.h"
#include <cgame/cg_view.h>
#include "g_main.h"
#include <cgame/cg_ents.h>
#include <xanim/xanim.h>
#include <server/sv_game.h>
#include <script/scr_const.h>
#include <qcommon/com_bsp.h>

float __cdecl G_GetEntInfoScale()
{
    return CG_GetViewZoomScale() * g_entinfo_scale->current.value;
}

void __cdecl SP_info_notnull(gentity_s *self)
{
    G_SetOrigin(self, self->r.currentOrigin);
}

void __cdecl SP_light(gentity_s *self)
{
    const ComPrimaryLight *light; // r30
    unsigned int primaryLightIndex; // [sp+50h] [-80h] BYREF
    float facingDir[4]; // [sp+58h] [-78h] BYREF
    float normalizedColor[4]; // [sp+68h] [-68h] BYREF
    float facingAngles[6]; // [sp+78h] [-58h] BYREF

    iassert(level.spawnVar.spawnVarsValid);

    if (G_SpawnInt("pl#", "0", (int *)&primaryLightIndex))
    {
        light = Com_GetPrimaryLight(primaryLightIndex);

        if (!VecNCompareCustomEpsilon(light->origin, self->r.currentOrigin, 1.0, 3))
        {
            primaryLightIndex = Com_FindClosestPrimaryLight(self->r.currentOrigin);
            light = Com_GetPrimaryLight(primaryLightIndex);
            if (!VecNCompareCustomEpsilon(light->origin, self->r.currentOrigin, 1.0, 3))
                Com_PrintError(
                    1,
                    "No primary light was found at (%.0f %.0f %.0f).  You may have added, deleted, or moved a primary light since the last full map compile.  You should recompile the map before using MyMapEnts to avoid issues with primary lights.\n",
                    self->r.currentOrigin[0],
                    self->r.currentOrigin[1],
                    self->r.currentOrigin[2]
                );
        }

        self->s.index.item = primaryLightIndex;

        iassert(self->s.index.primaryLight == primaryLightIndex);

        self->s.lerp.u.primaryLight.intensity = ColorNormalize(&light->color[0], normalizedColor);
        Byte4PackRgba(normalizedColor, &self->s.lerp.u.primaryLight.colorAndExp[0]);
        self->s.lerp.u.primaryLight.colorAndExp[3] = light->exponent;
        self->s.lerp.u.primaryLight.radius = light->radius;
        self->s.lerp.u.primaryLight.cosHalfFovOuter = light->cosHalfFovOuter;
        self->s.lerp.u.primaryLight.cosHalfFovInner = light->cosHalfFovInner;
        facingDir[0] = -light->dir[0];
        facingDir[1] = -light->dir[1];
        facingDir[2] = -light->dir[2];
        vectoangles(facingDir, facingAngles);
        G_SetAngle(self, facingAngles);
        G_SetOrigin(self, (float*)light->origin);

        self->r.mins[0] = -light->radius;
        self->r.mins[1] = -light->radius;
        self->r.mins[2] = -light->radius;

        self->r.maxs[0] = light->radius;
        self->r.maxs[1] = light->radius;
        self->r.maxs[2] = light->radius;

        self->s.eType = ET_PRIMARY_LIGHT;

        iassert(self->r.contents == 0);

        self->handler = ENT_HANDLER_PRIMARY_LIGHT;
        SV_LinkEntity(self);
    }
    else
    {
        G_FreeEntity(self);
    }
}

void __cdecl SP_info_volume(gentity_s *self)
{
    int v2; // r11

    if (SV_SetBrushModel(self))
    {
        v2 = self->s.lerp.eFlags | 1;
        self->r.contents = 0;
        self->r.svFlags = 1;
        self->s.lerp.eFlags = v2;
        SV_LinkEntity(self);
    }
    else
    {
        Com_PrintError(
            1,
            "Killing info_volume at (%f %f %f) because the brush model is invalid.\n",
            self->s.lerp.pos.trBase[0],
            self->s.lerp.pos.trBase[1],
            self->s.lerp.pos.trBase[2]
        );
        G_FreeEntity(self);
    }
}

void __cdecl TeleportPlayer(gentity_s *player, float *origin, const float *angles)
{
    SetClientOrigin(player, origin);
    SetClientViewAngle(player, angles);
    if (!player->tagInfo)
        player->r.currentAngles[0] = 0.0;
    SV_LinkEntity(player);
}

void __cdecl SP_sound_blend(gentity_s *self)
{
    self->s.lerp.u.turret.gunAngles[0] = 0.0;
    self->r.contents = 0;
    self->s.eType = ET_SOUND_BLEND;
    self->s.lerp.pos.trType = TR_STATIONARY;
    self->s.lerp.apos.trType = TR_STATIONARY;
    self->s.eventParms[0] = 0;
    self->s.lerp.u.turret.gunAngles[1] = 1.0;
    self->s.eventParms[1] = 0;
}

gentity_s *__cdecl G_SpawnSoundBlend()
{
    gentity_s *v0; // r31
    gentity_s *result; // r3

    v0 = G_Spawn();
    Scr_SetString(&v0->classname, scr_const.sound_blend);
    result = v0;
    v0->s.lerp.u.turret.gunAngles[0] = 0.0;
    v0->r.contents = 0;
    v0->s.eType = ET_SOUND_BLEND;
    v0->s.lerp.pos.trType = TR_STATIONARY;
    v0->s.lerp.apos.trType = TR_STATIONARY;
    v0->s.eventParms[0] = 0;
    v0->s.lerp.u.turret.gunAngles[1] = 1.0;
    v0->s.eventParms[1] = 0;
    return result;
}

void __cdecl G_SetSoundBlend(gentity_s *ent, unsigned __int16 alias0, unsigned __int16 alias1, double lerp)
{
    if (!ent->r.inuse)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_misc.cpp", 311, 0, "%s", "ent->r.inuse");
    if (ent->s.eType != 6)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_misc.cpp", 312, 0, "%s", "ent->s.eType == ET_SOUND_BLEND");
    ent->s.lerp.u.turret.gunAngles[0] = lerp;
    ent->s.eventParms[0] = alias0;
    ent->s.eventParms[1] = alias1;
    SV_LinkEntity(ent);
}

void __cdecl G_SetSoundBlendVolumeScale(gentity_s *ent, double scale)
{
    ent->s.lerp.u.turret.gunAngles[1] = scale;
}

float __cdecl G_GetSoundBlendVolumeScale(gentity_s *ent)
{
    double v1; // fp1

    v1 = ent->s.lerp.u.turret.gunAngles[1];
    return *((float *)&v1 + 1);
}

void __cdecl EntinfoPosAndScale(gentity_s *self, float *source, float *pos, float *textScale, float *dist)
{
    double v10; // fp12
    double v11; // fp0
    double v12; // fp0
    const dvar_s *v13; // r11
    double v14; // fp13
    double v15; // fp12
    double v16; // fp0
    double value; // fp13

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_misc.cpp", 28, 0, "%s", "self");
    *pos = self->r.absmin[0] + self->r.absmax[0];
    pos[1] = self->r.absmin[1] + self->r.absmax[1];
    pos[2] = self->r.absmin[2] + self->r.absmax[2];
    v10 = (float)(pos[1] * (float)0.5);
    v11 = (float)(pos[2] * (float)0.5);
    *pos = *pos * (float)0.5;
    pos[1] = v10;
    pos[2] = v11;
    *textScale = CG_GetViewZoomScale() * g_entinfo_scale->current.value;
    v12 = (float)(*source - self->r.currentOrigin[0]);
    v13 = g_entinfo;
    v14 = (float)(source[2] - self->r.currentOrigin[2]);
    v15 = (float)(source[1] - self->r.currentOrigin[1]);
    v16 = sqrtf((float)((float)((float)v15 * (float)v15)
        + (float)((float)((float)v14 * (float)v14) + (float)((float)v12 * (float)v12))));
    *dist = v16;
    if (v13->current.integer >= 2 || (value = g_entinfo_maxdist->current.value, value <= 0.0) || v16 <= value)
        *textScale = (float)(*textScale * (float)v16) * (float)0.0026041667;
}


void __cdecl misc_EntInfo(gentity_s *self, float *source)
{
    static const float color[4] = { 0.5, 0.5, 0.5, 1.0 };

    const float *v4; // r6
    int integer; // r11
    const char *v6; // r30
    const char *v7; // r3
    const char *v8; // r5
    char *txt; // r3
    float textScale; // [sp+50h] [-40h] BYREF
    float dist; // [sp+54h] [-3Ch] BYREF
    float origin[14]; // [sp+58h] [-38h] BYREF

    iassert(self);
    EntinfoPosAndScale(self, source, origin, &textScale, &dist);
    G_DebugBox(self->r.currentOrigin, self->r.mins, self->r.maxs, self->r.currentAngles[1], colorMagenta, 1, 0);
    iassert(self->classname);
    integer = g_entinfo->current.integer;
    if (integer == 4 || integer == 5)
    {
        txt = va("%i", self->s.number);
    }
    else
    {
        if (self->targetname)
            v6 = SL_ConvertToString(self->targetname);
        else
            v6 = "<noname>";
        v7 = SL_ConvertToString(self->classname);
        txt = va("%i : %s : %i : %s", self->s.number, v6, self->health, v7);
    }
    G_AddDebugString(origin, color, (textScale * 0.75f), txt);
}

void __cdecl EntInfo_Item(gentity_s *self, float *source)
{
    static float MY_MAX_DIST = 500.0f;
    static float MY_MAX_DIST_HALF = 0.0f;
    static float MY_RGB[3] = { 0.6f, 0.5f, 0.5f };
    static unsigned int _S1 = 0;
    static float MY_NEXTLINE = -3.0f;

    double v4; // fp0
    int *p_index; // r30
    int v6; // r25
    double v7; // fp31
    WeaponDef *WeaponDef; // r3
    char *v9; // r3
    const char *v10; // r5
    const float *v11; // r6
    float dist; // [sp+50h] [-80h] BYREF
    float textScale; // [sp+54h] [-7Ch] BYREF
    float pos[3]; // [sp+58h] [-78h] BYREF
    //float v15; // [sp+60h] [-70h]
    float v16[6]; // [sp+70h] [-60h] BYREF

    if ((_S1 & 1) == 0)
    {
        MY_MAX_DIST_HALF = MY_MAX_DIST * 0.5f;
        _S1 |= 1u;
    }

    iassert(self);

    EntinfoPosAndScale(self, source, pos, &textScale, &dist);

    if (dist <= (double)MY_MAX_DIST)
    {
        v16[0] = MY_RGB[0];
        v16[1] = MY_RGB[1];
        v16[2] = MY_RGB[2];
        if (dist >= (double)MY_MAX_DIST_HALF)
            v4 = (float)(1.0 - (float)((float)(dist - MY_MAX_DIST_HALF) / MY_MAX_DIST_HALF));
        else
            v4 = 1.0;
        v16[3] = v4;
        p_index = &self->item[0].index;
        v6 = 2;
        pos[2] = -(float)((float)(MY_NEXTLINE * (float)0.5) - pos[2]);
        v7 = textScale;
        do
        {
            if (*p_index)
            {
                WeaponDef = BG_GetWeaponDef(*p_index);
                v9 = va("%i: %s (%i + %i)", self->s.number, WeaponDef->szInternalName, *(p_index - 1), *(p_index - 2));
                G_AddDebugString(pos, v16, (float)((float)v7 * 0.8f), v9);
                G_DebugBox(
                    self->r.currentOrigin,
                    self->r.mins,
                    self->r.maxs,
                    self->r.currentAngles[1],
                    colorRedFaded,
                    1,
                    0);
                pos[2] = MY_NEXTLINE + pos[2];
            }
            --v6;
            p_index += 3;
        } while (v6);
    }
}

int ByteFromFloatColor(float from)
{
    int value = (int)(from * 255.0 + 0.5);

    if (value < 0)
        return 0;
    if (value > 255)
        return 255;

    return value;
}