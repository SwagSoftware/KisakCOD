#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

enum aiphys_t : __int32
{
    AIPHYS_BAD = 0x0,
    AIPHYS_NORMAL_ABSOLUTE = 0x1,
    AIPHYS_NORMAL_RELATIVE = 0x2,
    AIPHYS_NOCLIP = 0x3,
    AIPHYS_NOGRAVITY = 0x4,
    AIPHYS_ZONLY_PHYSICS_RELATIVE = 0x5,
};

enum SlideMoveResult : __int32
{
    SLIDEMOVE_COMPLETE = 0x0,
    SLIDEMOVE_CLIPPED = 0x1,
    SLIDEMOVE_FAIL = 0x2,
};

struct __declspec(align(4)) actor_physics_t
{
    float vOrigin[3];
    float vVelocity[3];
    unsigned __int16 groundEntNum;
    int iFootstepTimer;
    int bHasGroundPlane;
    float groundplaneSlope;
    int iSurfaceType;
    float vWishDelta[3];
    int bIsAlive;
    int iEntNum;
    aiphys_t ePhysicsType;
    float fGravity;
    int iMsec;
    float vMins[3];
    float vMaxs[3];
    bool prone;
    int iTraceMask;
    int foliageSoundTime;
    int iNumTouch;
    int iTouchEnts[32];
    int iHitEntnum;
    float vHitOrigin[2];
    float vHitNormal[2];
    unsigned __int8 bStuck;
    unsigned __int8 bDeflected;
};

void __cdecl TRACK_actor_physics();
void __cdecl AIPhys_AddTouchEnt(int entityNum);
void __cdecl AIPhys_ClipVelocity(
    const float *in,
    const float *normal,
    bool isWalkable,
    float *out,
    float overbounce);
SlideMoveResult __cdecl AIPhys_SlideMove(int gravity, int zonly);
int __cdecl AIPhys_StepSlideMove(int gravity, int zonly);
int __cdecl AIPhys_AirMove();
int __cdecl AIPhys_WalkMove();
int __cdecl AIPhys_ZOnlyPhysicsMove();
void AIPhys_NoClipMove();
SlideMoveResult AIPhys_NoGravityMove();
void AIPhys_GroundTrace();
void AIPhys_Footsteps();
void __cdecl AIPhys_FoliageSounds();
int __cdecl Actor_Physics(actor_physics_t *pPhys);
void __cdecl Actor_PostPhysics(actor_physics_t *pPhys);
