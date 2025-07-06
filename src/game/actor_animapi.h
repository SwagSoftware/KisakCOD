#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

void __fastcall Actor_InitAnim(actor_s *self);
unsigned int __fastcall Actor_IsAnimScriptAlive(actor_s *self);
void __fastcall Actor_KillAnimScript(actor_s *self);
void __fastcall Actor_SetAnimScript(
    actor_s *self,
    scr_animscript_t *pAnimScriptFunc,
    unsigned __int8 moveMode,
    ai_animmode_t animMode);
void __fastcall Actor_AnimMoveAway(actor_s *self, scr_animscript_t *pAnimScriptFunc);
void __fastcall Actor_AnimStop(actor_s *self, scr_animscript_t *pAnimScriptFunc);
void __fastcall Actor_AnimWalk(actor_s *self, scr_animscript_t *pAfterMoveAnimScriptFunc);
scr_animscript_t *__fastcall Actor_GetStopAnim(actor_s *self);
void __fastcall Actor_AnimTryWalk(actor_s *self);
void __fastcall Actor_AnimRun(actor_s *self, scr_animscript_t *pAfterMoveAnimScriptFunc);
void __fastcall Actor_AnimTryRun(actor_s *self);
void __fastcall Actor_AnimCombat(actor_s *self);
void __fastcall Actor_AnimPain(actor_s *self);
void __fastcall Actor_AnimDeath(actor_s *self);
void __fastcall Actor_AnimSpecific(actor_s *self, scr_animscript_t *func, ai_animmode_t eAnimMode, bool bUseGoalWeight);
void __fastcall Actor_AnimScripted(actor_s *self);
