#pragma once
#include <cgame_mp/cg_local_mp.h>
#include <game_mp/g_main_mp.h>

#include <script/scr_stringlist.h>
#include <script/scr_variable.h>


// g_client_fields
enum fieldtype_t : __int32
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

struct client_fields_s // sizeof=0x14
{                                       // ...
    const char *name;
    int ofs;
    fieldtype_t type;
    void(__cdecl *setter)(gclient_s *, const client_fields_s *);
    void(__cdecl *getter)(gclient_s *, const client_fields_s *);
};

void __cdecl ClientScr_ReadOnly(gclient_s *pSelf, const client_fields_s *pField);
void __cdecl ClientScr_SetSessionTeam(gclient_s *pSelf);
void __cdecl ClientScr_GetName(gclient_s *pSelf);
void __cdecl ClientScr_GetSessionTeam(gclient_s *pSelf);
void __cdecl ClientScr_SetSessionState(gclient_s *pSelf);
void __cdecl ClientScr_GetSessionState(gclient_s *pSelf);
void __cdecl ClientScr_SetMaxHealth(gclient_s *pSelf);
void __cdecl ClientScr_SetScore(gclient_s *pSelf);
void __cdecl ClientScr_SetSpectatorClient(gclient_s *pSelf);
void __cdecl ClientScr_SetKillCamEntity(gclient_s *pSelf);
void __cdecl ClientScr_SetStatusIcon(gclient_s *pSelf);
void __cdecl ClientScr_GetStatusIcon(gclient_s *pSelf);
void __cdecl ClientScr_SetHeadIcon(gclient_s *pSelf);
void __cdecl ClientScr_GetHeadIcon(gclient_s *pSelf);
void __cdecl ClientScr_SetHeadIconTeam(gclient_s *pSelf);
void __cdecl ClientScr_GetHeadIconTeam(gclient_s *pSelf);
void __cdecl ClientScr_SetArchiveTime(gclient_s *pSelf);
void __cdecl ClientScr_GetArchiveTime(gclient_s *pSelf);
void __cdecl ClientScr_SetPSOffsetTime(gclient_s *pSelf);
void __cdecl ClientScr_GetPSOffsetTime(gclient_s *pSelf);
void __cdecl GScr_AddFieldsForClient();
void __cdecl Scr_SetClientField(gclient_s *client, int offset);
void __cdecl Scr_GetClientField(gclient_s *client, int offset);



// g_helicopter
void __cdecl VEH_CheckForPredictedCrash(gentity_s *ent);
void __cdecl VEH_UpdateClientChopper(gentity_s *ent);
void __cdecl HELI_CalcAccel(gentity_s *ent, char *move, float *bodyAccel, float *rotAccel);
void __cdecl HELI_CmdScale(char *move, float *outFracs);
void __cdecl HELI_UpdateJitter(VehicleJitter *jitter);
void __cdecl HELI_SoftenCollisions(gentity_s *ent, float *worldAccel);
bool __cdecl VEH_TestSlideMove(gentity_s *ent, float *outPos);



// g_hudelem
struct game_hudelem_s // sizeof=0xAC
{
    hudelem_s elem;
    int clientNum;
    int team;
    int archived;
};
struct game_hudelem_field_t // sizeof=0x1C
{                                       // ...
    const char *name;
    int ofs;                            // ...
    fieldtype_t type;
    int mask;
    int shift;
    void(__cdecl *setter)(game_hudelem_s *, int);
    void(__cdecl *getter)(game_hudelem_s *, int);
};
enum hudelem_update_t : __int32
{                                       // ...
    HUDELEM_UPDATE_ARCHIVAL = 0x1,
    HUDELEM_UPDATE_CURRENT = 0x2,
    HUDELEM_UPDATE_ARCHIVAL_AND_CURRENT = 0x3,
};
void __cdecl TRACK_g_hudelem();
game_hudelem_s *__cdecl HudElem_Alloc(int clientNum, int teamNum);
void __cdecl HudElem_SetDefaults(game_hudelem_s *hud);
void __cdecl HudElem_ClearTypeSettings(game_hudelem_s *hud);
void __cdecl HudElem_Free(game_hudelem_s *hud);
void __cdecl HudElem_ClientDisconnect(gentity_s *ent);
void __cdecl HudElem_DestroyAll();
void __cdecl HudElem_SetLocalizedString(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetFlagForeground(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetFlagForeground(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetFlagHideWhenDead(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetFlagHideWhenDead(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetFlagHideWhenInMenu(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetFlagHideWhenInMenu(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetBoolean(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetColor(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetColor(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetAlpha(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetAlpha(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetGlowColor(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetGlowColor(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetGlowAlpha(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetGlowAlpha(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetFontScale(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetFont(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetEnumString(
    game_hudelem_s *hud,
    const game_hudelem_field_t *f,
    const char **names,
    int nameCount);
void __cdecl HudElem_GetFont(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetEnumString(
    game_hudelem_s *hud,
    const game_hudelem_field_t *f,
    const char **names,
    int nameCount);
void __cdecl HudElem_SetAlignX(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetAlignX(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetAlignY(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetAlignY(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetHorzAlign(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetHorzAlign(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetVertAlign(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetVertAlign(game_hudelem_s *hud, int offset);
void __cdecl Scr_GetHudElemField(unsigned int entnum, unsigned int offset);
void __cdecl Scr_SetHudElemField(unsigned int entnum, unsigned int offset);
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
VariableUnion __cdecl GetIntGTZero(unsigned int index);
void(__cdecl *__cdecl HudElem_GetMethod(const char **pName))(scr_entref_t);
void __cdecl HudElem_UpdateClient(gclient_s *client, int clientNum, hudelem_update_t which);



// g_items
void __cdecl Fill_Clip(playerState_s *ps, unsigned int weapon);
int __cdecl Add_Ammo(gentity_s *ent, unsigned int weaponIndex, unsigned __int8 weaponModel, int count, int fillClip);
void __cdecl Touch_Item_Auto(gentity_s *ent, gentity_s *other, int bTouched);
void __cdecl Touch_Item(gentity_s *ent, gentity_s *other, int touched);
int __cdecl WeaponPickup(gentity_s *weaponEnt, gentity_s *player, int *pickupEvent, int touched);
int __cdecl WeaponPickup_Grab(gentity_s *weaponEnt, gentity_s *player, int weapIdx, int *pickupEvent);
int __cdecl WeaponPickup_AddWeapon(
    gentity_s *ent,
    gentity_s *other,
    int weapon,
    unsigned __int8 weaponModel,
    gentity_s **pDroppedWeapon);
int __cdecl CurrentPrimaryWeapon(playerState_s *ps);
int __cdecl G_ItemClipMask(gentity_s *ent);
bool __cdecl WeaponPickup_LeechFromWeaponEnt(
    gentity_s *weaponEnt,
    gentity_s *player,
    int haveExactWeapon,
    int *pickupEvent,
    bool suppressNotifies);
void __cdecl PrintPlayerPickupMessage(gentity_s *player, unsigned int weapIdx, WeaponDef *weapDef);
void __cdecl WeaponPickup_AddAmmoForNewWeapon(gentity_s *weaponEnt, gentity_s *player);
void __cdecl WeaponPickup_Notifies(
    gentity_s *thisItem,
    gentity_s *newDroppedItem,
    gentity_s *player,
    WeaponDef *weapDef);
bool __cdecl WeaponPickup_Touch(gentity_s *weaponEnt, gentity_s *player, int weapIdx, int *pickupEvent);
void __cdecl PrintMessage_CannotGrabItem(gentity_s *ent, gentity_s *player, int touched, gitem_s *item, int weapIndex);
void __cdecl DroppedItemClearOwner(gentity_s *pSelf);
void __cdecl G_GetItemClassname(const gitem_s *item, unsigned __int16 *out);
gentity_s *__cdecl Drop_Item(gentity_s *ent, const gitem_s *item, float angle, int novelocity);
gentity_s *__cdecl LaunchItem(const gitem_s *item, float *origin, float *angles, float *velocity, int ownerNum);
int __cdecl GetFreeDropCueIdx();
bool __cdecl PlayerHasAnyAmmoToTransferToWeapon(gentity_s *player, unsigned int transferWeapon);
int __cdecl GetNonClipAmmoToTransferToWeaponEntity(gentity_s *player, unsigned int transferWeapon);
gentity_s *__cdecl Drop_Weapon(gentity_s *ent, int weapIdx, unsigned __int8 weaponModel, unsigned int tag);
int __cdecl TransferPlayerAmmoToWeaponEntity(gentity_s *player, gentity_s *weaponEnt, int transferWeapon);
int __cdecl TransferRandomAmmoToWeaponEntity(gentity_s *weaponEnt, int transferWeapon);
void __cdecl FinishSpawningItem(gentity_s *ent);
void __cdecl ClearRegisteredItems();
void __cdecl SaveRegisteredWeapons();
void __cdecl SaveRegisteredItems();
void __cdecl G_RegisterWeapon(unsigned int weapIndex);
int __cdecl IsItemRegistered(unsigned int iItemIndex);
void __cdecl G_SpawnItem(gentity_s *ent, const gitem_s *item);
void __cdecl G_RunItem(gentity_s *ent);
void __cdecl G_OrientItemToGround(gentity_s *ent, trace_t *trace);



// g_load_utils
void __cdecl G_ResetEntityParsePoint();
const char *__cdecl G_GetEntityParsePoint();
int __cdecl G_GetEntityToken(char *buffer, int bufferSize);
int __cdecl G_ParseSpawnVars(SpawnVar *spawnVar);
char *__cdecl G_AddSpawnVarToken(char *string, SpawnVar *spawnVar);
int __cdecl G_SpawnString(const SpawnVar *spawnVar, const char *key, const char *defaultString, const char **out);
HashEntry_unnamed_type_u __cdecl G_NewString(const char *string);
char *__cdecl vtos(const float *v);




// g_missile
void __cdecl G_RegisterMissileDvars();
void __cdecl G_RegisterMissileDebugDvars();
void __cdecl G_TimedObjectThink(gentity_s *ent);
void __cdecl G_ExplodeMissile(gentity_s *ent);
int __cdecl GetSplashMethodOfDeath(gentity_s *ent);
void __cdecl G_MissileTrace(trace_t *results, float *start, float *end, int passEntityNum, int contentmask);
void __cdecl TRACK_missile_attractors();
void __cdecl Missile_InitAttractors();
void __cdecl Missile_FreeAttractorRefs(gentity_s *ent);
void __cdecl Scr_MissileCreateAttractorEnt();
unsigned int __cdecl Missile_GetFreeAttractor();
void __cdecl Scr_MissileCreateAttractorOrigin();
void __cdecl Scr_MissileCreateRepulsorEnt();
void __cdecl Scr_MissileCreateRepulsorOrigin();
void __cdecl Scr_MissileDeleteAttractor();
void __cdecl G_MakeMissilePickupItem(gentity_s *ent);
void __cdecl G_RunMissile(gentity_s *ent);
void __cdecl MissileImpact(gentity_s *ent, trace_t *trace, float *dir, float *endpos);
bool __cdecl CheckCrumpleMissile(gentity_s *ent, trace_t *trace);
bool __cdecl BounceMissile(gentity_s *ent, trace_t *trace);
void __cdecl MissileLandAngles(gentity_s *ent, trace_t *trace, float *vAngles, int bForceAlign);
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
    int damage,
    bool predicted);
void __cdecl DrawMissileDebug(float *start, float *end);
void __cdecl RunMissile_Destabilize(gentity_s *missile);
double __cdecl RunMissile_GetPerturbation(float destabilizationCurvatureMax);
void __cdecl Missile_ApplyAttractorsRepulsors(gentity_s *missile);
void __cdecl RunMissile_CreateWaterSplash(const gentity_s *missile, const trace_t *trace);
void __cdecl MissileTrajectory(gentity_s *ent, float *result);
bool __cdecl MissileIsReadyForSteering(gentity_s *ent);
void __cdecl GuidedMissileSteering(gentity_s *ent);
char __cdecl IsMissileLockedOn(gentity_s *ent);
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
void __cdecl G_InitGrenadeMovement(gentity_s *grenade, const float *start, const float *dir, int rotate);
gentity_s *__cdecl G_FireGrenade(
    gentity_s *parent,
    float *start,
    float *dir,
    unsigned int grenadeWPID,
    unsigned __int8 grenModel,
    int rotate,
    int time);
int __cdecl CalcMissileNoDrawTime(float speed);
void __cdecl InitGrenadeTimer(const gentity_s *parent, gentity_s *grenade, const WeaponDef *weapDef, int time);
gentity_s *__cdecl G_FireRocket(
    gentity_s *parent,
    unsigned int weaponIndex,
    float *start,
    float *dir,
    const float *gunVel,
    gentity_s *target,
    const float *targetOffset);
void __cdecl InitRocketTimer(gentity_s *bolt, WeaponDef *weapDef);




// g_mover
void __cdecl TRACK_g_mover();
gentity_s *__cdecl G_TestEntityPosition(gentity_s *ent, float *vOrigin);
void __cdecl G_CreateRotationMatrix(const float *angles, float (*matrix)[3]);
int __cdecl G_TryPushingEntity(gentity_s *check, gentity_s *pusher, float *move, float *amove);
void __cdecl G_MoverTeam(gentity_s *ent);
char __cdecl G_MoverPush(gentity_s *pusher, float *move, float *amove, gentity_s **obstacle);
void __cdecl G_RunMover(gentity_s *ent);
void __cdecl trigger_use(gentity_s *ent);
void __cdecl trigger_use_shared(gentity_s *self);
void __cdecl G_RotatePoint(float *point, float (*matrix)[3]);




// g_scr_mover
void __cdecl Reached_ScriptMover(gentity_s *pEnt);
int __cdecl ScriptMover_UpdateMove(
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
void __cdecl ScriptEnt_MoveAxis(scr_entref_t entref, int iAxis);
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
void __cdecl ScriptEnt_DevAddRotate(scr_entref_t entref, unsigned int iAxis);
void __cdecl ScriptEntCmd_DevAddYaw(scr_entref_t entref);
void __cdecl ScriptEntCmd_DevAddRoll(scr_entref_t entref);
void __cdecl ScriptEnt_RotateAxis(scr_entref_t entref, int iAxis);
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
gentity_s *__cdecl VEH_GetVehicle(int entNum);
void __cdecl VEH_InitEntity(gentity_s *ent, scr_vehicle_s *veh, int infoIdx);
void __cdecl VEH_InitVehicle(gentity_s *ent, scr_vehicle_s *veh, __int16 infoIdx);
void __cdecl VEH_SetPosition(gentity_s *ent, const float *origin, const float *vel, const float *angles);
void __cdecl VEH_InitPhysics(gentity_s *ent);
int __cdecl VEH_CorrectAllSolid(gentity_s *ent, trace_t *trace);
void __cdecl VEH_ClearGround();
bool __cdecl VEH_SlideMove(gentity_s *ent, int gravity);
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
    int index,
    float desiredAngle,
    float accel,
    float decel,
    float overShoot,
    vehicle_physic_t *phys);
double __cdecl VEH_AccelerateSpeed(float speed, float tgtSpeed, float accel, float dt);
double __cdecl VEH_UpdateMove_GetDesiredYaw(scr_vehicle_s *veh, float *desiredDir);
double __cdecl VEH_CalcAccelFraction(float accel, int infoIdx);
double __cdecl VEH_CalcAngularAccel(float accel, float accelFraction);
double __cdecl VEH_CalcAngleForAccel(float accel, float accelFraction);
double __cdecl VEH_CalcStoppingTime(float accel, float accelFraction);
void __cdecl VEH_UpdateYawAndNotify(gentity_s *ent, float desiredYaw);
double __cdecl VEH_GetAccelForAngles(scr_vehicle_s *veh);
void __cdecl VEH_AddFakeDrag(const float *velocity, float maxDragSpeed, float *accelVec);
void __cdecl VEH_CheckHorizontalVelocityToGoal(
    scr_vehicle_s *veh,
    const float *vecToGoal,
    float accelMax,
    float *accelVec);
void __cdecl VEH_CheckVerticalVelocityToGoal(scr_vehicle_s *veh, float verticalDist, float *accelVec);
int __cdecl VEH_UpdateMove_CheckGoalReached(gentity_s *ent, float distToGoal);
double __cdecl VEH_UpdateMove_CheckStop(scr_vehicle_s *veh, float distToGoal);
void __cdecl VEH_UpdateMove_CheckNearGoal(gentity_s *ent, float distToGoal);
void __cdecl VEH_GetNewSpeedAndAccel(scr_vehicle_s *veh, float dt, int hovering, float *newSpeed, float *accelMax);
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
int __cdecl VEH_GetTagBoneIndex(gentity_s *ent, int barrel);
void __cdecl VEH_SetPosition(gentity_s *ent, const float *origin, const float *angles);
void __cdecl VEH_JoltBody(gentity_s *ent, const float *dir, float intensity, float speedFrac, float decel);
void __cdecl VEH_StepSlideMove(gentity_s *ent, int gravity, float frameTime);
bool __cdecl VEH_SlideMove(gentity_s *ent, int gravity, float frameTime);
void __cdecl VEH_AirMove(gentity_s *ent, int gravity, float frameTime);




// g_weapon
struct AntilagClientStore // sizeof=0x340
{                                       // ...
    float realClientPositions[64][3];
    bool clientMoved[64];
};
struct weaponParms // sizeof=0x40
{                                       // ...
    float forward[3];                   // ...
    float right[3];                     // ...
    float up[3];                        // ...
    float muzzleTrace[3];               // ...
    float gunForward[3];                // ...
    WeaponDef *weapDef;                 // ...
};
void __cdecl G_AntiLagRewindClientPos(int gameTime, AntilagClientStore *antilagStore);
void __cdecl G_AntiLag_RestoreClientPos(AntilagClientStore *antilagStore);
gentity_s *__cdecl Weapon_Melee(gentity_s *ent, weaponParms *wp, float range, float width, float height, int gametime);
gentity_s *__cdecl Weapon_Melee_internal(gentity_s *ent, weaponParms *wp, float range, float width, float height);
char __cdecl Melee_Trace(
    gentity_s *ent,
    weaponParms *wp,
    int damage,
    float range,
    float width,
    float height,
    trace_t *trace,
    float *endPos);
gentity_s *__cdecl Weapon_Throw_Grenade(
    gentity_s *ent,
    unsigned int grenType,
    unsigned __int8 grenModel,
    weaponParms *wp);
void __cdecl Vec2NormalizeFast(float *v);
gentity_s *__cdecl Weapon_GrenadeLauncher_Fire(
    gentity_s *ent,
    unsigned int grenType,
    unsigned __int8 grenModel,
    weaponParms *wp);
gentity_s *__cdecl Weapon_RocketLauncher_Fire(
    gentity_s *ent,
    unsigned int weaponIndex,
    float spread,
    weaponParms *wp,
    const float *gunVel,
    gentity_s *target,
    const float *targetOffset);
void __cdecl gunrandom(float *x, float *y);
bool __cdecl LogAccuracyHit(gentity_s *target, gentity_s *attacker);
void __cdecl FireWeapon(gentity_s *ent, int gametime);
void __cdecl CalcMuzzlePoints(const gentity_s *ent, weaponParms *wp);
void __cdecl G_UseOffHand(gentity_s *ent);
void __cdecl FireWeaponMelee(gentity_s *ent, int gametime);
int __cdecl G_GivePlayerWeapon(playerState_s *pPS, int iWeaponIndex, unsigned __int8 altModelIndex);
void __cdecl G_SetupWeaponDef();
unsigned int __cdecl G_GetWeaponIndexForName(const char *name);
void __cdecl G_SelectWeaponIndex(int clientNum, int iWeaponIndex);
void __cdecl G_SetEquippedOffHand(int clientNum, unsigned int offHandIndex);



// g_debug
void __cdecl G_DebugLine(const float *start, const float *end, const float *color, int depthTest);
void __cdecl G_DebugLineWithDuration(
    const float *start,
    const float *end,
    const float *color,
    int depthTest,
    int duration);
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
    int depthTest,
    int duration);
void __cdecl G_DebugCircle(
    const float *center,
    float radius,
    const float *color,
    int depthTest,
    int onGround,
    int duration);
void __cdecl G_DebugCircleEx(
    const float *center,
    float radius,
    const float *dir,
    const float *color,
    int depthTest,
    int duration);



// g_svcmds
struct ipFilter_s // sizeof=0x8
{                                       // ...
    unsigned int mask;                  // ...
    unsigned int compare;               // ...
};
void __cdecl G_ProcessIPBans();
void __cdecl AddIP(char *str);
int __cdecl StringToFilter(char *s, ipFilter_s *f);
void UpdateIPBans();
void __cdecl Svcmd_AddIP_f();
void __cdecl Svcmd_RemoveIP_f();
void __cdecl Svcmd_EntityList_f();
int __cdecl ConsoleCommand();
