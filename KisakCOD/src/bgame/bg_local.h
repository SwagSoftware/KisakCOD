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
