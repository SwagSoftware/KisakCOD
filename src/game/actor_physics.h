#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

void __fastcall TRACK_actor_physics();
void __fastcall AIPhys_AddTouchEnt(int entityNum);
void __fastcall AIPhys_ClipVelocity(
    const float *in,
    const float *normal,
    bool isWalkable,
    float *out,
    double overbounce);
SlideMoveResult __fastcall AIPhys_SlideMove(int gravity, int zonly);
int __fastcall AIPhys_StepSlideMove(int gravity, int zonly);
int __fastcall AIPhys_AirMove();
int __fastcall AIPhys_WalkMove();
int __fastcall AIPhys_ZOnlyPhysicsMove();
void AIPhys_NoClipMove();
SlideMoveResult AIPhys_NoGravityMove();
void AIPhys_GroundTrace();
void AIPhys_Footsteps();
void __fastcall AIPhys_FoliageSounds();
int __fastcall Actor_Physics(actor_physics_t *pPhys);
void __fastcall Actor_PostPhysics(actor_physics_t *pPhys);
