#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

actor_prone_info_s *__fastcall G_GetActorProneInfo(actor_s *actor);
actor_prone_info_s *__fastcall G_GetActorProneInfoFromEntNum(int iEntNum);
actor_prone_info_s *__fastcall G_BypassForCG_GetClientActorProneInfo(int iEntNum);
void __fastcall G_InitActorProneInfo(actor_s *actor);
void __fastcall G_ActorEnterProne(actor_s *actor, unsigned int iTransTime);
void __fastcall G_ActorExitProne(actor_s *actor, unsigned int iTransTime);
