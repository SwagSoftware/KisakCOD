#include "qcommon.h"
#include <xanim/xmodel.h>
#include <universal/com_memory.h>
#include "com_bsp.h"
#include <universal/q_parse.h>
#include <DynEntity/DynEntity_client.h>

void __cdecl CM_InitStaticModel(cStaticModel_s *staticModel, float *origin, float *angles, float scale)
{
    float v4; // [esp+10h] [ebp-48h]
    float axis[3][3]; // [esp+34h] [ebp-24h] BYREF

    if ((COERCE_UNSIGNED_INT(*origin) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(origin[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(origin[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\qcommon\\cm_staticmodel_load_obj.cpp",
            20,
            0,
            "%s",
            "!IS_NAN((origin)[0]) && !IS_NAN((origin)[1]) && !IS_NAN((origin)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*angles) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(angles[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(angles[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\qcommon\\cm_staticmodel_load_obj.cpp",
            21,
            0,
            "%s",
            "!IS_NAN((angles)[0]) && !IS_NAN((angles)[1]) && !IS_NAN((angles)[2])");
    }
    if ((LODWORD(scale) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\qcommon\\cm_staticmodel_load_obj.cpp", 22, 0, "%s", "!IS_NAN(scale)");
    staticModel->origin[0] = *origin;
    staticModel->origin[1] = origin[1];
    staticModel->origin[2] = origin[2];
    AnglesToAxis(angles, axis);
    MatrixTranspose(axis, staticModel->invScaledAxis);
    v4 = 1.0 / scale;
    Vec3Scale(staticModel->invScaledAxis[0], v4, staticModel->invScaledAxis[0]);
    Vec3Scale(staticModel->invScaledAxis[1], v4, staticModel->invScaledAxis[1]);
    Vec3Scale(staticModel->invScaledAxis[2], v4, staticModel->invScaledAxis[2]);
    Vec3Scale(axis[0], scale, axis[0]);
    Vec3Scale(axis[1], scale, axis[1]);
    Vec3Scale(axis[2], scale, axis[2]);
    if (XModelGetStaticBounds(staticModel->xmodel, axis, staticModel->absmin, staticModel->absmax))
    {
        Vec3Add(staticModel->absmin, origin, staticModel->absmin);
        Vec3Add(staticModel->absmax, origin, staticModel->absmax);
        if (staticModel->absmax[0] - staticModel->absmin[0] < 0.0)
            MyAssertHandler(
                ".\\qcommon\\cm_staticmodel_load_obj.cpp",
                45,
                0,
                "%s",
                "staticModel->absmax[0] - staticModel->absmin[0] >= 0");
        if (staticModel->absmax[1] - staticModel->absmin[1] < 0.0)
            MyAssertHandler(
                ".\\qcommon\\cm_staticmodel_load_obj.cpp",
                46,
                0,
                "%s",
                "staticModel->absmax[1] - staticModel->absmin[1] >= 0");
        if (staticModel->absmax[2] - staticModel->absmin[2] < 0.0)
            MyAssertHandler(
                ".\\qcommon\\cm_staticmodel_load_obj.cpp",
                47,
                0,
                "%s",
                "staticModel->absmax[2] - staticModel->absmin[2] >= 0");
    }
}

unsigned __int8 *__cdecl CM_Hunk_AllocXModel(unsigned int size)
{
    return Hunk_Alloc(size, "CM_Hunk_AllocXModel", 21);
}
unsigned __int8 *__cdecl CM_Hunk_AllocXModelColl(unsigned int size)
{
    return Hunk_Alloc(size, "CM_Hunk_AllocXModelColl", 27);
}

unsigned __int8 *__cdecl CM_Hunk_Alloc(unsigned int size, const char *name, int type)
{
    return Hunk_Alloc(size, name, type);
}

XModel *__cdecl CM_XModelPrecache(char *name)
{
    return XModelPrecache(
        name,
        (void *(__cdecl *)(int))CM_Hunk_AllocXModel,
        (void *(__cdecl *)(int))CM_Hunk_AllocXModelColl);
}

char __cdecl CM_CreateStaticModel(cStaticModel_s *staticModel, char *name, float *origin, float *angles, float scale)
{
    XModel *model; // [esp+34h] [ebp-4h]

    if ((COERCE_UNSIGNED_INT(*origin) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(origin[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(origin[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\qcommon\\cm_staticmodel_load_obj.cpp",
            60,
            0,
            "%s",
            "!IS_NAN((origin)[0]) && !IS_NAN((origin)[1]) && !IS_NAN((origin)[2])");
    }
    if ((COERCE_UNSIGNED_INT(*angles) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(angles[1]) & 0x7F800000) == 0x7F800000
        || (COERCE_UNSIGNED_INT(angles[2]) & 0x7F800000) == 0x7F800000)
    {
        MyAssertHandler(
            ".\\qcommon\\cm_staticmodel_load_obj.cpp",
            61,
            0,
            "%s",
            "!IS_NAN((angles)[0]) && !IS_NAN((angles)[1]) && !IS_NAN((angles)[2])");
    }
    if ((LODWORD(scale) & 0x7F800000) == 0x7F800000)
        MyAssertHandler(".\\qcommon\\cm_staticmodel_load_obj.cpp", 62, 0, "%s", "!IS_NAN(scale)");
    if (!name || !*name)
        Com_Error(ERR_DROP, "Missing model name at %.0f %.0f %.0f", *origin, origin[1], origin[2]);
    if (scale == 0.0)
        Com_Error(ERR_DROP, "Static model [%s] has scale of 0.0", name);
    model = CM_XModelPrecache(name);
    if (!model)
        return 0;
    staticModel->xmodel = model;
    CM_InitStaticModel(staticModel, origin, angles, scale);
    return 1;
}

void __cdecl CM_LoadStaticModels()
{
    char v0; // [esp+7h] [ebp-115h]
    char *v1; // [esp+Ch] [ebp-110h]
    char *v2; // [esp+10h] [ebp-10Ch]
    char v3; // [esp+17h] [ebp-105h]
    char *v4; // [esp+1Ch] [ebp-100h]
    char *v5; // [esp+20h] [ebp-FCh]
    unsigned int numStaticModels; // [esp+24h] [ebp-F8h]
    float origin[3]; // [esp+28h] [ebp-F4h] BYREF
    char modelName[64]; // [esp+34h] [ebp-E8h] BYREF
    float angles[3]; // [esp+74h] [ebp-A8h] BYREF
    int bMiscModel; // [esp+80h] [ebp-9Ch]
    char key[68]; // [esp+84h] [ebp-98h] BYREF
    const char *ptr; // [esp+C8h] [ebp-54h] BYREF
    const char *token; // [esp+CCh] [ebp-50h]
    float scale; // [esp+D0h] [ebp-4Ch]
    char value[68]; // [esp+D4h] [ebp-48h] BYREF

    ptr = Com_EntityString(0);
    if (!ptr)
        MyAssertHandler(".\\qcommon\\cm_staticmodel_load_obj.cpp", 100, 0, "%s", "ptr");
    cm.numStaticModels = 0;
    cm.staticModelList = 0;
    numStaticModels = 0;
    while (1)
    {
        token = (const char *)Com_Parse(&ptr);
        if (!ptr || *token != '{')
            break;
        while (1)
        {
            token = (const char *)Com_Parse(&ptr);
            if (!ptr)
                break;
            if (*token == 125)
                break;
            I_strncpyz(key, (char *)token, 64);
            token = (const char *)Com_Parse(&ptr);
            if (!ptr)
                break;
            I_strncpyz(value, (char *)token, 64);
            if (!I_stricmp(key, "classname") && !I_stricmp(value, "misc_model"))
                ++numStaticModels;
        }
    }
    if (numStaticModels)
    {
        cm.staticModelList = (cStaticModel_s *)CM_Hunk_Alloc(80 * numStaticModels, "CM_CreateStaticModel", 27);
        ptr = Com_EntityString(0);
        if (!ptr)
            MyAssertHandler(".\\qcommon\\cm_staticmodel_load_obj.cpp", 146, 0, "%s", "ptr");
        ProfLoad_Begin("Create static model collision");
        while (1)
        {
            token = (const char *)Com_Parse(&ptr);
            if (!ptr || *token != 123)
                break;
            modelName[0] = 0;
            origin[2] = 0.0;
            origin[1] = 0.0;
            origin[0] = 0.0;
            angles[2] = 0.0;
            angles[1] = 0.0;
            angles[0] = 0.0;
            scale = 1.0;
            bMiscModel = 0;
            while (1)
            {
                token = (const char *)Com_Parse(&ptr);
                if (!ptr)
                    break;
                if (*token == 125)
                    break;
                I_strncpyz(key, (char *)token, 64);
                token = (const char *)Com_Parse(&ptr);
                if (!ptr)
                    break;
                I_strncpyz(value, (char *)token, 64);
                if (I_stricmp(key, "classname"))
                {
                    if (I_stricmp(key, "model"))
                    {
                        if (I_stricmp(key, "origin"))
                        {
                            if (I_stricmp(key, "angles"))
                            {
                                if (!I_stricmp(key, "modelscale"))
                                    scale = atof(value);
                            }
                            else
                            {
                                sscanf(value, "%f %f %f", angles, &angles[1], &angles[2]);
                            }
                        }
                        else
                        {
                            sscanf(value, "%f %f %f", origin, &origin[1], &origin[2]);
                        }
                    }
                    else if (Com_IsLegacyXModelName(value))
                    {
                        v5 = &value[7];
                        v4 = modelName;
                        do
                        {
                            v3 = *v5;
                            *v4++ = *v5++;
                        } while (v3);
                    }
                    else
                    {
                        v2 = value;
                        v1 = modelName;
                        do
                        {
                            v0 = *v2;
                            *v1++ = *v2++;
                        } while (v0);
                    }
                }
                else if (!I_stricmp(value, "misc_model"))
                {
                    bMiscModel = 1;
                }
            }
            if (bMiscModel)
            {
                if (cm.numStaticModels >= numStaticModels)
                    MyAssertHandler(
                        ".\\qcommon\\cm_staticmodel_load_obj.cpp",
                        207,
                        0,
                        "%s",
                        "cm.numStaticModels < numStaticModels");
                if (CM_CreateStaticModel(&cm.staticModelList[cm.numStaticModels], modelName, origin, angles, scale))
                    ++cm.numStaticModels;
            }
        }
        ProfLoad_End();
    }
}

void __cdecl CM_LoadMapData_LoadObj(const char *name)
{
    if (!cm.isInUse || I_stricmp(cm.name, name))
    {
        ProfLoad_Begin("Load bsp collision");
        CM_LoadMapFromBsp(name, 1);
        ProfLoad_End();
        ProfLoad_Begin("Load static model collision");
        CM_LoadStaticModels();
        ProfLoad_End();
        ProfLoad_Begin("Load dynamic entities");
        DynEnt_LoadEntities();
        ProfLoad_End();
    }
}

cplane_s *__cdecl CM_GetPlanes()
{
    if (!cm.planes)
        MyAssertHandler(".\\qcommon\\cm_load_obj.cpp", 1412, 0, "%s", "cm.planes");
    return cm.planes;
}

int __cdecl CM_GetPlaneCount()
{
    return cm.planeCount;
}