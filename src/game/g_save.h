#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif


// g_save
void __fastcall TRACK_g_save();
void __fastcall Scr_FreeFields(const saveField_t *fields, unsigned __int8 *base);
void __fastcall Scr_FreeEntityFields(gentity_s *ent);
void __fastcall Scr_FreeActorFields(actor_s *pActor);
void __fastcall Scr_FreeSentientFields(sentient_s *sentient);
// local variable allocation has failed, the output may be wrong!
void G_SaveError(
    errorParm_t code,
    SaveErrorType errorType,
    __int64 fmt,
    __int64 a4,
    __int64 a5,
    int a6,
    int a7,
    int a8,
    int a9,
    int a10,
    int a11,
    int a12,
    int a13,
    ...);
void __fastcall WriteCStyleString(const char *psz, int maxlen, SaveGame *save);
void __fastcall ReadCStyleString(char *psz, int maxlen, SaveGame *save);
void __fastcall WriteWeaponIndex(unsigned int weapon, SaveGame *save);
int __fastcall ReadWeaponIndex(SaveGame *save);
void __fastcall WriteItemIndex(int iIndex, SaveGame *save);
int __fastcall ReadItemIndex(SaveGame *save);
void __fastcall WriteVehicleIndex(__int16 index, SaveGame *save);
int __fastcall ReadVehicleIndex(SaveGame *save);
void __fastcall WriteField1(const saveField_t *field, const unsigned __int8 *base, unsigned __int8 *original);
void __fastcall WriteField2(const saveField_t *field, unsigned __int8 *base, SaveGame *save);
void __fastcall ReadField(const saveField_t *field, unsigned __int8 *base, SaveGame *save);
void __fastcall G_WriteStruct(
    const saveField_t *fields,
    unsigned __int8 *original,
    const unsigned __int8 *source,
    int sourcesize,
    SaveGame *save);
void __fastcall G_ReadStruct(const saveField_t *fields, unsigned __int8 *dest, int tempsize, SaveGame *save);
void __fastcall WriteClient(gclient_s *cl, SaveGame *save);
void __fastcall ReadClient(gclient_s *client, SaveGame *save);
void __fastcall WriteEntity(gentity_s *ent, SaveGame *save);
void __fastcall ReadEntity(gentity_s *ent, SaveGame *save);
void __fastcall WriteActorPotentialCoverNodes(actor_s *pActor, SaveGame *save);
void __fastcall ReadActorPotentialCoverNodes(actor_s *pActor, SaveGame *save);
void __fastcall WriteActor(actor_s *pActor, SaveGame *save);
void __fastcall ReadActor(actor_s *pActor, SaveGame *save);
void __fastcall WriteSentient(sentient_s *sentient, SaveGame *save);
void __fastcall ReadSentient(sentient_s *sentient, SaveGame *save);
void __fastcall WriteVehicle(scr_vehicle_s *pVehicle, SaveGame *save);
void __fastcall ReadVehicle(scr_vehicle_s *pVehicle, SaveGame *save);
void __fastcall WriteTurretInfo(TurretInfo *pTurretInfo, SaveGame *save);
void __fastcall ReadTurretInfo(TurretInfo *pTurretInfo, SaveGame *save);
void __fastcall WritePathNodes(SaveGame *save);
void __fastcall ReadPathNodes(SaveGame *save);
const saveField_t *__fastcall BadPlaceParmSaveFields(const badplace_t *badplace);
void __fastcall WriteBadPlaces(SaveGame *save);
void __fastcall ReadBadPlaces(SaveGame *save);
void __fastcall WriteThreatBiasGroups(SaveGame *save);
void __fastcall ReadThreatBiasGroups(SaveGame *save);
void __fastcall WriteAIEventListeners(SaveGame *save);
void __fastcall ReadAIEventListeners(SaveGame *save);
char *__fastcall G_Save_DateStr();
void __fastcall G_SaveConfigstrings(int iFirst, int iCount, SaveGame *save);
void __fastcall G_LoadConfigstrings(int iFirst, int iCount, SaveGame *save);
void __fastcall G_LoadModelPrecacheList(SaveGame *save);
void __fastcall G_ClearConfigstrings(int iFirst, int iCount);
void __fastcall G_ClearAllConfigstrings();
void __fastcall G_SaveInitConfigstrings(SaveGame *save);
void __fastcall G_LoadInitConfigstrings(SaveGame *save);
void __fastcall G_SaveItems(SaveGame *save);
void __fastcall G_SaveWeaponCue(SaveGame *save);
void __fastcall G_LoadWeaponCue(SaveGame *save);
void __fastcall G_SaveDvars(SaveGame *save);
void __fastcall G_LoadDvars(SaveGame *save);
void __fastcall G_CheckEntityDefaultModel(gentity_s *e);
void __fastcall G_UpdateAllEntities();
void G_CheckAllEntities();
void __fastcall G_SaveInitState(SaveGame *save);
void __fastcall G_SaveMainState(bool savegame, SaveGame *save);
void __fastcall G_SaveState(bool savegame, SaveGame *save);
int __fastcall G_IsSavePossible(SaveType saveType);
int __fastcall G_WriteGame(const PendingSave *pendingSave, int checksum, SaveGame *save);
void __fastcall G_WriteCurrentCommitToDevice();
void __fastcall G_PrepareSaveMemoryForWrite(char commitLevel);
int __fastcall G_ProcessCommitActions(const PendingSave *pendingSave, SaveGame *save);
int __fastcall G_SaveGame(const PendingSave *pendingSave, int checksum);
bool __fastcall G_CommitSavedGame(int saveId);
void __fastcall G_LoadItems(SaveGame *save);
void __fastcall G_SetPendingLoadName(const char *filename);
void __fastcall G_PreLoadGame(int checksum, int *useLoadedSourceFiles, SaveGame **save);
int __fastcall G_LoadWeapons(SaveGame *save);
void __fastcall G_InitLoadGame(SaveGame *save);
void __fastcall G_LoadMainState(SaveGame *save);
void __fastcall G_LoadGame(int checksum, SaveGame *save);
int __fastcall G_LoadErrorCleanup();
