#pragma once

#include "actor.h"
#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

actor_prone_info_s *__cdecl G_GetActorProneInfo(actor_s *actor);
actor_prone_info_s *__cdecl G_GetActorProneInfoFromEntNum(int iEntNum);
actor_prone_info_s *__cdecl G_BypassForCG_GetClientActorProneInfo(int iEntNum);
void __cdecl G_InitActorProneInfo(actor_s *actor);
void __cdecl G_ActorEnterProne(actor_s *actor, unsigned int iTransTime);
void __cdecl G_ActorExitProne(actor_s *actor, unsigned int iTransTime);
