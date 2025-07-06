#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

void __fastcall TRACK_actor_threat();
void __fastcall Actor_InitThreatBiasGroups();
void __fastcall Actor_ClearThreatBiasGroups();
int __fastcall Actor_FindThreatBiasGroupIndex(unsigned int name);
void __fastcall Actor_CreateThreatBiasGroup(unsigned int name);
void __fastcall Actor_SetThreatBiasEntireGroup(int group, int threatBias);
void __fastcall Actor_SetThreatBias(int groupSelf, int groupEnemy, int threatBias);
void __fastcall Actor_SetIgnoreMeGroup(int groupSelf, int groupIgnoreMe);
int __fastcall Actor_GetThreatBias(int groupSelf, int groupEnemy);
void __fastcall Actor_FlagEnemyUnattackable(actor_s *self);
int __fastcall Actor_CaresAboutInfo(actor_s *self, sentient_s *pOther);
int __fastcall DebugThreatInfoDuration();
void __fastcall DebugResetThreatStrings(const actor_s *self);
void __fastcall DebugSetThreatString(ThreatDebugStringCategory category, int threat);
void __fastcall DebugSetThreatStringFromString(ThreatDebugStringCategory category, const char *string);
void __fastcall DebugThreatStringAll(const actor_s *self, sentient_s *enemy, int threat);
void __fastcall DebugThreatStringSimple(const actor_s *self, gentity_s *enemy, const char *string, const float *color);
void __fastcall DebugThreatNodes(
    const actor_s *self,
    sentient_s *enemy,
    pathnode_t *selfNode,
    pathnode_t *enemyNode,
    const float *color);
int __fastcall Actor_ThreatFromScariness(double fScariness);
// local variable allocation has failed, the output may be wrong!
int __fastcall Actor_ThreatFromDistance(double fDistance);
bool __fastcall Actor_IsFullyAware(actor_s *self, sentient_s *enemy, int isCurrentEnemy);
int __fastcall Actor_ThreatFromVisibilityAndAwareness(int isVisible, int isFullyAware, int friendlyTimingOut);
int __fastcall Actor_ThreatFromAttackerCount(actor_s *self, sentient_s *enemy, int isCurrentEnemy);
int __fastcall Actor_ThreatBonusForCurrentEnemy(
    int isCurrentEnemy,
    int isFullyAware,
    int friendlyTimingOut,
    int isPlayer,
    int isDamaged);
int __fastcall Actor_ThreatCoveringFire(actor_s *self, sentient_s *enemy);
int __fastcall Actor_ThreatFlashed(sentient_s *enemy);
int __fastcall Actor_UpdateSingleThreat(actor_s *self, sentient_s *enemy);
void __fastcall Actor_InitThreatUpdateInterval(actor_s *self);
void __fastcall Actor_IncrementThreatTime(actor_s *self);
void __fastcall Actor_CanAttackAll(actor_s *self);
// local variable allocation has failed, the output may be wrong!
void __fastcall Actor_SetPotentialThreat(potential_threat_t *self, double yaw);
void __fastcall Actor_ClearPotentialThreat(potential_threat_t *self);
bool __fastcall Actor_GetPotentialThreat(potential_threat_t *self, double *potentialThreatDir);
void __fastcall Actor_PotentialThreat_Debug(actor_s *self);
int __fastcall Actor_CheckIgnore(sentient_s *self, sentient_s *enemy);
void __fastcall Actor_UpdateThreat(actor_s *self);
