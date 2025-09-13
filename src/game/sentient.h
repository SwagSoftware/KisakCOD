#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "enthandle.h"

#include <bgame/bg_local.h>
#include <bgame/bg_public.h>
#include <script/scr_variable.h>

struct pathnode_t;

struct sentient_s
{
    gentity_s *ent;
    team_t eTeam;
    int iThreatBias;
    int iThreatBiasGroupIndex;
    bool bIgnoreMe;
    bool bIgnoreAll;
    bool originChanged;
    float oldOrigin[3];
    float maxVisibleDist;
    int iEnemyNotifyTime;
    int attackerCount;
    gentity_s *lastAttacker;
    EntHandle syncedMeleeEnt;
    EntHandle targetEnt;
    EntHandle scriptTargetEnt;
    float entityTargetThreat;
    int meleeAttackerSpot[4];
    float attackerAccuracy;
    bool ignoreRandomBulletDamage;
    bool turretInvulnerability;
    pathnode_t *pClaimedNode;
    pathnode_t *pPrevClaimedNode;
    pathnode_t *pActualChainPos;
    int iActualChainPosTime;
    pathnode_t *pNearestNode;
    unsigned __int8 bNearestNodeValid;
    unsigned __int8 bNearestNodeBad;
    bool inuse;
    int banNodeTime;
};

struct sentient_sort_t
{
    sentient_s *sentient;
    float fMetric;
};

struct SentientGlobals
{
    int lastTime;
    int lastSample;
    float playerTrail[2][3];
    int sampleTime[2];
};

struct vis_cache_t
{
    bool bVisible;
    int iLastUpdateTime;
    int iLastVisTime;
};

struct sentient_info_t
{
    vis_cache_t VisCache;
    int iLastAttackMeTime;
    int lastKnownPosTime;
    int attackTime;
    float vLastKnownPos[3];
    pathnode_t *pLastKnownNode;
};

struct SaveGame;

sentient_s *__cdecl Sentient_Alloc();
void __cdecl Sentient_DissociateSentient(sentient_s *self, sentient_s *other, team_t eOtherTeam);
void __cdecl Sentient_GetOrigin(const sentient_s *self, float *vOriginOut);
void __cdecl Sentient_GetForwardDir(sentient_s *self, float *vDirOut);
void __cdecl Sentient_GetVelocity(const sentient_s *self, float *vVelOut);
void __cdecl Sentient_GetCentroid(sentient_s *self, float *vCentroidOut);
void __cdecl Sentient_GetEyePosition(const sentient_s *self, float *vEyePosOut);
void __cdecl Sentient_GetDebugEyePosition(const sentient_s *self, float *vEyePosOut);
float __cdecl Sentient_GetHeadHeight(const sentient_s *self);
void __cdecl Sentient_UpdateActualChainPos(sentient_s *self);
pathnode_t *__cdecl Sentient_NearestNode(sentient_s *self);
pathnode_t *__cdecl Sentient_NearestNodeSuppressed(
    sentient_s *self,
    float (*vNormal)[2],
    float *fDist,
    int iPlaneCount);
void __cdecl Sentient_InvalidateNearestNode(sentient_s *self);
void __cdecl Sentient_SetEnemy(sentient_s *self, gentity_s *enemy, int bNotify);
sentient_s *__cdecl Sentient_FirstSentient(int iTeamFlags);
sentient_s *__cdecl Sentient_NextSentient(sentient_s *pPrevSentient, int iTeamFlags);
const char *__cdecl Sentient_NameForTeam(unsigned int eTeam);
void __cdecl Sentient_SetTeam(sentient_s *self, team_t eTeam);
void __cdecl Sentient_ClaimNode(sentient_s *self, pathnode_t *node);
void __cdecl Sentient_NodeClaimRevoked(sentient_s *self, pathnode_t *node);
void __cdecl Sentient_StealClaimNode(sentient_s *self, sentient_s *other);
void __cdecl Sentient_BanNearNodes(sentient_s *self);
void __cdecl Sentient_UpdatePlayerTrail(gentity_s *ent);
void __cdecl Sentient_WriteGlob(SaveGame *save);
void __cdecl Sentient_ReadGlob(SaveGame *save);
void __cdecl Sentient_GetTrailPos(sentient_s *self, float *pos, int *time);
void __cdecl G_InitSentients();
int __cdecl Sentient_NearestNodeDirty(sentient_s *self, bool originChanged);
void __cdecl Sentient_Dissociate(sentient_s *pSentient);
void __cdecl Sentient_Free(sentient_s *sentient);


// sentient_script_cmd
sentient_s *__cdecl Sentient_Get(scr_entref_t entref);
void __cdecl SentientCmd_GetEnemySqDist(scr_entref_t entref);
void __cdecl SentientCmd_GetClosestEnemySqDist(scr_entref_t entref);
void SentientCmd_CreateThreatBiasGroup();
void SentientCmd_ThreatBiasGroupExists();
void SentientCmd_GetThreatBias();
void SentientCmd_SetThreatBias();
void SentientCmd_SetThreatBiasAgainstAll();
void SentientCmd_SetIgnoreMeGroup();
void __cdecl SentientCmd_SetThreatBiasGroup(scr_entref_t entref);
void __cdecl SentientCmd_GetThreatBiasGroup(scr_entref_t entref);
void(__cdecl *__cdecl Sentient_GetMethod(const char **pName))(scr_entref_t);
void(__cdecl *__cdecl Sentient_GetFunction(const char **pName))();
