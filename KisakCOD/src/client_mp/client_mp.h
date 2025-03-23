#pragma once
#include <qcommon/qcommon.h>
#include <xanim/xanim.h>

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
enum ViewLockTypes : __int32
{                                       // XREF: playerState_s/r
    PLAYERVIEWLOCK_NONE = 0x0,
    PLAYERVIEWLOCK_FULL = 0x1,
    PLAYERVIEWLOCK_WEAPONJITTER = 0x2,
    PLAYERVIEWLOCKCOUNT = 0x3,
};

enum ActionSlotType : __int32
{                                       // XREF: playerState_s/r
    ACTIONSLOTTYPE_DONOTHING = 0x0,
    ACTIONSLOTTYPE_SPECIFYWEAPON = 0x1,
    ACTIONSLOTTYPE_ALTWEAPONTOGGLE = 0x2,
    ACTIONSLOTTYPE_NIGHTVISION = 0x3,
    ACTIONSLOTTYPECOUNT = 0x4,
};
struct MantleState // sizeof=0x10
{                                       // XREF: playerState_s/r
    float yaw;
    int timer;
    int transIndex;
    int flags;
};

struct ActionSlotParam_SpecifyWeapon // sizeof=0x4
{                                       // XREF: ActionSlotParam/r
    unsigned int index;
};
struct ActionSlotParam // sizeof=0x4
{                                       // XREF: playerState_s/r
    ActionSlotParam_SpecifyWeapon specifyWeapon;
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
struct playerState_s_hud // sizeof=0x26C0
{                                       // XREF: playerState_s/r
    hudelem_s current[31];              // XREF: Sys_GetPhysicalCpuCount+131/o
                                        // unzlocal_CheckCurrentFileCoherencyHeader:loc_67D5A6/o ...
    hudelem_s archival[31];             // XREF: SV_Shutdown(char const *):loc_5D1039/o
                                        // TRACK_sv_main(void)+A/o ...
};

enum OffhandSecondaryClass : __int32
{
    PLAYER_OFFHAND_SECONDARY_SMOKE = 0x0,
    PLAYER_OFFHAND_SECONDARY_FLASH = 0x1,
    PLAYER_OFFHAND_SECONDARIES_TOTAL = 0x2,
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
struct clSnapshot_t // sizeof=0x2F94
{                                       // XREF: .data:newSnap/r
                                        // clientActive_t/r ...
    int valid;                          // XREF: CL_ParseSnapshot+AF/w
                                        // CL_ParseSnapshot:loc_4A5715/w ...
    int snapFlags;                      // XREF: CL_ParseSnapshot+A1/w
    int serverTime;                     // XREF: CL_ParseSnapshot+52/w
                                        // CL_ParseSnapshot+200/r ...
    int messageNum;                     // XREF: CL_ParseSnapshot+60/w
                                        // CL_ParseSnapshot:loc_4A55D6/r ...
    int deltaNum;                       // XREF: CL_ParseSnapshot+7A/w
                                        // CL_ParseSnapshot+8F/w ...
    int ping;
    int cmdNum;
    playerState_s ps;                   // XREF: CL_ParseSnapshot+1F4/o
                                        // CL_ParseSnapshot+21B/o ...
    int numEntities;
    int numClients;
    int parseEntitiesNum;
    int parseClientsNum;
    int serverCommandNum;               // XREF: CL_ParseSnapshot+41/w
};

enum team_t : __int32
{                                       // XREF: GetOtherTeamField/r
    TEAM_FREE = 0x0,
    TEAM_AXIS = 0x1,
    TEAM_ALLIES = 0x2,
    TEAM_SPECTATOR = 0x3,
    TEAM_NUM_TEAMS = 0x4,
};

struct clientState_s // sizeof=0x64
{                                       // XREF: ?MSG_WriteDeltaClient@@YAXPAUSnapshotInfo_s@@PAUmsg_t@@HPBUclientState_s@@2H@Z/r
                                        // ?MSG_ReadDeltaClient@@YAHPAUmsg_t@@HPBUclientState_s@@PAU2@H@Z/r ...
    int clientIndex;
    team_t team;                        // XREF: SpectatorClientEndFrame(gentity_s *):loc_4F9933/r
                                        // SpectatorClientEndFrame(gentity_s *):loc_4F9A78/r ...
    int modelindex;
    int attachModelIndex[6];            // XREF: FX_RestorePhysicsData+156/o
                                        // FX_SavePhysicsData+156/o ...
    int attachTagIndex[6];              // XREF: AimTarget_ProcessEntity(int,centity_s const *)+133/o
                                        // AimTarget_IsTargetValid+228/o ...
    char name[16];                      // XREF: FX_UpdateEffectBolt+E7/o
                                        // _memmove:UnwindDown2/o ...
    float maxSprintTimeMultiplier;      // XREF: RB_LogPrintState_0(int,int)+123/o
                                        // R_ChangeState_0(GfxCmdBufState *,uint)+2E6/o
    int rank;
    int prestige;
    int perks;
    int attachedVehEntNum;
    int attachedVehSlotIndex;           // XREF: .rdata:_hexc_10_32_table/o
};

struct gameState_t // sizeof=0x2262C
{                                       // XREF: clientActive_t/r
    int stringOffsets[2442];
    char stringData[131072];
    int dataCount;
};

enum StanceState : __int32
{                                       // XREF: ?CL_SetStance@@YAXHW4StanceState@@@Z/r
    CL_STANCE_STAND = 0x0,
    CL_STANCE_CROUCH = 0x1,
    CL_STANCE_PRONE = 0x2,
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

struct ClientArchiveData // sizeof=0x30
{                                       // XREF: clientActive_t/r
    int serverTime;
    float origin[3];
    float velocity[3];
    int bobCycle;
    int movementDir;
    float viewangles[3];
};

struct outPacket_t // sizeof=0xC
{                                       // XREF: clientActive_t/r
    int p_cmdNumber;
    int p_serverTime;
    int p_realtime;
};

enum trType_t : __int32
{                                       // XREF: trajectory_t/r
    TR_STATIONARY = 0x0,
    TR_INTERPOLATE = 0x1,
    TR_LINEAR = 0x2,
    TR_LINEAR_STOP = 0x3,
    TR_SINE = 0x4,
    TR_GRAVITY = 0x5,
    TR_ACCELERATE = 0x6,
    TR_DECELERATE = 0x7,
    TR_PHYSICS = 0x8,
    TR_FIRST_RAGDOLL = 0x9,
    TR_RAGDOLL = 0x9,
    TR_RAGDOLL_GRAVITY = 0xA,
    TR_RAGDOLL_INTERPOLATE = 0xB,
    TR_LAST_RAGDOLL = 0xB,
};

struct trajectory_t // sizeof=0x24
{                                       // XREF: LerpEntityState/r
                                        // LerpEntityState/r ...
    trType_t trType;                    // XREF: ScriptMover_SetupMoveSpeed+563/w
                                        // .data:00946420/o
    int trTime;                         // XREF: ScriptMover_SetupMoveSpeed+570/w
                                        // VEH_TouchEntities+1DB/o ...
    int trDuration;                     // XREF: ScriptMover_SetupMoveSpeed+583/w
                                        // ScriptMover_SetupMoveSpeed+642/r ...
    float trBase[3];                    // XREF: Mantle_FindMantleSurface+244/o
                                        // FX_UpdateElementPosition+5B/o ...
    float trDelta[3];                   // XREF: CountBitsEnabled(uint)+1B/o
                                        // CountBitsEnabled(uint)+77/o ...
};

struct LerpEntityStateTurret // sizeof=0xC
{                                       // XREF: LerpEntityStateTypeUnion/r
    float gunAngles[3];
};
struct LerpEntityStateLoopFx // sizeof=0x8
{                                       // XREF: LerpEntityStateTypeUnion/r
    float cullDist;
    int period;
};
struct LerpEntityStatePrimaryLight // sizeof=0x14
{                                       // XREF: LerpEntityStateTypeUnion/r
    unsigned __int8 colorAndExp[4];
    float intensity;
    float radius;
    float cosHalfFovOuter;
    float cosHalfFovInner;
};
struct LerpEntityStatePlayer // sizeof=0x8
{                                       // XREF: LerpEntityStateTypeUnion/r
    float leanf;
    int movementDir;
};
struct LerpEntityStateVehicle // sizeof=0x1C
{                                       // XREF: LerpEntityStateTypeUnion/r
    float bodyPitch;
    float bodyRoll;
    float steerYaw;
    int materialTime;
    float gunPitch;
    float gunYaw;
    int teamAndOwnerIndex;
};
struct LerpEntityStateMissile // sizeof=0x4
{                                       // XREF: LerpEntityStateTypeUnion/r
    int launchTime;
};
struct LerpEntityStateSoundBlend // sizeof=0x4
{                                       // XREF: LerpEntityStateTypeUnion/r
    float lerp;
};
struct LerpEntityStateBulletHit // sizeof=0xC
{                                       // XREF: LerpEntityStateTypeUnion/r
    float start[3];
};
struct LerpEntityStateEarthquake // sizeof=0xC
{                                       // XREF: LerpEntityStateTypeUnion/r
    float scale;
    float radius;
    int duration;
};
struct LerpEntityStateCustomExplode // sizeof=0x4
{                                       // XREF: LerpEntityStateTypeUnion/r
    int startTime;
};
struct LerpEntityStateExplosion // sizeof=0x8
{                                       // XREF: LerpEntityStateTypeUnion/r
    float innerRadius;
    float magnitude;
};
struct LerpEntityStateExplosionJolt // sizeof=0x10
{                                       // XREF: LerpEntityStateTypeUnion/r
    float innerRadius;
    float impulse[3];
};
struct LerpEntityStatePhysicsJitter // sizeof=0xC
{                                       // XREF: LerpEntityStateTypeUnion/r
    float innerRadius;
    float minDisplacement;
    float maxDisplacement;
};
struct LerpEntityStateAnonymous // sizeof=0x1C
{                                       // XREF: LerpEntityStateTypeUnion/r
    int data[7];
};
union LerpEntityStateTypeUnion // sizeof=0x1C
{                                       // XREF: LerpEntityState/r
    LerpEntityStateTurret turret;
    LerpEntityStateLoopFx loopFx;
    LerpEntityStatePrimaryLight primaryLight;
    LerpEntityStatePlayer player;
    LerpEntityStateVehicle vehicle;
    LerpEntityStateMissile missile;
    LerpEntityStateSoundBlend soundBlend;
    LerpEntityStateBulletHit bulletHit;
    LerpEntityStateEarthquake earthquake;
    LerpEntityStateCustomExplode customExplode;
    LerpEntityStateExplosion explosion;
    LerpEntityStateExplosionJolt explosionJolt;
    LerpEntityStatePhysicsJitter physicsJitter;
    LerpEntityStateAnonymous anonymous;
};

struct LerpEntityState // sizeof=0x68
{                                       // XREF: entityState_s/r
                                        // centity_s/r
    int eFlags;                         // XREF: ClientEndFrame(gentity_s *)+5D9/r
    trajectory_t pos;                   // XREF: Mantle_FindMantleSurface+244/o
                                        // FX_UpdateElementPosition+5B/o ...
    trajectory_t apos;                  // XREF: CountBitsEnabled(uint)+1B/o
                                        // CountBitsEnabled(uint)+77/o ...
    LerpEntityStateTypeUnion u;
};

union entityState_s_type_index // sizeof=0x4
{                                       // XREF: PM_UpdateLean(playerState_s *,float,usercmd_s *,void (*)(trace_t *,float const * const,float const * const,float const * const,float const * const,int,int))+2AC/o
    int brushmodel;
    int item;
    int xmodel;
    int primaryLight;
};

union entityState_s_un1 // sizeof=0x4
{                                       // XREF: IsValidArrayIndex(uint)+B/o
                                        // entityState_s/r
    int scale;
    int eventParm2;
    int helicopterStage;
};

union entityState_s_un2 // sizeof=0x4
{                                       // XREF: entityState_s/r
    int hintString;
    int vehicleXModel;
};

struct entityState_s // sizeof=0xF4
{                                       // XREF: ?SV_SendClientGameState@@YAXPAUclient_t@@@Z/r
                                        // archivedEntity_s/r ...
    int number;
    int eType;                          // XREF: BounceMissile+1ED/r
                                        // .rdata:0091FF68/o ...
    LerpEntityState lerp;               // XREF: Mantle_FindMantleSurface+244/o
                                        // CountBitsEnabled(uint)+1B/o ...
    int time2;
    int otherEntityNum;                 // XREF: CG_CompassUpdateActors(int)+540/o
    int attackerEntityNum;
    int groundEntityNum;
    int loopSound;
    int surfType;
    entityState_s_type_index index;
                                        // XREF: PM_UpdateLean(playerState_s *,float,usercmd_s *,void (*)(trace_t *,float const * const,float const * const,float const * const,float const * const,int,int))+2AC/o
                                        // CG_DrawMaterial+35/o ...
    int clientNum;
    int iHeadIcon;
    int iHeadIconTeam;                  // XREF: G_InitGrenadeEntity(gentity_s *,gentity_s *)+218/o
                                        // G_FireRocket(gentity_s *,uint,float * const,float * const,float const * const,gentity_s *,float const * const)+188/o ...
    int solid;
    unsigned int eventParm;
    int eventSequence;
    int events[4];                      // XREF: G_MoverPush+3FE/o
    int eventParms[4];
    int weapon;
    int weaponModel;
    int legsAnim;
    int torsoAnim;
    entityState_s_un1 un1;
                                        // XREF: IsValidArrayIndex(uint)+B/o
    entityState_s_un2 un2;
    float fTorsoPitch;
    float fWaistPitch;
    unsigned int partBits[4];           // XREF: Fire_Lead:loc_5189EC/o
                                        // turret_think(gentity_s *):loc_518D27/o ...
};

struct clientActive_t // sizeof=0x1B1BDC
{                                       // XREF: .data:clientActive_t * clients/r
    bool usingAds;
    // padding byte
    // padding byte
    // padding byte
    int timeoutcount;
    clSnapshot_t snap;
    bool alwaysFalse;
    // padding byte
    // padding byte
    // padding byte
    int serverTime;
    int oldServerTime;
    int oldFrameServerTime;
    int serverTimeDelta;
    int oldSnapServerTime;
    int extrapolatedSnapshot;
    int newSnapshots;
    gameState_t gameState;
    char mapname[64];
    int parseEntitiesNum;
    int parseClientsNum;
    int mouseDx[2];
    int mouseDy[2];
    int mouseIndex;
    bool stanceHeld;
    // padding byte
    // padding byte
    // padding byte
    StanceState stance;
    StanceState stancePosition;
    int stanceTime;
    int cgameUserCmdWeapon;
    int cgameUserCmdOffHandIndex;
    float cgameFOVSensitivityScale;
    float cgameMaxPitchSpeed;
    float cgameMaxYawSpeed;
    float cgameKickAngles[3];
    float cgameOrigin[3];
    float cgameVelocity[3];
    float cgameViewangles[3];
    int cgameBobCycle;
    int cgameMovementDir;
    int cgameExtraButtons;
    int cgamePredictedDataServerTime;
    float viewangles[3];
    int serverId;
    int skelTimeStamp;                  // XREF: CL_GetSkelTimeStamp(void)+E/r
    volatile int skelMemPos;            // XREF: CL_AllocSkelMemory(uint)+97/o
    char skelMemory[262144];
    char *skelMemoryStart;              // XREF: CL_AllocSkelMemory(uint)+66/r
                                        // CL_AllocSkelMemory(uint)+BB/r
    bool allowedAllocSkel;
    // padding byte
    // padding byte
    // padding byte
    usercmd_s cmds[128];
    int cmdNumber;
    ClientArchiveData clientArchive[256];
    int clientArchiveIndex;
    outPacket_t outPackets[32];
    clSnapshot_t snapshots[32];         // XREF: Sys_GetPhysicalCpuCount+131/o
                                        // RB_LogPrintState_0(int,int)+19D/o ...
    entityState_s entityBaselines[1024];
    entityState_s parseEntities[2048];  // XREF: CG_CompassUpdateActors(int)+540/o
                                        // CountBitsEnabled(uint)+1B/o ...
    clientState_s parseClients[2048];   // XREF: AimTarget_ProcessEntity(int,centity_s const *)+133/o
                                        // AimTarget_IsTargetValid+228/o ...
    int corruptedTranslationFile;
    char translationVersion[256];
    float vehicleViewYaw;
    float vehicleViewPitch;
};

extern	clientActive_t*	clients;


struct netProfilePacket_t // sizeof=0xC
{                                       // ...
    int iTime;
    int iSize;
    int bFragment;
};
struct netProfileStream_t // sizeof=0x2F0
{                                       // ...
    netProfilePacket_t packets[60];
    int iCurrPacket;
    int iBytesPerSecond;
    int iLastBPSCalcTime;
    int iCountedPackets;
    int iCountedFragments;
    int iFragmentPercentage;
    int iLargestPacket;
    int iSmallestPacket;
};
struct netProfileInfo_t // sizeof=0x5E0
{                                       // ...
    netProfileStream_t send;
    netProfileStream_t recieve;         // ...
};

struct netchan_t // sizeof=0x62C
{                                       // ...
    int outgoingSequence;
    netsrc_t sock;
    int dropped;
    int incomingSequence;
    netadr_t remoteAddress;             // ...
    int qport;
    int fragmentSequence;
    int fragmentLength;
    unsigned __int8 *fragmentBuffer;
    int fragmentBufferSize;
    int unsentFragments;
    int unsentFragmentStart;
    int unsentLength;
    unsigned __int8 *unsentBuffer;
    int unsentBufferSize;
    netProfileInfo_t prof;
};

struct clientConnection_t // sizeof=0x615E8
{                                       // ...
    int qport;
    int clientNum;
    int lastPacketSentTime;
    int lastPacketTime;
    netadr_t serverAddress;
    int connectTime;
    int connectPacketCount;
    char serverMessage[256];
    int challenge;
    int checksumFeed;
    int reliableSequence;
    int reliableAcknowledge;
    char reliableCommands[128][1024];
    int serverMessageSequence;
    int serverCommandSequence;
    int lastExecutedServerCommand;
    char serverCommands[128][1024];     // ...
    bool isServerRestarting;
    // padding byte
    // padding byte
    // padding byte
    int lastClientArchiveIndex;
    char demoName[64];
    int demorecording;
    int demoplaying;
    int isTimeDemo;
    int demowaiting;
    int firstDemoFrameSkipped;
    int demofile;
    int timeDemoLog;
    int timeDemoFrames;
    int timeDemoStart;
    int timeDemoPrev;
    int timeDemoBaseTime;
    netchan_t netchan;
    char netchanOutgoingBuffer[2048];
    char netchanIncomingBuffer[131072];
    netProfileInfo_t OOBProf;
    unsigned __int8 statPacketsToSend;
    // padding byte
    // padding byte
    // padding byte
    int statPacketSendTime[7];
};

struct GfxConfiguration // sizeof=0x30
{                                       // ...
    unsigned int maxClientViews;        // ...
    unsigned int entCount;              // ...
    unsigned int entnumNone;            // ...
    unsigned int entnumOrdinaryEnd;     // ...
    int threadContextCount;             // ...
    int critSectCount;
    const char *codeFastFileName;       // ...
    const char *uiFastFileName;         // ...
    const char *commonFastFileName;     // ...
    const char *localizedCodeFastFileName; // ...
    const char *localizedCommonFastFileName; // ...
    const char *modFastFileName;        // ...
};

struct ScreenPlacement // sizeof=0x44
{                                       // ...
    float scaleVirtualToReal[2];        // ...
    float scaleVirtualToFull[2];        // ...
    float scaleRealToVirtual[2];
    float virtualViewableMin[2];
    float virtualViewableMax[2];
    float realViewportSize[2];
    float realViewableMin[2];
    float realViewableMax[2];
    float subScreenLeft;
};

struct __declspec(align(4)) serverInfo_t // sizeof=0x94
{                                       // ...
    netadr_t adr;                       // ...
    unsigned __int8 allowAnonymous;     // ...
    unsigned __int8 bPassword;
    unsigned __int8 pure;
    char consoleDisabled;
    unsigned __int8 netType;            // ...
    unsigned __int8 clients;            // ...
    unsigned __int8 maxClients;         // ...
    unsigned __int8 dirty;              // ...
    char friendlyfire;
    char killcam;
    unsigned __int8 hardware;
    unsigned __int8 mod;
    unsigned __int8 voice;
    unsigned __int8 punkbuster;         // ...
    unsigned __int8 requestCount;       // ...
    // padding byte
    __int16 minPing;                    // ...
    __int16 maxPing;                    // ...
    __int16 ping;                       // ...
    char hostName[32];                  // ...
    char mapName[32];                   // ...
    char game[24];                      // ...
    char gameType[16];                  // ...
    // padding byte
    // padding byte
};

struct clientLogo_t // sizeof=0x18
{                                       // ...
    int startTime;                      // ...
    int duration;                       // ...
    int fadein;                         // ...
    int fadeout;                        // ...
    Material *material[2];              // ...
};
struct __declspec(align(4)) vidConfig_t // sizeof=0x30
{                                       // ...
    unsigned int sceneWidth;            // ...
    unsigned int sceneHeight;           // ...
    unsigned int displayWidth;          // ...
    unsigned int displayHeight;         // ...
    unsigned int displayFrequency;      // ...
    int isFullscreen;                   // ...
    float aspectRatioWindow;            // ...
    float aspectRatioScenePixel;        // ...
    float aspectRatioDisplayPixel;      // ...
    unsigned int maxTextureSize;        // ...
    unsigned int maxTextureMaps;        // ...
    bool deviceSupportsGamma;           // ...
    // padding byte
    // padding byte
    // padding byte
};

struct trDebugString_t // sizeof=0x80
{
    float xyz[3];
    float color[4];
    float scale;
    char text[96];
};
struct clientDebugStringInfo_t // sizeof=0x10
{                                       // ...
    int max;                            // ...
    int num;                            // ...
    trDebugString_t *strings;           // ...
    int *durations;                     // ...
};
struct trDebugLine_t // sizeof=0x2C
{
    float start[3];
    float end[3];
    float color[4];
    int depthTest;
};
struct clientDebugLineInfo_t // sizeof=0x10
{                                       // ...
    int max;                            // ...
    int num;                            // ...
    trDebugLine_t *lines;               // ...
    int *durations;                     // ...
};
struct clientDebug_t // sizeof=0x68
{                                       // ...
    int prevFromServer;
    int fromServer;                     // ...
    clientDebugStringInfo_t clStrings;  // ...
    clientDebugStringInfo_t svStringsBuffer; // ...
    clientDebugStringInfo_t svStrings;  // ...
    clientDebugLineInfo_t clLines;      // ...
    clientDebugLineInfo_t svLinesBuffer; // ...
    clientDebugLineInfo_t svLines;      // ...
};

struct clientStatic_t // sizeof=0x2DD070
{                                       // ...
    int quit;                           // ...
    int hunkUsersStarted;               // ...
    char servername[256];               // ...
    int rendererStarted;                // ...
    int soundStarted;                   // ...
    int uiStarted;                      // ...
    int devGuiStarted;                  // ...
    int frametime;                      // ...
    int realtime;                       // ...
    int realFrametime;                  // ...
    clientLogo_t logo;                  // ...
    float mapCenter[3];                 // ...
    int numlocalservers;                // ...
    serverInfo_t localServers[128];     // ...
    int waitglobalserverresponse;       // ...
    int numglobalservers;               // ...
    serverInfo_t globalServers[20000];  // ...
    int numfavoriteservers;             // ...
    serverInfo_t favoriteServers[128];  // ...
    int pingUpdateSource;               // ...
    netadr_t updateServer;
    char updateChallenge[1024];
    char updateInfoString[1024];        // ...
    netadr_t authorizeServer;           // ...
    Material *whiteMaterial;            // ...
    Material *consoleMaterial;          // ...
    Font_s *consoleFont;                // ...
    char autoupdateServerNames[5][64];  // ...
    netadr_t autoupdateServer;          // ...
    vidConfig_t vidConfig;              // ...
    clientDebug_t debug;                // ...
    int download;                       // ...
    char downloadTempName[256];         // ...
    char downloadName[256];             // ...
    int downloadNumber;
    int downloadBlock;                  // ...
    int downloadCount;                  // ...
    int downloadSize;                   // ...
    char downloadList[1024];            // ...
    int downloadRestart;                // ...
    int gameDirChanged;                 // ...
    int wwwDlDisconnected;              // ...
    int wwwDlInProgress;                // ...
    int downloadFlags;                  // ...
    char originalDownloadName[64];      // ...
    float debugRenderPos[3];            // ...
};

enum connstate_t : __int32
{                                       // ...
    CA_DISCONNECTED = 0x0,
    CA_CINEMATIC = 0x1,
    CA_LOGO = 0x2,
    CA_CONNECTING = 0x3,
    CA_CHALLENGING = 0x4,
    CA_CONNECTED = 0x5,
    CA_SENDINGSTATS = 0x6,
    CA_LOADING = 0x7,
    CA_PRIMED = 0x8,
    CA_ACTIVE = 0x9,


};

struct clientUIActive_t // sizeof=0x10
{
    bool active;
    bool isRunning;
    bool cgameInitialized;
    bool cgameInitCalled;
    int keyCatchers;
    bool displayHUDWithKeycatchUI;
    // padding byte
    // padding byte
    // padding byte
    connstate_t connectionState;
#ifdef _XBOX
    int nextScrollTime;
    bool invited;                       // XREF: CL_CheckForResend(int)+29C/r
                                         // Live_MPAcceptInvite+208/w ...
    // padding byte
    // padding byte
    // padding byte
    int numVoicePacketsSent;
#endif
};

struct ClientVoicePacket_t // sizeof=0x104
{                                       // ...
    unsigned __int8 data[256];
    int dataSize;
};
struct voiceCommunication_t // sizeof=0xA30
{                                       // ...
    ClientVoicePacket_t voicePackets[10];
    int voicePacketCount;
    int voicePacketLastTransmit;
};

struct ping_t // sizeof=0x41C
{                                       // ...
    netadr_t adr;                       // ...
    int start;                          // ...
    int time;                           // ...
    char info[1024];                    // ...
};

//void __cdecl TRACK_cl_main();
char __cdecl CL_IsLocalClientActive(int localClientNum);
int __cdecl CL_LocalActiveIndexFromClientNum(int localClientNum);
int __cdecl CL_ControllerIndexFromClientNum(int clientIndex);
char __cdecl CL_AllLocalClientsDisconnected();
char __cdecl CL_AnyLocalClientChallenging();
const char *__cdecl CL_GetUsernameForLocalClient();
void __cdecl CL_AddReliableCommand(int localClientNum, const char *cmd);
void __cdecl CL_ShutdownDevGui();
void __cdecl CL_ShutdownHunkUsers();
void __cdecl CL_ShutdownAll();
char __cdecl CL_AnyLocalClientsRunning();
void __cdecl CL_MapLoading(const char *mapname);
void __cdecl CL_ResetSkeletonCache(int localClientNum);
void __cdecl CL_ClearState(int localClientNum);
void __cdecl CL_Disconnect(int localClientNum);
void __cdecl CL_ForwardCommandToServer(int localClientNum, const char *string);
void __cdecl CL_RequestAuthorization(netsrc_t localClientNum);
void __cdecl CL_ForwardToServer_f();
void __cdecl CL_Setenv_f();
void __cdecl CL_DisconnectLocalClient(int localClientNum);
void __cdecl CL_Reconnect_f();
void __cdecl CL_Vid_Restart_f();
void __cdecl CL_Snd_Restart_f();
void __cdecl CL_Configstrings_f();
void __cdecl CL_Clientinfo_f();
void __cdecl CL_DownloadsComplete(int localClientNum);
void __cdecl CL_CheckForResend(netsrc_t localClientNum);
int __cdecl CL_HighestPriorityStatPacket(clientConnection_t *clc);
void __cdecl CL_DisconnectError(char *message);
char __cdecl CL_ConnectionlessPacket(netsrc_t localClientNum, netadr_t from, msg_t *msg, int time);
char __cdecl CL_DispatchConnectionlessPacket(netsrc_t localClientNum, netadr_t from, msg_t *msg, int time);
void __cdecl CL_DisconnectPacket(int localClientNum, netadr_t from, char *reason);
void __cdecl CL_InitLoad(const char *mapname, const char *gametype);
char __cdecl CL_PacketEvent(netsrc_t localClientNum, netadr_t from, msg_t *msg, int time);
void __cdecl CL_VoiceTransmit(int localClientNum);
void __cdecl CL_RunOncePerClientFrame(int localClientNum, int msec);
void __cdecl CL_Frame(netsrc_t localClientNum);
void __cdecl CL_CheckTimeout(int localClientNum);
void __cdecl CL_ServerTimedOut();
void __cdecl CL_CheckUserinfo(int localClientNum);
void __cdecl CL_UpdateInGameState(int localClientNum);
void __cdecl CL_VoiceFrame(int localClientNum);
bool __cdecl CL_IsLocalClientInGame(int localClientNum);
char __cdecl CL_IsClientLocal(int clientNum);
void __cdecl CL_ParseBadPacket_f();
void __cdecl CL_ShutdownRef();
void __cdecl CL_InitRenderer();
void __cdecl CL_ShutdownRenderer(int destroyWindow);
void __cdecl CL_StartHunkUsers();
void CL_InitDevGui();
void __cdecl CL_DevGuiDvar_f();
void __cdecl CL_DevGuiCmd_f();
void __cdecl CL_DevGuiOpen_f();
int __cdecl CL_ScaledMilliseconds();
void __cdecl CL_InitRef();
void __cdecl SetupGfxConfig(GfxConfiguration *config);
void __cdecl CL_startSingleplayer_f();
void __cdecl CL_DrawLogo(int localClientNum);
void __cdecl CL_StopLogo(int localClientNum);
void __cdecl CL_PlayLogo_f();
void __cdecl CL_StopLogoOrCinematic(int localClientNum);
void __cdecl CL_ToggleMenu_f();
void __cdecl CL_InitOnceForAllClients();
void __cdecl CL_Disconnect_f();
void __cdecl CL_Init(int localClientNum);
int __cdecl CountBitsEnabled(unsigned int num);
void __cdecl CL_Shutdown(int localClientNum);
void __cdecl CL_LocalServers_f();
void __cdecl CL_GetPing(int n, char *buf, int buflen, int *pingtime);
void __cdecl CL_ClearPing(unsigned int n);
int __cdecl CL_GetPingQueueCount();
int __cdecl CL_UpdateDirtyPings(netsrc_t localClientNum, unsigned int source);
void __cdecl CL_ShowIP_f();
void __cdecl CL_SetupForNewServerMap(char *pszMapName, char *pszGametype);
bool __cdecl CL_IsServerLoadingMap();
bool __cdecl CL_IsWaitingOnServerToLoadMap(int localClientNum);
void __cdecl CL_SetWaitingOnServerToLoadMap(int localClientNum, bool waiting);
void __cdecl CL_DrawTextPhysical(
    char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    const float *color,
    int style);
void __cdecl CL_DrawTextPhysicalWithEffects(
    char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    const float *color,
    int style,
    const float *glowColor,
    struct Material *fxMaterial,
    struct Material *fxMaterialGlow,
    int fxBirthTime,
    int fxLetterTime,
    int fxDecayStartTime,
    int fxDecayDuration);
void __cdecl CL_DrawText(
    const ScreenPlacement *scrPlace,
    char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    int horzAlign,
    int vertAlign,
    float xScale,
    float yScale,
    const float *color,
    int style);
void __cdecl CL_DrawTextRotate(
    const ScreenPlacement *scrPlace,
    char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    float rotation,
    int horzAlign,
    int vertAlign,
    float xScale,
    float yScale,
    const float *color,
    int style);
void __cdecl CL_DrawTextPhysicalWithCursor(
    char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    float xScale,
    float yScale,
    const float *color,
    int style,
    int cursorPos,
    char cursor);
void __cdecl CL_DrawTextWithCursor(
    const ScreenPlacement *scrPlace,
    char *text,
    int maxChars,
    Font_s *font,
    float x,
    float y,
    int horzAlign,
    int vertAlign,
    float xScale,
    float yScale,
    const float *color,
    int style,
    int cursorPos,
    char cursor);
bool __cdecl CL_ShouldDisplayHud(int localClientNum);
bool __cdecl CL_IsUIActive(int localClientNum);
struct Font_s *__cdecl CL_RegisterFont(const char *fontName, int imageTrack);
void __cdecl CL_UpdateSound();
float (*__cdecl CL_GetMapCenter())[3];
void __cdecl CL_ResetStats_f();
int __cdecl CL_GetLocalClientActiveCount();

#define MAX_CLIENTS 1 // LWSS Add

extern clientConnection_t clientConnections[MAX_CLIENTS];
extern clientUIActive_t clientUIActives[MAX_CLIENTS];
extern clientActive_t clients[MAX_CLIENTS];
extern dvar_t *name;

extern clientStatic_t cls;