#pragma once

#ifdef KISAK_MP
#include <qcommon/msg_mp.h>
#elif KISAK_SP
#include <qcommon/msg.h>
#endif

#include <qcommon/qcommon.h>
#include <qcommon/ent.h>

#include <game/enthandle.h>

#include "bg_weapons.h"

#include <cstdint>

struct FxEffect;
struct snd_alias_list_t;
struct XAnim_s;
struct XAnimTree_s;

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
inline aistateEnum_t &operator--(aistateEnum_t &e) {
    e = static_cast<aistateEnum_t>(static_cast<int>(e) - 1);
    return e;
}
inline aistateEnum_t &operator--(aistateEnum_t &e, int i)
{
    --e;
    return e;
}

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
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
    uint32_t rgba;
};
static_assert(sizeof(union hudelem_color_t) == 0x4);

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
{   
    scr_anim_s()
    {
        linkPointer = NULL;
    }
    scr_anim_s(int i)
    {
        linkPointer = (const char *)i; // KISAKHACK
    }
    // ...
    //$76411D3CC105A18E6E4A61D5A929E310 ___u0; // ...
    union
    {
        struct
        {
            uint16_t index;
            uint16_t tree;
        };
        const char* linkPointer;
    };
};
static_assert(sizeof(struct scr_anim_s) == 0x4);

struct loadAnim_t // sizeof=0x48
{
    scr_anim_s anim;
    int32_t iNameHash;
    char szAnimName[64];
};
static_assert((sizeof(struct loadAnim_t) * 512) == 36864);

struct pml_t // sizeof=0x80
{                                       // ...
    float forward[3];
    float right[3];                     // ...
    float up[3];                        // ...
    float frametime;                    // ...
    int32_t msec;                           // ...
    int32_t walking;                        // ...
    int32_t groundPlane;                    // ...
    int32_t almostGroundPlane;              // ...
    trace_t groundTrace;
    float impactSpeed;
    float previous_origin[3];           // ...
    float previous_velocity[3];         // ...
};
static_assert(sizeof(pml_t) == 0x80);

struct animStringItem_t // sizeof=0x8
{                                       // ...
    const char *string;                 // ...
    int32_t hash;                           // ...
};
static_assert(sizeof(animStringItem_t) == 0x8);

struct controller_info_t // sizeof=0x60
{                                       // ...
    float angles[6][3];
    float tag_origin_angles[3];         // ...
    float tag_origin_offset[3];         // ...
};
static_assert(sizeof(controller_info_t) == 0x60);

struct animConditionTable_t // sizeof=0x8
{                                       // ...
    animScriptConditionTypes_t type;    // ...
    animStringItem_t *values;           // ...
};
static_assert(sizeof(animConditionTable_t) == 0x8);

struct viewDamage_t // sizeof=0xC
{                                       // ...
    int32_t time;
    int32_t duration;
    float yaw;
};
static_assert(sizeof(viewDamage_t) == 0xC);

struct shellshock_parms_t_screenblend // sizeof=0x14
{                                       // ...
    int32_t blurredFadeTime;
    int32_t blurredEffectTime;
    int32_t flashWhiteFadeTime;
    int32_t flashShotFadeTime;
    ShockViewTypes type;
};
static_assert(sizeof(shellshock_parms_t_screenblend) == 0x14);

struct shellshock_parms_t_view // sizeof=0xC
{                                       // ...
    int32_t fadeTime;
    float kickRate;
    float kickRadius;
};
static_assert(sizeof(shellshock_parms_t_view) == 0xC);

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
    int32_t fadeInTime;
    int32_t fadeOutTime;
    float drylevel;
    float wetlevel;
    char roomtype[16];
    float channelvolume[64];
    int32_t modEndDelay;
    int32_t loopFadeTime;
    int32_t loopEndDelay;
};
static_assert(sizeof(shellshock_parms_t_sound) == 0x230);

struct shellshock_parms_t_lookcontrol // sizeof=0x14
{                                       // ...
    bool affect;
    // padding byte
    // padding byte
    // padding byte
    int32_t fadeTime;
    float mouseSensitivity;
    float maxPitchSpeed;
    float maxYawSpeed;
};
static_assert(sizeof(shellshock_parms_t_lookcontrol) == 0x14);

struct shellshock_parms_t_movement // sizeof=0x1
{                                       // ...
    bool affect;
};
static_assert(sizeof(shellshock_parms_t_movement) == 0x1);

const struct shellshock_parms_t // sizeof=0x268
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
static_assert(sizeof(shellshock_parms_t) == 0x268);

struct shellshock_t // sizeof=0x20
{                                       // ...
    const shellshock_parms_t* parms;
    int32_t startTime;
    int32_t duration;
    int32_t loopEndTime;
    float sensitivity;
    float viewDelta[2];
    int32_t hasSavedScreen;
};
static_assert(sizeof(shellshock_t) == 0x20);

struct __declspec(align(8)) animation_s // sizeof=0x68
{                                       // ...
    char name[64];
    int32_t initialLerp;
    float moveSpeed;
    int32_t duration;
    int32_t nameHash;
    int32_t flags;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    int64_t movetype;
    int32_t noteType;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};
static_assert(sizeof(animation_s) == 0x68);

struct animScriptCondition_t // sizeof=0xC
{                                       // ...
    int32_t index;
    uint32_t value[2];
};
static_assert(sizeof(animScriptCondition_t) == 0xC);


struct animScriptCommand_t // sizeof=0x10
{                                       // ...
    int16_t bodyPart[2];
    int16_t animIndex[2];
    int16_t animDuration[2];
    snd_alias_list_t* soundAlias;
};
static_assert(sizeof(animScriptCommand_t) == 0x10);

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
    int32_t numConditions;
    animScriptCondition_t conditions[10];
    int32_t numCommands;
    animScriptCommand_t commands[8];
};
static_assert(sizeof(animScriptItem_t) == 0x100);

struct animScript_t // sizeof=0x204
{                                       // ...
    int32_t numItems;
    animScriptItem_t* items[128];
};
static_assert(sizeof(animScript_t) == 0x204);

struct scr_animtree_t // sizeof=0x4
{                                       // ...
    scr_animtree_t()
    {
        anims = NULL;
    }
    XAnim_s* anims;                     // ...
};
static_assert(sizeof(scr_animtree_t) == 0x4);

struct __declspec(align(8)) animScriptData_t // sizeof=0x9A9D0
{                                       // ...
    animation_s animations[512];
    uint32_t numAnimations;
    animScript_t scriptAnims[1][43];
    animScript_t scriptCannedAnims[1][43];
    animScript_t scriptStateChange[1][1];
    animScript_t scriptEvents[21];
    animScriptItem_t scriptItems[2048];
    int32_t numScriptItems;
    scr_animtree_t animTree;            // ...
    uint16_t torsoAnim;
    uint16_t legsAnim;
    uint16_t turningAnim;
    // padding byte
    // padding byte
    snd_alias_list_t* (__cdecl* soundAlias)(const char*);
    int32_t(__cdecl* playSoundAlias)(int32_t, snd_alias_list_t*);
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};
static_assert(sizeof(animScriptData_t) == 0x9A9D0);

struct lerpFrame_t // sizeof=0x30
{                                       // ...
    float yawAngle;
    int32_t yawing;
    float pitchAngle;
    int32_t pitching;
    int32_t animationNumber;
    animation_s* animation;
    int32_t animationTime;
    float oldFramePos[3];
    float animSpeedScale;
    int32_t oldFrameSnapshotTime;
};
static_assert(sizeof(lerpFrame_t) == 0x30);

struct clientControllers_t // sizeof=0x60
{                                       // ...
    float angles[6][3];
    float tag_origin_angles[3];
    float tag_origin_offset[3];
};
static_assert(sizeof(clientControllers_t) == 0x60);

enum team_t : __int32
{                                       // XREF: GetOtherTeamField/r
    TEAM_FREE = 0x0,
    TEAM_BAD = 0x0,
    TEAM_AXIS = 0x1,
    TEAM_ALLIES = 0x2,
#ifdef KISAK_MP
    TEAM_SPECTATOR = 0x3,
    TEAM_NUM_TEAMS = 0x4,
#elif KISAK_SP
    TEAM_NEUTRAL = 0x3,
    TEAM_DEAD = 0x4,
    TEAM_NUM_TEAMS = 0x5,
#endif
};

struct clientInfo_t // sizeof=0x4CC
{                                       // ...
    int32_t infoValid;                      // ...
    int32_t nextValid;
    int32_t clientNum;
    char name[16];
    team_t team;
    team_t oldteam;
    int32_t rank;
    int32_t prestige;
    int32_t perks;
    int32_t score;
    int32_t location;
    int32_t health;
    char model[64];
    char attachModelNames[6][64];
    char attachTagNames[6][64];
    lerpFrame_t legs;
    lerpFrame_t torso;
    float lerpMoveDir;
    float lerpLean;
    float playerAngles[3];              // ...
    int32_t leftHandGun;
    int32_t dobjDirty;
    clientControllers_t control;
    uint32_t clientConditions[10][2];
    XAnimTree_s* pXAnimTree;            // ...
    int32_t iDObjWeapon;
    uint8_t weaponModel;
    // padding byte
    // padding byte
    // padding byte
    int32_t stanceTransitionTime;
    int32_t turnAnimEndTime;
    char turnAnimType;
    // padding byte
    // padding byte
    // padding byte
    int32_t attachedVehEntNum;
    int32_t attachedVehSlotIndex;
    bool hideWeapon;
    bool usingKnife;
    // padding byte
    // padding byte
};
static_assert(sizeof(clientInfo_t) == 0x4CC);

struct bgs_t_human // sizeof=0x10
{                                       // ...
    scr_animtree_t tree;                // ...
    scr_anim_s torso;
    scr_anim_s legs;
    scr_anim_s turning;
};
static_assert(sizeof(bgs_t_human) == 0x10);

struct bgs_t // sizeof=0xADD08
{                                       // ...
    animScriptData_t animScriptData;    // ...
    bgs_t_human generic_human; // ...
    int32_t time;                           // ...
    int32_t latestSnapshotTime;             // ...
    int32_t frametime;                      // ...
    int32_t anim_user;                      // ...
    XModel* (__cdecl* GetXModel)(const char*); // ...
    void(__cdecl* CreateDObj)(DObjModel_s*, uint16_t, XAnimTree_s*, int32_t, int32_t, clientInfo_t*); // ...
    uint16_t(__cdecl* AttachWeapon)(DObjModel_s*, uint16_t, clientInfo_t*); // ...
    DObj_s* (__cdecl* GetDObj)(uint32_t, int32_t); // ...
    void(__cdecl* SafeDObjFree)(uint32_t, int32_t); // ...
    void* (__cdecl* AllocXAnim)(int32_t);   // ...
    clientInfo_t clientinfo[64];        // ...
};
static_assert(sizeof(bgs_t) == 0xADD08);

struct hudElemSoundInfo_t // sizeof=0x4
{                                       // ...
    int32_t lastPlayedTime;
};
static_assert(sizeof(hudElemSoundInfo_t) == 0x4);

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

#ifdef KISAK_MP
struct hudelem_s // sizeof=0xA0
{                                       // XREF: .data:g_dummyHudCurrent/r
    he_type_t type;
    float x;
    float y;
    float z;                            // XREF: .rdata:off_866438/o
    int32_t targetEntNum;
    float fontScale;
    int32_t font;
    int32_t alignOrg;
    int32_t alignScreen;
    hudelem_color_t color;
    hudelem_color_t fromColor;
    int32_t fadeStartTime;                  // XREF: _memmove:UnwindDown3/o
    int32_t fadeTime;                       // XREF: Sys_GetPhysicalCpuCount+131/o
    int32_t label;
    int32_t width;
    int32_t height;
    int32_t materialIndex;
    int32_t offscreenMaterialIdx;           // XREF: Image_CopyBitmapData:off_810011/o
    int32_t fromWidth;                      // XREF: .rdata:008CF9F1/o
    int32_t fromHeight;
    int32_t scaleStartTime;                 // XREF: .rdata:008CFA4D/o
    int32_t scaleTime;
    float fromX;
    float fromY;
    int32_t fromAlignOrg;
    int32_t fromAlignScreen;                // XREF: SV_Shutdown(char const *):loc_5D1039/o
    int32_t moveStartTime;                  // XREF: .rdata:val_dc_luminance/o
    int32_t moveTime;                       // XREF: .rdata:008CFA2D/o
    int32_t time;                           // XREF: .rdata:off_866450/o
    int32_t duration;
    float value;                        // XREF: unzlocal_CheckCurrentFileCoherencyHeader:loc_67D5A6/o
    int32_t text;
    float sort;
    hudelem_color_t glowColor;
    int32_t fxBirthTime;                    // XREF: R_Cinematic_BinkOpenPath:loc_792B62/o
    int32_t fxLetterTime;                   // XREF: .rdata:008CFA1D/o
    int32_t fxDecayStartTime;               // XREF: .rdata:008CFA31/o
    int32_t fxDecayDuration;                // XREF: .rdata:008E8CBD/o
    int32_t soundID;
    int32_t flags;
};
static_assert(sizeof(hudelem_s) == 0xA0);
#elif KISAK_SP
struct hudelem_s
{
    he_type_t type;
    float x;
    float y;
    float z;
    int targetEntNum;
    float fontScale;
    float fromFontScale;
    int fontScaleStartTime;
    int fontScaleTime;
    int font;
    int alignOrg;
    int alignScreen;
    hudelem_color_t color;
    hudelem_color_t fromColor;
    int fadeStartTime;
    int fadeTime;
    int label;
    int width;
    int height;
    int materialIndex;
    int offscreenMaterialIdx;
    int fromWidth;
    int fromHeight;
    int scaleStartTime;
    int scaleTime;
    float fromX;
    float fromY;
    int fromAlignOrg;
    int fromAlignScreen;
    int moveStartTime;
    int moveTime;
    int time;
    int duration;
    float value;
    int text;
    float sort;
    hudelem_color_t glowColor;
    int fxBirthTime;
    int fxLetterTime;
    int fxDecayStartTime;
    int fxDecayDuration;
    int soundID;
    int flags;
};
#endif

struct MantleState // sizeof=0x10
{                                       // XREF: playerState_s/r
    float yaw;
    int32_t timer;
    int32_t transIndex;
    int32_t flags;
};
static_assert(sizeof(MantleState) == 0x10);

#ifdef KISAK_MP
struct playerState_s_hud // sizeof=0x26C0
{                                       // XREF: playerState_s/r
    hudelem_s current[31];              // XREF: Sys_GetPhysicalCpuCount+131/o
    hudelem_s archival[31];             // XREF: SV_Shutdown(char const *):loc_5D1039/o
};
static_assert(sizeof(playerState_s_hud) == 0x26C0);
#elif KISAK_SP
struct playerState_s_hud
{
    hudelem_s elem[256];
};
#endif

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
    uint32_t index;
};
static_assert(sizeof(ActionSlotParam_SpecifyWeapon) == 0x4);

struct ActionSlotParam // sizeof=0x4
{                                       // XREF: playerState_s/r
    ActionSlotParam_SpecifyWeapon specifyWeapon;
};
static_assert(sizeof(ActionSlotParam) == 0x4);

struct SprintState // sizeof=0x14
{                                       // XREF: playerState_s/r cg_s/r
    int32_t sprintButtonUpRequired;
    int32_t sprintDelay;
    int32_t lastSprintStart;
    int32_t lastSprintEnd;
    int32_t sprintStartMaxLength;
};
static_assert(sizeof(SprintState) == 0x14);

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
    int32_t entNum;
    int32_t teamNum;                        // XREF: _memmove+2E8/o
    // _memcpy+2E8/o
    int32_t icon;
};
static_assert(sizeof(objective_t) == 0x1C);

#ifdef KISAK_MP
struct playerState_s // sizeof=0x2F64
{                                       // XREF: gclient_s/r
                                        // clSnapshot_t/r ...
    int32_t commandTime;
    int32_t pm_type;
    int32_t bobCycle;                       // XREF: R_ChangeState_1(GfxCmdBufState *,uint)+2AB/o
    int32_t pm_flags;
    int32_t weapFlags;
    int32_t otherFlags;                     // XREF: SpectatorClientEndFrame(gentity_s *):loc_4F9901/r
    // SpectatorClientEndFrame(gentity_s *):loc_4F990E/r ...
    int32_t pm_time;
    float origin[3];                    // XREF: SV_GetClientPositionAtTime(int,int,float * const)+12C/r
    // SV_GetClientPositionAtTime(int,int,float * const)+138/r ...
    float velocity[3];
    float oldVelocity[2];
    int32_t weaponTime;
    int32_t weaponDelay;
    int32_t grenadeTimeLeft;
    int32_t throwBackGrenadeOwner;
    int32_t throwBackGrenadeTimeLeft;
    int32_t weaponRestrictKickTime;
    int32_t foliageSoundTime;
    int32_t gravity;
    float leanf;
    int32_t speed;
    float delta_angles[3];
    int32_t groundEntityNum;
    float vLadderVec[3];
    int32_t jumpTime;
    float jumpOriginZ;                  // XREF: .rdata:008CFA21/o
    // .rdata:008CFA25/o ...
    int32_t legsTimer;
    int32_t legsAnim;
    int32_t torsoTimer;
    int32_t torsoAnim;
    int32_t legsAnimDuration;
    int32_t torsoAnimDuration;
    int32_t damageTimer;
    int32_t damageDuration;
    int32_t flinchYawAnim;
    int32_t movementDir;
    int32_t eFlags;                         // XREF: SpectatorClientEndFrame(gentity_s *):doFollow/r
    int32_t eventSequence;                  // XREF: R_HW_SetSamplerState(IDirect3DDevice9 *,uint,uint,uint)+337/o
    int32_t events[4];
    uint32_t eventParms[4];
    int32_t oldEventSequence;
    int32_t clientNum;
    int32_t offHandIndex;
    OffhandSecondaryClass offhandSecondary;
    uint32_t weapon;
    int32_t weaponstate;
    uint32_t weaponShotCount;
    float fWeaponPosFrac;
    int32_t adsDelayTime;
    int32_t spreadOverride;
    int32_t spreadOverrideState;
    int32_t viewmodelIndex;
    float viewangles[3];
    int32_t viewHeightTarget;
    float viewHeightCurrent;
    int32_t viewHeightLerpTime;
    int32_t viewHeightLerpTarget;
    int32_t viewHeightLerpDown;
    float viewAngleClampBase[2];
    float viewAngleClampRange[2];
    int32_t damageEvent;
    int32_t damageYaw;
    int32_t damagePitch;
    int32_t damageCount;
    int32_t stats[5];                       // XREF: SV_GetClientPositionAtTime(int,int,float * const)+E9/r
    int32_t ammo[128];
    int32_t ammoclip[128];
    uint32_t weapons[4];
    uint32_t weaponold[4];
    uint32_t weaponrechamber[4];
    float proneDirection;
    float proneDirectionPitch;
    float proneTorsoPitch;
    ViewLockTypes viewlocked;
    int32_t viewlocked_entNum;
    int32_t cursorHint;
    int32_t cursorHintString;
    int32_t cursorHintEntIndex;
    int32_t iCompassPlayerInfo;
    int32_t radarEnabled;
    int32_t locationSelectionInfo;
    SprintState sprintState;
    float fTorsoPitch;
    float fWaistPitch;
    float holdBreathScale;
    int32_t holdBreathTimer;
    float moveSpeedScaleMultiplier;
    MantleState mantleState;
    float meleeChargeYaw;
    int32_t meleeChargeDist;
    int32_t meleeChargeTime;
    int32_t perks;
    ActionSlotType actionSlotType[4];
    ActionSlotParam actionSlotParam[4];
    int32_t entityEventSequence;
    int32_t weapAnim;
    float aimSpreadScale;
    int32_t shellshockIndex;
    int32_t shellshockTime;
    int32_t shellshockDuration;
    float dofNearStart;
    float dofNearEnd;
    float dofFarStart;
    float dofFarEnd;
    float dofNearBlur;
    float dofFarBlur;
    float dofViewmodelStart;
    float dofViewmodelEnd;
    int32_t hudElemLastAssignedSoundID;
    objective_t objective[16];          // XREF: _memmove+2E8/o
    // _memcpy+2E8/o ...
    uint8_t weaponmodels[128];
    int32_t deltaTime;
    int32_t killCamEntity;                  // XREF: SpectatorClientEndFrame(gentity_s *)+163/w
    // SpectatorClientEndFrame(gentity_s *)+17B/w
    playerState_s_hud hud;
    // XREF: SV_Shutdown(char const *):loc_5D1039/o
    // TRACK_sv_main(void)+A/o ...
};
static_assert(sizeof(playerState_s) == 0x2F64);

#elif KISAK_SP
/* 9184 */
struct playerState_s
{
    int commandTime;
    int pm_type;
    int bobCycle;
    int pm_flags;
    int weapFlags;
    int otherFlags;
    int pm_time;
    float origin[3];
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
    float jumpOriginZ;
    int movementDir;
    int eFlags;
    int eventSequence;
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
    int stats[4];
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
    int vehicleType;
    float linkAngles[3];
    float groundTiltAngles[3];
    int cursorHint;
    int cursorHintString;
    int cursorHintEntIndex;
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
    int weapLockFlags;
    int weapLockedEntnum;
    unsigned int forcedViewAnimWeaponIdx;
    int forcedViewAnimWeaponState;
    unsigned int forcedViewAnimOriginalWeaponIdx;
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
    unsigned __int8 weaponmodels[128];
    playerState_s_hud hud;
};
#endif

struct CEntPlayerInfo // sizeof=0xC
{                                       // ...
    clientControllers_t* control;       // ...
    uint8_t tag[6];             // ...
    // padding byte
    // padding byte
};
static_assert(sizeof(CEntPlayerInfo) == 0xC);

struct CEntTurretAngles // sizeof=0x8
{                                       // ...
    float pitch;
    float yaw;
};
static_assert(sizeof(CEntTurretAngles) == 0x8);

struct CEntTurretInfo // sizeof=0x10
{                                       // ...
    union
    {
        CEntTurretAngles angles;
        const float *viewAngles;
    };
    float barrelPitch;
    bool playerUsing;
    uint8_t tag_aim;
    uint8_t tag_aim_animated;
    uint8_t tag_flash;
};
static_assert(sizeof(CEntTurretInfo) == 0x10);

struct CEntVehicleInfo // sizeof=0x24
{                                       // ...
    int16_t pitch;
    int16_t yaw;
    int16_t roll;
    int16_t barrelPitch;
    float barrelRoll;
    int16_t steerYaw;
    // padding byte
    // padding byte
    float time;
    uint16_t wheelFraction[4];
    uint8_t wheelBoneIndex[4];
    uint8_t tag_body;
    uint8_t tag_turret;
    uint8_t tag_barrel;
    // padding byte
};
static_assert(sizeof(CEntVehicleInfo) == 0x24);

struct CEntFx // sizeof=0x8  (SP/MP Same)
{                                       // ...
    int32_t triggerTime;
    FxEffect* effect;
};
static_assert(sizeof(CEntFx) == 0x8);

#ifdef KISAK_MP
struct GfxSkinCacheEntry // sizeof=0xC
{                                       // ...
    uint32_t frameCount;
    int skinnedCachedOffset;
    uint16_t numSkinnedVerts;
    uint16_t ageCount;
};
static_assert(sizeof(GfxSkinCacheEntry) == 0xC);
struct cpose_t // sizeof=0x64
{                                       // ...
    uint16_t lightingHandle;
    uint8_t eType;
    uint8_t eTypeUnion;
    uint8_t localClientNum;
    volatile uint32_t cullIn;
    uint8_t isRagdoll;
    int32_t ragdollHandle;
    int32_t killcamRagdollHandle;
    int32_t physObjId;
    float origin[3];
    float angles[3];
    GfxSkinCacheEntry skinCacheEntry;
    union //$9D88A49AD898204B3D6E378457DD8419 // sizeof=0x24
    {                                       // ...
        CEntPlayerInfo player;
        CEntTurretInfo turret;
        CEntVehicleInfo vehicle;
        CEntFx fx;
    };
};
static_assert(sizeof(cpose_t) == 0x64);
#elif KISAK_SP
struct CEntActorInfo
{
    int proneType;
    float pitch;
    float roll;
    float height;
};
struct cpose_t
{
    unsigned __int16 lightingHandle;
    unsigned __int8 eType;
    unsigned __int8 eTypeUnion;
    bool isRagdoll;
    int ragdollHandle;
    int physObjId;
    volatile int cullIn;
    float origin[3];
    float angles[3];
    //$51809EA76892896F64281DFB626CE797 ___u9;
    union //$51809EA76892896F64281DFB626CE797
    {
        CEntActorInfo actor;
        CEntTurretInfo turret;
        CEntVehicleInfo vehicle;
        CEntFx fx;
    };
};
#endif

struct centity_s // sizeof=0x1DC
{                                       // ...
    cpose_t pose;
    LerpEntityState currentState;
    entityState_s nextState;
    bool nextValid;
    bool bMuzzleFlash;
    bool bTrailMade;
#ifdef KISAK_MP
    // padding byte
    int32_t previousEventSequence;
    int32_t miscTime;
    float lightingOrigin[3];
    XAnimTree_s* tree;
#else
    unsigned __int8 oldEType;
    int previousEventSequence;
    float lightingOrigin[3];
#endif
};

struct turretInfo_s // sizeof=0x48
{                                       // ...
    int32_t inuse;                          // ...
    int32_t flags;
    int32_t fireTime;
    float arcmin[2];
    float arcmax[2];
    float dropPitch;
    int32_t stance;
    int32_t prevStance;
    int32_t fireSndDelay;
    float userOrigin[3];
    float playerSpread;
    float pitchCap;
    int32_t triggerDown;
    uint8_t fireSnd;
    uint8_t fireSndPlayer;
    uint8_t stopSnd;
    uint8_t stopSndPlayer;
};
static_assert(sizeof(turretInfo_s) == 0x48);

struct VehicleRideSlot_t // sizeof=0xC
{                                       // ...
    uint32_t tagName;
    int32_t boneIdx;
    int32_t entNum;
};
static_assert(sizeof(VehicleRideSlot_t) == 0xC);

struct vehicle_node_t // sizeof=0x44
{                                       // ...
    uint16_t name;
    uint16_t target;
    uint16_t script_linkname;
    uint16_t script_noteworthy;
    int16_t index;
    // padding byte
    // padding byte
    int32_t rotated;
    float speed;
    float lookAhead;
    float origin[3];
    float dir[3];
    float angles[3];
    float length;
    int16_t nextIdx;
    int16_t prevIdx;
};
static_assert(sizeof(vehicle_node_t) == 0x44);

struct vehicle_pathpos_t // sizeof=0xC0
{                                       // ...
    int16_t nodeIdx;
    int16_t endOfPath;
    float frac;
    float speed;
    float lookAhead;
    float slide;
    float origin[3];
    float angles[3];
    float lookPos[3];
    vehicle_node_t switchNode[2];
};
static_assert(sizeof(vehicle_pathpos_t) == 0xC0);

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
    int32_t onGround;
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
    int32_t wheelSurfType[4];
    float worldTilt[3];
    float worldTiltVel[3];
};
static_assert(sizeof(vehicle_physic_t) == 0xF8);

struct VehicleTags // sizeof=0x60
{                                       // ...
    VehicleRideSlot_t riderSlots[3];
    int32_t detach;
    int32_t popout;
    int32_t body;
    int32_t turret;
    int32_t turret_base;
    int32_t barrel;
    int32_t flash[5];
    int32_t wheel[4];
};
static_assert(sizeof(VehicleTags) == 0x60);

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
    int32_t fireTime;
    int32_t fireBarrel;
    float barrelOffset;
    int32_t barrelBlocked;
    VehicleTurretState turretState;
};
static_assert(sizeof(VehicleTurret) == 0x14);

struct VehicleJitter // sizeof=0x3C
{                                       // ...
    int32_t jitterPeriodMin;
    int32_t jitterPeriodMax;
    int32_t jitterEndTime;
    float jitterOffsetRange[3];
    float jitterDeltaAccel[3];
    float jitterAccel[3];
    float jitterPos[3];
};
static_assert(sizeof(VehicleJitter) == 0x3C);

struct VehicleHover // sizeof=0x1C
{                                       // ...
    float hoverRadius;
    float hoverSpeed;
    float hoverAccel;
    float hoverGoalPos[3];
    int32_t useHoverAccelForAngles;
};
static_assert(sizeof(VehicleHover) == 0x1C);

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
    int32_t timeOfBirth;
    float travelDist;
    float surfaceNormal[3];
    team_t team;
    float curvature[3];
    float targetOffset[3];
    MissileStage stage;
    MissileFlightMode flightMode;
};
static_assert(sizeof(missile_ent_t) == 0x3C);

struct scr_vehicle_s // sizeof=0x354
{                                       // ...
    vehicle_pathpos_t pathPos;
    vehicle_physic_t phys;
    int32_t entNum;                         // ...
    int16_t infoIdx;
    // padding byte
    // padding byte
    int32_t flags;
    int32_t team;
    VehicleMoveState moveState;
    int16_t waitNode;
    // padding byte
    // padding byte
    float waitSpeed;
    VehicleTurret turret;
    VehicleJitter jitter;
    VehicleHover hover;
    int32_t drawOnCompass;
    uint16_t lookAtText0;
    uint16_t lookAtText1;
    int32_t manualMode;
    float manualSpeed;
    float manualAccel;
    float manualDecel;
    float manualTime;
    float speed;
    float maxDragSpeed;
    float turningAbility;
    int32_t hasTarget;
    int32_t hasTargetYaw;
    int32_t hasGoalYaw;
    int32_t stopAtGoal;
    int32_t stopping;
    int32_t targetEnt;
    EntHandle lookAtEnt;
    float targetOrigin[3];
    float targetOffset[3];
    float targetYaw;
    float goalPosition[3];
    float goalYaw;
    float prevGoalYaw;
    float yawOverShoot;
    int32_t yawSlowDown;
    float nearGoalNotifyDist;
    float joltDir[2];
    float joltTime;
    float joltWave;
    float joltSpeed;
    float joltDecel;
    int32_t playEngineSound;
    EntHandle idleSndEnt;
    EntHandle engineSndEnt;
    float idleSndLerp;
    float engineSndLerp;
    VehicleTags boneIndex;
    int32_t turretHitNum;
    float forcedMaterialSpeed;
};
static_assert(sizeof(scr_vehicle_s) == 0x354);

#ifdef KISAK_MP
struct entityShared_t // sizeof=0x68
{                                       // ...
    uint8_t linked;
    uint8_t bmodel;
    uint8_t svFlags;
    int32_t clientMask[2];
    uint8_t inuse;              // ...
    int32_t broadcastTime;
    float mins[3];                      // ...
    float maxs[3];
    int32_t contents;                       // ...
    float absmin[3];                    // ...
    float absmax[3];
    float currentOrigin[3];             // ...
    float currentAngles[3];
    EntHandle ownerNum;
    int32_t eventTime;
};
static_assert(sizeof(entityShared_t) == 0x68);
#elif KISAK_SP
struct entityShared_t
{
    unsigned __int8 linked;
    unsigned __int8 bmodel;
    unsigned __int8 svFlags;
    unsigned __int8 eventType;
    unsigned __int8 inuse;
    float mins[3];
    float maxs[3];
    int contents;
    float absmin[3];
    float absmax[3];
    float currentOrigin[3];
    float currentAngles[3];
    EntHandle ownerNum;
    int eventTime;
};
#endif

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
static_assert(sizeof(gitem_s) == 0x4);

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
    int32_t iFrac;
    float fViewHeight;
    int32_t iOffset;
};
static_assert(sizeof(viewLerpWaypoint_s) == 0xC);

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
uint32_t __cdecl BG_AnimationIndexForString(const char *string);
int32_t __cdecl BG_StringHashValue(const char *fname);
animScriptParseMode_t __cdecl BG_IndexForString(const char *token, animStringItem_t *strings, int32_t allowFail);
void __cdecl BG_InitWeaponString(int32_t index, const char *name);
void __cdecl BG_InitWeaponStrings();
void __cdecl BG_ParseCommands(const char **input, animScriptItem_t *scriptItem, animScriptData_t *scriptData);
int32_t __cdecl GetValueForBitfield(uint32_t bitfield);
int32_t __cdecl BG_PlayAnim(
    playerState_s *ps,
    int32_t animNum,
    animBodyPart_t bodyPart,
    int32_t forceDuration,
    int32_t setTimer,
    int32_t isContinue,
    int32_t force);
int32_t __cdecl BG_ExecuteCommand(
    playerState_s *ps,
    animScriptCommand_t *scriptCommand,
    int32_t setTimer,
    int32_t isContinue,
    int32_t force);
int32_t __cdecl BG_AnimScriptAnimation(playerState_s *ps, aistateEnum_t state, scriptAnimMoveTypes_t movetype, int32_t force);
animScriptItem_t *__cdecl BG_FirstValidItem(uint32_t client, animScript_t *script);
int32_t __cdecl BG_EvaluateConditions(clientInfo_t *ci, animScriptItem_t *scriptItem);
const char *__cdecl GetMoveTypeName(int32_t type);
const char *__cdecl GetWeaponTypeName(int32_t type);
const char *__cdecl GetBodyPart(int32_t bodypart);
int32_t __cdecl BG_AnimScriptEvent(playerState_s *ps, scriptAnimEventTypes_t event, int32_t isContinue, int32_t force);
void __cdecl BG_SetConditionValue(uint32_t client, uint32_t condition, uint64_t value);
const char *__cdecl BG_GetConditionString(int32_t condition, uint32_t value);
void __cdecl BG_SetConditionBit(uint32_t client, int32_t condition, int32_t value);
uint32_t __cdecl BG_GetConditionBit(const clientInfo_t *ci, uint32_t condition);
animScriptData_t *__cdecl BG_GetAnimationForIndex(int32_t client, uint32_t index);
void __cdecl BG_AnimUpdatePlayerStateConditions(pmove_t *pmove);
bool __cdecl BG_IsCrouchingAnim(const clientInfo_t *ci, int32_t animNum);
bool __cdecl BG_IsAds(const clientInfo_t *ci, int32_t animNum);
bool __cdecl BG_IsProneAnim(const clientInfo_t *ci, int32_t animNum);
bool __cdecl BG_IsKnifeMeleeAnim(const clientInfo_t *ci, int32_t animNum);
void __cdecl BG_LerpOffset(float *offset_goal, float maxOffsetChange, float *offset);
void __cdecl BG_Player_DoControllersSetup(const entityState_s *es, clientInfo_t *ci, int32_t frametime);
void __cdecl BG_Player_DoControllersInternal(const entityState_s *es, const clientInfo_t *ci, controller_info_t *info);
uint32_t __cdecl BG_GetConditionValue(const clientInfo_t *ci, uint32_t condition);
void __cdecl BG_LerpAngles(float *angles_goal, float maxAngleChange, float *angles);
void __cdecl BG_PlayerAnimation(int32_t localClientNum, const entityState_s *es, clientInfo_t *ci);
void __cdecl BG_RunLerpFrameRate(
    int32_t localClientNum,
    clientInfo_t *ci,
    lerpFrame_t *lf,
    int32_t newAnimation,
    const entityState_s *es);
void __cdecl BG_SetNewAnimation(
    int32_t localClientNum,
    clientInfo_t *ci,
    lerpFrame_t *lf,
    int32_t newAnimation,
    const entityState_s *es);
void __cdecl BG_PlayerAnimation_VerifyAnim(XAnimTree_s *pAnimTree, lerpFrame_t *lf);
void __cdecl BG_PlayerAngles(const entityState_s *es, clientInfo_t *ci);
void __cdecl BG_SwingAngles(
    float destination,
    float swingTolerance,
    float clampTolerance,
    float speed,
    float *angle,
    int32_t*swinging);
void __cdecl BG_AnimPlayerConditions(const entityState_s *es, clientInfo_t *ci);
void __cdecl BG_UpdatePlayerDObj(
    int32_t localClientNum,
    DObj_s *pDObj,
    entityState_s *es,
    clientInfo_t *ci,
    int32_t attachIgnoreCollision);
void __cdecl BG_LoadAnim();
void BG_FinalizePlayerAnims();
loadAnim_t *__cdecl BG_LoadAnimForAnimIndex(uint32_t iAnimIndex);
void __cdecl BG_SetupAnimNoteTypes(animScriptData_t *scriptData);
void __cdecl BG_AnimParseAnimScript(animScriptData_t *scriptData, loadAnim_t *pLoadAnims, uint32_t*piNumAnims);
char *__cdecl BG_CopyStringIntoBuffer(const char *string, char *buffer, uint32_t bufSize, uint32_t*offset);
void __cdecl BG_ParseConditionBits(
    const char **text_pp,
    animStringItem_t *stringTable,
    int32_t condIndex,
    uint32_t*result);
int32_t __cdecl BG_ParseConditions(const char **text_pp, animScriptItem_t *scriptItem);
void BG_FindAnims();
void BG_FindAnimTrees();
scr_animtree_t __cdecl BG_FindAnimTree(const char *filename, int32_t bEnforceExists);

extern bgs_t *bgs;

// bg_misc
struct WeaponDef;
void __cdecl BG_RegisterShockVolumeDvars();
void __cdecl BG_RegisterDvars();
char *__cdecl BG_GetEntityTypeName(int32_t eType);
const gitem_s *__cdecl BG_FindItemForWeapon(uint32_t weapon, int32_t model);
const gitem_s *__cdecl G_FindItem(const char *pickupName, int32_t model);
bool __cdecl BG_PlayerTouchesItem(const playerState_s *ps, const entityState_s *item, int32_t atTime);
bool __cdecl BG_PlayerCanPickUpWeaponType(const WeaponDef *weapDef, const playerState_s *ps);
bool __cdecl BG_CanItemBeGrabbed(const entityState_s *ent, const playerState_s *ps, int32_t touched);
char __cdecl WeaponEntCanBeGrabbed(
    const entityState_s *weaponEntState,
    const playerState_s *ps,
    int32_t touched,
    uint32_t weapIdx);
char __cdecl HaveRoomForAmmo(const playerState_s *ps, uint32_t weaponIndex);
bool __cdecl BG_PlayerHasRoomForEntAllAmmoTypes(const entityState_s *ent, const playerState_s *ps);
void __cdecl BG_EvaluateTrajectory(const trajectory_t *tr, int32_t atTime, float *result);
void __cdecl BG_EvaluateTrajectoryDelta(const trajectory_t *tr, int32_t atTime, float *result);
void __cdecl BG_AddPredictableEventToPlayerstate(uint32_t newEvent, uint32_t eventParm, playerState_s *ps);
void __cdecl BG_PlayerStateToEntityState(playerState_s *ps, entityState_s *s, int32_t snap, uint8_t handler);
void __cdecl BG_PlayerToEntityEventParm(playerState_s *ps, entityState_s *s);
void __cdecl BG_PlayerToEntityProcessEvents(playerState_s *ps, entityState_s *s, uint8_t handler);
void __cdecl BG_PlayerToEntitySetFlags(playerState_s *ps, entityState_s *s);
void __cdecl BG_PlayerToEntitySetPitchAngles(playerState_s *ps, entityState_s *s);
void __cdecl BG_PlayerToEntitySetMisc(playerState_s *ps, entityState_s *s);
void __cdecl BG_PlayerToEntitySetTrajectory(playerState_s *ps, entityState_s *s, int32_t snap);
char __cdecl BG_CheckProneValid(
    int32_t passEntityNum,
    const float *vPos,
    float fSize,
    float fHeight,
    float fYaw,
    float *pfTorsoPitch,
    float *pfWaistPitch,
    bool isAlreadyProne,
    bool isOnGround,
    bool groundIsWalkable,
    uint8_t handler,
    proneCheckType_t proneCheckType,
    float prone_feet_dist);
void __cdecl BG_GetPlayerViewOrigin(const playerState_s *ps, float *origin, int32_t time);
void __cdecl BG_GetPlayerViewDirection(const playerState_s *ps, float *forward, float *right, float *up);
char __cdecl BG_CheckProne(
    int32_t passEntityNum,
    const float *vPos,
    float fSize,
    float fHeight,
    float fYaw,
    float *pfTorsoPitch,
    float *pfWaistPitch,
    bool isAlreadyProne,
    bool isOnGround,
    bool groundIsWalkable,
    uint8_t handler,
    proneCheckType_t proneCheckType,
    float prone_feet_dist);
void __cdecl BG_LerpHudColors(const hudelem_s *elem, int32_t time, hudelem_color_t *toColor);
int32_t __cdecl BG_LoadShellShockDvars(const char *name);
void __cdecl BG_SetShellShockParmsFromDvars(shellshock_parms_t *parms);
int32_t __cdecl BG_SaveShellShockDvars(const char *name);
shellshock_parms_t *__cdecl BG_GetShellshockParms(uint32_t index);
void __cdecl BG_CreateXAnim(XAnim_s *anims, uint32_t animIndex, const char *name);
void __cdecl BG_CheckThread();
int32_t __cdecl BG_GetMaxSprintTime(const playerState_s *ps);

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

extern int32_t surfaceTypeSoundListCount;



// bg_perks_mp
uint32_t __cdecl BG_GetPerkIndexForName(const char *perkName);
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
    int32_t passEntityNum,
    int32_t contentMask);
void __cdecl PM_playerTrace(
    pmove_t *pm,
    trace_t *results,
    const float *start,
    const float *mins,
    const float *maxs,
    const float *end,
    int32_t passEntityNum,
    int32_t contentMask);
void __cdecl PM_AddEvent(playerState_s *ps, uint32_t newEvent);
void __cdecl PM_AddTouchEnt(pmove_t *pm, int32_t entityNum);
void __cdecl PM_ClipVelocity(const float *in, const float *normal, float *out);
void __cdecl PM_ProjectVelocity(const float *velIn, const float *normal, float *velOut);
int32_t __cdecl PM_GetEffectiveStance(const playerState_s *ps);
int32_t __cdecl PM_GetSprintLeft(const playerState_s *ps, int32_t gametime);
int32_t __cdecl PM_GetSprintLeftLastTime(const playerState_s *ps);
bool __cdecl PM_IsSprinting(const playerState_s *ps);
double __cdecl PM_DamageScale_Walk(int32_t damage_timer);
uint32_t __cdecl PM_GroundSurfaceType(pml_t *pml);
int32_t __cdecl PM_GetViewHeightLerpTime(const playerState_s *ps, int32_t iTarget, int32_t bDown);
bool __cdecl PlayerProneAllowed(pmove_t *pm);
void __cdecl PM_FootstepEvent(pmove_t *pm, pml_t *pml, char iOldBobCycle, char iNewBobCycle, int32_t bFootStep);
int32_t __cdecl PM_FootstepType(playerState_s *ps, pml_t *pml);
bool __cdecl PM_ShouldMakeFootsteps(pmove_t *pm);
void __cdecl PM_UpdateLean(
    playerState_s *ps,
    float msec,
    usercmd_s *cmd,
    void(__cdecl *capsuleTrace)(trace_t *, float *, float *, float *, float *, int32_t));
void __cdecl PM_UpdateViewAngles(playerState_s *ps, float msec, usercmd_s *cmd, uint8_t handler);
void __cdecl PM_UpdateViewAngles_Clamp(playerState_s *ps, usercmd_s *cmd);
void __cdecl PM_UpdateViewAngles_RangeLimited(playerState_s *ps, float oldYaw);
void __cdecl PM_UpdateViewAngles_LadderClamp(playerState_s *ps);
void __cdecl PM_UpdateViewAngles_Prone(
    playerState_s *ps,
    float msec,
    usercmd_s *cmd,
    uint8_t handler,
    float oldViewYaw);
int32_t __cdecl BG_CheckProneTurned(playerState_s *ps, float newProneYaw, uint8_t handler);
void __cdecl PM_UpdateViewAngles_ProneYawClamp(
    playerState_s *ps,
    float delta,
    int32_t proneBlocked,
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
void __cdecl PM_StartSprint(playerState_s *ps, pmove_t *pm, const pml_t *pml, int32_t sprintLeft);
void __cdecl PM_EndSprint(playerState_s *ps, pmove_t *pm);
bool __cdecl PM_SprintStartInterferingButtons(const playerState_s *ps, int32_t forwardSpeed, int16_t buttons);
bool __cdecl PM_SprintEndingButtons(const playerState_s *ps, int32_t forwardSpeed, int16_t buttons);
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
int32_t __cdecl PM_LightLandingForSurface(pml_t *pml);
int32_t __cdecl PM_MediumLandingForSurface(pml_t *pml);
uint32_t __cdecl PM_HardLandingForSurface(pml_t *pml);
uint32_t __cdecl PM_DamageLandingForSurface(pml_t *pml);
int32_t __cdecl PM_CorrectAllSolid(pmove_t *pm, pml_t *pml, trace_t *trace);
void __cdecl PM_GroundTraceMissed(pmove_t *pm, pml_t *pml);
double __cdecl PM_GetViewHeightLerp(const pmove_t *pm, int32_t iFromHeight, int32_t iToHeight);
bool __cdecl PM_IsPlayerFrozenByWeapon(const playerState_s *ps);
void __cdecl PM_CheckDuck(pmove_t *pm, pml_t *pml);
void __cdecl PM_ViewHeightAdjust(pmove_t *pm, pml_t *pml);
double __cdecl PM_ViewHeightTableLerp(int32_t iFrac, viewLerpWaypoint_s *pTable, float *pfPosOfs);
void __cdecl PM_Footsteps(pmove_t *pm, pml_t *pml);
int32_t __cdecl PM_GetStanceEx(int32_t stance, int32_t backward);
void __cdecl PM_Footstep_LadderMove(pmove_t *pm, pml_t *pml);
void __cdecl PM_Footsteps_NotMoving(pmove_t *pm, int32_t stance);
uint32_t __cdecl PM_GetFlinchAnim(uint32_t flinchAnimDir);
int32_t __cdecl PM_Footsteps_TurnAnim(clientInfo_t *ci);
scriptAnimMoveTypes_t __cdecl PM_GetNotMovingAnim(int32_t stance, int32_t turnAdjust);
bool __cdecl PM_ShouldFlinch(playerState_s *ps);
double __cdecl PM_GetMaxSpeed(pmove_t *pm, int32_t walking, int32_t sprinting);
double __cdecl PM_GetBobMove(PmStanceFrontBack stance, float xyspeed, float fMaxSpeed, int32_t walking, int32_t sprinting);
int32_t __cdecl PM_GetStanceIdleAnim(char stanceFlag);
int32_t __cdecl PM_GetMoveAnim(playerState_s *ps, PmStanceFrontBack stance, int32_t walking, int32_t sprinting);
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
double __cdecl BG_GetSpeed(const playerState_s *ps, int32_t time);


// bg_mantle
struct MantleResults // sizeof=0x38
{                                       // ...
    float dir[3];                       // ...
    float startPos[3];                  // ...
    float ledgePos[3];
    float endPos[3];
    int32_t flags;                          // ...
    int32_t duration;
};
static_assert(sizeof(MantleResults) == 0x38);

struct MantleAnimTransition // sizeof=0xC
{                                       // ...
    int32_t upAnimIndex;                    // ...
    int32_t overAnimIndex;                  // ...
    float height;                       // ...
};
static_assert(sizeof(MantleAnimTransition) == 0xC);

void __cdecl Mantle_RegisterDvars();
void __cdecl Mantle_CreateAnims(void *(__cdecl *xanimAlloc)(int32_t));
void __cdecl Mantle_ShutdownAnims();
void __cdecl Mantle_Check(pmove_t *pm, pml_t *pml);
void __cdecl Mantle_DebugPrint(const char *msg);
char __cdecl Mantle_CheckLedge(pmove_t *pm, pml_t *pml, MantleResults *mresults, float height);
void __cdecl Mantle_CalcEndPos(pmove_t *pm, MantleResults *mresults);
void __cdecl Mantle_Start(pmove_t *pm, playerState_s *ps, MantleResults *mresults);
int32_t __cdecl Mantle_GetUpLength(MantleState *mstate);
int32_t __cdecl Mantle_GetOverLength(MantleState *mstate);
void __cdecl Mantle_GetAnimDelta(MantleState *mstate, int32_t time, float *delta);
int32_t __cdecl Mantle_FindTransition(float curHeight, float goalHeight);
char __cdecl Mantle_FindMantleSurface(pmove_t *pm, pml_t *pml, trace_t *trace, float *mantleDir);
void __cdecl Mantle_Move(pmove_t *pm, playerState_s *ps, pml_t *pml);
int32_t __cdecl Mantle_GetAnim(MantleState *mstate);
void __cdecl Mantle_CapView(playerState_s *ps);
void __cdecl Mantle_ClearHint(playerState_s *ps);
bool __cdecl Mantle_IsWeaponInactive(playerState_s *ps);

// bg_weapons
struct BulletFireParams // sizeof=0x40
{                                       // ...
    int32_t weaponEntIndex;                 // ...
    int32_t ignoreEntIndex;                 // ...
    float damageMultiplier;             // ...
    int32_t methodOfDeath;                  // ...
    float origStart[3];                 // ...
    float start[3];                     // ...
    float end[3];                       // ...
    float dir[3];                       // ...
};
static_assert(sizeof(BulletFireParams) == 0x40);

struct BulletTraceResults // sizeof=0x44
{                                       // ...
    trace_t trace;                      // ...
    struct gentity_s *hitEnt;                  // ...
    float hitPos[3];                    // ...
    bool ignoreHitEnt;                  // ...
    // padding byte
    // padding byte
    // padding byte
    int32_t depthSurfaceType;               // ...
};
static_assert(sizeof(BulletTraceResults) == 0x44);

struct viewState_t // sizeof=0x24
{                                       // ...
    playerState_s *ps;                  // ...
    int32_t damageTime;                     // ...
    int32_t time;                           // ...
    float v_dmg_pitch;                  // ...
    float v_dmg_roll;                   // ...
    float xyspeed;                      // ...
    float frametime;                    // ...
    float fLastIdleFactor;              // ...
    int32_t*weapIdleTime;                  // ...
};
static_assert(sizeof(viewState_t) == 0x24);

struct weaponState_t // sizeof=0x54
{                                       // ...
    const playerState_s *ps;            // ...
    float xyspeed;                      // ...
    float frametime;                    // ...
    float vLastMoveAng[3];              // ...
    float fLastIdleFactor;              // ...
    int32_t time;                           // ...
    int32_t damageTime;                     // ...
    float v_dmg_pitch;                  // ...
    float v_dmg_roll;                   // ...
    float vGunOffset[3];                // ...
    float vGunSpeed[3];                 // ...
    float swayAngles[3];                // ...
    int32_t*weapIdleTime;                  // ...
};
static_assert(sizeof(weaponState_t) == 0x54);

void __cdecl TRACK_bg_weapons();
void __cdecl BG_LoadPenetrationDepthTable();
void __cdecl BG_ParsePenetrationDepthTable(const char *penetrateType, float *depthTable, char *buffer);
char __cdecl BG_AdvanceTrace(BulletFireParams *bp, BulletTraceResults *br, float dist);
double __cdecl BG_GetSurfacePenetrationDepth(const WeaponDef *weapDef, uint32_t surfaceType);
void __cdecl BG_ShutdownWeaponDefFiles();
void __cdecl BG_ClearWeaponDef();
void __cdecl BG_FillInAllWeaponItems();
void __cdecl BG_SetupWeaponIndex(uint32_t weapIndex);
void __cdecl BG_FillInWeaponItems(uint32_t weapIndex);
void __cdecl BG_SetupAmmoIndexes(uint32_t weapIndex);
void __cdecl BG_SetupSharedAmmoIndexes(uint32_t weapIndex);
void __cdecl BG_SetupClipIndexes(uint32_t weapIndex);
void __cdecl PM_StartWeaponAnim(playerState_s *ps, int32_t anim);
WeaponDef *__cdecl BG_GetWeaponDef(uint32_t weaponIndex);
uint32_t __cdecl BG_GetWeaponIndex(const WeaponDef *weapDef);
uint32_t __cdecl BG_GetNumWeapons();
int32_t __cdecl BG_GetSharedAmmoCapSize(uint32_t capIndex);
uint32_t __cdecl BG_FindWeaponIndexForName(const char *name);
uint32_t __cdecl BG_GetWeaponIndexForName(const char *name, void(__cdecl *regWeap)(uint32_t));
uint32_t __cdecl BG_SetupWeaponDef(WeaponDef *weapDef, void(__cdecl *regWeap)(uint32_t));
void __cdecl BG_SetupWeaponAlts(uint32_t weapIndex, void(__cdecl *regWeap)(uint32_t));
uint32_t __cdecl BG_GetViewmodelWeaponIndex(const playerState_s *ps);
int32_t __cdecl BG_GetFirstAvailableOffhand(const playerState_s *ps, int32_t offhandClass);
int32_t __cdecl BG_GetFirstEquippedOffhand(const playerState_s *ps, int32_t offhandClass);
int32_t __cdecl BG_IsAimDownSightWeapon(uint32_t weaponIndex);
bool __cdecl BG_CanPlayerHaveWeapon(uint32_t weaponIndex);
bool __cdecl BG_ValidateWeaponNumber(uint32_t weaponIndex);
bool __cdecl BG_IsWeaponValid(const playerState_s *ps, uint32_t weaponIndex);
bool __cdecl BG_WeaponBlocksProne(uint32_t weapIndex);
int32_t __cdecl BG_TakePlayerWeapon(playerState_s *ps, uint32_t weaponIndex, int32_t takeAwayAmmo);
int32_t __cdecl AmmoAfterWeaponRemoved(const playerState_s *ps, uint32_t weaponIndex);
int32_t __cdecl BG_GetAmmoPlayerMax(const playerState_s *ps, uint32_t weaponIndex, uint32_t weaponIndexToSkip);
int32_t __cdecl BG_GetMaxPickupableAmmo(const playerState_s *ps, uint32_t weaponIndex);
int32_t __cdecl BG_GetTotalAmmoReserve(const playerState_s *ps, uint32_t weaponIndex);
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
int32_t __cdecl PM_InteruptWeaponWithProneMove(playerState_s *ps);
int32_t __cdecl BG_ClipForWeapon(uint32_t weapon);
int32_t __cdecl BG_AmmoForWeapon(uint32_t weapon);
int32_t __cdecl BG_WeaponIsClipOnly(uint32_t weapon);
int32_t __cdecl BG_WeaponAmmo(const playerState_s *ps, uint32_t weapon);
int32_t __cdecl PM_WeaponAmmoAvailable(playerState_s *ps);
void __cdecl PM_AdjustAimSpreadScale(pmove_t *pm, pml_t *pml);
bool __cdecl ShotLimitReached(playerState_s *ps, WeaponDef *weapDef);
int32_t __cdecl PM_GetWeaponFireButton(uint32_t weapon);
void __cdecl PM_Weapon_Idle(playerState_s *ps);
void __cdecl PM_Weapon(pmove_t *pm, pml_t *pml);
void __cdecl PM_UpdateHoldBreath(pmove_t *pm, pml_t *pml);
void __cdecl PM_StartHoldBreath(playerState_s *ps);
void __cdecl PM_EndHoldBreath(playerState_s *ps);
int32_t __cdecl PM_Weapon_CheckForRechamber(playerState_s *ps, int32_t delayedAction);
void __cdecl PM_Weapon_FinishRechamber(playerState_s *ps);
void __cdecl PM_ContinueWeaponAnim(playerState_s *ps, int32_t anim);
void __cdecl PM_Weapon_FinishWeaponChange(pmove_t *pm, bool quick);
bool __cdecl PM_WeaponClipEmpty(playerState_s *ps);
void __cdecl PM_Weapon_BeginWeaponRaise(
    playerState_s *ps,
    uint32_t anim,
    uint32_t time,
    float aim,
    int32_t altSwitch);
void __cdecl BG_TakeClipOnlyWeaponIfEmpty(playerState_s *ps, int32_t weaponIndex);
void __cdecl PM_Weapon_FinishWeaponRaise(playerState_s *ps);
void __cdecl PM_Weapon_FinishReloadStart(pmove_t *pm, int32_t delayedAction);
void __cdecl PM_SetReloadingState(playerState_s *ps);
void __cdecl PM_SetWeaponReloadAddAmmoDelay(playerState_s *ps);
int32_t __cdecl PM_Weapon_AllowReload(playerState_s *ps);
void __cdecl PM_Weapon_ReloadDelayedAction(playerState_s *ps);
void __cdecl PM_ReloadClip(playerState_s *ps);
void __cdecl PM_Weapon_FinishReload(pmove_t *pm, int32_t delayedAction);
void __cdecl PM_Weapon_FinishReloadEnd(playerState_s *ps);
void __cdecl PM_Weapon_CheckForReload(pmove_t *pm);
void __cdecl PM_BeginWeaponReload(playerState_s *ps);
bool __cdecl BurstFirePending(playerState_s *ps);
void __cdecl UpdatePendingTriggerPull(pmove_t *pm);
int32_t __cdecl PM_Weapon_WeaponTimeAdjust(pmove_t *pm, pml_t *pml);
bool __cdecl WeaponUsesBurstCooldown(uint32_t weaponIdx);
void __cdecl PM_Weapon_CheckForChangeWeapon(pmove_t *pm);
void __cdecl PM_BeginWeaponChange(playerState_s *ps, uint32_t newweapon, bool quick);
int32_t __cdecl PM_Weapon_ShouldBeFiring(pmove_t *pm, int32_t delayedAction);
void __cdecl PM_Weapon_FireWeapon(playerState_s *ps, int32_t delayedAction);
void __cdecl PM_HoldBreathFire(playerState_s *ps);
void __cdecl PM_WeaponUseAmmo(playerState_s *ps, uint32_t wp, int32_t amount);
void __cdecl BG_SwitchWeaponsIfEmpty(playerState_s *ps);
void __cdecl PM_Weapon_StartFiring(playerState_s *ps, int32_t delayedAction);
int32_t __cdecl PM_Weapon_CheckFiringAmmo(playerState_s *ps);
void __cdecl PM_Weapon_SetFPSFireAnim(playerState_s *ps);
void __cdecl PM_Weapon_AddFiringAimSpreadScale(playerState_s *ps);
void __cdecl PM_Weapon_MeleeEnd(playerState_s *ps);
void __cdecl PM_Weapon_MeleeFire(playerState_s *ps);
void __cdecl PM_Weapon_CheckForMelee(pmove_t *pm, int32_t delayedAction);
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
char __cdecl PM_Weapon_CheckGrenadeHold(pmove_t *pm, int32_t delayedAction);
void __cdecl PM_Weapon_CheckForDetonation(pmove_t *pm);
void __cdecl PM_Weapon_CheckForGrenadeThrowCancel(pmove_t *pm);
void __cdecl PM_Detonate(playerState_s *ps, int32_t delayedAction);
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
int32_t __cdecl BG_CalculateWeaponPosition_GunRecoil_SingleAngle(
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
    int32_t frametime);
int32_t __cdecl BG_PlayerWeaponCountPrimaryTypes(const playerState_s *ps);
bool __cdecl BG_PlayerWeaponsFull_Primaries(const playerState_s *ps);
char __cdecl BG_PlayerHasCompatibleWeapon(const playerState_s *ps, uint32_t weaponIndex);
bool __cdecl BG_ThrowingBackGrenade(const playerState_s *ps);
WeaponDef *__cdecl BG_LoadWeaponDef(const char *name);
WeaponDef *__cdecl BG_LoadWeaponDef_FastFile(const char *name);
void __cdecl BG_AssertOffhandIndexOrNone(uint32_t offHandIndex);
void __cdecl BG_StringCopy(uint8_t *member, const char *keyValue);


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
void __cdecl PM_StepSlideMove(pmove_t *pm, pml_t *pml, int32_t gravity);
int32_t __cdecl PM_VerifyPronePosition(pmove_t *pm, float *vFallbackOrg, float *vFallbackVel);
bool __cdecl PM_SlideMove(pmove_t *pm, pml_t *pml, int32_t gravity);
double __cdecl PM_PermuteRestrictiveClipPlanes(
    const float *velocity,
    int32_t planeCount,
    const float (*planes)[3],
    int32_t *permutation);


// bg_weapons_load_obj
char *__cdecl BG_GetPlayerAnimTypeName(int32_t index);
void __cdecl TRACK_bg_weapons_load_obj();
const char *__cdecl BG_GetWeaponTypeName(weapType_t type);
const char *__cdecl BG_GetWeaponClassName(weapClass_t type);
const char *__cdecl BG_GetWeaponInventoryTypeName(weapInventoryType_t type);
void __cdecl BG_LoadWeaponStrings();
void __cdecl BG_LoadPlayerAnimTypes();
WeaponDef *__cdecl BG_LoadDefaultWeaponDef();
WeaponDef *__cdecl BG_LoadDefaultWeaponDef_FastFile();



// bg_misctables
extern gitem_s bg_itemlist[2048];
extern int itemRegistered[2048];

const float playerMins[] = { -15.0, -15.0, 0.0 };
const float playerMaxs[] = { 15.0, 15.0, 70.0 };