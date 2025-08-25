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
    double v0; // fp1

    v0 = (float)(CG_GetViewZoomScale() * g_entinfo_scale->current.value);
    return *((float *)&v0 + 1);
}

void __cdecl SP_info_notnull(gentity_s *self)
{
    G_SetOrigin(self, self->r.currentOrigin);
}

void __cdecl SP_light(gentity_s *self)
{
    const ComPrimaryLight *PrimaryLight; // r30
    const float *origin; // r28
    int v4; // r5
    int v5; // r5
    unsigned int v6; // r8
    int v7; // r7
    bool v8; // cr58
    double v9; // fp0
    double radius; // fp0
    unsigned int ClosestPrimaryLight; // [sp+50h] [-80h] BYREF
    float v13[4]; // [sp+58h] [-78h] BYREF
    float v14[4]; // [sp+68h] [-68h] BYREF
    float v15[6]; // [sp+78h] [-58h] BYREF

    if (!level.spawnVar.spawnVarsValid)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_misc.cpp", 178, 0, "%s", "level.spawnVar.spawnVarsValid");
    if (G_SpawnInt("pl#", "0", (int *)&ClosestPrimaryLight))
    {
        PrimaryLight = Com_GetPrimaryLight(ClosestPrimaryLight);
        origin = PrimaryLight->origin;
        if (!VecNCompareCustomEpsilon(PrimaryLight->origin, self->r.currentOrigin, 1.0, v4))
        {
            ClosestPrimaryLight = Com_FindClosestPrimaryLight(self->r.currentOrigin);
            PrimaryLight = Com_GetPrimaryLight(ClosestPrimaryLight);
            origin = PrimaryLight->origin;
            if (!VecNCompareCustomEpsilon(PrimaryLight->origin, self->r.currentOrigin, 1.0, v5))
                Com_PrintError(
                    1,
                    "No primary light was found at (%.0f %.0f %.0f).  You may have added, deleted, or moved a primary light since the last full map compile.  You should recompile the map before using MyMapEnts to avoid issues with primary lights.\n",
                    self->r.currentOrigin[0],
                    self->r.currentOrigin[1],
                    self->r.currentOrigin[2]
                );
        }
        v6 = ClosestPrimaryLight;
        v7 = ClosestPrimaryLight & 0xFFFF;
        v8 = (ClosestPrimaryLight & 0xFFFF) == ClosestPrimaryLight;
        //*(_WORD *)self->s.index = ClosestPrimaryLight;
        self->s.index.item = ClosestPrimaryLight;
        if (!v8)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_misc.cpp",
                198,
                1,
                "self->s.index.primaryLight == primaryLightIndex\n\t%i, %i",
                v7,
                v6);
        self->s.lerp.u.turret.gunAngles[1] = ColorNormalize(PrimaryLight->color, v14);
        Byte4PackRgba(v14, (unsigned __int8 *)&self->s.lerp.u);
        self->s.lerp.u.primaryLight.colorAndExp[3] = PrimaryLight->exponent;
        self->s.lerp.u.turret.gunAngles[2] = PrimaryLight->radius;
        self->s.lerp.u.primaryLight.cosHalfFovOuter = PrimaryLight->cosHalfFovOuter;
        self->s.lerp.u.primaryLight.cosHalfFovInner = PrimaryLight->cosHalfFovInner;
        v13[0] = -PrimaryLight->dir[0];
        v13[1] = -PrimaryLight->dir[1];
        v13[2] = -PrimaryLight->dir[2];
        vectoangles(v13, v15);
        G_SetAngle(self, v15);
        G_SetOrigin(self, (float*)origin);
        v9 = -PrimaryLight->radius;
        self->r.mins[0] = v9;
        self->r.mins[2] = v9;
        self->r.mins[1] = v9;
        radius = PrimaryLight->radius;
        self->r.maxs[0] = PrimaryLight->radius;
        self->r.maxs[2] = radius;
        self->r.maxs[1] = radius;
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
    char *v9; // r3
    float v10; // [sp+50h] [-40h] BYREF
    float v11; // [sp+54h] [-3Ch] BYREF
    float v12[14]; // [sp+58h] [-38h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_misc.cpp", 53, 0, "%s", "self");
    EntinfoPosAndScale(self, source, v12, &v10, &v11);
    G_DebugBox(self->r.currentOrigin, self->r.mins, self->r.maxs, self->r.currentAngles[1], v4, (int)colorMagenta, 1);
    if (!self->classname)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_misc.cpp", 58, 0, "%s", "self->classname");
    integer = g_entinfo->current.integer;
    if (integer == 4 || integer == 5)
    {
        v9 = va("%i", self->s.number);
    }
    else
    {
        if (self->targetname)
            v6 = SL_ConvertToString(self->targetname);
        else
            v6 = "<noname>";
        v7 = SL_ConvertToString(self->classname);
        v9 = va("%i : %s : %i : %s", self->s.number, v6, self->health, v7);
    }
    G_AddDebugString(v12, color, (float)(v10 * (float)0.75), v8);
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
    float v12; // [sp+50h] [-80h] BYREF
    float v13; // [sp+54h] [-7Ch] BYREF
    float v14[2]; // [sp+58h] [-78h] BYREF
    float v15; // [sp+60h] [-70h]
    float v16[6]; // [sp+70h] [-60h] BYREF

    if ((_S1 & 1) == 0)
    {
        MY_MAX_DIST_HALF = MY_MAX_DIST * 0.5f;
        _S1 |= 1u;
    }

    iassert(self);

    EntinfoPosAndScale(self, source, v14, &v13, &v12);

    if (v12 <= (double)MY_MAX_DIST)
    {
        v16[0] = MY_RGB[0];
        v16[1] = MY_RGB[1];
        v16[2] = MY_RGB[2];
        if (v12 >= (double)MY_MAX_DIST_HALF)
            v4 = (float)(1.0 - (float)((float)(v12 - MY_MAX_DIST_HALF) / MY_MAX_DIST_HALF));
        else
            v4 = 1.0;
        v16[3] = v4;
        p_index = &self->item[0].index;
        v6 = 2;
        v15 = -(float)((float)(MY_NEXTLINE * (float)0.5) - v15);
        v7 = v13;
        do
        {
            if (*p_index)
            {
                WeaponDef = BG_GetWeaponDef(*p_index);
                v9 = va("%i: %s (%i + %i)", self->s.number, WeaponDef->szInternalName, *(p_index - 1), *(p_index - 2));
                G_AddDebugString(v14, v16, (float)((float)v7 * 0.8f), v10);
                G_DebugBox(
                    self->r.currentOrigin,
                    self->r.mins,
                    self->r.maxs,
                    self->r.currentAngles[1],
                    v11,
                    (int)colorRedFaded,
                    1);
                v15 = MY_NEXTLINE + v15;
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