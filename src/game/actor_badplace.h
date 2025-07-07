#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

void __cdecl TRACK_actor_badplace();
void __cdecl Path_UpdateBadPlaceCount(badplace_t *place, int delta);
void __cdecl Path_FreeBadPlace(int index);
int __cdecl Path_FindBadPlace(unsigned int name);
badplace_t *__cdecl Path_AllocBadPlace(unsigned int name, int duration, int a3, int a4, int a5, int a6, __int64 a7);
void __cdecl Path_MakeBadPlace(
    unsigned int name,
    int duration,
    int teamflags,
    int type,
    badplace_parms_t *parms,
    int a6,
    __int64 a7);
void __cdecl Path_MakeArcBadPlace(
    unsigned int name,
    int duration,
    int teamflags,
    badplace_arc_t *arc,
    int a5,
    int a6);
void __cdecl Path_MakeBrushBadPlace(unsigned int name, int duration, int teamflags, gentity_s *volume);
void __cdecl Path_RemoveBadPlaceEntity(gentity_s *entity);
void __cdecl Path_DrawBadPlace(badplace_t *place);
void __cdecl Path_InitBadPlaces();
void __cdecl Path_ShutdownBadPlaces();
void __cdecl Actor_Badplace_Ping(actor_s *self);
int __cdecl Actor_IsInAnyBadPlace(actor_s *self);
actor_s *Actor_BadPlace_UpdateFleeingActors();
float __cdecl Actor_BadPlace_GetMaximumFleeRadius();
int __cdecl Actor_BadPlace_HasPotentialNodeDuplicates(
    pathsort_t *potentialNodes,
    int potentialNodeCount,
    pathnode_t *checkNode);
int __cdecl Actor_BadPlace_IsNodeInAnyBadPlace(pathnode_t *node);
pathnode_t *__cdecl Actor_BadPlace_FindSafeNodeAlongPath(actor_s *self);
void __cdecl Actor_BadPlace_Flee_Finish(actor_s *self, ai_state_t eNextState);
void __cdecl Path_RemoveBadPlace(unsigned int name);
void __cdecl Path_RunBadPlaces();
int __cdecl Actor_BadPlace_FindSafeNodeOutsideBadPlace(
    actor_s *self,
    pathsort_t *potentialNodes,
    double maxFleeDist);
int __cdecl Actor_BadPlace_AttemptEscape(actor_s *self);
int __cdecl Actor_BadPlace_Flee_Start(actor_s *self, ai_state_t ePrevState);
int __cdecl Actor_BadPlace_Flee_Think(actor_s *self);
