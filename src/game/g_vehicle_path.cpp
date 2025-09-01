#include "g_vehicle_path.h"
#include "game_public.h"

#ifndef KISAK_SP
#error This file is Single-Player only
#endif
#include <script/scr_vm.h>
#include "g_main.h"
#include "g_local.h"

struct vn_field_t
{
    const char *name;
    int ofs;
    fieldtype_t type;
};

int s_newDebugLine = 1;

__int16 s_numNodes = 0;

float s_start[3] = { 0.0, 0.0, 0.0 };
float s_end[3] = { 0.0, 0.0, 0.0 };
float s_dir[3] = { 0.0, 0.0, 0.0 };

float s_invalidAngles[3] = { M_PI, M_PI, M_PI };

vehicle_node_t s_nodes[4000] = { 0 };

vn_field_t vn_fields[9] =
{
  { "targetname", 0, F_STRING },
  { "target", 2, F_STRING },
  { "script_linkname", 4, F_STRING },
  { "script_noteworthy", 6, F_STRING },
  { "origin", 24, F_VECTOR },
  { "angles", 48, F_VECTOR },
  { "speed", 16, F_FLOAT },
  { "lookahead", 20, F_FLOAT },
  { NULL, 0, F_INT }
};

void __cdecl VP_AddDebugLine(float *start, float *end, int forceDraw)
{
    double v6; // fp13
    double v7; // fp12
    double v10; // fp10
    double v11; // fp31
    double v12; // fp30
    double v13; // fp29
    double v14; // fp0
    float v15[6]; // [sp+50h] [-60h] BYREF

    //v5 = *start;
    v6 = (float)(end[2] - start[2]);
    v7 = (float)(end[1] - start[1]);

    // aislop
    //_FP7 = -sqrtf((float)((float)((float)v7 * (float)v7)
    //    + (float)((float)((float)v6 * (float)v6)
    //        + (float)((float)(*end - *start) * (float)(*end - *start)))));
    //__asm { fsel      f10, f7, f0, f10 }
    //v10 = (float)((float)1.0 / (float)_FP10);
    {
        float delta = *end - *start;
        float magnitude = sqrtf(v7 * v7 + v6 * v6 + delta * delta);
        float value = -magnitude;

        // Replace PowerPC `fsel` with portable C conditional:
        float selected = (value >= 0.0f) ? 0.0f : value;

        float v10 = 1.0f / selected;
    }

    v11 = (float)((float)v10 * (float)(*end - *start));
    v12 = (float)((float)v10 * (float)(end[1] - start[1]));
    v13 = (float)((float)(end[2] - start[2]) * (float)v10);

    if (s_newDebugLine)
    {
        s_newDebugLine = 0;
        s_start[0] = start[0];
        s_start[1] = start[1];
        s_start[2] = start[2];

        s_end[0] = end[0];
        s_end[1] = end[1];
        s_end[2] = end[2];

        s_dir[0] = v11;
        s_dir[1] = v12;
        s_dir[2] = v13;
    }
    else if ((float)((float)(s_dir[1] * (float)((float)v10 * (float)(end[1] - start[1])))
        + (float)((float)(s_dir[2] * (float)((float)(end[2] - start[2]) * (float)v10))
            + (float)(s_dir[0] * (float)((float)v10 * (float)(*end - *start))))) < 0.99989998
        || forceDraw)
    {
        v15[0] = 1.0;
        v15[3] = 1.0;
        v15[1] = 0.0;
        v15[2] = 0.0;
        G_DebugLine(s_start, s_end, v15, 1);
        s_start[0] = *start;
        s_start[1] = start[1];
        s_start[2] = start[2];
        s_end[0] = *end;
        s_end[1] = end[1];
        v14 = end[2];
        s_dir[0] = v11;
        s_dir[1] = v12;
        s_end[2] = v14;
        s_dir[2] = v13;
    }
    else
    {
        s_end[0] = *end;
        s_end[1] = end[1];
        s_end[2] = end[2];
    }
}

void __cdecl VP_SetScriptVariable(const char *key, const char *value, vehicle_node_t *node)
{
    unsigned int index; // r30
    int type; // [sp+50h] [-40h] BYREF
    float x; // [sp+58h] [-38h] BYREF
    float y; // [sp+5Ch] [-34h] BYREF
    float z; // [sp+60h] [-30h] BYREF

    index = Scr_FindField(key, &type);
    if (index)
    {
        switch (type)
        {
        case 2:
            Scr_AddString(value);
            Scr_SetDynamicEntityField(node->index, 3, index);
            break;
        case 4:
            x = 0.0;
            y = 0.0;
            z = 0.0;
            sscanf(value, "%f %f %f", &x, &y, &z);
            Scr_AddVector(&x);
            Scr_SetDynamicEntityField(node->index, 3, index);
            break;
        case 5:
            Scr_AddFloat(atof(value));
            Scr_SetDynamicEntityField(node->index, 3, index);
            break;
        case 6:
            Scr_AddInt(atol(value));
            Scr_SetDynamicEntityField(node->index, 3, index);
            break;
        default:
            Scr_Error(va("VP_SetScriptVariable: bad case %d", type));
            break;
        }
    }
}

void __cdecl VP_ParseField(const char *key, const char *value, vehicle_node_t *node)
{
    vn_field_t *f; // r31
    float x; // [sp+50h] [-40h] BYREF
    float y; // [sp+54h] [-3Ch] BYREF
    float z; // [sp+58h] [-38h] BYREF

    f = vn_fields;
    if (vn_fields[0].name)
    {
        while (I_stricmp(f->name, key))
        {
            ++f;
            if (!f->name)
            {
                VP_SetScriptVariable(key, value, node);
                return;
            }
        }
        switch (f->type)
        {
        case F_INT:
            *(unsigned int *)((char *)&node->name + f->ofs) = atol(value);
            break;
        case F_SHORT:
            *(unsigned __int16 *)((char *)&node->name + f->ofs) = atol(value);
            break;
        case F_BYTE:
            *((_BYTE *)&node->name + f->ofs) = atol(value);
            break;
        case F_FLOAT:
            *(float *)((char *)&node->name + f->ofs) = atof(value);
            break;
        case F_STRING:
            Scr_SetString((unsigned __int16 *)((char *)&node->name + f->ofs), 0);
            *(unsigned __int16 *)((char *)&node->name + f->ofs) = G_NewString(value);
            break;
        case F_VECTOR:
            x = 0.0f;
            y = 0.0f;
            z = 0.0f;
            sscanf(value, "%f %f %f", &x, &y, &z);
            *(float *)((char *)&node->name + f->ofs) = x;
            *(float *)((char *)&node->name + f->ofs + 4) = y;
            *(float *)((char *)&node->name + f->ofs + 8) = z;
            break;
        case F_VECTORHACK:
            x = 0.0;
            y = 0.0;
            z = 0.0;
            sscanf(value, "%f %f %f", &x, &y, &z);
            *(float *)((char *)&node->name + f->ofs) = AngleNormalize360(y);
            break;
        default:
            return;
        }
    }
    else
    {
        VP_SetScriptVariable(key, value, node);
    }
}

void __cdecl VP_ParseFields(vehicle_node_t *node)
{
    iassert(level.spawnVar.spawnVarsValid);

    for (int i = 0; i < level.spawnVar.numSpawnVars; i++)
    {
        VP_ParseField(level.spawnVar.spawnVars[i][0], level.spawnVar.spawnVars[i][1], node);
    }
}

void __cdecl VP_ClearNode(vehicle_node_t *node)
{
    Scr_SetString(&node->name, 0);
    Scr_SetString(&node->target, 0);
    Scr_SetString(&node->script_linkname, 0);
    Scr_SetString(&node->script_noteworthy, 0);
}

void __cdecl VP_CopyNode(const vehicle_node_t *src, vehicle_node_t *dst)
{
    Scr_SetString(&dst->name, src->name);
    Scr_SetString(&dst->target, src->target);
    Scr_SetString(&dst->script_linkname, src->script_linkname);
    Scr_SetString(&dst->script_noteworthy, src->script_noteworthy);
    dst->index = src->index;
    dst->rotated = src->rotated;
    dst->speed = src->speed;
    dst->lookAhead = src->lookAhead;
    dst->origin[0] = src->origin[0];
    dst->origin[1] = src->origin[1];
    dst->origin[2] = src->origin[2];
    dst->dir[0] = src->dir[0];
    dst->dir[1] = src->dir[1];
    dst->dir[2] = src->dir[2];
    dst->angles[0] = src->angles[0];
    dst->angles[1] = src->angles[1];
    dst->angles[2] = src->angles[2];
    dst->length = src->length;
    dst->nextIdx = src->nextIdx;
    dst->prevIdx = src->prevIdx;
}


int __cdecl VP_GetNodeIndex(unsigned __int16 name, float *origin)
{
    int v2; // r7
    int result; // r3
    int v4; // r10
    vehicle_node_t *v5; // r11

    v2 = name;
    if (!name)
        return -1;
    result = 0;
    if (s_numNodes <= 0)
        return -1;
    v4 = 0;
    while (1)
    {
        v5 = &s_nodes[v4];
        if (v5->name == v2
            && (!origin || v5->origin[0] == *origin && v5->origin[1] == origin[1] && v5->origin[2] == origin[2]))
        {
            break;
        }
        result = (__int16)(v4 + 1);
        v4 = result;
        if (result >= s_numNodes)
            return -1;
    }
    return result;
}

// aislop
float VP_CalcNodeSpeed(int16_t nodeIdx)
{
    if (nodeIdx < 0 || nodeIdx >= s_numNodes)
        return 0.0f;

    vehicle_node_t *node = &s_nodes[nodeIdx];
    float speed = node->speed;

    if (speed >= 0.0f)
        return speed;

    // Look backward for a node with valid speed
    float backLength = 0.0f;
    float backSpeed = -1.0f;
    int count = 0;

    int prevIdx = node->prevIdx;
    while (prevIdx >= 0 && prevIdx != nodeIdx && count < s_numNodes)
    {
        vehicle_node_t *prev = &s_nodes[prevIdx];
        backLength += prev->length;

        if (prev->speed >= 0.0f)
        {
            backSpeed = prev->speed;
            break;
        }

        prevIdx = prev->prevIdx;
        count++;
    }

    // Look forward for a node with valid speed
    float forwardLength = 0.0f;
    float forwardSpeed = -1.0f;
    count = 0;

    vehicle_node_t *cur = node;
    int nextIdx = cur->nextIdx;
    while (nextIdx >= 0 && nextIdx != nodeIdx && count < s_numNodes)
    {
        forwardLength += cur->length;
        cur = &s_nodes[nextIdx];

        if (cur->speed >= 0.0f)
        {
            forwardSpeed = cur->speed;
            break;
        }

        nextIdx = cur->nextIdx;
        count++;
    }

    // Interpolate if both forward and backward speeds are found
    if (backSpeed >= 0.0f && forwardSpeed >= 0.0f)
    {
        float totalLength = backLength + forwardLength;
        if (totalLength > 0.0f)
        {
            float factor = backLength / totalLength;
            return backSpeed + factor * (forwardSpeed - backSpeed);
        }
        else
        {
            return 0.0f;
        }
    }

    // If only one direction has a valid speed, return that
    if (forwardSpeed >= 0.0f)
        return forwardSpeed;

    if (backSpeed >= 0.0f)
        return backSpeed;

    // No valid speed found
    return 0.0f;
}


//aislop
float VP_CalcNodeLookAhead(int16_t nodeIdx)
{
    if (nodeIdx < 0 || nodeIdx >= s_numNodes)
        return 0.0f;

    vehicle_node_t *node = &s_nodes[nodeIdx];
    float lookAhead = node->lookAhead;

    // If current node already has valid lookAhead, return it
    if (lookAhead >= 0.0f)
        return lookAhead;

    // Look backward to find previous node with valid lookAhead
    float backLength = 0.0f;
    float backLookAhead = -1.0f;
    int count = 0;

    int prevIdx = node->prevIdx;
    while (prevIdx >= 0 && prevIdx != nodeIdx && count < s_numNodes)
    {
        vehicle_node_t *prev = &s_nodes[prevIdx];
        backLength += prev->length;

        if (prev->lookAhead >= 0.0f)
        {
            backLookAhead = prev->lookAhead;
            break;
        }

        prevIdx = prev->prevIdx;
        count++;
    }

    // Look forward to find next node with valid lookAhead
    float forwardLength = 0.0f;
    float forwardLookAhead = -1.0f;
    count = 0;

    vehicle_node_t *cur = node;
    int nextIdx = cur->nextIdx;
    while (nextIdx >= 0 && nextIdx != nodeIdx && count < s_numNodes)
    {
        forwardLength += cur->length;
        cur = &s_nodes[nextIdx];

        if (cur->lookAhead >= 0.0f)
        {
            forwardLookAhead = cur->lookAhead;
            break;
        }

        nextIdx = cur->nextIdx;
        count++;
    }

    // Interpolate lookAhead if both directions have values
    if (backLookAhead >= 0.0f && forwardLookAhead >= 0.0f)
    {
        float totalLength = backLength + forwardLength;
        if (totalLength > 0.0f)
        {
            float factor = backLength / totalLength;
            return backLookAhead + factor * (forwardLookAhead - backLookAhead);
        }
        else
        {
            return 0.0f;
        }
    }

    // If only one direction had a valid lookAhead, return that
    if (forwardLookAhead >= 0.0f)
        return forwardLookAhead;

    if (backLookAhead >= 0.0f)
        return backLookAhead;

    // No valid lookAhead found in either direction
    return 0.0f;
}


void __cdecl VP_CalcNodeAngles(__int16 nodeIdx, float *angles)
{
    vehicle_node_t *v2; // r11
    double v4; // fp0
    double v5; // fp13
    double v6; // fp12
    double v7; // fp1
    double v8; // fp28
    double v9; // fp27
    double v10; // fp2
    int prevIdx; // r10
    double v12; // fp9
    double v13; // fp10
    double v14; // fp30
    double v15; // fp29
    __int16 v16; // r9
    vehicle_node_t *v17; // r10
    int v18; // r10
    __int16 v19; // r9
    int nextIdx; // r10
    double v21; // fp31

    v2 = &s_nodes[nodeIdx];
    v4 = s_invalidAngles[0];
    if (v2->angles[0] != s_invalidAngles[0]
        || (v5 = s_invalidAngles[1], v2->angles[1] != s_invalidAngles[1])
        || (v6 = s_invalidAngles[2], v2->angles[2] != s_invalidAngles[2]))
    {
        *angles = v2->angles[0];
        angles[1] = v2->angles[1];
        angles[2] = v2->angles[2];
        return;
    }
    v7 = s_invalidAngles[0];
    v8 = s_invalidAngles[1];
    v9 = s_invalidAngles[2];
    v10 = s_invalidAngles[0];
    prevIdx = v2->prevIdx;
    v12 = 0.0;
    v13 = 0.0;
    v14 = s_invalidAngles[1];
    v15 = s_invalidAngles[2];
    if (prevIdx >= 0)
    {
        v16 = 0;
        v17 = &s_nodes[prevIdx];
        if (s_numNodes > 0)
        {
            while (1)
            {
                v13 = (float)(v17->length + (float)v13);
                ++v16;
                if (v17->angles[0] != v4 || v17->angles[1] != v5 || v17->angles[2] != v6)
                    break;
                v18 = v17->prevIdx;
                if (v18 >= 0 && v18 != nodeIdx)
                {
                    v17 = &s_nodes[v18];
                    if (v16 < s_numNodes)
                        continue;
                }
                goto LABEL_15;
            }
            v7 = v17->angles[0];
            v8 = v17->angles[1];
            v9 = v17->angles[2];
        }
    }
LABEL_15:
    v19 = 0;
    if (s_numNodes > 0)
    {
        while (1)
        {
            ++v19;
            if (v2->angles[0] != v4 || v2->angles[1] != v5 || v2->angles[2] != v6)
                break;
            nextIdx = v2->nextIdx;
            if (nextIdx >= 0 && nextIdx != nodeIdx)
            {
                v12 = (float)(v2->length + (float)v12);
                v2 = &s_nodes[nextIdx];
                if (v19 < s_numNodes)
                    continue;
            }
            goto LABEL_24;
        }
        v10 = v2->angles[0];
        v14 = v2->angles[1];
        v15 = v2->angles[2];
    }
LABEL_24:
    if (v7 != v4)
        goto LABEL_36;
    if (v8 == v5 && v9 == v6 && v10 == v4 && v14 == v5 && v15 == v6)
        goto LABEL_30;
    if (v8 == v5 && v9 == v6)
    {
        *angles = v10;
        angles[1] = v14;
        angles[2] = v15;
    }
    else
    {
    LABEL_36:
        if (v10 == v4 && v14 == v5 && v15 == v6)
        {
            *angles = v7;
            angles[1] = v8;
            angles[2] = v9;
        }
        else
        {
            if ((float)((float)v12 + (float)v13) <= 0.0)
            {
            LABEL_30:
                *angles = 0.0;
                angles[1] = 0.0;
                angles[2] = 0.0;
                return;
            }
            v21 = (float)((float)v13 / (float)((float)v12 + (float)v13));
            *angles = LerpAngle(v7, v10, v21);
            angles[1] = LerpAngle(v8, v14, v21);
            angles[2] = LerpAngle(v9, v15, v21);
        }
    }
}

float __cdecl VP_GetSpeed(const vehicle_pathpos_t *vpp)
{
    vehicle_node_t *v1; // r11
    int nextIdx; // r9
    double speed; // fp1

    v1 = &s_nodes[vpp->nodeIdx];
    nextIdx = v1->nextIdx;
    if (nextIdx >= 0)
        speed = (float)((float)((float)(s_nodes[nextIdx].speed - v1->speed) * vpp->frac) + v1->speed);
    else
        speed = v1->speed;
    return *((float *)&speed + 1);
}

float __cdecl VP_GetLookAhead(const vehicle_pathpos_t *vpp)
{
    vehicle_node_t *v1; // r11
    int nextIdx; // r9
    double lookAhead; // fp1

    v1 = &s_nodes[vpp->nodeIdx];
    nextIdx = v1->nextIdx;
    if (nextIdx >= 0)
        lookAhead = (float)((float)((float)(s_nodes[nextIdx].lookAhead - v1->lookAhead) * vpp->frac) + v1->lookAhead);
    else
        lookAhead = v1->lookAhead;
    return *((float *)&lookAhead + 1);
}

float __cdecl VP_GetSlide(const vehicle_pathpos_t *vpp)
{
    vehicle_node_t *v1; // r11
    __int16 nextIdx; // r9
    int rotated; // r11
    double frac; // fp1
    vehicle_node_t *v5; // r10

    v1 = &s_nodes[vpp->nodeIdx];
    nextIdx = v1->nextIdx;
    rotated = v1->rotated;
    if (nextIdx >= 0)
    {
        v5 = &s_nodes[nextIdx];
        if (rotated)
        {
            if (v5->rotated)
                goto LABEL_3;
            if (!v5->rotated)
            {
                frac = (float)((float)1.0 - vpp->frac);
                return *((float *)&frac + 1);
            }
        }
        else if (v5->rotated)
        {
            frac = vpp->frac;
            return *((float *)&frac + 1);
        }
    LABEL_12:
        frac = 0.0;
        return *((float *)&frac + 1);
    }
    if (!rotated)
        goto LABEL_12;
LABEL_3:
    frac = 1.0;
    return *((float *)&frac + 1);
}

void __cdecl VP_GetAngles(const vehicle_pathpos_t *vpp, float *angles)
{
    vehicle_node_t *v4; // r11
    int nextIdx; // r9
    vehicle_node_t *v6; // r10
    int i; // r31
    double v8; // fp1
    double v9; // fp31
    long double v10; // fp2
    long double v11; // fp2
    float v12; // [sp+50h] [-60h] BYREF
    float v13; // [sp+54h] [-5Ch]
    float v14; // [sp+58h] [-58h]
    float v15; // [sp+60h] [-50h] BYREF
    float v16; // [sp+64h] [-4Ch]
    float v17; // [sp+68h] [-48h]

    v4 = &s_nodes[vpp->nodeIdx];
    nextIdx = v4->nextIdx;
    if (nextIdx < 0)
    {
        if (v4->rotated)
        {
            *angles = v4->angles[0];
            angles[1] = v4->angles[1];
            angles[2] = v4->angles[2];
        }
        return;
    }
    v6 = &s_nodes[nextIdx];
    if (v4->rotated)
    {
        if (v6->rotated)
        {
            v15 = v4->angles[0];
            v16 = v4->angles[1];
            v17 = v4->angles[2];
            v12 = v6->angles[0];
            v13 = v6->angles[1];
            v14 = v6->angles[2];
            goto LABEL_13;
        }
    }
    else
    {
        if (!v6->rotated)
            return;
        if (v6->rotated)
        {
            v15 = *angles;
            v16 = angles[1];
            v17 = angles[2];
            v12 = v6->angles[0];
            v13 = v6->angles[1];
            v14 = v6->angles[2];
            goto LABEL_13;
        }
    }
    v12 = *angles;
    v13 = angles[1];
    v14 = angles[2];
    v15 = v4->angles[0];
    v16 = v4->angles[1];
    v17 = v4->angles[2];
LABEL_13:
    for (i = 0; i < 3; ++i)
    {
        v8 = LerpAngle(*(float *)((char *)&v15 + i * 4), *(float *)((char *)&v12 + i * 4), vpp->frac);
        v9 = (float)((float)v8 * (float)0.0027777778);
        angles[i] = v8;
        *(double *)&v10 = (float)((float)((float)v8 * (float)0.0027777778) + (float)0.5);
        v11 = floor(v10);
        angles[i] = (float)((float)v9 - (float)*(double *)&v11) * (float)360.0;
    }
}

void __cdecl VP_GetLookAheadXYZ(const vehicle_pathpos_t *vpp, float *lookXYZ)
{
    __int16 v2; // r10
    vehicle_node_t *v3; // r11
    double v4; // fp13
    int nextIdx; // r9
    double length; // fp0

    v2 = 0;
    v3 = &s_nodes[vpp->nodeIdx];
    v4 = (float)((float)(v3->length * vpp->frac) + (float)(vpp->lookAhead * vpp->speed));
    if (s_numNodes > 0)
    {
        while (1)
        {
            nextIdx = v3->nextIdx;
            ++v2;
            if (nextIdx < 0)
                break;
            length = v3->length;
            if (length == 0.0)
                break;
            if (v4 >= length)
            {
                v4 = (float)((float)v4 - v3->length);
                v3 = &s_nodes[nextIdx];
                if (v2 < s_numNodes)
                    continue;
            }
            goto LABEL_8;
        }
        v4 = 0.0;
    }
LABEL_8:
    *lookXYZ = (float)((float)v4 * v3->dir[0]) + v3->origin[0];
    lookXYZ[1] = (float)(v3->dir[1] * (float)v4) + v3->origin[1];
    lookXYZ[2] = (float)(v3->dir[2] * (float)v4) + v3->origin[2];
}

int __cdecl VP_UpdatePathPos(vehicle_pathpos_t *vpp, const float *dir, __int16 nodeTest)
{
    __int16 nodeIdx; // r8
    double frac; // fp6
    int v5; // r29
    __int16 v6; // r6
    vehicle_node_t *v7; // r11
    int v8; // r30
    __int16 nextIdx; // r5
    float *origin; // r10
    double v11; // fp11
    double v12; // fp0
    __int16 v13; // r11
    vehicle_node_t *v14; // r10
    double speed; // fp0
    int v16; // r11
    double lookAhead; // fp0
    int v18; // r11
    double Slide; // fp1
    int v20; // r3

    nodeIdx = vpp->nodeIdx;
    frac = vpp->frac;
    v5 = 0;
    v6 = 0;
    v7 = &s_nodes[vpp->nodeIdx];
    if (s_numNodes > 0)
    {
        v8 = nodeTest;
        while (1)
        {
            ++v6;
            v7 = &s_nodes[nodeIdx];
            if (nodeIdx == v8)
                v5 = 1;
            nextIdx = v7->nextIdx;
            if (nextIdx < 0)
                break;
            if (v7->length == 0.0)
                break;
            origin = s_nodes[nextIdx].origin;
            v11 = (float)((float)(*dir * (float)(vpp->origin[0] - v7->origin[0]))
                + (float)((float)(dir[1] * (float)(vpp->origin[1] - v7->origin[1]))
                    + (float)(dir[2] * (float)(vpp->origin[2] - v7->origin[2]))));
            v12 = (float)((float)(*dir * (float)(*origin - vpp->origin[0]))
                + (float)((float)(dir[1] * (float)(origin[1] - vpp->origin[1]))
                    + (float)(dir[2] * (float)(origin[2] - vpp->origin[2]))));
            if (v11 == 0.0 && v12 == 0.0)
                break;
            if (v11 >= 0.0 && v12 >= 0.0)
            {
                frac = (float)((float)((float)(*dir * (float)(vpp->origin[0] - v7->origin[0]))
                    + (float)((float)(dir[1] * (float)(vpp->origin[1] - v7->origin[1]))
                        + (float)(dir[2] * (float)(vpp->origin[2] - v7->origin[2]))))
                    / (float)((float)((float)(*dir * (float)(*origin - vpp->origin[0]))
                        + (float)((float)(dir[1] * (float)(origin[1] - vpp->origin[1]))
                            + (float)(dir[2] * (float)(origin[2] - vpp->origin[2]))))
                        + (float)((float)(*dir * (float)(vpp->origin[0] - v7->origin[0]))
                            + (float)((float)(dir[1] * (float)(vpp->origin[1] - v7->origin[1]))
                                + (float)(dir[2] * (float)(vpp->origin[2] - v7->origin[2]))))));
                goto LABEL_15;
            }
            nodeIdx = v7->nextIdx;
            if (v6 >= s_numNodes)
                goto LABEL_15;
        }
        frac = 0.0;
    }
LABEL_15:
    vpp->nodeIdx = nodeIdx;
    v13 = v7->nextIdx;
    vpp->frac = frac;
    v14 = &s_nodes[nodeIdx];
    //vpp->endOfPath = _cntlzw(v13) == 0;
    vpp->endOfPath = v13;
    speed = v14->speed;
    v16 = v14->nextIdx;
    if (v16 >= 0)
        speed = (float)((float)((float)(s_nodes[v16].speed - v14->speed) * (float)frac) + v14->speed);
    vpp->speed = speed;
    lookAhead = v14->lookAhead;
    v18 = v14->nextIdx;
    if (v18 >= 0)
        lookAhead = (float)((float)((float)(s_nodes[v18].lookAhead - v14->lookAhead) * (float)frac) + v14->lookAhead);
    vpp->lookAhead = lookAhead;
    Slide = VP_GetSlide(vpp);
    *(float *)(v20 + 16) = Slide;
    return v5;
}

void __cdecl VP_BeginSwitchNode(const vehicle_pathpos_t *vpp)
{
    int name; // r8
    const vehicle_node_t *switchNode; // r3
    __int16 v3; // r10
    int v4; // r11

    name = vpp->switchNode[0].name;
    switchNode = vpp->switchNode;
    if (name && (v3 = 0, s_numNodes > 0))
    {
        v4 = 0;
        while (s_nodes[v4].name != name)
        {
            v3 = v4 + 1;
            v4 = (__int16)(v4 + 1);
            if (v4 >= s_numNodes)
                goto LABEL_6;
        }
    }
    else
    {
    LABEL_6:
        v3 = -1;
    }
    if (v3 >= 0)
        VP_CopyNode(switchNode, &s_nodes[v3]);
}

void __cdecl VP_EndSwitchNode(const vehicle_pathpos_t *vpp)
{
    __int16 v1; // r10
    int v2; // r11

    if (vpp->switchNode[0].name && (v1 = 0, s_numNodes > 0))
    {
        v2 = 0;
        while (s_nodes[v2].name != vpp->switchNode[0].name)
        {
            v1 = v2 + 1;
            v2 = (__int16)(v2 + 1);
            if (v2 >= s_numNodes)
                goto LABEL_6;
        }
    }
    else
    {
    LABEL_6:
        v1 = -1;
    }
    if (v1 >= 0)
        VP_CopyNode(&vpp->switchNode[1], &s_nodes[v1]);
}

void __cdecl G_InitVehiclePaths()
{
    s_numNodes = 0;
}

void __cdecl G_FreeVehiclePaths()
{
    int v0; // r30
    vehicle_node_t *v1; // r31

    if (s_numNodes > 0)
    {
        v0 = 0;
        do
        {
            v1 = &s_nodes[v0];
            Scr_FreeEntityNum(v1->index, 3u);
            Scr_SetString(&v1->name, 0);
            Scr_SetString(&v1->target, 0);
            Scr_SetString(&v1->script_linkname, 0);
            Scr_SetString(&v1->script_noteworthy, 0);
            v0 = (__int16)(v0 + 1);
        } while (v0 < s_numNodes);
    }
    s_numNodes = 0;
}

void __cdecl G_FreeVehiclePathsScriptInfo()
{
    int v0; // r31

    if (s_numNodes > 0)
    {
        v0 = 0;
        do
        {
            Scr_FreeEntityNum(s_nodes[v0].index, 3u);
            v0 = (__int16)(v0 + 1);
        } while (v0 < s_numNodes);
    }
}

void __cdecl G_SetupVehiclePaths()
{
    // AKA G_SetupSplinePaths()

    for (int i = 0; i < s_numNodes; ++i)
    {
        vehicle_node_t *node = &s_nodes[i];

        if (node->target)
            node->nextIdx = VP_GetNodeIndex(node->target, 0);
        for (int j = 0; j < s_numNodes; ++j)
        {
            if (node->name && i != j && node->name == s_nodes[j].target)
            {
                node->prevIdx = j;
                break;
            }
        }
        if (node->nextIdx == i)
            node->nextIdx = -1;
        if (node->prevIdx == i)
            node->prevIdx = -1;
    }

    for (int i = 0; i < s_numNodes; ++i)
    {
        vehicle_node_t *node = &s_nodes[i];
        if (node->nextIdx >= 0)
        {
            float *origin = s_nodes[node->nextIdx].origin;
            node->dir[0] = origin[0] - node->origin[0];
            node->dir[1] = origin[1] - node->origin[1];
            node->dir[2] = origin[2] - node->origin[2];
            node->length = Vec3Normalize(node->dir);
            if (!node->rotated)
            {
                vectoangles(node->dir, node->angles);
            }
        }
    }

    for (int i = 0; i < s_numNodes; ++i)
    {
        vehicle_node_t *node = &s_nodes[i];

        node->speed = VP_CalcNodeSpeed(i);
        node->lookAhead = VP_CalcNodeLookAhead(i);

        if (node->speed < 0.0)
            Com_Error(ERR_DROP, "Vehicle path node at (%f, %f, %f) has negative speed", node->origin[0], node->origin[1], node->origin[2]);

        if (node->rotated)
            VP_CalcNodeAngles(i, node->angles);

        node->angles[0] = AngleNormalize180(node->angles[0]);
        node->angles[1] = AngleNormalize180(node->angles[1]);
        node->angles[2] = AngleNormalize180(node->angles[2]);

        if (node->speed <= 0.0 || node->lookAhead <= 0.0)
            node->nextIdx = -1;

        if (node->nextIdx < 0)
        {
            if (node->speed <= 0.0)
                node->speed = 1.0f;
            if (node->lookAhead <= 0.0)
                node->lookAhead = 1.0f;
        }
    }
}

void __cdecl G_VehInitPathPos(vehicle_pathpos_t *vpp)
{
    vpp->frac = 0.0;
    vpp->endOfPath = 0;
    vpp->speed = 0.0;
    vpp->nodeIdx = -1;
    vpp->lookAhead = 0.0;
    vpp->slide = 0.0;
    vpp->origin[0] = 0.0;
    vpp->origin[1] = 0.0;
    vpp->origin[2] = 0.0;
    vpp->angles[0] = 0.0;
    vpp->angles[1] = 0.0;
    vpp->angles[2] = 0.0;
    vpp->lookPos[0] = 0.0;
    vpp->lookPos[1] = 0.0;
    vpp->lookPos[2] = 0.0;
    vpp->switchNode[0].name = 0;
    vpp->switchNode[0].speed = -1.0;
    vpp->switchNode[0].target = 0;
    vpp->switchNode[0].lookAhead = -1.0;
    vpp->switchNode[0].script_linkname = 0;
    *(unsigned int *)&vpp->switchNode[0].script_noteworthy = 0xFFFF;
    vpp->switchNode[0].rotated = 0;
    vpp->switchNode[0].origin[0] = 0.0;
    vpp->switchNode[0].origin[1] = 0.0;
    vpp->switchNode[0].origin[2] = 0.0;
    vpp->switchNode[0].dir[0] = 0.0;
    vpp->switchNode[0].dir[1] = 0.0;
    vpp->switchNode[0].dir[2] = 0.0;
    vpp->switchNode[0].angles[0] = s_invalidAngles[0];
    vpp->switchNode[0].angles[1] = s_invalidAngles[1];
    vpp->switchNode[0].angles[2] = s_invalidAngles[2];
    vpp->switchNode[0].nextIdx = -1;
    vpp->switchNode[0].length = 0.0;
    vpp->switchNode[0].prevIdx = -1;
    vpp->switchNode[1].speed = -1.0;
    vpp->switchNode[1].name = 0;
    vpp->switchNode[1].lookAhead = -1.0;
    vpp->switchNode[1].target = 0;
    vpp->switchNode[1].script_linkname = 0;
    *(unsigned int *)&vpp->switchNode[1].script_noteworthy = 0xFFFF;
    vpp->switchNode[1].rotated = 0;
    vpp->switchNode[1].origin[0] = 0.0;
    vpp->switchNode[1].origin[1] = 0.0;
    vpp->switchNode[1].origin[2] = 0.0;
    vpp->switchNode[1].dir[0] = 0.0;
    vpp->switchNode[1].dir[1] = 0.0;
    vpp->switchNode[1].dir[2] = 0.0;
    vpp->switchNode[1].angles[0] = s_invalidAngles[0];
    vpp->switchNode[1].angles[1] = s_invalidAngles[1];
    vpp->switchNode[1].angles[2] = s_invalidAngles[2];
    vpp->switchNode[1].nextIdx = -1;
    vpp->switchNode[1].length = 0.0;
    vpp->switchNode[1].prevIdx = -1;
}

void __cdecl G_VehFreePathPos(vehicle_pathpos_t *vpp)
{
    vehicle_node_t *switchNode; // r31

    switchNode = vpp->switchNode;
    Scr_SetString(&vpp->switchNode[0].name, 0);
    Scr_SetString(&switchNode->target, 0);
    Scr_SetString(&switchNode->script_linkname, 0);
    Scr_SetString(&switchNode->script_noteworthy, 0);
    Scr_SetString(&vpp->switchNode[1].name, 0);
    Scr_SetString(&vpp->switchNode[1].target, 0);
    Scr_SetString(&vpp->switchNode[1].script_linkname, 0);
    Scr_SetString(&vpp->switchNode[1].script_noteworthy, 0);
}

void __cdecl G_VehSetUpPathPos(vehicle_pathpos_t *vpp, __int16 nodeIdx)
{
    vehicle_node_t *v2; // r9
    double v3; // fp13

    vpp->nodeIdx = nodeIdx;
    vpp->frac = 0.0;
    vpp->endOfPath = 0;
    v2 = &s_nodes[nodeIdx];
    vpp->speed = v2->speed;
    vpp->lookAhead = v2->lookAhead;
    if (v2->rotated)
        v3 = 1.0;
    else
        v3 = 0.0;
    vpp->slide = v3;
    vpp->origin[0] = v2->origin[0];
    vpp->origin[1] = v2->origin[1];
    vpp->origin[2] = v2->origin[2];
    vpp->angles[0] = v2->angles[0];
    vpp->angles[1] = v2->angles[1];
    vpp->angles[2] = v2->angles[2];
    vpp->lookPos[0] = v2->origin[0];
    vpp->lookPos[1] = v2->origin[1];
    vpp->lookPos[2] = v2->origin[2];
    vpp->switchNode[0].name = 0;
    vpp->switchNode[0].speed = -1.0;
    vpp->switchNode[0].target = 0;
    vpp->switchNode[0].lookAhead = -1.0;
    vpp->switchNode[0].script_linkname = 0;
    vpp->switchNode[0].script_noteworthy = 0;
    vpp->switchNode[0].index = -1;
    vpp->switchNode[0].rotated = 0;
    vpp->switchNode[0].origin[0] = 0.0;
    vpp->switchNode[0].origin[1] = 0.0;
    vpp->switchNode[0].origin[2] = 0.0;
    vpp->switchNode[0].dir[0] = 0.0;
    vpp->switchNode[0].dir[1] = 0.0;
    vpp->switchNode[0].dir[2] = 0.0;
    vpp->switchNode[0].angles[0] = s_invalidAngles[0];
    vpp->switchNode[0].angles[1] = s_invalidAngles[1];
    vpp->switchNode[0].angles[2] = s_invalidAngles[2];
    vpp->switchNode[0].nextIdx = -1;
    vpp->switchNode[0].length = 0.0;
    vpp->switchNode[0].prevIdx = -1;
    vpp->switchNode[1].speed = -1.0;
    vpp->switchNode[1].name = 0;
    vpp->switchNode[1].lookAhead = -1.0;
    vpp->switchNode[1].target = 0;
    vpp->switchNode[1].script_linkname = 0;
    vpp->switchNode[1].script_noteworthy = 0;
    vpp->switchNode[1].index = -1;
    vpp->switchNode[1].rotated = 0;
    vpp->switchNode[1].origin[0] = 0.0;
    vpp->switchNode[1].origin[1] = 0.0;
    vpp->switchNode[1].origin[2] = 0.0;
    vpp->switchNode[1].dir[0] = 0.0;
    vpp->switchNode[1].dir[1] = 0.0;
    vpp->switchNode[1].dir[2] = 0.0;
    vpp->switchNode[1].angles[0] = s_invalidAngles[0];
    vpp->switchNode[1].angles[1] = s_invalidAngles[1];
    vpp->switchNode[1].angles[2] = s_invalidAngles[2];
    vpp->switchNode[1].nextIdx = -1;
    vpp->switchNode[1].length = 0.0;
    vpp->switchNode[1].prevIdx = -1;
}

// aislop
int G_VehUpdatePathPos(vehicle_pathpos_t *vpp, int16_t testNode)
{
    if (vpp->endOfPath)
        return 0;

    VP_BeginSwitchNode(vpp);
    VP_GetLookAheadXYZ(vpp, vpp->lookPos);

    float *lookAhead = vpp->lookPos;
    float dx = lookAhead[0] - vpp->origin[0];
    float dy = lookAhead[1] - vpp->origin[1];
    float dz = lookAhead[2] - vpp->origin[2];

    float magnitude = sqrtf(dx * dx + dy * dy + dz * dz);

    if (magnitude > 0.0f)
    {
        // Normalize direction vector
        float invMag = 1.0f / magnitude;
        float vx = dx * invMag;
        float vy = dy * invMag;
        float vz = dz * invMag;

        // Convert direction vector to angles
        float dir[3] = { vx, vy, vz };
        vectoangles(dir, vpp->angles);

        // Normalize angles
        vpp->angles[0] = AngleNormalize180(vpp->angles[0]);
        vpp->angles[1] = AngleNormalize180(vpp->angles[1]);
        vpp->angles[2] = AngleNormalize180(vpp->angles[2]);

        // Move along the direction vector
        float step = vpp->speed * 0.05f;
        vpp->origin[0] += vx * step;
        vpp->origin[1] += vy * step;
        vpp->origin[2] += vz * step;

        int updated = VP_UpdatePathPos(vpp, dir, testNode);

        VP_GetAngles(vpp, vpp->angles);
        VP_EndSwitchNode(vpp);

        return updated;
    }
    else
    {
        vpp->endOfPath = 1;
        VP_EndSwitchNode(vpp);
        return 0;
    }
}


// aislop
void G_VehSetSwitchNode(vehicle_pathpos_t *vpp, int16_t srcNodeIdx, int16_t dstNodeIdx)
{
    G_VehFreePathPos(vpp);

    // Reset switchNode[0] and switchNode[1]
    for (int i = 0; i < 2; ++i) {
        vpp->switchNode[i].speed = -1.0f;
        vpp->switchNode[i].lookAhead = -1.0f;
        vpp->switchNode[i].index = -1;
        vpp->switchNode[i].name = NULL;
        vpp->switchNode[i].target = NULL;
        vpp->switchNode[i].script_linkname = NULL;
        vpp->switchNode[i].script_noteworthy = NULL;
        vpp->switchNode[i].rotated = 0;

        for (int j = 0; j < 3; ++j) {
            vpp->switchNode[i].origin[j] = 0.0f;
            vpp->switchNode[i].dir[j] = 0.0f;
            vpp->switchNode[i].angles[j] = s_invalidAngles[j];
        }

        vpp->switchNode[i].nextIdx = -1;
        vpp->switchNode[i].prevIdx = -1;
        vpp->switchNode[i].length = 0.0f;
    }

    // Set switch node data if indices are valid
    if (srcNodeIdx >= 0 && dstNodeIdx >= 0)
    {
        vehicle_node_t *srcNode = &s_nodes[srcNodeIdx];
        vehicle_node_t *dstNode = &s_nodes[dstNodeIdx];

        // Copy source node into both switch slots
        VP_CopyNode(srcNode, &vpp->switchNode[0]);
        VP_CopyNode(srcNode, &vpp->switchNode[1]);

        // Set path to next node
        vpp->switchNode[0].nextIdx = dstNodeIdx;

        // Compute direction vector from src to dst
        float dx = dstNode->origin[0] - srcNode->origin[0];
        float dy = dstNode->origin[1] - srcNode->origin[1];
        float dz = dstNode->origin[2] - srcNode->origin[2];

        float length = sqrtf(dx * dx + dy * dy + dz * dz);

        // Normalize direction vector if length is valid
        if (length > 0.0f) {
            float invLength = 1.0f / length;
            vpp->switchNode[0].dir[0] = dx * invLength;
            vpp->switchNode[0].dir[1] = dy * invLength;
            vpp->switchNode[0].dir[2] = dz * invLength;
        }
        else {
            vpp->switchNode[0].dir[0] = 0.0f;
            vpp->switchNode[0].dir[1] = 0.0f;
            vpp->switchNode[0].dir[2] = 0.0f;
        }

        vpp->switchNode[0].length = length;
    }
}


void __cdecl TRACK_g_vehicle_path()
{
    track_static_alloc_internal(s_nodes, 272000, "s_nodes", 9);
}

static void VP_ZeroNode(vehicle_node_t *node)
{
    node->name = 0;
    node->target = 0;
    node->script_linkname = 0;
    node->script_noteworthy = 0;
}

static void VP_InitNode(vehicle_node_t *node, short nodeIdx)
{
    VP_ZeroNode(node);

    node->index = nodeIdx;
    node->speed = -1.0f;
    node->lookAhead = -1.0f;

    node->rotated = 0;

    node->origin[0] = 0.0f;
    node->origin[1] = 0.0f;
    node->origin[2] = 0.0f;

    node->dir[0] = 0.0f;
    node->dir[1] = 0.0f;
    node->dir[2] = 0.0f;

    node->angles[0] = s_invalidAngles[0];
    node->angles[1] = s_invalidAngles[1];
    node->angles[2] = s_invalidAngles[2];

    node->length = 0.0f;

    node->prevIdx = -1;
    node->nextIdx = -1;
}

void __cdecl SP_info_vehicle_node(int rotated)
{
    vehicle_node_t *node; // r31
    int name; // r11

    if (s_numNodes >= 4000)
    {
        Com_Error(ERR_DROP, "Hit max vehicle path node count [%d]", 4000);
    }

    node = &s_nodes[s_numNodes];

    VP_InitNode(node, s_numNodes);

    s_numNodes++;

    VP_ParseFields(node);

    name = node->name;
    node->rotated = rotated;
    if (!name)
        Com_Error(
            ERR_DROP,
            "Vehicle path node (%f, %f, %f) found with no name",
            node->origin[0],
            node->origin[1],
            node->origin[2]
        );
    if (node->speed >= 0.0f)
        node->speed = node->speed * 17.6f;
}

int __cdecl GScr_GetVehicleNodeIndex(int index)
{
    scr_entref_t EntityRef; // [sp+50h] [-20h]

    EntityRef = Scr_GetEntityRef(index);
    if (EntityRef.classnum == 3)
    {
        if (EntityRef.entnum >= s_numNodes)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\universal\\g_vehicle_path.cpp",
                1421,
                0,
                "%s",
                "entref.entnum < s_numNodes");
        return EntityRef.entnum;
    }
    else
    {
        Scr_ParamError((unsigned int)index, "Not a vehicle node");
        return -1;
    }
}

void __cdecl GScr_AddFieldsForVehicleNode()
{
    vn_field_t *v0; // r28
    int v1; // r30

    v0 = vn_fields;
    if (vn_fields[0].name)
    {
        v1 = 0;
        do
        {
            if (v1 / 12 != (unsigned __int16)(v1 / 12))
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\universal\\g_vehicle_path.cpp",
                    1436,
                    0,
                    "%s",
                    "(f - vn_fields) == (unsigned short)( f - vn_fields )");
            Scr_AddClassField(3u, (char*)v0->name, (unsigned __int16)(v1 / 12));
            ++v0;
            v1 += 12;
        } while (v0->name);
    }
}

void __cdecl GScr_GetVehicleNodeField(unsigned int entnum, unsigned int offset)
{
    if (offset >= 8)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\universal\\g_vehicle_path.cpp",
            1447,
            0,
            "offset doesn't index ARRAY_COUNT( vn_fields ) - 1\n\t%i not in [0, %i)",
            offset,
            8);
    if (entnum >= s_numNodes)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\universal\\g_vehicle_path.cpp",
            1448,
            0,
            "entnum doesn't index s_numNodes\n\t%i not in [0, %i)",
            entnum,
            s_numNodes);
    Scr_GetGenericField((unsigned __int8 *)&s_nodes[entnum], vn_fields[offset].type, vn_fields[offset].ofs);
}

void __cdecl GScr_GetVehicleNode()
{
    unsigned int ConstString; // r25
    const char *String; // r3
    int Offset; // r3
    int v3; // r31
    vn_field_t *v4; // r28
    vehicle_node_t *v5; // r9
    __int16 v6; // r10
    int v7; // r30
    int ofs; // r8
    vehicle_node_t *v9; // r31

    ConstString = Scr_GetConstString(0);
    String = Scr_GetString(1);
    Offset = Scr_GetOffset(3u, String);
    v3 = Offset;
    if (Offset >= 0)
    {
        if ((unsigned int)Offset >= 8)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\universal\\g_vehicle_path.cpp",
                1485,
                0,
                "offset doesn't index ARRAY_COUNT( vn_fields ) - 1\n\t%i not in [0, %i)",
                Offset,
                8);
        v4 = &vn_fields[v3];
        if (v4->type != F_STRING)
            Scr_ParamError(1u, "key is not internally a string");
        v5 = 0;
        v6 = s_numNodes;
        if (s_numNodes > 0)
        {
            v7 = 0;
            do
            {
                ofs = v4->ofs;
                v9 = &s_nodes[v7];
                if (*(unsigned __int16 *)((char *)&v9->name + ofs)
                    && *(unsigned __int16 *)((char *)&v9->name + ofs) == ConstString)
                {
                    if (v5)
                    {
                        Scr_Error("GetVehicleNode used with more than one node");
                        v6 = s_numNodes;
                    }
                    v5 = &s_nodes[v7];
                }
                v7 = (__int16)(v7 + 1);
            } while (v7 < v6);
            if (v5)
                Scr_AddEntityNum(v5->index, 3u);
        }
    }
}

void __cdecl GScr_GetVehicleNodeArray()
{
    unsigned int ConstString; // r27
    const char *String; // r3
    int Offset; // r3
    int v3; // r31
    vn_field_t *v4; // r29
    int v5; // r30
    __int16 v6; // r10
    vehicle_node_t *v7; // r31

    ConstString = Scr_GetConstString(0);
    String = Scr_GetString(1);
    Offset = Scr_GetOffset(3u, String);
    v3 = Offset;
    if (Offset >= 0)
    {
        if ((unsigned int)Offset >= 8)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\universal\\g_vehicle_path.cpp",
                1543,
                0,
                "offset doesn't index ARRAY_COUNT( vn_fields ) - 1\n\t%i not in [0, %i)",
                Offset,
                8);
        v4 = &vn_fields[v3];
        if (v4->type != F_STRING)
            Scr_ParamError(1u, "key is not internally a string");
        Scr_MakeArray();
        v5 = 0;
        v6 = s_numNodes;
        if (s_numNodes > 0)
        {
            v7 = s_nodes;
            do
            {
                if (*(unsigned __int16 *)((char *)&v7->name + v4->ofs))
                {
                    if (*(unsigned __int16 *)((char *)&v7->name + v4->ofs) == ConstString)
                    {
                        Scr_AddEntityNum(v7->index, 3u);
                        Scr_AddArray();
                        v6 = s_numNodes;
                    }
                }
                ++v5;
                ++v7;
            } while (v5 < v6);
        }
    }
}

void __cdecl GScr_GetAllVehicleNodes()
{
    int v0; // r31

    Scr_MakeArray();
    if (s_numNodes > 0)
    {
        v0 = 0;
        do
        {
            Scr_AddEntityNum(s_nodes[v0].index, 3u);
            Scr_AddArray();
            v0 = (__int16)(v0 + 1);
        } while (v0 < s_numNodes);
    }
}

void __cdecl VP_DrawPath(const vehicle_pathpos_t *vpp)
{
    int v2; // r29
    int v3; // r30
    __int16 nodeIdx; // r31
    int updated; // r3
    const float *v6; // r6
    int v7; // r29
    int v8; // r11
    vehicle_node_t *i; // r31
    double v10; // fp1
    float *v11; // r7
    int nextIdx; // r10
    float v13[4]; // [sp+50h] [-200h] BYREF
    float v14[4]; // [sp+60h] [-1F0h] BYREF
    float v15[4]; // [sp+70h] [-1E0h] BYREF
    float v16[4]; // [sp+80h] [-1D0h] BYREF
    vehicle_pathpos_t v17; // [sp+90h] [-1C0h] BYREF
    float v18[48]; // [sp+150h] [-100h] BYREF

    v2 = 0;
    v3 = 0;
    nodeIdx = -1;
    memcpy(v18, vpp, sizeof(v18));
    memcpy(&v17, vpp, sizeof(v17));
    s_newDebugLine = 1;
    while (++v3 <= 50000)
    {
        if (SHIWORD(v18[0]) != vpp->nodeIdx)
            nodeIdx = vpp->nodeIdx;
        memcpy(v18, &v17, sizeof(v18));
        updated = G_VehUpdatePathPos(&v17, nodeIdx);
        if (v17.endOfPath || updated)
            v2 = 1;
        VP_AddDebugLine(&v18[5], v17.origin, v2);
        if (v2)
            goto LABEL_11;
    }
    Com_PrintWarning(16, "WARNING: Invalid vehicle path.  Possible infinite loop\n");
LABEL_11:
    v7 = 0;
    v8 = vpp->nodeIdx;
    v15[0] = 0.0;
    v15[2] = 0.0;
    v16[0] = 0.0;
    v16[1] = 0.0;
    v15[1] = 1.0;
    v15[3] = 1.0;
    v16[2] = 1.0;
    v16[3] = 1.0;
    for (i = &s_nodes[v8]; v7 < s_numNodes; i = &s_nodes[nextIdx])
    {
        v10 = i->angles[1];
        v14[0] = -4.0;
        v14[1] = -4.0;
        v14[2] = -4.0;
        v13[0] = 4.0;
        v13[1] = 4.0;
        v13[2] = 4.0;
        ++v7;
        v11 = v15;
        if (i != &s_nodes[v8])
            v11 = v16;
        G_DebugBox(i->origin, v14, v13, v10, v6, (int)v11, 1);
        nextIdx = i->nextIdx;
        if (nextIdx < 0)
            break;
        v8 = vpp->nodeIdx;
        if (nextIdx == v8)
            break;
    }
}

void __cdecl G_DrawVehiclePaths()
{
    __int16 v0; // r29
    int v1; // r31
    const char *v2; // r11
    unsigned __int8 *integer; // r10
    int v4; // r8
    vehicle_pathpos_t v5; // [sp+50h] [-100h] BYREF

    if (*(_BYTE *)g_vehicleDrawPath->current.integer)
    {
        if (*(_BYTE *)g_vehicleDrawPath->current.integer != 48)
        {
            v0 = 0;
            if (s_numNodes > 0)
            {
                v1 = 0;
                while (1)
                {
                    v2 = SL_ConvertToString(s_nodes[v1].name);
                    integer = (unsigned __int8 *)g_vehicleDrawPath->current.integer;
                    do
                    {
                        v4 = *(unsigned __int8 *)v2 - *integer;
                        if (!*v2)
                            break;
                        ++v2;
                        ++integer;
                    } while (!v4);
                    if (!v4)
                        break;
                    v0 = v1 + 1;
                    v1 = (__int16)(v1 + 1);
                    if (v1 >= s_numNodes)
                        return;
                }
                v5.switchNode[0].name = 0;
                v5.switchNode[0].target = 0;
                v5.switchNode[0].script_linkname = 0;
                v5.switchNode[0].script_noteworthy = 0;
                v5.switchNode[1].name = 0;
                v5.switchNode[1].target = 0;
                v5.switchNode[1].script_linkname = 0;
                v5.switchNode[1].script_noteworthy = 0;
                G_VehSetUpPathPos(&v5, v0);
                VP_DrawPath(&v5);
            }
        }
    }
}

