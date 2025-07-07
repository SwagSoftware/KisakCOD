#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

void __cdecl TRACK_actor_physics();
void __cdecl AIPhys_AddTouchEnt(int entityNum);
void __cdecl AIPhys_ClipVelocity(
    const float *in,
    const float *normal,
    bool isWalkable,
    float *out,
    double overbounce);
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
