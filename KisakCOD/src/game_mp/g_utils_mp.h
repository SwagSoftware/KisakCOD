#pragma once
#include <qcommon/qcommon.h>

void __cdecl G_SafeDObjFree(unsigned int handle, int unusedLocalClientNum);
void __cdecl TRACK_g_utils();
int __cdecl G_FindConfigstringIndex(char *name, int start, int max, int create, const char *errormsg);
int __cdecl G_LocalizedStringIndex(char *string);
int __cdecl G_MaterialIndex(const char *name);
int __cdecl G_ModelIndex(char *name);
bool __cdecl G_GetModelBounds(int index, float *outMins, float *outMaxs);
XModel *__cdecl G_GetModel(int index);
bool __cdecl G_XModelBad(int index);
unsigned int __cdecl G_ModelName(unsigned int index);
int __cdecl G_TagIndex(char *name);
int __cdecl G_EffectIndex(char *name);
int __cdecl G_ShellShockIndex(char *name);
int __cdecl G_SoundAliasIndex(char *name);
void __cdecl G_DObjUpdate(gentity_s *ent);
void __cdecl G_SetModel(gentity_s *ent, char *modelName);
void __cdecl G_OverrideModel(unsigned int modelIndex, char *defaultModelName);
int __cdecl G_EntAttach(gentity_s *ent, char *modelName, unsigned int tagName, int ignoreCollision);
int __cdecl G_EntDetach(gentity_s *ent, const char *modelName, unsigned int tagName);
void __cdecl G_EntDetachAll(gentity_s *ent);
int __cdecl G_EntLinkTo(gentity_s *ent, gentity_s *parent, unsigned int tagName);
int __cdecl G_EntLinkToInternal(gentity_s *ent, gentity_s *parent, unsigned int tagName);
int __cdecl G_EntLinkToWithOffset(
    gentity_s *ent,
    gentity_s *parent,
    unsigned int tagName,
    const float *originOffset,
    const float *anglesOffset);
void __cdecl G_EntUnlink(gentity_s *ent);
void __cdecl G_UpdateTagInfo(gentity_s *ent, int bParentHasDObj);
void __cdecl G_UpdateTagInfoOfChildren(gentity_s *parent, int bHasDObj);
void __cdecl G_CalcTagParentAxis(gentity_s *ent, float (*parentAxis)[3]);
void __cdecl G_CalcTagAxis(gentity_s *ent, int bAnglesOnly);
void __cdecl G_SetFixedLink(gentity_s *ent, int eAngles);
void __cdecl G_GeneralLink(gentity_s *ent);
void __cdecl G_SafeDObjFree(gentity_s *ent);
int __cdecl G_DObjUpdateServerTime(gentity_s *ent, int bNotify);
void __cdecl G_DObjCalcPose(gentity_s *ent, int *partBits);
void __cdecl G_DObjCalcBone(const gentity_s *ent, int boneIndex);
DObjAnimMat *__cdecl G_DObjGetLocalTagMatrix(gentity_s *ent, unsigned int tagName);
int __cdecl G_DObjGetWorldTagMatrix(gentity_s *ent, unsigned int tagName, float (*tagMat)[3]);
int __cdecl G_DObjGetWorldTagPos(gentity_s *ent, unsigned int tagName, float *pos);
DObjAnimMat *__cdecl G_DObjGetLocalBoneIndexMatrix(gentity_s *ent, int boneIndex);
void __cdecl G_DObjGetWorldBoneIndexPos(gentity_s *ent, int boneIndex, float *pos);
void __cdecl G_DObjGetWorldBoneIndexMatrix(gentity_s *ent, int boneIndex, float (*tagMat)[3]);
gentity_s *__cdecl G_Find(gentity_s *from, int fieldofs, unsigned __int16 match);
void __cdecl G_InitGentity(gentity_s *e);
void __cdecl G_PrintEntities();
gentity_s *__cdecl G_Spawn();
bool __cdecl G_MaySpawnEntity(gentity_s *e);
gentity_s *__cdecl G_SpawnPlayerClone();
void __cdecl G_FreeEntityRefs(gentity_s *ed);
void __cdecl G_FreeEntity(gentity_s *ed);
void __cdecl G_FreeEntityDelay(gentity_s *ed);
void __cdecl G_BroadcastEntity(gentity_s *ent);
void __cdecl G_FreeEntityAfterEvent(gentity_s *ent);
gentity_s *__cdecl G_TempEntity(const float *origin, int event);
void __cdecl G_AddPredictableEvent(gentity_s *ent, unsigned int event, unsigned int eventParm);
void __cdecl G_AddEvent(gentity_s *ent, unsigned int event, unsigned int eventParm);
void __cdecl G_PlaySoundAlias(gentity_s *ent, unsigned __int8 index);
int __cdecl G_AnimScriptSound(int client, snd_alias_list_t *aliasList);
void __cdecl G_SetOrigin(gentity_s *ent, const float *origin);
void __cdecl G_SetAngle(gentity_s *ent, const float *angle);
void __cdecl G_SetConstString(unsigned __int16 *to, char *from);
const char *__cdecl G_GetEntityTypeName(const gentity_s *ent);
int __cdecl G_rand();
double __cdecl G_flrand(float min, float max);
int __cdecl G_irand(int min, int max);
double __cdecl G_random();
double __cdecl G_crandom();
