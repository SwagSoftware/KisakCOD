#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif


float __fastcall Actor_EventDefaultRadiusSqrd(ai_event_t eType);
float __fastcall Actor_EventDefaultHeightDiff(ai_event_t eType);
const char *__fastcall Actor_NameForEvent(ai_event_t eType);
void __fastcall Actor_DumpEvents(actor_s *self, ai_event_t event, gentity_s *originator);
int __fastcall Actor_EventForName(const char *name);
// attributes: thunk
void __fastcall Actor_GetPerfectInfo(actor_s *self, sentient_s *pOther);
void __fastcall Actor_WasAttackedBy(actor_s *self, sentient_s *pOther);
void __fastcall Actor_EventFootstep(actor_s *self, sentient_s *originator, const float *vOrigin);
void __fastcall Actor_EventNewEnemy(actor_s *self, sentient_s *originator);
void __fastcall Actor_EventPain(actor_s *self, sentient_s *pCasualty, sentient_s *pAttacker);
void __fastcall Actor_EventDeath(actor_s *self, sentient_s *pCasualty, sentient_s *pAttacker);
void __fastcall Actor_EventExplosion(actor_s *self, gentity_s *originator, const float *vOrigin);
void __fastcall Actor_EventGrenadePing(actor_s *self, gentity_s *originator, const float *vOrigin);
void __fastcall Actor_EventGunshot(actor_s *self, sentient_s *originator, const float *vOrigin);
void __fastcall Actor_EventBullet(
    actor_s *self,
    gentity_s *originator,
    const float *vStart,
    const float *vEnd,
    const float *vClosest,
    double fDistSqrd,
    double fRadiusSqrd,
    PARM_SUPPRESSION suppression,
    int a9,
    int a10);
void __fastcall Actor_ReceivePointEvent(
    actor_s *self,
    gentity_s *originator,
    ai_event_t eType,
    const float *vOrigin,
    double fDistSqrd,
    double fRadiusSqrd);
void __fastcall Actor_ReceiveLineEvent(
    actor_s *self,
    gentity_s *originator,
    ai_event_t eType,
    const float *vStart,
    const float *vEnd,
    const float *vClosest,
    double fDistSqrd,
    double fRadiusSqrd,
    int a9);
void __fastcall Actor_ReceiveArcEvent(
    actor_s *self,
    gentity_s *originator,
    ai_event_t eType,
    const float *origin,
    double distSqrd,
    double radiusSqrd,
    double angle0,
    double angle1,
    double halfHeight);
void __fastcall Actor_ReceiveVolumeEvent(actor_s *self, gentity_s *originator, ai_event_t eType);
void __fastcall Actor_BroadcastPointEvent(
    gentity_s *originator,
    int eType,
    int teamFlags,
    const float *vOrigin,
    double fRadiusSqrd);
void __fastcall Actor_BroadcastLineEvent(
    gentity_s *originator,
    int eType,
    int teamFlags,
    const float *vStart,
    const float *vEnd,
    double fRadiusSqrd);
void __fastcall Actor_BroadcastArcEvent(
    gentity_s *originator,
    __int32 eventType,
    int teamFlags,
    const float *origin,
    double radius,
    double angle0,
    double angle1,
    double halfHeight);
void __fastcall Actor_BroadcastVolumeEvent(
    gentity_s *originator,
    __int32 eventType,
    int teamFlags,
    gentity_s *volumeEnt,
    double radius);
void __fastcall Actor_BroadcastTeamEvent(sentient_s *sentient, int eType);
