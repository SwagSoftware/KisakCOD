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
#include <universal/profile.h>

#define AI_DEBUG_ACCURACY_MSG_COUNT 8
static const float ACTOR_MAX_ACCURAY_DISTANCE = 4000.0f;


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
int g_accuGraphTime[2][128]; // [bufferIndex][weapIndex] (dword_82C31FB0)

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
}

void __cdecl Actor_DebugAccuracyMsg(
    unsigned int msgIndex,
    const char *msg,
    float accuracy,
    const float *color)
{
    bcassert(msgIndex, AI_DEBUG_ACCURACY_MSG_COUNT);
    iassert(color);

    if (msg)
    {
        I_strncpyz(debugAccuracyStrings[msgIndex], va("%s: %1.3f", msg, accuracy), 32);
    }
    else
    {
        debugAccuracyStrings[msgIndex][0] = 0;
    }

    debugAccuracyColors[msgIndex][0] = color[0];
    debugAccuracyColors[msgIndex][1] = color[1];
    debugAccuracyColors[msgIndex][2] = color[2];
    debugAccuracyColors[msgIndex][3] = color[3];
}

float __cdecl Actor_GetAccuracyFraction(
    float dist,
    const WeaponDef *weapDef,
    WeapAccuracyType accuracyType)
{
    iassert(accuracyType >= WEAP_ACCURACY_AI_VS_AI && accuracyType < WEAP_ACCURACY_COUNT);

    float (*knots)[2] = weapDef->accuracyGraphKnots[accuracyType];
    int knotCount = weapDef->accuracyGraphKnotCount[accuracyType];

    vassert(
        !knots || !knotCount || knots[knotCount - 1][0] == 1.0f,
        "weapon '%s' has invalid graph...max range %f != 1.0.",
        weapDef->szInternalName,
        knots[knotCount - 1][0]);

    float frac = dist / ACTOR_MAX_ACCURAY_DISTANCE;
    if (frac > 1.0f)
        frac = 1.0f;

    if (!knots || !knotCount)
    {
        if (accuracyType == WEAP_ACCURACY_AI_VS_PLAYER)
            Com_Error(ERR_DROP, "No 'AI vs Player' accuracy graph for weapon '%s'", weapDef->szInternalName);
        else if (accuracyType == WEAP_ACCURACY_AI_VS_AI)
            Com_Error(ERR_DROP, "No 'AI vs AI' accuracy graph for weapon '%s'", weapDef->szInternalName);
    }

    return GraphGetValueFromFraction(knotCount, knots, frac);
}

float __cdecl Actor_GetWeaponAccuracy(
    const actor_s *self,
    const sentient_s *enemy,
    const WeaponDef *weapDef,
    WeapAccuracyType accuracyType)
{
    float distance; // fp1
    float accuracy; // fp1
    float selfOrigin[3]; // [sp+50h] [-60h] BYREF
    float enemyOrigin[3]; // [sp+60h] [-50h] BYREF

    iassert(self);
    iassert(enemy);
    iassert(enemy->ent);
    iassert(weapDef);
    bcassert(accuracyType, WEAP_ACCURACY_COUNT);

    Sentient_GetOrigin(self->sentient, selfOrigin);
    Sentient_GetOrigin(enemy, enemyOrigin);

    distance = Vec3Distance(enemyOrigin, selfOrigin);

    if (accuracyType == WEAP_ACCURACY_AI_VS_PLAYER)
        distance *= ai_accuracyDistScale->current.value;

    accuracy = Actor_GetAccuracyFraction(distance, weapDef, accuracyType);

    iassert(accuracy >= 0.0f && accuracy <= 1.0f);
    return accuracy;
}

float __cdecl Actor_GetPlayerStanceAccuracy(const actor_s *self, const sentient_s *enemy)
{
    iassert(self);
    iassert(enemy);
    iassert(enemy->ent);
    iassert(enemy->ent->client);

    int pm_flags = enemy->ent->client->ps.pm_flags;

    if ((pm_flags & 1) != 0)
    {
        return 0.5f;
    }
    else if ((pm_flags & 2) != 0)
    {
        return 0.75f;
    }
    else
    {
        return 1.0f;
    }
}

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

    // Unit direction from self -> enemy.
    float dirX = dx * inv_dist;
    float dirY = dy * inv_dist;
    float dirZ = dz * inv_dist;

    float dot = velocity[0] * dirX + velocity[1] * dirY + velocity[2] * dirZ;

    float absDot = fabsf(dot);
    float clampedDot = I_fmin(absDot, 250.0f);

    float accuracy = 1.0f - clampedDot * 0.004f;

    if (accuracy >= 0.1f)
    {
        iassert(accuracy >= 0.0f && accuracy <= 1.0f);
        return accuracy;
    }

    return 0.1f;
}

float __cdecl Actor_GetPlayerSightAccuracy(actor_s *self, const sentient_s *enemy)
{
    float accuracyFactor; // fp25
    float accuracy; // fp31

    float enemyEyePos[3];

    iassert(self);
    iassert(enemy);
    iassert(enemy->ent);
    iassert(enemy->ent->client);

    Sentient_GetEyePosition(enemy, enemyEyePos);

    accuracyFactor = 0.0;

    float enemyOrigin[3];
    Vec3Copy(enemy->ent->client->ps.origin, enemyOrigin);

    float enemyEyeOffset[3];
    Vec3Sub(enemyEyePos, enemy->ent->client->ps.origin, enemyEyeOffset);
    if (Actor_CanSeeEntityPoint(self, enemyEyePos, enemy->ent))
        accuracyFactor = 10.0;

    float eyeOffset75[3];
    Vec3Mad(enemyOrigin, 0.75f, enemyEyeOffset, eyeOffset75);
    if (Actor_CanSeeEntityPoint(self, eyeOffset75, enemy->ent))
        accuracyFactor += 30.0f;

    float eyeOffset50[3];
    Vec3Mad(enemyOrigin, 0.5f, enemyEyeOffset, eyeOffset50);
    if (Actor_CanSeeEntityPoint(self, eyeOffset50, enemy->ent))
        accuracyFactor += 30.0f;

    float eyeOffset25[3];
    Vec3Mad(enemyOrigin, 0.25f, enemyEyeOffset, eyeOffset25);
    if (Actor_CanSeeEntityPoint(self, eyeOffset25, enemy->ent))
        accuracyFactor += 30.0f;

    accuracy = (accuracyFactor * 0.01f);

    if (accuracy >= 0.1f)
    {
        iassert(accuracy >= 0.0f && accuracy <= 1.0f);
        return accuracy;
    }

    return 0.1f;
}

float __cdecl Actor_GetFinalAccuracy(actor_s *self, weaponParms *wp, double accuracyMod)
{
    double accuracy; // fp1
    sentient_s *enemy; // r30
    WeaponDef *weapDef; // r5
    double WeaponAccuracy; // fp30
    double PlayerStanceAccuracy; // fp29
    double PlayerMovementAccuracy; // fp1
    double playerSightAccuracy; // fp27

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
        playerSightAccuracy = Actor_GetPlayerSightAccuracy(self, enemy);
    }
    else
    {
        WeaponAccuracy = Actor_GetWeaponAccuracy(self, enemy, weapDef, WEAP_ACCURACY_AI_VS_AI);
        PlayerStanceAccuracy = 1.0;
        PlayerMovementAccuracy = 1.0;
        playerSightAccuracy = 1.0;
    }
    accuracy = (float)((float)((float)((float)((float)((float)(self->accuracy * enemy->attackerAccuracy)
        * (float)accuracyMod)
        * (float)WeaponAccuracy)
        * (float)PlayerStanceAccuracy)
        * (float)PlayerMovementAccuracy)
        * (float)playerSightAccuracy);

    if (ai_debugAccuracy->current.enabled && ai_debugEntIndex->current.integer == self->ent->s.number)
    {
        Actor_DebugAccuracyMsg(0, "Self    ", self->accuracy, colorWhite);
        Actor_DebugAccuracyMsg(1, "Target  ", enemy->attackerAccuracy, colorWhite);
        Actor_DebugAccuracyMsg(2, "Script  ", accuracyMod, colorWhite);
        Actor_DebugAccuracyMsg(3, "Weapon  ", WeaponAccuracy, colorWhite);
        Actor_DebugAccuracyMsg(4, "Stance  ", PlayerStanceAccuracy, colorWhite);
        Actor_DebugAccuracyMsg(5, "Movement", PlayerMovementAccuracy, colorWhite);
        Actor_DebugAccuracyMsg(6, "Sight   ", playerSightAccuracy, colorWhite);
        Actor_DebugAccuracyMsg(7, "TOTAL   ", accuracy, colorRed);
    }

    iassert(accuracy >= 0.0f);

    float dbg = accuracy <= 0.0f ? 0.0f : (accuracy >= 1.0f ? 1.0f : accuracy);

    self->debugLastAccuracy = dbg;

    return dbg;
}

void __cdecl Actor_FillWeaponParms(actor_s *self, weaponParms *wp)
{
    iassert(self);
    iassert(wp);
    nanassertvec3(self->vLookForward);
    nanassertvec3(self->vLookRight);
    nanassertvec3(self->vLookUp);

    Vec3Copy(self->vLookForward, wp->forward);
    Vec3Copy(self->vLookRight, wp->right);
    Vec3Copy(self->vLookUp, wp->up);

    if (!Actor_GetMuzzleInfo(self, wp->muzzleTrace, wp->gunForward))
    {
        Actor_GetEyePosition(self, wp->muzzleTrace);
        Vec3Copy(wp->forward, wp->gunForward);
    }

    nanassertvec3(wp->muzzleTrace);
    nanassertvec3(wp->gunForward);
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

    // Normalized direction from muzzle to eye
    float dir[3];
    Vec3Sub(eyePos, wp->muzzleTrace, dir);
    float dist = Vec3Normalize(dir);
    iassert(dist > 0.0f);

    // Lateral axis = normalize(cross(dir, up))
    float lat[3];
    Vec3Cross(dir, wp->up, lat);
    float latLen = Vec3Normalize(lat);
    iassert(latLen > 0.0f);

    // Determine spread parameters
    float vertMax, horizMax;
    if (enemy->ent->client)
    {
		iassert(accuracy >= 0.0f && accuracy <= 1.0f);
		horizMax = ((1.0f - accuracy) + 1.0f) * 0.5f * -44.0f;
		vertMax = ((1.0f - accuracy) * 0.9f + 0.1f) * 8.0f;
    }
    else
    {
        iassert(accuracy >= 0.0f && accuracy <= 1.0f);
        horizMax = ((1.0f - accuracy) + 1.0f) * 0.5f * -44.0f;
        vertMax = ((1.0f - accuracy) * 0.9f + 0.1f) * 15.0f;
    }

    // Add randomness
    float rVert = G_crandom() * vertMax;
    float rHoriz = G_random() * horizMax;
    float center = rHoriz + headHeight;

    // forward = center*up + rVert*lat + (eyePos - muzzle), normalized
    Vec3ScaleMad(center, wp->up, rVert, lat, wp->forward);
    Vec3Add(wp->forward, eyePos, wp->forward);
    Vec3Sub(wp->forward, wp->muzzleTrace, wp->forward);
    float fwdLen = Vec3Normalize(wp->forward);
    iassert(fwdLen > 0.0f);
}

// aislop
void Actor_HitTarget(const weaponParms *wp, const float *target, float *forward)
{
    iassert(wp && target && forward);

    const float *up = wp->up;

    // Normalized direction from muzzle to target
    float dir[3];
    Vec3Sub(target, wp->muzzleTrace, dir);
    float dist = Vec3Normalize(dir);
    iassert(dist > 0.0f);

    // Lateral axis = normalize(cross(dir, up))
    float lateral[3];
    Vec3Cross(dir, up, lateral);
    float lateralLen = Vec3Normalize(lateral);
    iassert(lateralLen > 0.0f);

    // Random spread offset along the lateral and up axes
    float r1 = G_crandom() * 8.0f;
    float r2 = G_crandom() * 8.0f;

    float aimPos[3];
    Vec3Mad(target, r1, lateral, aimPos);   // aimPos  = target + r1 * lateral
    Vec3Mad(aimPos, r2, up, aimPos);        // aimPos += r2 * up

    // Final normalized forward vector from muzzle to the spread aim point
    Vec3Sub(aimPos, wp->muzzleTrace, forward);
    float fwdLen = Vec3Normalize(forward);
    iassert(fwdLen > 0.0f);
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

static float outerRadius = 6969.0f;
void __cdecl Actor_MissSentient(weaponParms *wp, sentient_s *enemy, float accuracy)
{
    gentity_s *ent = enemy->ent;
    if (outerRadius == 6969.0f)
    {
        outerRadius = sqrtf(15.0f * 15.0f * 2.0f * 3.0f);
    }

    float vEyePos[3];
    Sentient_GetEyePosition(enemy, vEyePos);

    float dx = vEyePos[0] - wp->muzzleTrace[0];
    float dy = vEyePos[1] - wp->muzzleTrace[1];
    float dz = vEyePos[2] - wp->muzzleTrace[2];
    float dist = sqrtf(dx * dx + dy * dy + dz * dz);
    float inv_dist = (dist != 0.0f) ? (1.0f / dist) : 1.0f;

    float dir[3];
    dir[0] = dx * inv_dist;
    dir[1] = dy * inv_dist;
    dir[2] = dz * inv_dist;

    float crossVec[3];
    Vec3Cross(dir, wp->up, crossVec);

    float clen = sqrtf(crossVec[0] * crossVec[0] + crossVec[1] * crossVec[1] + crossVec[2] * crossVec[2]);
    float inv_clen = (clen != 0.0f) ? (1.0f / clen) : 1.0f;

    float lat[3];
    lat[0] = crossVec[0] * inv_clen;
    lat[1] = crossVec[1] * inv_clen;
    lat[2] = crossVec[2] * inv_clen;

    const float negOffset = -(15.0f + 1.0f);

    float aimPos[3];
    if (ent->client)
    {
        float signf = (G_random() > 0.5f) ? 1.0f : -1.0f;
        float r2 = G_random();
        float lateral = (r2 + 1.0f) * 8.0f * signf;

        aimPos[0] = lat[0] * lateral + negOffset * dir[0] + vEyePos[0];
        aimPos[1] = lat[1] * lateral + negOffset * dir[1] + vEyePos[1];
        aimPos[2] = lat[2] * lateral + negOffset * dir[2] + vEyePos[2];

        float headHeight = (float)Sentient_GetHeadHeight(enemy);
        float r3 = G_random();
        float vertical = headHeight - r3 * 44.0f;

        aimPos[0] += wp->up[0] * vertical;
        aimPos[1] += wp->up[1] * vertical;
        aimPos[2] += wp->up[2] * vertical;
    }
    else
    {
        float signf = (G_random() > 0.5f) ? 1.0f : -1.0f;
        float inaccuracy = 1.0f - accuracy;
        float r1 = G_random();
        float lateral = (r1 * inaccuracy * 10.0f + outerRadius) * signf;

        aimPos[0] = lat[0] * lateral + negOffset * dir[0] + vEyePos[0];
        aimPos[1] = lat[1] * lateral + negOffset * dir[1] + vEyePos[1];
        aimPos[2] = lat[2] * lateral + negOffset * dir[2] + vEyePos[2];

        float vertical = G_crandom() * inaccuracy * -22.0f;

        aimPos[0] += wp->up[0] * vertical;
        aimPos[1] += wp->up[1] * vertical;
        aimPos[2] += wp->up[2] * vertical;
    }

    wp->forward[0] = aimPos[0] - wp->muzzleTrace[0];
    wp->forward[1] = aimPos[1] - wp->muzzleTrace[1];
    wp->forward[2] = aimPos[2] - wp->muzzleTrace[2];

    float fwdLenSq = wp->forward[0] * wp->forward[0]
                   + wp->forward[1] * wp->forward[1]
                   + wp->forward[2] * wp->forward[2];
    float fwdLen = sqrtf(fwdLenSq);
    float inv_fwdLen = (fwdLen != 0.0f) ? (1.0f / fwdLen) : 1.0f;
    wp->forward[0] *= inv_fwdLen;
    wp->forward[1] *= inv_fwdLen;
    wp->forward[2] *= inv_fwdLen;
}


// some aislop
static float outerRadius_0 = 6969.0f;
void __cdecl Actor_MissTarget(const weaponParms *wp, const float *target, float *forward)
{
    float startX; // fp0
    float startY; // fp13
    float startZ; // fp12
    float len; // fp11
    float len2; // fp0
    float crossN[3]; // fp27
    float baseX; // fp30
    float baseY; // fp29
    float baseZ; // fp28
    float len3; // fp12
    float vec[3];
    float crossVec[3]; // [sp+60h] [-80h] BYREF

    if (outerRadius_0 == 6969.0f)
    {
        float a = actorMaxs[1];
        outerRadius_0 = sqrtf(a * a * 2.0f * 3.0f);
    }

    startX = (target[0] - wp->muzzleTrace[0]);
    startY = (target[2] - wp->muzzleTrace[2]);
    startZ = (target[1] - wp->muzzleTrace[1]);

    len = (1.0f / sqrtf(((startZ * startZ) + ((startY * startY) + (startX * startX)))));

    vec[0] = len * (target[0] - wp->muzzleTrace[0]);
    vec[1] = len * (target[1] - wp->muzzleTrace[1]);
    vec[2] = len * (target[2] - wp->muzzleTrace[2]);

    Vec3Cross(vec, wp->up, crossVec);

    len2 = (1.0f / sqrtf(((crossVec[0] * crossVec[0]) + ((crossVec[2] * crossVec[2]) + (crossVec[1] * crossVec[1])))));

    crossN[0] = (crossVec[0] * len2);
    crossN[1] = (crossVec[1] * len2);
    crossN[2] = (crossVec[2] * len2);

    int sign_i = (G_random() > 0.5f) ? 1 : -1;
    float rnd_sign_f = (float)sign_i;
    
    float neg_offset = -(playerMaxs[0] + 1.0f);
    baseX = neg_offset * vec[0] + target[0];
    baseY = neg_offset * vec[1] + target[1];
    baseZ = neg_offset * vec[2] + target[2];

    float cross_scale = rnd_sign_f * outerRadius_0;
    baseX = crossN[0] * cross_scale + baseX;
    baseY = crossN[1] * cross_scale + baseY;
    baseZ = crossN[2] * cross_scale + baseZ;

    float cr_scale = G_crandom() * 12.0f;
    
    float finalX = wp->up[0] * cr_scale + baseX;
    float finalY = wp->up[1] * cr_scale + baseY;
    float finalZ = wp->up[2] * cr_scale + baseZ;

    forward[0] = finalX - wp->muzzleTrace[0];
    forward[1] = finalY - wp->muzzleTrace[1];
    forward[2] = finalZ - wp->muzzleTrace[2];

    float flen = sqrtf(forward[0] * forward[0] + forward[1] * forward[1] + forward[2] * forward[2]);
    float finv = 1.0f / flen;

    forward[0] *= finv;
    forward[1] *= finv;
    forward[2] *= finv;
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
    Vec3Copy(wp->gunForward, wp->forward);
}

void __cdecl Actor_ShootPos(actor_s *self, weaponParms *wp, float *pos)
{
    Vec3Sub(pos, wp->muzzleTrace, wp->forward);
    Vec3NormalizeFast(wp->forward);
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
        Vec3Copy(dest, wp->forward);
    }
}

void __cdecl Actor_Shoot(actor_s *self, float accuracyMod, float (*posOverride)[3], enumLastShot lastShot)
{
    gentity_s *ent; // r27
    unsigned int weaponName; // r3
    const char *v11; // r3
    unsigned int weapon; // r26
    gentity_s *TargetEntity;
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

    PROF_SCOPED("Actor_Shoot");

    if (self->lastShotTime == level.time)
    {
        Com_PrintError(18, "ERROR: Attempt for same actor (entnum %d) to shoot/melee more than once in a frame.\n", ent->s.number);
        return;
    }

    weaponName = self->weaponName;
    self->lastShotTime = level.time;
    v11 = SL_ConvertToString(weaponName);
    weapon = G_GetWeaponIndexForName(v11);
    wp.weapDef = BG_GetWeaponDef(weapon);
    Actor_FillWeaponParms(self, &wp);
    TargetEntity = Actor_GetTargetEntity(self);

    if (posOverride)
    {
        const float *pos = *posOverride;
        float dx = pos[0] - wp.muzzleTrace[0];
        float dy = pos[1] - wp.muzzleTrace[1];
        float dz = pos[2] - wp.muzzleTrace[2];

        float len = sqrtf(dx * dx + dy * dy + dz * dz);
        invLen = (len != 0.0f) ? 1.0f / len : 1.0f;

        wp.forward[0] = invLen * dx;
        wp.forward[1] = invLen * dy;
        wp.forward[2] = invLen * dz;

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
        Bullet_Fire(ent, 0.0, &wp, ent, level.time);
        if (lastShot == LAST_SHOT_IN_CLIP)
        {
            G_AddEvent(ent, EV_FIRE_WEAPON_LASTSHOT, 0);
            return;
        }
        G_AddEvent(ent, EV_FIRE_WEAPON, 0);
        return;
    }

    if (weapType == WEAPTYPE_PROJECTILE)
    {
        Weapon_RocketLauncher_Fire(ent, weapon, 0.0, &wp, vec3_origin, TargetEntity, NULL);
        G_AddEvent(ent, EV_FIRE_WEAPON, 0);
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
        G_AddEvent(self->ent, EV_FIRE_WEAPON, 0);
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
    float vEyePos[3]; // [sp+50h] [-90h] BYREF
    //float v22; // [sp+54h] [-8Ch]
    //float v23; // [sp+58h] [-88h]
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
        Sentient_GetEyePosition(TargetSentient, vEyePos);
        v9 = (float)(vEyePos[0] - wp.muzzleTrace[0]);
        v10 = (float)(vEyePos[1] - wp.muzzleTrace[1]);
        if (direction)
        {
            v11 = (float)((float)sqrtf((float)((float)((float)(vEyePos[1] - wp.muzzleTrace[1])
                * (float)(vEyePos[1] - wp.muzzleTrace[1])) + (float)((float)(vEyePos[0] - wp.muzzleTrace[0])
                    * (float)(vEyePos[0] - wp.muzzleTrace[0]))))
                / (float)sqrtf((float)((float)(*direction * *direction) + (float)(direction[1] * direction[1]))));
            v9 = (float)(*direction * (float)v11);
            v10 = (float)((float)v11 * direction[1]);
        }

        //_FP9 = -sqrtf((float)((float)((float)v9 * (float)v9)
        //    + (float)((float)((float)(v23 - v24[0].muzzleTrace[2]) * (float)(v23 - v24[0].muzzleTrace[2]))
        //        + (float)((float)v10 * (float)v10))));
        //__asm { fsel      f12, f9, f11, f12 }
        //v14 = (float)((float)1.0 / (float)_FP12);

        mag = sqrtf(v9 * v9 + v10 * v10 + (vEyePos[2] - wp.muzzleTrace[2]) * (vEyePos[2] - wp.muzzleTrace[2]));
        v14 = mag > 0.0f ? 1.0f / mag : 0.0f;

        wp.forward[0] = (float)v14 * (float)v9;
        wp.forward[1] = (float)v10 * (float)v14;
        v15 = (float)((float)(vEyePos[2] - wp.muzzleTrace[2]) * (float)v14);
        goto LABEL_11;
    }
    if (direction)
    {
        v16 = direction[2];

        //_FP9 = -sqrtf(...);                   ; -mag
        //__asm { fsel      f11, f9, f10, f11 } ; (mag == 0) ? safe : mag
        //v19 = 1.0 / _FP11
        x = direction[0], y = direction[1], z = direction[2];
        mag = sqrtf(x * x + y * y + z * z);
        v19 = (mag > 0.0f) ? (1.0f / mag) : 0.0f;

        v20 = (float)(direction[1] * (float)v19);
        wp.forward[0] = (float)v19 * *direction;
        wp.forward[1] = v20;
        v15 = (float)((float)v16 * (float)v19);
    LABEL_11:
        wp.forward[2] = v15;
    }
    ent->s.weapon = WeaponIndexForName;
    iassert(ent->s.weapon == WeaponIndexForName); // weapon index must fit the s.weapon byte

    // CoD3SP calls Weapon_Melee(ent, wp, 64.0, 0.0, 0.0) with no time arg -- its
    // Weapon_Melee/Weapon_Melee_internal are time-independent. kisak's signature
    // adds a gametime param that the melee path ignores, so level.time is harmless.
    return Weapon_Melee(self->ent, &wp, 64.0, 0.0, 0.0, level.time);
}

float __cdecl Sentient_GetScarinessForDistance(sentient_s *self, sentient_s *enemy, double fDist)
{
    unsigned int WeaponIndexForName; // r3
    WeaponDef *weapDef; // r3
    float AccuracyFraction; // fp1
    double value; // fp0

    iassert(self);
    iassert(self->ent);
    iassert(enemy);
    iassert(fDist >= 0);

    if (self->ent->actor)
    {
        WeaponIndexForName = G_GetWeaponIndexForName(SL_ConvertToString(self->ent->actor->weaponName));
        weapDef = BG_GetWeaponDef(WeaponIndexForName);
        AccuracyFraction = Actor_GetAccuracyFraction(fDist, weapDef, WEAP_ACCURACY_AI_VS_AI);
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

    return AccuracyFraction;
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
    //_QWORD v15[1030]; // [sp+70h] [-2030h] BYREF
    char v15[8240];
    static_assert(sizeof(_QWORD[1030]) == sizeof(char[8240])); // We changed the type, so this is a check

    if (!graph)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 1054, 0, "%s", "graph");
    if (!graph->data)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_aim.cpp", 1055, 0, "%s", "graph->data");
    data = (const char **)graph->data;
    if (event == EVENT_ACCEPT)
    {
        memset(v15, 0, 0x2000u);
        snprintf((char *)v15, ARRAYSIZE(v15), "Weapon: %s\nKnot Count: %d\n", *data, *graph->knotCount); // This probably needs to be changed type
        v7 = 0;
        if (*graph->knotCount > 0)
        {
            v8 = 0;
            do
            {
                snprintf(
                    v14,
                    ARRAYSIZE(v14),
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
    const float inputX,
    const float inputY,
    char *text,
    const int textLength)
{
    sprintf(text, "Distance: %.2f, Accuracy: %.4f", inputX * ACTOR_MAX_ACCURAY_DISTANCE, inputY);
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
    WeaponDef *weaponDef = BG_GetWeaponDef(weaponIndex);
    iassert(weaponDef);

    int *knotCount = &weaponDef->accuracyGraphKnotCount[accuracyType];
    if (!*knotCount)
        return 0;

    const unsigned int bufferIndex = g_accuracyBufferIndex & 1;
    float (*srcKnots)[2] = weaponDef->originalAccuracyGraphKnots[accuracyType];
    float (*workingKnots)[2] = weaponDef->accuracyGraphKnots[accuracyType];

    if (srcKnots != workingKnots)
    {
        const unsigned int prevBufferIndex = (g_accuracyBufferIndex - 1) & 1;
        unsigned int byteOff = (unsigned int)((const char *)workingKnots - (const char *)g_accuGraphBuf[prevBufferIndex]);
        if (byteOff < sizeof(g_accuGraphBuf[prevBufferIndex]))
        {
            unsigned int weapInPrev = byteOff / sizeof(AccuracyGraphBackup);
            iassert(weapInPrev < 128);
            if (g_accuGraphTime[prevBufferIndex][weapInPrev] == (int)(g_accuracyBufferIndex - 1)
                && g_accuGraphWeapon[prevBufferIndex][weapInPrev] == weaponDef)
            {
                srcKnots = workingKnots;
            }
        }
    }

    float (*bufSlot)[2] = g_accuGraphBuf[bufferIndex][weaponIndex].accuracyGraphKnots[accuracyType];
    memcpy(bufSlot, srcKnots, 8 * *knotCount);
    g_accuGraphWeapon[bufferIndex][weaponIndex] = weaponDef;
    g_accuGraphTime[bufferIndex][weaponIndex] = g_accuracyBufferIndex;
    weaponDef->accuracyGraphKnots[accuracyType] = bufSlot;

    iassert(g_numAccuracyGraphs < ARRAY_COUNT(g_accuracyGraphs));

    DevGraph *result = &g_accuracyGraphs[g_numAccuracyGraphs++];
    result->knotCountMax = 16;
    result->knotCount = knotCount;
    result->eventCallback = eventCallback;
    result->textCallback = Actor_AccuracyGraphTextCallback;
    result->data = weaponDef;
    result->knots = bufSlot;
    return result;
}

void __cdecl Actor_CopyAccuGraphBuf(WeaponDef *from, WeaponDef *to)
{
    if (!bg_lastParsedWeaponIndex)
        return;

    // Find `from`'s index in the parsed weapon list.
    int fromIndex = 1;
    for (WeaponDef **i = &bg_weaponDefs[1]; *i != from; ++i)
    {
        if (++fromIndex > bg_lastParsedWeaponIndex)
            return; // `from` not found
    }

    const unsigned int bufferIndex = g_accuracyBufferIndex & 1;
    for (unsigned int accuracyType = 0; accuracyType < WEAP_ACCURACY_COUNT; ++accuracyType)
    {
        if (!from->accuracyGraphKnotCount[accuracyType] || !to->accuracyGraphKnotCount[accuracyType])
            continue;

        // Repoint `to`'s working graph at `from`'s double-buffer slot, copying
        // `to`'s original knots into it.
        float (*bufSlot)[2] = g_accuGraphBuf[bufferIndex][fromIndex].accuracyGraphKnots[accuracyType];
        int count = to->originalAccuracyGraphKnotCount[accuracyType];
        memcpy(bufSlot, to->originalAccuracyGraphKnots[accuracyType], 8 * count);

        // The slot must already have been set up for `from` this generation.
        iassert(g_accuGraphTime[bufferIndex][fromIndex] == g_accuracyBufferIndex);
        iassert(g_accuGraphWeapon[bufferIndex][fromIndex] == from);

        to->accuracyGraphKnots[accuracyType] = bufSlot;
        to->accuracyGraphKnotCount[accuracyType] = count;

        // The graph must reach normalized distance 1.0 at its last knot.
        iassert(bufSlot[count - 1][0] == 1.0f);
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
        snprintf(v5, ARRAYSIZE(v5), "AI/AI Vs. AI Accuracy/%s", WeaponDef->szInternalName);
        DevGui_AddGraph(v5, inited);
    }
    v4 = Actor_InitWeaponAccuracyGraphForWeaponType(
        weaponIndex,
        WEAP_ACCURACY_AI_VS_PLAYER,
        Actor_AiVsPlayerAccuracyGraphEventCallback);
    if (v4)
    {
        snprintf(v5, ARRAYSIZE(v5), "AI/AI Vs. Player Accuracy/%s", WeaponDef->szInternalName);
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
            snprintf(v3, ARRAYSIZE(v3), "AI/AI Vs. AI Accuracy/%s", **p_data);
            DevGui_RemoveMenu(v3);
            snprintf(v3, ARRAYSIZE(v3), "AI/AI Vs. Player Accuracy/%s", *v2);
            DevGui_RemoveMenu(v3);
            ++v0;
            p_data += 8;
        } while (v0 < g_numAccuracyGraphs);
    }
    g_numAccuracyGraphs = 0;
}

