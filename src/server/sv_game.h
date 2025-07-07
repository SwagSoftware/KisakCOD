#pragma once

#ifdef KISAK_MP
#include <cgame_mp/cg_local_mp.h>
#include <server_mp/server_mp.h>
#elif KISAK_SP
#include <cgame/cg_local.h>
#include <server/server.h>
#endif


void __cdecl TRACK_sv_game();
gentity_s *__cdecl SV_GentityNum(int num);
playerState_s *__cdecl SV_GameClientNum(int num);
svEntity_s *__cdecl SV_SvEntityForGentity(const gentity_s *gEnt);
gentity_s *__cdecl SV_GEntityForSvEntity(svEntity_s *svEnt);
void __cdecl SV_GameSendServerCommand(int clientNum, svscmd_type type, const char *text);
void __cdecl SV_GameDropClient(int clientNum, const char *reason);
void __cdecl SV_SetMapCenter(float *mapCenter);
void __cdecl SV_SetGameEndTime(int gameEndTime);
char __cdecl SV_SetBrushModel(gentity_s *ent);
bool __cdecl SV_inSnapshot(const float *origin, int iEntityNum);
bool __cdecl SV_EntityContact(const float *mins, const float *maxs, const gentity_s *gEnt);
void __cdecl SV_GetServerinfo(char *buffer, int bufferSize);
void __cdecl SV_LocateGameData(
    gentity_s *gEnts,
    int numGEntities,
    int sizeofGEntity_t,
    playerState_s *clients,
    int sizeofGameClient);
void __cdecl SV_GetUsercmd(int clientNum, usercmd_s *cmd);
XModel *__cdecl SV_XModelGet(char *name);
unsigned __int8 *__cdecl SV_AllocXModelPrecache(unsigned int size);
unsigned __int8 *__cdecl SV_AllocXModelPrecacheColl(unsigned int size);
void __cdecl SV_DObjDumpInfo(gentity_s *ent);
void __cdecl SV_ResetSkeletonCache();
bool __cdecl SV_DObjCreateSkelForBone(DObj_s *obj, int boneIndex);
char *__cdecl SV_AllocSkelMemory(unsigned int size);
int __cdecl SV_DObjCreateSkelForBones(DObj_s *obj, int *partBits);
int __cdecl SV_DObjUpdateServerTime(gentity_s *ent, float dtime, int bNotify);
void __cdecl SV_DObjInitServerTime(gentity_s *ent, float dtime);
int __cdecl SV_DObjGetBoneIndex(const gentity_s *ent, unsigned int boneName);
DObjAnimMat *__cdecl SV_DObjGetMatrixArray(const gentity_s *ent);
void __cdecl SV_DObjDisplayAnim(gentity_s *ent, const char *header);
void __cdecl SV_DObjGetBounds(gentity_s *ent, float *mins, float *maxs);
XAnimTree_s *__cdecl SV_DObjGetTree(gentity_s *ent);
void __cdecl SV_XModelDebugBoxes(gentity_s *ent);
bool __cdecl SV_MapExists(char *name);
bool __cdecl SV_DObjExists(gentity_s *ent);
void __cdecl SV_track_shutdown();
char *__cdecl SV_GetGuid(int clientNum);
int __cdecl SV_GetClientPing(int clientNum);
bool __cdecl SV_IsLocalClient(int clientNum);
void __cdecl SV_ShutdownGameProgs();
void __cdecl SV_SetGametype();
void __cdecl SV_RestartGameProgs(int savepersist);
void __cdecl SV_InitGameVM(int restart, int savepersist);
void __cdecl SV_InitGameProgs(int savepersist);
int __cdecl SV_GameCommand();

extern int gameInitialized;