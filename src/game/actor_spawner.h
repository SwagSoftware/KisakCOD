#pragma once
#include <server/sv_world.h>

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

enum enumForceSpawn : __int32
{
	CHECK_SPAWN = 0x0,
	FORCE_SPAWN = 0x1,
};

int __cdecl PointCouldSeeSpawn(const float *vEyePos, const float *vSpawnPos, int iIgnoreEnt1, int iIgnoreEnt2);
gentity_s *__cdecl SpawnActor(gentity_s *ent, unsigned int targetname, enumForceSpawn forceSpawn, int getEnemyInfo);
void __cdecl G_DropActorSpawnersToFloor();
int __cdecl SP_actor_spawner(gentity_s *pEnt);
