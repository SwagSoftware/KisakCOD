#pragma once

#include <qcommon/qcommon.h>
#include <qcommon/ent.h>

#include <game/enthandle.h>

#include "bg_weapons.h"

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

enum scriptAnimStrafeStates_t : __int32
{                                       // ...
    ANIM_STRAFE_NOT = 0x0,
    ANIM_STRAFE_LEFT = 0x1,
    ANIM_STRAFE_RIGHT = 0x2,
    NUM_ANIM_STRAFESTATES = 0x3,
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


enum ShockViewTypes : __int32
{                                       // ...
    SHELLSHOCK_VIEWTYPE_BLURRED = 0x0,
    SHELLSHOCK_VIEWTYPE_FLASHED = 0x1,
    SHELLSHOCK_VIEWTYPE_NONE = 0x2,
};

union hudelem_color_t // sizeof=0x4
{                                       // XREF: DrawSingleHudElem2d+114/r
    struct
    {
        __int8 r;
        __int8 g;
        __int8 b;
        __int8 a;
    };
    int rgba;
};

enum ViewLockTypes : __int32
{                                       // XREF: playerState_s/r
    PLAYERVIEWLOCK_NONE = 0x0,
    PLAYERVIEWLOCK_FULL = 0x1,
    PLAYERVIEWLOCK_WEAPONJITTER = 0x2,
    PLAYERVIEWLOCKCOUNT = 0x3,
};

//struct $6CB7272563F4458FB40A4A5E123C4ABA // sizeof=0x4
//{                                       // ...
//    unsigned __int16 index;
//    unsigned __int16 tree;
//};
//union $76411D3CC105A18E6E4A61D5A929E310 // sizeof=0x4
//{                                       // ...
//    $6CB7272563F4458FB40A4A5E123C4ABA __s0;
//    const char *linkPointer;
//};
struct scr_anim_s // sizeof=0x4
{                                       // ...
    //$76411D3CC105A18E6E4A61D5A929E310 ___u0; // ...
    union
    {
        struct
        {
            unsigned __int16 index;
            unsigned __int16 tree;
        };
        const char* linkPointer;
    };
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


struct viewDamage_t // sizeof=0xC
{                                       // ...
    int time;
    int duration;
    float yaw;
};
struct shellshock_parms_t_screenblend // sizeof=0x14
{                                       // ...
    int blurredFadeTime;
    int blurredEffectTime;
    int flashWhiteFadeTime;
    int flashShotFadeTime;
    ShockViewTypes type;
};
struct shellshock_parms_t_view // sizeof=0xC
{                                       // ...
    int fadeTime;
    float kickRate;
    float kickRadius;
};
struct shellshock_parms_t_sound // sizeof=0x230
{                                       // ...
    bool affect;
    char loop[64];
    char loopSilent[64];
    char end[64];
    char endAbort[64];
    // padding byte
    // padding byte
    // padding byte
    int fadeInTime;
    int fadeOutTime;
    float drylevel;
    float wetlevel;
    char roomtype[16];
    float channelvolume[64];
    int modEndDelay;
    int loopFadeTime;
    int loopEndDelay;
};
struct shellshock_parms_t_lookcontrol // sizeof=0x14
{                                       // ...
    bool affect;
    // padding byte
    // padding byte
    // padding byte
    int fadeTime;
    float mouseSensitivity;
    float maxPitchSpeed;
    float maxYawSpeed;
};
struct shellshock_parms_t_movement // sizeof=0x1
{                                       // ...
    bool affect;
};
const struct __declspec(align(4)) shellshock_parms_t // sizeof=0x268
{                                       // ...
    shellshock_parms_t_screenblend screenBlend;
    shellshock_parms_t_view view;
    shellshock_parms_t_sound sound;
    shellshock_parms_t_lookcontrol lookControl;
    shellshock_parms_t_movement movement;
    // padding byte
    // padding byte
    // padding byte
};
struct shellshock_t // sizeof=0x20
{                                       // ...
    const shellshock_parms_t* parms;
    int startTime;
    int duration;
    int loopEndTime;
    float sensitivity;
    float viewDelta[2];
    int hasSavedScreen;
};


struct __declspec(align(8)) animation_s // sizeof=0x68
{                                       // ...
    char name[64];
    int initialLerp;
    float moveSpeed;
    int duration;
    int nameHash;
    int flags;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    __int64 movetype;
    int noteType;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};
struct animScriptCondition_t // sizeof=0xC
{                                       // ...
    int index;
    unsigned int value[2];
};

struct snd_alias_list_t;
struct animScriptCommand_t // sizeof=0x10
{                                       // ...
    __int16 bodyPart[2];
    __int16 animIndex[2];
    __int16 animDuration[2];
    snd_alias_list_t* soundAlias;
};
enum animScriptParseMode_t : __int32
{                                       // ...
    PARSEMODE_DEFINES = 0x0,
    PARSEMODE_ANIMATION = 0x1,
    PARSEMODE_CANNED_ANIMATIONS = 0x2,
    PARSEMODE_STATECHANGES = 0x3,
    PARSEMODE_EVENTS = 0x4,
    NUM_PARSEMODES = 0x5,
};
struct animScriptItem_t // sizeof=0x100
{                                       // ...
    int numConditions;
    animScriptCondition_t conditions[10];
    int numCommands;
    animScriptCommand_t commands[8];
};
struct animScript_t // sizeof=0x204
{                                       // ...
    int numItems;
    animScriptItem_t* items[128];
};

struct XAnim_s;
struct scr_animtree_t // sizeof=0x4
{                                       // ...
    XAnim_s* anims;                     // ...
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
    snd_alias_list_t* (__cdecl* soundAlias)(const char*);
    int(__cdecl* playSoundAlias)(int, snd_alias_list_t*);
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};


struct lerpFrame_t // sizeof=0x30
{                                       // ...
    float yawAngle;
    int yawing;
    float pitchAngle;
    int pitching;
    int animationNumber;
    animation_s* animation;
    int animationTime;
    float oldFramePos[3];
    float animSpeedScale;
    int oldFrameSnapshotTime;
};

struct clientControllers_t // sizeof=0x60
{                                       // ...
    float angles[6][3];
    float tag_origin_angles[3];
    float tag_origin_offset[3];
};

enum team_t : __int32
{                                       // XREF: GetOtherTeamField/r
    TEAM_FREE = 0x0,
    TEAM_AXIS = 0x1,
    TEAM_ALLIES = 0x2,
    TEAM_SPECTATOR = 0x3,
    TEAM_NUM_TEAMS = 0x4,
};

struct __declspec(align(4)) clientInfo_t // sizeof=0x4CC
{                                       // ...
    int infoValid;                      // ...
    int nextValid;
    int clientNum;
    char name[16];
    team_t team;
    team_t oldteam;
    int rank;
    int prestige;
    int perks;
    int score;
    int location;
    int health;
    char model[64];
    char attachModelNames[6][64];
    char attachTagNames[6][64];
    lerpFrame_t legs;
    lerpFrame_t torso;
    float lerpMoveDir;
    float lerpLean;
    float playerAngles[3];              // ...
    int leftHandGun;
    int dobjDirty;
    clientControllers_t control;
    unsigned int clientConditions[10][2];
    XAnimTree_s* pXAnimTree;            // ...
    int iDObjWeapon;
    unsigned __int8 weaponModel;
    // padding byte
    // padding byte
    // padding byte
    int stanceTransitionTime;
    int turnAnimEndTime;
    char turnAnimType;
    // padding byte
    // padding byte
    // padding byte
    int attachedVehEntNum;
    int attachedVehSlotIndex;
    bool hideWeapon;
    bool usingKnife;
    // padding byte
    // padding byte
};
struct bgs_t_human // sizeof=0x10
{                                       // ...
    scr_animtree_t tree;                // ...
    scr_anim_s torso;
    scr_anim_s legs;
    scr_anim_s turning;
};
struct bgs_t // sizeof=0xADD08
{                                       // ...
    animScriptData_t animScriptData;    // ...
    bgs_t_human generic_human; // ...
    int time;                           // ...
    int latestSnapshotTime;             // ...
    int frametime;                      // ...
    int anim_user;                      // ...
    XModel* (__cdecl* GetXModel)(const char*); // ...
    void(__cdecl* CreateDObj)(DObjModel_s*, unsigned __int16, XAnimTree_s*, int, int, clientInfo_t*); // ...
    unsigned __int16(__cdecl* AttachWeapon)(DObjModel_s*, unsigned __int16, clientInfo_t*); // ...
    DObj_s* (__cdecl* GetDObj)(unsigned int, int); // ...
    void(__cdecl* SafeDObjFree)(unsigned int, int); // ...
    void* (__cdecl* AllocXAnim)(int);   // ...
    clientInfo_t clientinfo[64];        // ...
};


struct hudElemSoundInfo_t // sizeof=0x4
{                                       // ...
    int lastPlayedTime;
};

enum he_type_t : __int32
{                                       // XREF: hudelem_s/r
    HE_TYPE_FREE = 0x0,
    HE_TYPE_TEXT = 0x1,
    HE_TYPE_VALUE = 0x2,
    HE_TYPE_PLAYERNAME = 0x3,
    HE_TYPE_MAPNAME = 0x4,
    HE_TYPE_GAMETYPE = 0x5,
    HE_TYPE_MATERIAL = 0x6,
    HE_TYPE_TIMER_DOWN = 0x7,
    HE_TYPE_TIMER_UP = 0x8,
    HE_TYPE_TENTHS_TIMER_DOWN = 0x9,
    HE_TYPE_TENTHS_TIMER_UP = 0xA,
    HE_TYPE_CLOCK_DOWN = 0xB,
    HE_TYPE_CLOCK_UP = 0xC,
    HE_TYPE_WAYPOINT = 0xD,
    HE_TYPE_COUNT = 0xE,
};

struct hudelem_s // sizeof=0xA0
{                                       // XREF: .data:g_dummyHudCurrent/r
                                        // .data:g_dummyHudCurrent_0/r ...
    he_type_t type;
    float x;
    float y;
    float z;                            // XREF: .rdata:off_866438/o
    // .rdata:uint const * const g_swizzleYZXW__uint4/o
    int targetEntNum;
    float fontScale;
    int font;
    int alignOrg;
    int alignScreen;
    hudelem_color_t color;
    hudelem_color_t fromColor;
    int fadeStartTime;                  // XREF: _memmove:UnwindDown3/o
    // _memcpy:UnwindDown3_0/o ...
    int fadeTime;                       // XREF: Sys_GetPhysicalCpuCount+131/o
    // RB_LogPrintState_0(int,int)+19D/o ...
    int label;
    int width;
    int height;
    int materialIndex;
    int offscreenMaterialIdx;           // XREF: Image_CopyBitmapData:off_810011/o
    int fromWidth;                      // XREF: .rdata:008CF9F1/o
    // .rdata:008CFBF1/o ...
    int fromHeight;
    int scaleStartTime;                 // XREF: .rdata:008CFA4D/o
    // .rdata:008CFC01/o
    int scaleTime;
    float fromX;
    float fromY;
    int fromAlignOrg;
    int fromAlignScreen;                // XREF: SV_Shutdown(char const *):loc_5D1039/o
    // TRACK_sv_main(void)+A/o
    int moveStartTime;                  // XREF: .rdata:val_dc_luminance/o
    // .rdata:val_dc_chrominance/o ...
    int moveTime;                       // XREF: .rdata:008CFA2D/o
    // .rdata:008CFBFD/o ...
    int time;                           // XREF: .rdata:off_866450/o
    // .rdata:008CFA51/o ...
    int duration;
    float value;                        // XREF: unzlocal_CheckCurrentFileCoherencyHeader:loc_67D5A6/o
    int text;
    float sort;
    hudelem_color_t glowColor;
    int fxBirthTime;                    // XREF: R_Cinematic_BinkOpenPath:loc_792B62/o
    int fxLetterTime;                   // XREF: .rdata:008CFA1D/o
    int fxDecayStartTime;               // XREF: .rdata:008CFA31/o
    // .rdata:008CFA35/o ...
    int fxDecayDuration;                // XREF: .rdata:008E8CBD/o
    int soundID;
    int flags;
};

struct MantleState // sizeof=0x10
{                                       // XREF: playerState_s/r
    float yaw;
    int timer;
    int transIndex;
    int flags;
};

struct playerState_s_hud // sizeof=0x26C0
{                                       // XREF: playerState_s/r
    hudelem_s current[31];              // XREF: Sys_GetPhysicalCpuCount+131/o
    // unzlocal_CheckCurrentFileCoherencyHeader:loc_67D5A6/o ...
    hudelem_s archival[31];             // XREF: SV_Shutdown(char const *):loc_5D1039/o
    // TRACK_sv_main(void)+A/o ...
};

enum ActionSlotType : __int32
{                                       // XREF: playerState_s/r
    ACTIONSLOTTYPE_DONOTHING = 0x0,
    ACTIONSLOTTYPE_SPECIFYWEAPON = 0x1,
    ACTIONSLOTTYPE_ALTWEAPONTOGGLE = 0x2,
    ACTIONSLOTTYPE_NIGHTVISION = 0x3,
    ACTIONSLOTTYPECOUNT = 0x4,
};

struct ActionSlotParam_SpecifyWeapon // sizeof=0x4
{                                       // XREF: ActionSlotParam/r
    unsigned int index;
};
struct ActionSlotParam // sizeof=0x4
{                                       // XREF: playerState_s/r
    ActionSlotParam_SpecifyWeapon specifyWeapon;
};

struct SprintState // sizeof=0x14
{                                       // XREF: playerState_s/r cg_s/r
    int sprintButtonUpRequired;
    int sprintDelay;
    int lastSprintStart;
    int lastSprintEnd;
    int sprintStartMaxLength;
};

enum objectiveState_t : __int32
{                                       // XREF: objective_t/r
                                        // Scr_Objective_Add/r ...
    OBJST_EMPTY = 0x0,
    OBJST_ACTIVE = 0x1,
    OBJST_INVISIBLE = 0x2,
    OBJST_DONE = 0x3,
    OBJST_CURRENT = 0x4,
    OBJST_FAILED = 0x5,
    OBJST_NUMSTATES = 0x6,
};
struct objective_t // sizeof=0x1C
{                                       // XREF: playerState_s/r
    objectiveState_t state;
    float origin[3];                    // XREF: .data:00946428/o
    int entNum;
    int teamNum;                        // XREF: _memmove+2E8/o
    // _memcpy+2E8/o
    int icon;
};


struct playerState_s // sizeof=0x2F64
{                                       // XREF: gclient_s/r
                                        // clSnapshot_t/r ...
    int commandTime;
    int pm_type;
    int bobCycle;                       // XREF: R_ChangeState_1(GfxCmdBufState *,uint)+2AB/o
    int pm_flags;
    int weapFlags;
    int otherFlags;                     // XREF: SpectatorClientEndFrame(gentity_s *):loc_4F9901/r
    // SpectatorClientEndFrame(gentity_s *):loc_4F990E/r ...
    int pm_time;
    float origin[3];                    // XREF: SV_GetClientPositionAtTime(int,int,float * const)+12C/r
    // SV_GetClientPositionAtTime(int,int,float * const)+138/r ...
    float velocity[3];
    float oldVelocity[2];
    int weaponTime;
    int weaponDelay;
    int grenadeTimeLeft;
    int throwBackGrenadeOwner;
    int throwBackGrenadeTimeLeft;
    int weaponRestrictKickTime;
    int foliageSoundTime;
    int gravity;
    float leanf;
    int speed;
    float delta_angles[3];
    int groundEntityNum;
    float vLadderVec[3];
    int jumpTime;
    float jumpOriginZ;                  // XREF: .rdata:008CFA21/o
    // .rdata:008CFA25/o ...
    int legsTimer;
    int legsAnim;
    int torsoTimer;
    int torsoAnim;
    int legsAnimDuration;
    int torsoAnimDuration;
    int damageTimer;
    int damageDuration;
    int flinchYawAnim;
    int movementDir;
    int eFlags;                         // XREF: SpectatorClientEndFrame(gentity_s *):doFollow/r
    int eventSequence;                  // XREF: R_HW_SetSamplerState(IDirect3DDevice9 *,uint,uint,uint)+337/o
    int events[4];
    unsigned int eventParms[4];
    int oldEventSequence;
    int clientNum;
    int offHandIndex;
    OffhandSecondaryClass offhandSecondary;
    unsigned int weapon;
    int weaponstate;
    unsigned int weaponShotCount;
    float fWeaponPosFrac;
    int adsDelayTime;
    int spreadOverride;
    int spreadOverrideState;
    int viewmodelIndex;
    float viewangles[3];
    int viewHeightTarget;
    float viewHeightCurrent;
    int viewHeightLerpTime;
    int viewHeightLerpTarget;
    int viewHeightLerpDown;
    float viewAngleClampBase[2];
    float viewAngleClampRange[2];
    int damageEvent;
    int damageYaw;
    int damagePitch;
    int damageCount;
    int stats[5];                       // XREF: SV_GetClientPositionAtTime(int,int,float * const)+E9/r
    int ammo[128];
    int ammoclip[128];
    unsigned int weapons[4];
    unsigned int weaponold[4];
    unsigned int weaponrechamber[4];
    float proneDirection;
    float proneDirectionPitch;
    float proneTorsoPitch;
    ViewLockTypes viewlocked;
    int viewlocked_entNum;
    int cursorHint;
    int cursorHintString;
    int cursorHintEntIndex;
    int iCompassPlayerInfo;
    int radarEnabled;
    int locationSelectionInfo;
    SprintState sprintState;
    float fTorsoPitch;
    float fWaistPitch;
    float holdBreathScale;
    int holdBreathTimer;
    float moveSpeedScaleMultiplier;
    MantleState mantleState;
    float meleeChargeYaw;
    int meleeChargeDist;
    int meleeChargeTime;
    int perks;
    ActionSlotType actionSlotType[4];
    ActionSlotParam actionSlotParam[4];
    int entityEventSequence;
    int weapAnim;
    float aimSpreadScale;
    int shellshockIndex;
    int shellshockTime;
    int shellshockDuration;
    float dofNearStart;
    float dofNearEnd;
    float dofFarStart;
    float dofFarEnd;
    float dofNearBlur;
    float dofFarBlur;
    float dofViewmodelStart;
    float dofViewmodelEnd;
    int hudElemLastAssignedSoundID;
    objective_t objective[16];          // XREF: _memmove+2E8/o
    // _memcpy+2E8/o ...
    unsigned __int8 weaponmodels[128];
    int deltaTime;
    int killCamEntity;                  // XREF: SpectatorClientEndFrame(gentity_s *)+163/w
    // SpectatorClientEndFrame(gentity_s *)+17B/w
    playerState_s_hud hud;
    // XREF: SV_Shutdown(char const *):loc_5D1039/o
    // TRACK_sv_main(void)+A/o ...
};


struct __declspec(align(4)) CEntPlayerInfo // sizeof=0xC
{                                       // ...
    clientControllers_t* control;       // ...
    unsigned __int8 tag[6];             // ...
    // padding byte
    // padding byte
};

struct CEntTurretAngles // sizeof=0x8
{                                       // ...
    float pitch;
    float yaw;
};

struct CEntTurretInfo // sizeof=0x10
{                                       // ...
    union
    {
        CEntTurretAngles angles;
        const float *viewAngles;
    };
    float barrelPitch;
    bool playerUsing;
    unsigned __int8 tag_aim;
    unsigned __int8 tag_aim_animated;
    unsigned __int8 tag_flash;
};
struct __declspec(align(2)) CEntVehicleInfo // sizeof=0x24
{                                       // ...
    __int16 pitch;
    __int16 yaw;
    __int16 roll;
    __int16 barrelPitch;
    float barrelRoll;
    __int16 steerYaw;
    // padding byte
    // padding byte
    float time;
    unsigned __int16 wheelFraction[4];
    unsigned __int8 wheelBoneIndex[4];
    unsigned __int8 tag_body;
    unsigned __int8 tag_turret;
    unsigned __int8 tag_barrel;
    // padding byte
};

struct FxEffect;
struct CEntFx // sizeof=0x8
{                                       // ...
    int triggerTime;
    FxEffect* effect;
};

struct GfxSkinCacheEntry // sizeof=0xC
{                                       // ...
    unsigned int frameCount;
    int skinnedCachedOffset;
    unsigned __int16 numSkinnedVerts;
    unsigned __int16 ageCount;
};

struct cpose_t // sizeof=0x64
{                                       // ...
    unsigned __int16 lightingHandle;
    unsigned __int8 eType;
    unsigned __int8 eTypeUnion;
    unsigned __int8 localClientNum;
    // padding byte
    // padding byte
    // padding byte
    volatile unsigned long cullIn;
    unsigned __int8 isRagdoll;
    // padding byte
    // padding byte
    // padding byte
    int ragdollHandle;
    int killcamRagdollHandle;
    int physObjId;
    float origin[3];
    float angles[3];
    GfxSkinCacheEntry skinCacheEntry;
    //$9D88A49AD898204B3D6E378457DD8419 ___u12;
    union //$9D88A49AD898204B3D6E378457DD8419 // sizeof=0x24
    {                                       // ...
        CEntPlayerInfo player;
        CEntTurretInfo turret;
        CEntVehicleInfo vehicle;
        CEntFx fx;
    };
};

struct XAnimTree_s;

struct centity_s // sizeof=0x1DC
{                                       // ...
    cpose_t pose;
    LerpEntityState currentState;
    entityState_s nextState;
    bool nextValid;
    bool bMuzzleFlash;
    bool bTrailMade;
    // padding byte
    int previousEventSequence;
    int miscTime;
    float lightingOrigin[3];
    XAnimTree_s* tree;
};


enum DemoType : __int32
{                                       // ...
    DEMO_TYPE_NONE = 0x0,
    DEMO_TYPE_CLIENT = 0x1,
    DEMO_TYPE_SERVER = 0x2,
};
enum CubemapShot : __int32
{                                       // ...
    CUBEMAPSHOT_NONE = 0x0,
    CUBEMAPSHOT_RIGHT = 0x1,
    CUBEMAPSHOT_LEFT = 0x2,
    CUBEMAPSHOT_BACK = 0x3,
    CUBEMAPSHOT_FRONT = 0x4,
    CUBEMAPSHOT_UP = 0x5,
    CUBEMAPSHOT_DOWN = 0x6,
    CUBEMAPSHOT_COUNT = 0x7,
};


struct turretInfo_s // sizeof=0x48
{                                       // ...
    int inuse;                          // ...
    int flags;
    int fireTime;
    float arcmin[2];
    float arcmax[2];
    float dropPitch;
    int stance;
    int prevStance;
    int fireSndDelay;
    float userOrigin[3];
    float playerSpread;
    float pitchCap;
    int triggerDown;
    unsigned __int8 fireSnd;
    unsigned __int8 fireSndPlayer;
    unsigned __int8 stopSnd;
    unsigned __int8 stopSndPlayer;
};

struct VehicleRideSlot_t // sizeof=0xC
{                                       // ...
    unsigned int tagName;
    int boneIdx;
    int entNum;
};

struct vehicle_node_t // sizeof=0x44
{                                       // ...
    unsigned __int16 name;
    unsigned __int16 target;
    unsigned __int16 script_linkname;
    unsigned __int16 script_noteworthy;
    __int16 index;
    // padding byte
    // padding byte
    int rotated;
    float speed;
    float lookAhead;
    float origin[3];
    float dir[3];
    float angles[3];
    float length;
    __int16 nextIdx;
    __int16 prevIdx;
};

struct vehicle_pathpos_t // sizeof=0xC0
{                                       // ...
    __int16 nodeIdx;
    __int16 endOfPath;
    float frac;
    float speed;
    float lookAhead;
    float slide;
    float origin[3];
    float angles[3];
    float lookPos[3];
    vehicle_node_t switchNode[2];
};
struct vehicle_physic_t // sizeof=0xF8
{                                       // ...
    float origin[3];
    float prevOrigin[3];
    float angles[3];
    float prevAngles[3];
    float maxAngleVel[3];
    float yawAccel;
    float yawDecel;
    char inputAccelerationOLD;
    char inputTurning;
    // padding byte
    // padding byte
    float driverPedal;
    float driverSteer;
    int onGround;
    float colVelDelta[3];
    float mins[3];
    float maxs[3];
    float vel[3];
    float bodyVel[3];
    float rotVel[3];
    float accel[3];
    float maxPitchAngle;
    float maxRollAngle;
    float wheelZVel[4];
    float wheelZPos[4];
    int wheelSurfType[4];
    float worldTilt[3];
    float worldTiltVel[3];
};

struct VehicleTags // sizeof=0x60
{                                       // ...
    VehicleRideSlot_t riderSlots[3];
    int detach;
    int popout;
    int body;
    int turret;
    int turret_base;
    int barrel;
    int flash[5];
    int wheel[4];
};

enum VehicleMoveState : __int32
{                                       // ...
    VEH_MOVESTATE_STOP = 0x0,
    VEH_MOVESTATE_MOVE = 0x1,
    VEH_MOVESTATE_HOVER = 0x2,
};

enum VehicleTurretState : __int32
{                                       // ...
    VEH_TURRET_STOPPED = 0x0,
    VEH_TURRET_STOPPING = 0x1,
    VEH_TURRET_MOVING = 0x2,
};

struct VehicleTurret // sizeof=0x14
{                                       // ...
    int fireTime;
    int fireBarrel;
    float barrelOffset;
    int barrelBlocked;
    VehicleTurretState turretState;
};
struct VehicleJitter // sizeof=0x3C
{                                       // ...
    int jitterPeriodMin;
    int jitterPeriodMax;
    int jitterEndTime;
    float jitterOffsetRange[3];
    float jitterDeltaAccel[3];
    float jitterAccel[3];
    float jitterPos[3];
};
struct VehicleHover // sizeof=0x1C
{                                       // ...
    float hoverRadius;
    float hoverSpeed;
    float hoverAccel;
    float hoverGoalPos[3];
    int useHoverAccelForAngles;
};

struct item_ent_t // sizeof=0xC
{                                       // ...
    int ammoCount;
    int clipAmmoCount;
    int index;
};
struct __declspec(align(4)) trigger_ent_t // sizeof=0x14
{                                       // ...
    int threshold;
    int accumulate;
    int timestamp;
    int singleUserEntIndex;
    bool requireLookAt;
    // padding byte
    // padding byte
    // padding byte
};
struct mover_ent_t // sizeof=0x60
{                                       // ...
    float decelTime;
    float aDecelTime;
    float speed;
    float aSpeed;
    float midTime;
    float aMidTime;
    float pos1[3];
    float pos2[3];
    float pos3[3];
    float apos1[3];
    float apos2[3];
    float apos3[3];
};
struct corpse_ent_t // sizeof=0x4
{                                       // ...
    int deathAnimStartTime;
};

enum MissileStage : __int32
{                                       // ...
    MISSILESTAGE_SOFTLAUNCH = 0x0,
    MISSILESTAGE_ASCENT = 0x1,
    MISSILESTAGE_DESCENT = 0x2,
};
enum MissileFlightMode : __int32
{                                       // ...
    MISSILEFLIGHTMODE_TOP = 0x0,
    MISSILEFLIGHTMODE_DIRECT = 0x1,
};
struct missile_ent_t // sizeof=0x3C
{                                       // ...
    float time;
    int timeOfBirth;
    float travelDist;
    float surfaceNormal[3];
    team_t team;
    float curvature[3];
    float targetOffset[3];
    MissileStage stage;
    MissileFlightMode flightMode;
};

struct scr_vehicle_s // sizeof=0x354
{                                       // ...
    vehicle_pathpos_t pathPos;
    vehicle_physic_t phys;
    int entNum;                         // ...
    __int16 infoIdx;
    // padding byte
    // padding byte
    int flags;
    int team;
    VehicleMoveState moveState;
    __int16 waitNode;
    // padding byte
    // padding byte
    float waitSpeed;
    VehicleTurret turret;
    VehicleJitter jitter;
    VehicleHover hover;
    int drawOnCompass;
    unsigned __int16 lookAtText0;
    unsigned __int16 lookAtText1;
    int manualMode;
    float manualSpeed;
    float manualAccel;
    float manualDecel;
    float manualTime;
    float speed;
    float maxDragSpeed;
    float turningAbility;
    int hasTarget;
    int hasTargetYaw;
    int hasGoalYaw;
    int stopAtGoal;
    int stopping;
    int targetEnt;
    EntHandle lookAtEnt;
    float targetOrigin[3];
    float targetOffset[3];
    float targetYaw;
    float goalPosition[3];
    float goalYaw;
    float prevGoalYaw;
    float yawOverShoot;
    int yawSlowDown;
    float nearGoalNotifyDist;
    float joltDir[2];
    float joltTime;
    float joltWave;
    float joltSpeed;
    float joltDecel;
    int playEngineSound;
    EntHandle idleSndEnt;
    EntHandle engineSndEnt;
    float idleSndLerp;
    float engineSndLerp;
    VehicleTags boneIndex;
    int turretHitNum;
    float forcedMaterialSpeed;
};

struct entityShared_t // sizeof=0x68
{                                       // ...
    unsigned __int8 linked;
    unsigned __int8 bmodel;
    unsigned __int8 svFlags;
    // padding byte
    int clientMask[2];
    unsigned __int8 inuse;              // ...
    // padding byte
    // padding byte
    // padding byte
    int broadcastTime;
    float mins[3];                      // ...
    float maxs[3];
    int contents;                       // ...
    float absmin[3];                    // ...
    float absmax[3];
    float currentOrigin[3];             // ...
    float currentAngles[3];
    EntHandle ownerNum;
    int eventTime;
};

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


struct __declspec(align(2)) usercmd_s // sizeof=0x20
{                                       // XREF: ?SV_BotUserMove@@YAXPAUclient_t@@@Z/r
                                        // ?SV_UserMove@@YAXPAUclient_t@@PAUmsg_t@@H@Z/r ...
    int serverTime;                     // XREF: CG_DrawDisconnect+85/r
    // CG_DrawDisconnect+90/r ...
    int buttons;                        // XREF: CG_CheckForPlayerInput+5D/r
    // CG_CheckForPlayerInput+60/r ...
    int angles[3];                      // XREF: CG_CheckPlayerMovement+B/o
    // CG_CheckPlayerMovement+E/o ...
    unsigned __int8 weapon;             // XREF: CL_CreateCmd+64/w
    // SV_AddTestClient(void)+232/w ...
    unsigned __int8 offHandIndex;
    char forwardmove;                   // XREF: CG_CheckPlayerMovement:loc_4413AE/r
    // SV_BotUserMove(client_t *)+138/w ...
    char rightmove;                     // XREF: CG_CheckPlayerMovement+26/r
    // SV_BotUserMove(client_t *)+166/w ...
    float meleeChargeYaw;               // XREF: CL_CreateCmd+67/w
    // SV_AddTestClient(void)+238/w ...
    unsigned __int8 meleeChargeDist;    // XREF: CL_CreateCmd+6A/w
    // SV_AddTestClient(void)+23E/w ...
    char selectedLocation[2];
    // padding byte
};

// bg_jump

struct pmove_t;
struct pml_t;

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
animScriptParseMode_t __cdecl BG_IndexForString(const char *token, animStringItem_t *strings, int allowFail);
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
const char *__cdecl GetMoveTypeName(int type);
const char *__cdecl GetWeaponTypeName(int type);
const char *__cdecl GetBodyPart(int bodypart);
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
struct WeaponDef;
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
void __cdecl BG_CreateXAnim(XAnim_s *anims, unsigned int animIndex, const char *name);
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
extern const dvar_s *bg_shock_volume[64];
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

extern int surfaceTypeSoundListCount;



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
struct pmove_t;
struct trace_t;

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




// bg_weapons
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

struct gentity_s; // never used in bg
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
enum vehicleRideSlots_t : __int32
{
    VEHICLE_RIDESLOT_DRIVER = 0x0,
    VEHICLE_RIDESLOT_PASSENGER = 0x1,
    VEHICLE_RIDESLOT_GUNNER = 0x2,
    VEHICLE_RIDESLOTS_COUNT = 0x3,
};

uint16 BG_VehiclesGetSlotTagName(int slotIndex);


// bg_slidemove
void __cdecl PM_StepSlideMove(pmove_t *pm, pml_t *pml, int gravity);
int __cdecl PM_VerifyPronePosition(pmove_t *pm, float *vFallbackOrg, float *vFallbackVel);
bool __cdecl PM_SlideMove(pmove_t *pm, pml_t *pml, int gravity);
double __cdecl PM_PermuteRestrictiveClipPlanes(
    const float *velocity,
    int planeCount,
    const float (*planes)[3],
    int *permutation);


// bg_weapons_load_obj
char *__cdecl BG_GetPlayerAnimTypeName(int index);
void __cdecl TRACK_bg_weapons_load_obj();
const char *__cdecl BG_GetWeaponTypeName(weapType_t type);
const char *__cdecl BG_GetWeaponClassName(weapClass_t type);
const char *__cdecl BG_GetWeaponInventoryTypeName(weapInventoryType_t type);
void __cdecl BG_LoadWeaponStrings();
void __cdecl BG_LoadPlayerAnimTypes();
WeaponDef *__cdecl BG_LoadDefaultWeaponDef();
WeaponDef *__cdecl BG_LoadDefaultWeaponDef_FastFile();



// bg_misctables
extern _DWORD bg_itemlist[2048];

const float playerMins[] = { -15.0, -15.0, 0.0 };
const float playerMaxs[] = { 15.0, 15.0, 0.0 };