#include "xmodel.h"
#include <universal/com_files.h>
#include <universal/q_parse.h>
#include <common/brush.h>
#include <ode/mass.h>
#include <qcommon/qcommon.h>
#include <physics/phys_local.h>

// KISAKTODO: move to "mass.cpp", but it's different from the ODE one.
void __cdecl Phys_ComputeMassProperties(
    const float *mins,
    const float *maxs,
    bool(__cdecl *isBoxInSolid)(const float *, const float *, const PhysGeomList *),
    void *userData,
    float *centerOfMass,
    float *momentsOfInertia,
    float *productsOfInertia)
{
    float I12; // [esp+1Ch] [ebp-104h]
    float scale; // [esp+20h] [ebp-100h]
    float I23; // [esp+24h] [ebp-FCh]
    float I11; // [esp+28h] [ebp-F8h]
    float I22; // [esp+2Ch] [ebp-F4h]
    float I33; // [esp+30h] [ebp-F0h]
    float v13; // [esp+34h] [ebp-ECh]
    float v14; // [esp+38h] [ebp-E8h]
    float v15; // [esp+3Ch] [ebp-E4h]
    float v16; // [esp+4Ch] [ebp-D4h]
    float v17; // [esp+50h] [ebp-D0h]
    float v18; // [esp+58h] [ebp-C8h]
    float v19; // [esp+5Ch] [ebp-C4h]
    float v20; // [esp+64h] [ebp-BCh]
    float v21; // [esp+68h] [ebp-B8h]
    float v22; // [esp+74h] [ebp-ACh]
    float v23; // [esp+7Ch] [ebp-A4h]
    unsigned int j; // [esp+80h] [ebp-A0h]
    float pointSum[3]; // [esp+84h] [ebp-9Ch] BYREF
    unsigned int k; // [esp+90h] [ebp-90h]
    float halfLengths[3]; // [esp+94h] [ebp-8Ch] BYREF
    float xzDistSq; // [esp+A0h] [ebp-80h]
    float yzDistSq; // [esp+A4h] [ebp-7Ch]
    float xzSum; // [esp+A8h] [ebp-78h]
    unsigned int pointCount; // [esp+ACh] [ebp-74h]
    dMass mass; // [esp+B0h] [ebp-70h] BYREF
    float dz; // [esp+F8h] [ebp-28h]
    float invPointCount; // [esp+FCh] [ebp-24h]
    float xySum; // [esp+100h] [ebp-20h]
    unsigned int i; // [esp+104h] [ebp-1Ch]
    float dy; // [esp+108h] [ebp-18h]
    float yzSum; // [esp+10Ch] [ebp-14h]
    float point[3]; // [esp+110h] [ebp-10h] BYREF
    float xyDistSq; // [esp+11Ch] [ebp-4h]

    dMassSetZero(&mass);
    v23 = (*maxs - *mins) / 32.0;
    dy = (maxs[1] - mins[1]) / 32.0;
    dz = (maxs[2] - mins[2]) / 32.0;
    halfLengths[0] = v23 * 0.5;
    halfLengths[1] = dy * 0.5;
    halfLengths[2] = dz * 0.5;
    pointCount = 0;
    pointSum[0] = 0.0;
    pointSum[1] = 0.0;
    pointSum[2] = 0.0;
    xyDistSq = 0.0;
    xzDistSq = 0.0;
    yzDistSq = 0.0;
    xySum = 0.0;
    xzSum = 0.0;
    yzSum = 0.0;
    point[0] = *mins + halfLengths[0];
    for (i = 0; i < 0x20; ++i)
    {
        point[1] = mins[1] + halfLengths[1];
        for (j = 0; j < 0x20; ++j)
        {
            point[2] = mins[2] + halfLengths[2];
            for (k = 0; k < 0x20; ++k)
            {
                if (isBoxInSolid(point, halfLengths, (const PhysGeomList*)userData))
                {
                    ++pointCount;
                    Vec3Add(pointSum, point, pointSum);
                    v22 = point[0] * point[0];
                    v17 = point[1] * point[1];
                    xyDistSq = v22 + v17 + xyDistSq;
                    v16 = point[2] * point[2];
                    xzDistSq = v22 + v16 + xzDistSq;
                    yzDistSq = v17 + v16 + yzDistSq;
                    xySum = point[0] * point[1] + xySum;
                    xzSum = point[0] * point[2] + xzSum;
                    yzSum = point[1] * point[2] + yzSum;
                }
                point[2] = point[2] + dz;
            }
            point[1] = point[1] + dy;
        }
        point[0] = point[0] + v23;
    }
    if (pointCount)
    {
        invPointCount = 1.0 / pointCount;
        Vec3Scale(pointSum, invPointCount, centerOfMass);
        v15 = -yzSum * invPointCount;
        v14 = -xzSum * invPointCount;
        v13 = -xySum * invPointCount;
        I33 = xyDistSq * invPointCount;
        I22 = xzDistSq * invPointCount;
        I11 = yzDistSq * invPointCount;
        dMassSetParameters(&mass, 1.0, *centerOfMass, centerOfMass[1], centerOfMass[2], I11, I22, I33, v13, v14, v15);
        I23 = -centerOfMass[2];
        scale = -centerOfMass[1];
        I12 = -*centerOfMass;
        dMassTranslate(&mass, I12, scale, I23);
        v20 = mass.I[5];
        v21 = mass.I[10];
        *momentsOfInertia = mass.I[0];
        momentsOfInertia[1] = v20;
        momentsOfInertia[2] = v21;
        v18 = mass.I[2];
        v19 = mass.I[6];
        *productsOfInertia = mass.I[1];
        productsOfInertia[1] = v18;
        productsOfInertia[2] = v19;
    }
}





char __cdecl SkipEpair(char *token, const char **file)
{
    char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    int v6; // eax
    int v7; // eax
    int v8; // eax
    parseInfo_t *tokena; // [esp+28h] [ebp+8h]

    if (token[strlen(token) - 2] == 92)
    {
        Com_PrintError(19, "SkipEpair: key '%s' ends with a '\\'\n", token);
        return 0;
    }
    else
    {
        v3 = strchr(token, 0xAu);
        if (v3 || (strchr(token, 0xDu)))
        {
            Com_PrintError(19, "SkipEpair: key '%s' contains a newline character\n", token);
            return 0;
        }
        else
        {
            v5 = strchr(token, 0x22u);
            if (v5)
            {
                Com_PrintError(19, "SkipEpair: key '%s' contains a \" character, will cause parsing errors\n", token);
                return 0;
            }
            else
            {
                tokena = Com_ParseOnLine(file);
                if (tokena->token[strlen(tokena->token) - 2] == 92)
                {
                    Com_PrintError(19, "SkipEpair: value '%s' ends with a '\\'\n", tokena->token);
                    return 0;
                }
                else
                {
                    if (strchr(tokena->token, 0xAu) || (strchr(tokena->token, 0xDu)))
                    {
                        Com_PrintError(
                            19,
                            "SkipEpair: value '%s' contains a newline character (use of '\\' at end of value?)\n",
                            tokena->token);
                        return 0;
                    }
                    else
                    {
                        if (strchr(tokena->token, 0x22u))
                        {
                            Com_PrintError(
                                19,
                                "SkipEpair: value '%s' contains a \" character, will cause parsing errors\n",
                                tokena->token);
                            return 0;
                        }
                        else
                        {
                            return 1;
                        }
                    }
                }
            }
        }
    }
}

unsigned int __cdecl Xmodel_CountPhysicsCollMapGeoms(const char **file, const char *name)
{
    int CurrentParseLine; // eax
    unsigned int brushCount; // [esp+B4h] [ebp-8h]
    parseInfo_t *token; // [esp+B8h] [ebp-4h]
    parseInfo_t *tokena; // [esp+B8h] [ebp-4h]
    parseInfo_t *tokenb; // [esp+B8h] [ebp-4h]
    parseInfo_t *tokenc; // [esp+B8h] [ebp-4h]

    if (!strcmp(Com_Parse(file)->token, "iwmap"))
    {
        token = Com_Parse(file);
        if (atoi(token->token) == 4)
        {
            while (1)
            {
                tokena = Com_Parse(file);
                if (!tokena->token[0])
                    return 0;
                if (!strcmp(tokena->token, "{"))
                    break;
                Com_SkipRestOfLine(file);
            }
            if (!strcmp(tokena->token, "{"))
            {
                brushCount = 0;
                while (1)
                {
                    tokenb = Com_Parse(file);
                    if (!tokenb->token[0])
                    {
                        Com_PrintError(19, "ERROR: ParsePhysicsCollMap: EOF without closing brace");
                        return 0;
                    }
                    if (!strcmp(tokenb->token, "}"))
                        break;
                    if (!strcmp(tokenb->token, "{"))
                    {
                        tokenc = Com_Parse(file);
                        if (!tokenc->token[0])
                            return brushCount;
                        if (!strcmp(tokenc->token, "curve"))
                        {
                            Com_PrintError(19, "ERROR: cannot have curves in collision maps");
                            return 0;
                        }
                        if (!strcmp(tokenc->token, "mesh"))
                        {
                            Com_PrintError(19, "ERROR: cannot have patch terrain in collision maps");
                            return 0;
                        }
                        if (!strcmp(tokenc->token, "physics_cylinder"))
                        {
                            if (Com_SkipBracedSection(file, 0, 1))
                                return 0;
                            if (!Com_MatchToken(file, "}", 1))
                                return 0;
                            ++brushCount;
                        }
                        else if (!strcmp(tokenc->token, "physics_box"))
                        {
                            if (Com_SkipBracedSection(file, 0, 1))
                                return 0;
                            if (!Com_MatchToken(file, "}", 1))
                                return 0;
                            ++brushCount;
                        }
                        else
                        {
                            Com_UngetToken();
                            if (Com_SkipBracedSection(file, 1u, 1))
                                return 0;
                            ++brushCount;
                        }
                    }
                    else if (!SkipEpair(tokenb->token, file))
                    {
                        return 0;
                    }
                }
                return brushCount;
            }
            else
            {
                CurrentParseLine = Com_GetCurrentParseLine();
                Com_PrintError(
                    19,
                    "ERROR: ParsePhysicsCollMap: { not found, found %s on line %d",
                    tokena->token,
                    CurrentParseLine);
                return 0;
            }
        }
        else
        {
            Com_PrintError(19, "ERROR: '%s' is version '%s'; should be version '%i'\n", name, token->token, 4);
            return 0;
        }
    }
    else
    {
        Com_PrintError(19, "ERROR: '%s' is missing 'iwmap' version specification\n", name);
        return 0;
    }
}

char __cdecl IsBoxInBrush(const float *boxCenter, const float *boxHalfLengths, const BrushWrapper *brush)
{
    float v4; // [esp+0h] [ebp-30h]
    float v5; // [esp+4h] [ebp-2Ch]
    float v6; // [esp+8h] [ebp-28h]
    float v7; // [esp+Ch] [ebp-24h]
    float v8; // [esp+14h] [ebp-1Ch]
    float v9; // [esp+1Ch] [ebp-14h]
    cplane_s *plane; // [esp+20h] [ebp-10h]
    float dist; // [esp+24h] [ebp-Ch]
    float offset; // [esp+28h] [ebp-8h]
    unsigned int sideIndex; // [esp+2Ch] [ebp-4h]
    unsigned int sideIndexa; // [esp+2Ch] [ebp-4h]

    for (sideIndex = 0; sideIndex < 3; ++sideIndex)
    {
        if (brush->mins[sideIndex] >= boxCenter[sideIndex] + boxHalfLengths[sideIndex])
            return 0;
        if (brush->maxs[sideIndex] <= boxCenter[sideIndex] - boxHalfLengths[sideIndex])
            return 0;
    }
    for (sideIndexa = 0; sideIndexa < brush->numsides; ++sideIndexa)
    {
        plane = brush->sides[sideIndexa].plane;
        v9 = *boxHalfLengths * plane->normal[0];
        v6 = I_fabs(v9);
        v8 = boxHalfLengths[1] * plane->normal[1];
        v5 = I_fabs(v8);
        v7 = boxHalfLengths[2] * plane->normal[2];
        v4 = I_fabs(v7);
        offset = v6 + v5 + v4;
        dist = Vec3Dot(boxCenter, plane->normal) - plane->dist - offset;
        if (dist >= 0.0)
            return 0;
    }
    return 1;
}

char __cdecl IsBoxInBox(const float *boxCenter, const float *boxHalfLengths, const PhysGeomInfo *geom)
{
    float v4; // [esp+0h] [ebp-44h]
    float v5; // [esp+4h] [ebp-40h]
    float v6; // [esp+8h] [ebp-3Ch]
    float v7; // [esp+Ch] [ebp-38h]
    float v8; // [esp+10h] [ebp-34h]
    float v9; // [esp+18h] [ebp-2Ch]
    float v10; // [esp+20h] [ebp-24h]
    float v11; // [esp+28h] [ebp-1Ch]
    float dist; // [esp+2Ch] [ebp-18h]
    float orientedBoxDist; // [esp+34h] [ebp-10h]
    float offset; // [esp+38h] [ebp-Ch]
    unsigned int sideIndex; // [esp+3Ch] [ebp-8h]
    float alignedBoxDist; // [esp+40h] [ebp-4h]

    if (geom->type != 1)
        MyAssertHandler(".\\xanim\\xmodel_load_phys_collmap.cpp", 594, 0, "%s", "geom->type == PHYS_GEOM_BOX");
    for (sideIndex = 0; sideIndex < 3; ++sideIndex)
    {
        v11 = *boxHalfLengths * geom->orientation[sideIndex][0];
        v8 = I_fabs(v11);
        v10 = boxHalfLengths[1] * geom->orientation[sideIndex][1];
        v7 = I_fabs(v10);
        v9 = boxHalfLengths[2] * geom->orientation[sideIndex][2];
        v6 = I_fabs(v9);
        offset = v8 + v7 + v6;
        orientedBoxDist = Vec3Dot(geom->orientation[sideIndex], geom->offset);
        alignedBoxDist = Vec3Dot(geom->orientation[sideIndex], boxCenter);
        v5 = alignedBoxDist - orientedBoxDist;
        v4 = I_fabs(v5);
        dist = v4 - geom->halfLengths[sideIndex] - offset;
        if (dist >= 0.0)
            return 0;
    }
    return 1;
}

char __cdecl IsBoxInCylinder(const float *boxCenter, const float *boxHalfLengths, const PhysGeomInfo *geom)
{
    float v4; // [esp+0h] [ebp-38h]
    float v5; // [esp+4h] [ebp-34h]
    float v6; // [esp+8h] [ebp-30h]
    float v7; // [esp+Ch] [ebp-2Ch]
    float v8; // [esp+10h] [ebp-28h]
    float v9; // [esp+14h] [ebp-24h]
    float v10; // [esp+1Ch] [ebp-1Ch]
    float dist; // [esp+28h] [ebp-10h]
    unsigned int axis; // [esp+2Ch] [ebp-Ch]
    float cylOffset; // [esp+30h] [ebp-8h]

    if (geom->type != 4)
        MyAssertHandler(".\\xanim\\xmodel_load_phys_collmap.cpp", 623, 0, "%s", "geom->type == PHYS_GEOM_CYLINDER");
    for (axis = 0; axis < 3; ++axis)
    {
        v9 = I_fabs(geom->orientation[0][axis]);
        v8 = 1.0 - v9;
        if (v8 < 0.0)
            v7 = 1.0;
        else
            v7 = v9;
        v6 = 1.0 - v7 * v7;
        v5 = sqrt(v6);
        cylOffset = v7 * geom->halfLengths[0] + v5 * geom->halfLengths[1];
        v10 = geom->offset[axis] - boxCenter[axis];
        v4 = I_fabs(v10);
        dist = v4 - boxHalfLengths[axis] - cylOffset;
        if (dist >= 0.0)
            return 0;
    }
    return 1;
}

bool __cdecl IsBoxInGeom(const float *boxCenter, const float *boxHalfLengths, const PhysGeomList *geomList)
{
    int type; // [esp+0h] [ebp-Ch]
    PhysGeomInfo *geomIter; // [esp+4h] [ebp-8h]
    unsigned int geomIndex; // [esp+8h] [ebp-4h]

    geomIndex = 0;
    geomIter = geomList->geoms;
    while (geomIndex < geomList->count)
    {
        if (geomIter->brush)
        {
            if (IsBoxInBrush(boxCenter, boxHalfLengths, geomIter->brush))
                return 1;
        }
        else
        {
            type = geomIter->type;
            if (type == 1)
            {
                if (IsBoxInBox(boxCenter, boxHalfLengths, geomIter))
                    return 1;
            }
            else if (type == 4)
            {
                if (IsBoxInCylinder(boxCenter, boxHalfLengths, geomIter))
                    return 1;
            }
            else if (!alwaysfails)
            {
                MyAssertHandler(".\\xanim\\xmodel_load_phys_collmap.cpp", 674, 0, "unhandled geom type");
            }
        }
        ++geomIndex;
        ++geomIter;
    }
    return 0;
}

char __cdecl Xmodel_ParsePhysicsCylinder(const char **file, PhysGeomInfo *geom)
{
    float angles[3]; // [esp+0h] [ebp-1Ch] BYREF
    const char *token; // [esp+Ch] [ebp-10h]
    float axis[3]; // [esp+10h] [ebp-Ch] BYREF

    if (!geom)
        MyAssertHandler(".\\xanim\\xmodel_load_phys_collmap.cpp", 484, 0, "%s", "geom");
    token = Com_Parse(file)->token;
    if (*token == 123)
    {
        geom->type = 4;
        axis[0] = Com_ParseFloat(file);
        axis[1] = Com_ParseFloatOnLine(file);
        axis[2] = Com_ParseFloatOnLine(file);
        geom->offset[0] = Com_ParseFloatOnLine(file);
        geom->offset[1] = Com_ParseFloatOnLine(file);
        geom->offset[2] = Com_ParseFloatOnLine(file);
        geom->halfLengths[0] = Com_ParseFloatOnLine(file) * 0.5;
        geom->halfLengths[1] = Com_ParseFloatOnLine(file);
        if (Com_MatchToken(file, "}", 0))
        {
            vectoangles(axis, angles);
            AnglesToAxis(angles, geom->orientation);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_PrintError(19, "Expecting '{' while parsing physics cylinder");
        return 0;
    }
}

bool __cdecl Xmodel_ParsePhysicsBox(const char **file, PhysGeomInfo *geom)
{
    if (!geom)
        MyAssertHandler(".\\xanim\\xmodel_load_phys_collmap.cpp", 445, 0, "%s", "geom");
    if (Com_Parse(file)->token[0] == 123)
    {
        geom->type = 1;
        geom->orientation[0][0] = Com_ParseFloat(file);
        geom->orientation[0][1] = Com_ParseFloatOnLine(file);
        geom->orientation[0][2] = Com_ParseFloatOnLine(file);
        geom->orientation[1][0] = Com_ParseFloat(file);
        geom->orientation[1][1] = Com_ParseFloatOnLine(file);
        geom->orientation[1][2] = Com_ParseFloatOnLine(file);
        geom->orientation[2][0] = Com_ParseFloat(file);
        geom->orientation[2][1] = Com_ParseFloatOnLine(file);
        geom->orientation[2][2] = Com_ParseFloatOnLine(file);
        geom->offset[0] = Com_ParseFloatOnLine(file);
        geom->offset[1] = Com_ParseFloatOnLine(file);
        geom->offset[2] = Com_ParseFloatOnLine(file);
        geom->halfLengths[0] = Com_ParseFloatOnLine(file);
        geom->halfLengths[1] = Com_ParseFloatOnLine(file);
        geom->halfLengths[2] = Com_ParseFloatOnLine(file);
        return Com_MatchToken(file, "}", 0) != 0;
    }
    else
    {
        Com_PrintError(19, "Expecting '{' while parsing physics cylinder");
        return 0;
    }
}

char __cdecl AddBrushBevels(
    float (*planes)[4],
    adjacencyWinding_t *windings,
    unsigned int *sideCount,
    unsigned int maxCount,
    const float *mins,
    const float *maxs,
    const char *mapname,
    unsigned int brushnum)
{
    _DWORD *v9; // ecx
    float *v10; // [esp+Ch] [ebp-60h]
    float *v11; // [esp+10h] [ebp-5Ch]
    unsigned int order; // [esp+18h] [ebp-54h]
    float planetemp; // [esp+1Ch] [ebp-50h]
    float planetemp_4; // [esp+20h] [ebp-4Ch]
    float planetemp_8; // [esp+24h] [ebp-48h]
    float planetemp_12; // [esp+28h] [ebp-44h]
    int dir; // [esp+2Ch] [ebp-40h]
    unsigned int i; // [esp+30h] [ebp-3Ch]
    unsigned int axis; // [esp+34h] [ebp-38h]
    adjacencyWinding_t tempWinding; // [esp+38h] [ebp-34h] BYREF

    order = 0;
    for (axis = 0; axis < 3; ++axis)
    {
        for (dir = -1; dir <= 1; dir += 2)
        {
            for (i = 0; i < *sideCount && (*planes)[4 * i + axis] != (double)dir; ++i)
                ;
            if (i == *sideCount)
            {
                if (*sideCount == maxCount)
                {
                    Com_PrintError(19, "ERROR: MAX_BUILD_SIDES for physics collmap %s, Brush %i\n", mapname, brushnum);
                    return 0;
                }
                v9 = (uint32*)&(*planes)[4 * *sideCount];
                *v9 = 0;
                v9[1] = 0;
                v9[2] = 0;
                v9[3] = 0;
                (*planes)[4 * *sideCount + axis] = (float)dir;
                if (dir == 1)
                    (*planes)[4 * *sideCount + 3] = maxs[axis];
                else
                    (*planes)[4 * *sideCount + 3] = -mins[axis];
                windings[(*sideCount)++].numsides = 0;
            }
            if (i != order)
            {
                planetemp = (*planes)[4 * order];
                planetemp_4 = (*planes)[4 * order + 1];
                planetemp_8 = (*planes)[4 * order + 2];
                planetemp_12 = (*planes)[4 * order + 3];
                v10 = &(*planes)[4 * order];
                v11 = &(*planes)[4 * i];
                *v10 = *v11;
                v10[1] = v11[1];
                v10[2] = v11[2];
                v10[3] = v11[3];
                *v11 = planetemp;
                v11[1] = planetemp_4;
                v11[2] = planetemp_8;
                v11[3] = planetemp_12;
                qmemcpy(&tempWinding, &windings[order], sizeof(tempWinding));
                qmemcpy(&windings[order], &windings[i], sizeof(adjacencyWinding_t));
                qmemcpy(&windings[i], &tempWinding, sizeof(adjacencyWinding_t));
            }
            ++order;
        }
    }
    return 1;
}

void __cdecl Map_SkipOptionalArg(const char **file, const char *argName)
{
    if (!strcmp(Com_Parse(file)->token, argName))
        Com_ParseOnLine(file);
    else
        Com_UngetToken();
}

char __cdecl Map_SkipNamedFlags(const char **file, const char *key)
{
    parseInfo_t *token; // [esp+14h] [ebp-4h]

    if (!strcmp(Com_Parse(file)->token, key))
    {
        do
        {
            token = Com_ParseOnLine(file);
            if (!token->token[0])
            {
                Com_PrintError(19, "missing token for '%s'", key);
                return 0;
            }
        } while (token->token[0] != 59);
        return 1;
    }
    else
    {
        Com_UngetToken();
        return 1;
    }
}

char __cdecl SnapAxialVector(float *normal)
{
    float v2; // [esp+0h] [ebp-14h]
    float v3; // [esp+4h] [ebp-10h]
    float v4; // [esp+8h] [ebp-Ch]
    float v5; // [esp+Ch] [ebp-8h]
    int axis; // [esp+10h] [ebp-4h]

    for (axis = 0; axis < 3; ++axis)
    {
        v5 = normal[axis] - 1.0;
        v3 = I_fabs(v5);
        if (v3 < 0.0000001)
        {
            *normal = 0.0;
            normal[1] = 0.0;
            normal[2] = 0.0;
            normal[axis] = 1.0;
            return 1;
        }
        v4 = normal[axis] - -1.0;
        v2 = I_fabs(v4);
        if (v2 < 0.0000001)
        {
            *normal = 0.0;
            normal[1] = 0.0;
            normal[2] = 0.0;
            normal[axis] = -1.0;
            return 1;
        }
    }
    return 0;
}

void __cdecl SnapPlane(float *plane)
{
    float v1; // [esp+4h] [ebp-8h]
    float v2; // [esp+8h] [ebp-4h]

    SnapAxialVector(plane);
    v2 = plane[3] - Q_rint(plane[3]);
    v1 = I_fabs(v2);
    if (v1 < 0.001)
        plane[3] = Q_rint(plane[3]);
}

bool __cdecl PlaneEqual(const float *p1, const float *p2)
{
    float v3; // [esp+8h] [ebp-8h]
    float v4; // [esp+Ch] [ebp-4h]

    if (!VecNCompareCustomEpsilon(p1, p2, 0.001f, 3))
        return 0;
    v4 = p1[3] - p2[3];
    v3 = I_fabs(v4);
    return v3 < 0.001;
}

char __cdecl RemoveDuplicateBrushPlanes(
    float (*planes)[4],
    unsigned int *sideCount,
    const char *mapname,
    unsigned int brushnum)
{
    float *v5; // [esp+Ch] [ebp-2Ch]
    float *v6; // [esp+10h] [ebp-28h]
    float *v7; // [esp+14h] [ebp-24h]
    float *v8; // [esp+18h] [ebp-20h]
    unsigned int sideIndexK; // [esp+20h] [ebp-18h]
    unsigned int sideIndexKa; // [esp+20h] [ebp-18h]
    float negatedNormal[3]; // [esp+24h] [ebp-14h] BYREF
    unsigned int sideIndexJ; // [esp+30h] [ebp-8h]
    unsigned int sideIndexI; // [esp+34h] [ebp-4h]

    for (sideIndexI = 1; sideIndexI < *sideCount; ++sideIndexI)
    {
        if (Vec3IsNormalized(&(*planes)[4 * sideIndexI]))
        {
            for (sideIndexJ = 0; sideIndexJ < sideIndexI; ++sideIndexJ)
            {
                if (PlaneEqual(&(*planes)[4 * sideIndexI], &(*planes)[4 * sideIndexJ]))
                {
                    for (sideIndexKa = sideIndexI + 1; sideIndexKa < *sideCount; ++sideIndexKa)
                    {
                        v5 = &(*planes)[4 * sideIndexKa - 4];
                        v6 = &(*planes)[4 * sideIndexKa];
                        *v5 = *v6;
                        v5[1] = v6[1];
                        v5[2] = v6[2];
                        v5[3] = v6[3];
                    }
                    --sideCount;
                    --sideIndexI;
                    break;
                }
                negatedNormal[0] = -(*planes)[4 * sideIndexJ];
                negatedNormal[1] = -(*planes)[4 * sideIndexJ + 1];
                negatedNormal[2] = -(*planes)[4 * sideIndexJ + 2];
                if (VecNCompareCustomEpsilon(&(*planes)[4 * sideIndexI], negatedNormal, 0.001f, 3)
                    && (*planes)[4 * sideIndexI + 3] <= -(*planes)[4 * sideIndexJ + 3])
                {
                    Com_PrintWarning(19, "Map %s, Brush %i: mirrored plane\n", mapname, brushnum);
                    return 0;
                }
            }
        }
        else
        {
            Com_PrintWarning(19, "Collmap for %s, Brush %i: degenerate plane\n", mapname, brushnum);
            for (sideIndexK = sideIndexI + 1; sideIndexK < *sideCount; ++sideIndexK)
            {
                v7 = &(*planes)[4 * sideIndexK - 4];
                v8 = &(*planes)[4 * sideIndexK];
                *v7 = *v8;
                v7[1] = v8[1];
                v7[2] = v8[2];
                v7[3] = v8[3];
            }
            --sideCount;
            --sideIndexI;
        }
    }
    return 1;
}

int __cdecl AddSimplePoint(
    const float (*planes)[4],
    unsigned int planeCount,
    const unsigned int *planeIndex,
    const float *xyz,
    SimplePlaneIntersection *pts,
    int ptCount,
    int maxPtCount)
{
    SimplePlaneIntersection *v8; // [esp+0h] [ebp-Ch]
    float dist; // [esp+4h] [ebp-8h]
    unsigned int sideIndex; // [esp+8h] [ebp-4h]

    for (sideIndex = 0; sideIndex < planeCount; ++sideIndex)
    {
        if (sideIndex != *planeIndex && sideIndex != planeIndex[1] && sideIndex != planeIndex[2])
        {
            dist = Vec3Dot(&(*planes)[4 * sideIndex], xyz) - (float)(*planes)[4 * sideIndex + 3];
            if (dist > 0.009999999776482582)
                return ptCount;
        }
    }
    if (ptCount < maxPtCount)
    {
        v8 = &pts[ptCount];
        v8->xyz[0] = *xyz;
        v8->xyz[1] = xyz[1];
        v8->xyz[2] = xyz[2];
        v8->planeIndex[0] = *planeIndex;
        v8->planeIndex[1] = planeIndex[1];
        v8->planeIndex[2] = planeIndex[2];
        return ptCount + 1;
    }
    else
    {
        Com_PrintError(
            19,
            "More than %i points from plane intersections on %i-sided brush.  Simplify the collision geometry.\n",
            ptCount,
            planeCount);
        return 0;
    }
}

unsigned int __cdecl GetPlaneIntersections(
    const float (*planes)[4],
    unsigned int planeCount,
    SimplePlaneIntersection *OutPts,
    unsigned int maxPtCount)
{
    const float *plane[3]; // [esp+8h] [ebp-34h] BYREF
    unsigned int ptCount; // [esp+14h] [ebp-28h]
    float xyz[3]; // [esp+18h] [ebp-24h] BYREF
    unsigned int sideIndex[3]; // [esp+24h] [ebp-18h]
    unsigned int planeIndex[3]; // [esp+30h] [ebp-Ch] BYREF

    ptCount = 0;
    for (sideIndex[0] = 0; sideIndex[0] < planeCount - 2; ++sideIndex[0])
    {
        planeIndex[0] = sideIndex[0];
        plane[0] = &(*planes)[4 * sideIndex[0]];
        for (sideIndex[1] = sideIndex[0] + 1; sideIndex[1] < planeCount - 1; ++sideIndex[1])
        {
            planeIndex[1] = sideIndex[1];
            plane[1] = &(*planes)[4 * sideIndex[1]];
            for (sideIndex[2] = sideIndex[1] + 1; sideIndex[2] < planeCount; ++sideIndex[2])
            {
                planeIndex[2] = sideIndex[2];
                plane[2] = &(*planes)[4 * sideIndex[2]];
                if (IntersectPlanes(plane, xyz))
                {
                    SnapPointToIntersectingPlanes(plane, xyz, 0.25f, 0.0099999998f);
                    ptCount = AddSimplePoint(planes, planeCount, planeIndex, xyz, OutPts, ptCount, maxPtCount);
                }
            }
        }
    }
    return ptCount;
}

char __cdecl Xmodel_ParsePhysicsBrush(
    char **file,
    char *mapname,
    unsigned int brushCount,
    PhysGeomInfo *geom,
    void *(__cdecl *Alloc)(int))
{
    cplane_s *v6; // [esp+18h] [ebp-68F8h]
    float *v7; // [esp+1Ch] [ebp-68F4h]
    float m[3]; // [esp+28h] [ebp-68E8h] BYREF
    float v1[3]; // [esp+34h] [ebp-68DCh] BYREF
    float v2[3]; // [esp+40h] [ebp-68D0h] BYREF
    int order; // [esp+4Ch] [ebp-68C4h]
    unsigned int sideCount; // [esp+50h] [ebp-68C0h] BYREF
    unsigned int dir; // [esp+54h] [ebp-68BCh]
    float mins[3]; // [esp+58h] [ebp-68B8h] BYREF
    int edgeIndex; // [esp+64h] [ebp-68ACh]
    float plane[128]; // [esp+68h] [ebp-68A8h] BYREF
    SimplePlaneIntersection OutPts[1024]; // [esp+268h] [ebp-66A8h] BYREF
    int InPtCount; // [esp+6268h] [ebp-6A8h]
    unsigned int ptIndex; // [esp+626Ch] [ebp-6A4h]
    int sideIndex; // [esp+6270h] [ebp-6A0h]
    float maxs[3]; // [esp+6274h] [ebp-69Ch] BYREF
    adjacencyWinding_t windings[32]; // [esp+6280h] [ebp-690h] BYREF
    int v29; // [esp+6900h] [ebp-10h]
    parseInfo_t *v30; // [esp+6904h] [ebp-Ch]
    int totalEdges; // [esp+6908h] [ebp-8h]
    unsigned int axis; // [esp+690Ch] [ebp-4h]

    v29 = 32;

    iassert(geom);

    sideCount = 0;
    Map_SkipOptionalArg((const char**)file, "layer");

    if (!Map_SkipNamedFlags((const char**)file, "contents"))
        return 0;

    if (!Map_SkipNamedFlags((const char **)file, "toolFlags"))
        return 0;

    while (1)
    {
        v30 = Com_Parse(file);
        if (!v30->token[0])
            return 0;
        if (!strcmp(v30->token, "}"))
            break;

        Com_UngetToken();

        if (sideCount == 32)
        {
            Com_PrintError(19, "ERROR: MAX_BUILD_SIDES (%i) -- brush too many sides.  Simplify the collision geometry.", 32);
            return 0;
        }

        Com_Parse1DMatrix((const char**)file, 3, m);
        Com_Parse1DMatrix((const char**)file, 3, v1);
        Com_Parse1DMatrix((const char**)file, 3, v2);
        PlaneFromPoints(&plane[4 * sideCount], m, v1, v2);
        SnapPlane(&plane[4 * sideCount++]);
        Com_SkipRestOfLine((const char**)file);
    }

    if (!RemoveDuplicateBrushPlanes((float(*)[4])plane, &sideCount, mapname, brushCount))
        return 0;

    if (!sideCount)
        return 0;

    InPtCount = GetPlaneIntersections((const float(*)[4])plane, sideCount, OutPts, 0x400u);
    totalEdges = 0;
    mins[0] = OutPts[0].xyz[0];
    mins[1] = OutPts[0].xyz[1];
    mins[2] = OutPts[0].xyz[2];
    maxs[0] = OutPts[0].xyz[0];
    maxs[1] = OutPts[0].xyz[1];
    maxs[2] = OutPts[0].xyz[2];

    for (ptIndex = 1; ptIndex < InPtCount; ++ptIndex)
    {
        for (axis = 0; axis < 3; ++axis)
        {
            if (mins[axis] <= (double)OutPts[ptIndex].xyz[axis])
            {
                if (maxs[axis] < (double)OutPts[ptIndex].xyz[axis])
                    maxs[axis] = OutPts[ptIndex].xyz[axis];
            }
            else
            {
                mins[axis] = OutPts[ptIndex].xyz[axis];
            }
        }
    }

    if (!AddBrushBevels((float(*)[4])plane, windings, &sideCount, 0x20u, mins, maxs, mapname, brushCount))
        return 0;

    InPtCount = GetPlaneIntersections((const float(*)[4])plane, sideCount, OutPts, 0x400u);
    totalEdges = 0;
    for (sideIndex = 0; sideIndex < sideCount; ++sideIndex)
    {
        if (BuildBrushdAdjacencyWindingForSide(
            &plane[4 * sideIndex],
            sideIndex,
            OutPts,
            InPtCount,
            &windings[sideIndex],
            12))
        {
            iassert(windings[sideIndex].numsides > 0);
            totalEdges += windings[sideIndex].numsides;
        }
        else
        {
            windings[sideIndex].numsides = 0;
        }
    }

    if (!totalEdges)
        return 0;

    geom->brush = (BrushWrapper*)Alloc(80);
    memset(geom->brush, 0, sizeof(BrushWrapper));

    geom->brush->mins[0] = mins[0];
    geom->brush->mins[1] = mins[1];
    geom->brush->mins[2] = mins[2];

    geom->brush->maxs[0] = maxs[0];
    geom->brush->maxs[1] = maxs[1];
    geom->brush->maxs[2] = maxs[2];

    geom->brush->totalEdgeCount = totalEdges;
    geom->brush->baseAdjacentSide = (unsigned char*)Alloc(totalEdges);

    edgeIndex = 0;
    order = 0;
    for (axis = 0; axis < 3; ++axis)
    {
        dir = 0;
        while (dir < 2)
        {
            geom->brush->edgeCount[dir][axis] = windings[order].numsides;
            iassert(geom->brush->edgeCount[dir][axis] == windings[order].numsides);
            geom->brush->firstAdjacentSideOffsets[dir][axis] = edgeIndex;
            for (ptIndex = 0; ptIndex < windings[order].numsides; ++ptIndex)
            {
                geom->brush->baseAdjacentSide[ptIndex + edgeIndex] = windings[order].sides[ptIndex];
                iassert(geom->brush->baseAdjacentSide[edgeIndex + ptIndex] == windings[order].sides[ptIndex]);
            }
            edgeIndex += windings[order].numsides;
            ++dir;
            ++order;
        }
    }
    geom->brush->numsides = sideCount - 6;
    if (geom->brush->numsides)
    {
        geom->brush->sides = (cbrushside_t*)Alloc(12 * geom->brush->numsides);
        geom->brush->planes = (cplane_s*)Alloc(20 * geom->brush->numsides);
        sideIndex = 0;
        while (sideIndex < geom->brush->numsides)
        {
            iassert(order == sideIndex + 6);
            geom->brush->sides[sideIndex].edgeCount = windings[order].numsides;
            iassert(geom->brush->sides[sideIndex].edgeCount == windings[order].numsides);
            geom->brush->sides[sideIndex].firstAdjacentSideOffset = edgeIndex;
            for (ptIndex = 0; ptIndex < windings[order].numsides; ++ptIndex)
            {
                geom->brush->baseAdjacentSide[ptIndex + edgeIndex] = windings[order].sides[ptIndex];
                iassert(geom->brush->baseAdjacentSide[edgeIndex + ptIndex] == windings[order].sides[ptIndex]);
            }
            edgeIndex += windings[order].numsides;
            v6 = &geom->brush->planes[sideIndex];
            v7 = &plane[4 * order];
            v6->normal[0] = *v7;
            v6->normal[1] = v7[1];
            v6->normal[2] = v7[2];
            geom->brush->planes[sideIndex].dist = plane[4 * order + 3];
            geom->brush->sides[sideIndex].plane = &geom->brush->planes[sideIndex];
            ++sideIndex;
            ++order;
        }
    }

    iassert(edgeIndex == totalEdges);

    return 1;
}

void __cdecl GetGeomAABB(const PhysGeomInfo *geom, float *mins, float *maxs)
{
    float v3; // [esp+0h] [ebp-40h]
    float v4; // [esp+4h] [ebp-3Ch]
    float v5; // [esp+8h] [ebp-38h]
    int type; // [esp+Ch] [ebp-34h]
    float v7; // [esp+10h] [ebp-30h]
    float v8; // [esp+18h] [ebp-28h]
    float v9; // [esp+20h] [ebp-20h]
    float *v10; // [esp+24h] [ebp-1Ch]
    BrushWrapper *brush; // [esp+28h] [ebp-18h]
    float range; // [esp+2Ch] [ebp-14h]
    float range_4; // [esp+30h] [ebp-10h]
    float range_8; // [esp+34h] [ebp-Ch]
    unsigned int axisIndex; // [esp+38h] [ebp-8h]
    float axisRange; // [esp+3Ch] [ebp-4h]

    if (geom->brush)
    {
        brush = geom->brush;
        *mins = geom->brush->mins[0];
        mins[1] = brush->mins[1];
        mins[2] = brush->mins[2];
        v10 = geom->brush->maxs;
        *maxs = *v10;
        maxs[1] = v10[1];
        maxs[2] = v10[2];
    }
    else
    {
        range = 0.0;
        range_4 = 0.0;
        range_8 = 0.0;
        type = geom->type;
        if (type == 1)
        {
            range = geom->halfLengths[0];
            range_4 = geom->halfLengths[1];
            range_8 = geom->halfLengths[2];
        }
        else if (type == 4)
        {
            range = geom->halfLengths[0];
            range_4 = geom->halfLengths[1];
            range_8 = range_4;
        }
        else if (!alwaysfails)
        {
            MyAssertHandler(".\\xanim\\xmodel_load_phys_collmap.cpp", 713, 0, "unhandled geom type");
        }
        for (axisIndex = 0; axisIndex < 3; ++axisIndex)
        {
            v9 = range * geom->orientation[0][axisIndex];
            v5 = I_fabs(v9);
            v8 = range_4 * geom->orientation[1][axisIndex];
            v4 = I_fabs(v8);
            v7 = range_8 * geom->orientation[2][axisIndex];
            v3 = I_fabs(v7);
            axisRange = v5 + v4 + v3;
            mins[axisIndex] = geom->offset[axisIndex] - axisRange;
            maxs[axisIndex] = geom->offset[axisIndex] + axisRange;
        }
    }
}

PhysGeomList *__cdecl Xmodel_ParsePhysicsCollMap(
    const char **file,
    char *name,
    unsigned int geomCount,
    void *(__cdecl *Alloc)(int))
{
    int CurrentParseLine; // eax
    float v6; // [esp+0h] [ebp-120h]
    float v7; // [esp+4h] [ebp-11Ch]
    float v8; // [esp+8h] [ebp-118h]
    float v9; // [esp+Ch] [ebp-114h]
    float v10; // [esp+10h] [ebp-110h]
    float v11; // [esp+14h] [ebp-10Ch]
    float v12; // [esp+18h] [ebp-108h]
    float v13; // [esp+1Ch] [ebp-104h]
    float v14; // [esp+20h] [ebp-100h]
    float v15; // [esp+24h] [ebp-FCh]
    float v16; // [esp+28h] [ebp-F8h]
    float v17; // [esp+2Ch] [ebp-F4h]
    float mins[3]; // [esp+E4h] [ebp-3Ch] BYREF
    float geomMaxs[3]; // [esp+F0h] [ebp-30h] BYREF
    float maxs[3]; // [esp+FCh] [ebp-24h] BYREF
    PhysGeomList *geomList; // [esp+108h] [ebp-18h]
    float geomMins[3]; // [esp+10Ch] [ebp-14h] BYREF
    const char *token; // [esp+118h] [ebp-8h]
    unsigned int geomIndex; // [esp+11Ch] [ebp-4h]

    if (!geomCount)
        MyAssertHandler(".\\xanim\\xmodel_load_phys_collmap.cpp", 840, 0, "%s", "geomCount > 0");
    if (!strcmp(Com_Parse(file)->token, "iwmap"))
    {
        token = Com_Parse(file)->token;
        if (atoi(token) == 4)
        {
            while (1)
            {
                token = Com_Parse(file)->token;
                if (!*token)
                    return 0;
                if (!strcmp(token, "{"))
                    break;
                Com_SkipRestOfLine(file);
            }
            if (!strcmp(token, "{"))
            {
                geomList = (PhysGeomList*)Alloc(44);
                memset(geomList, 0, sizeof(PhysGeomList));
                geomList->count = geomCount;
                geomList->geoms = (PhysGeomInfo*)Alloc(68 * geomCount);
                memset(geomList->geoms, 0, 68 * geomCount);
                geomIndex = 0;
                while (geomIndex < geomCount)
                {
                    token = Com_Parse(file)->token;
                    if (!*token)
                    {
                        Com_PrintError(19, "ERROR: ParsePhysicsCollMap: EOF without closing brace");
                        return 0;
                    }
                    if (!strcmp(token, "}"))
                        break;
                    if (!strcmp(token, "{"))
                    {
                        token = Com_Parse(file)->token;
                        if (!*token)
                            break;
                        if (!strcmp(token, "curve"))
                        {
                            Com_PrintError(19, "ERROR: cannot have curves in collision maps");
                            return 0;
                        }
                        if (!strcmp(token, "mesh"))
                        {
                            Com_PrintError(19, "ERROR: cannot have patch terrain in collision maps");
                            return 0;
                        }
                        if (!strcmp(token, "physics_cylinder"))
                        {
                            if (!Xmodel_ParsePhysicsCylinder(file, &geomList->geoms[geomIndex]))
                                return 0;
                            if (!Com_MatchToken(file, "}", 0))
                                return 0;
                            ++geomIndex;
                        }
                        else if (!strcmp(token, "physics_box"))
                        {
                            if (!Xmodel_ParsePhysicsBox(file, &geomList->geoms[geomIndex]))
                                return 0;
                            if (!Com_MatchToken(file, "}", 0))
                                return 0;
                            ++geomIndex;
                        }
                        else
                        {
                            Com_UngetToken();
                            if (!Xmodel_ParsePhysicsBrush((char**)file, name, geomIndex, &geomList->geoms[geomIndex], Alloc))
                                return 0;
                            if (!geomList->geoms[geomIndex].brush)
                                MyAssertHandler(
                                    ".\\xanim\\xmodel_load_phys_collmap.cpp",
                                    930,
                                    1,
                                    "%s",
                                    "geomList->geoms[geomIndex].brush");
                            ++geomIndex;
                        }
                    }
                    else if (!SkipEpair((char*)token, file))
                    {
                        return 0;
                    }
                }
                if (geomIndex != geomCount)
                    MyAssertHandler(
                        ".\\xanim\\xmodel_load_phys_collmap.cpp",
                        943,
                        0,
                        "geomIndex == geomCount\n\t%i, %i",
                        geomIndex,
                        geomCount);
                mins[0] = FLT_MAX;
                mins[1] = FLT_MAX;
                mins[2] = FLT_MAX;
                maxs[0] = -FLT_MAX;
                maxs[1] = -FLT_MAX;
                maxs[2] = -FLT_MAX;
                for (geomIndex = 0; geomIndex < geomCount; ++geomIndex)
                {
                    GetGeomAABB(&geomList->geoms[geomIndex], geomMins, geomMaxs);
                    v17 = maxs[0] - geomMaxs[0];
                    if (v17 < 0.0)
                        v16 = geomMaxs[0];
                    else
                        v16 = maxs[0];
                    maxs[0] = v16;
                    v15 = maxs[1] - geomMaxs[1];
                    if (v15 < 0.0)
                        v14 = geomMaxs[1];
                    else
                        v14 = maxs[1];
                    maxs[1] = v14;
                    v13 = maxs[2] - geomMaxs[2];
                    if (v13 < 0.0)
                        v12 = geomMaxs[2];
                    else
                        v12 = maxs[2];
                    maxs[2] = v12;
                    v11 = geomMins[0] - mins[0];
                    if (v11 < 0.0)
                        v10 = geomMins[0];
                    else
                        v10 = mins[0];
                    mins[0] = v10;
                    v9 = geomMins[1] - mins[1];
                    if (v9 < 0.0)
                        v8 = geomMins[1];
                    else
                        v8 = mins[1];
                    mins[1] = v8;
                    v7 = geomMins[2] - mins[2];
                    if (v7 < 0.0)
                        v6 = geomMins[2];
                    else
                        v6 = mins[2];
                    mins[2] = v6;
                }
                if (maxs[0] <= mins[0])
                    MyAssertHandler(".\\xanim\\xmodel_load_phys_collmap.cpp", 957, 0, "%s", "mins[0] < maxs[0]");
                if (maxs[1] <= mins[1])
                    MyAssertHandler(".\\xanim\\xmodel_load_phys_collmap.cpp", 958, 0, "%s", "mins[1] < maxs[1]");
                if (maxs[2] <= mins[2])
                    MyAssertHandler(".\\xanim\\xmodel_load_phys_collmap.cpp", 959, 0, "%s", "mins[2] < maxs[2]");
                Phys_ComputeMassProperties(
                    mins,
                    maxs,
                    IsBoxInGeom,
                    geomList,
                    geomList->mass.centerOfMass,
                    geomList->mass.momentsOfInertia,
                    geomList->mass.productsOfInertia);
                return geomList;
            }
            else
            {
                CurrentParseLine = Com_GetCurrentParseLine();
                Com_PrintError(19, "ERROR: ParsePhysicsCollMap: { not found, found %s on line %d", token, CurrentParseLine);
                return 0;
            }
        }
        else
        {
            Com_PrintError(19, "ERROR: '%s' is version '%s'; should be version '%i'\n", name, token, 4);
            return 0;
        }
    }
    else
    {
        Com_PrintError(19, "ERROR: '%s' is missing 'iwmap' version specification\n", name);
        return 0;
    }
}

PhysGeomList *__cdecl XModel_LoadPhysicsCollMap(const char *name, void *(__cdecl *Alloc)(int))
{
    char filename[1024]; // [esp+0h] [ebp-418h] BYREF
    const char *buf; // [esp+404h] [ebp-14h] BYREF
    unsigned __int8 *file; // [esp+408h] [ebp-10h] BYREF
    int fileSize; // [esp+40Ch] [ebp-Ch]
    unsigned int geomCount; // [esp+410h] [ebp-8h]
    PhysGeomList *geomList; // [esp+414h] [ebp-4h]

    if (Com_sprintf(filename, 0x400u, "phys_collmaps/%s.map", name) >= 0)
    {
        fileSize = FS_ReadFile(filename, (void**)&file);
        if (fileSize >= 0)
        {
            if (fileSize)
            {
                buf = (const char*)file;
                geomCount = Xmodel_CountPhysicsCollMapGeoms(&buf, filename);
                FS_FreeFile((char*)file);
                if (geomCount)
                {
                    fileSize = FS_ReadFile(filename, (void**)&file);
                    if (fileSize <= 0)
                        MyAssertHandler(".\\xanim\\xmodel_load_phys_collmap.cpp", 1004, 0, "%s", "fileSize > 0");
                    buf = (const char*)file;
                    geomList = Xmodel_ParsePhysicsCollMap(&buf, filename, geomCount, Alloc);
                    FS_FreeFile((char*)file);
                    return geomList;
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                Com_PrintError(19, "ERROR: physics collmap '%s' has 0 length\n", name);
                FS_FreeFile((char*)file);
                return 0;
            }
        }
        else
        {
            if (file)
                MyAssertHandler(".\\xanim\\xmodel_load_phys_collmap.cpp", 985, 0, "%s", "!file");
            return 0;
        }
    }
    else
    {
        Com_PrintError(19, "ERROR: filename '%s' too long\n", filename);
        return 0;
    }
}