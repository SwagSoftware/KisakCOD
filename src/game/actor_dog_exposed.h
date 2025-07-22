#pragma once
#include "actor.h"

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

bool __cdecl Actor_Dog_Exposed_Start(actor_s *self, ai_state_t ePrevState);
void __cdecl Actor_Dog_Exposed_Finish(actor_s *self, ai_state_t eNextState);
void __cdecl Actor_Dog_Exposed_Suspend(actor_s *self, ai_state_t eNextState);
int __cdecl Actor_Dog_IsInSyncedMelee(actor_s *self, sentient_s *enemy);
void __cdecl Actor_Dog_Attack(actor_s *self);
void __cdecl Actor_FindPathToGoalNearestNode(actor_s *self);
int __cdecl Actor_SetMeleeAttackSpot(actor_s *self, const float *enemyPosition, float *attackPosition);
void __cdecl Actor_UpdateMeleeGoalPos(actor_s *self, float *goalPos);
int __cdecl Actor_Dog_IsAttackScriptRunning(actor_s *self);
float __cdecl Actor_Dog_GetEnemyPos(actor_s *self, sentient_s *enemy, float *enemyPos);
bool __cdecl Actor_Dog_IsEnemyInAttackRange(actor_s *self, sentient_s *enemy, int *goalPosSet);
actor_think_result_t __cdecl Actor_Dog_Exposed_Think(actor_s *self);
