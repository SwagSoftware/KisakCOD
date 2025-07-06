#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

void __fastcall TRACK_actor_badplace();
void __fastcall Path_UpdateBadPlaceCount(badplace_t *place, int delta);
void __fastcall Path_FreeBadPlace(int index);
int __fastcall Path_FindBadPlace(unsigned int name);
badplace_t *__fastcall Path_AllocBadPlace(unsigned int name, int duration, int a3, int a4, int a5, int a6, __int64 a7);
void __fastcall Path_MakeBadPlace(
    unsigned int name,
    int duration,
    int teamflags,
    int type,
    badplace_parms_t *parms,
    int a6,
    __int64 a7);
void __fastcall Path_MakeArcBadPlace(
    unsigned int name,
    int duration,
    int teamflags,
    badplace_arc_t *arc,
    int a5,
    int a6);
void __fastcall Path_MakeBrushBadPlace(unsigned int name, int duration, int teamflags, gentity_s *volume);
void __fastcall Path_RemoveBadPlaceEntity(gentity_s *entity);
void __fastcall Path_DrawBadPlace(badplace_t *place);
void __fastcall Path_InitBadPlaces();
void __fastcall Path_ShutdownBadPlaces();
void __fastcall Actor_Badplace_Ping(actor_s *self);
int __fastcall Actor_IsInAnyBadPlace(actor_s *self);
actor_s *Actor_BadPlace_UpdateFleeingActors();
float __fastcall Actor_BadPlace_GetMaximumFleeRadius();
int __fastcall Actor_BadPlace_HasPotentialNodeDuplicates(
    pathsort_t *potentialNodes,
    int potentialNodeCount,
    pathnode_t *checkNode);
int __fastcall Actor_BadPlace_IsNodeInAnyBadPlace(pathnode_t *node);
pathnode_t *__fastcall Actor_BadPlace_FindSafeNodeAlongPath(actor_s *self);
void __fastcall Actor_BadPlace_Flee_Finish(actor_s *self, ai_state_t eNextState);
void __fastcall Path_RemoveBadPlace(unsigned int name);
void __fastcall Path_RunBadPlaces();
int __fastcall Actor_BadPlace_FindSafeNodeOutsideBadPlace(
    actor_s *self,
    pathsort_t *potentialNodes,
    double maxFleeDist);
int __fastcall Actor_BadPlace_AttemptEscape(actor_s *self);
int __fastcall Actor_BadPlace_Flee_Start(actor_s *self, ai_state_t ePrevState);
int __fastcall Actor_BadPlace_Flee_Think(actor_s *self);
