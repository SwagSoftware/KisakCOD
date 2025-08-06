#pragma once

#include <universal/q_shared.h>

#ifdef KISAK_MP
#include <cgame_mp/cg_local_mp.h>
#include <game_mp/g_main_mp.h>
#include <client_mp/client_mp.h>
#elif KISAK_SP
#include <server/server.h>
#include <bgame/bg_local.h>
#endif

#include <script/scr_stringlist.h>
#include <script/scr_variable.h>
#include <bgame/bg_public.h>
//#include <game_mp/g_public_mp.h>

static const char *hintStrings[] = { "", "HINT_NOICON", "HINT_ACTIVATE", "HINT_HEALTH" }; // idb

// g_client_fields
#ifdef KISAK_MP
enum fieldtype_t : int32_t
{                                       // ...
    F_INT = 0x0,                 // ...
    F_FLOAT = 0x1,                 // ...
    F_LSTRING = 0x2,                 // ...
    F_STRING = 0x3,                 // ...
    F_VECTOR = 0x4,                 // ...
    F_ENTITY = 0x5,
    F_ENTHANDLE = 0x6,
    F_VECTORHACK = 0x7,
    F_OBJECT = 0x8,                 // ...
    F_MODEL = 0x9,                 // ...
};
#elif KISAK_SP
enum fieldtype_t : __int32
{
    F_INT = 0x0,
    F_SHORT = 0x1,
    F_BYTE = 0x2,
    F_FLOAT = 0x3,
    F_STRING = 0x4,
    F_VECTOR = 0x5,
    F_ENTITY = 0x6,
    F_ENTHANDLE = 0x7,
    F_ACTOR = 0x8,
    F_SENTIENT = 0x9,
    F_SENTIENTHANDLE = 0xA,
    F_CLIENT = 0xB,
    F_PATHNODE = 0xC,
    F_VECTORHACK = 0xD,
    F_MODEL = 0xE,
    F_ACTORGROUP = 0xF,
};
#endif

struct vehicle_info_t // sizeof=0x270
{                                       // ...
    char name[64];
    int16_t type;
    // padding byte
    // padding byte
    int32_t steerWheels;
    int32_t texScroll;
    int32_t quadBarrel;
    int32_t bulletDamage;
    int32_t armorPiercingDamage;
    int32_t grenadeDamage;
    int32_t projectileDamage;
    int32_t projectileSplashDamage;
    int32_t heavyExplosiveDamage;
    float texScrollScale;
    float maxSpeed;
    float accel;
    float rotRate;
    float rotAccel;
    float maxBodyPitch;
    float maxBodyRoll;
    float collisionDamage;
    float collisionSpeed;
    float suspensionTravel;
    char turretWeapon[64];
    float turretHorizSpanLeft;
    float turretHorizSpanRight;
    float turretVertSpanUp;
    float turretVertSpanDown;
    float turretRotRate;
    char sndNames[6][64];
    uint8_t sndIndices[6];
    // padding byte
    // padding byte
    float engineSndSpeed;
};
static_assert(sizeof(vehicle_info_t) == 0x270);

struct client_fields_s // sizeof=0x14
{                                       // ...
    const char *name;
    int32_t ofs;
    fieldtype_t type;
    void(__cdecl *setter)(gclient_s *, const client_fields_s *);
    void(__cdecl *getter)(gclient_s *, const client_fields_s *);
};
static_assert(sizeof(client_fields_s) == 0x14);

struct VehicleLocalPhysics // sizeof=0x34
{                                       // ...
    trace_t groundTrace;                // ...
    int32_t hasGround;                      // ...
    int32_t onGround;                       // ...
};
static_assert(sizeof(VehicleLocalPhysics) == 0x34);

struct VehiclePhysicsBackup // sizeof=0x1B8
{                                       // ...
    vehicle_pathpos_t pathPos;
    vehicle_physic_t phys;              // ...
};

void __cdecl ClientScr_ReadOnly(gclient_s *pSelf, const client_fields_s *pField);
void __cdecl ClientScr_SetSessionTeam(gclient_s *pSelf, const client_fields_s *pField);
void __cdecl ClientScr_GetName(gclient_s *pSelf, const client_fields_s *pField);
void __cdecl ClientScr_GetSessionTeam(gclient_s *pSelf, const client_fields_s *pField);
void __cdecl ClientScr_SetSessionState(gclient_s *pSelf, const client_fields_s *pField);
void __cdecl ClientScr_GetSessionState(gclient_s *pSelf, const client_fields_s *pField);
void __cdecl ClientScr_SetMaxHealth(gclient_s *pSelf, const client_fields_s *pField);
void __cdecl ClientScr_SetScore(gclient_s *pSelf, const client_fields_s *pField);
void __cdecl ClientScr_SetSpectatorClient(gclient_s *pSelf, const client_fields_s *pField);
void __cdecl ClientScr_SetKillCamEntity(gclient_s *pSelf, const client_fields_s *pField);
void __cdecl ClientScr_SetStatusIcon(gclient_s *pSelf, const client_fields_s *pField);
void __cdecl ClientScr_GetStatusIcon(gclient_s *pSelf, const client_fields_s *pField);
void __cdecl ClientScr_SetHeadIcon(gclient_s *pSelf, const client_fields_s *pField);
void __cdecl ClientScr_GetHeadIcon(gclient_s *pSelf, const client_fields_s *pField);
void __cdecl ClientScr_SetHeadIconTeam(gclient_s *pSelf, const client_fields_s *pField);
void __cdecl ClientScr_GetHeadIconTeam(gclient_s *pSelf, const client_fields_s *pField);
void __cdecl ClientScr_SetArchiveTime(gclient_s *pSelf, const client_fields_s *pField);
void __cdecl ClientScr_GetArchiveTime(gclient_s *pSelf, const client_fields_s *pField);
void __cdecl ClientScr_SetPSOffsetTime(gclient_s *pSelf, const client_fields_s *pField);
void __cdecl ClientScr_GetPSOffsetTime(gclient_s *pSelf, const client_fields_s *pField);

void __cdecl GScr_AddFieldsForClient();
void __cdecl Scr_SetClientField(gclient_s *client, int32_t offset);
void __cdecl Scr_GetClientField(gclient_s *client, int32_t offset);



// g_helicopter
void __cdecl VEH_CheckForPredictedCrash(gentity_s *ent);
void __cdecl VEH_UpdateClientChopper(gentity_s *ent);
void __cdecl HELI_CalcAccel(gentity_s *ent, char *move, float *bodyAccel, float *rotAccel);
void __cdecl HELI_CmdScale(char *move, float *outFracs);
void __cdecl HELI_UpdateJitter(VehicleJitter *jitter);
void __cdecl HELI_SoftenCollisions(gentity_s *ent, float *worldAccel);
bool __cdecl VEH_TestSlideMove(gentity_s *ent, float *outPos);



// g_hudelem
#ifdef KISAK_MP
struct game_hudelem_s // sizeof=0xAC
{
    hudelem_s elem;
    int32_t clientNum;
    int32_t team;
    int32_t archived;
};
static_assert(sizeof(game_hudelem_s) == 0xAC);

enum hudelem_update_t : int32_t
{                                       // ...
    HUDELEM_UPDATE_ARCHIVAL = 0x1,
    HUDELEM_UPDATE_CURRENT = 0x2,
    HUDELEM_UPDATE_ARCHIVAL_AND_CURRENT = 0x3,
};
#elif KISAK_SP
struct game_hudelem_s
{
    hudelem_s elem;
};
#endif

struct game_hudelem_field_t // sizeof=0x1C  (SP/MP same)
{                                       // ...
    const char *name;
    int32_t ofs;                            // ...
    fieldtype_t type;
    int32_t mask;
    int32_t shift;
    void(__cdecl *setter)(game_hudelem_s *, int);
    void(__cdecl *getter)(game_hudelem_s *, int);
};
static_assert(sizeof(game_hudelem_field_t) == 0x1C);



void __cdecl TRACK_g_hudelem();
game_hudelem_s *__cdecl HudElem_Alloc(int32_t clientNum, int32_t teamNum);
void __cdecl HudElem_SetDefaults(game_hudelem_s *hud);
void __cdecl HudElem_ClearTypeSettings(game_hudelem_s *hud);
void __cdecl HudElem_Free(game_hudelem_s *hud);
void __cdecl HudElem_ClientDisconnect(gentity_s *ent);
void __cdecl HudElem_DestroyAll();
void __cdecl HudElem_SetLocalizedString(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_SetFlagForeground(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_GetFlagForeground(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_SetFlagHideWhenDead(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_GetFlagHideWhenDead(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_SetFlagHideWhenInMenu(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_GetFlagHideWhenInMenu(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_SetBoolean(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_SetColor(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_GetColor(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_SetAlpha(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_GetAlpha(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_SetGlowColor(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_GetGlowColor(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_SetGlowAlpha(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_GetGlowAlpha(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_SetFontScale(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_SetFont(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_SetEnumString(
    game_hudelem_s *hud,
    const game_hudelem_field_t *f,
    const char **names,
    int32_t nameCount);
void __cdecl HudElem_GetFont(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_GetEnumString(
    game_hudelem_s *hud,
    const game_hudelem_field_t *f,
    const char **names,
    int32_t nameCount);
void __cdecl HudElem_SetAlignX(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_GetAlignX(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_SetAlignY(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_GetAlignY(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_SetHorzAlign(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_GetHorzAlign(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_SetVertAlign(game_hudelem_s *hud, int32_t offset);
void __cdecl HudElem_GetVertAlign(game_hudelem_s *hud, int32_t offset);
void __cdecl Scr_GetHudElemField(uint32_t entnum, uint32_t offset);
void __cdecl Scr_SetHudElemField(uint32_t entnum, uint32_t offset);
void __cdecl Scr_FreeHudElemConstStrings(game_hudelem_s *hud);
void __cdecl GScr_NewHudElem();
void __cdecl GScr_NewClientHudElem();
void __cdecl GScr_NewTeamHudElem();
void __cdecl GScr_AddFieldsForHudElems();
void __cdecl HECmd_SetText(scr_entref_t entref);
game_hudelem_s *__cdecl HECmd_GetHudElem(scr_entref_t entref);
void __cdecl HECmd_ClearAllTextAfterHudElem(scr_entref_t entref);
void __cdecl HECmd_SetMaterial(scr_entref_t entref);
void __cdecl HECmd_SetTargetEnt(scr_entref_t entref);
void __cdecl HECmd_ClearTargetEnt(scr_entref_t entref);
void __cdecl HECmd_SetTimer(scr_entref_t entref);
void __cdecl HECmd_SetTimer_Internal(scr_entref_t entref, he_type_t type, const char *cmdName);
void __cdecl HECmd_SetTimerUp(scr_entref_t entref);
void __cdecl HECmd_SetTenthsTimer(scr_entref_t entref);
void __cdecl HECmd_SetTenthsTimerUp(scr_entref_t entref);
void __cdecl HECmd_SetClock(scr_entref_t entref);
void __cdecl HECmd_SetClock_Internal(scr_entref_t entref, he_type_t type, const char *cmdName);
void __cdecl HECmd_SetClockUp(scr_entref_t entref);
void __cdecl HECmd_SetValue(scr_entref_t entref);
void __cdecl HECmd_SetWaypoint(scr_entref_t entref);
void __cdecl HECmd_FadeOverTime(scr_entref_t entref);
void __cdecl HECmd_ScaleOverTime(scr_entref_t entref);
void __cdecl HECmd_MoveOverTime(scr_entref_t entref);
void __cdecl HECmd_Reset(scr_entref_t entref);
void __cdecl HECmd_Destroy(scr_entref_t entref);
void __cdecl HECmd_SetPlayerNameString(scr_entref_t entref);
void __cdecl HECmd_SetGameTypeString(scr_entref_t entref);
void __cdecl HECmd_SetMapNameString(scr_entref_t entref);
void __cdecl HECmd_SetPulseFX(scr_entref_t entref);
VariableUnion __cdecl GetIntGTZero(uint32_t index);
void(__cdecl *__cdecl HudElem_GetMethod(const char **pName))(scr_entref_t);
#ifdef KISAK_MP
void __cdecl HudElem_UpdateClient(gclient_s *client, int32_t clientNum, hudelem_update_t which);
#elif KISAK_SP
void __cdecl HudElem_UpdateClient(gclient_s *client);
#endif

extern game_hudelem_s g_hudelems[1024];


// g_items
void __cdecl Fill_Clip(playerState_s *ps, uint32_t weapon);
int32_t __cdecl Add_Ammo(gentity_s *ent, uint32_t weaponIndex, uint8_t weaponModel, int32_t count, int32_t fillClip);
void __cdecl Touch_Item_Auto(gentity_s *ent, gentity_s *other, int32_t bTouched);
void __cdecl Touch_Item(gentity_s *ent, gentity_s *other, int32_t touched);
int32_t __cdecl WeaponPickup(gentity_s *weaponEnt, gentity_s *player, int32_t *pickupEvent, int32_t touched);
int32_t __cdecl WeaponPickup_Grab(gentity_s *weaponEnt, gentity_s *player, int32_t weapIdx, int32_t *pickupEvent);
int32_t __cdecl WeaponPickup_AddWeapon(
    gentity_s *ent,
    gentity_s *other,
    int32_t weapon,
    uint8_t weaponModel,
    gentity_s **pDroppedWeapon);
int32_t __cdecl CurrentPrimaryWeapon(playerState_s *ps);
int32_t __cdecl G_ItemClipMask(gentity_s *ent);
bool __cdecl WeaponPickup_LeechFromWeaponEnt(
    gentity_s *weaponEnt,
    gentity_s *player,
    int32_t haveExactWeapon,
    int32_t *pickupEvent,
    bool suppressNotifies);
void __cdecl PrintPlayerPickupMessage(gentity_s *player, uint32_t weapIdx, WeaponDef *weapDef);
void __cdecl WeaponPickup_AddAmmoForNewWeapon(gentity_s *weaponEnt, gentity_s *player);
void __cdecl WeaponPickup_Notifies(
    gentity_s *thisItem,
    gentity_s *newDroppedItem,
    gentity_s *player,
    WeaponDef *weapDef);
bool __cdecl WeaponPickup_Touch(gentity_s *weaponEnt, gentity_s *player, int32_t weapIdx, int32_t *pickupEvent);
void __cdecl PrintMessage_CannotGrabItem(gentity_s *ent, gentity_s *player, int32_t touched, gitem_s *item, int32_t weapIndex);
void __cdecl DroppedItemClearOwner(gentity_s *pSelf);
void __cdecl G_GetItemClassname(const gitem_s *item, uint16_t *out);
gentity_s *__cdecl Drop_Item(gentity_s *ent, const gitem_s *item, float angle, int32_t novelocity);
gentity_s *__cdecl LaunchItem(const gitem_s *item, float *origin, float *angles, float *velocity, int32_t ownerNum);
int32_t __cdecl GetFreeDropCueIdx();
bool __cdecl PlayerHasAnyAmmoToTransferToWeapon(gentity_s *player, uint32_t transferWeapon);
int32_t __cdecl GetNonClipAmmoToTransferToWeaponEntity(gentity_s *player, uint32_t transferWeapon);
gentity_s *__cdecl Drop_Weapon(gentity_s *ent, int32_t weapIdx, uint8_t weaponModel, uint32_t tag);
int32_t __cdecl TransferPlayerAmmoToWeaponEntity(gentity_s *player, gentity_s *weaponEnt, int32_t transferWeapon);
int32_t __cdecl TransferRandomAmmoToWeaponEntity(gentity_s *weaponEnt, int32_t transferWeapon);
void __cdecl FinishSpawningItem(gentity_s *ent);
void __cdecl ClearRegisteredItems();
void __cdecl SaveRegisteredWeapons();
void __cdecl SaveRegisteredItems();
void __cdecl G_RegisterWeapon(uint32_t weapIndex);
int32_t __cdecl IsItemRegistered(uint32_t iItemIndex);
void __cdecl G_SpawnItem(gentity_s *ent, const gitem_s *item);
void __cdecl G_RunItem(gentity_s *ent);
void __cdecl G_OrientItemToGround(gentity_s *ent, trace_t *trace);



// g_load_utils
void __cdecl G_ResetEntityParsePoint();
const char *__cdecl G_GetEntityParsePoint();
void __cdecl G_SetEntityParsePoint(const char *beginParsePoint);
int32_t __cdecl G_GetEntityToken(char *buffer, int32_t bufferSize);
int32_t __cdecl G_ParseSpawnVars(SpawnVar *spawnVar);
char *__cdecl G_AddSpawnVarToken(char *string, SpawnVar *spawnVar);
int32_t __cdecl G_SpawnString(const SpawnVar *spawnVar, const char *key, const char *defaultString, const char **out);
HashEntry_unnamed_type_u __cdecl G_NewString(const char *string);
char *__cdecl vtos(const float *v);




// g_missile
void __cdecl G_RegisterMissileDvars();
void __cdecl G_RegisterMissileDebugDvars();
void __cdecl G_TimedObjectThink(gentity_s *ent);
void __cdecl G_ExplodeMissile(gentity_s *ent);
int32_t __cdecl GetSplashMethodOfDeath(gentity_s *ent);
void __cdecl G_MissileTrace(trace_t *results, float *start, float *end, int32_t passEntityNum, int32_t contentmask);
void __cdecl TRACK_missile_attractors();
void __cdecl Missile_InitAttractors();
void __cdecl Missile_FreeAttractorRefs(gentity_s *ent);
void __cdecl Scr_MissileCreateAttractorEnt();
uint32_t __cdecl Missile_GetFreeAttractor();
void __cdecl Scr_MissileCreateAttractorOrigin();
void __cdecl Scr_MissileCreateRepulsorEnt();
void __cdecl Scr_MissileCreateRepulsorOrigin();
void __cdecl Scr_MissileDeleteAttractor();
void __cdecl G_MakeMissilePickupItem(gentity_s *ent);
void __cdecl G_RunMissile(gentity_s *ent);
void __cdecl MissileImpact(gentity_s *ent, trace_t *trace, float *dir, float *endpos);
bool __cdecl CheckCrumpleMissile(gentity_s *ent, trace_t *trace);
bool __cdecl BounceMissile(gentity_s *ent, trace_t *trace);
void __cdecl MissileLandAngles(gentity_s *ent, trace_t *trace, float *vAngles, int32_t bForceAlign);
void __cdecl MissileLandAnglesFlat(gentity_s *ent, trace_t *trace, float *angles);
void __cdecl MissileLandAnglesFlatMaintainingDirection(gentity_s *ent, trace_t *trace, float *angles);
void __cdecl CheckGrenadeDanger(gentity_s *grenadeEnt);
bool __cdecl GrenadeDud(gentity_s *ent, WeaponDef *weapDef);
bool __cdecl JavelinProjectile(gentity_s *ent, WeaponDef *weapDef);
bool __cdecl JavelinDud(gentity_s *ent, WeaponDef *weapDef);
void __cdecl Missile_PenetrateGlass(
    trace_t *results,
    gentity_s *ent,
    float *start,
    float *end,
    int32_t damage,
    bool predicted);
void __cdecl DrawMissileDebug(float *start, float *end);
void __cdecl RunMissile_Destabilize(gentity_s *missile);
double __cdecl RunMissile_GetPerturbation(float destabilizationCurvatureMax);
void __cdecl Missile_ApplyAttractorsRepulsors(gentity_s *missile);
void __cdecl RunMissile_CreateWaterSplash(const gentity_s *missile, const trace_t *trace);
void __cdecl MissileTrajectory(gentity_s *ent, float *result);
bool __cdecl MissileIsReadyForSteering(gentity_s *ent);
void __cdecl GuidedMissileSteering(gentity_s *ent);
bool IsMissileLockedOn(gentity_s *ent);
void __cdecl MissileHorzSteerToTarget(
    gentity_s *ent,
    const float *currentRight,
    const float *toTargetRelative,
    float currentHorzSpeed,
    float *steer);
void __cdecl MissileVerticalSteering(
    gentity_s *ent,
    const float *toTargetRelative,
    float currentHorzSpeed,
    float *steer);
void __cdecl MissileVerticalSteerToTarget(
    gentity_s *ent,
    const float *toTargetHorzRelDir,
    float horzDistToTarg,
    float vertDistToTarg,
    float currentHorzSpeed,
    float *steer);
void __cdecl GetTargetPosition(gentity_s *ent, float *result);
void __cdecl JavelinSteering(gentity_s *ent, WeaponDef *weapDef);
void __cdecl JavelinClimbOffset(gentity_s *ent, float *targetPos);
void __cdecl JavelinRotateVelocity(gentity_s *ent, const float *currentVel, const float *targetDir, float *resultVel);
double __cdecl JavelinRotateDir(gentity_s *ent, const float *currentDir, const float *targetDir, float *resultDir);
double __cdecl JavelinMaxDPS(gentity_s *ent);
void __cdecl VecToQuat(const float *vec, float *quat);
double __cdecl JavelinClimbCeiling(gentity_s *ent);
bool __cdecl JavelinClimbEnd(gentity_s *ent, const float *targetPos);
char __cdecl JavelinClimbExceededAngle(gentity_s *ent, const float *targetPos);
char __cdecl JavelinClimbWithinDistance(gentity_s *ent, const float *targetPos);
bool __cdecl JavelinClimbIsAboveCeiling(gentity_s *ent, const float *targetPos);
void __cdecl G_InitGrenadeEntity(gentity_s *parent, gentity_s *grenade);
void __cdecl G_InitGrenadeMovement(gentity_s *grenade, const float *start, const float *dir, int32_t rotate);
gentity_s *__cdecl G_FireGrenade(
    gentity_s *parent,
    float *start,
    float *dir,
    uint32_t grenadeWPID,
    uint8_t grenModel,
    int32_t rotate,
    int32_t time);
int32_t __cdecl CalcMissileNoDrawTime(float speed);
void __cdecl InitGrenadeTimer(const gentity_s *parent, gentity_s *grenade, const WeaponDef *weapDef, int32_t time);
gentity_s *__cdecl G_FireRocket(
    gentity_s *parent,
    uint32_t weaponIndex,
    float *start,
    float *dir,
    const float *gunVel,
    gentity_s *target,
    const float *targetOffset);
void __cdecl InitRocketTimer(gentity_s *bolt, WeaponDef *weapDef);
int G_PredictMissile(gentity_s *ent, int duration, float *vLandPos, int allowBounce, int *timeAtRest);

void Missile_LoadAttractors(struct MemoryFile *memFile);
void Missile_SaveAttractors(struct MemoryFile *memFile);




// g_mover
void __cdecl TRACK_g_mover();
gentity_s *__cdecl G_TestEntityPosition(gentity_s *ent, float *vOrigin);
void __cdecl G_CreateRotationMatrix(const float *angles, float (*matrix)[3]);
void __cdecl G_TransposeMatrix(float (*matrix)[3], float (*transpose)[3]);
int32_t __cdecl G_TryPushingEntity(gentity_s *check, gentity_s *pusher, float *move, float *amove);
void __cdecl G_MoverTeam(gentity_s *ent);
char __cdecl G_MoverPush(gentity_s *pusher, float *move, float *amove, gentity_s **obstacle);
void __cdecl G_RunMover(gentity_s *ent);
void __cdecl trigger_use(gentity_s *ent);
void __cdecl trigger_use_shared(gentity_s *self);
void __cdecl G_RotatePoint(float *point, float (*matrix)[3]);




// g_scr_mover
void __cdecl Reached_ScriptMover(gentity_s *pEnt);
int32_t __cdecl ScriptMover_UpdateMove(
    trajectory_t *pTr,
    float *vCurrPos,
    float fSpeed,
    float fMidTime,
    float fDecelTime,
    const float *vPos1,
    const float *vPos2,
    const float *vPos3);
void __cdecl InitScriptMover(gentity_s *pSelf);
void __cdecl SP_script_brushmodel(gentity_s *self);
void __cdecl SP_script_model(gentity_s *pSelf);
void __cdecl SP_script_origin(gentity_s *pSelf);
void __cdecl ScriptEntCmdGetCommandTimes(float *pfTotalTime, float *pfAccelTime, float *pfDecelTime);
void __cdecl ScriptEntCmd_MoveTo(scr_entref_t entref);
void __cdecl ScriptMover_Move(gentity_s *pEnt, const float *vPos, float fTotalTime, float fAccelTime, float fDecelTime);
void __cdecl ScriptMover_SetupMove(
    trajectory_t *pTr,
    const float *vPos,
    float fTotalTime,
    float fAccelTime,
    float fDecelTime,
    float *vCurrPos,
    float *pfSpeed,
    float *pfMidTime,
    float *pfDecelTime,
    float *vPos1,
    float *vPos2,
    float *vPos3);
void __cdecl ScriptEntCmd_GravityMove(scr_entref_t entref);
void __cdecl ScriptMover_GravityMove(gentity_s *mover, float *velocity, float totalTime);
void __cdecl ScriptEnt_MoveAxis(scr_entref_t entref, int32_t iAxis);
void __cdecl ScriptEntCmd_MoveX(scr_entref_t entref);
void __cdecl ScriptEntCmd_MoveY(scr_entref_t entref);
void __cdecl ScriptEntCmd_MoveZ(scr_entref_t entref);
void __cdecl ScriptEntCmd_RotateTo(scr_entref_t entref);
void __cdecl ScriptMover_Rotate(
    gentity_s *pEnt,
    const float *vRot,
    float fTotalTime,
    float fAccelTime,
    float fDecelTime);
void __cdecl ScriptEntCmd_DevAddPitch(scr_entref_t entref);
void __cdecl ScriptEnt_DevAddRotate(scr_entref_t entref, uint32_t iAxis);
void __cdecl ScriptEntCmd_DevAddYaw(scr_entref_t entref);
void __cdecl ScriptEntCmd_DevAddRoll(scr_entref_t entref);
void __cdecl ScriptEnt_RotateAxis(scr_entref_t entref, int32_t iAxis);
void __cdecl ScriptEntCmd_RotatePitch(scr_entref_t entref);
void __cdecl ScriptEntCmd_RotateYaw(scr_entref_t entref);
void __cdecl ScriptEntCmd_RotateRoll(scr_entref_t entref);
void __cdecl ScriptEntCmd_Vibrate(scr_entref_t entref);
void __cdecl ScriptEntCmd_RotateVelocity(scr_entref_t entref);
void __cdecl ScriptMover_RotateSpeed(
    gentity_s *pEnt,
    const float *vRotSpeed,
    float fTotalTime,
    float fAccelTime,
    float fDecelTime);
void __cdecl ScriptMover_SetupMoveSpeed(
    trajectory_t *pTr,
    const float *vSpeed,
    float fTotalTime,
    float fAccelTime,
    float fDecelTime,
    float *vCurrPos,
    float *pfSpeed,
    float *pfMidTime,
    float *pfDecelTime,
    float *vPos1,
    float *vPos2,
    float *vPos3);
void __cdecl ScriptEntCmd_SetCanDamage(scr_entref_t entref);
void __cdecl ScriptEntCmd_PhysicsLaunch(scr_entref_t entref);
void __cdecl ScriptMover_SetupPhysicsLaunch(
    trajectory_t *pTr,
    trajectory_t *paTr,
    const float *contact_point,
    const float *initial_force);
void __cdecl ScriptEntCmd_Solid(scr_entref_t entref);
void __cdecl ScriptEntCmd_NotSolid(scr_entref_t entref);
void(__cdecl *__cdecl ScriptEnt_GetMethod(const char **pName))(scr_entref_t);




// g_scr_vehicle
gentity_s *__cdecl GScr_GetVehicle(scr_entref_t entref);
gentity_s *__cdecl VEH_GetVehicle(int32_t entNum);
void __cdecl VEH_InitEntity(gentity_s *ent, scr_vehicle_s *veh, int32_t infoIdx);
void __cdecl VEH_InitVehicle(gentity_s *ent, scr_vehicle_s *veh, __int16 infoIdx);
void __cdecl VEH_SetPosition(gentity_s *ent, const float *origin, const float *vel, const float *angles);
void __cdecl VEH_InitPhysics(gentity_s *ent);
int32_t __cdecl VEH_CorrectAllSolid(gentity_s *ent, trace_t *trace);
void __cdecl VEH_ClearGround();
bool __cdecl VEH_SlideMove(gentity_s *ent, int32_t gravity);
void __cdecl VEH_ClipVelocity(float *in, float *normal, float *out);
void __cdecl Scr_Vehicle_Think(gentity_s *pSelf);
void __cdecl VEH_MoveTrace(gentity_s *ent);
void __cdecl VEH_BackupPosition(gentity_s *ent);
void __cdecl VEH_TouchEntities(gentity_s *ent);
void __cdecl VEH_PushEntity(gentity_s *ent, gentity_s *target, float *pushDir, float *deltaOrigin, float *deltaAngles);
bool __cdecl AttachedStickyMissile(gentity_s *vehicle, gentity_s *missile);
void __cdecl PushAttachedStickyMissile(gentity_s *vehicle, gentity_s *missile);
void __cdecl VEH_UpdateAim(gentity_s *ent);
void __cdecl VEH_UpdateAIMove(gentity_s *ent);
void __cdecl VEH_UpdateMoveToGoal(gentity_s *ent, const float *goalPos);
bool __cdecl VEH_IsHovering(scr_vehicle_s *veh);
void __cdecl VEH_UpdateMoveOrientation(gentity_s *ent, float *desiredDir);
void __cdecl VEH_UpdateAngleAndAngularVel(
    int32_t index,
    float desiredAngle,
    float accel,
    float decel,
    float overShoot,
    vehicle_physic_t *phys);
float __cdecl VEH_AccelerateSpeed(float speed, float tgtSpeed, float accel, float dt);
float __cdecl VEH_UpdateMove_GetDesiredYaw(scr_vehicle_s *veh, float *desiredDir);
float __cdecl VEH_CalcAccelFraction(float accel, int32_t infoIdx);
float __cdecl VEH_CalcAngularAccel(float accel, float accelFraction);
float __cdecl VEH_CalcAngleForAccel(float accel, float accelFraction);
float __cdecl VEH_CalcStoppingTime(float accel, float accelFraction);
void __cdecl VEH_UpdateYawAndNotify(gentity_s *ent, float desiredYaw);
float __cdecl VEH_GetAccelForAngles(scr_vehicle_s *veh);
void __cdecl VEH_AddFakeDrag(const float *velocity, float maxDragSpeed, float *accelVec);
void __cdecl VEH_CheckHorizontalVelocityToGoal(
    scr_vehicle_s *veh,
    const float *vecToGoal,
    float accelMax,
    float *accelVec);
void __cdecl VEH_CheckVerticalVelocityToGoal(scr_vehicle_s *veh, float verticalDist, float *accelVec);
int32_t __cdecl VEH_UpdateMove_CheckGoalReached(gentity_s *ent, float distToGoal);
double __cdecl VEH_UpdateMove_CheckStop(scr_vehicle_s *veh, float distToGoal);
void __cdecl VEH_UpdateMove_CheckNearGoal(gentity_s *ent, float distToGoal);
void __cdecl VEH_GetNewSpeedAndAccel(scr_vehicle_s *veh, float dt, int32_t hovering, float *newSpeed, float *accelMax);
void __cdecl VEH_UpdateHover(gentity_s *ent);
void __cdecl VEH_SetHoverGoal(gentity_s *ent);
void __cdecl CMD_VEH_SetSpeed(scr_entref_t entref);
void __cdecl CMD_VEH_Script_SetSpeed(gentity_s *ent);
void __cdecl CMD_VEH_GetSpeed(scr_entref_t entref);
void __cdecl CMD_VEH_GetSpeedMPH(scr_entref_t entref);
void __cdecl CMD_VEH_ResumeSpeed(scr_entref_t entref);
void __cdecl CMD_VEH_SetYawSpeed(scr_entref_t entref);
void __cdecl CMD_VEH_SetMaxPitchRoll(scr_entref_t entref);
void __cdecl CMD_VEH_SetAirResitance(scr_entref_t entref);
void __cdecl CMD_VEH_SetTurningAbility(scr_entref_t entref);
void __cdecl CMD_VEH_SetHoverParams(scr_entref_t entref);
void __cdecl CMD_VEH_SetVehicleTeam(scr_entref_t entref);
void __cdecl CMD_VEH_NearGoalNotifyDist(scr_entref_t entref);
void __cdecl CMD_VEH_SetGoalPos(scr_entref_t entref);
void __cdecl CMD_VEH_SetGoalYaw(scr_entref_t entref);
void __cdecl CMD_VEH_ClearGoalYaw(scr_entref_t entref);
void __cdecl CMD_VEH_SetTargetYaw(scr_entref_t entref);
void __cdecl CMD_VEH_ClearTargetYaw(scr_entref_t entref);
void __cdecl CMD_VEH_SetTurretTargetVec(scr_entref_t entref);
void __cdecl CMD_VEH_SetTurretTargetEnt(scr_entref_t entref);
void __cdecl CMD_VEH_ClearTurretTargetEnt(scr_entref_t entref);
void __cdecl CMD_VEH_SetLookAtEnt(scr_entref_t entref);
void __cdecl CMD_VEH_ClearLookAtEnt(scr_entref_t entref);
void __cdecl CMD_VEH_SetWeapon(scr_entref_t entref);
void __cdecl CMD_VEH_FireWeapon(scr_entref_t entref);
int32_t __cdecl VEH_GetTagBoneIndex(gentity_s *ent, int32_t barrel);
void __cdecl VEH_SetPosition(gentity_s *ent, const float *origin, const float *angles);
void __cdecl VEH_JoltBody(gentity_s *ent, const float *dir, float intensity, float speedFrac, float decel);
void __cdecl VEH_StepSlideMove(gentity_s *ent, int32_t gravity, float frameTime);
bool __cdecl VEH_SlideMove(gentity_s *ent, int32_t gravity, float frameTime);
void __cdecl VEH_AirMove(gentity_s *ent, int32_t gravity, float frameTime);

gentity_s *G_IsVehicleUnusable(gentity_s *player);
bool G_IsVehicleImmune(gentity_s *ent, int mod, char damageFlags, unsigned int weapon);
bool G_IsPlayerDrivingVehicle(const gentity_s *player);
gentity_s *VEH_GetCollMap(const char *modelname);
void VEH_SetupCollmap(gentity_s *ent);
void G_UpdateVehicleTags(gentity_s *ent);
void G_SpawnVehicle(gentity_s *ent, const char *typeName, int load);
const char *G_GetVehicleInfoName(__int16 index);
int G_GetVehicleInfoIndex(const char *name);
bool G_IsVehicleUsable(gentity_s *ent, gentity_s *player);
void G_PrecacheDefaultVehicle();
void G_FreeVehicleRefs(gentity_s *ent);
gentity_s *G_GetPlayerVehicle(const gentity_s *player);

void G_InitScrVehicles();
void G_SetupScrVehicles();
void G_FreeScrVehicles();
void G_RestartScrVehicleInfo();
void G_ParseScrVehicleInfo();
void G_FreeVehicle(gentity_s *ent);

vehicle_info_t *VEH_GetVehicleInfo(__int16 index);

void(*ScriptVehicle_GetMethod(const char **pName))(scr_entref_t);
void G_SaveVehicleInfo(struct SaveGame *save);
void G_LoadVehicleInfo(SaveGame *save);


// g_weapon
struct AntilagClientStore // sizeof=0x340
{                                       // ...
    float realClientPositions[64][3];
    bool clientMoved[64];
};
struct weaponParms;
void __cdecl G_AntiLagRewindClientPos(int32_t gameTime, AntilagClientStore *antilagStore);
void __cdecl G_AntiLag_RestoreClientPos(AntilagClientStore *antilagStore);
gentity_s *__cdecl Weapon_Melee(gentity_s *ent, weaponParms *wp, float range, float width, float height, int32_t gametime);
gentity_s *__cdecl Weapon_Melee_internal(gentity_s *ent, weaponParms *wp, float range, float width, float height);
char __cdecl Melee_Trace(
    gentity_s *ent,
    weaponParms *wp,
    int32_t damage,
    float range,
    float width,
    float height,
    trace_t *trace,
    float *endPos);
gentity_s *__cdecl Weapon_Throw_Grenade(
    gentity_s *ent,
    uint32_t grenType,
    uint8_t grenModel,
    weaponParms *wp);
void __cdecl Vec2NormalizeFast(float *v);
gentity_s *__cdecl Weapon_GrenadeLauncher_Fire(
    gentity_s *ent,
    uint32_t grenType,
    uint8_t grenModel,
    weaponParms *wp);
gentity_s *__cdecl Weapon_RocketLauncher_Fire(
    gentity_s *ent,
    uint32_t weaponIndex,
    float spread,
    struct weaponParms *wp,
    const float *gunVel,
    gentity_s *target,
    const float *targetOffset);
void __cdecl gunrandom(float *x, float *y);
bool __cdecl LogAccuracyHit(gentity_s *target, gentity_s *attacker);
void __cdecl FireWeapon(gentity_s *ent, int32_t gametime);
void __cdecl CalcMuzzlePoints(const gentity_s *ent, weaponParms *wp);
void __cdecl G_UseOffHand(gentity_s *ent);
void __cdecl FireWeaponMelee(gentity_s *ent, int32_t gametime);
int32_t __cdecl G_GivePlayerWeapon(playerState_s *pPS, int32_t iWeaponIndex, uint8_t altModelIndex);
void __cdecl G_SetupWeaponDef();
uint32_t __cdecl G_GetWeaponIndexForName(const char *name);
void __cdecl G_SelectWeaponIndex(int32_t clientNum, int32_t iWeaponIndex);
void __cdecl G_SetEquippedOffHand(int32_t clientNum, uint32_t offHandIndex);



// g_debug
void __cdecl G_DebugLine(const float *start, const float *end, const float *color, int32_t depthTest);
void __cdecl G_DebugLineWithDuration(
    const float *start,
    const float *end,
    const float *color,
    int32_t depthTest,
    int32_t duration);
void __cdecl G_DebugStar(const float *point, const float *color);
void __cdecl G_DebugStarWithText(
    const float *point,
    const float *starColor,
    const float *textColor,
    char *string,
    float fontsize);
void __cdecl G_DebugBox(
    const float *origin,
    const float *mins,
    const float *maxs,
    float yaw,
    const float *color,
    int32_t depthTest,
    int32_t duration);
void __cdecl G_DebugCircle(
    const float *center,
    float radius,
    const float *color,
    int32_t depthTest,
    int32_t onGround,
    int32_t duration);
void __cdecl G_DebugCircleEx(
    const float *center,
    float radius,
    const float *dir,
    const float *color,
    int32_t depthTest,
    int32_t duration);
void G_DebugArc(
    const float *center,
    double radius,
    double angle0,
    double angle1,
    const float *color,
    int depthTest,
    int duration);
void G_DebugDrawBrushModel(gentity_s *entity, const float *color, int depthTest, int duration);
void G_DebugPlane(
    const float *const normal,
    float dist,
    const float *const origin,
    const float *const color,
    float size,
    int depthTest,
    int duration);


// g_svcmds
struct ipFilter_s // sizeof=0x8
{                                       // ...
    uint32_t mask;                  // ...
    uint32_t compare;               // ...
};
void __cdecl G_ProcessIPBans();
void __cdecl AddIP(char *str);
int32_t __cdecl StringToFilter(char *s, ipFilter_s *f);
void UpdateIPBans();
void __cdecl Svcmd_AddIP_f();
void __cdecl Svcmd_RemoveIP_f();
void __cdecl Svcmd_EntityList_f();
int32_t __cdecl ConsoleCommand();

void __cdecl G_FreeEntity(gentity_s *ed);

void __cdecl Touch_Multi(gentity_s *self, gentity_s *other, int32_t extra);
void __cdecl hurt_use(gentity_s *self, gentity_s *other, gentity_s *third);
void __cdecl hurt_touch(gentity_s *self, gentity_s *other, int32_t extra);
void __cdecl Use_trigger_damage(gentity_s *pEnt, gentity_s *pOther, gentity_s *third);
void __cdecl Pain_trigger_damage(gentity_s *pSelf, gentity_s *pAttacker, int32_t iDamage, const float *vPoint, int32_t iMod, const float *idk, hitLocation_t hit, int32_t swag);
void Die_trigger_damage(
    gentity_s *pSelf,
    gentity_s *pInflictor,
    gentity_s *pAttacker,
    int32_t iDamage,
    int32_t iMod,
    int32_t iWeapon,
    const float *vDir,
    const hitLocation_t hitLoc,
    int32_t timeOffset);
void __cdecl player_die(
    gentity_s *self,
    gentity_s *inflictor,
    gentity_s *attacker,
    int32_t damage,
    int32_t meansOfDeath,
    int32_t iWeapon,
    const float *vDir,
    hitLocation_t hitLoc,
    int32_t psTimeOffset);
void __cdecl G_PlayerController(const gentity_s *self, int32_t *partBits);
void __cdecl BodyEnd(gentity_s *ent);
void __cdecl turret_think_init(gentity_s *self);
void __cdecl turret_use(gentity_s *self, gentity_s *owner, gentity_s *activator);
void __cdecl turret_controller(const gentity_s *self, int32_t *partBits);
void __cdecl turret_think(gentity_s *self);
void __cdecl G_VehEntHandler_Think(gentity_s *pSelf);
void __cdecl G_VehEntHandler_Touch(gentity_s *pSelf, gentity_s *pOther, int32_t bTouched);
void __cdecl G_VehEntHandler_Use(gentity_s *pEnt, gentity_s *pOther, gentity_s *pActivator);
void __cdecl Helicopter_Pain(
    gentity_s *pSelf,
    gentity_s *pAttacker,
    int32_t damage,
    const float *point,
    const int32_t mod,
    const float *dir,
    const hitLocation_t hitLoc,
    const int32_t weaponIdx);
void __cdecl G_VehEntHandler_Die(
    gentity_s *pSelf,
    gentity_s *pInflictor,
    gentity_s *pAttacker,
    const int32_t damage,
    const int32_t mod,
    const int32_t weapon,
    const float *dir,
    const hitLocation_t hitLoc,
    int32_t psTimeOffset);
void __cdecl G_VehEntHandler_Controller(const gentity_s *pSelf, int32_t *partBits);
void __cdecl Helicopter_Think(gentity_s *ent);
void __cdecl Helicopter_Die(
    gentity_s *pSelf,
    gentity_s *pInflictor,
    gentity_s *pAttacker,
    const int32_t damage,
    const int32_t mod,
    const int32_t weapon,
    const float *dir,
    const hitLocation_t hitLoc,
    int32_t psTimeOffset);

void __cdecl Helicopter_Controller(const gentity_s *pSelf, int32_t *partBits);




extern vehicle_info_t s_vehicleInfos[32];

extern VehicleLocalPhysics s_phys;
extern VehicleLocalPhysics s_phys_0;
extern VehiclePhysicsBackup s_backup_0;
extern VehiclePhysicsBackup s_backup;