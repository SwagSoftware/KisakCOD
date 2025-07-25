#pragma once

#include <universal/q_shared.h>

struct LerpEntityStateTurret // sizeof=0xC
{                                       // XREF: LerpEntityStateTypeUnion/r
    float gunAngles[3];
};
struct LerpEntityStateLoopFx // sizeof=0x8
{                                       // XREF: LerpEntityStateTypeUnion/r
    float cullDist;
    int period;
};
#ifdef KISAK_SP
struct __declspec(align(4)) LerpEntityStateActor
{
    float visionFov;
    float visionDist;
    unsigned __int8 species;
};
#endif
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
#ifdef KISAK_MP
    int movementDir;
#endif
};
struct LerpEntityStateVehicle // sizeof=0x1C
{                                       // XREF: LerpEntityStateTypeUnion/r
    float bodyPitch;
    float bodyRoll;
    float steerYaw;
    int materialTime;
    float gunPitch;
    float gunYaw;
#ifdef KISAK_MP
    int teamAndOwnerIndex;
#endif
};
struct LerpEntityStateMissile // sizeof=0x4
{                                       // XREF: LerpEntityStateTypeUnion/r
    int launchTime;
};
struct LerpEntityStateSoundBlend // sizeof=0x4
{                                       // XREF: LerpEntityStateTypeUnion/r
    float lerp;
#ifdef KISAK_SP
    float volumeScale;
#endif
};
#ifdef KISAK_SP
struct LerpEntityStateBulletTracer
{
    float end[3];
};
#endif
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
#ifdef KISAK_MP
struct LerpEntityStateAnonymous // sizeof=0x1C
{                                       // XREF: LerpEntityStateTypeUnion/r
    int data[7];
};
#endif
union LerpEntityStateTypeUnion // sizeof=0x1C
{                                       // XREF: LerpEntityState/r
    LerpEntityStateTurret turret;
    LerpEntityStateLoopFx loopFx;
#ifdef KISAK_SP
    LerpEntityStateActor actor;
#endif
    LerpEntityStatePrimaryLight primaryLight;
    LerpEntityStatePlayer player;
    LerpEntityStateVehicle vehicle;
    LerpEntityStateMissile missile;
    LerpEntityStateSoundBlend soundBlend;
#ifdef KISAK_SP
    LerpEntityStateBulletTracer bulletTracer;
#endif
    LerpEntityStateBulletHit bulletHit;
    LerpEntityStateEarthquake earthquake;
    LerpEntityStateCustomExplode customExplode;
    LerpEntityStateExplosion explosion;
    LerpEntityStateExplosionJolt explosionJolt;
    LerpEntityStatePhysicsJitter physicsJitter;
#ifdef KISAK_MP
    LerpEntityStateAnonymous anonymous;
#endif
};

struct LerpEntityState // sizeof=0x68
{                                       // XREF: entityState_s/r
    int eFlags;                         // XREF: ClientEndFrame(gentity_s *)+5D9/r
    trajectory_t pos;                   // XREF: Mantle_FindMantleSurface+244/o
    trajectory_t apos;                  // XREF: CountBitsEnabled(uint)+1B/o
    LerpEntityStateTypeUnion u;
#ifdef KISAK_SP
    int useCount;
#endif
};


#ifdef KISAK_MP
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

struct entityState_s // sizeof=0xF4 // (KISAKTODO: should be in q_shared?)
{                                       // XREF: ?SV_SendClientGameState@@YAXPAUclient_t@@@Z/r
    int number;
    int eType;                          // XREF: BounceMissile+1ED/r
    LerpEntityState lerp;               // XREF: Mantle_FindMantleSurface+244/o
    int time2;
    int otherEntityNum;                 // XREF: CG_CompassUpdateActors(int)+540/o
    int attackerEntityNum;
    int groundEntityNum;
    int loopSound;
    int surfType;
    entityState_s_type_index index;
    int clientNum;
    int iHeadIcon;
    int iHeadIconTeam;                  // XREF: G_InitGrenadeEntity(gentity_s *,gentity_s *)+218/o
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
    entityState_s_un2 un2;
    float fTorsoPitch;
    float fWaistPitch;
    unsigned int partBits[4];           // XREF: Fire_Lead:loc_5189EC/o
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
#endif

#ifdef KISAK_SP

enum entityType_t : __int32
{
    ET_GENERAL = 0x0,
    ET_PLAYER = 0x1,
    ET_ITEM = 0x2,
    ET_MISSILE = 0x3,
    ET_INVISIBLE = 0x4,
    ET_SCRIPTMOVER = 0x5,
    ET_SOUND_BLEND = 0x6,
    ET_FX = 0x7,
    ET_LOOP_FX = 0x8,
    ET_PRIMARY_LIGHT = 0x9,
    ET_MG42 = 0xA,
    ET_VEHICLE = 0xB,
    ET_VEHICLE_COLLMAP = 0xC,
    ET_VEHICLE_CORPSE = 0xD,
    ET_ACTOR = 0xE,
    ET_ACTOR_SPAWNER = 0xF,
    ET_ACTOR_CORPSE = 0x10,
    ET_EVENTS = 0x11,
};

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

union entityState_s_tag
{
    unsigned __int8 scale;
    unsigned __int8 eventParm2;
    unsigned __int8 vehicleCompassType;
};

union entityState_s_type_index
{
    uint16_t brushmodel;
    uint16_t item;
    uint16_t xmodel;
    uint16_t primaryLight;
};

struct entityState_s
{
    unsigned __int8 eType; // entityType_t
    unsigned __int8 surfType;
    unsigned __int8 weapon;
    unsigned __int8 weaponModel;
    entityState_s_tag un1;
    LerpEntityState lerp;
    unsigned int eventParm;
    unsigned __int16 loopSound;
    unsigned __int16 number;
    unsigned __int16 otherEntityNum;
    unsigned __int16 groundEntityNum;
    //_BYTE index[2];
    entityState_s_type_index index;
    int time2;
    int solid;
    int eventSequence;
    unsigned __int8 events[4];
    unsigned int eventParms[4];
    _BYTE un2[4];
};
#endif