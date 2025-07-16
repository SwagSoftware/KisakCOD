#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

enum ai_event_t : __int32
{
    AI_EV_BAD = 0x0,
    AI_EV_FIRST_POINT_EVENT = 0x1,
    AI_EV_FOOTSTEP = 0x2,
    AI_EV_FOOTSTEP_LITE = 0x3,
    AI_EV_NEW_ENEMY = 0x4,
    AI_EV_PAIN = 0x5,
    AI_EV_DEATH = 0x6,
    AI_EV_EXPLOSION = 0x7,
    AI_EV_GRENADE_PING = 0x8,
    AI_EV_PROJECTILE_PING = 0x9,
    AI_EV_GUNSHOT = 0xA,
    AI_EV_SILENCED_SHOT = 0xB,
    AI_EV_LAST_POINT_EVENT = 0xC,
    AI_EV_FIRST_LINE_EVENT = 0xD,
    AI_EV_BULLET = 0xE,
    AI_EV_BLOCK_FRIENDLIES = 0xE,
    AI_EV_PROJECTILE_IMPACT = 0xF,
    AI_EV_LAST_LINE_EVENT = 0x10,
    AI_EV_FIRST_ARC_EVENT = 0x11,
    AI_EV_BADPLACE_ARC = 0x12,
    AI_EV_LAST_ARC_EVENT = 0x13,
    AI_EV_FIRST_VOLUME_EVENT = 0x14,
    AI_EV_BADPLACE_VOLUME = 0x15,
    AI_EV_LAST_VOLUME_EVENT = 0x16,
    AI_EV_NUM_EVENTS = 0x17,
};

enum PARM_SUPPRESSION : __int32
{
    DO_SUPPRESSION = 0x0,
    DONT_SUPPRESS = 0x1,
};

struct sentient_s;
struct gentity_s;
struct actor_s;


float __cdecl Actor_EventDefaultRadiusSqrd(ai_event_t eType);
float __cdecl Actor_EventDefaultHeightDiff(ai_event_t eType);
const char *__cdecl Actor_NameForEvent(ai_event_t eType);
void __cdecl Actor_DumpEvents(actor_s *self, ai_event_t event, gentity_s *originator);
int __cdecl Actor_EventForName(const char *name);
void __cdecl Actor_GetPerfectInfo(actor_s *self, sentient_s *pOther);
void __cdecl Actor_WasAttackedBy(actor_s *self, sentient_s *pOther);
void __cdecl Actor_EventFootstep(actor_s *self, sentient_s *originator, const float *vOrigin);
void __cdecl Actor_EventNewEnemy(actor_s *self, sentient_s *originator);
void __cdecl Actor_EventPain(actor_s *self, sentient_s *pCasualty, sentient_s *pAttacker);
void __cdecl Actor_EventDeath(actor_s *self, sentient_s *pCasualty, sentient_s *pAttacker);
void __cdecl Actor_EventExplosion(actor_s *self, gentity_s *originator, const float *vOrigin);
void __cdecl Actor_EventGrenadePing(actor_s *self, gentity_s *originator, const float *vOrigin);
void __cdecl Actor_EventGunshot(actor_s *self, sentient_s *originator, const float *vOrigin);
void __cdecl Actor_EventBullet(
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
void __cdecl Actor_ReceivePointEvent(
    actor_s *self,
    gentity_s *originator,
    ai_event_t eType,
    const float *vOrigin,
    double fDistSqrd,
    double fRadiusSqrd);
void __cdecl Actor_ReceiveLineEvent(
    actor_s *self,
    gentity_s *originator,
    ai_event_t eType,
    const float *vStart,
    const float *vEnd,
    const float *vClosest,
    double fDistSqrd,
    double fRadiusSqrd,
    int a9);
void __cdecl Actor_ReceiveArcEvent(
    actor_s *self,
    gentity_s *originator,
    ai_event_t eType,
    const float *origin,
    double distSqrd,
    double radiusSqrd,
    double angle0,
    double angle1,
    double halfHeight);
void __cdecl Actor_ReceiveVolumeEvent(actor_s *self, gentity_s *originator, ai_event_t eType);
void __cdecl Actor_BroadcastPointEvent(
    gentity_s *originator,
    int eType,
    int teamFlags,
    const float *vOrigin,
    double fRadiusSqrd);
void __cdecl Actor_BroadcastLineEvent(
    gentity_s *originator,
    int eType,
    int teamFlags,
    const float *vStart,
    const float *vEnd,
    double fRadiusSqrd);
void __cdecl Actor_BroadcastArcEvent(
    gentity_s *originator,
    __int32 eventType,
    int teamFlags,
    const float *origin,
    double radius,
    double angle0,
    double angle1,
    double halfHeight);
void __cdecl Actor_BroadcastVolumeEvent(
    gentity_s *originator,
    __int32 eventType,
    int teamFlags,
    gentity_s *volumeEnt,
    double radius);
void __cdecl Actor_BroadcastTeamEvent(sentient_s *sentient, int eType);
