#include "game_public.h"
#include <qcommon/mem_track.h>
#include <server/sv_world.h>
#include <game_mp/g_utils_mp.h>
#include <server/sv_game.h>

struct pushed_t // sizeof=0x2C
{                                       // ...
    gentity_s *ent;
    float origin[3];
    float angles[3];
    float surfaceNormal[3];
    float deltayaw;
};

//Line 51761:  0006 : 0000559c       char const **hintStrings      827b559c     g_mover.obj

pushed_t pushed[1024];
pushed_t *pushed_p;

void __cdecl TRACK_g_mover()
{
    track_static_alloc_internal(pushed, 45056, "pushed", 9);
    track_static_alloc_internal(hintStrings, 20, "hintStrings", 9);
}

gentity_s *__cdecl G_TestEntityPosition(gentity_s *ent, float *vOrigin)
{
    int passEntityNum; // [esp+0h] [ebp-38h]
    trace_t tr; // [esp+4h] [ebp-34h] BYREF
    int mask; // [esp+30h] [ebp-8h]
    unsigned __int16 hitEntId; // [esp+34h] [ebp-4h]

    if (ent->clipmask)
    {
        if ((ent->r.contents & 0x4000000) != 0)
            return 0;
        mask = ent->clipmask;
    }
    else
    {
        mask = 2065;
    }
    if (ent->s.eType == 4)
    {
        if (EntHandle::isDefined(&ent->r.ownerNum))
        {
            passEntityNum = EntHandle::entnum(&ent->r.ownerNum);
            G_TraceCapsule(&tr, vOrigin, ent->r.mins, ent->r.maxs, vOrigin, passEntityNum, mask);
        }
        else
        {
            G_TraceCapsule(&tr, vOrigin, ent->r.mins, ent->r.maxs, vOrigin, 1023, mask);
        }
    }
    else
    {
        G_TraceCapsule(&tr, vOrigin, ent->r.mins, ent->r.maxs, vOrigin, ent->s.number, mask);
    }
    hitEntId = Trace_GetEntityHitId(&tr);
    if (tr.startsolid || tr.allsolid)
        return &g_entities[hitEntId];
    else
        return 0;
}

void __cdecl G_CreateRotationMatrix(const float *angles, float (*matrix)[3])
{
    AngleVectors(angles, (float *)matrix, &(*matrix)[3], &(*matrix)[6]);
    (*matrix)[3] = -(*matrix)[3];
    (*matrix)[4] = -(*matrix)[4];
    (*matrix)[5] = -(*matrix)[5];
}

void __cdecl G_TransposeMatrix(float (*matrix)[3], float (*transpose)[3])
{
    int j; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    for (i = 0; i < 3; ++i)
    {
        for (j = 0; j < 3; ++j)
            (*transpose)[3 * i + j] = (*matrix)[3 * j + i];
    }
}

int __cdecl G_TryPushingEntity(gentity_s *check, gentity_s *pusher, float *move, float *amove)
{
    float *origin; // [esp+0h] [ebp-ACh]
    float *v6; // [esp+Ch] [ebp-A0h]
    float matrix[3][3]; // [esp+1Ch] [ebp-90h] BYREF
    float transpose[3][3]; // [esp+40h] [ebp-6Ch] BYREF
    float fy; // [esp+64h] [ebp-48h]
    float z; // [esp+68h] [ebp-44h]
    float fz; // [esp+6Ch] [ebp-40h]
    float vOrigin[3]; // [esp+70h] [ebp-3Ch] BYREF
    float org2[3]; // [esp+7Ch] [ebp-30h] BYREF
    float move2[3]; // [esp+88h] [ebp-24h] BYREF
    float org[3]; // [esp+94h] [ebp-18h] BYREF
    float fx; // [esp+A0h] [ebp-Ch]
    float x; // [esp+A4h] [ebp-8h]
    float y; // [esp+A8h] [ebp-4h]

    Vec3Add(check->r.currentOrigin, move, vOrigin);
    G_CreateRotationMatrix(amove, transpose);
    G_TransposeMatrix(transpose, matrix);
    Vec3Sub(vOrigin, pusher->r.currentOrigin, org);
    org2[0] = org[0];
    org2[1] = org[1];
    org2[2] = org[2];
    G_RotatePoint(org2, matrix);
    Vec3Sub(org2, org, move2);
    Vec3Add(vOrigin, move2, vOrigin);
    if (G_TestEntityPosition(check, vOrigin))
    {
        if (check->r.maxs[0] / 2.0 <= 4.0)
            goto LABEL_42;
        org[0] = vOrigin[0];
        org[1] = vOrigin[1];
        org[2] = vOrigin[2];
        z = 0.0;
    LABEL_11:
        if (check->r.maxs[0] / 2.0 <= z)
        {
        LABEL_42:
            if (G_TestEntityPosition(check, check->r.currentOrigin))
            {
                return 0;
            }
            else
            {
                check->s.groundEntityNum = 1023;
                return 1;
            }
        }
        else
        {
            for (fz = -z; ; fz = z * 2.0 + fz)
            {
                if (z < (double)fz)
                {
                LABEL_36:
                    z = z + 4.0;
                    goto LABEL_11;
                }
                x = 4.0;
            LABEL_15:
                if (check->r.maxs[0] / 2.0 > x)
                    break;
                if (fz == 0.0)
                    goto LABEL_36;
            }
            for (fx = -x; ; fx = x * 2.0 + fx)
            {
                if (x < (double)fx)
                {
                    x = x + 4.0;
                    goto LABEL_15;
                }
                y = 4.0;
            LABEL_19:
                if (check->r.maxs[0] / 2.0 > y)
                    break;
            }
            for (fy = -y; ; fy = y * 2.0 + fy)
            {
                if (y < (double)fy)
                {
                    y = y + 4.0;
                    goto LABEL_19;
                }
                move2[0] = fx;
                move2[1] = fy;
                move2[2] = fz;
                Vec3Add(org, move2, org2);
                if (!G_TestEntityPosition(check, org2))
                    break;
            }
            if (check->s.groundEntityNum != pusher->s.number)
                check->s.groundEntityNum = 1023;
            check->r.currentOrigin[0] = org2[0];
            check->r.currentOrigin[1] = org2[1];
            check->r.currentOrigin[2] = org2[2];
            check->s.lerp.pos.trBase[0] = org2[0];
            check->s.lerp.pos.trBase[1] = org2[1];
            check->s.lerp.pos.trBase[2] = org2[2];
            if (check->client)
            {
                check->client->ps.delta_angles[1] = check->client->ps.delta_angles[1] + amove[1];
                origin = check->client->ps.origin;
                *origin = org2[0];
                origin[1] = org2[1];
                origin[2] = org2[2];
            }
            if (check->s.eType == 4)
                G_RotatePoint(&check->mover.aSpeed, matrix);
            ++pushed_p;
            return 1;
        }
    }
    else
    {
        if (check->s.groundEntityNum != pusher->s.number)
            check->s.groundEntityNum = 1023;
        check->r.currentOrigin[0] = vOrigin[0];
        check->r.currentOrigin[1] = vOrigin[1];
        check->r.currentOrigin[2] = vOrigin[2];
        check->s.lerp.pos.trBase[0] = vOrigin[0];
        check->s.lerp.pos.trBase[1] = vOrigin[1];
        check->s.lerp.pos.trBase[2] = vOrigin[2];
        if (check->client)
        {
            check->client->ps.delta_angles[1] = check->client->ps.delta_angles[1] + amove[1];
            v6 = check->client->ps.origin;
            *v6 = vOrigin[0];
            v6[1] = vOrigin[1];
            v6[2] = vOrigin[2];
        }
        if (check->s.eType == 4)
        {
            Vec3Add(check->r.currentAngles, amove, check->r.currentAngles);
            Vec3Add(check->s.lerp.apos.trBase, amove, check->s.lerp.apos.trBase);
            G_RotatePoint(&check->mover.aSpeed, matrix);
        }
        ++pushed_p;
        return 1;
    }
}

void __cdecl G_MoverTeam(gentity_s *ent)
{
    float *v2; // [esp+Ch] [ebp-60h]
    float *v3; // [esp+10h] [ebp-5Ch]
    float *trBase; // [esp+14h] [ebp-58h]
    float *v5; // [esp+18h] [ebp-54h]
    float *currentOrigin; // [esp+1Ch] [ebp-50h]
    float *v7; // [esp+20h] [ebp-4Ch]
    trajectory_t *p_pos; // [esp+24h] [ebp-48h]
    float move[3]; // [esp+28h] [ebp-44h] BYREF
    float origin[3]; // [esp+34h] [ebp-38h] BYREF
    float amove[3]; // [esp+40h] [ebp-2Ch] BYREF
    gentity_s *obstacle; // [esp+4Ch] [ebp-20h] BYREF
    float angles[3]; // [esp+50h] [ebp-1Ch] BYREF
    void(__cdecl * blocked)(gentity_s *, gentity_s *); // [esp+5Ch] [ebp-10h]
    void(__cdecl * reached)(gentity_s *); // [esp+60h] [ebp-Ch]
    gentity_s *check; // [esp+64h] [ebp-8h]
    pushed_t *p; // [esp+68h] [ebp-4h]

    if (ent->s.lerp.pos.trType != TR_PHYSICS)
    {
        p_pos = &ent->s.lerp.pos;
        if (ent == (gentity_s *)-12)
            MyAssertHandler("c:\\trees\\cod3\\src\\game_mp\\../universal/q_shared.h", 1957, 0, "%s", "trajectory");
        if (p_pos->trType < TR_FIRST_RAGDOLL || p_pos->trType > TR_RAGDOLL_INTERPOLATE)
        {
            obstacle = 0;
            pushed_p = pushed;
            BG_EvaluateTrajectory(&ent->s.lerp.pos, level.time, origin);
            BG_EvaluateTrajectory(&ent->s.lerp.apos, level.time, angles);
            Vec3Sub(origin, ent->r.currentOrigin, move);
            Vec3Sub(angles, ent->r.currentAngles, amove);
            if (G_MoverPush(ent, move, amove, &obstacle))
            {
                if (ent->s.lerp.pos.trType)
                {
                    if (level.time >= ent->s.lerp.pos.trDuration + ent->s.lerp.pos.trTime)
                    {
                        reached = entityHandlers[ent->handler].reached;
                        if (reached)
                            reached(ent);
                    }
                }
                if (ent->s.lerp.apos.trType)
                {
                    if (level.time >= ent->s.lerp.apos.trDuration + ent->s.lerp.apos.trTime)
                    {
                        reached = entityHandlers[ent->handler].reached;
                        if (reached)
                            reached(ent);
                    }
                }
            }
            else
            {
                for (p = pushed_p - 1; p >= pushed; --p)
                {
                    check = p->ent;
                    currentOrigin = check->r.currentOrigin;
                    v7 = p->origin;
                    check->r.currentOrigin[0] = p->origin[0];
                    currentOrigin[1] = v7[1];
                    currentOrigin[2] = v7[2];
                    trBase = check->s.lerp.pos.trBase;
                    v5 = p->origin;
                    check->s.lerp.pos.trBase[0] = p->origin[0];
                    trBase[1] = v5[1];
                    trBase[2] = v5[2];
                    if (check->client)
                    {
                        check->client->ps.delta_angles[1] = check->client->ps.delta_angles[1] - p->deltayaw;
                        v2 = check->client->ps.origin;
                        v3 = p->origin;
                        *v2 = p->origin[0];
                        v2[1] = v3[1];
                        v2[2] = v3[2];
                    }
                    if (check->s.eType == 4)
                        check->item[1] = *(item_ent_t *)p->surfaceNormal;
                    SV_LinkEntity(check);
                }
                ent->s.lerp.pos.trTime += level.time - level.previousTime;
                ent->s.lerp.apos.trTime += level.time - level.previousTime;
                BG_EvaluateTrajectory(&ent->s.lerp.pos, level.time, ent->r.currentOrigin);
                BG_EvaluateTrajectory(&ent->s.lerp.apos, level.time, ent->r.currentAngles);
                SV_LinkEntity(ent);
                blocked = entityHandlers[ent->handler].blocked;
                if (blocked)
                    blocked(ent, obstacle);
            }
        }
    }
}

char __cdecl G_MoverPush(gentity_s *pusher, float *move, float *amove, gentity_s **obstacle)
{
    float *origin; // [esp+8h] [ebp-208Ch]
    float *currentOrigin; // [esp+Ch] [ebp-2088h]
    float outMaxs[3]; // [esp+18h] [ebp-207Ch] BYREF
    float outMins[3]; // [esp+24h] [ebp-2070h] BYREF
    int v9; // [esp+30h] [ebp-2064h]
    int v10; // [esp+34h] [ebp-2060h]
    int j; // [esp+38h] [ebp-205Ch]
    float maxs[3]; // [esp+3Ch] [ebp-2058h] BYREF
    float v13[3]; // [esp+48h] [ebp-204Ch] BYREF
    float v14[3]; // [esp+54h] [ebp-2040h] BYREF
    float v15[3]; // [esp+60h] [ebp-2034h]
    float mins[3]; // [esp+6Ch] [ebp-2028h] BYREF
    float v17; // [esp+78h] [ebp-201Ch]
    char v18; // [esp+7Fh] [ebp-2015h]
    float v19[3]; // [esp+80h] [ebp-2014h]
    int entityList[1024]; // [esp+8Ch] [ebp-2008h] BYREF
    int i; // [esp+108Ch] [ebp-1008h]
    gentity_s *ent; // [esp+1090h] [ebp-1004h]
    unsigned int v23[1024]; // [esp+1094h] [ebp-1000h]

    *obstacle = 0;
    v18 = 1;
    mins[0] = pusher->r.mins[0];
    mins[1] = pusher->r.mins[1];
    mins[2] = pusher->r.mins[2];
    maxs[0] = pusher->r.maxs[0];
    maxs[1] = pusher->r.maxs[1];
    maxs[2] = pusher->r.maxs[2];
    if (pusher->s.eType == 6 && pusher->model && G_GetModelBounds(pusher->model, outMins, outMaxs))
    {
        for (i = 0; i < 3; ++i)
        {
            if (mins[i] > (double)outMins[i])
                mins[i] = outMins[i];
            if (maxs[i] < (double)outMaxs[i])
                maxs[i] = outMaxs[i];
        }
    }
    if (pusher->r.currentAngles[0] == 0.0
        && pusher->r.currentAngles[1] == 0.0
        && pusher->r.currentAngles[2] == 0.0
        && *amove == 0.0
        && amove[1] == 0.0
        && amove[2] == 0.0)
    {
        for (i = 0; i < 3; ++i)
        {
            v14[i] = pusher->r.currentOrigin[i] + mins[i] - 1.0;
            v13[i] = pusher->r.currentOrigin[i] + maxs[i] + 1.0;
        }
    }
    else
    {
        v17 = RadiusFromBounds(mins, maxs);
        for (i = 0; i < 3; ++i)
        {
            v14[i] = pusher->r.currentOrigin[i] - v17;
            v13[i] = pusher->r.currentOrigin[i] + v17;
        }
    }
    for (i = 0; i < 3; ++i)
    {
        v15[i] = v14[i] + move[i];
        v19[i] = v13[i] + move[i];
        if (move[i] <= 0.0)
            v14[i] = v14[i] + move[i];
        else
            v13[i] = v13[i] + move[i];
    }
    SV_UnlinkEntity(pusher);
    v9 = CM_AreaEntities(v14, v13, entityList, 1024, 0x6000180);
    Vec3Add(pusher->r.currentOrigin, move, pusher->r.currentOrigin);
    Vec3Add(pusher->r.currentAngles, amove, pusher->r.currentAngles);
    SV_LinkEntity(pusher);
    v10 = 0;
    for (j = 0; j < v9; ++j)
    {
        ent = &g_entities[entityList[j]];
        if ((ent->s.eType == 4 || ent->s.eType == 3 || ent->s.eType == 1 || ent->physicsObject)
            && (ent->s.groundEntityNum == pusher->s.number
                || v19[0] > (double)ent->r.absmin[0]
                && v19[1] > (double)ent->r.absmin[1]
                && v19[2] > (double)ent->r.absmin[2]
                && v15[0] < (double)ent->r.absmax[0]
                && v15[1] < (double)ent->r.absmax[1]
                && v15[2] < (double)ent->r.absmax[2]
                && G_TestEntityPosition(ent, ent->r.currentOrigin) == pusher))
        {
            v23[v10++] = entityList[j];
        }
    }
    for (j = 0; j < v10; ++j)
    {
        ent = &g_entities[v23[j]];
        SV_UnlinkEntity(ent);
    }
    for (j = 0; j < v10; ++j)
    {
        ent = &g_entities[v23[j]];
        if (pushed_p >= (pushed_t *)&pushed_p)
            MyAssertHandler(".\\game\\g_mover.cpp", 428, 0, "%s", "pushed_p < &pushed[MAX_GENTITIES]");
        pushed_p->ent = ent;
        origin = pushed_p->origin;
        currentOrigin = ent->r.currentOrigin;
        pushed_p->origin[0] = ent->r.currentOrigin[0];
        origin[1] = currentOrigin[1];
        origin[2] = currentOrigin[2];
        pushed_p->deltayaw = amove[1];
        if (ent->s.eType == 4)
            *(item_ent_t *)pushed_p->surfaceNormal = *(item_ent_t *)ent->missile.surfaceNormal;
        if (G_TryPushingEntity(ent, pusher, move, amove) || ent->s.eType == 3 || ent->s.eType == 4)
        {
            SV_LinkEntity(ent);
        }
        else
        {
            if (pusher->s.lerp.pos.trType != TR_SINE && pusher->s.lerp.apos.trType != TR_SINE)
            {
                *obstacle = ent;
                v18 = 0;
                break;
            }
            G_Damage(ent, pusher, pusher, 0, 0, 99999, 0, 9, 0xFFFFFFFF, HITLOC_NONE, 0, 0, 0);
        }
    }
    for (j = 0; j < v10; ++j)
    {
        ent = &g_entities[v23[j]];
        SV_LinkEntity(ent);
    }
    return v18;
}

void __cdecl G_RunMover(gentity_s *ent)
{
    if (ent->tagInfo)
    {
        G_GeneralLink(ent);
    }
    else if (ent->s.lerp.pos.trType || ent->s.lerp.apos.trType)
    {
        G_MoverTeam(ent);
    }
    G_RunThink(ent);
}

void __cdecl trigger_use(gentity_s *ent)
{
    trigger_use_shared(ent);
}

void __cdecl trigger_use_shared(gentity_s *self)
{
    char szConfigString[1028]; // [esp+34h] [ebp-410h] BYREF
    const char *cursorhint; // [esp+43Ch] [ebp-8h] BYREF
    unsigned int i; // [esp+440h] [ebp-4h]

    if (self->s.eType == 4)
        MyAssertHandler(".\\game\\g_mover.cpp", 749, 0, "%s", "self->s.eType != ET_MISSILE");
    if (SV_SetBrushModel(self))
    {
        self->r.contents = 0x200000;
        SV_LinkEntity(self);
        self->item[1].ammoCount = 1023;
        self->s.lerp.pos.trType = TR_STATIONARY;
        self->s.lerp.pos.trBase[0] = self->r.currentOrigin[0];
        self->s.lerp.pos.trBase[1] = self->r.currentOrigin[1];
        self->s.lerp.pos.trBase[2] = self->r.currentOrigin[2];
        self->r.svFlags = 1;
        self->s.lerp.eFlags |= 1u;
        if (!self->model)
            self->s.lerp.eFlags |= 0x20u;
        self->handler = 19;
        self->s.un2.hintString = 1;
        if (G_LevelSpawnString("cursorhint", "", &cursorhint))
        {
            if (I_stricmp(cursorhint, "HINT_INHERIT"))
            {
                for (i = 1; i < 5; ++i)
                {
                    if (!I_stricmp(cursorhint, hintStrings[i]))
                    {
                        self->s.un2.hintString = i;
                        break;
                    }
                }
            }
            else
            {
                self->s.un2.hintString = -1;
            }
        }
        self->s.un1.scale = 255;
        if (G_LevelSpawnString("hintstring", "", &cursorhint))
        {
            for (i = 0; i < 0x20; ++i)
            {
                SV_GetConfigstring(i + 277, szConfigString, 1024);
                if (!szConfigString[0])
                {
                    SV_SetConfigstring(i + 277, (char *)cursorhint);
                    self->s.un1.scale = (unsigned __int8)i;
                    break;
                }
                if (!strcmp(cursorhint, szConfigString))
                {
                    self->s.un1.scale = (unsigned __int8)i;
                    break;
                }
            }
            if (i == 32)
                Com_Error(ERR_DROP, "Too many different hintstring key values on trigger_use entities. Max allowed is %i different strings", 32);
        }
    }
    else
    {
        Com_PrintError(
            1,
            "Killing trigger_use_shared at (%f %f %f) because the brush model is invalid.\n",
            self->s.lerp.pos.trBase[0],
            self->s.lerp.pos.trBase[1],
            self->s.lerp.pos.trBase[2]);
        G_FreeEntity(self);
    }
}

void __cdecl G_RotatePoint(float *point, float (*matrix)[3])
{
    float tvec[3]; // [esp+0h] [ebp-Ch] BYREF

    tvec[0] = *point;
    tvec[1] = point[1];
    tvec[2] = point[2];
    *point = Vec3Dot((const float *)matrix, tvec);
    point[1] = Vec3Dot(&(*matrix)[3], tvec);
    point[2] = Vec3Dot(&(*matrix)[6], tvec);
}

