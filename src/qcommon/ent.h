#pragma once

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
    operator int()
    {
        return scale;
    }
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

struct archivedEntityShared_t // sizeof=0x24
{                                       // ...
    int svFlags;                        // ...
    int clientMask[2];                  // ...
    float absmin[3];                    // ...
    float absmax[3];                    // ...
};

struct archivedEntity_s // sizeof=0x118
{                                       // ...
    entityState_s s;                    // ...
    archivedEntityShared_t r;           // ...
};