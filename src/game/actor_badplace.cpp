#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "pathnode.h"
#include "actor_badplace.h"
#include "g_main.h"
#include <script/scr_vm.h>
#include "actor_corpse.h"
#include "actor_events.h"
#include "game_public.h"
#include "g_local.h"
#include <server/sv_game.h>
#include "actor_cover.h"
#include "actor_orientation.h"
#include "actor_team_move.h"
#include "actor_state.h"

#include <algorithm>

// Line 38954:  0006 : 005a27c8       struct badplace_t *g_badplaces 82c327c8     actor_badplace.obj

badplace_t g_badplaces[32];

void __cdecl TRACK_actor_badplace()
{
    track_static_alloc_internal(g_badplaces, 1280, "g_badplaces", 5);
}

void __cdecl Path_UpdateBadPlaceCount(badplace_t *place, int delta)
{
    int type; // r4
    const char *v5; // r3

    if (!place)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_badplace.cpp", 49, 0, "%s", "place");
    if (delta != -1 && delta != 1)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_badplace.cpp",
            50,
            0,
            "%s\n\t(delta) = %i",
            "(delta == -1 || delta == +1)",
            delta);
    type = place->type;
    if (type == 1)
    {
        Path_UpdateArcBadPlaceCount(&place->parms.arc, place->teamflags, delta);
    }
    else if (type == 2)
    {
        Path_UpdateBrushBadPlaceCount(place->parms.brush.volume, place->teamflags, delta);
    }
    else if (!alwaysfails)
    {
        v5 = va("unhandled bad place type %i", type);
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_badplace.cpp", 63, 0, v5);
    }
}

void __cdecl Path_FreeBadPlace(int index)
{
    int v1; // r30

    if (index >= 0)
    {
        v1 = index;
        Path_UpdateBadPlaceCount(&g_badplaces[index], -1);
        g_badplaces[v1].type = 0;
        Scr_SetString(&g_badplaces[v1].name, 0);
    }
}

int __cdecl Path_FindBadPlace(unsigned int name)
{
    int v1; // r10
    unsigned __int16 *p_name; // r11

    v1 = 0;
    p_name = &g_badplaces[0].name;
    while (*p_name != name)
    {
        p_name += 20;
        ++v1;
        if ((int)p_name >= (int)&playerEyePos[1])
            return -1;
    }
    return v1;
}

badplace_t *__cdecl Path_AllocBadPlace(unsigned int name, int duration)
{
    __int64 v2; // r10
    int v5; // r10
    unsigned __int16 *p_name; // r11
    int v7; // r29
    badplace_t *result; // r3
    double v9; // r5
    int v10; // r11
    unsigned __int8 *p_type; // r10
    const char *v12; // r3
    int v13; // r31

    if (name)
    {
        v5 = 0;
        p_name = &g_badplaces[0].name;
        while (*p_name != name)
        {
            p_name += 20;
            ++v5;
            if ((int)p_name >= (int)&playerEyePos[1])
            {
                v5 = -1;
                break;
            }
        }
        Path_FreeBadPlace(v5);
    }
    v7 = level.time + duration;
    if (duration > 0)
    {
        if (duration < 250)
        {
            LODWORD(v2) = duration;
            v9 = (float)((float)v2 * (float)0.001);
            Com_PrintWarning(18, (const char *)HIDWORD(v9), LODWORD(v9), 0);
        }
    }
    else
    {
        if (!name)
        {
            Scr_Error("anonymous bad places must have a duration");
            return 0;
        }
        v7 = 0x7FFFFFFF;
    }
    v10 = 0;
    p_type = &g_badplaces[0].type;
    while (*p_type)
    {
        p_type += 40;
        ++v10;
        if ((int)p_type >= (int)((unsigned __int8 *)&playerEyePos[1] + 2))
        {
            v12 = va("too many bad places (more than %i)", 32);
            Scr_Error(v12);
            return 0;
        }
    }
    v13 = v10;
    Scr_SetString(&g_badplaces[v10].name, name);
    result = &g_badplaces[v13];
    g_badplaces[v13].endtime = v7;
    return result;
}

void __cdecl Path_MakeBadPlace(unsigned int name, int duration, int teamflags, int type, badplace_parms_t *parms)
{
    badplace_t *v10; // r3
    badplace_brush_t *p_parms; // r10
    badplace_parms_t *v12; // r11
    int v13; // ctr

    if (!parms)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_badplace.cpp", 171, 0, "%s", "parms");
    if (teamflags <= 0 || teamflags >= 16)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_badplace.cpp",
            172,
            0,
            "%s\n\t(teamflags) = %i",
            "(teamflags > 0 && teamflags < (1 << (sizeof( ((pathlink_t *) 0)->ubBadPlaceCount ) / (sizeof( ((pathlink_t *) 0)->"
            "ubBadPlaceCount[0] ) * (sizeof( ((pathlink_t *) 0)->ubBadPlaceCount ) != 4 || sizeof( ((pathlink_t *) 0)->ubBadPla"
            "ceCount[0] ) <= 4)))))",
            teamflags);
    if (teamflags != (unsigned __int8)teamflags)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_badplace.cpp",
            173,
            0,
            "%s\n\t(teamflags) = %i",
            "(teamflags == (byte) teamflags)",
            teamflags);
    if (type != (unsigned __int8)type)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\actor_badplace.cpp",
            174,
            0,
            "%s\n\t(type) = %i",
            "(type == (byte) type)",
            type);
    v10 = Path_AllocBadPlace(name, duration);
    if (v10)
    {
        p_parms = (badplace_brush_t *)&v10->parms;
        v10->teamflags = teamflags;
        v12 = parms;
        v10->type = type;
        v13 = 7;
        do
        {
            p_parms->volume = v12->brush.volume;
            v12 = (badplace_parms_t *)((char *)v12 + 4);
            p_parms = (badplace_brush_t *)((char *)p_parms + 4);
            --v13;
        } while (v13);
        v10->pingTime = level.time;
        Path_UpdateBadPlaceCount(v10, 1);
        Actor_BadPlacesChanged();
    }
}

void __cdecl Path_MakeArcBadPlace(unsigned int name, int duration, int teamflags, badplace_arc_t *arc)
{
    badplace_parms_t *v8; // r10
    badplace_arc_t *v9; // r11
    int v10; // ctr
    badplace_parms_t v11[2]; // [sp+50h] [-50h] BYREF

    if (!arc)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_badplace.cpp", 199, 0, "%s", "arc");
    v8 = v11;
    v9 = arc;
    v10 = 7;
    do
    {
        v8->brush.volume = (gentity_s *)LODWORD(v9->origin[0]);
        v9 = (badplace_arc_t *)((char *)v9 + 4);
        v8 = (badplace_parms_t *)((char *)v8 + 4);
        --v10;
    } while (v10);
    Path_MakeBadPlace(name, duration, teamflags, 1, v11);
}

void __cdecl Path_MakeBrushBadPlace(unsigned int name, int duration, int teamflags, gentity_s *volume)
{
    int flags; // r11
    badplace_parms_t v9; // [sp+50h] [-50h] BYREF

    if (!volume)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_badplace.cpp", 215, 0, "%s", "volume");
    flags = volume->flags;
    v9.brush.volume = volume;
    volume->flags = flags | 0x200000;
    v9.arc.origin[1] = RadiusFromBounds2D(volume->r.mins, volume->r.maxs);
    Path_MakeBadPlace(name, duration, teamflags, 2, &v9);
}

void __cdecl Path_RemoveBadPlaceEntity(gentity_s *entity)
{
    unsigned int v2; // r9
    int v3; // r11
    badplace_parms_t *i; // r10
    int v5; // r31

    if (!entity)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_badplace.cpp", 231, 0, "%s", "entity");
    v2 = 0;
    v3 = 0;
    for (i = &g_badplaces[0].parms;
        *((_BYTE *)&i[-1].brush + 26) != 2 || i->brush.volume != entity;
        i = (badplace_parms_t *)((char *)i + 40))
    {
        v2 += 40;
        ++v3;
        if (v2 >= 0x500)
            return;
    }
    if (v3 >= 0)
    {
        v5 = v3;
        Path_UpdateBadPlaceCount(&g_badplaces[v3], -1);
        g_badplaces[v5].type = 0;
        Scr_SetString(&g_badplaces[v5].name, 0);
    }
}

void __cdecl Path_DrawBadPlace(badplace_t *place)
{
    const float *type; // r4
    const char *v3; // r3
    double radius; // fp31
    double angle0; // fp1
    const float *v6; // r4
    const float *v7; // r4
    const float *v8; // r4
    float *v9; // r3
    double v10; // fp0
    double angle1; // fp3
    double v12; // fp2
    int v13; // r6
    int v14; // r5
    const float *v15; // r4
    double v16; // fp0
    double v17; // fp3
    double v18; // fp2
    int v19; // r6
    int v20; // r5
    const float *v21; // r4
    float v22; // [sp+50h] [-70h] BYREF
    float v23; // [sp+54h] [-6Ch]
    float v24; // [sp+58h] [-68h]
    float v25; // [sp+60h] [-60h] BYREF
    float v26; // [sp+64h] [-5Ch]
    float v27; // [sp+68h] [-58h]
    float v28; // [sp+70h] [-50h] BYREF
    float v29; // [sp+74h] [-4Ch]
    float v30; // [sp+78h] [-48h]
    float v31; // [sp+80h] [-40h] BYREF
    float v32; // [sp+84h] [-3Ch]
    float v33; // [sp+88h] [-38h]

    type = (const float *)place->type;
    if (type == (const float *)1)
    {
        if ((float)(place->parms.arc.radius * place->parms.arc.radius) == 0.0)
            //radius = sqrtf(Actor_EventDefaultRadiusSqrd(AI_EV_BADPLACE_ARC));
            radius = sqrtf(Actor_EventDefaultRadiusSqrd(AI_EV_BADPLACE_ARC));
        else
            radius = place->parms.arc.radius;
        angle0 = place->parms.arc.angle0;
        if (angle0 == 0.0 && place->parms.arc.angle1 == 360.0)
        {
            v22 = place->parms.arc.origin[0];
            v23 = place->parms.arc.origin[1];
            v24 = place->parms.arc.origin[2];
            G_DebugCircle(&v22, radius, type, (int)colorRed, 1, 1);
            v24 = v24 - place->parms.arc.halfheight;
            G_DebugCircle(&v22, radius, v6, (int)colorRed, 1, 1);
            v24 = (float)(place->parms.arc.halfheight * (float)2.0) + v24;
            G_DebugCircle(&v22, radius, v7, (int)colorRed, 1, 1);
            v24 = (float)(place->parms.arc.halfheight * (float)2.0) + v24;
            G_DebugCircle(&v22, radius, v8, (int)colorRed, 1, 1);
        }
        else
        {
            YawVectors(angle0, (float *)&place->endtime, &v28);
            YawVectors(place->parms.arc.angle1, v9, &v31);
            v24 = place->parms.arc.origin[2] - place->parms.arc.halfheight;
            v10 = place->parms.arc.origin[1];
            angle1 = place->parms.arc.angle1;
            v12 = place->parms.arc.angle0;
            v22 = place->parms.arc.origin[0];
            v23 = v10;
            G_DebugArc(&v22, radius, v12, angle1, v15, v14, v13);
            v25 = (float)(v28 * (float)radius) + v22;
            v26 = (float)(v29 * (float)radius) + v23;
            v27 = (float)(v30 * (float)radius) + v24;
            G_DebugLine(&v22, &v25, colorRed, 1);
            v25 = (float)(v31 * (float)radius) + v22;
            v26 = (float)(v32 * (float)radius) + v23;
            v27 = (float)(v33 * (float)radius) + v24;
            G_DebugLine(&v22, &v25, colorRed, 1);
            v16 = (float)((float)(place->parms.arc.halfheight * (float)2.0) + v24);
            v25 = (float)(v28 * (float)radius) + v22;
            v17 = place->parms.arc.angle1;
            v18 = place->parms.arc.angle0;
            v24 = v16;
            v26 = (float)(v29 * (float)radius) + v23;
            v27 = (float)(v30 * (float)radius) + (float)v16;
            G_DebugArc(&v22, radius, v18, v17, v21, v20, v19);
            v25 = (float)(v28 * (float)radius) + v22;
            v26 = (float)(v29 * (float)radius) + v23;
            v27 = (float)(v30 * (float)radius) + v24;
            G_DebugLine(&v22, &v25, colorRed, 1);
            v25 = (float)(v31 * (float)radius) + v22;
            v26 = (float)(v32 * (float)radius) + v23;
            v27 = (float)(v33 * (float)radius) + v24;
            G_DebugLine(&v22, &v25, colorRed, 1);
        }
    }
    else if (type == (const float *)2)
    {
        G_DebugDrawBrushModel(place->parms.brush.volume, colorRed, 1, 0);
    }
    else if (!alwaysfails)
    {
        v3 = va("unhandled bad place type %i", type);
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_badplace.cpp", 304, 0, v3);
    }
}

void __cdecl Path_InitBadPlaces()
{
    memset(g_badplaces, 0, sizeof(g_badplaces));
}

void __cdecl Path_ShutdownBadPlaces()
{
    int v0; // r31
    unsigned __int16 *p_name; // r30

    v0 = 32;
    p_name = &g_badplaces[0].name;
    do
    {
        Scr_SetString(p_name, 0);
        --v0;
        p_name += 20;
    } while (v0);
    memset(g_badplaces, 0, sizeof(g_badplaces));
}

void __cdecl Actor_Badplace_Ping(actor_s *self)
{
    __int64 v2; // r11

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_badplace.cpp", 398, 0, "%s", "self");
    if (!self->flashBanged)
    {
        LODWORD(v2) = G_rand();
        if ((float)v2 <= (double)(float)(self->badPlaceAwareness * (float)32767.0))
            self->isInBadPlace = 1;
    }
}

int __cdecl Actor_IsInAnyBadPlace(actor_s *self)
{
    unsigned int v2; // r27
    float *i; // r31
    int v4; // r4
    const char *v5; // r3
    bool v6; // cr58

    v2 = 0;
    for (i = &g_badplaces[0].parms.arc.angle1; ; i += 10)
    {
        v4 = *((unsigned __int8 *)i - 26);
        if (*((_BYTE *)i - 26))
        {
            if (v4 == 1)
            {
                v6 = !Actor_IsInsideArc(self, i - 6, *(i - 3), *(i - 1), *i, *(i - 2));
            }
            else
            {
                if (v4 != 2)
                {
                    if (!alwaysfails)
                    {
                        v5 = va("unhandled bad place type %i", v4);
                        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_badplace.cpp", 439, 0, v5);
                    }
                    goto LABEL_10;
                }
                v6 = SV_EntityContact(self->ent->r.mins, self->ent->r.maxs, *((const gentity_s **)i - 6)) == 0;
            }
            if (!v6)
                break;
        }
    LABEL_10:
        v2 += 40;
        if (v2 >= 0x500)
            return 0;
    }
    return 1;
}

actor_s *Actor_BadPlace_UpdateFleeingActors()
{
    actor_s *result; // r3
    actor_s *i; // r31

    result = Actor_FirstActor(-1);
    for (i = result; result; i = result)
    {
        if (i->eState[i->stateLevel] == AIS_BADPLACE_FLEE && !(unsigned __int8)Actor_IsInAnyBadPlace(i))
            i->isInBadPlace = 0;
        result = Actor_NextActor(i, -1);
    }
    return result;
}

float __cdecl Actor_BadPlace_GetMaximumFleeRadius()
{
    int v0; // r28
    unsigned __int8 *p_type; // r31
    double v2; // fp31
    int v3; // r4
    const char *v4; // r3
    double v5; // fp0
    double v6; // fp1

    v0 = 32;
    p_type = &g_badplaces[0].type;
    v2 = -1.0;
    do
    {
        v3 = *p_type;
        if (!*p_type)
            goto LABEL_11;
        if (v3 == 1)
        {
            v5 = *(float *)(p_type + 14);
        }
        else
        {
            if (v3 != 2)
            {
                if (!alwaysfails)
                {
                    v4 = va("unhandled bad place type %i", v3);
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_badplace.cpp", 500, 0, v4);
                }
                goto LABEL_11;
            }
            v5 = *(float *)(p_type + 6);
        }
        if (v5 > v2)
            v2 = v5;
    LABEL_11:
        --v0;
        p_type += 40;
    } while (v0);
    v6 = v2;
    return *((float *)&v6 + 1);
}

int __cdecl Actor_BadPlace_HasPotentialNodeDuplicates(
    pathsort_t *potentialNodes,
    int potentialNodeCount,
    pathnode_t *checkNode)
{
    int v6; // r10
    pathsort_t *i; // r11

    if (!potentialNodes)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_badplace.cpp", 517, 0, "%s", "potentialNodes");
    if (!checkNode)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_badplace.cpp", 518, 0, "%s", "checkNode");
    v6 = 0;
    if (potentialNodeCount <= 0)
        return 0;
    for (i = potentialNodes; i->node != checkNode; ++i)
    {
        if (++v6 >= potentialNodeCount)
            return 0;
    }
    return 1;
}

int __cdecl Actor_BadPlace_IsNodeInAnyBadPlace(pathnode_t *node)
{
    unsigned int v2; // r30
    float *i; // r31
    int v4; // r4
    const char *v5; // r3
    int IsNodeInArc; // r3

    if (!node)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_badplace.cpp", 542, 0, "%s", "node");
    v2 = 0;
    for (i = &g_badplaces[0].parms.arc.angle1; ; i += 10)
    {
        v4 = *((unsigned __int8 *)i - 26);
        if (*((_BYTE *)i - 26))
        {
            if (v4 == 1)
            {
                IsNodeInArc = Path_IsNodeInArc(node, i - 6, *(i - 3), *(i - 1), *i, *(i - 2));
            }
            else
            {
                if (v4 != 2)
                {
                    if (!alwaysfails)
                    {
                        v5 = va("unhandled bad place type %i", v4);
                        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_badplace.cpp", 563, 0, v5);
                    }
                    goto LABEL_12;
                }
                IsNodeInArc = SV_EntityContact(node->constant.vOrigin, node->constant.vOrigin, *((const gentity_s **)i - 6));
            }
            if (IsNodeInArc)
                break;
        }
    LABEL_12:
        v2 += 40;
        if (v2 >= 0x500)
            return 0;
    }
    return 1;
}

pathnode_t *__cdecl Actor_BadPlace_FindSafeNodeAlongPath(actor_s *self)
{
    int v2; // r30
    int *i; // r29
    pathnode_t *v4; // r3
    const pathnode_t *v5; // r31

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_badplace.cpp", 641, 0, "%s", "self");
    if (!Actor_HasPath(self))
        return 0;
    v2 = self->Path.lookaheadNextNode + 1;
    if (v2 < 0)
        return 0;
    for (i = &self->Path.pts[v2].iNodeNum; ; i -= 7)
    {
        if (*i > 0)
        {
            v4 = Path_ConvertIndexToNode(*i);
            v5 = v4;
            if (v4)
            {
                if (!(unsigned __int8)Actor_BadPlace_IsNodeInAnyBadPlace(v4) && Path_CanClaimNode(v5, self->sentient))
                    break;
            }
        }
        if (--v2 < 0)
            return 0;
    }
    return (pathnode_t *)v5;
}

void __cdecl Actor_BadPlace_Flee_Finish(actor_s *self, ai_state_t eNextState)
{
    ;
}

void __cdecl Path_RemoveBadPlace(unsigned int name)
{
    int v1; // r10
    unsigned __int16 *p_name; // r11

    v1 = 0;
    p_name = &g_badplaces[0].name;
    while (*p_name != name)
    {
        p_name += 20;
        ++v1;
        if ((int)p_name >= (int)&playerEyePos[1])
        {
            v1 = -1;
            break;
        }
    }
    Path_FreeBadPlace(v1);
    Actor_BadPlace_UpdateFleeingActors();
}

void __cdecl Path_RunBadPlaces()
{
    char v0; // r24
    float *p_angle1; // r31
    int v2; // r25
    int v3; // r28
    int v4; // r4
    const char *v5; // r3

    v0 = 0;
    p_angle1 = &g_badplaces[0].parms.arc.angle1;
    v2 = 0;
    v3 = 32;
    do
    {
        v4 = *((unsigned __int8 *)p_angle1 - 26);
        if (*((_BYTE *)p_angle1 - 26))
        {
            if (level.time < *((unsigned int *)p_angle1 - 9))
            {
                if (level.time - *((unsigned int *)p_angle1 - 8) >= 250)
                {
                    if (v4 == 1)
                    {
                        Actor_BroadcastArcEvent(
                            0,
                            AI_EV_BADPLACE_ARC,
                            *((unsigned __int8 *)p_angle1 - 25),
                            p_angle1 - 6,
                            *(p_angle1 - 3),
                            *(p_angle1 - 1),
                            *p_angle1,
                            *(p_angle1 - 2));
                    }
                    else if (v4 == 2)
                    {
                        Actor_BroadcastVolumeEvent(
                            0,
                            AI_EV_BADPLACE_VOLUME,
                            *((unsigned __int8 *)p_angle1 - 25),
                            *((gentity_s **)p_angle1 - 6),
                            *(p_angle1 - 5));
                    }
                    else if (!alwaysfails)
                    {
                        v5 = va("unhandled bad place type %i", v4);
                        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_badplace.cpp", 374, 0, v5);
                    }
                    *(p_angle1 - 8) = *(float *)&level.time;
                }
                if (ai_showBadPlaces->current.enabled)
                    Path_DrawBadPlace((badplace_t *)(p_angle1 - 9));
            }
            else
            {
                if (v2 >= 0)
                {
                    Path_UpdateBadPlaceCount((badplace_t *)(p_angle1 - 9), -1);
                    *((_BYTE *)p_angle1 - 26) = 0;
                    Scr_SetString((unsigned __int16 *)p_angle1 - 14, 0);
                }
                v0 = 1;
            }
        }
        --v3;
        ++v2;
        p_angle1 += 10;
    } while (v3);
    if (v0)
        Actor_BadPlace_UpdateFleeingActors();
}

int __cdecl Actor_BadPlace_FindSafeNodeOutsideBadPlace(
    actor_s *self,
    pathsort_t *potentialNodes,
    double maxFleeDist)
{
    int v6; // r5
    pathsort_t *v7; // r4
    int nodeCount;
    int nodesWritten; // r30
    pathsort_t *pOutNode; // r29
    pathsort_t *pNode; // r27
    pathnode_t *node; // r31
    double v14; // fp0
    double v15; // fp13
    double v16; // fp12
    double v17; // fp31
    pathsort_t nodes[256];

    //Profile_Begin(357);
    iassert(self);
    iassert(self->sentient);
    iassert(potentialNodes);
    iassert(maxFleeDist > 0.0f);

    nodeCount = Path_NodesInCylinder(self->ent->r.currentOrigin, maxFleeDist, 80.0, nodes, 256, -2);

    if (nodeCount > 0)
    {
        nodesWritten = 0;
        pOutNode = potentialNodes;
        pNode = &nodes[0];
        do
        {
            node = pNode->node;

            if (!Actor_BadPlace_IsNodeInAnyBadPlace(node) && Path_CanClaimNode(node, self->sentient))
            {
                iassert(node);

                if ((((1 << node->constant.type) & 0x41FFC) == 0 || Actor_Cover_IsValidCover(self, node))
                    && !Actor_BadPlace_HasPotentialNodeDuplicates(potentialNodes, nodesWritten, node))
                {
                    v14 = (float)(node->constant.vOrigin[0] - self->ent->r.currentOrigin[0]);
                    v15 = (float)(node->constant.vOrigin[2] - self->ent->r.currentOrigin[2]);
                    v16 = (float)(node->constant.vOrigin[1] - self->ent->r.currentOrigin[1]);
                    v17 = (float)((float)((float)v16 * (float)v16) + (float)((float)((float)v15 * (float)v15) + (float)((float)v14 * (float)v14)));

                    if (Path_IsCoverNode(node))
                        v17 = (float)((float)v17 * 0.89999998);

                    pOutNode->metric = v17;
                    pOutNode->node = node;
                    ++nodesWritten;
                    ++pOutNode;
                }
            }
            --nodeCount;
            pNode += 3;
        } while (nodeCount);

        if (nodesWritten > 1)
        {
            //std::_Sort<pathsort_t *, int, bool(__cdecl *)(pathsort_t const &, pathsort_t const &)>(
            //    potentialNodes,
            //    &potentialNodes[v9],
            //    12 * v9 / 12,
            //    Path_CompareNodesIncreasing);
            std::sort(&potentialNodes[0], &potentialNodes[nodesWritten], Path_CompareNodesIncreasing);
        }
    }

    //Profile_EndInternal(0);
    return 0;
}

int __cdecl Actor_BadPlace_AttemptEscape(actor_s *self)
{
    double MaximumFleeRadius; // fp1
    double v3; // fp31
    pathnode_t *SafeNodeAlongPath; // r3
    pathnode_t *node; // r31
    int SafeNodeOutsideBadPlace; // r27
    int v8; // r29
    pathsort_t *i; // r30
    pathsort_t v10[256]; // [sp+50h] [-C50h] BYREF

    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_badplace.cpp", 684, 0, "%s", "self");
    if (!self->sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_badplace.cpp", 685, 0, "%s", "self->sentient");
    MaximumFleeRadius = Actor_BadPlace_GetMaximumFleeRadius();
    if (MaximumFleeRadius >= 0.0)
    {
        v3 = (float)((float)MaximumFleeRadius + (float)100.0);
        SafeNodeAlongPath = Actor_BadPlace_FindSafeNodeAlongPath(self);
        node = SafeNodeAlongPath;
        if (SafeNodeAlongPath && Actor_FindPathToNode(self, SafeNodeAlongPath, 1))
        {
        LABEL_8:
            Actor_ClearKeepClaimedNode(self);
            Sentient_ClaimNode(self->sentient, node);
            return 1;
        }
        SafeNodeOutsideBadPlace = Actor_BadPlace_FindSafeNodeOutsideBadPlace(self, v10, v3);
        v8 = 0;
        if (SafeNodeOutsideBadPlace > 0)
        {
            for (i = v10; ; ++i)
            {
                node = i->node;
                if (!i->node)
                    MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_badplace.cpp", 711, 0, "%s", "safeNode");
                if (Actor_FindPathToNode(self, node, 1))
                    break;
                if (++v8 >= SafeNodeOutsideBadPlace)
                    return 0;
            }
            goto LABEL_8;
        }
    }
    return 0;
}

bool __cdecl Actor_BadPlace_Flee_Start(actor_s *self, ai_state_t ePrevState)
{
    if (!self)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\actor_badplace.cpp", 732, 0, "%s", "self");
    self->isInBadPlace = Actor_BadPlace_AttemptEscape(self);
    return 1;
}

actor_think_result_t __cdecl Actor_BadPlace_Flee_Think(actor_s *self)
{
    actor_s *v2; // r3

    iassert(self);
    v2 = self;
    if (self->isInBadPlace)
    {
        Actor_PreThink(self);
        self->pszDebugInfo = "badplace_flee";
        if (Actor_HasPath(self))
        {
            Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY_OR_MOTION);
            Actor_TeamMoveBlockedClear(self);
            Actor_MoveAlongPathWithTeam(self, 1, 0, 0);
            if (self->pCloseEnt.isDefined() || self->pPileUpActor)
            {
                Actor_ClearPath(self);
                Actor_PostThink(self);
                return ACTOR_THINK_DONE;
            }
            goto LABEL_13;
        }
        if ((unsigned __int8)Actor_IsInAnyBadPlace(self))
        {
            if (Actor_NearClaimNode(self, 32.0))
                Path_MarkNodeInvalid(self->sentient->pClaimedNode, self->sentient->eTeam);
            if ((unsigned __int8)Actor_BadPlace_AttemptEscape(self))
            {
                Actor_AnimStop(self, &g_animScriptTable[self->species]->stop);
            LABEL_13:
                Actor_PostThink(self);
                return ACTOR_THINK_DONE;
            }
        }
        v2 = self;
        self->isInBadPlace = 0;
    }
    Actor_SetState(v2, AIS_EXPOSED);
    return ACTOR_THINK_REPEAT;
}

