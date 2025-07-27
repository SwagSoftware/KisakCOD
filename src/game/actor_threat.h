#pragma once
#include "actor.h"

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

enum ThreatDebugStringCategory : __int32
{
    TDS_TOTAL_THREAT = 0x0,
    TDS_FLASHED = 0x1,
    TDS_SUPPRESSED = 0x2,
    TDS_THREATBIAS = 0x3,
    TDS_THREATBIAS_GROUP = 0x4,
    TDS_ATTACKER_COUNT = 0x5,
    TDS_CUR_ENEMY_BONUS = 0x6,
    TDS_AWARENESS = 0x7,
    TDS_DIST_THREAT = 0x8,
    TDS_SCARINESS_THREAT = 0x9,
    NUM_TDS = 0xA,
};

struct threat_bias_t
{
    unsigned __int16 groupName[16];
    int threatTable[16][16];
    int threatGroupCount;
};

void __cdecl TRACK_actor_threat();
void __cdecl Actor_InitThreatBiasGroups();
void __cdecl Actor_ClearThreatBiasGroups();
int __cdecl Actor_FindThreatBiasGroupIndex(unsigned int name);
void __cdecl Actor_CreateThreatBiasGroup(unsigned int name);
void __cdecl Actor_SetThreatBiasEntireGroup(int group, int threatBias);
void __cdecl Actor_SetThreatBias(int groupSelf, int groupEnemy, int threatBias);
void __cdecl Actor_SetIgnoreMeGroup(int groupSelf, int groupIgnoreMe);
int __cdecl Actor_GetThreatBias(int groupSelf, int groupEnemy);
void __cdecl Actor_FlagEnemyUnattackable(actor_s *self);
int __cdecl Actor_CaresAboutInfo(actor_s *self, sentient_s *pOther);
int __cdecl DebugThreatInfoDuration();
void __cdecl DebugResetThreatStrings(const actor_s *self);
void __cdecl DebugSetThreatString(ThreatDebugStringCategory category, int threat);
void __cdecl DebugSetThreatStringFromString(ThreatDebugStringCategory category, const char *string);
void __cdecl DebugThreatStringAll(const actor_s *self, sentient_s *enemy, int threat);
void __cdecl DebugThreatStringSimple(const actor_s *self, gentity_s *enemy, const char *string, const float *color);
void __cdecl DebugThreatNodes(
    const actor_s *self,
    sentient_s *enemy,
    pathnode_t *selfNode,
    pathnode_t *enemyNode,
    const float *color);
int __cdecl Actor_ThreatFromScariness(double fScariness);
// local variable allocation has failed, the output may be wrong!
int __cdecl Actor_ThreatFromDistance(double fDistance);
bool __cdecl Actor_IsFullyAware(actor_s *self, sentient_s *enemy, int isCurrentEnemy);
int __cdecl Actor_ThreatFromVisibilityAndAwareness(int isVisible, int isFullyAware, int friendlyTimingOut);
int __cdecl Actor_ThreatFromAttackerCount(actor_s *self, sentient_s *enemy, int isCurrentEnemy);
int __cdecl Actor_ThreatBonusForCurrentEnemy(int isCurrentEnemy, int isFullyAware, int friendlyTimingOut, int isPlayer, int isDamaged);
int __cdecl Actor_ThreatCoveringFire(actor_s *self, sentient_s *enemy);
int __cdecl Actor_ThreatFlashed(sentient_s *enemy);
int __cdecl Actor_UpdateSingleThreat(actor_s *self, sentient_s *enemy);
void __cdecl Actor_InitThreatUpdateInterval(actor_s *self);
void Actor_IncrementThreatTime(actor_s *self);
void __cdecl Actor_CanAttackAll(actor_s *self);
// local variable allocation has failed, the output may be wrong!
void __cdecl Actor_SetPotentialThreat(potential_threat_t *self, double yaw);
void __cdecl Actor_ClearPotentialThreat(potential_threat_t *self);
bool __cdecl Actor_GetPotentialThreat(potential_threat_t *self, float *potentialThreatDir);
void __cdecl Actor_PotentialThreat_Debug(actor_s *self);
int __cdecl Actor_CheckIgnore(sentient_s *self, sentient_s *enemy);
void __cdecl Actor_UpdateThreat(actor_s *self);


extern threat_bias_t g_threatBias;