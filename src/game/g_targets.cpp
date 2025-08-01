#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "g_local.h"
#include <server/sv_public.h>
#include "g_main.h"
#include <script/scr_vm.h>
#include <server/sv_game.h>
#include <script/scr_const.h>

//TargetGlob targGlob;
TargetGlob targGlob;

void __cdecl G_InitTargets()
{
    int v0; // r30
    TargetGlob *v1; // r31

    v0 = 0;
    v1 = &targGlob;
    targGlob.targetCount = 0;
    do
    {
        v1->targets[0].ent = 0;
        SV_SetConfigstring(v0 + 27, 0);
        v1 = (TargetGlob *)((char *)v1 + 28);
        ++v0;
    } while ((int)v1 < (int)&targGlob.targetCount);
}

void __cdecl G_LoadTargets()
{
    int v0; // r20
    float *v1; // r31
    const char *v2; // r3
    const char *v3; // r30
    unsigned int v4; // r7
    const char *v5; // r3
    const char *v6; // r3
    int v7; // r11
    const char *v8; // r3
    int v9; // r3
    char v10[1032]; // [sp+50h] [-480h] BYREF

    v0 = 0;
    v1 = &targGlob.targets[0].offset[2];
    targGlob.targetCount = 0;
    do
    {
        SV_GetConfigstring(v0 + 27, v10, 1024);
        if (v10[0])
        {
            ++targGlob.targetCount;
            v2 = Info_ValueForKey(v10, "ent");
            v3 = v2;
            if (*v2)
            {
                v4 = atol(v2);
                if (v4 >= 0x880)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\g_targets.cpp",
                        64,
                        0,
                        "entNum doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
                        v4,
                        2176);
                *((unsigned int *)v1 - 3) = (unsigned int)&level.gentities[atol(v3)];
            }
            else
            {
                *(v1 - 3) = 0.0;
            }
            v5 = Info_ValueForKey(v10, "offs");
            *(v1 - 2) = 0.0;
            *(v1 - 1) = 0.0;
            *v1 = 0.0;
            if (*v5)
                sscanf(v5, "%f %f %f", v1 - 2, v1 - 1, v1);
            v6 = Info_ValueForKey(v10, "mat");
            if (*v6)
                v7 = atol(v6);
            else
                v7 = -1;
            *((unsigned int *)v1 + 1) = v7;
            v8 = Info_ValueForKey(v10, "offmat");
            if (*v8)
                v9 = atol(v8);
            else
                v9 = -1;
            *((unsigned int *)v1 + 2) = v9;
        }
        else
        {
            *(v1 - 3) = 0.0;
        }
        v1 += 7;
        ++v0;
    } while ((uintptr_t)v1 < (uintptr_t)&targGlob.targets[32]);
}

void __cdecl Scr_Target_SetShader()
{
    unsigned int v0; // r4
    gentity_s *Entity; // r3
    int v2; // r31
    target_t *v3; // r11
    unsigned int v4; // r9
    const char *v5; // r3
    const char *String; // r3
    int v7; // r11
    int v8; // r30
    int v9; // r31
    const char *v10; // r3
    char v11[1056]; // [sp+50h] [-420h] BYREF

    if (Scr_GetNumParam() < 2)
        Scr_Error("Too few arguments\n");
    Entity = Scr_GetEntity(0);
    v2 = 0;
    v3 = &targGlob.targets[1];
    v4 = 0;
    while (v3[-1].ent != Entity)
    {
        if (v3->ent == Entity)
        {
            ++v2;
            break;
        }
        if (v3[1].ent == Entity)
        {
            v2 += 2;
            break;
        }
        if (v3[2].ent == Entity)
        {
            v2 += 3;
            break;
        }
        v4 += 112;
        v2 += 4;
        v3 += 4;
        if (v4 >= 0x380)
            break;
    }
    if (v2 == 32)
    {
        v5 = va("Entity %i is not a target", Entity->s.number);
        Scr_Error(v5);
    }
    if (*Scr_GetString(1u))
    {
        String = Scr_GetString(1u);
        v7 = G_MaterialIndex(String);
    }
    else
    {
        v7 = -1;
    }
    v8 = v2;
    targGlob.targets[v2].materialIndex = v7;
    v9 = v2 + 27;
    SV_GetConfigstring(v9, v11, 1024);
    v10 = va("%i", targGlob.targets[v8].materialIndex);
    Info_SetValueForKey(v11, "mat", v10);
    SV_SetConfigstring(v9, v11);
}

void __cdecl Scr_Target_SetOffscreenShader()
{
    unsigned int v0; // r4
    gentity_s *Entity; // r3
    int v2; // r31
    target_t *v3; // r11
    unsigned int v4; // r9
    const char *v5; // r3
    const char *String; // r3
    int v7; // r11
    int v8; // r30
    int v9; // r31
    const char *v10; // r3
    char v11[1056]; // [sp+50h] [-420h] BYREF

    if (Scr_GetNumParam() < 2)
        Scr_Error("Too few arguments\n");
    Entity = Scr_GetEntity(0);
    v2 = 0;
    v3 = &targGlob.targets[1];
    v4 = 0;
    while (v3[-1].ent != Entity)
    {
        if (v3->ent == Entity)
        {
            ++v2;
            break;
        }
        if (v3[1].ent == Entity)
        {
            v2 += 2;
            break;
        }
        if (v3[2].ent == Entity)
        {
            v2 += 3;
            break;
        }
        v4 += 112;
        v2 += 4;
        v3 += 4;
        if (v4 >= 0x380)
            break;
    }
    if (v2 == 32)
    {
        v5 = va("Entity %i is not a target", Entity->s.number);
        Scr_Error(v5);
    }
    if (*Scr_GetString(1u))
    {
        String = Scr_GetString(1u);
        v7 = G_MaterialIndex(String);
    }
    else
    {
        v7 = -1;
    }
    v8 = v2;
    targGlob.targets[v2].offscreenMaterialIndex = v7;
    v9 = v2 + 27;
    SV_GetConfigstring(v9, v11, 1024);
    v10 = va("%i", targGlob.targets[v8].offscreenMaterialIndex);
    Info_SetValueForKey(v11, "offmat", v10);
    SV_SetConfigstring(v9, v11);
}

void __cdecl Scr_Target_GetArray()
{
    int v0; // r30
    TargetGlob *v1; // r31

    Scr_MakeArray();
    v0 = 32;
    v1 = &targGlob;
    do
    {
        if (v1->targets[0].ent)
        {
            Scr_AddEntity(v1->targets[0].ent);
            Scr_AddArray();
        }
        --v0;
        v1 = (TargetGlob *)((char *)v1 + 28);
    } while (v0);
}

int __cdecl TargetIndex(gentity_s *ent)
{
    int v1; // r9
    TargetGlob *v2; // r11
    unsigned int v3; // r10

    v1 = 0;
    v2 = &targGlob;
    v3 = 0;
    while (v2->targets[0].ent != ent)
    {
        v3 += 28;
        ++v1;
        v2 = (TargetGlob *)((char *)v2 + 28);
        if (v3 >= 0x380)
            return 32;
    }
    return v1;
}

void __cdecl Scr_Target_IsTarget()
{
    unsigned int v0; // r4
    gentity_s *Entity; // r3
    int v2; // r9
    TargetGlob *v3; // r11
    unsigned int v4; // r10

    if (!Scr_GetNumParam())
        Scr_Error("Too few arguments\n");
    Entity = Scr_GetEntity(0);
    v2 = 0;
    v3 = &targGlob;
    v4 = 0;
    while (v3->targets[0].ent != Entity)
    {
        v4 += 28;
        ++v2;
        v3 = (TargetGlob *)((char *)v3 + 28);
        if (v4 >= 0x380)
            goto LABEL_6;
    }
    if (v2 == 32)
    {
    LABEL_6:
        Scr_AddBool(0);
        return;
    }
    Scr_AddBool(1);
}

void __cdecl Scr_Target_Set()
{
    unsigned int v0; // r4
    gentity_s *Entity; // r3
    gentity_s *v2; // r26
    unsigned int v3; // r28
    target_t *v4; // r11
    unsigned int v5; // r10
    unsigned int v6; // r10
    target_t *v7; // r11
    unsigned int v8; // r11
    float *offset; // r10
    unsigned int v10; // r30
    float *v11; // r27
    const char *v12; // r3
    const char *v13; // r3
    const char *v14; // r3
    const char *v15; // r3
    const char *v16; // r3
    char v17[1024]; // [sp+60h] [-440h] BYREF

    if (!Scr_GetNumParam())
        Scr_Error("Too few arguments\n");
    Entity = Scr_GetEntity(0);
    v2 = Entity;
    v3 = 0;
    v4 = &targGlob.targets[1];
    v5 = 0;
    while (v4[-1].ent != Entity)
    {
        if (v4->ent == Entity)
        {
            ++v3;
            break;
        }
        if (v4[1].ent == Entity)
        {
            v3 += 2;
            break;
        }
        if (v4[2].ent == Entity)
        {
            v3 += 3;
            break;
        }
        v5 += 112;
        v3 += 4;
        v4 += 4;
        if (v5 >= 0x380)
            break;
    }
    if (v3 == 32)
    {
        if (targGlob.targetCount >= 0x20)
            Scr_Error("Maximum number of targets exceeded");
        v3 = 0;
        v6 = 0;
        v7 = &targGlob.targets[1];
        while (v7[-1].ent)
        {
            if (!v7->ent)
            {
                ++v3;
                break;
            }
            if (!v7[1].ent)
            {
                v3 += 2;
                break;
            }
            if (!v7[2].ent)
            {
                v3 += 3;
                break;
            }
            v6 += 112;
            v3 += 4;
            v7 += 4;
            if (v6 >= 0x380)
                goto LABEL_27;
        }
        if (v3 < 0x20)
            goto LABEL_28;
    LABEL_27:
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_targets.cpp",
            263,
            0,
            "targetIndex doesn't index MAX_TARGETS\n\t%i not in [0, %i)",
            v3,
            32);
    LABEL_28:
        v8 = v3;
        targGlob.targets[v8].ent = v2;
        offset = targGlob.targets[v3].offset;
        v2->flags |= 0x2000000u;
        targGlob.targets[v8].materialIndex = -1;
        targGlob.targets[v8].offscreenMaterialIndex = -1;
        *offset = 0.0;
        offset[1] = 0.0;
        offset[2] = 0.0;
        ++targGlob.targetCount;
    }
    v10 = v3;
    v11 = targGlob.targets[v3].offset;
    if (Scr_GetNumParam() <= 1)
    {
        *v11 = 0.0;
        v11[1] = 0.0;
        v11[2] = 0.0;
    }
    else
    {
        Scr_GetVector(1u, targGlob.targets[v3].offset);
    }
    v17[0] = 0;
    v12 = va("%i", v2->s.number);
    Info_SetValueForKey(v17, "ent", v12);
    v13 = va("%i %i %i", (int)*v11, (int)targGlob.targets[v10].offset[1], (int)targGlob.targets[v10].offset[2]);
    Info_SetValueForKey(v17, "offs", v13);
    v14 = va("%i", targGlob.targets[v10].materialIndex);
    Info_SetValueForKey(v17, "mat", v14);
    v15 = va("%i", targGlob.targets[v10].offscreenMaterialIndex);
    Info_SetValueForKey(v17, "offmat", v15);
    v16 = va("%i", targGlob.targets[v10].flags);
    Info_SetValueForKey(v17, "flags", v16);
    SV_SetConfigstring(v3 + 27, v17);
}

int __cdecl Targ_Remove(gentity_s *ent)
{
    int v1; // r31
    unsigned int v2; // r10
    TargetGlob *i; // r11
    unsigned int v5; // r7

    v1 = 0;
    v2 = 0;
    for (i = &targGlob; i->targets[0].ent != ent; i = (TargetGlob *)((char *)i + 28))
    {
        v2 += 28;
        ++v1;
        if (v2 >= 0x380)
            return 0;
    }
    targGlob.targets[v1].ent = 0;
    v5 = targGlob.targetCount - 1;
    targGlob.targetCount = v5;
    if (v5 >= 0x20)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_targets.cpp",
            299,
            0,
            "targGlob.targetCount doesn't index MAX_TARGETS\n\t%i not in [0, %i)",
            v5,
            32);
    SV_SetConfigstring(v1 + 27, "");
    return 1;
}

void __cdecl Targ_RemoveAll()
{
    int v0; // r29
    TargetGlob *v1; // r31
    int v2; // r28
    unsigned int v3; // r7

    v0 = 0;
    v1 = &targGlob;
    v2 = 32;
    do
    {
        if (v1->targets[0].ent)
        {
            v1->targets[0].ent = 0;
            v3 = targGlob.targetCount - 1;
            targGlob.targetCount = v3;
            if (v3 >= 0x20)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\g_targets.cpp",
                    320,
                    0,
                    "targGlob.targetCount doesn't index MAX_TARGETS\n\t%i not in [0, %i)",
                    v3,
                    32);
            SV_SetConfigstring(v0 + 27, "");
        }
        --v2;
        ++v0;
        v1 = (TargetGlob *)((char *)v1 + 28);
    } while (v2);
}

void __cdecl Scr_Target_Remove()
{
    unsigned int v0; // r4
    gentity_s *Entity; // r31
    const char *v2; // r3

    if (!Scr_GetNumParam())
        Scr_Error("Too few arguments\n");
    Entity = Scr_GetEntity(0);
    if (!(unsigned __int8)Targ_Remove(Entity))
    {
        v2 = va("Entity %i is not a target", Entity->s.number);
        Scr_Error(v2);
    }
}

int __cdecl G_WorldDirToScreenPos(
    const gentity_s *player,
    double fov_x,
    const float *worldDir,
    const float *outScreenPos,
    float *a5)
{
    long double v9; // fp2
    int result; // r3
    double v11; // fp28
    double v12; // fp27
    long double v13; // fp2
    double v14; // fp31
    double v15; // fp30
    float v16[4]; // [sp+50h] [-90h] BYREF
    float v17[6][3]; // [sp+60h] [-80h] BYREF

    if (fov_x <= 0.0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_targets.cpp", 360, 0, "%s", "fov_x > 0");
    AnglesToAxis(player->s.lerp.apos.trBase, v17);
    MatrixTransposeTransformVector(outScreenPos, (const mat3x3&)v17, v16);
    if (v16[0] <= 0.0)
        return 0;
    *(double *)&v9 = (float)((float)((float)fov_x * (float)0.017453292) * (float)0.5);
    v11 = (float)((float)((float)1.0 / v16[0]) * v16[1]);
    v12 = (float)((float)((float)1.0 / v16[0]) * v16[2]);
    v13 = tan(v9);
    v14 = (float)*(double *)&v13;
    v15 = (float)((float)*(double *)&v13 * (float)0.75);
    if (v14 <= 0.0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_targets.cpp", 373, 1, "%s", "tanHalfFovX > 0");
    if (v15 <= 0.0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_targets.cpp", 374, 1, "%s", "tanHalfFovY > 0");
    result = 1;
    *a5 = (float)((float)v11 / (float)v14) * (float)-320.0;
    a5[1] = (float)((float)v12 / (float)v15) * (float)-240.0;
    return result;
}

int __cdecl ScrGetTargetScreenPos(float *screenPos)
{
    unsigned int v2; // r4
    gentity_s *Entity; // r28
    unsigned int v4; // r4
    gentity_s *v5; // r3
    const gentity_s *v6; // r30
    const char *v7; // r3
    const float *v8; // r4
    double Float; // fp31
    int v10; // r31
    unsigned int v11; // r10
    target_t *v12; // r11
    const char *v13; // r3
    gentity_s *ent; // r11
    float *offset; // r10
    float v17; // [sp+50h] [-50h] BYREF
    float v18; // [sp+54h] [-4Ch]
    float v19; // [sp+58h] [-48h]

    if (Scr_GetNumParam() < 2)
        Scr_Error("Too few arguments\n");
    Entity = Scr_GetEntity(0);
    v5 = Scr_GetEntity(1);
    v6 = v5;
    if (!v5->client)
    {
        v7 = va("entity %i is not a player", v5->s.number);
        Scr_ObjectError(v7);
    }
    Float = Scr_GetFloat(2u);
    if (Float <= 0.0)
        Scr_ParamError(2u, "FOV must be positive");
    v10 = 0;
    v11 = 0;
    v12 = &targGlob.targets[1];
    while (v12[-1].ent != Entity)
    {
        if (v12->ent == Entity)
        {
            ++v10;
            break;
        }
        if (v12[1].ent == Entity)
        {
            v10 += 2;
            break;
        }
        if (v12[2].ent == Entity)
        {
            v10 += 3;
            break;
        }
        v11 += 112;
        v10 += 4;
        v12 += 4;
        if (v11 >= 0x380)
            break;
    }
    if (v10 == 32)
    {
        v13 = va("Entity %i is not a target", Entity->s.number);
        Scr_Error(v13);
    }
    ent = targGlob.targets[v10].ent;
    offset = targGlob.targets[v10].offset;
    v17 = ent->r.currentOrigin[0] + *offset;
    v18 = ent->r.currentOrigin[1] + offset[1];
    v19 = ent->r.currentOrigin[2] + offset[2];
    v17 = v17 - v6->r.currentOrigin[0];
    v18 = v18 - v6->r.currentOrigin[1];
    v19 = v19 - v6->r.currentOrigin[2];
    v19 = v19 - v6->client->ps.viewHeightCurrent;
    return G_WorldDirToScreenPos(v6, Float, v8, &v17, screenPos);
}

void __cdecl Scr_Target_IsInCircle()
{
    double Float; // fp31
    int v1; // r3
    float v2; // [sp+50h] [-20h] BYREF
    float v3; // [sp+54h] [-1Ch]

    Float = Scr_GetFloat(3u);
    if (!(unsigned __int8)ScrGetTargetScreenPos(&v2)
        || (v1 = 1, (float)((float)(v2 * v2) + (float)(v3 * v3)) >= (double)(float)((float)Float * (float)Float)))
    {
        v1 = 0;
    }
    Scr_AddBool(v1);
}

void __cdecl Scr_Target_IsInRect()
{
    double Float; // fp31
    double v1; // fp30
    int v2; // r3
    float v3[2]; // [sp+50h] [-20h] BYREF

    Float = Scr_GetFloat(3u);
    v1 = Scr_GetFloat(4u);
    if (!(unsigned __int8)ScrGetTargetScreenPos(v3) || I_fabs(v3[0]) >= Float || (v2 = 1, I_fabs(v3[1]) >= v1))
        v2 = 0;
    Scr_AddBool(v2);
}

void __cdecl Scr_Target_StartLockOn()
{
    gentity_s *Entity; // r31
    double Float; // fp1
    int number; // r4
    const char *v5; // r3
    int v6; // [sp+50h] [-20h]

    Entity = Scr_GetEntity(0);
    Float = Scr_GetFloat(1u);
    number = Entity->s.number;
    v6 = (int)(float)((float)Float * (float)1000.0);
    v5 = va("ret_lock_on %i %i", number, v6);
    SV_GameSendServerCommand(-1, v5);
}

void __cdecl Scr_Target_ClearLockOn()
{
    const char *v0; // r3

    v0 = va("ret_lock_on %i %i", 2175, 0);
    SV_GameSendServerCommand(-1, v0);
}

int __cdecl GetTargetIdx(const gentity_s *ent)
{
    int v1; // r9
    TargetGlob *v2; // r11
    unsigned int v3; // r10

    if (!ent)
        return 32;
    v1 = 0;
    v2 = &targGlob;
    v3 = 0;
    while (v2->targets[0].ent != ent)
    {
        v3 += 28;
        ++v1;
        v2 = (TargetGlob *)((char *)v2 + 28);
        if (v3 >= 0x380)
            return 32;
    }
    return v1;
}

int __cdecl G_TargetGetOffset(const gentity_s *targ, float *result)
{
    float *v2; // r4
    int TargetIdx; // r3
    int v4; // r3
    float *offset; // r11

    TargetIdx = GetTargetIdx(targ);
    if (TargetIdx == 32)
    {
        v4 = 0;
        *v2 = 0.0;
        v2[1] = 0.0;
        v2[2] = 0.0;
    }
    else
    {
        offset = targGlob.targets[TargetIdx].offset;
        v4 = 1;
        *v2 = *offset;
        v2[1] = offset[1];
        v2[2] = offset[2];
    }
    return v4;
}

int __cdecl G_TargetAttackProfileTop(const gentity_s *ent)
{
    int TargetIdx; // r3

    TargetIdx = GetTargetIdx(ent);
    if (TargetIdx == 32)
        return 0;
    else
        return targGlob.targets[TargetIdx].flags & 1;
}

void __cdecl Scr_Target_SetAttackMode()
{
    unsigned int v0; // r4
    gentity_s *Entity; // r3
    int TargetIdx; // r29
    int v3; // r7
    const char *v4; // r3
    unsigned int ConstString; // r3
    const char *v6; // r3
    char v7[1056]; // [sp+50h] [-420h] BYREF

    if (Scr_GetNumParam() < 2)
        Scr_Error("Too few arguments\n");
    Entity = Scr_GetEntity(0);
    TargetIdx = GetTargetIdx(Entity);
    if (TargetIdx == 32)
    {
        v4 = va("Entity %i is not a target", *(unsigned __int16 *)(v3 + 118));
        Scr_Error(v4);
    }
    ConstString = Scr_GetConstString(1u);
    if (ConstString == scr_const.top)
    {
        targGlob.targets[TargetIdx].flags |= 1u;
    }
    else if (ConstString == scr_const.direct)
    {
        targGlob.targets[TargetIdx].flags &= ~1u;
    }
    else
    {
        Scr_Error("Incorrect mode name passed to target_setAttackMode().\n");
    }
    SV_GetConfigstring(TargetIdx + 27, v7, 1024);
    v6 = va("%i", targGlob.targets[TargetIdx].flags);
    Info_SetValueForKey(v7, "flags", v6);
    SV_SetConfigstring(TargetIdx + 27, v7);
}

void __cdecl Scr_Target_SetJavelinOnly()
{
    unsigned int v0; // r4
    gentity_s *Entity; // r3
    int TargetIdx; // r30
    int v3; // r7
    const char *v4; // r3
    int Int; // r3
    int *p_flags; // r31
    int v7; // r11
    unsigned int v8; // r11
    int v9; // r30
    const char *v10; // r3
    char v11[1032]; // [sp+50h] [-420h] BYREF

    if (Scr_GetNumParam() < 2)
        Scr_Error("Too few arguments\n");
    Entity = Scr_GetEntity(0);
    TargetIdx = GetTargetIdx(Entity);
    if (TargetIdx == 32)
    {
        v4 = va("Entity %i is not a target", *(unsigned __int16 *)(v3 + 118));
        Scr_Error(v4);
    }
    Int = Scr_GetInt(1u);
    p_flags = &targGlob.targets[TargetIdx].flags;
    v7 = *p_flags;
    if (Int)
        v8 = v7 | 2;
    else
        v8 = v7 & 0xFFFFFFFD;
    v9 = TargetIdx + 27;
    *p_flags = v8;
    SV_GetConfigstring(v9, v11, 1024);
    v10 = va("%i", *p_flags);
    Info_SetValueForKey(v11, "flags", v10);
    SV_SetConfigstring(v9, v11);
}

