#pragma once

#include <qcommon/qcommon.h>
#include <qcommon/ent.h>

//#ifdef KISAK_MP
//#include <server_mp/server_mp.h>
//#elif KISAK_SP 
//#include "server.h"
//#endif

struct gentity_s;
struct IgnoreEntParams;
struct sightpointtrace_t;

unsigned int __cdecl SV_ClipHandleForEntity(const gentity_s *ent);
void __cdecl SV_UnlinkEntity(gentity_s *gEnt);
void __cdecl SV_LinkEntity(gentity_s *gEnt);
void __cdecl SnapAngles(float *vAngles);
void __cdecl SV_ClipMoveToEntity(const moveclip_t *clip, svEntity_s *check, trace_t *trace);
void __cdecl SV_PointTraceToEntity(const pointtrace_t *clip, svEntity_s *check, trace_t *trace);
DObj_s *__cdecl SV_LocationalTraceDObj(const pointtrace_t *clip, const gentity_s *touch);
int __cdecl SV_ClipSightToEntity(const sightclip_t *clip, svEntity_s *check);
int __cdecl SV_PointSightTraceToEntity(const sightpointtrace_t *clip, svEntity_s *check);
DObj_s *__cdecl SV_LocationalSightTraceDObj(const sightpointtrace_t *clip, const gentity_s *touch);
void __cdecl SV_SetupIgnoreEntParams(IgnoreEntParams *ignoreEntParams, int baseEntity);
void __cdecl SV_Trace(
    trace_t *results,
    const float *start,
    const float *mins,
    const float *maxs,
    const float *end,
    const IgnoreEntParams *ignoreEntParams,
    int contentmask,
    int locational,
    unsigned __int8 *priorityMap,
    int staticmodels);
int __cdecl SV_TracePassed(
    const float *start,
    const float *mins,
    const float *maxs,
    const float *end,
    int passEntityNum0,
    int passEntityNum1,
    int contentmask,
    int locational,
    unsigned __int8 *priorityMap,
    int staticmodels);
void __cdecl SV_SightTrace(
    int *hitNum,
    const float *start,
    const float *mins,
    const float *maxs,
    const float *end,
    int passEntityNum0,
    int passEntityNum1,
    int contentmask);
int __cdecl SV_SightTraceToEntity(float *start, float *mins, float *maxs, float *end, int entityNum, int contentmask);
int __cdecl SV_PointContents(float *p, int passEntityNum, int contentmask);
