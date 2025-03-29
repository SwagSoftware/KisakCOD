#pragma once

#include <client_mp/client_mp.h>

enum animBodyPart_t : __int32
{                                       // ...
    ANIM_BP_UNUSED = 0x0,
    ANIM_BP_LEGS = 0x1,
    ANIM_BP_TORSO = 0x2,
    ANIM_BP_BOTH = 0x3,
    NUM_ANIM_BODYPARTS = 0x4,
};

enum aistateEnum_t : __int32
{                                       // ...
    AISTATE_COMBAT = 0x0,
    MAX_AISTATES = 0x1,
};

enum scriptAnimMoveTypes_t : __int32
{                                       // ...
    ANIM_MT_UNUSED = 0x0,
    ANIM_MT_IDLE = 0x1,
    ANIM_MT_IDLECR = 0x2,
    ANIM_MT_IDLEPRONE = 0x3,
    ANIM_MT_WALK = 0x4, // ...
    ANIM_MT_WALKBK = 0x5, // ...
    ANIM_MT_WALKCR = 0x6, // ...
    ANIM_MT_WALKCRBK = 0x7, // ...
    ANIM_MT_WALKPRONE = 0x8, // ...
    ANIM_MT_WALKPRONEBK = 0x9, // ...
    ANIM_MT_RUN = 0xA, // ...
    ANIM_MT_RUNBK = 0xB, // ...
    ANIM_MT_RUNCR = 0xC, // ...
    ANIM_MT_RUNCRBK = 0xD, // ...
    ANIM_MT_TURNRIGHT = 0xE,
    ANIM_MT_TURNLEFT = 0xF,
    ANIM_MT_TURNRIGHTCR = 0x10,
    ANIM_MT_TURNLEFTCR = 0x11,
    ANIM_MT_CLIMBUP = 0x12,
    ANIM_MT_CLIMBDOWN = 0x13,
    ANIM_MT_SPRINT = 0x14,
    ANIM_MT_MANTLE_ROOT = 0x15,
    ANIM_MT_MANTLE_UP_57 = 0x16,
    ANIM_MT_MANTLE_UP_51 = 0x17,
    ANIM_MT_MANTLE_UP_45 = 0x18,
    ANIM_MT_MANTLE_UP_39 = 0x19,
    ANIM_MT_MANTLE_UP_33 = 0x1A,
    ANIM_MT_MANTLE_UP_27 = 0x1B,
    ANIM_MT_MANTLE_UP_21 = 0x1C,
    ANIM_MT_MANTLE_OVER_HIGH = 0x1D,
    ANIM_MT_MANTLE_OVER_MID = 0x1E,
    ANIM_MT_MANTLE_OVER_LOW = 0x1F,
    ANIM_MT_FLINCH_FORWARD = 0x20,
    ANIM_MT_FLINCH_BACKWARD = 0x21,
    ANIM_MT_FLINCH_LEFT = 0x22,
    ANIM_MT_FLINCH_RIGHT = 0x23,
    ANIM_MT_STUMBLE_FORWARD = 0x24, // ...
    ANIM_MT_STUMBLE_BACKWARD = 0x25, // ...
    ANIM_MT_STUMBLE_WALK_FORWARD = 0x26, // ...
    ANIM_MT_STUMBLE_WALK_BACKWARD = 0x27, // ...
    ANIM_MT_STUMBLE_CROUCH_FORWARD = 0x28, // ...
    ANIM_MT_STUMBLE_CROUCH_BACKWARD = 0x29, // ...
    ANIM_MT_STUMBLE_SPRINT_FORWARD = 0x2A,
    NUM_ANIM_MOVETYPES = 0x2B,
};

enum scriptAnimEventTypes_t : __int32
{                                       // ...
    ANIM_ET_PAIN = 0x0,
    ANIM_ET_DEATH = 0x1,
    ANIM_ET_FIREWEAPON = 0x2,
    ANIM_ET_JUMP = 0x3,
    ANIM_ET_JUMPBK = 0x4,
    ANIM_ET_LAND = 0x5,
    ANIM_ET_DROPWEAPON = 0x6,
    ANIM_ET_RAISEWEAPON = 0x7,
    ANIM_ET_CLIMB_MOUNT = 0x8,
    ANIM_ET_CLIMB_DISMOUNT = 0x9,
    ANIM_ET_RELOAD = 0xA,
    ANIM_ET_CROUCH_TO_PRONE = 0xB,
    ANIM_ET_PRONE_TO_CROUCH = 0xC,
    ANIM_ET_STAND_TO_CROUCH = 0xD,
    ANIM_ET_CROUCH_TO_STAND = 0xE,
    ANIM_ET_STAND_TO_PRONE = 0xF,
    ANIM_ET_PRONE_TO_STAND = 0x10,
    ANIM_ET_MELEEATTACK = 0x11,
    ANIM_ET_KNIFE_MELEE = 0x12,
    ANIM_ET_KNIFE_MELEE_CHARGE = 0x13,
    ANIM_ET_SHELLSHOCK = 0x14,
    NUM_ANIM_EVENTTYPES = 0x15,
};

enum animScriptConditionTypes_t : __int32
{                                       // ...
    ANIM_CONDTYPE_BITFLAGS = 0x0,       // ...
    ANIM_CONDTYPE_VALUE = 0x1,       // ...
    NUM_ANIM_CONDTYPES = 0x2,
};

struct loadAnim_t // sizeof=0x48
{
    scr_anim_s anim;
    int iNameHash;
    char szAnimName[64];
};

struct pml_t // sizeof=0x80
{                                       // ...
    float forward[3];
    float right[3];                     // ...
    float up[3];                        // ...
    float frametime;                    // ...
    int msec;                           // ...
    int walking;                        // ...
    int groundPlane;                    // ...
    int almostGroundPlane;              // ...
    trace_t groundTrace;
    float impactSpeed;
    float previous_origin[3];           // ...
    float previous_velocity[3];         // ...
};

struct animStringItem_t // sizeof=0x8
{                                       // ...
    const char *string;                 // ...
    int hash;                           // ...
};

struct controller_info_t // sizeof=0x60
{                                       // ...
    float angles[6][3];
    float tag_origin_angles[3];         // ...
    float tag_origin_offset[3];         // ...
};

struct animConditionTable_t // sizeof=0x8
{                                       // ...
    animScriptConditionTypes_t type;    // ...
    animStringItem_t *values;           // ...
};

struct __declspec(align(8)) animScriptData_t // sizeof=0x9A9D0
{                                       // ...
    animation_s animations[512];
    unsigned int numAnimations;
    animScript_t scriptAnims[1][43];
    animScript_t scriptCannedAnims[1][43];
    animScript_t scriptStateChange[1][1];
    animScript_t scriptEvents[21];
    animScriptItem_t scriptItems[2048];
    int numScriptItems;
    scr_animtree_t animTree;            // ...
    unsigned __int16 torsoAnim;
    unsigned __int16 legsAnim;
    unsigned __int16 turningAnim;
    // padding byte
    // padding byte
    snd_alias_list_t *(__cdecl *soundAlias)(const char *);
    int(__cdecl *playSoundAlias)(int, snd_alias_list_t *);
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};


// bg_jump
void __cdecl Jump_RegisterDvars();
void __cdecl Jump_ClearState(playerState_s *ps);
char __cdecl Jump_GetStepHeight(playerState_s *ps, const float *origin, float *stepSize);
bool __cdecl Jump_IsPlayerAboveMax(playerState_s *ps);
void __cdecl Jump_ActivateSlowdown(playerState_s *ps);
void __cdecl Jump_ApplySlowdown(playerState_s *ps);
double __cdecl Jump_ReduceFriction(playerState_s *ps);
double __cdecl Jump_GetSlowdownFriction(playerState_s *ps);
void __cdecl Jump_ClampVelocity(playerState_s *ps, const float *origin);
char __cdecl Jump_Check(pmove_t *pm, pml_t *pml);
void __cdecl Jump_Start(pmove_t *pm, pml_t *pml, float height);
double __cdecl Jump_GetLandFactor(playerState_s *ps);
void __cdecl Jump_PushOffLadder(playerState_s *ps, pml_t *pml);
void __cdecl Jump_AddSurfaceEvent(playerState_s *ps, pml_t *pml);

// bg_animation_mp
void __cdecl TRACK_bg_animation_mp();
void BG_AnimParseError(const char *msg, ...);
unsigned int __cdecl BG_AnimationIndexForString(const char *string);
int __cdecl BG_StringHashValue(const char *fname);
int __cdecl BG_IndexForString(const char *token, animStringItem_t *strings, int allowFail);
void __cdecl BG_InitWeaponString(int index, const char *name);
void __cdecl BG_InitWeaponStrings();
void __cdecl BG_ParseCommands(const char **input, animScriptItem_t *scriptItem, animScriptData_t *scriptData);
int __cdecl GetValueForBitfield(unsigned int bitfield);
bool __cdecl Com_BitCheckAssert(const unsigned int *array, int bitNum, int size);
int __cdecl BG_PlayAnim(
    playerState_s *ps,
    int animNum,
    animBodyPart_t bodyPart,
    int forceDuration,
    int setTimer,
    int isContinue,
    int force);
int __cdecl BG_ExecuteCommand(
    playerState_s *ps,
    animScriptCommand_t *scriptCommand,
    int setTimer,
    int isContinue,
    int force);
int __cdecl BG_AnimScriptAnimation(playerState_s *ps, aistateEnum_t state, scriptAnimMoveTypes_t movetype, int force);
animScriptItem_t *__cdecl BG_FirstValidItem(unsigned int client, animScript_t *script);
int __cdecl BG_EvaluateConditions(clientInfo_t *ci, animScriptItem_t *scriptItem);
char *__cdecl GetMoveTypeName(int type);
char *__cdecl GetWeaponTypeName(int type);
char *__cdecl GetBodyPart(int bodypart);
int __cdecl BG_AnimScriptEvent(playerState_s *ps, scriptAnimEventTypes_t event, int isContinue, int force);
void __cdecl BG_SetConditionValue(unsigned int client, unsigned int condition, unsigned __int64 value);
const char *__cdecl BG_GetConditionString(int condition, unsigned int value);
void __cdecl BG_SetConditionBit(unsigned int client, int condition, int value);
void __cdecl Com_BitSetAssert(unsigned int *array, int bitNum, int size);
unsigned int __cdecl BG_GetConditionBit(const clientInfo_t *ci, unsigned int condition);
animScriptData_t *__cdecl BG_GetAnimationForIndex(int client, unsigned int index);
void __cdecl BG_AnimUpdatePlayerStateConditions(pmove_t *pmove);
bool __cdecl BG_IsCrouchingAnim(const clientInfo_t *ci, int animNum);
bool __cdecl BG_IsAds(const clientInfo_t *ci, int animNum);
bool __cdecl BG_IsProneAnim(const clientInfo_t *ci, int animNum);
bool __cdecl BG_IsKnifeMeleeAnim(const clientInfo_t *ci, int animNum);
void __cdecl BG_LerpOffset(float *offset_goal, float maxOffsetChange, float *offset);
double __cdecl I_rsqrt(int number);
void __cdecl Vec3Mad(const float *start, float scale, const float *dir, float *result);
void __cdecl BG_Player_DoControllersSetup(const entityState_s *es, clientInfo_t *ci, int frametime);
void __cdecl BG_Player_DoControllersInternal(const entityState_s *es, const clientInfo_t *ci, controller_info_t *info);
unsigned int __cdecl BG_GetConditionValue(const clientInfo_t *ci, unsigned int condition);
void __cdecl BG_LerpAngles(float *angles_goal, float maxAngleChange, float *angles);
void __cdecl BG_PlayerAnimation(int localClientNum, const entityState_s *es, clientInfo_t *ci);
void __cdecl BG_RunLerpFrameRate(
    int localClientNum,
    clientInfo_t *ci,
    lerpFrame_t *lf,
    int newAnimation,
    const entityState_s *es);
void __cdecl BG_SetNewAnimation(
    int localClientNum,
    clientInfo_t *ci,
    lerpFrame_t *lf,
    int newAnimation,
    const entityState_s *es);
void __cdecl BG_PlayerAnimation_VerifyAnim(XAnimTree_s *pAnimTree, lerpFrame_t *lf);
void __cdecl BG_PlayerAngles(const entityState_s *es, clientInfo_t *ci);
void __cdecl BG_SwingAngles(
    float destination,
    float swingTolerance,
    float clampTolerance,
    float speed,
    float *angle,
    int *swinging);
void __cdecl BG_AnimPlayerConditions(const entityState_s *es, clientInfo_t *ci);
void __cdecl BG_UpdatePlayerDObj(
    int localClientNum,
    DObj_s *pDObj,
    entityState_s *es,
    clientInfo_t *ci,
    int attachIgnoreCollision);
void __cdecl BG_LoadAnim();
void BG_FinalizePlayerAnims();
loadAnim_t *__cdecl BG_LoadAnimForAnimIndex(unsigned int iAnimIndex);
void __cdecl BG_SetupAnimNoteTypes(animScriptData_t *scriptData);
void __cdecl BG_AnimParseAnimScript(animScriptData_t *scriptData, loadAnim_t *pLoadAnims, unsigned int *piNumAnims);
char *__cdecl BG_CopyStringIntoBuffer(const char *string, char *buffer, unsigned int bufSize, unsigned int *offset);
void __cdecl BG_ParseConditionBits(
    const char **text_pp,
    animStringItem_t *stringTable,
    int condIndex,
    unsigned int *result);
int __cdecl BG_ParseConditions(const char **text_pp, animScriptItem_t *scriptItem);
void BG_FindAnims();
bgs_t *BG_FindAnimTrees();
scr_animtree_t __cdecl BG_FindAnimTree(const char *filename, int bEnforceExists);
void __cdecl strstr(unsigned __int8 *str1, unsigned __int8 *str2);

extern bgs_t *bgs;

// bg_misc
enum proneCheckType_t : __int32
{                                       // ...
    PCT_CLIENT = 0x0,
    PCT_ACTOR = 0x1,
};
enum itemType_t : __int32
{                                       // ...
    IT_BAD = 0x0,
    IT_WEAPON = 0x1,
};
struct gitem_s // sizeof=0x4
{
    itemType_t giType;
};
void __cdecl BG_RegisterShockVolumeDvars();
void __cdecl BG_RegisterDvars();
char *__cdecl BG_GetEntityTypeName(int eType);
const gitem_s *__cdecl BG_FindItemForWeapon(unsigned int weapon, int model);
const gitem_s *__cdecl G_FindItem(const char *pickupName, int model);
bool __cdecl BG_PlayerTouchesItem(const playerState_s *ps, const entityState_s *item, int atTime);
bool __cdecl BG_PlayerCanPickUpWeaponType(const WeaponDef *weapDef, const playerState_s *ps);
bool __cdecl BG_CanItemBeGrabbed(const entityState_s *ent, const playerState_s *ps, int touched);
char __cdecl WeaponEntCanBeGrabbed(
    const entityState_s *weaponEntState,
    const playerState_s *ps,
    int touched,
    unsigned int weapIdx);
char __cdecl HaveRoomForAmmo(const playerState_s *ps, unsigned int weaponIndex);
bool __cdecl BG_PlayerHasRoomForEntAllAmmoTypes(const entityState_s *ent, const playerState_s *ps);
void __cdecl BG_EvaluateTrajectory(const trajectory_t *tr, int atTime, float *result);
void __cdecl BG_EvaluateTrajectoryDelta(const trajectory_t *tr, int atTime, float *result);
void __cdecl BG_AddPredictableEventToPlayerstate(unsigned int newEvent, unsigned int eventParm, playerState_s *ps);
void __cdecl BG_PlayerStateToEntityState(playerState_s *ps, entityState_s *s, int snap, unsigned __int8 handler);
void __cdecl BG_PlayerToEntityEventParm(playerState_s *ps, entityState_s *s);
void __cdecl BG_PlayerToEntityProcessEvents(playerState_s *ps, entityState_s *s, unsigned __int8 handler);
void __cdecl BG_PlayerToEntitySetFlags(playerState_s *ps, entityState_s *s);
void __cdecl BG_PlayerToEntitySetPitchAngles(playerState_s *ps, entityState_s *s);
void __cdecl BG_PlayerToEntitySetMisc(playerState_s *ps, entityState_s *s);
void __cdecl BG_PlayerToEntitySetTrajectory(playerState_s *ps, entityState_s *s, int snap);
char __cdecl BG_CheckProneValid(
    int passEntityNum,
    const float *vPos,
    float fSize,
    float fHeight,
    float fYaw,
    float *pfTorsoPitch,
    float *pfWaistPitch,
    bool isAlreadyProne,
    bool isOnGround,
    bool groundIsWalkable,
    unsigned __int8 handler,
    proneCheckType_t proneCheckType,
    float prone_feet_dist);
void __cdecl BG_GetPlayerViewOrigin(const playerState_s *ps, float *origin, int time);
void __cdecl BG_GetPlayerViewDirection(const playerState_s *ps, float *forward, float *right, float *up);
char __cdecl BG_CheckProne(
    int passEntityNum,
    const float *vPos,
    float fSize,
    float fHeight,
    float fYaw,
    float *pfTorsoPitch,
    float *pfWaistPitch,
    bool isAlreadyProne,
    bool isOnGround,
    bool groundIsWalkable,
    unsigned __int8 handler,
    proneCheckType_t proneCheckType,
    float prone_feet_dist);
void __cdecl BG_LerpHudColors(const hudelem_s *elem, int time, hudelem_color_t *toColor);
int __cdecl BG_LoadShellShockDvars(const char *name);
void __cdecl BG_SetShellShockParmsFromDvars(shellshock_parms_t *parms);
int __cdecl BG_SaveShellShockDvars(const char *name);
shellshock_parms_t *__cdecl BG_GetShellshockParms(unsigned int index);
void __cdecl BG_CreateXAnim(XAnim_s *anims, unsigned int animIndex, char *name);
void __cdecl BG_CheckThread();
int __cdecl BG_GetMaxSprintTime(const playerState_s *ps);

extern const dvar_t *player_footstepsThreshhold;
extern const dvar_t *player_debugHealth;
extern const dvar_t *bg_shock_lookControl_mousesensitivityscale;
extern const dvar_t *bg_shock_soundEnd;
extern const dvar_t *player_view_pitch_down;
extern const dvar_t *player_runbkThreshhold;
extern const dvar_t *player_sprintForwardMinimum;
extern const dvar_t *player_turnAnims;
extern const dvar_t *stopspeed;
extern const dvar_t *bg_shock_soundEndAbort;
extern const dvar_t *bullet_penetrationMinFxDist;
extern const dvar_t *player_view_pitch_up;
extern const dvar_t *bg_maxGrenadeIndicatorSpeed;
extern const dvar_t *player_meleeHeight;
extern const dvar_t *bg_foliagesnd_minspeed;
extern const dvar_t *bg_shock_screenBlurBlendTime;
extern const dvar_t *player_dmgtimer_flinchTime;
extern const dvar_t *player_move_factor_on_torso;
extern const dvar_t *bg_shock_screenBlurBlendFadeTime;
extern const dvar_t *animscript_debug;
extern const dvar_t *player_adsExitDelay;
extern const dvar_t *bg_shock_soundDryLevel;
extern const dvar_t *bg_swingSpeed;
extern const dvar_t *bg_shock_movement;
extern const dvar_t *bg_shock_volume;
extern const dvar_t *bg_aimSpreadMoveSpeedThreshold;
extern const dvar_t *bg_shock_lookControl;
extern const dvar_t *player_breath_snd_lerp;
extern const dvar_t *player_breath_gasp_scale;
extern const dvar_t *bg_shock_soundLoopFadeTime;
extern const dvar_t *player_sprintThreshhold;
extern const dvar_t *bg_shock_screenType;
extern const dvar_t *player_meleeRange;
extern const dvar_t *bg_shock_viewKickFadeTime;
extern const dvar_t *bg_shock_lookControl_fadeTime;
extern const dvar_t *player_strafeAnimCosAngle;
extern const dvar_t *player_moveThreshhold;
extern const dvar_t *player_dmgtimer_minScale;
extern const dvar_t *player_sprintMinTime;
extern const dvar_t *bg_viewKickMin;
extern const dvar_t *bg_foliagesnd_fastinterval;
extern const dvar_t *bg_shock_soundLoopSilent;
extern const dvar_t *player_breath_snd_delay;
extern const dvar_t *inertiaDebug;
extern const dvar_t *bg_fallDamageMaxHeight;
extern const dvar_t *player_runThreshhold;
extern const dvar_t *bg_shock_soundFadeInTime;
extern const dvar_t *player_spectateSpeedScale;
extern const dvar_t *bg_shock_soundLoopEndDelay;
extern const dvar_t *player_dmgtimer_timePerPoint;
extern const dvar_t *bg_prone_yawcap;
extern const dvar_t *friction;
extern const dvar_t *bg_bobAmplitudeSprinting;
extern const dvar_t *inertiaMax;
extern const dvar_t *bg_shock_soundFadeOutTime;
extern const dvar_t *player_scopeExitOnDamage;
extern const dvar_t *player_dmgtimer_stumbleTime;
extern const dvar_t *bg_foliagesnd_resetinterval;
extern const dvar_t *bg_shock_lookControl_maxyawspeed;
extern const dvar_t *player_backSpeedScale;
extern const dvar_t *player_breath_fire_delay;
extern const dvar_t *bg_foliagesnd_slowinterval;
extern const dvar_t *bg_viewKickScale;
extern const dvar_t *bg_shock_soundWetLevel;
extern const dvar_t *player_breath_hold_lerp;
extern const dvar_t *inertiaAngle;
extern const dvar_t *player_dmgtimer_maxTime;
extern const dvar_t *bg_bobMax;
extern const dvar_t *player_burstFireCooldown;
extern const dvar_t *bg_shock_screenFlashWhiteFadeTime;
extern const dvar_t *player_breath_gasp_time;
extern const dvar_t *bg_shock_soundLoop;
extern const dvar_t *bg_shock_viewKickPeriod;
extern const dvar_t *bg_bobAmplitudeProne;
extern const dvar_t *player_meleeChargeFriction;
extern const dvar_t *player_sprintSpeedScale;
extern const dvar_t *xanim_debug;
extern const dvar_t *bg_shock_sound;
extern const dvar_t *player_meleeWidth;
extern const dvar_t *player_sprintRechargePause;
extern const dvar_t *bg_legYawTolerance;
extern const dvar_t *bg_shock_lookControl_maxpitchspeed;
extern const dvar_t *bg_shock_viewKickRadius;
extern const dvar_t *player_breath_gasp_lerp;
extern const dvar_t *player_sprintStrafeSpeedScale;
extern const dvar_t *player_sprintTime;
extern const dvar_t *bg_fallDamageMinHeight;
extern const dvar_t *bg_bobAmplitudeDucked;
extern const dvar_t *player_strafeSpeedScale;
extern const dvar_t *bg_shock_soundRoomType;
extern const dvar_t *player_breath_hold_time;
extern const dvar_t *anim_debugSpeeds;
extern const dvar_t *bg_ladder_yawcap;
extern const dvar_t *bg_shock_screenFlashShotFadeTime;
extern const dvar_t *bg_shock_soundModEndDelay;
extern const dvar_t *bg_viewKickRandom;
extern const dvar_t *bg_bobAmplitudeStanding;
extern const dvar_t *bg_viewKickMax;
extern const dvar_t *bg_foliagesnd_maxspeed;
extern const dvar_t *player_sprintCameraBob;
extern const dvar_t *player_sustainAmmo;

extern const dvar_t *player_lean_shift_left;
extern const dvar_t *player_lean_shift_right;
extern const dvar_t *player_lean_shift_crouch_left;
extern const dvar_t *player_lean_shift_crouch_right;
extern const dvar_t *player_lean_rotate_left;
extern const dvar_t *player_lean_rotate_right;
extern const dvar_t *player_lean_rotate_crouch_left;
extern const dvar_t *player_lean_rotate_crouch_right;


// bg_perks_mp
unsigned int __cdecl BG_GetPerkIndexForName(const char *perkName);
void __cdecl Perks_RegisterDvars();

extern const dvar_t *perk_parabolicIcon;
extern const dvar_t *perk_parabolicRadius;
extern const dvar_t *perk_parabolicAngle;
extern const dvar_t *perk_bulletPenetrationMultiplier;
extern const dvar_t *perk_weapSpreadMultiplier;
extern const dvar_t *perk_extraBreath;
extern const dvar_t *perk_grenadeDeath;
extern const dvar_t *perk_weapReloadMultiplier;
extern const dvar_t *perk_weapRateMultiplier;
extern const dvar_t *perk_sprintMultiplier;

// bg_pmove
enum PmStanceFrontBack : __int32
{                                       // ...
    PM_STANCE_STAND = 0x0,
    PM_STANCE_PRONE = 0x1,
    PM_STANCE_CROUCH = 0x2,
    PM_STANCE_BACKWARD_FIRST = 0x3,
    PM_STANCE_BACKWARD_RUN = 0x3,
    PM_STANCE_BACKWARD_PRONE = 0x4,
    PM_STANCE_BACKWARD_CROUCH = 0x5,
    NUM_PM_STANCE_FRONTBACK = 0x6,
};
struct viewLerpWaypoint_s // sizeof=0xC
{                                       // ...
    int iFrac;
    float fViewHeight;
    int iOffset;
};
void __cdecl PM_trace(
    pmove_t *pm,
    trace_t *results,
    const float *start,
    const float *mins,
    const float *maxs,
    const float *end,
    int passEntityNum,
    int contentMask);
void __cdecl PM_playerTrace(
    pmove_t *pm,
    trace_t *results,
    const float *start,
    const float *mins,
    const float *maxs,
    const float *end,
    int passEntityNum,
    int contentMask);
void __cdecl PM_AddEvent(playerState_s *ps, unsigned int newEvent);
void __cdecl PM_AddTouchEnt(pmove_t *pm, int entityNum);
void __cdecl PM_ClipVelocity(const float *in, const float *normal, float *out);
void __cdecl PM_ProjectVelocity(const float *velIn, const float *normal, float *velOut);
int __cdecl PM_GetEffectiveStance(const playerState_s *ps);
int __cdecl PM_GetSprintLeft(const playerState_s *ps, int gametime);
int __cdecl PM_GetSprintLeftLastTime(const playerState_s *ps);
bool __cdecl PM_IsSprinting(const playerState_s *ps);
double __cdecl PM_DamageScale_Walk(int damage_timer);
unsigned int __cdecl PM_GroundSurfaceType(pml_t *pml);
int __cdecl PM_GetViewHeightLerpTime(const playerState_s *ps, int iTarget, int bDown);
bool __cdecl PlayerProneAllowed(pmove_t *pm);
void __cdecl PM_FootstepEvent(pmove_t *pm, pml_t *pml, char iOldBobCycle, char iNewBobCycle, int bFootStep);
int __cdecl PM_FootstepType(playerState_s *ps, pml_t *pml);
bool __cdecl PM_ShouldMakeFootsteps(pmove_t *pm);
void __cdecl PM_UpdateLean(
    playerState_s *ps,
    float msec,
    usercmd_s *cmd,
    void(__cdecl *capsuleTrace)(trace_t *, float *, float *, float *, float *, int));
void __cdecl PM_UpdateViewAngles(playerState_s *ps, float msec, usercmd_s *cmd, unsigned __int8 handler);
void __cdecl PM_UpdateViewAngles_Clamp(playerState_s *ps, usercmd_s *cmd);
void __cdecl PM_UpdateViewAngles_RangeLimited(playerState_s *ps, float oldYaw);
void __cdecl PM_UpdateViewAngles_LadderClamp(playerState_s *ps);
void __cdecl PM_UpdateViewAngles_Prone(
    playerState_s *ps,
    float msec,
    usercmd_s *cmd,
    unsigned __int8 handler,
    float oldViewYaw);
int __cdecl BG_CheckProneTurned(playerState_s *ps, float newProneYaw, unsigned __int8 handler);
void __cdecl PM_UpdateViewAngles_ProneYawClamp(
    playerState_s *ps,
    float delta,
    int proneBlocked,
    float oldViewYaw,
    float newViewYaw);
void __cdecl PM_UpdateViewAngles_PronePitchClamp(playerState_s *ps);
void __cdecl PM_UpdatePronePitch(pmove_t *pm, pml_t *pml);
void __cdecl PM_SetProneMovementOverride(playerState_s *ps);
void __cdecl PM_MeleeChargeStart(pmove_t *pm);
void __cdecl PM_MeleeChargeClear(playerState_s *ps);
void __cdecl Pmove(pmove_t *pm);
void __cdecl PmoveSingle(pmove_t *pm);
void __cdecl PM_UpdateSprint(pmove_t *pm, const pml_t *pml);
void __cdecl PM_StartSprint(playerState_s *ps, pmove_t *pm, const pml_t *pml, int sprintLeft);
void __cdecl PM_EndSprint(playerState_s *ps, pmove_t *pm);
bool __cdecl PM_SprintStartInterferingButtons(const playerState_s *ps, int forwardSpeed, __int16 buttons);
bool __cdecl PM_SprintEndingButtons(const playerState_s *ps, int forwardSpeed, __int16 buttons);
bool __cdecl PM_CanStand(playerState_s *ps, pmove_t *pm);
void __cdecl PM_FlyMove(pmove_t *pm, pml_t *pml);
void __cdecl Vec3Cross(const float *v0, const float *v1, float *cross);
void __cdecl PM_Friction(playerState_s *ps, pml_t *pml);
void __cdecl PM_Accelerate(playerState_s *ps, const pml_t *pml, const float *wishdir, float wishspeed, float accel);
double __cdecl PM_PlayerInertia(const playerState_s *ps, float accelspeed, const float *wishdir);
char __cdecl PM_DoPlayerInertia(const playerState_s *ps, float accelspeed, const float *wishdir);
double __cdecl PM_MoveScale(playerState_s *ps, float fmove, float rmove, float umove);
double __cdecl PM_CmdScale(playerState_s *ps, usercmd_s *cmd);
void __cdecl PM_AirMove(pmove_t *pm, pml_t *pml);
void __cdecl PM_SetMovementDir(pmove_t *pm, pml_t *pml);
void __cdecl PM_WalkMove(pmove_t *pm, pml_t *pml);
double __cdecl PM_CmdScale_Walk(pmove_t *pm, usercmd_s *cmd);
double __cdecl PM_CmdScaleForStance(const pmove_t *pm);
void __cdecl PM_DeadMove(playerState_s *ps, pml_t *pml);
void __cdecl PM_NoclipMove(pmove_t *pm, pml_t *pml);
void __cdecl PM_UFOMove(pmove_t *pm, pml_t *pml);
void __cdecl PM_GroundTrace(pmove_t *pm, pml_t *pml);
void __cdecl PM_CrashLand(playerState_s *ps, pml_t *pml);
int __cdecl PM_LightLandingForSurface(pml_t *pml);
int __cdecl PM_MediumLandingForSurface(pml_t *pml);
unsigned int __cdecl PM_HardLandingForSurface(pml_t *pml);
unsigned int __cdecl PM_DamageLandingForSurface(pml_t *pml);
int __cdecl PM_CorrectAllSolid(pmove_t *pm, pml_t *pml, trace_t *trace);
void __cdecl PM_GroundTraceMissed(pmove_t *pm, pml_t *pml);
double __cdecl PM_GetViewHeightLerp(const pmove_t *pm, int iFromHeight, int iToHeight);
bool __cdecl PM_IsPlayerFrozenByWeapon(const playerState_s *ps);
void __cdecl PM_CheckDuck(pmove_t *pm, pml_t *pml);
void __cdecl PM_ViewHeightAdjust(pmove_t *pm, pml_t *pml);
double __cdecl PM_ViewHeightTableLerp(int iFrac, viewLerpWaypoint_s *pTable, float *pfPosOfs);
void __cdecl PM_Footsteps(pmove_t *pm, pml_t *pml);
double __cdecl Vec2Length(const float *v);
int __cdecl PM_GetStanceEx(int stance, int backward);
void __cdecl PM_Footstep_LadderMove(pmove_t *pm, pml_t *pml);
void __cdecl PM_Footsteps_NotMoving(pmove_t *pm, int stance);
unsigned int __cdecl PM_GetFlinchAnim(unsigned int flinchAnimDir);
int __cdecl PM_Footsteps_TurnAnim(clientInfo_t *ci);
scriptAnimMoveTypes_t __cdecl PM_GetNotMovingAnim(int stance, int turnAdjust);
bool __cdecl PM_ShouldFlinch(playerState_s *ps);
double __cdecl PM_GetMaxSpeed(pmove_t *pm, int walking, int sprinting);
double __cdecl PM_GetBobMove(PmStanceFrontBack stance, float xyspeed, float fMaxSpeed, int walking, int sprinting);
int __cdecl PM_GetStanceIdleAnim(char stanceFlag);
int __cdecl PM_GetMoveAnim(playerState_s *ps, PmStanceFrontBack stance, int walking, int sprinting);
void __cdecl PM_SetStrafeCondition(pmove_t *pm);
void __cdecl PM_Footstep_NotTryingToMove(pmove_t *pm);
void __cdecl PM_FoliageSounds(pmove_t *pm);
void __cdecl PM_DropTimers(playerState_s *ps, pml_t *pml);
void __cdecl PM_UpdatePlayerWalkingFlag(pmove_t *pm);
void __cdecl PM_ClearLadderFlag(playerState_s *ps);
void __cdecl PM_CheckLadderMove(pmove_t *pm, pml_t *pml);
void __cdecl PM_SetLadderFlag(playerState_s *ps);
void __cdecl PM_LadderMove(pmove_t *pm, pml_t *pml);
void __cdecl PM_MeleeChargeUpdate(pmove_t *pm, pml_t *pml);
void __cdecl TurretNVGTrigger(pmove_t *pm);
double __cdecl BG_GetSpeed(const playerState_s *ps, int time);


// bg_mantle
struct MantleResults // sizeof=0x38
{                                       // ...
    float dir[3];                       // ...
    float startPos[3];                  // ...
    float ledgePos[3];
    float endPos[3];
    int flags;                          // ...
    int duration;
};
struct MantleAnimTransition // sizeof=0xC
{                                       // ...
    int upAnimIndex;                    // ...
    int overAnimIndex;                  // ...
    float height;                       // ...
};
void __cdecl Mantle_RegisterDvars();
void __cdecl Mantle_CreateAnims(void *(__cdecl *xanimAlloc)(int));
void __cdecl Mantle_ShutdownAnims();
void __cdecl Mantle_Check(pmove_t *pm, pml_t *pml);
void __cdecl Mantle_DebugPrint(const char *msg);
char __cdecl Mantle_CheckLedge(pmove_t *pm, pml_t *pml, MantleResults *mresults, float height);
void __cdecl Mantle_CalcEndPos(pmove_t *pm, MantleResults *mresults);
void __cdecl Mantle_Start(pmove_t *pm, playerState_s *ps, MantleResults *mresults);
int __cdecl Mantle_GetUpLength(MantleState *mstate);
int __cdecl Mantle_GetOverLength(MantleState *mstate);
void __cdecl Mantle_GetAnimDelta(MantleState *mstate, int time, float *delta);
int __cdecl Mantle_FindTransition(float curHeight, float goalHeight);
char __cdecl Mantle_FindMantleSurface(pmove_t *pm, pml_t *pml, trace_t *trace, float *mantleDir);
void __cdecl Mantle_Move(pmove_t *pm, playerState_s *ps, pml_t *pml);
int __cdecl Mantle_GetAnim(MantleState *mstate);
void __cdecl Mantle_CapView(playerState_s *ps);
void __cdecl Mantle_ClearHint(playerState_s *ps);
bool __cdecl Mantle_IsWeaponInactive(playerState_s *ps);
void __cdecl Vec3Negate(const float *from, float *to);





// bg_weapons
enum weaponstate_t : __int32
{                                       // ...
    WEAPON_READY = 0x0,
    WEAPON_RAISING = 0x1,
    WEAPON_RAISING_ALTSWITCH = 0x2,
    WEAPON_DROPPING = 0x3,
    WEAPON_DROPPING_QUICK = 0x4,
    WEAPON_FIRING = 0x5,
    WEAPON_RECHAMBERING = 0x6,
    WEAPON_RELOADING = 0x7,
    WEAPON_RELOADING_INTERUPT = 0x8,
    WEAPON_RELOAD_START = 0x9,
    WEAPON_RELOAD_START_INTERUPT = 0xA,
    WEAPON_RELOAD_END = 0xB,
    WEAPON_MELEE_INIT = 0xC,
    WEAPON_MELEE_FIRE = 0xD,
    WEAPON_MELEE_END = 0xE,
    WEAPON_OFFHAND_INIT = 0xF,
    WEAPON_OFFHAND_PREPARE = 0x10,
    WEAPON_OFFHAND_HOLD = 0x11,
    WEAPON_OFFHAND_START = 0x12,
    WEAPON_OFFHAND = 0x13,
    WEAPON_OFFHAND_END = 0x14,
    WEAPON_DETONATING = 0x15,
    WEAPON_SPRINT_RAISE = 0x16,
    WEAPON_SPRINT_LOOP = 0x17,
    WEAPON_SPRINT_DROP = 0x18,
    WEAPON_NIGHTVISION_WEAR = 0x19,
    WEAPON_NIGHTVISION_REMOVE = 0x1A,
    WEAPONSTATES_NUM = 0x1B,
};
struct BulletFireParams // sizeof=0x40
{                                       // ...
    int weaponEntIndex;                 // ...
    int ignoreEntIndex;                 // ...
    float damageMultiplier;             // ...
    int methodOfDeath;                  // ...
    float origStart[3];                 // ...
    float start[3];                     // ...
    float end[3];                       // ...
    float dir[3];                       // ...
};
struct BulletTraceResults // sizeof=0x44
{                                       // ...
    trace_t trace;                      // ...
    gentity_s *hitEnt;                  // ...
    float hitPos[3];                    // ...
    bool ignoreHitEnt;                  // ...
    // padding byte
    // padding byte
    // padding byte
    int depthSurfaceType;               // ...
};
struct viewState_t // sizeof=0x24
{                                       // ...
    playerState_s *ps;                  // ...
    int damageTime;                     // ...
    int time;                           // ...
    float v_dmg_pitch;                  // ...
    float v_dmg_roll;                   // ...
    float xyspeed;                      // ...
    float frametime;                    // ...
    float fLastIdleFactor;              // ...
    int *weapIdleTime;                  // ...
};
struct weaponState_t // sizeof=0x54
{                                       // ...
    const playerState_s *ps;            // ...
    float xyspeed;                      // ...
    float frametime;                    // ...
    float vLastMoveAng[3];              // ...
    float fLastIdleFactor;              // ...
    int time;                           // ...
    int damageTime;                     // ...
    float v_dmg_pitch;                  // ...
    float v_dmg_roll;                   // ...
    float vGunOffset[3];                // ...
    float vGunSpeed[3];                 // ...
    float swayAngles[3];                // ...
    int *weapIdleTime;                  // ...
};
void __cdecl TRACK_bg_weapons();
void __cdecl BG_LoadPenetrationDepthTable();
void __cdecl BG_ParsePenetrationDepthTable(const char *penetrateType, float *depthTable, char *buffer);
char __cdecl BG_AdvanceTrace(BulletFireParams *bp, BulletTraceResults *br, float dist);
double __cdecl BG_GetSurfacePenetrationDepth(const WeaponDef *weapDef, unsigned int surfaceType);
void __cdecl BG_ShutdownWeaponDefFiles();
void __cdecl BG_ClearWeaponDef();
void __cdecl BG_FillInAllWeaponItems();
void __cdecl BG_SetupWeaponIndex(unsigned int weapIndex);
void __cdecl BG_FillInWeaponItems(unsigned int weapIndex);
void __cdecl BG_SetupAmmoIndexes(unsigned int weapIndex);
void __cdecl BG_SetupSharedAmmoIndexes(unsigned int weapIndex);
void __cdecl BG_SetupClipIndexes(unsigned int weapIndex);
void __cdecl PM_StartWeaponAnim(playerState_s *ps, int anim);
WeaponDef *__cdecl BG_GetWeaponDef(unsigned int weaponIndex);
unsigned int __cdecl BG_GetWeaponIndex(const WeaponDef *weapDef);
unsigned int __cdecl BG_GetNumWeapons();
int __cdecl BG_GetSharedAmmoCapSize(unsigned int capIndex);
unsigned int __cdecl BG_FindWeaponIndexForName(const char *name);
unsigned int __cdecl BG_GetWeaponIndexForName(const char *name, void(__cdecl *regWeap)(unsigned int));
unsigned int __cdecl BG_SetupWeaponDef(WeaponDef *weapDef, void(__cdecl *regWeap)(unsigned int));
void __cdecl BG_SetupWeaponAlts(unsigned int weapIndex, void(__cdecl *regWeap)(unsigned int));
unsigned int __cdecl BG_GetViewmodelWeaponIndex(const playerState_s *ps);
int __cdecl BG_GetFirstAvailableOffhand(const playerState_s *ps, int offhandClass);
int __cdecl BG_GetFirstEquippedOffhand(const playerState_s *ps, int offhandClass);
int __cdecl BG_IsAimDownSightWeapon(unsigned int weaponIndex);
bool __cdecl BG_CanPlayerHaveWeapon(unsigned int weaponIndex);
bool __cdecl BG_ValidateWeaponNumber(unsigned int weaponIndex);
bool __cdecl BG_IsWeaponValid(const playerState_s *ps, unsigned int weaponIndex);
bool __cdecl BG_WeaponBlocksProne(unsigned int weapIndex);
int __cdecl BG_TakePlayerWeapon(playerState_s *ps, unsigned int weaponIndex, int takeAwayAmmo);
void __cdecl Com_BitClearAssert(unsigned int *array, int bitNum, int size);
int __cdecl AmmoAfterWeaponRemoved(const playerState_s *ps, unsigned int weaponIndex);
int __cdecl BG_GetAmmoPlayerMax(const playerState_s *ps, unsigned int weaponIndex, unsigned int weaponIndexToSkip);
int __cdecl BG_GetMaxPickupableAmmo(const playerState_s *ps, unsigned int weaponIndex);
int __cdecl BG_GetTotalAmmoReserve(const playerState_s *ps, unsigned int weaponIndex);
void __cdecl BG_GetSpreadForWeapon(
    const playerState_s *ps,
    const WeaponDef *weapDef,
    float *minSpread,
    float *maxSpread);
void __cdecl PM_UpdateAimDownSightFlag(pmove_t *pm, pml_t *pml);
bool __cdecl PM_IsAdsAllowed(playerState_s *ps, pml_t *pml);
void __cdecl PM_ExitAimDownSight(playerState_s *ps);
void __cdecl PM_UpdateAimDownSightLerp(pmove_t *pm, pml_t *pml);
bool __cdecl BG_UsingSniperScope(playerState_s *ps);
int __cdecl PM_InteruptWeaponWithProneMove(playerState_s *ps);
int __cdecl BG_ClipForWeapon(unsigned int weapon);
int __cdecl BG_AmmoForWeapon(unsigned int weapon);
int __cdecl BG_WeaponIsClipOnly(unsigned int weapon);
int __cdecl BG_WeaponAmmo(const playerState_s *ps, unsigned int weapon);
int __cdecl PM_WeaponAmmoAvailable(playerState_s *ps);
void __cdecl PM_AdjustAimSpreadScale(pmove_t *pm, pml_t *pml);
bool __cdecl ShotLimitReached(playerState_s *ps, WeaponDef *weapDef);
int __cdecl PM_GetWeaponFireButton(unsigned int weapon);
void __cdecl PM_Weapon_Idle(playerState_s *ps);
void __cdecl PM_Weapon(pmove_t *pm, pml_t *pml);
void __cdecl PM_UpdateHoldBreath(pmove_t *pm, pml_t *pml);
void __cdecl PM_StartHoldBreath(playerState_s *ps);
void __cdecl PM_EndHoldBreath(playerState_s *ps);
int __cdecl PM_Weapon_CheckForRechamber(playerState_s *ps, int delayedAction);
void __cdecl PM_Weapon_FinishRechamber(playerState_s *ps);
void __cdecl PM_ContinueWeaponAnim(playerState_s *ps, int anim);
void __cdecl PM_Weapon_FinishWeaponChange(pmove_t *pm, bool quick);
bool __cdecl PM_WeaponClipEmpty(playerState_s *ps);
void __cdecl PM_Weapon_BeginWeaponRaise(
    playerState_s *ps,
    unsigned int anim,
    unsigned int time,
    float aim,
    int altSwitch);
void __cdecl BG_TakeClipOnlyWeaponIfEmpty(playerState_s *ps, int weaponIndex);
void __cdecl PM_Weapon_FinishWeaponRaise(playerState_s *ps);
void __cdecl PM_Weapon_FinishReloadStart(pmove_t *pm, int delayedAction);
void __cdecl PM_SetReloadingState(playerState_s *ps);
void __cdecl PM_SetWeaponReloadAddAmmoDelay(playerState_s *ps);
int __cdecl PM_Weapon_AllowReload(playerState_s *ps);
void __cdecl PM_Weapon_ReloadDelayedAction(playerState_s *ps);
void __cdecl PM_ReloadClip(playerState_s *ps);
void __cdecl PM_Weapon_FinishReload(pmove_t *pm, int delayedAction);
void __cdecl PM_Weapon_FinishReloadEnd(playerState_s *ps);
void __cdecl PM_Weapon_CheckForReload(pmove_t *pm);
void __cdecl PM_BeginWeaponReload(playerState_s *ps);
bool __cdecl BurstFirePending(playerState_s *ps);
void __cdecl UpdatePendingTriggerPull(pmove_t *pm);
int __cdecl PM_Weapon_WeaponTimeAdjust(pmove_t *pm, pml_t *pml);
bool __cdecl WeaponUsesBurstCooldown(unsigned int weaponIdx);
void __cdecl PM_Weapon_CheckForChangeWeapon(pmove_t *pm);
void __cdecl PM_BeginWeaponChange(playerState_s *ps, unsigned int newweapon, bool quick);
int __cdecl PM_Weapon_ShouldBeFiring(pmove_t *pm, int delayedAction);
void __cdecl PM_Weapon_FireWeapon(playerState_s *ps, int delayedAction);
void __cdecl PM_HoldBreathFire(playerState_s *ps);
void __cdecl PM_WeaponUseAmmo(playerState_s *ps, unsigned int wp, int amount);
void __cdecl BG_SwitchWeaponsIfEmpty(playerState_s *ps);
void __cdecl PM_Weapon_StartFiring(playerState_s *ps, int delayedAction);
int __cdecl PM_Weapon_CheckFiringAmmo(playerState_s *ps);
void __cdecl PM_Weapon_SetFPSFireAnim(playerState_s *ps);
void __cdecl PM_Weapon_AddFiringAimSpreadScale(playerState_s *ps);
void __cdecl PM_Weapon_MeleeEnd(playerState_s *ps);
void __cdecl PM_Weapon_MeleeFire(playerState_s *ps);
void __cdecl PM_Weapon_CheckForMelee(pmove_t *pm, int delayedAction);
void __cdecl PM_Weapon_MeleeInit(playerState_s *ps);
bool __cdecl PM_WeaponHasChargeMelee(playerState_s *ps);
void __cdecl PM_Weapon_OffHandPrepare(playerState_s *ps);
void __cdecl PM_Weapon_OffHandHold(playerState_s *ps);
void __cdecl PM_Weapon_OffHandStart(pmove_t *pm);
void __cdecl PM_Weapon_OffHand(pmove_t *pm);
void __cdecl PM_Weapon_OffHandEnd(playerState_s *ps);
void __cdecl PM_Weapon_CheckForOffHand(pmove_t *pm);
void __cdecl PM_Weapon_OffHandInit(playerState_s *ps);
void __cdecl PM_SendEmtpyOffhandEvent(playerState_s *ps, OffhandClass offhandClass);
bool __cdecl PM_Weapon_IsHoldingGrenade(pmove_t *pm);
char __cdecl PM_UpdateGrenadeThrow(playerState_s *ps, pml_t *pml);
char __cdecl PM_Weapon_CheckGrenadeHold(pmove_t *pm, int delayedAction);
void __cdecl PM_Weapon_CheckForDetonation(pmove_t *pm);
void __cdecl PM_Weapon_CheckForGrenadeThrowCancel(pmove_t *pm);
void __cdecl PM_Detonate(playerState_s *ps, int delayedAction);
void __cdecl PM_Weapon_CheckForNightVision(pmove_t *pm);
void __cdecl PM_Weapon_FinishNightVisionWear(playerState_s *ps);
void __cdecl PM_Weapon_FinishNightVisionRemove(playerState_s *ps);
void __cdecl Sprint_State_Loop(playerState_s *ps);
void __cdecl PM_Weapon_CheckForSprint(pmove_t *pm);
void __cdecl Sprint_State_Raise(playerState_s *ps);
void __cdecl Sprint_State_Drop(playerState_s *ps);
void __cdecl PM_ResetWeaponState(playerState_s *ps);
void __cdecl BG_WeaponFireRecoil(const playerState_s *ps, float *vGunSpeed, float *kickAVel);
double __cdecl BG_GetBobCycle(const playerState_s *ps);
double __cdecl BG_GetVerticalBobFactor(const playerState_s *ps, float cycle, float speed, float maxAmp);
double __cdecl BG_GetHorizontalBobFactor(const playerState_s *ps, float cycle, float speed, float maxAmp);
void __cdecl BG_CalculateWeaponAngles(weaponState_t *ws, float *angles);
void __cdecl BG_CalculateWeaponPosition_BaseAngles(weaponState_t *ws, float *angles);
void __cdecl BG_CalculateWeaponPosition_BasePosition_angles(weaponState_t *ws, float *angles);
void __cdecl BG_CalculateWeaponPosition_IdleAngles(weaponState_t *ws, float *angles);
void __cdecl BG_CalculateWeaponPosition_BobOffset(weaponState_t *ws, float *angles);
void __cdecl BG_CalculateWeaponPosition_DamageKick(weaponState_t *ws, float *angles);
void __cdecl BG_CalculateWeaponPosition_GunRecoil(weaponState_t *ws, float *angles);
int __cdecl BG_CalculateWeaponPosition_GunRecoil_SingleAngle(
    float *fOffset,
    float *speed,
    float fTimeStep,
    float fOfsCap,
    float fGunKickAccel,
    float fGunKickSpeedMax,
    float fGunKickSpeedDecay,
    float fGunKickStaticDecay);
void __cdecl BG_CalculateViewAngles(viewState_t *vs, float *angles);
void __cdecl BG_CalculateView_DamageKick(viewState_t *vs, float *angles);
void __cdecl BG_CalculateView_IdleAngles(viewState_t *vs, float *angles);
void __cdecl BG_CalculateView_BobAngles(viewState_t *vs, float *angles);
void __cdecl BG_CalculateView_Velocity(viewState_t *vs, float *angles);
void __cdecl BG_CalculateWeaponPosition_Sway(
    const playerState_s *ps,
    float *swayViewAngles,
    float *swayOffset,
    float *swayAngles,
    float ssSwayScale,
    int frametime);
int __cdecl BG_PlayerWeaponCountPrimaryTypes(const playerState_s *ps);
bool __cdecl BG_PlayerWeaponsFull_Primaries(const playerState_s *ps);
char __cdecl BG_PlayerHasCompatibleWeapon(const playerState_s *ps, unsigned int weaponIndex);
bool __cdecl BG_ThrowingBackGrenade(const playerState_s *ps);
WeaponDef *__cdecl BG_LoadWeaponDef(const char *name);
WeaponDef *__cdecl BG_LoadWeaponDef_FastFile(const char *name);
void __cdecl BG_AssertOffhandIndexOrNone(unsigned int offHandIndex);
void __cdecl BG_StringCopy(unsigned __int8 *member, const char *keyValue);


// bg_vehicles_mp
unsigned __int16 __cdecl BG_VehiclesGetSlotTagName(int slotIndex);


// bg_slidemove
void __cdecl PM_StepSlideMove(pmove_t *pm, pml_t *pml, int gravity);
int __cdecl PM_VerifyPronePosition(pmove_t *pm, float *vFallbackOrg, float *vFallbackVel);
bool __cdecl PM_SlideMove(pmove_t *pm, pml_t *pml, int gravity);
double __cdecl PM_PermuteRestrictiveClipPlanes(
    const float *velocity,
    int planeCount,
    const float (*planes)[3],
    int *permutation);
