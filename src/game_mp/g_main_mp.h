#pragma once

#include <qcommon/qcommon.h>
#include <universal/q_parse.h>
#include <client_mp/client_mp.h>

enum hitLocation_t : int32_t
{                                       // ...
    HITLOC_NONE = 0x0,
    HITLOC_HELMET = 0x1,
    HITLOC_HEAD = 0x2,
    HITLOC_NECK = 0x3,
    HITLOC_TORSO_UPR = 0x4,
    HITLOC_TORSO_LWR = 0x5,
    HITLOC_R_ARM_UPR = 0x6,
    HITLOC_L_ARM_UPR = 0x7,
    HITLOC_R_ARM_LWR = 0x8,
    HITLOC_L_ARM_LWR = 0x9,
    HITLOC_R_HAND = 0xA,
    HITLOC_L_HAND = 0xB,
    HITLOC_R_LEG_UPR = 0xC,
    HITLOC_L_LEG_UPR = 0xD,
    HITLOC_R_LEG_LWR = 0xE,
    HITLOC_L_LEG_LWR = 0xF,
    HITLOC_R_FOOT = 0x10,
    HITLOC_L_FOOT = 0x11,
    HITLOC_GUN = 0x12,
    HITLOC_NUM = 0x13,
};

struct entityHandler_t // sizeof=0x28
{
    void(__cdecl *think)(gentity_s *);
    void(__cdecl *reached)(gentity_s *);
    void(__cdecl *blocked)(gentity_s *, gentity_s *);
    void(__cdecl *touch)(gentity_s *, gentity_s *, int32_t);
    void(__cdecl *use)(gentity_s *, gentity_s *, gentity_s *);
    void(__cdecl *pain)(gentity_s *, gentity_s *, int32_t, const float *, const int32_t, const float *, const hitLocation_t, const int32_t);
    void(__cdecl *die)(gentity_s *, gentity_s *, gentity_s *, int32_t, int32_t, const int, const float *, const hitLocation_t, int32_t);
    void(__cdecl *controller)(const gentity_s *, int32_t *);
    int32_t methodOfDeath;
    int32_t splashMethodOfDeath;
};
static_assert(sizeof(entityHandler_t) == 0x28);

struct SpawnVar // sizeof=0xA0C
{                                       // ...
    bool spawnVarsValid;                // ...
    // padding byte
    // padding byte
    // padding byte
    int32_t numSpawnVars;                   // ...
    char *spawnVars[64][2];             // ...
    int32_t numSpawnVarChars;
    char spawnVarChars[2048];
};
static_assert(sizeof(SpawnVar) == 0xA0C);

struct trigger_info_t // sizeof=0xC
{                                       // ...
    uint16_t entnum;
    uint16_t otherEntnum;
    int32_t useCount;
    int32_t otherUseCount;
};
static_assert(sizeof(trigger_info_t) == 0xC);

struct cached_tag_mat_t // sizeof=0x3C
{                                       // ...
    int32_t time;
    int32_t entnum;
    uint16_t name;              // ...
    // padding byte
    // padding byte
    float tagMat[4][3];                 // ...
};
static_assert(sizeof(cached_tag_mat_t) == 0x3C);

struct level_locals_t // sizeof=0x2E6C
{                                       // ...
    gclient_s *clients;                 // ...
    gentity_s *gentities;               // ...
    int32_t gentitySize;
    int32_t num_entities;                   // ...
    gentity_s *firstFreeEnt;            // ...
    gentity_s *lastFreeEnt;             // ...
    int32_t logFile;                        // ...
    int32_t initializing;                   // ...
    int32_t clientIsSpawning;               // ...
    objective_t objectives[16];         // ...
    int32_t maxclients;                     // ...
    int32_t framenum;                       // ...
    int32_t time;                           // ...
    int32_t previousTime;                   // ...
    int32_t frametime;                      // ...
    int32_t startTime;                      // ...
    int32_t teamScores[4];                  // ...
    int32_t lastTeammateHealthTime;         // ...
    int32_t bUpdateScoresForIntermission;   // ...
    bool teamHasRadar[4];               // ...
    int32_t manualNameChange;               // ...
    int32_t numConnectedClients;            // ...
    int32_t sortedClients[64];              // ...
    char voteString[1024];              // ...
    char voteDisplayString[1024];       // ...
    int32_t voteTime;                       // ...
    int32_t voteExecuteTime;                // ...
    int32_t voteYes;                        // ...
    int32_t voteNo;                         // ...
    int32_t numVotingClients;               // ...
    SpawnVar spawnVar;                  // ...
    int32_t savepersist;                    // ...
    EntHandle droppedWeaponCue[32];
    float fFogOpaqueDist;               // ...
    float fFogOpaqueDistSqrd;           // ...
    int32_t remapCount;
    int32_t currentPlayerClone;             // ...
    trigger_info_t pendingTriggerList[256]; // ...
    trigger_info_t currentTriggerList[256]; // ...
    int32_t pendingTriggerListSize;         // ...
    int32_t currentTriggerListSize;         // ...
    int32_t finished;                       // ...
    int32_t bPlayerIgnoreRadiusDamage;      // ...
    int32_t bPlayerIgnoreRadiusDamageLatched; // ...
    int32_t registerWeapons;                // ...
    int32_t bRegisterItems;                 // ...
    int32_t currentEntityThink;             // ...
    int32_t openScriptIOFileHandles[1];     // ...
    char *openScriptIOFileBuffers[1];   // ...
    com_parse_mark_t currentScriptIOLineMark[1]; // ...
    cached_tag_mat_t cachedTagMat;      // ...
    int32_t scriptPrintChannel;             // ...
    float compassMapUpperLeft[2];       // ...
    float compassMapWorldSize[2];       // ...
    float compassNorth[2];              // ...
    scr_vehicle_s *vehicles;            // ...
};
static_assert(sizeof(level_locals_t) == 0x2E6C);

void __cdecl TRACK_g_main();
int32_t __cdecl G_GetSavePersist();
void __cdecl G_SetSavePersist(int32_t savepersist);
double __cdecl G_GetFogOpaqueDistSqrd();
int32_t __cdecl G_GetClientScore(int32_t clientNum);
int32_t __cdecl G_GetClientArchiveTime(int32_t clientNum);
void __cdecl G_SetClientArchiveTime(int32_t clientNum, int32_t time);
clientState_s *__cdecl G_GetClientState(int32_t clientNum);
gclient_s *__cdecl G_GetPlayerState(int32_t clientNum);
int32_t __cdecl G_GetClientSize();
void __cdecl G_FreeEntities();
bool __cdecl G_ExitAfterConnectPaths();
int32_t __cdecl G_IsServerGameSystem(int32_t clientNum);
void __cdecl G_InitGame(int32_t levelTime, int32_t randomSeed, int32_t restart, int32_t savepersist);
const dvar_s *G_RegisterDvars();
void __cdecl G_CreateDObj(
    DObjModel_s *dobjModels,
    uint16_t numModels,
    XAnimTree_s *tree,
    uint32_t handle,
    int32_t unusedLocalClientNum);
DObj_s *__cdecl G_GetDObj(uint32_t handle, int32_t unusedLocalClientNum);
XAnimTree_s *G_LoadAnimTreeInstances();
void G_PrintAllFastFileErrors();
void __cdecl G_PrintFastFileErrors(const char *fastfile);
void __cdecl G_ShutdownGame(int32_t freeScripts);
int32_t G_FreeAnimTreeInstances();
void __cdecl SendScoreboardMessageToAllIntermissionClients();
void __cdecl CalculateRanks();
int32_t __cdecl SortRanks(uint32_t *a, uint32_t *b);
void __cdecl ExitLevel();
void G_LogPrintf(const char *fmt, ...);
void __cdecl CheckVote();
void __cdecl G_UpdateObjectiveToClients();
void __cdecl G_UpdateHudElemsToClients();
void __cdecl G_RunThink(gentity_s *ent);
void __cdecl DebugDumpAnims();
void __cdecl G_XAnimUpdateEnt(gentity_s *ent);
void __cdecl G_RunFrame(int32_t levelTime);
void __cdecl G_ClientDoPerFrameNotifies(gentity_s *ent);
bool __cdecl DoPerFrameNotify(
    gentity_s *ent,
    bool isCurrently,
    bool wasPreviously,
    uint16_t begin,
    uint16_t end);
const dvar_s *ShowEntityInfo();
void __cdecl ShowEntityInfo_Items(gentity_s *ent);
void __cdecl G_RunFrameForEntity(gentity_s *ent);
void __cdecl G_TraceCapsule(
    trace_t *results,
    const float *start,
    const float *mins,
    const float *maxs,
    const float *end,
    int32_t passEntityNum,
    int32_t contentmask);
int32_t __cdecl G_TraceCapsuleComplete(
    float *start,
    float *mins,
    float *maxs,
    float *end,
    int32_t passEntityNum,
    int32_t contentmask);
void __cdecl G_LocationalTrace(
    trace_t *results,
    float *start,
    float *end,
    int32_t passEntityNum,
    int32_t contentmask,
    uint8_t *priorityMap);
void __cdecl G_LocationalTraceAllowChildren(
    trace_t *results,
    float *start,
    float *end,
    int32_t passEntityNum,
    int32_t contentmask,
    uint8_t *priorityMap);
int32_t __cdecl G_LocationalTracePassed(
    float *start,
    float *end,
    int32_t passEntityNum,
    int32_t passEntityNum1,
    int32_t contentmask,
    uint8_t *priorityMap);
void __cdecl G_SightTrace(int32_t *hitNum, float *start, float *end, int32_t passEntityNum, int32_t contentmask);
void __cdecl G_AddDebugString(const float *xyz, const float *color, float scale, const char *text, int32_t duration);
BOOL __cdecl OnSameTeam(struct gentity_s *ent1, struct gentity_s *ent2);

extern const dvar_t *pickupPrints;
extern const dvar_t *g_password;
extern const dvar_t *g_smoothClients;
extern const dvar_t *g_voteAbstainWeight;
extern const dvar_t *g_TeamColor_MyTeam;
extern const dvar_t *g_gravity;
extern const dvar_t *g_filterBan;
extern const dvar_t *g_voiceChatsAllowed;
extern const dvar_t *g_deadChat;
extern const dvar_t *radius_damage_debug;
extern const dvar_t *g_dumpAnims;
extern const dvar_t *g_ScoresColor_Allies;
extern const dvar_t *g_friendlyfireDist;
extern const dvar_t *g_dropUpSpeedRand;
extern const dvar_t *g_maxclients;
extern const dvar_t *player_MGUseRadius;
extern const dvar_t *g_TeamName_Allies;
extern const dvar_t *g_debugBullets;
extern const dvar_t *g_synchronousClients;
extern const dvar_t *g_knockback;
extern const dvar_t *player_throwbackInnerRadius;
extern const dvar_t *g_ScoresColor_MyTeam;
extern const dvar_t *g_allowVote;
extern const dvar_t *anim_deltas_debug;
extern const dvar_t *g_dedicated;
extern const dvar_t *g_TeamColor_Allies;
extern const dvar_t *g_antilag;
extern const dvar_t *g_TeamIcon_Allies;
extern const dvar_t *g_playerCollisionEjectSpeed;
extern const dvar_t *g_entinfo;
extern const dvar_t *melee_debug;
extern const dvar_t *g_useholdspawndelay;
extern const dvar_t *g_TeamColor_Free;
extern const dvar_t *g_debugPlayerAnimScript;
extern const dvar_t *g_fogColorReadOnly;
extern const dvar_t *g_dropUpSpeedBase;
extern const dvar_t *g_listEntity;
extern const dvar_t *g_inactivity;
extern const dvar_t *g_TeamIcon_Spectator;
extern const dvar_t *g_redCrosshairs;
extern const dvar_t *g_cheats;
extern const dvar_t *g_TeamColor_Spectator;
extern const dvar_t *g_fogHalfDistReadOnly;
extern const dvar_t *g_maxDroppedWeapons;
extern const dvar_t *g_dropForwardSpeed;
extern const dvar_t *g_ScoresColor_Free;
extern const dvar_t *g_minGrenadeDamageSpeed;
extern const dvar_t *g_dropHorzSpeedRand;
extern const dvar_t *g_voiceChatTalkingDuration;
extern const dvar_t *g_ScoresColor_Spectator;
extern const dvar_t *g_useholdtime;
extern const dvar_t *g_ScoresColor_EnemyTeam;
extern const dvar_t *g_compassShowEnemies;
extern const dvar_t *g_speed;
extern const dvar_t *g_friendlyNameDist;
extern const dvar_t *g_log;
extern const dvar_t *g_TeamName_Axis;
extern const dvar_t *g_TeamIcon_Axis;
extern const dvar_t *bullet_penetrationEnabled;
extern const dvar_t *g_TeamColor_Axis;
extern const dvar_t *g_NoScriptSpam;
extern const dvar_t *g_banIPs;
extern const dvar_t *g_gametype;
extern const dvar_t *g_fogStartDistReadOnly;
extern const dvar_t *g_debugLocDamage;
extern const dvar_t *g_logSync;
extern const dvar_t *g_mantleBlockTimeBuffer;
extern const dvar_t *player_throwbackOuterRadius;
extern const dvar_t *g_oldVoting;
extern const dvar_t *g_ScoresColor_Axis;
extern const dvar_t *g_TeamIcon_Free;
extern const dvar_t *g_motd;
extern const dvar_t *g_TeamColor_EnemyTeam;
extern const dvar_t *g_debugDamage;
extern const dvar_t *g_clonePlayerMaxVelocity;

extern const dvar_t *voice_global;
extern const dvar_t *voice_localEcho;
extern const dvar_t *voice_deadChat;

extern gentity_s g_entities[1024];
extern level_locals_t level;
extern bgs_t level_bgs;


