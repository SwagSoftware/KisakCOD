#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

sentient_s *__fastcall Sentient_Alloc();
void __fastcall Sentient_DissociateSentient(sentient_s *self, sentient_s *other, team_t eOtherTeam);
void __fastcall Sentient_GetOrigin(const sentient_s *self, float *vOriginOut);
void __fastcall Sentient_GetForwardDir(sentient_s *self, float *vDirOut);
void __fastcall Sentient_GetVelocity(const sentient_s *self, float *vVelOut);
void __fastcall Sentient_GetCentroid(sentient_s *self, float *vCentroidOut);
void __fastcall Sentient_GetEyePosition(const sentient_s *self, float *vEyePosOut);
void __fastcall Sentient_GetDebugEyePosition(const sentient_s *self, float *vEyePosOut);
float __fastcall Sentient_GetHeadHeight(const sentient_s *self);
void __fastcall Sentient_UpdateActualChainPos(sentient_s *self);
pathnode_t *__fastcall Sentient_NearestNode(sentient_s *self);
pathnode_t *__fastcall Sentient_NearestNodeSuppressed(
    sentient_s *self,
    float (*vNormal)[2],
    float *fDist,
    int *iPlaneCount);
void __fastcall Sentient_InvalidateNearestNode(sentient_s *self);
void __fastcall Sentient_SetEnemy(sentient_s *self, gentity_s *enemy, int bNotify);
sentient_s *__fastcall Sentient_FirstSentient(int iTeamFlags);
sentient_s *__fastcall Sentient_NextSentient(sentient_s *pPrevSentient, int iTeamFlags);
const char *__fastcall Sentient_NameForTeam(unsigned int eTeam);
void __fastcall Sentient_SetTeam(sentient_s *self, team_t eTeam);
void __fastcall Sentient_ClaimNode(sentient_s *self, pathnode_t *node);
void __fastcall Sentient_NodeClaimRevoked(sentient_s *self, pathnode_t *node);
void __fastcall Sentient_StealClaimNode(sentient_s *self, sentient_s *other);
void __fastcall Sentient_BanNearNodes(sentient_s *self);
void __fastcall Sentient_UpdatePlayerTrail(gentity_s *ent);
void __fastcall Sentient_WriteGlob(SaveGame *save);
void __fastcall Sentient_ReadGlob(SaveGame *save);
void __fastcall Sentient_GetTrailPos(sentient_s *self, float *pos, int *time);
void __fastcall G_InitSentients();
int __fastcall Sentient_NearestNodeDirty(sentient_s *self, bool originChanged);
void __fastcall Sentient_Dissociate(sentient_s *pSentient);
void __fastcall Sentient_Free(sentient_s *sentient);


// sentient_script_cmd
sentient_s *__fastcall Sentient_Get(scr_entref_t *entref);
void __fastcall SentientCmd_GetEnemySqDist(scr_entref_t *entref);
void __fastcall SentientCmd_GetClosestEnemySqDist(scr_entref_t *entref);
void SentientCmd_CreateThreatBiasGroup();
void SentientCmd_ThreatBiasGroupExists();
void SentientCmd_GetThreatBias();
void SentientCmd_SetThreatBias();
void SentientCmd_SetThreatBiasAgainstAll();
void SentientCmd_SetIgnoreMeGroup();
void __fastcall SentientCmd_SetThreatBiasGroup(scr_entref_t *entref);
void __fastcall SentientCmd_GetThreatBiasGroup(scr_entref_t *entref);
void(__fastcall *__fastcall Sentient_GetMethod(const char **pName))(scr_entref_t);
void(__fastcall *__fastcall Sentient_GetFunction(const char **pName))();
