#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

enum ai_animmode_t : __int32
{
    AI_ANIM_UNKNOWN = 0x0,
    AI_ANIM_MOVE_CODE = 0x1,
    AI_ANIM_USE_POS_DELTAS = 0x2,
    AI_ANIM_USE_ANGLE_DELTAS = 0x3,
    AI_ANIM_USE_BOTH_DELTAS = 0x4,
    AI_ANIM_USE_BOTH_DELTAS_NOCLIP = 0x5,
    AI_ANIM_USE_BOTH_DELTAS_NOGRAVITY = 0x6,
    AI_ANIM_USE_BOTH_DELTAS_ZONLY_PHYSICS = 0x7,
    AI_ANIM_NOPHYSICS = 0x8,
    AI_ANIM_POINT_RELATIVE = 0x9,
};

struct __declspec(align(4)) scr_animscript_t
{
    int func;
    unsigned __int16 name;
};

struct AnimScriptList
{
    scr_animscript_t combat;
    scr_animscript_t concealment_crouch;
    scr_animscript_t concealment_prone;
    scr_animscript_t concealment_stand;
    scr_animscript_t cover_arrival;
    scr_animscript_t cover_crouch;
    scr_animscript_t cover_left;
    scr_animscript_t cover_prone;
    scr_animscript_t cover_right;
    scr_animscript_t cover_stand;
    scr_animscript_t cover_wide_left;
    scr_animscript_t cover_wide_right;
    scr_animscript_t death;
    scr_animscript_t grenade_return_throw;
    scr_animscript_t init;
    scr_animscript_t pain;
    scr_animscript_t move;
    scr_animscript_t scripted;
    scr_animscript_t stop;
    scr_animscript_t grenade_cower;
    scr_animscript_t flashed;
    scr_animscript_t weapons[128];
};

struct actor_s;

void __cdecl Actor_InitAnim(actor_s *self);
unsigned int __cdecl Actor_IsAnimScriptAlive(actor_s *self);
void __cdecl Actor_KillAnimScript(actor_s *self);
void __cdecl Actor_SetAnimScript(
    actor_s *self,
    scr_animscript_t *pAnimScriptFunc,
    unsigned __int8 moveMode,
    ai_animmode_t animMode);
void __cdecl Actor_AnimMoveAway(actor_s *self, scr_animscript_t *pAnimScriptFunc);
void __cdecl Actor_AnimStop(actor_s *self, scr_animscript_t *pAnimScriptFunc);
void __cdecl Actor_AnimWalk(actor_s *self, scr_animscript_t *pAfterMoveAnimScriptFunc);
scr_animscript_t *__cdecl Actor_GetStopAnim(actor_s *self);
void __cdecl Actor_AnimTryWalk(actor_s *self);
void __cdecl Actor_AnimRun(actor_s *self, scr_animscript_t *pAfterMoveAnimScriptFunc);
void __cdecl Actor_AnimTryRun(actor_s *self);
void __cdecl Actor_AnimCombat(actor_s *self);
void __cdecl Actor_AnimPain(actor_s *self);
void __cdecl Actor_AnimDeath(actor_s *self);
void __cdecl Actor_AnimSpecific(actor_s *self, scr_animscript_t *func, ai_animmode_t eAnimMode, bool bUseGoalWeight);
void __cdecl Actor_AnimScripted(actor_s *self);
