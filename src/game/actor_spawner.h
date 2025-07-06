#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

int __fastcall PointCouldSeeSpawn(const float *vEyePos, const float *vSpawnPos, int iIgnoreEnt1, int iIgnoreEnt2);
gentity_s *__fastcall SpawnActor(gentity_s *ent, unsigned int targetname, enumForceSpawn forceSpawn, int getEnemyInfo);
void __fastcall G_DropActorSpawnersToFloor();
int __fastcall SP_actor_spawner(gentity_s *pEnt);
