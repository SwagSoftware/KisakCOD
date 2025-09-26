#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor_spawner.h"
#include "g_main.h"
#include <universal/com_math.h>
#include "actor.h"
#include "g_local.h"
#include <script/scr_const.h>
#include "actor_events.h"
#include "actor_senses.h"
#include "actor_threat.h"
#include <cgame/cg_local.h>

const float g_vSpawnCheckPoints[11][3] =
{
  { 0.5f, 0.5f, 0.8f },
  { 0.5f, 0.5f, 0.5f },
  { 0.5f, 0.5f, 0.2f },
  { 0.0f, 0.0f, 1.0f },
  { 0.0f, 1.0f, 1.0f },
  { 1.0f, 1.0f, 1.0f },
  { 1.0f, 0.0f, 1.0f },
  { 1.0f, 0.0f, 0.0f },
  { 1.0f, 1.0f, 0.0f },
  { 0.0f, 1.0f, 0.0f },
  { 0.0f, 0.0f, 0.0f }
};


int __cdecl PointCouldSeeSpawn(const float *vEyePos, const float *vSpawnPos, int iIgnoreEnt1, int iIgnoreEnt2)
{
    double v8; // fp13
    double v9; // fp12
    unsigned int v10; // r23
    float *v11; // r30
    double v12; // fp11
    double v13; // fp0
    int v15; // [sp+50h] [-70h] BYREF
    float v16[26]; // [sp+58h] [-68h] BYREF

    v8 = (float)(vEyePos[2] - vSpawnPos[2]);
    v9 = (float)(vEyePos[1] - vSpawnPos[1]);
    if ((float)((float)((float)((float)v9 * (float)v9)
        + (float)((float)((float)(*vEyePos - *vSpawnPos) * (float)(*vEyePos - *vSpawnPos))
            + (float)((float)v8 * (float)v8)))
        * (float)0.68558401) > (double)level.fFogOpaqueDistSqrd)
        return 0;
    v10 = 0;
    v11 = (float *)&g_vSpawnCheckPoints[0][1];
    v15 = 0;
    while (1)
    {
        v12 = (float)((float)((float)(72.0 - 0.0) * v11[1]) + vSpawnPos[2]);
        v13 = (float)((float)((float)((float)(15.0 - -15.0) * *v11) + vSpawnPos[1]) + -15.0);
        v16[0] = (float)((float)((float)(15.0 - -15.0) * *(v11 - 1)) + *vSpawnPos) + -15.0;
        v16[1] = v13;
        v16[2] = (float)v12 + (float)0.0;
        SV_SightTrace(&v15, vEyePos, vec3_origin, vec3_origin, v16, iIgnoreEnt1, iIgnoreEnt2, 6145);
        if (!v15)
            break;
        v10 += 12;
        v11 += 3;
        if (v10 >= 0x84)
            return 0;
    }
    return 1;
}

gentity_s *__cdecl SpawnActor(gentity_s *ent, unsigned int targetname, enumForceSpawn forceSpawn, int getEnemyInfo)
{
    unsigned int v8; // r3
    const char *v9; // r5
    const char *v10; // r5
    gentity_s *v11; // r29
    const char *v12; // r5
    gentity_s *spawn; // r28
    const char *v14; // r5
    sentient_s *i; // r31
    actor_s *actor; // r29
    int v17; // r27
    actor_s *j; // r30
    unsigned int k; // r31
    int count; // r11
    float v21[24]; // [sp+50h] [-60h] BYREF

    if (ai_disableSpawn->current.enabled)
    {
        Com_DPrintf(18, "Attempted spawn prevented by ai_disableSpawn.\n");
        return 0;
    }
    if (!ent->count)
    {
        v8 = ent->targetname;
        if (ent->targetname)
            v9 = SL_ConvertToString(v8);
        else
            v9 = "<unnamed>";
        Com_DPrintf(18, "^3Warning: SpawnActor( %s ) failed due to 0 count.\n", v9);
        return 0;
    }
    if (forceSpawn)
        goto LABEL_24;
    if (SpotWouldTelefrag(ent))
    {
        if (ent->targetname)
            v10 = SL_ConvertToString(ent->targetname);
        else
            v10 = "<unnamed>";
        Com_DPrintf(18, "^3couldn't spawn from %s because spawnpoint would telefrag\n", v10);
        return 0;
    }
    if (level.loading)
        goto LABEL_24;
    v11 = G_Find(0, 284, scr_const.player);
    if (!v11)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_spawner.cpp", 106, 0, "%s", "player");
    if (!v11->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_spawner.cpp", 107, 0, "%s", "player->sentient");
    Sentient_GetEyePosition(v11->sentient, v21);
    if (!PointCouldSeeSpawn(v21, ent->r.currentOrigin, v11->s.number, ent->s.number))
    {
    LABEL_24:
        spawn = G_Spawn();
        G_DuplicateEntityFields(spawn, ent);
        G_DuplicateScriptFields(spawn, ent);
        Scr_SetString(&spawn->targetname, targetname);
        spawn->spawnflags &= ~1u;
        if (SP_actor(spawn))
        {
            if (!spawn->actor)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_spawner.cpp", 132, 0, "%s", "spawn->actor");
            if ((ent->spawnflags & 8) != 0)
            {
                for (i = Sentient_FirstSentient(-1); i; i = Sentient_NextSentient(i, -1))
                    Actor_GetPerfectInfo(spawn->actor, i);
            }
            Actor_FinishSpawning(spawn->actor);
            actor = spawn->actor;
            if (getEnemyInfo)
            {
                v17 = 1 << spawn->sentient->eTeam;
                for (j = Actor_FirstActor(v17); j; j = Actor_NextActor(j, v17))
                {
                    if (actor != j)
                    {
                        for (k = 0; k < 0x21; ++k)
                            SentientInfo_Copy(actor, j, k);
                    }
                }
            }
            Actor_UpdateSight(actor);
            Actor_UpdateThreat(actor);
            Actor_InitAnimScript(spawn->actor);
            Scr_AddEntity(spawn);
            Scr_Notify(ent, scr_const.spawned, 1u);
            count = ent->count;
            if (count > 0)
                ent->count = count - 1;
            return spawn;
        }
        else
        {
            if (ent->targetname)
                v14 = SL_ConvertToString(ent->targetname);
            else
                v14 = "<unnamed>";
            Com_DPrintf(18, "^3couldn't spawn from %s because there are no free actors\n", v14);
            return 0;
        }
    }
    else
    {
        if (ent->targetname)
            v12 = SL_ConvertToString(ent->targetname);
        else
            v12 = "<unnamed>";
        Com_DPrintf(18, "^3couldn't spawn from %s because player can see spawnpoint\n", v12);
        return 0;
    }
}

void __cdecl G_DropActorSpawnersToFloor()
{
    int entContents[MAX_GENTITIES + 1];

    for (int i = 0; i < level.num_entities; ++i)
    {
        gentity_s *ent = &level.gentities[i];
        if (ent->r.inuse)
        {
            entContents[i] = ent->r.contents;
            if (Path_IsDynamicBlockingEntity(ent))
                ent->r.contents = 0;
        }
    }

    for (int i = 0; i < level.num_entities; ++i)
    {
        gentity_s *ent = &level.gentities[i];
        if (ent->r.inuse)
        {
            if (ent->s.eType == ET_ACTOR_SPAWNER)
            {
                if (Actor_droptofloor(ent))
                {
                    Com_Printf(18, "^3Spawner at (%g %g %g) is in solid\n", ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2]);
                    ent->r.svFlags &= ~1u;
                }
            }
        }
    }

    for (int i = 0; i < level.num_entities; ++i)
    {
        gentity_s *ent = &level.gentities[i];
        if (ent->r.inuse)
            ent->r.contents = entContents[i];
    }
}

int __cdecl SP_actor_spawner(gentity_s *pEnt)
{
    int count; // r10
    int v3; // r31
    const char **i; // r30

    if (!level.spawnVar.spawnVarsValid)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_spawner.cpp",
            244,
            0,
            "%s",
            "level.spawnVar.spawnVarsValid");
    count = pEnt->count;
    pEnt->clipmask = 0;
    pEnt->r.contents = 0;
    pEnt->r.svFlags = 1;
    pEnt->s.eType = ET_ACTOR_SPAWNER;
    pEnt->item[0].clipAmmoCount = -1;
    pEnt->item[0].ammoCount = 0;
    if (!count)
    {
        pEnt->count = 1;
        v3 = 0;
        if (level.spawnVar.numSpawnVars > 0)
        {
            for (i = (const char **)level.spawnVar.spawnVars[0]; I_stricmp(*i, "count"); i += 2)
            {
                if (++v3 >= level.spawnVar.numSpawnVars)
                    return 1;
            }
            pEnt->count = 0;
        }
    }
    return 1;
}

