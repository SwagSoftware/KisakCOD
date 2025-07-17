#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "g_public_mp.h"
#include "g_utils_mp.h"
#include <server/sv_world.h>


int __cdecl G_GetPlayerCorpseIndex(gentity_s *ent)
{
    int i; // [esp+4h] [ebp-4h]

    for (i = 0; i < 8; ++i)
    {
        if (g_scr_data.playerCorpseInfo[i].entnum == ent->s.number)
            return i;
    }
    if (!alwaysfails)
        MyAssertHandler(".\\game_mp\\g_player_corpse_mp.cpp", 99, 0, "G_GetPlayerCorpseIndex called for non player corpse");
    return 0;
}

int __cdecl G_GetFreePlayerCorpseIndex()
{
    float diff[4]; // [esp+0h] [ebp-30h] BYREF
    float playerPos[3]; // [esp+10h] [ebp-20h] BYREF
    int bestIndex; // [esp+1Ch] [ebp-14h]
    gentity_s *ent; // [esp+20h] [ebp-10h]
    int i; // [esp+24h] [ebp-Ch]
    float distSq; // [esp+28h] [ebp-8h]
    float bestDistSq; // [esp+2Ch] [ebp-4h]

    bestDistSq = -1.0;
    bestIndex = 0;
    ent = G_Find(0, 368, scr_const.player);
    if (!ent)
        MyAssertHandler(".\\game_mp\\g_player_corpse_mp.cpp", 122, 0, "%s", "ent");
    //LODWORD(diff[3]) = ent->s.lerp.pos.trBase; // KISAKTODO??
    playerPos[0] = ent->s.lerp.pos.trBase[0];
    playerPos[1] = ent->s.lerp.pos.trBase[1];
    playerPos[2] = ent->s.lerp.pos.trBase[2];
    for (i = 0; i < 8; ++i)
    {
        if (g_scr_data.playerCorpseInfo[i].entnum == -1)
            return i;
        ent = &level.gentities[g_scr_data.playerCorpseInfo[i].entnum];
        Vec3Sub(playerPos, ent->r.currentOrigin, diff);
        distSq = Vec3LengthSq(diff);
        if (distSq > (double)bestDistSq)
        {
            bestDistSq = distSq;
            bestIndex = i;
        }
    }
    ent = &level.gentities[g_scr_data.playerCorpseInfo[bestIndex].entnum];
    G_FreeEntity(ent);
    g_scr_data.playerCorpseInfo[bestIndex].entnum = -1;
    return bestIndex;
}

void __cdecl PlayerCorpse_Free(gentity_s *ent)
{
    int playerCorpseIndex; // [esp+0h] [ebp-4h]

    playerCorpseIndex = G_GetPlayerCorpseIndex(ent);
    if (g_scr_data.playerCorpseInfo[playerCorpseIndex].entnum != ent->s.number)
        MyAssertHandler(
            ".\\game_mp\\g_player_corpse_mp.cpp",
            156,
            0,
            "%s",
            "g_scr_data.playerCorpseInfo[playerCorpseIndex].entnum == ent->s.number");
    g_scr_data.playerCorpseInfo[playerCorpseIndex].entnum = -1;
}

void __cdecl G_RunCorpseMove(gentity_s *ent)
{
    int v1; // eax
    int passEntityNum; // [esp+14h] [ebp-E0h]
    bool v5; // [esp+18h] [ebp-DCh]
    bool v6; // [esp+1Ch] [ebp-D8h]
    float deltaChange[3]; // [esp+60h] [ebp-94h] BYREF
    bool isRagdoll; // [esp+6Fh] [ebp-85h]
    float origin[3]; // [esp+70h] [ebp-84h] BYREF
    float right[3]; // [esp+7Ch] [ebp-78h] BYREF
    float start[3]; // [esp+88h] [ebp-6Ch] BYREF
    float forward[3]; // [esp+94h] [ebp-60h] BYREF
    float left[3]; // [esp+A0h] [ebp-54h] BYREF
    float endpos[3]; // [esp+ACh] [ebp-48h] BYREF
    int corpseIndex; // [esp+B8h] [ebp-3Ch]
    bool haveDelta; // [esp+BFh] [ebp-35h]
    trace_t tr; // [esp+C0h] [ebp-34h] BYREF
    int mask; // [esp+ECh] [ebp-8h]
    corpseInfo_t *corpseInfo; // [esp+F0h] [ebp-4h]

    isRagdoll = Com_IsRagdollTrajectory(&ent->s.lerp.pos);
    iassert(ent->s.eType == ET_PLAYER_CORPSE);
    corpseIndex = G_GetPlayerCorpseIndex(ent);
    corpseInfo = &g_scr_data.playerCorpseInfo[corpseIndex];
    haveDelta = G_GetAnimDeltaForCorpse(ent, deltaChange);
    v5 = haveDelta && !corpseInfo->falling && Vec3LengthSq(deltaChange) > 1.0;
    haveDelta = v5;
    if (corpseInfo->falling || haveDelta)
    {
        if (corpseInfo->falling)
        {
            if (ent->s.lerp.pos.trType != TR_GRAVITY && !isRagdoll)
                MyAssertHandler(
                    ".\\game_mp\\g_player_corpse_mp.cpp",
                    239,
                    0,
                    "%s",
                    "ent->s.lerp.pos.trType == TR_GRAVITY || isRagdoll");
        }
        else if (ent->s.lerp.pos.trType != TR_INTERPOLATE && !isRagdoll)
        {
            MyAssertHandler(
                ".\\game_mp\\g_player_corpse_mp.cpp",
                241,
                0,
                "%s",
                "ent->s.lerp.pos.trType == TR_INTERPOLATE || isRagdoll");
        }
        BG_EvaluateTrajectory(&ent->s.lerp.pos, level.time, origin);
        if (haveDelta)
        {
            AngleVectors(ent->r.currentAngles, forward, right, 0);
            Vec3Scale(right, -1.0, left);
            Vec3Normalize(forward);
            Vec3Normalize(left);
            Vec3Mad(origin, deltaChange[0], forward, origin);
            Vec3Mad(origin, deltaChange[1], left, origin);
        }
        else
        {
            left[0] = 0.0;
            left[1] = 0.0;
            left[2] = 0.0;
        }
        if (!ent->clipmask)
            MyAssertHandler(".\\game_mp\\g_player_corpse_mp.cpp", 264, 0, "%s", "ent->clipmask");
        mask = ent->clipmask;
        if ((mask & ent->r.contents) != 0)
            MyAssertHandler(".\\game_mp\\g_player_corpse_mp.cpp", 268, 0, "%s", "!( ent->r.contents & mask )");
        if (ent->r.ownerNum.isDefined())
        {
            passEntityNum = ent->r.ownerNum.entnum();
            G_TraceCapsule(&tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passEntityNum, mask);
        }
        else
        {
            G_TraceCapsule(&tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, 1023, mask);
        }
        Vec3Lerp(ent->r.currentOrigin, origin, tr.fraction, endpos);
        ent->r.currentOrigin[0] = endpos[0];
        ent->r.currentOrigin[1] = endpos[1];
        ent->r.currentOrigin[2] = endpos[2];
        if (tr.startsolid)
            tr.fraction = 0.0;
        SV_LinkEntity(ent);
        G_RunThink(ent);
        if (ent->r.inuse)
        {
            if (tr.fraction == 1.0)
            {
                if (!corpseInfo->falling && haveDelta)
                {
                    ent->s.lerp.pos.trType = isRagdoll ? TR_RAGDOLL_INTERPOLATE : TR_INTERPOLATE;
                    ent->s.lerp.pos.trBase[0] = endpos[0];
                    ent->s.lerp.pos.trBase[1] = endpos[1];
                    ent->s.lerp.pos.trBase[2] = endpos[2];
                    ent->s.lerp.pos.trTime = 0;
                    ent->s.lerp.pos.trDuration = 0;
                    ent->s.lerp.pos.trDelta[0] = 0.0;
                    ent->s.lerp.pos.trDelta[1] = 0.0;
                    ent->s.lerp.pos.trDelta[2] = 0.0;
                    origin[2] = origin[2] - 1.0;
                    if (ent->r.ownerNum.isDefined())
                    {
                        v1 = G_TraceCapsuleComplete(ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, ent->r.ownerNum.entnum(), mask);
                    }
                    else
                    {
                        v1 = G_TraceCapsuleComplete(ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, 1023, mask);
                    }
                    if (v1)
                    {
                        corpseInfo->falling = 1;
                        ent->s.lerp.pos.trType = isRagdoll ? TR_RAGDOLL_GRAVITY : TR_GRAVITY;
                        ent->s.lerp.pos.trBase[0] = endpos[0];
                        ent->s.lerp.pos.trBase[1] = endpos[1];
                        ent->s.lerp.pos.trBase[2] = endpos[2];
                        ent->s.lerp.pos.trDelta[0] = 0.0;
                        ent->s.lerp.pos.trDelta[1] = 0.0;
                        ent->s.lerp.pos.trDelta[2] = 0.0;
                        Vec3Mad(ent->s.lerp.pos.trDelta, deltaChange[0], forward, ent->s.lerp.pos.trDelta);
                        Vec3Mad(ent->s.lerp.pos.trDelta, deltaChange[1], left, ent->s.lerp.pos.trDelta);
                        ent->s.lerp.pos.trTime = level.time;
                        ent->s.lerp.pos.trDuration = 0;
                    }
                }
            }
            else if (SV_PointContents(ent->r.currentOrigin, -1, 0x80000000))
            {
                G_FreeEntity(ent);
            }
            else if (corpseInfo->falling)
            {
                if (tr.allsolid)
                {
                    start[0] = ent->r.currentOrigin[0];
                    start[1] = ent->r.currentOrigin[1];
                    start[2] = ent->r.currentOrigin[2];
                    start[2] = start[2] + 32.0f;
                    if (ent->r.ownerNum.isDefined())
                    {
                        G_TraceCapsule(&tr, start, ent->r.mins, ent->r.maxs, origin, ent->r.ownerNum.entnum(), mask & 0xFFFEFFFF);
                    }
                    else
                    {
                        G_TraceCapsule(&tr, start, ent->r.mins, ent->r.maxs, origin, 1023, mask & 0xFFFEFFFF);
                    }
                    if (!tr.allsolid)
                    {
                        Vec3Lerp(start, ent->r.currentOrigin, tr.fraction, endpos);
                        ent->r.currentOrigin[0] = endpos[0];
                        ent->r.currentOrigin[1] = endpos[1];
                        ent->r.currentOrigin[2] = endpos[2];
                    }
                }
                G_BounceCorpse(ent, corpseInfo, &tr, endpos);
            }
        }
    }
}

void __cdecl G_BounceCorpse(gentity_s *ent, corpseInfo_t *corpseInfo, trace_t *trace, float *endpos)
{
    bool v4; // [esp+0h] [ebp-58h]
    float vAngles[3]; // [esp+24h] [ebp-34h] BYREF
    float vAxis[3][3]; // [esp+30h] [ebp-28h] BYREF
    bool isRagdoll; // [esp+57h] [ebp-1h]

    isRagdoll = Com_IsRagdollTrajectory(&ent->s.lerp.pos);
    ent->s.lerp.pos.trDelta[0] = 0.0;
    ent->s.lerp.pos.trDelta[1] = 0.0;
    ent->s.lerp.pos.trDelta[2] = 0.0;
    if (trace->allsolid || trace->normal[2] > 0.0)
    {
        corpseInfo->falling = 0;
        ent->s.lerp.pos.trType = isRagdoll ? TR_RAGDOLL_INTERPOLATE : TR_INTERPOLATE;
        ent->s.lerp.pos.trDelta[0] = 0.0;
        ent->s.lerp.pos.trDelta[1] = 0.0;
        ent->s.lerp.pos.trDelta[2] = 0.0;
        ent->s.lerp.pos.trBase[0] = *endpos;
        ent->s.lerp.pos.trBase[1] = endpos[1];
        ent->s.lerp.pos.trBase[2] = endpos[2];
        ent->s.lerp.pos.trTime = 0;
        ent->s.lerp.pos.trDuration = 0;
        ent->s.groundEntityNum = Trace_GetEntityHitId(trace);
        g_entities[ent->s.groundEntityNum].flags |= 0x100000u;
        if (trace->allsolid)
        {
            G_SetAngle(ent, ent->r.currentAngles);
        }
        else
        {
            vAxis[2][0] = trace->normal[0];
            vAxis[2][1] = trace->normal[1];
            vAxis[2][2] = trace->normal[2];
            AngleVectors(ent->r.currentAngles, vAxis[0], 0, 0);
            Vec3Cross(vAxis[2], vAxis[0], vAxis[1]);
            Vec3Cross(vAxis[1], vAxis[2], vAxis[0]);
            AxisToAngles(vAxis, vAngles);
            G_SetAngle(ent, vAngles);
        }
        SV_LinkEntity(ent);
    }
    else
    {
        Vec3Add(ent->r.currentOrigin, trace->normal, ent->r.currentOrigin);
        ent->s.lerp.pos.trBase[0] = ent->r.currentOrigin[0];
        ent->s.lerp.pos.trBase[1] = ent->r.currentOrigin[1];
        ent->s.lerp.pos.trBase[2] = ent->r.currentOrigin[2];
        if (ent->s.lerp.pos.trType != TR_GRAVITY && !isRagdoll)
            MyAssertHandler(
                ".\\game_mp\\g_player_corpse_mp.cpp",
                72,
                0,
                "%s",
                "ent->s.lerp.pos.trType == TR_GRAVITY || isRagdoll");
        ent->s.lerp.pos.trTime = level.time;
    }
}

char __cdecl G_GetAnimDeltaForCorpse(gentity_s *ent, float *originChange)
{
    DObj_s *obj; // [esp+18h] [ebp-10h]
    float rot[3]; // [esp+1Ch] [ebp-Ch] BYREF

    obj = Com_GetServerDObj(ent->s.number);
    if (!obj)
        return 0;
    XAnimCalcDelta(obj, 0, rot, originChange, 1);
    if (anim_deltas_debug->current.enabled && *originChange != 0.0)
        Com_Printf(19, "got anim delta for this frame of ( %f, %f, %f )\n", *originChange, originChange[1], originChange[2]);
    return 1;
}

void __cdecl G_RunCorpse(gentity_s *ent)
{
    G_RunCorpseMove(ent);
    G_RunCorpseAnimate(ent);
    G_RunThink(ent);
}

void __cdecl G_RunCorpseAnimate(gentity_s *ent)
{
    DObj_s *ServerDObj; // eax
    int corpseIndex; // [esp+4h] [ebp-8h]

    corpseIndex = G_GetPlayerCorpseIndex(ent);
    if (corpseIndex < 0)
        MyAssertHandler(".\\game_mp\\g_player_corpse_mp.cpp", 363, 0, "%s", "corpseIndex >= 0");
    if (corpseIndex >= 8)
        MyAssertHandler(".\\game_mp\\g_player_corpse_mp.cpp", 364, 0, "%s", "corpseIndex < MAX_CLIENT_CORPSES");
    ServerDObj = Com_GetServerDObj(ent->s.number);
    BG_UpdatePlayerDObj(-1, ServerDObj, &ent->s, &g_scr_data.playerCorpseInfo[corpseIndex].ci, 0);
    if (Com_GetServerDObj(ent->s.number))
        BG_PlayerAnimation(-1, &ent->s, &g_scr_data.playerCorpseInfo[corpseIndex].ci);
}

