#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_aim.h"
#include <qcommon/mem_track.h>
#include <qcommon/graph.h>
#include <universal/assertive.h>
#include <universal/com_math.h>
#include "g_local.h"
#include "sentient.h"
#include "g_main.h"
#include <xanim/xanim.h>
#include "actor_senses.h"
#include "actor.h"
#include <game/bullet.h>
#include "actor_events.h"
#include <universal/com_files.h>
#include <devgui/devgui.h>
#include <win32/win_local.h>

struct AccuracyGraphBackup
{
    float accuracyGraphKnots[2][16][2];
};

unsigned int g_accuracyBufferIndex;
int g_numAccuracyGraphs;

float debugAccuracyColors[8][4] = { 0.0f };
WeaponDef *g_accuGraphWeapon[2][128];
DevGraph g_accuracyGraphs[128];
AccuracyGraphBackup g_accuGraphBuf[2][128];
char debugAccuracyStrings[8][32] = { 0 };
int dword_82C31FB0[256];

void __cdecl TRACK_actor_aim()
{
    track_static_alloc_internal(g_accuracyGraphs, 4096, "g_accuracyGraphs", 0);
    track_static_alloc_internal(g_accuGraphBuf, 0x10000, "g_accuGraphBuf", 0);
}

void __cdecl Actor_DrawDebugAccuracy(const float *pos, double scale, double rowHeight)
{
    const float *v8; // r30
    char *v9; // r31

    iassert(pos);

    v8 = debugAccuracyColors[0];
    v9 = debugAccuracyStrings[0];

    for (int i = 0; i < 8; i++)
    {
        G_AddDebugString(pos, debugAccuracyColors[i], scale, debugAccuracyStrings[i]);
        *((float *)pos + 2) = pos[2] - (float)rowHeight;
    }
    //do
    //{
    //    G_AddDebugString(pos, v8, scale, a5);
    //    v9 += 32;
    //    *((float *)pos + 2) = pos[2] - (float)rowHeight;
    //    v8 += 4;
    //} while ((int)v9 < (int)&dword_82C31FB0);
}

void __cdecl Actor_DebugAccuracyMsg(
    unsigned int msgIndex,
    const char *msg,
    double accuracy,
    const float *color,
    float *a5)
{
    const char *v9; // r3
    float *v10; // r11

    if (msgIndex >= 8)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp",
            99,
            0,
            "msgIndex doesn't index AI_DEBUG_ACCURACY_MSG_COUNT\n\t%i not in [0, %i)",
            msgIndex,
            8);
    if (!a5)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 100, 0, "%s", "color");
    if (msg)
    {
        v9 = va("%s: %1.3f", HIDWORD(accuracy), LODWORD(accuracy));
        I_strncpyz(debugAccuracyStrings[msgIndex], v9, 32);
    }
    else
    {
        debugAccuracyStrings[msgIndex][0] = 0;
    }
    v10 = debugAccuracyColors[msgIndex];
    *v10 = *a5;
    v10[1] = a5[1];
    v10[2] = a5[2];
    v10[3] = a5[3];
}

float __cdecl Actor_GetAccuracyFraction(
    double dist,
    const WeaponDef *weapDef,
    const char **accuracyType,
    unsigned int a4)
{
    double v6; // fp30
    int v7; // r28
    const char *v8; // r11
    const char *v9; // r10
    double v10; // fp1
    const char *v11; // r3
    double ValueFromFraction; // fp1

    v6 = (float)((float)dist * (float)0.00025000001);
    if (a4 >= 2)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp",
            124,
            0,
            "%s",
            "accuracyType >= WEAP_ACCURACY_AI_VS_AI && accuracyType < WEAP_ACCURACY_COUNT");
    v7 = a4 + 477;
    v8 = accuracyType[v7];
    if (v8)
    {
        v9 = accuracyType[a4 + 481];
        if (v9)
        {
            v10 = *(float *)&v8[8 * (unsigned int)v9 - 8];
            if (v10 != 1.0)
            {
                v11 = va("weapon '%s' has invalid graph...max range %f != 1.0.", *accuracyType, v10);
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp",
                    125,
                    0,
                    "%s\n\t%s",
                    "weapDef->accuracyGraphKnots[accuracyType] == 0 || weapDef->accuracyGraphKnotCount[accuracyType] == 0 || weapDe"
                    "f->accuracyGraphKnots[accuracyType][weapDef->accuracyGraphKnotCount[accuracyType] - 1][0] == 1.0f",
                    v11);
            }
        }
    }
    if (v6 > 1.0)
        v6 = 1.0;
    if (!accuracyType[v7] || !accuracyType[a4 + 481])
    {
        if (a4)
        {
            if (a4 == 1)
                Com_Error(ERR_DROP, "No AI vs Player accuracy for weapon %s", *accuracyType);
        }
        else
        {
            Com_Error(ERR_DROP, "No AI vs AI accuracy graph for weapon %s", *accuracyType);
        }
    }
    ValueFromFraction = GraphGetValueFromFraction((const int)accuracyType[a4 + 481], (const float (*)[2])accuracyType[v7], v6);
    return *((float *)&ValueFromFraction + 1);
}

float __cdecl Actor_GetWeaponAccuracy(
    const actor_s *self,
    const sentient_s *enemy,
    const WeaponDef *weapDef,
    WeapAccuracyType accuracyType)
{
    const WeaponDef *v8; // r3
    double v9; // fp1
    double AccuracyFraction; // fp1
    double v11; // fp31
    double v12; // fp1
    float v14; // [sp+50h] [-60h] BYREF
    float v15; // [sp+54h] [-5Ch]
    float v16; // [sp+58h] [-58h]
    float v17; // [sp+60h] [-50h] BYREF
    float v18; // [sp+64h] [-4Ch]
    float v19; // [sp+68h] [-48h]

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 162, 0, "%s", "self");
    if (!enemy)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 163, 0, "%s", "enemy");
    if (!enemy->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 164, 0, "%s", "enemy->ent");
    if (!weapDef)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 165, 0, "%s", "weapDef");
    if ((unsigned int)accuracyType >= WEAP_ACCURACY_COUNT)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp",
            166,
            0,
            "accuracyType doesn't index WEAP_ACCURACY_COUNT\n\t%i not in [0, %i)",
            accuracyType,
            2);
    Sentient_GetOrigin(self->sentient, &v14);
    Sentient_GetOrigin(enemy, &v17);
    //v9 = sqrtf((float)((float)((float)(v14 - v17) * (float)(v14 - v17)) + (float)((float)((float)(v16 - v19) * (float)(v16 - v19)) + (float)((float)(v15 - v18) * (float)(v15 - v18)))));
    v9 = sqrtf((float)((float)((float)(v14 - v17) * (float)(v14 - v17)) + (float)((float)((float)(v16 - v19) * (float)(v16 - v19)) + (float)((float)(v15 - v18) * (float)(v15 - v18)))));
    if (accuracyType == WEAP_ACCURACY_AI_VS_PLAYER)
        v9 = (float)(ai_accuracyDistScale->current.value * (float)v9);
    AccuracyFraction = Actor_GetAccuracyFraction(v9, v8, (const char**)&weapDef->szInternalName, (unsigned int)accuracyType);
    v11 = AccuracyFraction;
    if (AccuracyFraction < 0.0 || AccuracyFraction > 1.0)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp",
            179,
            1,
            (const char *)HIDWORD(AccuracyFraction),
            0,
            0,
            0);
    v12 = v11;
    return *((float *)&v12 + 1);
}

float __cdecl Actor_GetPlayerStanceAccuracy(const actor_s *self, const sentient_s *enemy)
{
    int pm_flags; // r11
    double v4; // fp1

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 192, 0, "%s", "self");
    if (!enemy)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 193, 0, "%s", "enemy");
    if (!enemy->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 194, 0, "%s", "enemy->ent");
    if (!enemy->ent->client)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 195, 0, "%s", "enemy->ent->client");
    pm_flags = enemy->ent->client->ps.pm_flags;
    if ((pm_flags & 1) != 0)
    {
        v4 = 0.5;
    }
    else if ((pm_flags & 2) != 0)
    {
        v4 = 0.75;
    }
    else
    {
        v4 = 1.0;
    }
    return *((float *)&v4 + 1);
}

//float __cdecl Actor_GetPlayerMovementAccuracy(const actor_s *self, const sentient_s *enemy)
//{
//    double v6; // fp0
//    double v10; // fp1
//    double v12; // fp31
//    float v14; // [sp+50h] [-50h] BYREF
//    float v15; // [sp+54h] [-4Ch]
//    float v16; // [sp+58h] [-48h]
//    float v17; // [sp+60h] [-40h] BYREF
//    float v18; // [sp+64h] [-3Ch]
//    float v19; // [sp+68h] [-38h]
//
//    iassert(self);
//    iassert(enemy);
//    iassert(enemy->ent);
//    iassert(enemy->ent->client);
//
//    Sentient_GetOrigin(self->sentient, &v14);
//    Sentient_GetOrigin(enemy, &v17);
//
//    _FP5 = -sqrtf((float)((float)((float)(v17 - v14) * (float)(v17 - v14)) + (float)((float)((float)(v19 - v16) * (float)(v19 - v16)) + (float)((float)(v18 - v15) * (float)(v18 - v15)))));
//    __asm { fsel      f10, f5, f11, f10 }
//    v6 = I_fabs((float)((float)(enemy->ent->client->ps.velocity[0]
//        * (float)((float)((float)1.0 / (float)_FP10) * (float)(v18 - v15)))
//        + (float)((float)(enemy->ent->client->ps.velocity[2]
//            * (float)((float)(v19 - v16) * (float)((float)1.0 / (float)_FP10)))
//            + (float)(enemy->ent->client->ps.velocity[1]
//                * (float)-(float)((float)((float)1.0 / (float)_FP10) * (float)(v17 - v14))))));
//    _FP12 = (float)((float)v6 - (float)250.0);
//    _FP10 = -v6;
//    __asm { fsel      f13, f12, f13, f0 }
//    v10 = 0.1;
//    __asm { fsel      f13, f10, f0, f13 }
//    v12 = (float)-(float)((float)((float)_FP13 * (float)0.0040000002) - (float)1.0);
//    if (v12 >= 0.1)
//    {
//        if (v12 < 0.0 || v12 > 1.0)
//            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 244, 1, (const char *)HIDWORD(v12), 0, 0, 0);
//        v10 = v12;
//    }
//    return *((float *)&v10 + 1);
//}

float __cdecl Actor_GetPlayerMovementAccuracy(const actor_s *self, const sentient_s *enemy)
{
    iassert(self);
    iassert(enemy);
    iassert(enemy->ent);
    iassert(enemy->ent->client);

    float selfPos[3];
    float enemyPos[3];

    Sentient_GetOrigin(self->sentient, selfPos);
    Sentient_GetOrigin(enemy, enemyPos);

    float dx = enemyPos[0] - selfPos[0];
    float dy = enemyPos[1] - selfPos[1];
    float dz = enemyPos[2] - selfPos[2];

    float dist = sqrtf(dx * dx + dy * dy + dz * dz);
    float inv_dist = (dist != 0.0f) ? (1.0f / dist) : 0.0f;

    const float *velocity = enemy->ent->client->ps.velocity;

    float dot =
        velocity[0] * dx * inv_dist +
        velocity[2] * dz * inv_dist -
        velocity[1] * dy * inv_dist;

    float absDot = fabsf(dot);
    float delta = absDot - 250.0f;

    float penalty = (delta >= 0.0f) ? delta : 0.0f;
    penalty = (-absDot >= 0.0f) ? 0.0f : penalty;

    float accuracy = -(penalty * 0.004f - 1.0f);

    if (accuracy >= 0.1f)
    {
        iassert(accuracy > 0.0f && accuracy <= 1.0f);
        return accuracy;
    }

    return 0.1f;
}

float __cdecl Actor_GetPlayerSightAccuracy(actor_s *self, const sentient_s *enemy)
{
    gclient_s *client; // r11
    double v5; // fp25
    double v6; // fp31
    double v7; // fp28
    double v8; // fp30
    double v9; // fp27
    double v10; // fp29
    double v11; // fp26
    gentity_s *ent; // r5
    gentity_s *v13; // r5
    gentity_s *v14; // r5
    double v15; // fp31
    double v16; // fp1
    float v18; // [sp+50h] [-90h] BYREF
    float v19; // [sp+54h] [-8Ch]
    float v20; // [sp+58h] [-88h]
    float v21[22]; // [sp+60h] [-80h] BYREF

    iassert(self);
    iassert(enemy);
    iassert(enemy->ent);
    iassert(enemy->ent->client);

    Sentient_GetEyePosition(enemy, v21);

    client = enemy->ent->client;
    v5 = 0.0;
    v6 = client->ps.origin[0];
    v7 = (float)(v21[0] - client->ps.origin[0]);
    v8 = client->ps.origin[1];
    v9 = (float)(v21[1] - client->ps.origin[1]);
    v10 = client->ps.origin[2];
    v11 = (float)(v21[2] - client->ps.origin[2]);
    if (Actor_CanSeeEntityPoint(self, v21, enemy->ent))
        v5 = 10.0;
    ent = enemy->ent;
    v18 = (float)((float)v7 * (float)0.75) + (float)v6;
    v19 = (float)((float)v9 * (float)0.75) + (float)v8;
    v20 = (float)((float)v11 * (float)0.75) + (float)v10;
    if (Actor_CanSeeEntityPoint(self, &v18, ent))
        v5 = (float)((float)v5 + (float)30.0);
    v13 = enemy->ent;
    v18 = (float)((float)v7 * (float)0.5) + (float)v6;
    v19 = (float)((float)v9 * (float)0.5) + (float)v8;
    v20 = (float)((float)v11 * (float)0.5) + (float)v10;
    if (Actor_CanSeeEntityPoint(self, &v18, v13))
        v5 = (float)((float)v5 + (float)30.0);
    v14 = enemy->ent;
    v18 = (float)((float)v7 * (float)0.25) + (float)v6;
    v19 = (float)((float)v9 * (float)0.25) + (float)v8;
    v20 = (float)((float)v11 * (float)0.25) + (float)v10;
    if (Actor_CanSeeEntityPoint(self, &v18, v14))
        v5 = (float)((float)v5 + (float)30.0);
    v15 = (float)((float)v5 * (float)0.0099999998);
    v16 = 0.1;
    if (v15 >= 0.1)
    {
        if (v15 < 0.0 || v15 > 1.0)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 294, 1, (const char *)HIDWORD(v15), 0, 0, 0);
        v16 = (float)((float)v5 * (float)0.0099999998);
    }
    return *((float *)&v16 + 1);
}

float __cdecl Actor_GetFinalAccuracy(actor_s *self, weaponParms *wp, double accuracyMod)
{
    double accuracy; // fp1
    sentient_s *enemy; // r30
    WeaponDef *weapDef; // r5
    double WeaponAccuracy; // fp30
    double PlayerStanceAccuracy; // fp29
    const float *v11; // r5
    double PlayerMovementAccuracy; // fp1
    double playerSightAccuracy; // fp27
    double v14; // fp28
    double accuracy; // fp31
    const float *v16; // r5
    const float *v17; // r5
    const float *v18; // r5
    const float *v19; // r5
    const float *v20; // r5
    const float *v21; // r5
    const float *v22; // r5

    iassert(self);
    iassert(self->sentient);
    iassert(self->sentient->targetEnt.isDefined());
    iassert(wp);

    accuracy = self->accuracy;

    iassert(self->accuracy >= 0.0f);
    iassert(accuracyMod >= 0.0f);

    enemy = Actor_GetTargetSentient(self);

    iassert(enemy);

    weapDef = wp->weapDef;
    if (enemy->ent->client)
    {
        WeaponAccuracy = Actor_GetWeaponAccuracy(self, enemy, weapDef, WEAP_ACCURACY_AI_VS_PLAYER);
        PlayerStanceAccuracy = Actor_GetPlayerStanceAccuracy(self, enemy);
        PlayerMovementAccuracy = Actor_GetPlayerMovementAccuracy(self, enemy);
        playerSightAccuracy = self->playerSightAccuracy;
        v14 = PlayerMovementAccuracy;
    }
    else
    {
        WeaponAccuracy = Actor_GetWeaponAccuracy(self, enemy, weapDef, WEAP_ACCURACY_AI_VS_AI);
        PlayerStanceAccuracy = 1.0;
        v14 = 1.0;
        playerSightAccuracy = 1.0;
    }
    accuracy = (float)((float)((float)((float)((float)((float)(self->accuracy * enemy->attackerAccuracy)
        * (float)accuracyMod)
        * (float)WeaponAccuracy)
        * (float)PlayerStanceAccuracy)
        * (float)v14)
        * (float)playerSightAccuracy);

    if (ai_debugAccuracy->current.enabled && ai_debugEntIndex->current.integer == self->ent->s.number)
    {
        Actor_DebugAccuracyMsg(0, "Self    ", self->accuracy, v11, (float *)colorWhite);
        Actor_DebugAccuracyMsg(1u, "Target  ", enemy->attackerAccuracy, v16, (float *)colorWhite);
        Actor_DebugAccuracyMsg(2u, "Script  ", accuracyMod, v17, (float *)colorWhite);
        Actor_DebugAccuracyMsg(3u, "Weapon  ", WeaponAccuracy, v18, (float *)colorWhite);
        Actor_DebugAccuracyMsg(4u, "Stance  ", PlayerStanceAccuracy, v19, (float *)colorWhite);
        Actor_DebugAccuracyMsg(5u, "Movement", v14, v20, (float *)colorWhite);
        Actor_DebugAccuracyMsg(6u, "Sight   ", playerSightAccuracy, v21, (float *)colorWhite);
        Actor_DebugAccuracyMsg(7u, "TOTAL   ", accuracy, v22, (float *)colorRed);
    }

    iassert(accuracy);

    float dbg = accuracy <= 0.0f ? 0.0f : (accuracy >= 1.0f ? 1.0f : accuracy);

    self->debugLastAccuracy = dbg;

    return dbg;
}

void __cdecl Actor_FillWeaponParms(actor_s *self, weaponParms *wp)
{
    float *gunForward; // r28

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 380, 0, "%s", "self");
    if (!wp)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 381, 0, "%s", "wp");
    if ((COERCE_UNSIGNED_INT(self->vLookForward[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(self->vLookForward[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(self->vLookForward[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp",
            383,
            0,
            "%s",
            "!IS_NAN((self->vLookForward)[0]) && !IS_NAN((self->vLookForward)[1]) && !IS_NAN((self->vLookForward)[2])");
    }
    if ((COERCE_UNSIGNED_INT(self->vLookRight[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(self->vLookRight[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(self->vLookRight[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp",
            384,
            0,
            "%s",
            "!IS_NAN((self->vLookRight)[0]) && !IS_NAN((self->vLookRight)[1]) && !IS_NAN((self->vLookRight)[2])");
    }
    if ((COERCE_UNSIGNED_INT(self->vLookUp[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(self->vLookUp[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(self->vLookUp[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp",
            385,
            0,
            "%s",
            "!IS_NAN((self->vLookUp)[0]) && !IS_NAN((self->vLookUp)[1]) && !IS_NAN((self->vLookUp)[2])");
    }
    gunForward = wp->gunForward;
    wp->forward[0] = self->vLookForward[0];
    wp->forward[1] = self->vLookForward[1];
    wp->forward[2] = self->vLookForward[2];
    wp->right[0] = self->vLookRight[0];
    wp->right[1] = self->vLookRight[1];
    wp->right[2] = self->vLookRight[2];
    wp->up[0] = self->vLookUp[0];
    wp->up[1] = self->vLookUp[1];
    wp->up[2] = self->vLookUp[2];
    if (!Actor_GetMuzzleInfo(self, wp->muzzleTrace, wp->gunForward))
    {
        Actor_GetEyePosition(self, wp->muzzleTrace);
        *gunForward = wp->forward[0];
        wp->gunForward[1] = wp->forward[1];
        wp->gunForward[2] = wp->forward[2];
    }
    if ((COERCE_UNSIGNED_INT(wp->muzzleTrace[0]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(wp->muzzleTrace[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(wp->muzzleTrace[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp",
            401,
            0,
            "%s",
            "!IS_NAN((wp->muzzleTrace)[0]) && !IS_NAN((wp->muzzleTrace)[1]) && !IS_NAN((wp->muzzleTrace)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*gunForward) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(wp->gunForward[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(wp->gunForward[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp",
            402,
            0,
            "%s",
            "!IS_NAN((wp->gunForward)[0]) && !IS_NAN((wp->gunForward)[1]) && !IS_NAN((wp->gunForward)[2])");
    }
}

// aislop
void Actor_HitSentient(weaponParms *wp, sentient_s *enemy, float accuracy)
{
    iassert(wp);
    iassert(enemy);

    // Get eye position and head height
    float eyePos[3];
    Sentient_GetEyePosition(enemy, eyePos);
    float headHeight = (float)Sentient_GetHeadHeight(enemy);

    // Vector from muzzle to eye
    float toEye[3] = {
        eyePos[0] - wp->muzzleTrace[0],
        eyePos[1] - wp->muzzleTrace[1],
        eyePos[2] - wp->muzzleTrace[2]
    };

    // Normalize toEye
    float lenSq = toEye[0] * toEye[0] + toEye[1] * toEye[1] + toEye[2] * toEye[2];
    iassert(lenSq > 0.0f);
    float invLen = 1.0f / sqrtf(lenSq);
    float dir[3] = { toEye[0] * invLen, toEye[1] * invLen, toEye[2] * invLen };

    // Compute lateral vector by cross with wp->up
    float lat[3] = {
        dir[1] * wp->up[2] - dir[2] * wp->up[1],
        dir[2] * wp->up[0] - dir[0] * wp->up[2],
        dir[0] * wp->up[1] - dir[1] * wp->up[0]
    };
    float latLenSq = lat[0] * lat[0] + lat[1] * lat[1] + lat[2] * lat[2];
    iassert(latLenSq > 0.0f);
    float latInv = 1.0f / sqrtf(latLenSq);
    lat[0] *= latInv; lat[1] *= latInv; lat[2] *= latInv;

    // Determine spread parameters
    float vertMax, horizMax;
    if (enemy->ent->client)
    {
        vertMax = 8.0f;
        horizMax = -44.0f;
    }
    else
    {
        iassert(accuracy >= 0.0f && accuracy <= 1.0f);
        horizMax = -44.0f * (1.0f + 0.5f * (1.0f - accuracy));
        vertMax = 15.0f * (0.1f + 0.9f * (1.0f - accuracy));
    }

    // Add randomness
    float rVert = G_crandom() * vertMax;
    float rHoriz = G_random() * horizMax;

    // Compute final forward vector components
    float center = rHoriz + headHeight;
    wp->forward[0] = wp->up[0] * center + dir[0] * rVert + eyePos[0] - wp->muzzleTrace[0];
    wp->forward[1] = wp->up[1] * center + dir[1] * rVert + eyePos[1] - wp->muzzleTrace[1];
    wp->forward[2] = wp->up[2] * center + dir[2] * rVert + eyePos[2] - wp->muzzleTrace[2];

    // Renormalize forward vector
    float fwdLenSq = wp->forward[0] * wp->forward[0]
        + wp->forward[1] * wp->forward[1]
        + wp->forward[2] * wp->forward[2];
    iassert(fwdLenSq > 0.0f);
    float invFwdLen = 1.0f / sqrtf(fwdLenSq);
    wp->forward[0] *= invFwdLen;
    wp->forward[1] *= invFwdLen;
    wp->forward[2] *= invFwdLen;
}

// aislop
void Actor_HitTarget(const weaponParms *wp, const float *target, float *forward)
{
    iassert(wp && target && forward);

    const float *up = wp->up;

    // Direction from muzzle to target
    float toTarget[3] = {
        target[0] - wp->muzzleTrace[0],
        target[1] - wp->muzzleTrace[1],
        target[2] - wp->muzzleTrace[2]
    };
    float distSq = toTarget[0] * toTarget[0] + toTarget[1] * toTarget[1] + toTarget[2] * toTarget[2];
    iassert(distSq > 0.0f);
    float invLen = 1.0f / sqrtf(distSq);
    float dir[3] = {
        toTarget[0] * invLen,
        toTarget[1] * invLen,
        toTarget[2] * invLen
    };

    // Lateral vector = cross(dir, up), normalized
    float crossVec[3] = {
        dir[1] * up[2] - dir[2] * up[1],
        dir[2] * up[0] - dir[0] * up[2],
        dir[0] * up[1] - dir[1] * up[0]
    };
    float crossLenSq = crossVec[0] * crossVec[0] + crossVec[1] * crossVec[1] + crossVec[2] * crossVec[2];
    iassert(crossLenSq > 0.0f);
    float crossInv = 1.0f / sqrtf(crossLenSq);
    crossVec[0] *= crossInv;
    crossVec[1] *= crossInv;
    crossVec[2] *= crossInv;

    // Random spread
    float r1 = G_crandom() * 8.0f;
    float r2 = G_crandom() * 8.0f;

    // Apply spread around right and up
    float aimPos[3] = {
        target[0] + crossVec[0] * r1 + up[0] * r2,
        target[1] + crossVec[1] * r1 + up[1] * r2,
        target[2] + crossVec[2] * r1 + up[2] * r2
    };

    // Compute final forward vector from muzzle to aimPos
    forward[0] = aimPos[0] - wp->muzzleTrace[0];
    forward[1] = aimPos[1] - wp->muzzleTrace[1];
    forward[2] = aimPos[2] - wp->muzzleTrace[2];

    // Normalize forward
    float fwdLenSq = forward[0] * forward[0] + forward[1] * forward[1] + forward[2] * forward[2];
    iassert(fwdLenSq > 0.0f);
    float fwdInvLen = 1.0f / sqrtf(fwdLenSq);
    forward[0] *= fwdInvLen;
    forward[1] *= fwdInvLen;
    forward[2] *= fwdInvLen;
}


void __cdecl Actor_HitEnemy(actor_s *self, weaponParms *wp, double accuracy)
{
    self->missCount = 0;
    self->hitCount = self->hitCount + 1;
    iassert(self->sentient);
    iassert(self->sentient->targetEnt.isDefined());
    iassert(self->sentient->targetEnt.ent()->sentient);

    Actor_HitSentient(wp, self->sentient->targetEnt.ent()->sentient, accuracy);
}

// aislop
float outerRadius;
void __cdecl Actor_MissSentient(weaponParms *wp, sentient_s *enemy, float accuracy)
{
    gentity_s *ent; // r27
    float *up; // r31
    float v8; // fp13
    float v11; // fp11
    float v12; // fp29
    float v13; // fp28
    float v14; // fp27
    float v17; // fp0
    float v18; // fp26
    float v19; // fp25
    float v20; // fp24
    int v21; // r28
    float v22; // fp1
    __int64 v23; // r10
    float v24; // fp0
    float HeadHeight; // fp30
    float v26; // fp1
    float v27; // fp0
    float v28; // fp13
    int v29; // r29
    float v30; // fp30
    float v31; // fp1
    __int64 v32; // r10
    float v33; // fp0
    float v34; // fp1
    float v35; // fp12
    float v36; // fp0
    float v37; // fp12
    float v38; // fp0
    float v39; // fp11
    float v40; // fp13
    float v41; // fp10
    float v44; // fp0
    __int64 v45; // [sp+50h] [-A0h] BYREF
    float v46; // [sp+58h] [-98h]
    float v47; // [sp+60h] [-90h] BYREF
    float v48; // [sp+64h] [-8Ch]
    float v49; // [sp+68h] [-88h]
    float v50; // [sp+70h] [-80h] BYREF
    float v51; // [sp+74h] [-7Ch]
    float v52; // [sp+78h] [-78h]

    ent = enemy->ent;
    if (outerRadius == 6969.0f)
        outerRadius = sqrtf(15.0f * 15.0f * 2.0f * 3.0f);
    Sentient_GetEyePosition(enemy, &v47);
    up = wp->up;
    v8 = v49 - wp->muzzleTrace[2];
    v11 = -sqrtf((v47 - wp->muzzleTrace[0]) * (v47 - wp->muzzleTrace[0]) +
        (v49 - wp->muzzleTrace[2]) * (v49 - wp->muzzleTrace[2]) +
        (v48 - wp->muzzleTrace[1]) * (v48 - wp->muzzleTrace[1]));
    v12 = 1.0f / v11;
    v13 = v12 * (v47 - wp->muzzleTrace[0]);
    v14 = v12 * (v48 - wp->muzzleTrace[1]);
    v46 = v12 * (v49 - wp->muzzleTrace[2]);
    Vec3Cross((const float *)&v45, wp->up, &v50);
    v17 = -sqrtf(v50 * v50 + v52 * v52 + v51 * v51);
    v18 = v50 * (1.0f / v17);
    v19 = v51 * (1.0f / v17);
    v20 = v52 * (1.0f / v17);
    if (ent->client)
    {
        v21 = 1;
        if (G_random() <= 0.5f)
            v21 = -1;
        v22 = G_random();
        LODWORD(v23) = v21;
        v45 = v23;
        v24 = -(15.0f + 1.0f);
        v47 = v18 * ((v22 + 1.0f) * 8.0f * v21) + (v24 * v13 + v47);
        v48 = v19 * ((v22 + 1.0f) * 8.0f * v21) + (v13 * v24 + v48);
        v49 = v20 * ((v22 + 1.0f) * 8.0f * v21) + (v14 * v24 + v49);
        HeadHeight = Sentient_GetHeadHeight(enemy);
        v26 = G_random();
        v27 = -(v26 * 44.0f - HeadHeight);
        v28 = v27 + v47;
    }
    else
    {
        v29 = 1;
        if (G_random() <= 0.5f)
            v29 = -1;
        v30 = 1.0f - accuracy;
        v31 = G_random();
        LODWORD(v32) = v29;
        v33 = -(15.0f + 1.0f);
        v45 = v32;
        v47 = v18 * ((v31 * v30 * 10.0f + outerRadius) * v29) + (v33 * v13 + v47);
        v48 = v19 * ((v31 * v30 * 10.0f + outerRadius) * v29) + (v13 * v33 + v48);
        v49 = v20 * ((v31 * v30 * 10.0f + outerRadius) * v29) + (v14 * v33 + v49);
        v34 = G_crandom();
        v27 = v34 * v30 * -22.0f;
        v28 = v27 + v47;
    }
    v35 = wp->up[1] * v27 + v48;
    v36 = wp->up[2] * v27 + v49;
    wp->forward[0] = v28 - wp->muzzleTrace[0];
    wp->forward[1] = v35 - wp->muzzleTrace[1];
    v37 = wp->forward[1];
    v38 = v36 - wp->muzzleTrace[2];
    v39 = wp->forward[0] * wp->forward[0];
    v40 = wp->forward[1] * wp->forward[1];
    wp->forward[2] = v38;
    v41 = v38;
    v44 = 1.0f / sqrtf(v38 * v38 + v40 + v39);
    wp->forward[0] *= v44;
    wp->forward[1] *= v44;
    wp->forward[2] *= v44;
}


// aislop
float outerRadius_0;
void __cdecl Actor_MissTarget(const weaponParms *wp, const float *target, float *forward)
{
    const float *up;
    float v7, v8, v9, v12, v13, v14, v15;
    float v18, v19, v20, v21;
    __int64 v22;
    float v23;
    int v24;
    float v25, v26, v27, v28;
    float v29, v30, v31;
    float v32, v33, v34, v35, v36;
    float v39;
    __int64 v40;
    float v41, v42, v43, v44;

    if (outerRadius_0 == 6969.0f)
        outerRadius_0 = sqrtf(15.0f * 15.0f * 2.0f * 3.0f);

    up = wp->up;
    v7 = target[0] - wp->muzzleTrace[0];
    v8 = target[2] - wp->muzzleTrace[2];
    v9 = target[1] - wp->muzzleTrace[1];

    v12 = -sqrtf(v9 * v9 + v8 * v8 + v7 * v7);
    v13 = 1.0f / v12;
    v14 = v13 * v7;
    v15 = v13 * v9;
    v41 = v13 * v8;

    // Pack vector for Vec3Cross
    *(float *)&v40 = v14;
    *((float *)&v40 + 1) = v15;

    Vec3Cross((const float *)&v40, wp->up, &v42);

    v18 = -sqrtf(v42 * v42 + v44 * v44 + v43 * v43);
    v19 = v42 * (1.0f / v18);
    v20 = v43 * (1.0f / v18);
    v21 = v44 * (1.0f / v18);

    v23 = G_random();
    v24 = 1;
    if (v23 <= 0.5f)
        v24 = -1;
    LODWORD(v22) = v24;

    v25 = -(15.0f + 1.0f);

    v26 = v19 * (v24 * outerRadius_0) + (v25 * v14 + target[0]);
    v27 = v20 * (v24 * outerRadius_0) + (v25 * v15 + target[1]);
    v28 = v21 * (v24 * outerRadius_0) + (v25 * v41 + target[2]);
    v40 = v22;

    v29 = G_crandom();
    v30 = wp->up[1] * (v29 * 12.0f) + v27;
    v31 = wp->up[2] * (v29 * 12.0f) + v28;

    *forward = up[0] * (v29 * 12.0f) + v26 - wp->muzzleTrace[0];
    forward[1] = v30 - wp->muzzleTrace[1];
    v32 = forward[1];
    v33 = v31 - wp->muzzleTrace[2];
    v34 = forward[1] * forward[1];
    v35 = *forward;
    forward[2] = v33;
    v36 = v33;

    v39 = 1.0f / sqrtf(v33 * v33 + v35 * v35 + v34);

    *forward *= v39;
    forward[1] *= v39;
    forward[2] *= v39;
}


void __cdecl Actor_MissEnemy(actor_s *self, weaponParms *wp, double accuracy)
{
    ++self->missCount;

    iassert(self->sentient);
    iassert(self->sentient->targetEnt.isDefined());
    iassert(self->sentient->targetEnt.ent()->sentient);

    Actor_MissSentient(wp, self->sentient->targetEnt.ent()->sentient, accuracy);
}

void __cdecl Actor_ShootNoEnemy(actor_s *self, weaponParms *wp)
{
    wp->forward[0] = wp->gunForward[0];
    wp->forward[1] = wp->gunForward[1];
    wp->forward[2] = wp->gunForward[2];
}

// aislop
void __cdecl Actor_ShootPos(actor_s *self, weaponParms *wp, float *pos)
{
    wp->forward[0] = *pos - wp->muzzleTrace[0];
    wp->forward[1] = pos[1] - wp->muzzleTrace[1];
    wp->forward[2] = pos[2] - wp->muzzleTrace[2];

    float magnitude = -sqrtf(wp->forward[0] * wp->forward[0] +
        wp->forward[1] * wp->forward[1] +
        wp->forward[2] * wp->forward[2]);
    wp->forward[0] /= magnitude;
    wp->forward[1] /= magnitude;
    wp->forward[2] /= magnitude;
}

void __cdecl Actor_ClampShot(actor_s *self, weaponParms *wp)
{
    float planeNormal[4]; // [sp+58h] [-68h] BYREF
    float dest[4]; // [sp+68h] [-58h] BYREF

    iassert(!IS_NAN((wp->gunForward)[0]) && !IS_NAN((wp->gunForward)[1]) && !IS_NAN((wp->gunForward)[2]));
    iassert(!IS_NAN((wp->forward)[0]) && !IS_NAN((wp->forward)[1]) && !IS_NAN((wp->forward)[2]));
    iassert(I_fabs(Vec3Dot(wp->gunForward, wp->gunForward) - 1.f) < 0.01f);
    iassert(I_fabs(Vec3Dot(wp->forward, wp->forward) - 1.f) < 0.01f);

    if (I_fabs(Vec3Dot(wp->gunForward, wp->forward)) < 0.96591997f)
    {
        Vec3Cross(wp->gunForward, wp->forward, planeNormal);

        iassert(!IS_NAN((planeNormal)[0]) && !IS_NAN((planeNormal)[1]) && !IS_NAN((planeNormal)[2]));

        float magnitudeSquared = Vec3Dot(planeNormal, planeNormal);
        float magnitudeInverse = 1.0f / sqrtf(magnitudeSquared);

        Vec3Scale(planeNormal, magnitudeInverse, planeNormal);

        iassert(!IS_NAN((planeNormal)[0]) && !IS_NAN((planeNormal)[1]) && !IS_NAN((planeNormal)[2]));
        iassert(planeNormal[0] || planeNormal[1] || planeNormal[2]);
        
        RotatePointAroundVector(dest, planeNormal, wp->gunForward, 15.0);
        wp->forward[0] = dest[0];
        wp->forward[1] = dest[1];
        wp->forward[2] = dest[2];
    }
}

void __cdecl Actor_Shoot(actor_s *self, float accuracyMod, float (*posOverride)[3], enumLastShot lastShot)
{
    gentity_s *ent; // r27
    unsigned int weaponName; // r3
    const char *v11; // r3
    unsigned int weapon; // r26
    gentity_s *TargetEntity; // r3
    const float *p_eType; // r25
    float invLen; // fp11
    double v18; // fp0
    double v19; // fp13
    double FinalAccuracy; // fp31
    const weaponParms *v22; // r4
    weapType_t weapType; // r11
    float v26; // [sp+50h] [-B0h]
    float v27; // [sp+50h] [-B0h]
    float v28[6]; // [sp+58h] [-A8h] BYREF
    weaponParms wp; // [sp+70h] [-90h] BYREF

    iassert(self);

    ent = self->ent;
    //Profile_Begin(236);
    if (self->lastShotTime == level.time)
    {
        Com_PrintError(
            18,
            "ERROR: Attempt for same actor (entnum %d) to shoot/melee more than once in a frame.\n",
            ent->s.number);
        //Profile_EndInternal(0);
        return;
    }
    weaponName = self->weaponName;
    self->lastShotTime = level.time;
    v11 = SL_ConvertToString(weaponName);
    weapon = G_GetWeaponIndexForName(v11);
    wp.weapDef = BG_GetWeaponDef(weapon);
    Actor_FillWeaponParms(self, &wp);
    TargetEntity = Actor_GetTargetEntity(self);
    p_eType = (const float *)&TargetEntity->s.eType;

    if (lastShot)
    {
        //_FP9 = -sqrtf((float)((float)((float)(*lastShot - wp.muzzleTrace[0]) * (float)(*lastShot - wp.muzzleTrace[0]))
        //    + (float)((float)((float)(lastShot[2] - wp.muzzleTrace[2])
        //        * (float)(lastShot[2] - wp.muzzleTrace[2]))
        //        + (float)((float)(lastShot[1] - wp.muzzleTrace[1])
        //            * (float)(lastShot[1] - wp.muzzleTrace[1])))));
        //__asm { fsel      f11, f9, f10, f11 }
        //v17 = (float)((float)1.0 / (float)_FP11);

        float dx = *posOverride[0] - wp.muzzleTrace[0];
        float dy = *posOverride[1] - wp.muzzleTrace[1];
        float dz = *posOverride[2] - wp.muzzleTrace[2];

        // Compute inverse length of the vector (dx, dy, dz)
        float len = sqrtf(dx * dx + dy * dy + dz * dz);
        invLen = (len != 0.0f ? 1.0f / len : 0.0f);

        wp.forward[0] = invLen * (*posOverride[0] - wp.muzzleTrace[0]);
        wp.forward[1] = invLen * (*posOverride[1] - wp.muzzleTrace[1]);
        wp.forward[2] = invLen * (*posOverride[2] - wp.muzzleTrace[2]);

        iassert(!IS_NAN((wp.forward)[0]) && !IS_NAN((wp.forward)[1]) && !IS_NAN((wp.forward)[2]));
    }

    else if (TargetEntity)
    {
        if (TargetEntity->sentient)
        {
            Actor_BroadcastTeamEvent(self->sentient, AI_EV_NEW_ENEMY);
            FinalAccuracy = Actor_GetFinalAccuracy(self, &wp, accuracyMod);
            if (FinalAccuracy <= G_random())
            {
                Actor_MissEnemy(self, &wp, FinalAccuracy);
                iassert(!IS_NAN((wp.forward)[0]) && !IS_NAN((wp.forward)[1]) && !IS_NAN((wp.forward)[2]));
            }
            else
            {
                Actor_HitEnemy(self, &wp, FinalAccuracy);
                iassert(!IS_NAN((wp.forward)[0]) && !IS_NAN((wp.forward)[1]) && !IS_NAN((wp.forward)[2]));
            }
        }
        else
        {
            G_EntityCentroid(TargetEntity, v28);
            Actor_HitTarget(&wp, v28, wp.forward);
        }
    }
    else
    {
        wp.forward[0] = wp.gunForward[0];
        wp.forward[1] = wp.gunForward[1];
        wp.forward[2] = wp.gunForward[2];
        iassert(!IS_NAN((wp.forward)[0]) && !IS_NAN((wp.forward)[1]) && !IS_NAN((wp.forward)[2]));
    }
    Actor_ClampShot(self, &wp);
    ent->s.weapon = weapon;
    weapType = wp.weapDef->weapType;

    if (weapType == WEAPTYPE_BULLET)
    {
        Bullet_Fire(ent, 0.0, &wp, ent, level.time); // KISAKTODO: guessed last arg
        if (lastShot == LAST_SHOT_IN_CLIP)
        {
            G_AddEvent(ent, 27, 0);
            return;
        }
        G_AddEvent(ent, 26, 0);
        return;
    }

    if (weapType == WEAPTYPE_PROJECTILE)
    {
        Weapon_RocketLauncher_Fire(ent, weapon, 0.0, &wp, wp.forward, (gentity_s *)vec3_origin, p_eType);
        G_AddEvent(ent, 26, 0);
        return;
    }
}

void __cdecl Actor_ShootBlank(actor_s *self)
{
    unsigned int weaponName; // r3
    const char *v3; // r3
    unsigned int WeaponIndexForName; // r28
    gentity_s *ent; // r11
    weaponParms v6; // [sp+50h] [-70h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 793, 0, "%s", "self");
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 794, 0, "%s", "self->ent");
    if (self->lastShotTime == level.time)
    {
        Com_PrintError(
            18,
            "ERROR: Attempt for same actor (entnum %d) to shoot/melee more than once in a frame.\n",
            self->ent->s.number);
    }
    else
    {
        weaponName = self->weaponName;
        self->lastShotTime = level.time;
        v3 = SL_ConvertToString(weaponName);
        WeaponIndexForName = G_GetWeaponIndexForName(v3);
        v6.weapDef = BG_GetWeaponDef(WeaponIndexForName);
        Actor_FillWeaponParms(self, &v6);
        ent = self->ent;
        v6.forward[0] = v6.gunForward[0];
        v6.forward[1] = v6.gunForward[1];
        v6.forward[2] = v6.gunForward[2];
        ent->s.weapon = WeaponIndexForName;
        if (self->ent->s.weapon != WeaponIndexForName)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp",
                819,
                0,
                "%s",
                "self->ent->s.weapon == weapIndex");
        if (v6.weapDef->weapType)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp",
                821,
                0,
                "%s",
                "wp.weapDef->weapType == WEAPTYPE_BULLET");
        G_AddEvent(self->ent, 26, 0);
    }
}

gentity_s *__cdecl Actor_Melee(actor_s *self, const float *direction)
{
    gentity_s *ent; // r27
    const char *v5; // r3
    unsigned int WeaponIndexForName; // r29
    const sentient_s *TargetSentient; // r30
    double v9; // fp0
    double v10; // fp13
    double v11; // fp13
    double v14; // fp12
    double v15; // fp0
    double v16; // fp0
    double v19; // fp11
    double v20; // fp13
    float v21; // [sp+50h] [-90h] BYREF
    float v22; // [sp+54h] [-8Ch]
    float v23; // [sp+58h] [-88h]
    weaponParms wp; // [sp+60h] [-80h] BYREF
    float x;
    float y;
    float z;
    float mag;

    iassert(self);

    ent = self->ent;
    v5 = SL_ConvertToString(self->weaponName);
    WeaponIndexForName = G_GetWeaponIndexForName(v5);
    if (self->lastShotTime == level.time)
    {
        Com_PrintError(
            18,
            "ERROR: Attempt for same actor (entnum %d) to shoot/melee more than once in a frame.\n",
            ent->s.number);
        return 0;
    }
    self->lastShotTime = level.time;
    TargetSentient = Actor_GetTargetSentient(self);
    Actor_FillWeaponParms(self, &wp);
    wp.weapDef = BG_GetWeaponDef(WeaponIndexForName);
    Actor_GetEyePosition(self, wp.muzzleTrace);
    if (TargetSentient)
    {
        Sentient_GetEyePosition(TargetSentient, &v21);
        v9 = (float)(v21 - wp.muzzleTrace[0]);
        v10 = (float)(v22 - wp.muzzleTrace[1]);
        if (direction)
        {
            v11 = (float)((float)sqrtf((float)((float)((float)(v22 - wp.muzzleTrace[1]) 
                * (float)(v22 - wp.muzzleTrace[1])) + (float)((float)(v21 - wp.muzzleTrace[0]) 
                    * (float)(v21 - wp.muzzleTrace[0]))))
                / (float)sqrtf((float)((float)(*direction * *direction) + (float)(direction[1] * direction[1]))));
            v9 = (float)(*direction * (float)v11);
            v10 = (float)((float)v11 * direction[1]);
        }

        //_FP9 = -sqrtf((float)((float)((float)v9 * (float)v9)
        //    + (float)((float)((float)(v23 - v24[0].muzzleTrace[2]) * (float)(v23 - v24[0].muzzleTrace[2]))
        //        + (float)((float)v10 * (float)v10))));
        //__asm { fsel      f12, f9, f11, f12 }
        //v14 = (float)((float)1.0 / (float)_FP12);

        mag = sqrtf(v9 * v9 + v10 * v10 + (v23 - wp.muzzleTrace[2]) * (v23 - wp.muzzleTrace[2]));
        v14 = mag > 0.0f ? 1.0f / mag : 0.0f;

        wp.forward[0] = (float)v14 * (float)v9;
        wp.forward[1] = (float)v10 * (float)v14;
        v15 = (float)((float)(v23 - wp.muzzleTrace[2]) * (float)v14);
        goto LABEL_11;
    }
    if (direction)
    {
        v16 = direction[2];

        //_FP9 = -sqrtf((float)((float)(*direction * *direction)
        //    + (float)((float)(direction[2] * direction[2]) + (float)(direction[1] * direction[1]))));
        //__asm { fsel      f11, f9, f10, f11 }
        //v19 = (float)((float)1.0 / (float)_FP11);

        x = direction[0], y = direction[1], z = direction[2];
        mag = sqrtf(x * x + y * y + z * z);
        v19 = 1.0f / mag;

        v20 = (float)(direction[1] * (float)v19);
        wp.forward[0] = (float)v19 * *direction;
        wp.forward[1] = v20;
        v15 = (float)((float)v16 * (float)v19);
    LABEL_11:
        wp.forward[2] = v15;
    }
    ent->s.weapon = WeaponIndexForName;
    return Weapon_Melee(self->ent, &wp, 64.0, 0.0, 0.0, level.time); // KISAKTODO: guessed last arg
}

float __cdecl Sentient_GetScarinessForDistance(sentient_s *self, sentient_s *enemy, double fDist)
{
    actor_s *actor; // r11
    const char *v7; // r3
    unsigned int WeaponIndexForName; // r3
    WeaponDef *WeaponDef; // r3
    double AccuracyFraction; // fp1
    double value; // fp0

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 902, 0, "%s", "self");
    if (!self->ent)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 903, 0, "%s", "self->ent");
    if (!enemy)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 904, 0, "%s", "enemy");
    if (fDist < 0.0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 905, 0, "%s", "fDist >= 0");
    actor = self->ent->actor;
    if (actor)
    {
        v7 = SL_ConvertToString(actor->weaponName);
        WeaponIndexForName = G_GetWeaponIndexForName(v7);
        WeaponDef = BG_GetWeaponDef(WeaponIndexForName);
        AccuracyFraction = Actor_GetAccuracyFraction(fDist, WeaponDef, &WeaponDef->szInternalName, 0);
    }
    else
    {
        if (fDist > ai_playerNearRange->current.value)
        {
            if (fDist >= ai_playerFarRange->current.value
                || ai_playerNearAccuracy->current.value == ai_playerFarAccuracy->current.value)
            {
                value = ai_playerFarAccuracy->current.value;
            }
            else
            {
                value = (float)((float)((float)((float)((float)fDist - ai_playerNearRange->current.value)
                    / (float)(ai_playerFarRange->current.value - ai_playerNearRange->current.value))
                    * (float)(ai_playerFarAccuracy->current.value - ai_playerNearAccuracy->current.value))
                    + ai_playerNearAccuracy->current.value);
            }
        }
        else
        {
            value = ai_playerNearAccuracy->current.value;
        }
        AccuracyFraction = (float)((float)value * (float)5.0);
    }
    return *((float *)&AccuracyFraction + 1);
}

void __cdecl Actor_GetAccuracyGraphFileName_FastFile(
    const WeaponDef *weaponDef,
    WeapAccuracyType accuracyType,
    char *filePath,
    unsigned int sizeofFilePath)
{
    if (accuracyType)
    {
        if (accuracyType == WEAP_ACCURACY_AI_VS_PLAYER)
        {
            Com_sprintf(filePath, 4, "accuracy\\aivsplayer\\%s", weaponDef->accuracyGraphName[1]);
        }
        else if (!alwaysfails)
        {
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 982, 1, "inconceivable");
        }
    }
    else
    {
        Com_sprintf(filePath, 4, "accuracy\\aivsai\\%s", weaponDef->accuracyGraphName[0]);
    }
}

void __cdecl Actor_GetAccuracyGraphFileName(
    const WeaponDef *weaponDef,
    WeapAccuracyType accuracyType,
    char *filePath,
    unsigned int sizeofFilePath)
{
    if (accuracyType)
    {
        if (accuracyType == WEAP_ACCURACY_AI_VS_PLAYER)
        {
            Com_sprintf(filePath, 4, "accuracy\\aivsplayer\\%s", weaponDef->accuracyGraphName[1]);
        }
        else if (!alwaysfails)
        {
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 982, 1, "inconceivable");
        }
    }
    else
    {
        Com_sprintf(filePath, 4, "accuracy\\aivsai\\%s", weaponDef->accuracyGraphName[0]);
    }
}

void __cdecl Actor_AccuracyGraphSaveToFile(
    const DevGraph *graph,
    WeaponDef *weaponDef,
    WeapAccuracyType accuracyType)
{
    int v6; // r27
    int v7; // r30
    char *v8; // r11
    int v10; // r31
    char *v11; // r11
    const char *RemotePCPath; // r3
    char v14[256]; // [sp+50h] [-340h] BYREF
    char v15[576]; // [sp+150h] [-240h] BYREF

    if (!graph)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 1006, 0, "%s", "graph");
    if (!weaponDef)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 1007, 0, "%s", "weaponDef");
    if (accuracyType)
    {
        if (accuracyType == WEAP_ACCURACY_AI_VS_PLAYER)
        {
            Com_sprintf(v14, 4, "accuracy\\aivsplayer\\%s", weaponDef->accuracyGraphName[1]);
        }
        else if (!alwaysfails)
        {
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 982, 1, "inconceivable");
        }
    }
    else
    {
        Com_sprintf(v14, 4, "accuracy\\aivsai\\%s", weaponDef->accuracyGraphName[0]);
    }
    v6 = FS_FOpenTextFileWrite(v14);
    if (v6)
    {
        iassert(graph->knotCount);
        v7 = *graph->knotCount;
        Com_sprintf(v15, 512, "%s%d\n", "WEAPONACCUFILE\n\n", v7);
        v8 = v15;
        while (*v8++)
            ;
        FS_Write(v15, v8 - v15 - 1, v6);
        if (v7 > 0)
        {
            v10 = 0;
            do
            {
                Com_sprintf(
                    v15,
                    512,
                    "%.4f %.4f\n",
                    graph->knots[v10][0],
                    graph->knots[v10][1]
                );
                v11 = v15;
                while (*v11++)
                    ;
                FS_Write(v15, v11 - v15 - 1, v6);
                --v7;
                ++v10;
            } while (v7);
        }
        FS_FCloseFile(v6);

        //if (FS_IsUsingRemotePCSharing())
        //    RemotePCPath = FS_GetRemotePCPath(0);
        //else
            RemotePCPath = Sys_DefaultInstallPath();
        Com_Printf(18, "^7Successfully saved accuracy file [%s\\%s].\n", RemotePCPath, v14);
    }
    else
    {
        Com_PrintError(18, "Could not save accuracy file [%s].\n", weaponDef->accuracyGraphName[accuracyType]);
    }
}

void __cdecl Actor_CommonAccuracyGraphEventCallback(
    const DevGraph *graph,
    DevEventType event,
    WeapAccuracyType accuracyType)
{
    const char **data; // r30
    int v7; // r28
    int v8; // r30
    char *v9; // r11
    unsigned __int8 *v10; // r10
    unsigned __int8 *v12; // r10
    int v13; // r9
    char v14[32]; // [sp+50h] [-2050h] BYREF
    _QWORD v15[1030]; // [sp+70h] [-2030h] BYREF

    if (!graph)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 1054, 0, "%s", "graph");
    if (!graph->data)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 1055, 0, "%s", "graph->data");
    data = (const char **)graph->data;
    if (event == EVENT_ACCEPT)
    {
        memset(v15, 0, 0x2000u);
        sprintf((char *)v15, "Weapon: %s\nKnot Count: %d\n", *data, *graph->knotCount);
        v7 = 0;
        if (*graph->knotCount > 0)
        {
            v8 = 0;
            do
            {
                sprintf(
                    v14,
                    "%.4f %.4f\n",
                    graph->knots[v8][0],
                    graph->knots[v8][1]
                );
                    
                v9 = v14;
                v10 = (unsigned __int8 *)v15;
                while (*v10++)
                    ;
                v12 = v10 - 1;
                do
                {
                    v13 = (unsigned __int8)*v9++;
                    *v12++ = v13;
                } while (v13);
                ++v7;
                ++v8;
            } while (v7 < *graph->knotCount);
        }
        Com_Printf(18, "^6%s", (const char *)v15);
    }
    else if (event == EVENT_SAVE)
    {
        Actor_AccuracyGraphSaveToFile(graph, (WeaponDef *)graph->data, accuracyType);
    }
}

void __cdecl Actor_AiVsAiAccuracyGraphEventCallback(
    const DevGraph *graph,
    DevEventType event,
    int unusedLocalClientNum)
{
    Actor_CommonAccuracyGraphEventCallback(graph, event, WEAP_ACCURACY_AI_VS_AI);
}

void __cdecl Actor_AiVsPlayerAccuracyGraphEventCallback(
    const DevGraph *graph,
    DevEventType event,
    int unusedLocalClientNum)
{
    Actor_CommonAccuracyGraphEventCallback(graph, event, WEAP_ACCURACY_AI_VS_PLAYER);
}

void __cdecl Actor_AccuracyGraphTextCallback(
    const DevGraph *graph,
    double inputX,
    double inputY,
    char *text,
    const int textLength)
{
    sprintf(
        text,
        "Distance: %.2f, Accuracy: %.4f",
        inputX * 4000.0f,
        inputY
    ); // KISAKTODO: unsure
}

void __cdecl G_SwapAccuracyBuffers()
{
    ++g_accuracyBufferIndex;
}

DevGraph *__cdecl Actor_InitWeaponAccuracyGraphForWeaponType(
    unsigned int weaponIndex,
    WeapAccuracyType accuracyType,
    void(__cdecl *eventCallback)(const DevGraph *, DevEventType, int))
{
    WeaponDef *WeaponDef; // r31
    int *v7; // r21
    DevGraph *result; // r3
    __int32 v9; // r25
    float *v10; // r24
    float *v11; // r11
    int v12; // r29
    int v13; // r22
    unsigned int v14; // r11
    unsigned int v15; // r28
    unsigned int v16; // r11
    unsigned int v17; // r28
    char *v18; // r27
    unsigned int v19; // r10
    unsigned int v20; // r11
    float *v21; // r11

    WeaponDef = BG_GetWeaponDef(weaponIndex);
    if (!WeaponDef)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 1115, 0, "%s", "weaponDef");
    v7 = &WeaponDef->accuracyGraphKnotCount[accuracyType];
    if (!*v7)
        return 0;
    v9 = 4 * (accuracyType + 477);
    v10 = (float *)WeaponDef->originalAccuracyGraphKnots[accuracyType];
    v11 = *(float **)((char *)&WeaponDef->szInternalName + v9);
    v12 = g_accuracyBufferIndex;
    if (v10 != v11)
    {
        v13 = ((_BYTE)g_accuracyBufferIndex - 1) & 1;
        v14 = (char *)&v11[-8192 * v13] - (char *)g_accuGraphBuf;
        if (v14 < 0x8000)
        {
            v15 = v14 >> 8;
            if (v14 >> 8 >= 0x80)
            {
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp",
                    1131,
                    0,
                    "%s\n\t(oldWeaponIndex) = %i",
                    "(oldWeaponIndex < (sizeof( g_accuGraphBuf[prevBufferIndex] ) / (sizeof( g_accuGraphBuf[prevBufferIndex][0] ) *"
                    " (sizeof( g_accuGraphBuf[prevBufferIndex] ) != 4 || sizeof( g_accuGraphBuf[prevBufferIndex][0] ) <= 4))))",
                    v14 >> 8);
                v12 = g_accuracyBufferIndex;
            }
            v16 = (v13 << 7) + v15;
            if (dword_82C31FB0[v16] == v12 - 1 && g_accuGraphWeapon[0][v16] == WeaponDef)
                v10 = *(float **)((char *)&WeaponDef->szInternalName + v9);
        }
    }
    v17 = ((v12 << 7) & 0x80) + weaponIndex;
    v18 = (char *)g_accuGraphBuf + 128 * (2 * v17 + accuracyType);
    memcpy(v18, v10, 8 * *v7);
    v19 = v17;
    v20 = g_numAccuracyGraphs;
    g_accuGraphWeapon[0][v19] = WeaponDef;
    dword_82C31FB0[v19] = v12;
    *(const char **)((char *)&WeaponDef->szInternalName + v9) = v18;
    if (v20 >= 0x80)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp",
            1146,
            0,
            "%s",
            "g_accuracyGraphCount < ARRAY_COUNT( g_accuracyGraphs )");
        v20 = g_numAccuracyGraphs;
    }
    result = &g_accuracyGraphs[v20];
    g_numAccuracyGraphs = v20 + 1;
    result->knotCountMax = 16;
    v21 = *(float **)((char *)&WeaponDef->szInternalName + v9);
    result->knotCount = v7;
    result->eventCallback = eventCallback;
    result->textCallback = (void(__cdecl *)(const DevGraph *, const float, const float, char *, const int))Actor_AccuracyGraphTextCallback;
    result->data = WeaponDef;
    result->knots = (float (*)[2])v21;
    return result;
}

void __cdecl Actor_CopyAccuGraphBuf(WeaponDef *from, WeaponDef *to)
{
    int v3; // r22
    WeaponDef **i; // r11
    unsigned int v5; // r25
    int *accuracyGraphKnotCount; // r30
    int v7; // r17
    unsigned int v8; // r31
    char *v9; // r29
    unsigned int v10; // r31
    int v11; // r11

    v3 = 1;
    if (bg_lastParsedWeaponIndex)
    {
        for (i = &bg_weaponDefs[1]; *i != from; ++i)
        {
            if (++v3 > bg_lastParsedWeaponIndex)
                return;
        }
        v5 = 0;
        accuracyGraphKnotCount = to->accuracyGraphKnotCount;
        v7 = (char *)from - (char *)to;
        do
        {
            if (*(int *)((char *)accuracyGraphKnotCount + v7) && *accuracyGraphKnotCount)
            {
                v8 = ((g_accuracyBufferIndex << 7) & 0x80) + v3;
                v9 = (char *)g_accuGraphBuf + 128 * (2 * v8 + v5);
                memcpy(v9, (const void *)*(accuracyGraphKnotCount - 2), 8 * accuracyGraphKnotCount[2]);
                v10 = v8;
                if (dword_82C31FB0[v10] != g_accuracyBufferIndex)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp",
                        1180,
                        0,
                        "%s",
                        "g_accuGraphTime[bufferIndex][weapIndex] == g_accuracyBufferIndex");
                if (g_accuGraphWeapon[0][v10] != from)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp",
                        1181,
                        0,
                        "%s",
                        "g_accuGraphWeapon[bufferIndex][weapIndex] == from");
                v11 = accuracyGraphKnotCount[2];
                *(accuracyGraphKnotCount - 4) = (int)v9;
                *accuracyGraphKnotCount = v11;
                if (*(float *)&v9[8 * v11 - 8] != 1.0)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp",
                        1185,
                        1,
                        "%s",
                        "to->accuracyGraphKnots[accuracyType][to->accuracyGraphKnotCount[accuracyType] - 1][0] == 1.0f");
            }
            ++v5;
            ++accuracyGraphKnotCount;
        } while (v5 < 2);
    }
}

void __cdecl Actor_InitWeaponAccuracyGraphForWeapon(unsigned int weaponIndex)
{
    WeaponDef *WeaponDef; // r29
    DevGraph *inited; // r31
    DevGraph *v4; // r31
    char v5[288]; // [sp+50h] [-120h] BYREF

    WeaponDef = BG_GetWeaponDef(weaponIndex);
    if (!WeaponDef)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 1201, 0, "%s", "weaponDef");
    inited = Actor_InitWeaponAccuracyGraphForWeaponType(
        weaponIndex,
        WEAP_ACCURACY_AI_VS_AI,
        Actor_AiVsAiAccuracyGraphEventCallback);
    if (inited)
    {
        sprintf(v5, "AI/AI Vs. AI Accuracy/%s", WeaponDef->szInternalName);
        DevGui_AddGraph(v5, inited);
    }
    v4 = Actor_InitWeaponAccuracyGraphForWeaponType(
        weaponIndex,
        WEAP_ACCURACY_AI_VS_PLAYER,
        Actor_AiVsPlayerAccuracyGraphEventCallback);
    if (v4)
    {
        sprintf(v5, "AI/AI Vs. Player Accuracy/%s", WeaponDef->szInternalName);
        DevGui_AddGraph(v5, v4);
    }
}

void __cdecl Actor_ShutdownWeaponAccuracyGraph()
{
    unsigned int v0; // r30
    const char ***p_data; // r29
    const char **v2; // r31
    char v3[320]; // [sp+50h] [-140h] BYREF

    v0 = 0;
    if (g_numAccuracyGraphs)
    {
        p_data = (const char ***)&g_accuracyGraphs[0].data;
        do
        {
            v2 = *p_data;
            sprintf(v3, "AI/AI Vs. AI Accuracy/%s", **p_data);
            DevGui_RemoveMenu(v3);
            sprintf(v3, "AI/AI Vs. Player Accuracy/%s", *v2);
            DevGui_RemoveMenu(v3);
            ++v0;
            p_data += 8;
        } while (v0 < g_numAccuracyGraphs);
    }
    g_numAccuracyGraphs = 0;
}

