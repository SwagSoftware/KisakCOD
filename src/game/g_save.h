#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include <universal/memfile.h>
#include <client/client.h>
#include <server/server.h>

enum saveFieldtype_t : __int32
{
    SF_NONE = 0x0,
    SF_STRING = 0x1,
    SF_ENTITY = 0x2,
    SF_ENTHANDLE = 0x3,
    SF_CLIENT = 0x4,
    SF_ACTOR = 0x5,
    SF_SENTIENT = 0x6,
    SF_SENTIENTHANDLE = 0x7,
    SF_VEHICLE = 0x8,
    SF_TURRETINFO = 0x9,
    SF_THREAD = 0xA,
    SF_ANIMSCRIPT = 0xB,
    SF_PATHNODE = 0xC,
    SF_ANIMTREE = 0xD,
    SF_TYPE_TAG_INFO = 0xE,
    SF_TYPE_SCRIPTED = 0xF,
    SF_MODELUSHORT = 0x10,
    SF_MODELINT = 0x11,
};

enum SaveHandleType : __int32
{
    SAVE_GAME_HANDLE = 0x0,
    SAVE_DEMO_HANDLE = 0x1,
    SAVE_LAST_COMMITTED = 0x2,
};

enum SaveErrorType : __int32
{
    SAVE_ERROR_MISSING_DEVICE = 0x0,
    SAVE_ERROR_CORRUPT_SAVE = 0x1,
};

struct saveField_t
{
    int ofs;
    saveFieldtype_t type;
};

struct actor_s;
struct gclient_s;
struct scr_vehicle_s;
struct TurretInfo;
struct badplace_t;

// g_save
void __cdecl TRACK_g_save();
void __cdecl Scr_FreeFields(const saveField_t *fields, unsigned __int8 *base);
void __cdecl Scr_FreeEntityFields(gentity_s *ent);
void __cdecl Scr_FreeActorFields(actor_s *pActor);
void __cdecl Scr_FreeSentientFields(sentient_s *sentient);
// local variable allocation has failed, the output may be wrong!
void G_SaveError(
    errorParm_t code,
    SaveErrorType errorType,
    const char *fmt,
    ...);
void __cdecl WriteCStyleString(const char *psz, int maxlen, SaveGame *save);
void __cdecl ReadCStyleString(char *psz, int maxlen, SaveGame *save);
void __cdecl WriteWeaponIndex(unsigned int weapon, SaveGame *save);
int __cdecl ReadWeaponIndex(SaveGame *save);
void __cdecl WriteItemIndex(int iIndex, SaveGame *save);
int __cdecl ReadItemIndex(SaveGame *save);
void __cdecl WriteVehicleIndex(__int16 index, SaveGame *save);
int __cdecl ReadVehicleIndex(SaveGame *save);
void __cdecl WriteField1(const saveField_t *field, const unsigned __int8 *base, unsigned __int8 *original);
void __cdecl WriteField2(const saveField_t *field, unsigned __int8 *base, SaveGame *save);
void __cdecl ReadField(const saveField_t *field, unsigned __int8 *base, SaveGame *save);
void __cdecl G_WriteStruct(
    const saveField_t *fields,
    unsigned __int8 *original,
    const unsigned __int8 *source,
    int sourcesize,
    SaveGame *save);
void __cdecl G_ReadStruct(const saveField_t *fields, unsigned __int8 *dest, int tempsize, SaveGame *save);
void __cdecl WriteClient(gclient_s *cl, SaveGame *save);
void __cdecl ReadClient(gclient_s *client, SaveGame *save);
void __cdecl WriteEntity(gentity_s *ent, SaveGame *save);
void __cdecl ReadEntity(gentity_s *ent, SaveGame *save);
void __cdecl WriteActorPotentialCoverNodes(actor_s *pActor, SaveGame *save);
void __cdecl ReadActorPotentialCoverNodes(actor_s *pActor, SaveGame *save);
void __cdecl WriteActor(actor_s *pActor, SaveGame *save);
void __cdecl ReadActor(actor_s *pActor, SaveGame *save);
void __cdecl WriteSentient(sentient_s *sentient, SaveGame *save);
void __cdecl ReadSentient(sentient_s *sentient, SaveGame *save);
void __cdecl WriteVehicle(scr_vehicle_s *pVehicle, SaveGame *save);
void __cdecl ReadVehicle(scr_vehicle_s *pVehicle, SaveGame *save);
void __cdecl WriteTurretInfo(TurretInfo *pTurretInfo, SaveGame *save);
void __cdecl ReadTurretInfo(TurretInfo *pTurretInfo, SaveGame *save);
void __cdecl WritePathNodes(SaveGame *save);
void __cdecl ReadPathNodes(SaveGame *save);
const saveField_t *__cdecl BadPlaceParmSaveFields(const badplace_t *badplace);
void __cdecl WriteBadPlaces(SaveGame *save);
void __cdecl ReadBadPlaces(SaveGame *save);
void __cdecl WriteThreatBiasGroups(SaveGame *save);
void __cdecl ReadThreatBiasGroups(SaveGame *save);
void __cdecl WriteAIEventListeners(SaveGame *save);
void __cdecl ReadAIEventListeners(SaveGame *save);
char *__cdecl G_Save_DateStr();
void __cdecl G_SaveConfigstrings(int iFirst, int iCount, SaveGame *save);
void __cdecl G_LoadConfigstrings(int iFirst, int iCount, SaveGame *save);
void __cdecl G_LoadModelPrecacheList(SaveGame *save);
void __cdecl G_ClearConfigstrings(int iFirst, int iCount);
void __cdecl G_ClearAllConfigstrings();
void __cdecl G_SaveInitConfigstrings(SaveGame *save);
void __cdecl G_LoadInitConfigstrings(SaveGame *save);
void __cdecl G_SaveItems(SaveGame *save);
void __cdecl G_SaveWeaponCue(SaveGame *save);
void __cdecl G_LoadWeaponCue(SaveGame *save);
void __cdecl G_SaveDvars(SaveGame *save);
void __cdecl G_LoadDvars(SaveGame *save);
void __cdecl G_CheckEntityDefaultModel(gentity_s *e);
void __cdecl G_UpdateAllEntities();
void G_CheckAllEntities();
void __cdecl G_SaveInitState(SaveGame *save);
void __cdecl G_SaveMainState(bool savegame, SaveGame *save);
void __cdecl G_SaveState(bool savegame, SaveGame *save);
int __cdecl G_IsSavePossible(SaveType saveType);
int __cdecl G_WriteGame(const PendingSave *pendingSave, int checksum, SaveGame *save);
void __cdecl G_WriteCurrentCommitToDevice();
void __cdecl G_PrepareSaveMemoryForWrite(char commitLevel);
int __cdecl G_ProcessCommitActions(const PendingSave *pendingSave, SaveGame *save);
int __cdecl G_SaveGame(const PendingSave *pendingSave, int checksum);
bool __cdecl G_CommitSavedGame(int saveId);
void __cdecl G_LoadItems(SaveGame *save);
void __cdecl G_SetPendingLoadName(const char *filename);
void __cdecl G_PreLoadGame(int checksum, int *useLoadedSourceFiles, SaveGame **save);
int __cdecl G_LoadWeapons(SaveGame *save);
void __cdecl G_InitLoadGame(SaveGame *save);
void __cdecl G_LoadMainState(SaveGame *save);
void __cdecl G_LoadGame(int checksum, SaveGame *save);
int __cdecl G_LoadErrorCleanup();


extern bool g_useDevSaveArea;