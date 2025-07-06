#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

int __fastcall Actor_Dog_Exposed_Start(actor_s *self, ai_state_t ePrevState);
void __fastcall Actor_Dog_Exposed_Finish(actor_s *self, ai_state_t eNextState);
void __fastcall Actor_Dog_Exposed_Suspend(actor_s *self, ai_state_t eNextState);
int __fastcall Actor_Dog_IsInSyncedMelee(actor_s *self, sentient_s *enemy);
void __fastcall Actor_Dog_Attack(actor_s *self);
void __fastcall Actor_FindPathToGoalNearestNode(actor_s *self, int a2, int a3, int *a4);
int __fastcall Actor_SetMeleeAttackSpot(actor_s *self, const float *enemyPosition, float *attackPosition);
void __fastcall Actor_UpdateMeleeGoalPos(actor_s *self, float *goalPos);
int __fastcall Actor_Dog_IsAttackScriptRunning(actor_s *self);
float __fastcall Actor_Dog_GetEnemyPos(actor_s *self, sentient_s *enemy, float *enemyPos);
bool __fastcall Actor_Dog_IsEnemyInAttackRange(actor_s *self, sentient_s *enemy, int *goalPosSet);
actor_think_result_t __fastcall Actor_Dog_Exposed_Think(actor_s *self);
