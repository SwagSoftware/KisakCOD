#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_modelpreviewer.h"
#include <game/g_local.h>
#include <qcommon/cmd.h>
#include "cg_main.h"
#include <universal/com_files.h>
#include <xanim/dobj_utils.h>
#include "cg_ents.h"
#include <gfx_d3d/r_model.h>
#include <devgui/devgui.h>
#include <ragdoll/ragdoll.h>
#include <database/database.h>

#include <algorithm>

ModelPreviewer g_mdlprv;

const dvar_t *modPrvModelMru;
const dvar_t *modPrvLoadModel;
const dvar_t *modPrvOrigin;
const dvar_t *modPrvRotationAngles;
const dvar_t *modPrvCenterOffset;
const dvar_t *modPrvLod;
const dvar_t *modPrvDrawAxis;
const dvar_t *modPrvDrawBoneInfo;
const dvar_t *modPrvFromAnimMru;
const dvar_t *modPrvToAnimMru;
const dvar_t *modPrvLoadFromAnim;
const dvar_t *modPrvLoadToAnim;
const dvar_t *modPrvAnimForceLoop;
const dvar_t *modPrvAnimRate;
const dvar_t *modPrvAnimBlendMode;
const dvar_t *modPrvAnimApplyDelta;
const dvar_t *modPrvAnimCrossBlendTime;
const dvar_t *modPrvAnimCrossBlendDuration;
const dvar_t *modPrvAnimBlendWeight;
const dvar_t *modPrvMatSelect;
const dvar_t *modPrvMatReplace;
const dvar_t *modPrvLightSetup;
const dvar_t *modPrvSunDirection;
const dvar_t *modPrvSunColor;
const dvar_t *modPrvSunLight;
const dvar_t *modPrvSunDiffuseColor;
const dvar_t *modPrvSunDiffuseFraction;
const dvar_t *modPrvAmbientColor;
const dvar_t *modPrvAmbientScale;
const dvar_t *modPrvDisplayToggle;
const dvar_t *modPrvGamepadControlSpeed;
const dvar_t *gpad_button_rstick_deflect_max;
const dvar_t *modPrvHideModel;
const dvar_t *modPrvDrawDistanceToModel;

HunkUser *g_modPrvHunkUser;

cmd_function_s CG_ModPrvResetOrientation_f_VAR;
cmd_function_s CG_ModPrvExit_f_VAR;

static const char *mpDefaultDrawBoneOptions[4] = { "Draw None", "Draw All Tags", "Draw All Bones", "Draw All" };

void __cdecl CG_ModPrvUpdateMru(const dvar_s **mruDvars, const char **stringTable, const dvar_s *dvar)
{
    const dvar_s **v3; // r31
    int v6; // r30
    char *v7; // r27

    v3 = mruDvars;
    if (!mruDvars)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp", 345, 0, "%s", "mruDvars");
    if (!stringTable)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp", 346, 0, "%s", "stringTable");
    v6 = 0;
    v7 = (char *)((char *)stringTable - (char *)v3);
    do
    {
        if (!*v3)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp",
                350,
                0,
                "%s",
                "mruDvars[dvarIndex]");
        if (v6 && !*(_BYTE *)(*v3)->current.integer)
            break;
        ++v6;
        *(const dvar_s **)((char *)v3 + (unsigned int)v7) = (const dvar_s *)(*v3)->current.integer;
        ++v3;
    } while (v6 < 4);
    stringTable[v6] = 0;
    if (dvar)
        Dvar_UpdateEnumDomain((dvar_s*)(dvar_s*)dvar, stringTable);
}

void __cdecl CG_ModPrvPushMruEntry(
    const char *entry,
    const dvar_s **mruDvars,
    const char **stringTable,
    const dvar_s *dvar)
{
    const char *v6; // r11
    char *v10; // r11
    int v11; // r10
    int v12; // r31
    const dvar_s **v13; // r30
    int v14; // r30
    const dvar_s **v15; // r31
    char v16[304]; // [sp+50h] [-130h] BYREF

    v6 = entry;
    while (*(unsigned __int8 *)v6++)
        ;
    if (v6 - entry - 1 >= 256)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp",
            369,
            0,
            "%s\n\t(entry) = %s",
            "(entryNameLength < 256)",
            entry);
    v10 = (char*)entry;
    do
    {
        v11 = *(unsigned __int8 *)v10;
        (v10++)[v16 - entry] = v11;
    } while (v11);
    v12 = 0;
    v13 = mruDvars;
    do
    {
        if (!I_stricmp(v16, (*v13)->current.string))
            break;
        ++v12;
        ++v13;
    } while (v12 < 3);
    v14 = v12;
    if (v12)
    {
        v15 = &mruDvars[v12];
        do
        {
            Dvar_SetString(*v15, (*(v15 - 1))->current.string);
            --v14;
            --v15;
        } while (v14);
    }
    Dvar_SetString(*mruDvars, v16);
    CG_ModPrvUpdateMru(mruDvars, stringTable, dvar);
}

void __cdecl CG_ModPrvRemoveMruEntry(const dvar_s **mruDvars, const char **stringTable, const dvar_s *dvar)
{
    int v6; // r11
    int v7; // r30
    const dvar_s **v8; // r31

    iassert(mruDvars);
    iassert(stringTable);
    iassert(dvar);

    v6 = dvar->current.integer + 1;
    if (v6 < 4)
    {
        v7 = 4 - v6;
        v8 = &mruDvars[v6];
        do
        {
            Dvar_SetString(*(v8 - 1), (*v8)->current.string);
            --v7;
            ++v8;
        } while (v7);
    }
    Dvar_SetString(mruDvars[3], "");
    CG_ModPrvUpdateMru(mruDvars, stringTable, dvar);
}

static const char *modPrvLodNames[5] = { "high", "medium", "low", "lowest", NULL };
static const char *modPrvAnimBlendModeNames[3] = { "Sequential", "Synchronous", NULL };

void CG_ModPrvRegisterDvars()
{
    g_mdlprv.model.mruNames[0] = Dvar_RegisterString(
        "modPrvModelMruName0",
        "",
        1u,
        "Model previewer most recently used model name 0");
    g_mdlprv.model.mruNames[1] = Dvar_RegisterString(
        "modPrvModelMruName1",
        "",
        1u,
        "Model previewer most recently used model name 1");
    g_mdlprv.model.mruNames[2] = Dvar_RegisterString(
        "modPrvModelMruName2",
        "",
        1u,
        "Model previewer most recently used model name 2");
    g_mdlprv.model.mruNames[3] = Dvar_RegisterString(
        "modPrvModelMruName3",
        "",
        1u,
        "Model previewer most recently used model name 3");
    CG_ModPrvUpdateMru(g_mdlprv.model.mruNames, g_mdlprv.model.mruNameTable, 0);
    modPrvModelMru = Dvar_RegisterEnum(
        "modPrvModelMru",
        g_mdlprv.model.mruNameTable,
        0,
        0x840u,
        "Model previewer most recently used model");
    modPrvLoadModel = Dvar_RegisterEnum(
        "modPrvLoadModel",
        g_mdlprv.system.modelNames,
        0,
        0x800u,
        "Model previewer loaded model");
    Dvar_UpdateEnumDomain((dvar_s*)(dvar_s*)modPrvLoadModel, g_mdlprv.system.modelNames);
    modPrvOrigin = Dvar_RegisterVec3("modPrvOrigin", 0.0, 0.0, 0.0, -1000.0, 1000.0, 0, NULL);
    modPrvRotationAngles = Dvar_RegisterVec3("modPrvRotationAngles", 0.0, 0.0, 0.0, -180.0, 180.0, 0, NULL);
    modPrvCenterOffset = Dvar_RegisterVec3("modPrvCenterOffset", 0.0, 0.0, 0.0, -1000.0, 1000.0, 0, NULL);
    modPrvLod = Dvar_RegisterEnum("modPrvLod", modPrvLodNames, 0, 0, "Model previewer level of detail");
    modPrvDrawAxis = Dvar_RegisterBool("modPrvDrawAxis", 0, 0, "Draw the model previewer axes");
    modPrvDrawBoneInfo = Dvar_RegisterEnum(
        "modPrvDrawBoneInfo",
        g_mdlprv.model.boneNameTable,
        0,
        0,
        "Draw model previewer bone information");
    g_mdlprv.anim.mruNames[0] = Dvar_RegisterString(
        "modPrvAnimMruName0",
        "",
        1u,
        "Model previewer most recently used anim name 0");
    g_mdlprv.anim.mruNames[1] = Dvar_RegisterString(
        "modPrvAnimMruName1",
        "",
        1u,
        "Model previewer most recently used anim name 1");
    g_mdlprv.anim.mruNames[2] = Dvar_RegisterString(
        "modPrvAnimMruName2",
        "",
        1u,
        "Model previewer most recently used anim name 2");
    g_mdlprv.anim.mruNames[3] = Dvar_RegisterString(
        "modPrvAnimMruName3",
        "",
        1u,
        "Model previewer most recently used anim name 3");
    CG_ModPrvUpdateMru(g_mdlprv.anim.mruNames, g_mdlprv.anim.mruNameTable, 0);
    modPrvFromAnimMru = Dvar_RegisterEnum(
        "modPrvFromAnimMru",
        g_mdlprv.anim.mruNameTable,
        0,
        0x840u,
        "Model previewer most recently used 'from' animation");
    Dvar_UpdateEnumDomain((dvar_s*)(dvar_s*)modPrvFromAnimMru, g_mdlprv.anim.mruNameTable);
    modPrvToAnimMru = Dvar_RegisterEnum(
        "modPrvToAnimMru",
        g_mdlprv.anim.mruNameTable,
        0,
        0x840u,
        "Model previewer most recently used 'to' animation");
    Dvar_UpdateEnumDomain((dvar_s*)(dvar_s *)modPrvToAnimMru, g_mdlprv.anim.mruNameTable);
    modPrvLoadFromAnim = Dvar_RegisterEnum(
        "modPrvLoadFromAnim",
        g_mdlprv.system.animNames,
        0,
        0x800u,
        "Model previewer loaded 'from' animation");
    Dvar_UpdateEnumDomain((dvar_s*)(dvar_s *)modPrvLoadFromAnim, g_mdlprv.system.animNames);
    modPrvLoadToAnim = Dvar_RegisterEnum(
        "modPrvLoadToAnim",
        g_mdlprv.system.animNames,
        0,
        0x800u,
        "Model previewer loaded 'to' animation");
    Dvar_UpdateEnumDomain((dvar_s*)(dvar_s *)modPrvLoadToAnim, g_mdlprv.system.animNames);
    modPrvAnimForceLoop = Dvar_RegisterBool("modPrvAnimForceLoop", 0, 0, "Model Previewer - Force an animation loop");
    modPrvAnimRate = Dvar_RegisterFloat("modPrvAnimRate", 1.0, 0.1, 2.0, 0, NULL);
    modPrvAnimBlendMode = Dvar_RegisterEnum(
        "modPrvAnimBlendMode",
        modPrvAnimBlendModeNames,
        0,
        0,
        "Model previewer animation blending mode");
    modPrvAnimCrossBlendTime = Dvar_RegisterFloat("modPrvAnimCrossBlendTime", 0.99000001, 0.0, 1.0, 0, 0);
    modPrvAnimCrossBlendDuration = Dvar_RegisterFloat("modPrvAnimCrossBlendDuration", 0.0, 0.0, 5.0, 0, 0);
    modPrvAnimBlendWeight = Dvar_RegisterFloat("modPrvAnimBlendWeight", 0.5, 0.0, 1.0, 0, 0);
    modPrvAnimApplyDelta = Dvar_RegisterBool("modPrvAnimApplyDelta", 1, 0, "Model previewer animation apply delta");
    g_mdlprv.mat.nameTable[0] = "<None>";
    g_mdlprv.mat.nameTable[1] = 0;
    modPrvMatSelect = Dvar_RegisterEnum(
        "modPrvMatSelect",
        g_mdlprv.mat.nameTable,
        0,
        0x840u,
        "Model previewer material select");
    modPrvMatReplace = Dvar_RegisterEnum(
        "modPrvMatReplace",
        g_mdlprv.mat.nameTable,
        0,
        0x840u,
        "Model previewer material replace");
    g_mdlprv.light.nameTable[0] = "Use Original Lighting";
    g_mdlprv.light.nameTable[1] = 0;
    modPrvLightSetup = Dvar_RegisterEnum(
        "modPrvLightSetup",
        g_mdlprv.light.nameTable,
        0,
        0x800u,
        "Model previewer light setup");
    modPrvSunDirection = Dvar_RegisterVec3("modPrvSunDirection", 0.0, 0.0, 0.0, -180.0, 180.0, 0, 0);
    modPrvSunColor = Dvar_RegisterColor("modPrvSunColor", 1.0, 1.0, 1.0, 1.0, 0, 0);
    modPrvSunLight = Dvar_RegisterFloat("modPrvSunLight", 1.0, 0.0, 2.0, 0, 0);
    modPrvSunDiffuseColor = Dvar_RegisterColor("modPrvSunDiffuseColor", 1.0, 1.0, 1.0, 1.0, 0, 0);
    modPrvSunDiffuseFraction = Dvar_RegisterFloat("modPrvSunDiffuseFraction", 0.5, 0.0, 1.0, 0, 0);
    modPrvAmbientColor = Dvar_RegisterColor("modPrvAmbientColor", 1.0, 1.0, 1.0, 1.0, 0, 0);
    modPrvAmbientScale = Dvar_RegisterFloat("modPrvAmbientScale", 0.0, 0.0, 1.0, 0, 0);
    modPrvDisplayToggle = Dvar_RegisterBool("modPrvDisplayToggle", 1, 1u, "Show model previewer overlay");
    modPrvGamepadControlSpeed = Dvar_RegisterFloat("modPrvGamepadControlSpeed", 1.0, 0.1, 10.0, 0, 0);
    gpad_button_rstick_deflect_max = Dvar_RegisterFloat("gpad_button_rstick_deflect_max", 1.0, 0.0, 1.0, 0, 0);
    modPrvHideModel = Dvar_RegisterBool("modPrvHideModel", 0, 0x80u, "Skip drawing the model.");
    modPrvDrawDistanceToModel = Dvar_RegisterBool("modPrvDrawDistanceToModel", 1, 0x80u, "Print viewer's distance to model.");
}

void __cdecl CG_ModPrvSetEntityAxis(float *angles, float *quat)
{
    double v2; // fp13
    float v3[6]; // [sp+50h] [-20h] BYREF

    v2 = (float)(g_mdlprv.anim.deltaYaw + angles[1]);
    v3[0] = *angles;
    v3[2] = angles[2];
    v3[1] = (float)v2 + (float)180.0;
    AnglesToQuat(v3, quat);
}

void __cdecl MdlPrvGetBounds(float *mins, float *maxs, float *center)
{
    *mins = g_mdlprv.model.currentEntity.cull.mins[0] - g_mdlprv.model.currentEntity.placement.base.origin[0];
    mins[1] = g_mdlprv.model.currentEntity.cull.mins[1] - g_mdlprv.model.currentEntity.placement.base.origin[1];
    mins[2] = g_mdlprv.model.currentEntity.cull.mins[2] - g_mdlprv.model.currentEntity.placement.base.origin[2];

    *maxs = g_mdlprv.model.currentEntity.cull.maxs[0] - g_mdlprv.model.currentEntity.placement.base.origin[0];
    maxs[1] = g_mdlprv.model.currentEntity.cull.maxs[1] - g_mdlprv.model.currentEntity.placement.base.origin[1];
    maxs[2] = g_mdlprv.model.currentEntity.cull.maxs[2] - g_mdlprv.model.currentEntity.placement.base.origin[2];

    *center = (float)(*mins + *maxs) * (float)0.5;
    center[1] = (float)(mins[1] + maxs[1]) * (float)0.5;
    center[2] = (float)(mins[2] + maxs[2]) * (float)0.5;
}

void CG_ModPrvFrameModel()
{
    double v0; // fp30
    double v1; // fp31
    double v2; // fp0
    double v3; // fp0
    float v4; // [sp+50h] [-50h] BYREF
    float v5; // [sp+54h] [-4Ch]
    float v6; // [sp+58h] [-48h]
    float v7; // [sp+60h] [-40h] BYREF
    float v8; // [sp+64h] [-3Ch]
    float v9; // [sp+68h] [-38h]
    float v10[6]; // [sp+70h] [-30h] BYREF

    MdlPrvGetBounds(&v4, &v7, v10);
    v0 = v9;
    v1 = v6;
    Dvar_SetVec3((dvar_s*)modPrvCenterOffset, 0.0, 0.0, (float)((float)(v9 - v6) * (float)0.5));
    v2 = -FLT_MAX;
    if (I_fabs(v4) > -FLT_MAX)
        v2 = I_fabs(v4);
    if (v2 < I_fabs(v7))
        v2 = I_fabs(v7);
    if (v2 < I_fabs(v5))
        v2 = I_fabs(v5);
    if (v2 < I_fabs(v8))
        v2 = I_fabs(v8);
    if (v2 < I_fabs(v1))
        v2 = I_fabs(v1);
    if (v2 < I_fabs(v0))
        v2 = I_fabs(v0);
    v3 = (float)((float)v2 * (float)1.5);
    g_mdlprv.viewer.zNearChangeLimit = g_mdlprv.viewer.centerRadius;
    if (v3 < 10.0)
        v3 = 10.0;
    if (g_mdlprv.viewer.centerRadius < v3)
        g_mdlprv.viewer.centerRadius = v3;
}

void CG_ModPrvResetOrientation_f()
{
    double v0; // fp13
    float value; // [sp+50h] [-30h] BYREF
    float v2; // [sp+54h] [-2Ch]
    float v3; // [sp+58h] [-28h]

    g_mdlprv.viewer.horizontal = 0.0;
    g_mdlprv.viewer.vertical = -20.0;
    g_mdlprv.viewer.centerRadius = 100.0;
    Dvar_SetVec3((dvar_s*)modPrvCenterOffset, 0.0, 0.0, 0.0);
    g_mdlprv.model.currentEntity.placement.base.origin[0] = g_mdlprv.model.initialOrigin[0];
    g_mdlprv.model.currentEntity.placement.base.origin[1] = g_mdlprv.model.initialOrigin[1];
    g_mdlprv.model.currentEntity.placement.base.origin[2] = g_mdlprv.model.initialOrigin[2];
    Dvar_SetVec3((dvar_s *)modPrvOrigin, 0.0, 0.0, 0.0);
    Dvar_SetVec3((dvar_s *)modPrvRotationAngles, 0.0, g_mdlprv.model.initialYaw, 0.0);
    value = modPrvRotationAngles->current.value;
    v0 = (float)(g_mdlprv.anim.deltaYaw + modPrvRotationAngles->current.vector[1]);
    v2 = modPrvRotationAngles->current.vector[1];
    v3 = modPrvRotationAngles->current.vector[2];
    v2 = (float)v0 + (float)180.0;
    AnglesToQuat(&value, g_mdlprv.model.currentEntity.placement.base.quat);
    if (g_mdlprv.model.currentIndex >= 0)
        CG_ModPrvFrameModel();
}

// attributes: thunk
void __cdecl SetViewerActive(bool active)
{
    G_SetPM_MPViewer(active);
}

void CG_ModPrvUnregisterCmds()
{
    Cmd_RemoveCommand("modPrvResetOrientation");
    Cmd_RemoveCommand("modPrvExit");
}

void *__cdecl CG_ModPrvAlloc(int size)
{
    HunkUser *v2; // r3

    v2 = g_modPrvHunkUser;
    if (!g_modPrvHunkUser)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp", 600, 0, "%s", "g_modPrvHunkUser");
        v2 = g_modPrvHunkUser;
    }
    return Hunk_UserAlloc(v2, size, 4);
}

void __cdecl CG_ModPrvFree(void *allocated, int size)
{
    if (!allocated)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp", 608, 0, "%s", "allocated");
    if (!size)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp", 609, 0, "%s", "size");
    if (!g_modPrvHunkUser)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp", 610, 0, "%s", "g_modPrvHunkUser");
}

void CG_ModPrvResetGlobals()
{
    DObj_s **p_obj; // r10
    int v1; // r11

    p_obj = &g_mdlprv.model.clones[0].obj;
    g_mdlprv.system.cachedAllModels = 0;
    g_mdlprv.system.uiModePC = SELECTION_MODE;
    g_mdlprv.system.uiModeGPad = MDLPRVMODE_FOCUSED;
    g_mdlprv.system.focusedMode = FOCUSEDMODE_CAMERA;
    g_mdlprv.system.modelRotCamMode = MROTCAMMODE_STATIC;
    g_mdlprv.system.buttonTimes.mode = 0;
    g_mdlprv.system.buttonTimes.mdlRotMode = 0;
    g_mdlprv.system.buttonTimes.camUp = 0;
    g_mdlprv.system.buttonTimes.camDown = 0;
    g_mdlprv.system.buttonTimes.dropToFloor = 0;
    g_mdlprv.system.buttonTimes.clone = 0;
    g_mdlprv.system.buttonTimes.clearClones = 0;
    g_mdlprv.system.buttonTimes.walkabout = 0;
    g_mdlprv.system.buttonTimes.freeSpeed = 0;
    g_mdlprv.system.walkaboutActive = 0;
    g_mdlprv.model.currentIndex = -1;
    g_mdlprv.model.currentObj = 0;
    g_mdlprv.model.lodDist[0] = 10;
    g_mdlprv.model.lodDist[1] = 20;
    g_mdlprv.model.lodDist[2] = 30;
    g_mdlprv.model.lodDist[3] = 40;
    g_mdlprv.model.cloneNextIdx = 0;
    v1 = 10;
    do
    {
        --v1;
        *p_obj = 0;
        p_obj += 78;
    } while (v1);
    g_mdlprv.model.ragdoll = 0;
    g_mdlprv.viewer.centerRadius = 100.0;
    g_mdlprv.viewer.horizontal = 0.0;
    g_mdlprv.viewer.vertical = -20.0;
    g_mdlprv.viewer.zNear = Dvar_GetFloat("r_znear");
    g_mdlprv.viewer.freeModeSpeed = FREESPEED_NORMAL;
    g_mdlprv.viewer.freeModeOrigin[2] = 0.0;
    g_mdlprv.viewer.freeModeOrigin[1] = 0.0;
    g_mdlprv.anim.fromCurrentIndex = -1;
    g_mdlprv.viewer.freeModeOrigin[0] = 0.0;
    g_mdlprv.anim.toCurrentIndex = -1;
    g_mdlprv.viewer.freeModeAngles[2] = 0.0;
    g_mdlprv.viewer.freeModeAngles[1] = 0.0;
    g_mdlprv.viewer.freeModeAngles[0] = 0.0;
    Dvar_Reset(modPrvFromAnimMru, DVAR_SOURCE_INTERNAL);
    Dvar_Reset(modPrvToAnimMru, DVAR_SOURCE_INTERNAL);
    Dvar_Reset(modPrvLoadFromAnim, DVAR_SOURCE_INTERNAL);
    Dvar_Reset(modPrvLoadToAnim, DVAR_SOURCE_INTERNAL);
    Dvar_Reset(modPrvAnimRate, DVAR_SOURCE_INTERNAL);
    Dvar_Reset(modPrvAnimBlendWeight, DVAR_SOURCE_INTERNAL);
    Dvar_Reset((const dvar_s *)modPrvAnimBlendMode, DVAR_SOURCE_INTERNAL);
    Dvar_Reset(modPrvAnimCrossBlendDuration, DVAR_SOURCE_INTERNAL);
    Dvar_Reset(modPrvAnimCrossBlendTime, DVAR_SOURCE_INTERNAL);
    Dvar_Reset((const dvar_s *)modPrvAnimApplyDelta, DVAR_SOURCE_INTERNAL);
    g_mdlprv.mat.handleCount = -1;
    g_mdlprv.mat.handleArray = 0;
    g_mdlprv.mat.surfMatHandles = 0;
    Dvar_Reset(modPrvMatSelect, DVAR_SOURCE_INTERNAL);
    g_mdlprv.mat.replaceIndex = -1;
    g_mdlprv.mat.prevReplaced = 0;
    g_mdlprv.mat.selectSliderIndex = 0;
    g_mdlprv.mat.replaceSliderIndex = 0;
    Dvar_Reset(modPrvLightSetup, DVAR_SOURCE_INTERNAL);
}

bool __cdecl CG_ModPrvCompareString(const char *string1, const char *string2)
{
    unsigned int v3; // r9

    do
    {
        v3 = *(unsigned __int8 *)string1 - *(unsigned __int8 *)string2;
        if (!*string1)
            break;
        ++string1;
        ++string2;
    } while (!v3);
    //return _cntlzw(v3) == 0;
    return v3 != 0;
}

void __cdecl CG_ModPrvGetAssetName(const XModel *header, unsigned int *data)
{
    const char *Name; // r3

    iassert(data);

    if (data[1] == 3)
    {
        Name = XModelGetName(header);
        g_mdlprv.system.modelNames[(*data)++] = Hunk_CopyString(*((HunkUser **)g_mdlprv.system.modelNames - 1), Name);
    }
    if (data[1] == 2)
        g_mdlprv.system.animNames[(*data)++] = Hunk_CopyString(*((HunkUser **)g_mdlprv.system.animNames - 1), header->name);
}

void CG_ModPrvUnloadModel()
{
    XAnimTree_s *Tree; // r30
    XAnim_s *Anims; // r29
    HunkUser *v2; // r3

    if (g_mdlprv.model.currentObj)
    {
        Tree = DObjGetTree(g_mdlprv.model.currentObj);
        DObjFree(g_mdlprv.model.currentObj);
        g_mdlprv.model.currentObj = 0;
        g_mdlprv.model.currentIndex = -1;
        if (Tree)
        {
            Anims = XAnimGetAnims(Tree);
            XAnimFreeTree(Tree, CG_ModPrvFree);
            XAnimFreeAnims(Anims, CG_ModPrvFree);
            v2 = g_modPrvHunkUser;
            if (!g_modPrvHunkUser)
            {
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp", 886, 0, "%s", "g_modPrvHunkUser");
                v2 = g_modPrvHunkUser;
            }
            Hunk_UserDestroy(v2);
            g_modPrvHunkUser = 0;
        }
    }
    if (g_mdlprv.mat.handleArray)
    {
        Z_VirtualFree(g_mdlprv.mat.handleArray);
        g_mdlprv.mat.handleArray = 0;
        g_mdlprv.mat.handleCount = -1;
    }
    if (g_mdlprv.mat.surfMatHandles)
    {
        Z_VirtualFree(g_mdlprv.mat.surfMatHandles);
        g_mdlprv.mat.surfMatHandles = 0;
    }
    Dvar_Reset(cg_r_forceLod, DVAR_SOURCE_INTERNAL);
    if (gpad_button_rstick_deflect_max)
        Dvar_SetFloat(gpad_button_rstick_deflect_max, g_mdlprv.system.gamePadRStickDeflect);
}

const char *g_emptyEnumList[1] = { NULL };

void CG_ModPrvShutdown()
{
    if (g_mdlprv.inited)
    {
        g_mdlprv.inited = 0;
        if (g_mdlprv.system.modelNames)
        {
            Dvar_UpdateEnumDomain((dvar_s*)(dvar_s*)modPrvLoadModel, g_emptyEnumList);
            FS_FreeFileList(g_mdlprv.system.modelNames);
            g_mdlprv.system.modelNames = 0;
        }
        g_mdlprv.system.modelCount = 0;
        if (g_mdlprv.system.animNames)
        {
            Dvar_UpdateEnumDomain((dvar_s*)(dvar_s *)modPrvLoadFromAnim, g_emptyEnumList);
            Dvar_UpdateEnumDomain((dvar_s*)(dvar_s *)modPrvLoadToAnim, g_emptyEnumList);
            FS_FreeFileList(g_mdlprv.system.animNames);
            g_mdlprv.system.animNames = 0;
        }
        CG_ModPrvUnloadModel();
    }
}

void __cdecl CG_ModPrvDrawViewAxis(const float *centerPos)
{
    int i; // r31
    double v3; // fp13
    double v4; // fp0
    float v5; // [sp+50h] [-60h] BYREF
    float v6; // [sp+54h] [-5Ch]
    float v7; // [sp+58h] [-58h]
    float v8[6]; // [sp+60h] [-50h] BYREF

    for (i = 0; i < 3; ++i)
    {
        v5 = 0.0;
        v6 = 0.0;
        v7 = 0.0;
        v3 = *centerPos;
        *(float *)((char *)&v5 + i * 4) = g_mdlprv.viewer.centerRadius * (float)0.1;
        v5 = (float)v3 + v5;
        v6 = centerPos[1] + v6;
        v4 = (float)(centerPos[2] + v7);
        v8[0] = 0.0;
        v8[1] = 0.0;
        v8[2] = 0.0;
        v8[3] = 0.0;
        v8[i] = 1.0;
        v7 = v4;
        CG_DebugLine(centerPos, &v5, v8, 1, 0);
    }
}

void CG_ModPrvOriginUpdate()
{
    Dvar_ClearModified((dvar_s*)(dvar_s*)modPrvOrigin);
    g_mdlprv.model.currentEntity.placement.base.origin[0] = modPrvOrigin->current.value + g_mdlprv.model.initialOrigin[0];
    g_mdlprv.model.currentEntity.placement.base.origin[1] = modPrvOrigin->current.vector[1] + g_mdlprv.model.initialOrigin[1];
    g_mdlprv.model.currentEntity.placement.base.origin[2] = modPrvOrigin->current.vector[2] + g_mdlprv.model.initialOrigin[2];
}

void CG_ModPrvRotateUpdate()
{
    double v0; // fp13
    float value; // [sp+50h] [-20h] BYREF
    float v2; // [sp+54h] [-1Ch]
    float v3; // [sp+58h] [-18h]

    Dvar_ClearModified((dvar_s*)(dvar_s*)modPrvRotationAngles);
    value = modPrvRotationAngles->current.value;
    v0 = (float)(g_mdlprv.anim.deltaYaw + modPrvRotationAngles->current.vector[1]);
    v2 = modPrvRotationAngles->current.vector[1];
    v3 = modPrvRotationAngles->current.vector[2];
    v2 = (float)v0 + (float)180.0;
    AnglesToQuat(&value, g_mdlprv.model.currentEntity.placement.base.quat);
}

void CG_ModPrvModelResetRotation()
{
    double v0; // fp13
    float value; // [sp+50h] [-30h] BYREF
    float v2; // [sp+54h] [-2Ch]
    float v3; // [sp+58h] [-28h]

    Dvar_SetVec3((dvar_s *)modPrvRotationAngles, 0.0, g_mdlprv.model.initialYaw, 0.0);
    value = modPrvRotationAngles->current.value;
    v0 = (float)(g_mdlprv.anim.deltaYaw + modPrvRotationAngles->current.vector[1]);
    v2 = modPrvRotationAngles->current.vector[1];
    v3 = modPrvRotationAngles->current.vector[2];
    v2 = (float)v0 + (float)180.0;
    AnglesToQuat(&value, g_mdlprv.model.currentEntity.placement.base.quat);
}

void CG_ModPrvModelResetRotationXY()
{
    double v0; // fp13
    float value; // [sp+50h] [-20h] BYREF
    float v2; // [sp+54h] [-1Ch]
    float v3; // [sp+58h] [-18h]

    Dvar_SetVec3((dvar_s *)modPrvRotationAngles, 0.0, modPrvRotationAngles->current.vector[1], 0.0);
    value = modPrvRotationAngles->current.value;
    v0 = (float)(g_mdlprv.anim.deltaYaw + modPrvRotationAngles->current.vector[1]);
    v2 = modPrvRotationAngles->current.vector[1];
    v3 = modPrvRotationAngles->current.vector[2];
    v2 = (float)v0 + (float)180.0;
    AnglesToQuat(&value, g_mdlprv.model.currentEntity.placement.base.quat);
}

int __cdecl CG_ModPrvGetNumTotalBones(DObj_s *dobj)
{
    int v2; // r30
    int NumModels; // r29
    int i; // r31
    const XModel *Model; // r3

    if (!dobj)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp", 1217, 0, "%s", "dobj");
    v2 = 0;
    NumModels = DObjGetNumModels(dobj);
    for (i = 0; i < NumModels; v2 += XModelNumBones(Model))
        Model = DObjGetModel(dobj, i++);
    return v2;
}

int __cdecl CG_ModPrvGetNumSurfaces(DObj_s *obj, int lod)
{
    int NumModels; // r29
    char *v5; // r11
    int v6; // ctr
    char v8[80]; // [sp+50h] [-50h] BYREF

    if (!obj)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp", 1238, 0, "%s", "obj");
    if (lod < 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp", 1239, 0, "%s", "lod >= 0");
    NumModels = DObjGetNumModels(obj);
    if ((unsigned int)NumModels > 0x20)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp",
            1242,
            1,
            "%s",
            "modelCount <= ARRAY_COUNT( lods )");
    if (NumModels > 0)
    {
        v5 = v8;
        v6 = NumModels;
        do
        {
            *v5++ = lod;
            --v6;
        } while (v6);
    }
    return DObjGetNumSurfaces(obj, v8);
}

const char *__cdecl CG_ModPrvModelGetBoneName(DObj_s *dobj, int modelIndex, int boneIndex)
{
    XModel *Model; // r3
    unsigned __int16 *v7; // r29

    if (!dobj)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp", 1253, 0, "%s", "dobj");
    if (modelIndex < 0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp", 1254, 0, "%s", "modelIndex >= 0");
    if (boneIndex < 0 || boneIndex >= DObjNumBones(dobj))
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp",
            1256,
            0,
            "%s",
            "boneIndex >= 0 && boneIndex < DObjNumBones( dobj )");
    if (modelIndex < 0 || modelIndex >= DObjGetNumModels(dobj))
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp",
            1257,
            0,
            "%s",
            "modelIndex >= 0 && modelIndex < DObjGetNumModels( dobj )");
    Model = DObjGetModel(dobj, modelIndex);
    v7 = XModelBoneNames(Model);
    if (!v7)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp", 1264, 0, "%s", "boneNames");
    return SL_ConvertToString(v7[boneIndex]);
}

void CG_ModPrvDrawBones()
{
    DObj_s *currentObj; // r3
    int NumModels; // r18
    double v2; // fp31
    const char *v3; // r19
    int v4; // r26
    const XModel *Model; // r3
    int v6; // r23
    int v7; // r28
    const char *BoneName; // r29
    const char *v9; // r10
    const char *v10; // r11
    int v11; // r8
    char v12; // r11
    char v13; // r30
    const char *v14; // r5
    int v15; // r3
    int integer; // r11
    double v17; // fp30
    const char *v18; // r5
    const char *v19; // r5
    const float *v20; // r4
    const char *v21; // r5
    float v22[2]; // [sp+50h] [-210h] BYREF
    float v23; // [sp+58h] [-208h]
    float v24[4]; // [sp+60h] [-200h] BYREF
    float v25[4]; // [sp+70h] [-1F0h] BYREF
    float v26[4]; // [sp+80h] [-1E0h] BYREF
    float v27[4][3]; // [sp+90h] [-1D0h] BYREF
    float v28[4][3]; // [sp+C0h] [-1A0h] BYREF
    float v29[4][3]; // [sp+F0h] [-170h] BYREF
    float v30[4][3]; // [sp+120h] [-140h] BYREF
    char v31[64]; // [sp+150h] [-110h] BYREF
    char v32[64]; // [sp+190h] [-D0h] BYREF

    currentObj = g_mdlprv.model.currentObj;
    if (!g_mdlprv.model.currentObj)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp",
            1295,
            0,
            "%s",
            "g_mdlprv.model.currentObj");
        currentObj = g_mdlprv.model.currentObj;
    }
    NumModels = DObjGetNumModels(currentObj);
    v2 = (float)(g_mdlprv.viewer.centerRadius * (float)0.001953125);
    if (modPrvDrawBoneInfo->current.integer < 4u)
        v3 = 0;
    else
        v3 = Dvar_EnumToString(modPrvDrawBoneInfo);
    v4 = 0;
    if (NumModels > 0)
    {
        while (1)
        {
            Model = DObjGetModel(g_mdlprv.model.currentObj, v4);
            v6 = XModelNumBones(Model);
            v7 = 0;
            if (v6 > 0)
                break;
        LABEL_25:
            if (++v4 >= NumModels)
                return;
        }
        while (1)
        {
            BoneName = CG_ModPrvModelGetBoneName(g_mdlprv.model.currentObj, v4, v7);
            CG_DObjGetWorldBoneMatrix(g_mdlprv.model.currentEntity.info.pose, g_mdlprv.model.currentObj, v7, v30, v22);
            UnitQuatToAxis(g_mdlprv.model.currentEntity.placement.base.quat, (mat3x3&)v29);
            MatrixTranspose((const mat3x3&)v29, (mat3x3&)v27);
            v24[0] = v22[0] - g_mdlprv.model.currentEntity.placement.base.origin[0];
            v24[1] = v22[1] - g_mdlprv.model.currentEntity.placement.base.origin[1];
            v24[2] = v23 - g_mdlprv.model.currentEntity.placement.base.origin[2];
            MatrixTransformVector(v24, (const mat3x3&)v27, v26);
            MatrixMultiply((const mat3x3&)v30, (const mat3x3&)v27, (mat3x3&)v28);
            AxisToAngles((const mat3x3&)v28, v25);

            sprintf(
                v32,
                "angles( %.2f, %.2f, %.2f )",
                v25[0],
                v25[1],
                v25[2]
            );
              
            sprintf(
                v31,
                "pos( %.2f, %.2f, %.2f )",
                v26[0],
                v26[1],
                v26[2]
            );

            if (modPrvDrawBoneInfo->current.integer < 4u)
                goto LABEL_13;
            v9 = BoneName;
            v10 = v3;
            do
            {
                v11 = *(unsigned __int8 *)v10 - *(unsigned __int8 *)v9;
                if (!*v10)
                    break;
                ++v10;
                ++v9;
            } while (!v11);
            v12 = 1;
            if (v11)
                LABEL_13:
            v12 = 0;
            v13 = v12;
            v15 = strncmp(BoneName, "tag_", 4u);
            integer = modPrvDrawBoneInfo->current.integer;
            if (v15)
            {
                if (integer != 2 && integer != 3 && !v13)
                    goto LABEL_24;
                CL_AddDebugString(v22, colorRed, v2, v14, (int)BoneName, 0);
                v17 = (float)((float)v2 * (float)12.0);
                v23 = v23 - (float)((float)v2 * (float)12.0);
                CL_AddDebugString(v22, colorRed, v2, v18, (int)v31, 0);
                v20 = colorRed;
            }
            else
            {
                if (integer != 1 && integer != 3 && !v13)
                    goto LABEL_24;
                CL_AddDebugString(v22, colorGreen, v2, v14, (int)BoneName, 0);
                v17 = (float)((float)v2 * (float)12.0);
                v23 = v23 - (float)((float)v2 * (float)12.0);
                CL_AddDebugString(v22, colorGreen, v2, v21, (int)v31, 0);
                v20 = colorGreen;
            }
            v23 = v23 - (float)v17;
            CL_AddDebugString(v22, v20, v2, v19, (int)v32, 0);
        LABEL_24:
            if (++v7 >= v6)
                goto LABEL_25;
        }
    }
}

void __cdecl CG_ModPrvLoadAnimations(const char *animationFilename)
{
    const XAnimTree_s *Tree; // r3
    XAnimTree_s *v3; // r30
    XAnim_s *Anims; // r29
    HunkUser *v5; // r3
    XAnim_s *xAnims; // r29
    XAnimTree_s *animTree; // r30
    int v8; // r7
    unsigned int v9; // r6
    int fromCurrentIndex; // r11
    double value; // fp1

    if (g_mdlprv.model.currentObj)
    {
        Tree = DObjGetTree(g_mdlprv.model.currentObj);
        v3 = (XAnimTree_s *)Tree;
        if (Tree)
        {
            Anims = XAnimGetAnims(Tree);
            XAnimFreeTree(v3, CG_ModPrvFree);
            DObjSetTree(g_mdlprv.model.currentObj, 0);
            XAnimFreeAnims(Anims, CG_ModPrvFree);
            v5 = g_modPrvHunkUser;
            if (!g_modPrvHunkUser)
            {
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp",
                    1389,
                    0,
                    "%s",
                    "g_modPrvHunkUser");
                v5 = g_modPrvHunkUser;
            }
            Hunk_UserDestroy(v5);
            g_modPrvHunkUser = 0;
        }
        else if (g_modPrvHunkUser)
        {
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp", 1394, 0, "%s", "!g_modPrvHunkUser");
        }

        g_modPrvHunkUser = Hunk_UserCreate(0x10000, "CG_ModPrvLoadAnimations", 0, 0, 0);
        xAnims = XAnimCreateAnims("ModelPreviewer", 3, CG_ModPrvAlloc);
        iassert(xAnims);
        XAnimBlend(xAnims, 0, "root", 1, 2, 0);
        if (g_mdlprv.anim.fromCurrentIndex >= 0)
            BG_CreateXAnim(xAnims, 1, g_mdlprv.system.animNames[g_mdlprv.anim.fromCurrentIndex]);
        if (g_mdlprv.anim.toCurrentIndex >= 0)
            BG_CreateXAnim(xAnims, 2, g_mdlprv.system.animNames[g_mdlprv.anim.toCurrentIndex]);
        animTree = XAnimCreateTree(xAnims, CG_ModPrvAlloc);
        iassert(animTree);
        DObjSetTree(g_mdlprv.model.currentObj, animTree);
        fromCurrentIndex = g_mdlprv.anim.fromCurrentIndex;
        if (g_mdlprv.anim.fromCurrentIndex >= 0)
        {
            if (g_mdlprv.anim.toCurrentIndex >= 0 && *(unsigned int *)(modPrvAnimBlendMode + 12) == 1)
                value = modPrvAnimBlendWeight->current.value;
            else
                value = (float)((float)1.0 - modPrvAnimBlendWeight->current.value);
            XAnimSetGoalWeightKnobAll(g_mdlprv.model.currentObj, 1, 0, value, 1.0, 1.0, 0, 0, 1); // KISAKTODO: args sus
            //g_mdlprv.anim.isFromLooped = (_cntlzw(XAnimIsLooped(v6, 1u)) & 0x20) == 0;
            g_mdlprv.anim.isFromLooped = XAnimIsLooped(xAnims, 1);
            fromCurrentIndex = g_mdlprv.anim.fromCurrentIndex;
        }
        if (g_mdlprv.anim.toCurrentIndex >= 0)
        {
            if (fromCurrentIndex >= 0 && *(unsigned int *)(modPrvAnimBlendMode + 12) == 1)
                XAnimSetGoalWeightKnobAll(g_mdlprv.model.currentObj, 2, 0, 1.0, 1.0, 1.0, 0, 0, 1);// KISAKTODO: args sus
            //g_mdlprv.anim.isToLooped = (_cntlzw(XAnimIsLooped(v6, 2u)) & 0x20) == 0;
            g_mdlprv.anim.isToLooped = XAnimIsLooped(xAnims, 2);
        }
        if (animationFilename)
        {
            CG_ModPrvPushMruEntry(animationFilename, g_mdlprv.anim.mruNames, g_mdlprv.anim.mruNameTable, modPrvFromAnimMru);
            Dvar_UpdateEnumDomain((dvar_s*)modPrvToAnimMru, g_mdlprv.anim.mruNameTable);
        }
        g_mdlprv.model.currentEntity.placement.base.origin[0] = g_mdlprv.model.initialOrigin[0];
        g_mdlprv.model.currentEntity.placement.base.origin[1] = g_mdlprv.model.initialOrigin[1];
        g_mdlprv.anim.isToAnimPlaying = 0;
        g_mdlprv.model.currentEntity.placement.base.origin[2] = g_mdlprv.model.initialOrigin[2];
        g_mdlprv.anim.isAnimPlaying = 1;
        g_mdlprv.anim.stepCounter = FLT_MAX;
        g_mdlprv.anim.deltaYaw = 0.0;
        g_mdlprv.model.currentEntity.placement.base.origin[0] = modPrvOrigin->current.value
            + g_mdlprv.model.initialOrigin[0];
        g_mdlprv.model.currentEntity.placement.base.origin[1] = modPrvOrigin->current.vector[1]
            + g_mdlprv.model.initialOrigin[1];
        g_mdlprv.model.currentEntity.placement.base.origin[2] = modPrvOrigin->current.vector[2]
            + g_mdlprv.model.initialOrigin[2];
    }
}

void __cdecl CG_ModPrvApplyAnimationBlend(double deltaTime)
{
    double Time; // fp31
    int v2; // r7
    unsigned int v3; // r6
    const XAnimTree_s *Tree; // r3

    Time = 0.0;
    if (g_mdlprv.model.currentObj)
    {
        Tree = DObjGetTree(g_mdlprv.model.currentObj);
        if (Tree)
        {
            if (g_mdlprv.anim.fromCurrentIndex >= 0)
                Time = XAnimGetTime(Tree, 1u);
            if (g_mdlprv.anim.toCurrentIndex >= 0
                && Time >= modPrvAnimCrossBlendTime->current.value
                && !g_mdlprv.anim.isToAnimPlaying)
            {
                XAnimSetGoalWeightKnobAll(
                    g_mdlprv.model.currentObj,
                    2u,
                    0,
                    1.0,
                    modPrvAnimCrossBlendDuration->current.value,
                    1.0,
                    0, 0, 1);// KISAKTODO: args sus
                g_mdlprv.anim.isToAnimPlaying = 1;
            }
        }
    }
}

void __cdecl CG_ModPrvApplyDelta(double deltaTime)
{
    float v1; // [sp+50h] [-50h] BYREF
    float v2; // [sp+54h] [-4Ch]
    float v3; // [sp+58h] [-48h] BYREF
    float v4; // [sp+5Ch] [-44h]
    float v5; // [sp+60h] [-40h] BYREF
    float v6; // [sp+64h] [-3Ch]
    float v7; // [sp+68h] [-38h]
    float v8; // [sp+70h] [-30h] BYREF
    float v9; // [sp+74h] [-2Ch]
    float v10; // [sp+78h] [-28h]

    if (g_mdlprv.model.currentObj
        && DObjGetTree(g_mdlprv.model.currentObj)
        && *(_BYTE *)(modPrvAnimApplyDelta + 12)
        && g_mdlprv.anim.isAnimPlaying)
    {
        v3 = 0.0;
        v4 = 0.0;
        v1 = 0.0;
        v2 = 0.0;
        v5 = 0.0;
        v6 = 0.0;
        v7 = 0.0;
        v8 = 0.0;
        v9 = 0.0;
        v10 = 0.0;
        if (g_mdlprv.anim.fromCurrentIndex >= 0)
            XAnimCalcDelta(g_mdlprv.model.currentObj, 1u, &v3, &v5, 1);
        if (g_mdlprv.anim.toCurrentIndex >= 0)
            XAnimCalcDelta(g_mdlprv.model.currentObj, 2u, &v1, &v8, 1);
        v1 = v1 + v3;
        v2 = v2 + v4;
        g_mdlprv.anim.deltaYaw = RotationToYaw(&v1);
        CG_ModPrvSetEntityAxis((float*)&modPrvRotationAngles->current.value, g_mdlprv.model.currentEntity.placement.base.quat);
        g_mdlprv.model.currentEntity.placement.base.origin[0] = g_mdlprv.model.currentEntity.placement.base.origin[0]
            + (float)((float)(v8 + v5) * (float)-1.0);
        g_mdlprv.model.currentEntity.placement.base.origin[1] = g_mdlprv.model.currentEntity.placement.base.origin[1]
            + (float)((float)(v9 + v6) * (float)-1.0);
        g_mdlprv.model.currentEntity.placement.base.origin[2] = g_mdlprv.model.currentEntity.placement.base.origin[2]
            + (float)((float)(v10 + v7) * (float)-1.0);
    }
}

int __cdecl CG_ModPrvLoopAnimation()
{
    double v0; // fp28
    double v1; // fp30
    const XAnimTree_s *Tree; // r3
    XAnimTree_s *v3; // r29
    int v4; // r7
    unsigned int v5; // r6
    int fromCurrentIndex; // r11
    double Time; // fp1
    int toCurrentIndex; // r10
    double v9; // fp1
    int result; // r3
    double value; // fp30
    int v12; // r7
    unsigned int v13; // r6

    v0 = 0.0;
    v1 = 0.0;
    if (!g_mdlprv.model.currentObj)
        return 0;
    Tree = DObjGetTree(g_mdlprv.model.currentObj);
    v3 = (XAnimTree_s *)Tree;
    if (!Tree || !XAnimGetAnims(Tree))
        return 0;
    fromCurrentIndex = g_mdlprv.anim.fromCurrentIndex;
    if (g_mdlprv.anim.fromCurrentIndex >= 0)
    {
        Time = XAnimGetTime(v3, 1u);
        fromCurrentIndex = g_mdlprv.anim.fromCurrentIndex;
        v0 = Time;
    }
    toCurrentIndex = g_mdlprv.anim.toCurrentIndex;
    if (g_mdlprv.anim.toCurrentIndex >= 0)
    {
        v9 = XAnimGetTime(v3, 2u);
        toCurrentIndex = g_mdlprv.anim.toCurrentIndex;
        fromCurrentIndex = g_mdlprv.anim.fromCurrentIndex;
        v1 = v9;
    }
    if (fromCurrentIndex >= 0)
    {
        if (toCurrentIndex >= 0)
            goto LABEL_15;
        if (v0 >= 1.0)
        {
            if (!g_mdlprv.anim.isFromLooped)
            {
                XAnimSetGoalWeightKnobAll(g_mdlprv.model.currentObj, 1u, 0, 1.0, 1.0, 1.0, 0, 0, 1);// KISAKTODO: args sus
                g_mdlprv.model.currentEntity.placement.base.origin[0] = g_mdlprv.model.initialOrigin[0];
                g_mdlprv.model.currentEntity.placement.base.origin[1] = g_mdlprv.model.initialOrigin[1];
                g_mdlprv.model.currentEntity.placement.base.origin[2] = g_mdlprv.model.initialOrigin[2];
                g_mdlprv.model.currentEntity.placement.base.origin[0] = modPrvOrigin->current.value
                    + g_mdlprv.model.initialOrigin[0];
                g_mdlprv.model.currentEntity.placement.base.origin[1] = modPrvOrigin->current.vector[1]
                    + g_mdlprv.model.initialOrigin[1];
                g_mdlprv.model.currentEntity.placement.base.origin[2] = modPrvOrigin->current.vector[2]
                    + g_mdlprv.model.initialOrigin[2];
            }
            return 1;
        }
    }
    if (toCurrentIndex < 0)
        return 0;
LABEL_15:
    if (fromCurrentIndex >= 0 || v1 < 1.0)
    {
        if (fromCurrentIndex >= 0 && v1 >= 1.0)
        {
            if (*(unsigned int *)(modPrvAnimBlendMode + 12) == 1)
                value = modPrvAnimBlendWeight->current.value;
            else
                value = (float)((float)1.0 - modPrvAnimBlendWeight->current.value);
            XAnimClearTreeGoalWeights(v3, 2u, 0.0);
            XAnimSetGoalWeightKnobAll(g_mdlprv.model.currentObj, 1u, 0, value, 1.0, 1.0, 0, 0, 1);// KISAKTODO: args sus
            g_mdlprv.model.currentEntity.placement.base.origin[0] = g_mdlprv.model.initialOrigin[0];
            result = 1;
            g_mdlprv.anim.isToAnimPlaying = 0;
            g_mdlprv.model.currentEntity.placement.base.origin[1] = g_mdlprv.model.initialOrigin[1];
            g_mdlprv.model.currentEntity.placement.base.origin[2] = g_mdlprv.model.initialOrigin[2];
            g_mdlprv.model.currentEntity.placement.base.origin[0] = modPrvOrigin->current.value
                + g_mdlprv.model.initialOrigin[0];
            g_mdlprv.model.currentEntity.placement.base.origin[1] = modPrvOrigin->current.vector[1]
                + g_mdlprv.model.initialOrigin[1];
            g_mdlprv.model.currentEntity.placement.base.origin[2] = modPrvOrigin->current.vector[2]
                + g_mdlprv.model.initialOrigin[2];
            return result;
        }
        return 0;
    }
    if (!g_mdlprv.anim.isToLooped)
    {
        XAnimSetGoalWeightKnobAll(g_mdlprv.model.currentObj, 2u, 0, 1.0, 1.0, 1.0, 0, 0, 1);// KISAKTODO: args sus
        g_mdlprv.model.currentEntity.placement.base.origin[0] = g_mdlprv.model.initialOrigin[0];
        g_mdlprv.model.currentEntity.placement.base.origin[1] = g_mdlprv.model.initialOrigin[1];
        g_mdlprv.model.currentEntity.placement.base.origin[2] = g_mdlprv.model.initialOrigin[2];
        g_mdlprv.model.currentEntity.placement.base.origin[0] = g_mdlprv.model.initialOrigin[0]
            + modPrvOrigin->current.value;
        g_mdlprv.model.currentEntity.placement.base.origin[1] = modPrvOrigin->current.vector[1]
            + g_mdlprv.model.initialOrigin[1];
        g_mdlprv.model.currentEntity.placement.base.origin[2] = modPrvOrigin->current.vector[2]
            + g_mdlprv.model.initialOrigin[2];
    }
    return 1;
}

void __cdecl CG_ModPrvAnimRecentAccept(const dvar_s *dvar, int *currentIndex)
{
    const char *v4; // r3
    const char *v5; // r29
    int v6; // r6
    const char **animNames; // r7
    const char *v8; // r10
    const char *v9; // r11
    int v10; // r8

    if (!dvar)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp", 1609, 0, "%s", "dvar");
    if (!currentIndex)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp", 1610, 0, "%s", "currentIndex");
    Dvar_ClearModified((dvar_s*)dvar);
    v4 = Dvar_EnumToString(dvar);
    v5 = v4;
    v6 = 0;
    if (g_mdlprv.system.animCount <= 0)
    {
    LABEL_12:
        if (dvar != modPrvFromAnimMru)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp",
                1627,
                1,
                "%s\n\t(dvar->name) = %s",
                "(dvar == modPrvFromAnimMru || dvar == modPrvFromAnimMru)",
                dvar->name);
        Com_Printf(14, "Model previewer could not load <%s> because it does not exist.\n ", v5);
        CG_ModPrvRemoveMruEntry(g_mdlprv.anim.mruNames, g_mdlprv.anim.mruNameTable, modPrvFromAnimMru);
        Dvar_UpdateEnumDomain((dvar_s*)modPrvToAnimMru, g_mdlprv.anim.mruNameTable);
    }
    else
    {
        animNames = g_mdlprv.system.animNames;
        while (1)
        {
            v8 = *animNames;
            v9 = v4;
            do
            {
                v10 = *(unsigned __int8 *)v9 - *(unsigned __int8 *)v8;
                if (!*v9)
                    break;
                ++v9;
                ++v8;
            } while (!v10);
            if (!v10)
                break;
            ++v6;
            ++animNames;
            if (v6 >= g_mdlprv.system.animCount)
                goto LABEL_12;
        }
        *currentIndex = v6;
        CG_ModPrvLoadAnimations(v4);
    }
}

void __cdecl CG_ModPrvLoadAnimAccept(const dvar_s *dvar, int *currentIndex)
{
    if (!dvar)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp", 1636, 0, "%s", "dvar");
    if (!currentIndex)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp", 1637, 0, "%s", "currentIndex");
    Dvar_ClearModified((dvar_s*)dvar);
    *currentIndex = dvar->current.integer;
    CG_ModPrvLoadAnimations(g_mdlprv.system.animNames[dvar->current.integer]);
}

XAnimTree_s *CG_ModPrvAnimBlendWeightUpdate()
{
    XAnimTree_s *result; // r3
    int fromCurrentIndex; // r11
    XAnimTree_s *v2; // r30
    double v3; // fp30
    int v4; // r7
    unsigned int v5; // r6
    unsigned int v6; // r5
    double value; // fp30
    int v8; // r7
    unsigned int v9; // r6
    unsigned int v10; // r5

    Dvar_ClearModified((dvar_s*)modPrvAnimBlendWeight);
    result = (XAnimTree_s *)g_mdlprv.model.currentObj;
    if (g_mdlprv.model.currentObj)
    {
        result = DObjGetTree(g_mdlprv.model.currentObj);
        fromCurrentIndex = g_mdlprv.anim.fromCurrentIndex;
        v2 = result;
        if (g_mdlprv.anim.fromCurrentIndex >= 0)
        {
            if (g_mdlprv.anim.toCurrentIndex >= 0)
                v3 = (float)((float)1.0 - modPrvAnimBlendWeight->current.value);
            else
                v3 = 1.0;
            XAnimClearTreeGoalWeights(result, 1u, 0.0);
            result = (XAnimTree_s *)XAnimSetGoalWeight(g_mdlprv.model.currentObj, 1u, v3, 1.0, 1.0, v6, v5, v4);
            fromCurrentIndex = g_mdlprv.anim.fromCurrentIndex;
        }
        if (g_mdlprv.anim.toCurrentIndex >= 0)
        {
            if (fromCurrentIndex >= 0)
                value = modPrvAnimBlendWeight->current.value;
            else
                value = 1.0;
            XAnimClearTreeGoalWeights(v2, 2u, 0.0);
            return (XAnimTree_s *)XAnimSetGoalWeight(g_mdlprv.model.currentObj, 2u, value, 1.0, 1.0, v10, v9, v8);
        }
    }
    return result;
}

void CG_ModPrvMatReplaceAccepted()
{
    int v0; // r31
    int v1; // r29

    Dvar_ClearModified((dvar_s*)modPrvMatReplace);
    if (!modPrvMatSelect->current.integer)
        g_mdlprv.mat.replaceIndex = -1;
    v0 = 0;
    if (g_mdlprv.model.surfaceCount > 0)
    {
        v1 = 0;
        do
            R_DObjReplaceMaterial(
                g_mdlprv.model.currentObj,
                modPrvLod->current.integer,
                v0++,
                g_mdlprv.mat.surfMatHandles[v1++]);
        while (v0 < g_mdlprv.model.surfaceCount);
    }
    g_mdlprv.mat.prevReplaced = 0;
}

void CG_ModPrvMatReplaceUpdate()
{
    int integer; // r11
    int replaceIndex; // r8
    Material *prevReplaced; // r6
    int surfaceCount; // r10
    int v4; // r30
    int v5; // r29
    Material **handleArray; // r11
    int v7; // r30
    int v8; // r29

    if (modPrvMatReplace->latched.integer != g_mdlprv.system.lastMatReplace)
    {
        g_mdlprv.system.lastMatReplace = modPrvMatReplace->latched.integer;
        if (modPrvMatSelect->current.integer)
        {
            integer = modPrvMatReplace->latched.integer;
            if (integer)
            {
                replaceIndex = integer - 1;
                g_mdlprv.mat.replaceIndex = integer - 1;
                prevReplaced = g_mdlprv.mat.prevReplaced;
                surfaceCount = g_mdlprv.model.surfaceCount;
                if (g_mdlprv.mat.prevReplaced)
                {
                    v4 = 0;
                    if (g_mdlprv.model.surfaceCount > 0)
                    {
                        v5 = 0;
                        do
                        {
                            if (g_mdlprv.mat.surfMatHandles[v5] == prevReplaced)
                            {
                                R_DObjReplaceMaterial(g_mdlprv.model.currentObj, modPrvLod->current.integer, v4, prevReplaced);
                                prevReplaced = g_mdlprv.mat.prevReplaced;
                                surfaceCount = g_mdlprv.model.surfaceCount;
                            }
                            ++v4;
                            ++v5;
                        } while (v4 < surfaceCount);
                        replaceIndex = g_mdlprv.mat.replaceIndex;
                    }
                }
                handleArray = g_mdlprv.mat.handleArray;
                v7 = 0;
                if (surfaceCount > 0)
                {
                    v8 = 0;
                    do
                    {
                        if (g_mdlprv.mat.surfMatHandles[v8] == handleArray[modPrvMatSelect->current.integer - 1])
                        {
                            R_DObjReplaceMaterial(
                                g_mdlprv.model.currentObj,
                                modPrvLod->current.integer,
                                v7,
                                handleArray[replaceIndex]);
                            replaceIndex = g_mdlprv.mat.replaceIndex;
                            handleArray = g_mdlprv.mat.handleArray;
                            surfaceCount = g_mdlprv.model.surfaceCount;
                        }
                        ++v7;
                        ++v8;
                    } while (v7 < surfaceCount);
                }
                g_mdlprv.mat.prevReplaced = handleArray[replaceIndex];
            }
        }
    }
}

void CG_ModPrvLightSetupModified()
{
    int integer; // r11
    double v1; // fp31
    double v2; // fp30
    double v3; // fp29

    Dvar_ClearModified((dvar_s*)modPrvLightSetup);
    integer = modPrvLightSetup->current.integer;
    if (integer)
    {
        v1 = g_mdlprv.light.tweakableSunLight.angles[0];
        v2 = g_mdlprv.light.tweakableSunLight.angles[1];
        v3 = g_mdlprv.light.tweakableSunLight.angles[2];
        memcpy(
            &g_mdlprv.light.tweakableSunLight,
            &g_mdlprv.mat.nameTable[32 * integer + 57],
            sizeof(g_mdlprv.light.tweakableSunLight));
        g_mdlprv.light.tweakableSunLight.angles[0] = v1;
        g_mdlprv.light.tweakableSunLight.angles[1] = v2;
        g_mdlprv.light.tweakableSunLight.angles[2] = v3;
        R_InterpretSunLightParseParams(&g_mdlprv.light.tweakableSunLight);
    }
}

bool __cdecl CG_ModPrvAnyLightValuesChanged()
{
    return *(_BYTE *)(modPrvSunDirection + 11)
        || *(_BYTE *)(modPrvSunColor + 11)
        || *(_BYTE *)(modPrvSunLight + 11)
        || *(_BYTE *)(modPrvSunDiffuseColor + 11)
        || *(_BYTE *)(modPrvSunDiffuseFraction + 11)
        || *(_BYTE *)(modPrvAmbientColor + 11)
        || modPrvAmbientScale->modified;
}

void CG_ModPrvLightValuesUpdate()
{
    float color[4]; // [sp+50h] [-30h] BYREF

    Dvar_ClearModified((dvar_s*)(const dvar_s *)modPrvSunDirection);
    Dvar_ClearModified((dvar_s*)(const dvar_s *)modPrvSunColor);
    Dvar_ClearModified((dvar_s*)(const dvar_s *)modPrvSunLight);
    Dvar_ClearModified((dvar_s*)(const dvar_s *)modPrvSunDiffuseColor);
    Dvar_ClearModified((dvar_s*)(const dvar_s *)modPrvSunDiffuseFraction);
    Dvar_ClearModified((dvar_s*)(const dvar_s *)modPrvAmbientColor);
    Dvar_ClearModified((dvar_s*)modPrvAmbientScale);

    g_mdlprv.light.tweakableSunLight.angles[0] = modPrvSunDirection->current.vector[0];
    g_mdlprv.light.tweakableSunLight.angles[1] = modPrvSunDirection->current.vector[1];
    g_mdlprv.light.tweakableSunLight.angles[2] = modPrvSunDirection->current.vector[2];
    Dvar_GetUnpackedColor((const dvar_s *)modPrvSunColor, color);

    g_mdlprv.light.tweakableSunLight.sunColor[0] = color[0];
    g_mdlprv.light.tweakableSunLight.sunColor[1] = color[1];
    g_mdlprv.light.tweakableSunLight.sunColor[2] = color[2];
    g_mdlprv.light.tweakableSunLight.sunLight = modPrvSunLight->current.value;
    Dvar_GetUnpackedColor((const dvar_s *)modPrvSunDiffuseColor, color);

    g_mdlprv.light.tweakableSunLight.diffuseColor[0] = color[0];
    g_mdlprv.light.tweakableSunLight.diffuseColor[1] = color[1];
    g_mdlprv.light.tweakableSunLight.diffuseColor[2] = color[2];
    g_mdlprv.light.tweakableSunLight.diffuseFraction = modPrvSunDiffuseFraction->current.value;
    Dvar_GetUnpackedColor((const dvar_s *)modPrvAmbientColor, color);

    g_mdlprv.light.tweakableSunLight.ambientColor[0] = color[0];
    g_mdlprv.light.tweakableSunLight.ambientColor[1] = color[1];
    g_mdlprv.light.tweakableSunLight.ambientColor[2] = color[2];
    g_mdlprv.light.tweakableSunLight.ambientScale = modPrvAmbientScale->current.value;
    R_InterpretSunLightParseParams(&g_mdlprv.light.tweakableSunLight);
}

void __cdecl TRACK_cg_modelpreviewer()
{
    track_static_alloc_internal(&g_mdlprv, 34904, "g_mdlprv", 0);
    track_static_alloc_internal(mpDefaultDrawBoneOptions, 16, "mpDefaultDrawBoneOptions", 0);
    track_static_alloc_internal(modPrvAnimBlendModeNames, 12, "modPrvAnimBlendModeNames", 0);
}

void __cdecl CG_ModelPreviewerPauseAnim()
{
    if (g_mdlprv.model.currentObj && (g_mdlprv.anim.fromCurrentIndex >= 0 || g_mdlprv.anim.toCurrentIndex >= 0))
    {
        g_mdlprv.anim.stepCounter = FLT_MAX;
        g_mdlprv.anim.isAnimPlaying = !g_mdlprv.anim.isAnimPlaying;
    }
}

// aislop
//void __cdecl CG_ModelPreviewerStepAnim(double deltaTime)
//{
//    if (g_mdlprv.model.currentObj && (g_mdlprv.anim.fromCurrentIndex >= 0 || g_mdlprv.anim.toCurrentIndex >= 0))
//    {
//        g_mdlprv.anim.isAnimPlaying = 1;
//        __asm { fsel      f0, f1, f1, f0 }
//        g_mdlprv.anim.stepCounter = _FP0;
//    }
//}

void __cdecl CG_ModelPreviewerStepAnim(float deltaTime)
{
    if (g_mdlprv.model.currentObj &&
        (g_mdlprv.anim.fromCurrentIndex >= 0 || g_mdlprv.anim.toCurrentIndex >= 0))
    {
        g_mdlprv.anim.isAnimPlaying = true;

        // Step only if deltaTime is positive
        g_mdlprv.anim.stepCounter = (deltaTime > 0.0f) ? deltaTime : 0.0f;
    }
}

//int __cdecl MdlPrvPrint(double x, double y, const char *txt, const float *a5, int a6, Font_s *a7)
//{
//    return CG_DrawDevString(&scrPlaceFull, x, y, 0.5, 0.5, txt, a4, a5, a6, a7);
//}

//int __cdecl MdlPrvPrintColor(
//    double x,
//    double y,
//    const char *txt,
//    const float *color,
//    const float *a5,
//    int a6,
//    Font_s *a7)
//{
//    return CG_DrawDevString(&scrPlaceFull, x, y, 0.5, 0.5, (const char *)color, a5, a6, a7);
//}

struct ButtonInfo
{
    char name[16];
    char desc[48];
    const float *nameColor;
    int id;
};
const ButtonInfo g_buttons[27] =
{
  {
    {
      'Y',
      '\0',
      '\0',
      '\0',
      '\0',
      '\0',
      '\0',
      '\0',
      '\0',
      '\0',
      '\0',
      '\0',
      '\0',
      '\0',
      '\0',
      '\0'
    },
    ": Free/Focus Toggle",
    colorYellow,
    4
  },
  { "X", ": Drop Model to Floor", colorBlue, 3 },
  { "L Click", ": Walkabout", colorLtGrey, 16 },
  { "L Click", ": Exit Walkabout", colorLtGrey, 16 },
  { "A", ": Place Model in Front of Camera", colorGreen, 1 },
  { "B", ": Place Model at Camera's Position", colorRed, 2 },
  { "RB", ": Toggle Move Speed", colorWhite, 6 },
  { "LB", ": Toggle Ragdoll", colorWhite, 5 },
  { "L Trig", ": Move Down", colorWhite, 18 },
  { "R Trig", ": Move Up", colorWhite, 19 },
  { "DPad", ": Move Up/Down", colorLtGrey, 0 },
  { "L Trig", ": Move Model", colorWhite, 18 },
  { "R Trig", ": Rotate Model", colorWhite, 19 },
  { "L+R Trig", ": Move Focus", colorWhite, 0 },
  { "A", ": Clone Model", colorGreen, 1 },
  { "B", ": Clear All Clones", colorRed, 2 },
  { "L Stick", ": Zoom", colorLtGrey, 0 },
  { "R Stick", ": Orbit", colorLtGrey, 0 },
  { "L Stick", ": Left/Right/Near/Far", colorLtGrey, 0 },
  { "R Stick", ": Up/Down", colorLtGrey, 0 },
  { "B", ": Camera Travel Toggle", colorRed, 2 },
  { "L Stick", ": Pitch/Roll", colorLtGrey, 0 },
  { "R Stick", ": Yaw", colorLtGrey, 0 },
  { "LB", ": Reset Model Rotation", colorWhite, 5 },
  { "L Stick", ": Left/Right/Near/Far", colorLtGrey, 0 },
  { "R Stick", ": Up/Down", colorLtGrey, 0 },
  { "A", ": Reset Focus", colorGreen, 1 }
};



void __cdecl MdlPrvPrintHelpLine(ButtonNames idx, float vPos)
{
    Font_s *v6; // r7
    int v7; // r6
    const float *v8; // r5
    const char *v9; // r4

    CG_DrawDevString(&scrPlaceFull, 0.0, vPos, 0.5, 0.5, (char*)g_buttons[(int)idx].name, g_buttons[(int)idx].nameColor, 0, NULL);
    CG_DrawDevString(&scrPlaceFull, 50.0, vPos, 0.5, 0.5, (char*)g_buttons[(int)idx].desc, g_buttons[(int)idx].nameColor, 0, NULL);
}

void DrawDistFromModel()
{
    char *v0; // r3
    const float *v1; // r5
    const char *v2; // r4
    double v3; // [sp+18h] [-48h]

    if (modPrvDrawDistanceToModel->current.enabled)
    {
        v3 = sqrtf((float)((float)((float)(cgArray[0].refdef.vieworg[1]
            - g_mdlprv.model.currentEntity.placement.base.origin[1])
            * (float)(cgArray[0].refdef.vieworg[1]
                - g_mdlprv.model.currentEntity.placement.base.origin[1]))
            + (float)((float)((float)(cgArray[0].refdef.vieworg[0]
                - g_mdlprv.model.currentEntity.placement.base.origin[0])
                * (float)(cgArray[0].refdef.vieworg[0]
                    - g_mdlprv.model.currentEntity.placement.base.origin[0]))
                + (float)((float)(cgArray[0].refdef.vieworg[2]
                    - g_mdlprv.model.currentEntity.placement.base.origin[2])
                    * (float)(cgArray[0].refdef.vieworg[2]
                        - g_mdlprv.model.currentEntity.placement.base.origin[2])))));
        v0 = va((const char *)HIDWORD(v3), LODWORD(v3));
        CG_DrawSmallDevStringColor(&scrPlaceFull, 500.0, 400.0, (char*)v2, v1, (int)v0);
    }
}

void __cdecl MdlPrvDrawOverlayGamepad()
{
    const float *v0; // r5
    const char *v1; // r4
    __int64 v2; // r11
    double v3; // fp30
    int v4; // r3
    __int64 v5; // r11
    double v6; // fp31
    int v7; // r3
    __int64 v8; // r11
    double v9; // fp31
    const float *v10; // r5
    const char *v11; // r4
    int v12; // r3
    __int64 v13; // r11
    double v14; // fp31
    const char *v15; // r6
    int v16; // r3
    __int64 v17; // r11
    double v18; // fp31
    int v19; // r3
    __int64 v20; // r11
    double v21; // fp31
    int v22; // r3
    __int64 v23; // r11
    double v24; // fp31
    int v25; // r3
    __int64 v26; // r11
    double v27; // fp31
    int v28; // r3
    __int64 v29; // r11
    double v30; // fp31
    int v31; // r3
    __int64 v32; // r11
    ButtonNames v33; // r3
    __int64 v34; // fp13
    int v35; // r3
    __int64 v36; // r11
    double v37; // fp31
    int v38; // r3
    __int64 v39; // r11
    double v40; // fp31
    const float *v41; // r5
    const char *v42; // r4
    int v43; // r3
    __int64 v44; // r10
    int v45; // r3
    __int64 v46; // r11
    const char *v47; // r6
    int v48; // r3
    __int64 v49; // r11
    int v50; // r3
    __int64 v51; // r11
    int v52; // r3
    __int64 v53; // r11
    int v54; // r3
    __int64 v55; // r11
    double v56; // fp31
    int v57; // r3
    __int64 v58; // r11
    int v59; // r3
    __int64 v60; // r11
    int v61; // r3
    __int64 v62; // r11
    int v63; // r3

    if (g_mdlprv.model.currentIndex >= 0 && modPrvDisplayToggle->current.enabled)
    {
        DrawDistFromModel();
        if (g_mdlprv.system.walkaboutActive)
        {
            CG_DrawSmallDevStringColor(&scrPlaceFull, 300.0, 0.0, (char*)"Walkabout Mode", colorRed, 7);
            MdlPrvPrintHelpLine(BTN_WALKABOUT_EXIT, 20.0);
            return;
        }
        if (g_mdlprv.system.uiModeGPad == MDLPRVMODE_FREE)
            LODWORD(v2) = CG_DrawSmallDevStringColor(
                &scrPlaceFull,
                300.0,
                0.0,
                (char*)"Freelook Mode",
                colorRed,
                7);
        else
            LODWORD(v2) = CG_DrawSmallDevStringColor(&scrPlaceFull, 300.0, 0.0, (char*)"Focused Mode", colorRed, 7);
        v3 = (float)((float)v2 * (float)0.75);
        MdlPrvPrintHelpLine(BTN_MODESWITCH, 20.0);
        LODWORD(v5) = v4;
        v6 = (float)((float)((float)v5 * (float)0.75) + 20.0);
        MdlPrvPrintHelpLine(BTN_DROPMDL, v6);
        LODWORD(v8) = v7;
        v9 = (float)((float)((float)v8 * 0.75) + (float)v6);
        MdlPrvPrintHelpLine(BTN_WALKABOUT_ENTER, v9);
        LODWORD(v13) = v12;
        v14 = (float)((float)((float)v13 * 0.75) + (float)v9);
        if (g_mdlprv.system.uiModeGPad != MDLPRVMODE_FREE)
        {
            MdlPrvPrintHelpLine(BTN_FOCUS_TOGGLEMOV, v14);
            LODWORD(v36) = v35;
            v37 = (float)((float)((float)v36 * 0.75) + (float)v14);
            MdlPrvPrintHelpLine(BTN_FOCUS_TOGGLEROT, v37);
            LODWORD(v39) = v38;
            v40 = (float)((float)((float)v39 * 0.75) + (float)v37);
            MdlPrvPrintHelpLine(BTN_FOCUS_TOGGLEFOCALMOVE, v40);
            LODWORD(v44) = v43;
            v30 = (float)((float)((float)v44 * 0.75) + (float)v40);
            if (g_mdlprv.system.focusedMode == FOCUSEDMODE_MODELMOVE)
            {
                CG_DrawSmallDevStringColor(&scrPlaceFull, 300.0, v3, (char*)"Model Move", colorWhiteFaded, 7);
                MdlPrvPrintHelpLine(BTN_FOCUS_MMOV_2D, v30);
                LODWORD(v46) = v45;
                v33 = BTN_FOCUS_MMOV_UPDOWN;
                v34 = v46;
            }
            else if (g_mdlprv.system.focusedMode == FOCUSEDMODE_MODELROTATE)
            {
                if (g_mdlprv.system.modelRotCamMode)
                    v47 = "Model Rotate (with camera)";
                else
                    v47 = "Model Rotate";
                CG_DrawSmallDevStringColor(&scrPlaceFull, 300.0, v3, (char*)v42, colorWhiteFaded, 7);
                MdlPrvPrintHelpLine(BTN_FOCUS_MROT_TOGGLECAM, v30);
                LODWORD(v49) = v48;
                v30 = (float)((float)((float)v49 * 0.75) + (float)v30);
                if (g_mdlprv.system.modelRotCamMode == MROTCAMMODE_STATIC)
                {
                    MdlPrvPrintHelpLine(BTN_FOCUS_MROT_PITCHROLL, v30);
                    LODWORD(v51) = v50;
                    v30 = (float)((float)((float)v51 * 0.75) + (float)v30);
                }
                MdlPrvPrintHelpLine(BTN_FOCUS_MROT_YAW, v30);
                LODWORD(v53) = v52;
                v33 = BTN_FOCUS_MROT_RESET;
                v34 = v53;
            }
            else
            {
                if (g_mdlprv.system.focusedMode)
                {
                    CG_DrawSmallDevStringColor(&scrPlaceFull, 300.0, v3, (char*)"Focus Move", colorWhiteFaded, 7);
                    MdlPrvPrintHelpLine(BTN_FOCUS_FOCALMOVE_2D, v30);
                    LODWORD(v62) = v61;
                    v30 = (float)((float)((float)v62 * 0.75) + (float)v30);
                    MdlPrvPrintHelpLine(BTN_FOCUS_FOCALMOVE_UPDOWN, v30);
                    LODWORD(v60) = v63;
                    v33 = BTN_FOCUS_FOCALMOVE_RESET;
                }
                else
                {
                    MdlPrvPrintHelpLine(BTN_FOCUS_DEFAULT_CLONEMODEL, v30);
                    LODWORD(v55) = v54;
                    v56 = (float)((float)((float)v55 * 0.75) + (float)v30);
                    MdlPrvPrintHelpLine(BTN_FOCUS_DEFAULT_CLEARCLONES, v56);
                    LODWORD(v58) = v57;
                    v30 = (float)((float)((float)v58 * 0.75) + (float)v56);
                    MdlPrvPrintHelpLine(BTN_FOCUS_DEFAULT_ZOOM, v30);
                    LODWORD(v60) = v59;
                    v33 = BTN_FOCUS_DEFAULT_ORBIT;
                }
                v34 = v60;
            }
            goto LABEL_28;
        }
        if (g_mdlprv.viewer.freeModeSpeed == FREESPEED_SLOW)
        {
            v15 = "Move Slow";
        }
        else
        {
            if (g_mdlprv.viewer.freeModeSpeed != FREESPEED_FAST)
            {
            LABEL_14:
                MdlPrvPrintHelpLine(BTN_FREE_DROPFRONT, v14);
                LODWORD(v17) = v16;
                v18 = (float)((float)((float)v17 * 0.75) + (float)v14);
                MdlPrvPrintHelpLine(BTN_FREE_DROPPOS, v18);
                LODWORD(v20) = v19;
                v21 = (float)((float)((float)v20 * 0.75) + (float)v18);
                MdlPrvPrintHelpLine(BTN_FREE_TOGGLEMOVESPEED, v21);
                LODWORD(v23) = v22;
                v24 = (float)((float)((float)v23 * 0.75) + (float)v21);
                MdlPrvPrintHelpLine(BTN_FREE_TOGGLERAGDOLL, v24);
                LODWORD(v26) = v25;
                v27 = (float)((float)((float)v26 * 0.75) + (float)v24);
                MdlPrvPrintHelpLine(BTN_FREE_UP, v27);
                LODWORD(v29) = v28;
                v30 = (float)((float)((float)v29 * 0.75) + (float)v27);
                MdlPrvPrintHelpLine(BTN_FREE_DOWN, v30);
                LODWORD(v32) = v31;
                v33 = BTN_FREE_UPDOWN;
                v34 = v32;
            LABEL_28:
                MdlPrvPrintHelpLine(v33, (float)((float)((float)v34 * 0.75) + (float)v30));
                return;
            }
            v15 = "Move Fast";
        }
        CG_DrawSmallDevStringColor(&scrPlaceFull, 315.0, v3, (char*)v15, colorWhiteFaded, 7);
        goto LABEL_14;
    }
}

// attributes: thunk
void __cdecl CG_ModelPreviewerDrawInfo()
{
    MdlPrvDrawOverlayGamepad();
}

void __cdecl CG_ModelPreviewerRotateCamera(double dx, double dy)
{
    g_mdlprv.viewer.vertical = g_mdlprv.viewer.vertical - (float)dy;
    g_mdlprv.viewer.horizontal = g_mdlprv.viewer.horizontal - (float)dx;
}

void __cdecl CG_ModelPreviewerZoomCamera(double dx, double dy)
{
    double v2; // fp11
    double v3; // fp0
    double v4; // fp0

    if (I_fabs(dy) <= I_fabs(dx))
        v2 = (float)((float)dx * (float)-0.30000001);
    else
        v2 = (float)((float)dy * (float)-0.30000001);
    v3 = (float)(g_mdlprv.viewer.centerRadius * (float)0.0040000002);
    if (v3 > 5.0)
        v3 = 5.0;
    v4 = (float)((float)((float)v3 * (float)v2) + g_mdlprv.viewer.centerRadius);
    g_mdlprv.viewer.centerRadius = v4;
    if (v4 < 1.0)
    {
        v4 = 1.0;
        g_mdlprv.viewer.centerRadius = 1.0;
    }
    if (v4 >= g_mdlprv.viewer.zNearChangeLimit)
        g_mdlprv.viewer.zNear = Dvar_GetFloat("r_znear");
    else
        g_mdlprv.viewer.zNear = 0.0099999998;
}

void __cdecl MdlPrvModelOriginSet(float *origin)
{
    Dvar_SetVec3((dvar_s *)modPrvOrigin, 0.0f, 0.0f, 0.0f);

    g_mdlprv.model.initialOrigin[0] = origin[0];
    g_mdlprv.model.initialOrigin[1] = origin[1];
    g_mdlprv.model.initialOrigin[2] = origin[2];

    g_mdlprv.model.currentEntity.placement.base.origin[0] = origin[0];
    g_mdlprv.model.currentEntity.placement.base.origin[1] = origin[1];
    g_mdlprv.model.currentEntity.placement.base.origin[2] = origin[2];
}

void __cdecl MdlPrvModelOriginOffset(double dx, double dy, double dz)
{
    g_mdlprv.model.initialOrigin[0] = g_mdlprv.model.initialOrigin[0] + (float)dx;
    g_mdlprv.model.initialOrigin[1] = g_mdlprv.model.initialOrigin[1] + (float)dy;
    g_mdlprv.model.initialOrigin[2] = g_mdlprv.model.initialOrigin[2] + (float)dz;
    g_mdlprv.model.currentEntity.placement.base.origin[0] = g_mdlprv.model.currentEntity.placement.base.origin[0]
        + (float)dx;
    g_mdlprv.model.currentEntity.placement.base.origin[1] = g_mdlprv.model.currentEntity.placement.base.origin[1]
        + (float)dy;
    g_mdlprv.model.currentEntity.placement.base.origin[2] = g_mdlprv.model.currentEntity.placement.base.origin[2]
        + (float)dz;
}

void __cdecl MdlPrvSpin_(unsigned int yprIdx, double deg)
{
    double v2; // fp29
    long double v3; // fp2
    long double v4; // fp2
    double v5; // fp28
    long double v6; // fp2
    double v7; // fp31
    long double v8; // fp2
    float v9; // [sp+50h] [-D0h] BYREF
    float v10; // [sp+54h] [-CCh]
    float v11; // [sp+58h] [-C8h]
    float v12[4][3]; // [sp+60h] [-C0h] BYREF
    float v13[4][3]; // [sp+90h] [-90h] BYREF
    float v14[6][3]; // [sp+C0h] [-60h] BYREF

    v9 = 0.0;
    v10 = 0.0;
    v11 = 0.0;
    *(&v9 + yprIdx) = deg;
    AnglesToAxis(&v9, v13);
    UnitQuatToAxis(g_mdlprv.model.currentEntity.placement.base.quat, (mat3x3&)v12);
    MatrixMultiply((const mat3x3&)v13, (const mat3x3&)v12, (mat3x3&)v14);
    AxisCopy((const mat3x3&)v14, (mat3x3&)v12);
    AxisToQuat(v12, g_mdlprv.model.currentEntity.placement.base.quat);
    AxisToAngles((const mat3x3&)v12, &v9);
    v2 = (float)((float)(v10 + (float)180.0) * (float)0.0027777778);
    *(double *)&v3 = (float)((float)((float)(v10 + (float)180.0) * (float)0.0027777778) + (float)0.5);
    v4 = floor(v3);
    v5 = (float)(v9 * (float)0.0027777778);
    v10 = (float)((float)v2 - (float)*(double *)&v4) * (float)360.0;
    *(double *)&v4 = (float)((float)(v9 * (float)0.0027777778) + (float)0.5);
    v6 = floor(v4);
    v7 = (float)(v11 * (float)0.0027777778);
    v9 = (float)((float)v5 - (float)*(double *)&v6) * (float)360.0;
    *(double *)&v6 = (float)((float)(v11 * (float)0.0027777778) + (float)0.5);
    v8 = floor(v6);
    v11 = (float)((float)v7 - (float)*(double *)&v8) * (float)360.0;
    Dvar_SetVec3((dvar_s*)modPrvRotationAngles, v9, v10, v11);
}

void __cdecl MdlPrvSpinYaw(double deg)
{
    MdlPrvSpin_(1u, deg);
}

void __cdecl MdlPrvSpinPitch(double deg)
{
    MdlPrvSpin_(0, deg);
}

void __cdecl MdlPrvSpinRoll(double deg)
{
    MdlPrvSpin_(2u, deg);
}

void __cdecl MdlPrvSpinYawOffset(double deg)
{
    double v1; // fp0
    double v2; // fp30
    double v3; // fp29
    double v4; // fp28
    double v5; // fp31
    double v8; // fp13
    double v9; // fp31
    double v10; // fp30
    double v11; // fp29
    float v12[4]; // [sp+50h] [-A0h] BYREF
    float v13[4]; // [sp+60h] [-90h] BYREF
    float v14[4]; // [sp+70h] [-80h] BYREF
    float v15; // [sp+80h] [-70h] BYREF
    float v16; // [sp+84h] [-6Ch]
    float v17; // [sp+88h] [-68h]
    float v18[6][3]; // [sp+90h] [-60h] BYREF

    v12[0] = 0.0;
    v12[2] = 0.0;
    v12[1] = -deg;
    MdlPrvSpin_(1, deg);
    AnglesToAxis(v12, v18);
    v1 = modPrvCenterOffset->current.vector[2];
    v2 = (float)(g_mdlprv.model.currentEntity.placement.base.origin[0] + modPrvCenterOffset->current.value);
    v3 = (float)(g_mdlprv.model.currentEntity.placement.base.origin[1] + modPrvCenterOffset->current.vector[1]);
    v4 = (float)(g_mdlprv.model.currentEntity.placement.base.origin[2] + modPrvCenterOffset->current.vector[2]);
    v5 = sqrtf((float)((float)(modPrvCenterOffset->current.vector[1] * modPrvCenterOffset->current.vector[1])
        + (float)((float)(modPrvCenterOffset->current.vector[2] * modPrvCenterOffset->current.vector[2])
        + (float)(modPrvCenterOffset->current.value * modPrvCenterOffset->current.value))));

    // aislop
    //_FP10 = -v5;
    //__asm { fsel      f10, f10, f11, f31 }
    //v8 = (float)((float)((float)1.0 / (float)_FP10) * modPrvCenterOffset->current.vector[1]);
    //v13[0] = (float)((float)1.0 / (float)_FP10) * modPrvCenterOffset->current.value;

    float _FP10 = -v5;
    _FP10 = (_FP10 > 0.0f) ? _FP10 : 0.0f;
    v8 = (1.0f / _FP10) * modPrvCenterOffset->current.vector[1];
    v13[0] = (1.0f / _FP10) * modPrvCenterOffset->current.value;


    v13[1] = v8;
    v13[2] = (float)v1 * (float)((float)1.0 / (float)_FP10);
    Vec3Rotate(v13, (const mat3x3&)v18, v14);
    v15 = (float)(v14[0] * (float)-v5) + (float)v2;
    v16 = (float)(v14[1] * (float)-v5) + (float)v3;
    v17 = (float)(v14[2] * (float)-v5) + (float)v4;
    v9 = (float)((float)v2 - v15);
    v10 = (float)((float)v3 - v16);
    v11 = (float)((float)v4 - v17);
    MdlPrvModelOriginSet(&v15);
    Dvar_SetVec3((dvar_s*)modPrvCenterOffset, v9, v10, v11);
}

void MdlPrvSpinClearPitchRoll()
{
    float (*v0)[3]; // r3
    float v1[4]; // [sp+50h] [-50h] BYREF
    float v2[4][3]; // [sp+60h] [-40h] BYREF

    UnitQuatToAxis(g_mdlprv.model.currentEntity.placement.base.quat, (mat3x3&)v2);
    AxisToAngles((const mat3x3&)v2, v1);
    YawToAxis(v1[1], (mat3x3&)v0);
    AxisToQuat(v2, g_mdlprv.model.currentEntity.placement.base.quat);
}

void __cdecl MdlPrvMoveModelUpDown(double dist)
{
    g_mdlprv.model.initialOrigin[2] = g_mdlprv.model.initialOrigin[2] + (float)dist;
    g_mdlprv.model.currentEntity.placement.base.origin[2] = g_mdlprv.model.currentEntity.placement.base.origin[2]
        + (float)dist;
}

// aislop
void __cdecl MdlPrvMoveModel2D(const cg_s *cgGlob, float away, float left)
{
    float v10, v11, v12, v13;
    float lenHorz, lenVert;
    float invLenHorz, invLenVert;

    // Compute forward/right vectors from yaw
    YawVectors(cgGlob->refdefViewAngles[1], (float *)&cgGlob->clientNum, &v10);  // v10..v13 filled

    // Compute negative magnitudes of the horizontal and vertical vectors
    lenHorz = sqrtf(v10 * v10 + v11 * v11);
    lenVert = sqrtf(v12 * v12 + v13 * v13);

    // Prevent division by zero (replacing fsel)
    invLenHorz = (lenHorz > 0.0f) ? (1.0f / lenHorz) : 0.0f;
    invLenVert = (lenVert > 0.0f) ? (1.0f / lenVert) : 0.0f;

    // Apply movement to model origin
    g_mdlprv.model.initialOrigin[1] += (invLenVert * v13 * left) + (invLenHorz * v11 * away);
    g_mdlprv.model.initialOrigin[0] += (invLenVert * v12 * left) + (invLenHorz * v10 * away);

    // Apply movement to entity placement
    g_mdlprv.model.currentEntity.placement.base.origin[0] += (invLenVert * v12 * left) + (invLenHorz * v10 * away);
    g_mdlprv.model.currentEntity.placement.base.origin[1] += (invLenVert * v13 * left) + (invLenHorz * v11 * away);
}


void __cdecl MdlPrvMoveFocusUpDown(double dist)
{
    Dvar_SetVec3(
        (dvar_s *)modPrvCenterOffset,
        modPrvCenterOffset->current.value,
        modPrvCenterOffset->current.vector[1],
        (float)(modPrvCenterOffset->current.vector[2] + (float)dist));
}

// aislop
void __cdecl MdlPrvMoveFocus2D(const cg_s *cgGlob, float away, float left)
{
    float v11, v12, v13, v14, v15, v16;

    YawVectors(cgGlob->refdefViewAngles[1], (float *)&cgGlob->clientNum, &v11);

    float lenHorz = sqrtf(v12 * v12 + v11 * v11);
    float lenVert = sqrtf(v15 * v15 + v14 * v14);

    float invLenHorz = (lenHorz > 0.0f) ? (1.0f / lenHorz) : 0.0f;
    float invLenVert = (lenVert > 0.0f) ? (1.0f / lenVert) : 0.0f;

    float leftOffsetX = (invLenVert * v14) * left;
    float leftOffsetY = (invLenVert * v15) * left;
    float awayOffsetX = (invLenHorz * v11) * away;
    float awayOffsetY = (invLenHorz * v12) * away;

    v14 = leftOffsetX;
    v15 = leftOffsetY;
    v11 = awayOffsetX;
    v13 = invLenHorz * 0.0f * away; // likely a no-op
    v16 = invLenVert * 0.0f * left; // likely a no-op

    Dvar_SetVec3(
        (dvar_s *)modPrvCenterOffset,
        modPrvCenterOffset->current.value + v14 + v11,
        modPrvCenterOffset->current.vector[1] + v15 + awayOffsetY,
        modPrvCenterOffset->current.vector[2]);
}


void MdlPrvMoveFocusReset()
{
    float v0[4]; // [sp+50h] [-40h] BYREF
    float v1[4]; // [sp+60h] [-30h] BYREF
    float v2[6]; // [sp+70h] [-20h] BYREF

    if (g_mdlprv.model.currentIndex >= 0)
    {
        MdlPrvGetBounds(v1, v0, v2);
        Dvar_SetVec3((dvar_s*)modPrvCenterOffset, 0.0, 0.0, (float)((float)(v0[2] - v1[2]) * (float)0.5));
    }
}

void __cdecl MdlPrvFreeMove(const cg_s *cgGlob, double dx, double dy)
{
    float v5[4]; // [sp+50h] [-50h] BYREF
    float v6[4]; // [sp+60h] [-40h] BYREF
    float v7[6]; // [sp+70h] [-30h] BYREF

    AngleVectors(cgGlob->refdefViewAngles, v6, v5, v7);
    g_mdlprv.viewer.freeModeOrigin[0] = (float)(g_mdlprv.viewer.freeModeOrigin[0] + (float)(v5[0] * (float)dy))
        + (float)(v6[0] * (float)dx);
    g_mdlprv.viewer.freeModeOrigin[1] = (float)(g_mdlprv.viewer.freeModeOrigin[1] + (float)(v5[1] * (float)dy))
        + (float)(v6[1] * (float)dx);
    g_mdlprv.viewer.freeModeOrigin[2] = (float)(g_mdlprv.viewer.freeModeOrigin[2] + (float)(v5[2] * (float)dy))
        + (float)(v6[2] * (float)dx);
}

void __cdecl MdlPrvFreeMoveVertical(const cg_s *cgGlob, double dz)
{
    float v3[4]; // [sp+50h] [-40h] BYREF
    float v4[4]; // [sp+60h] [-30h] BYREF
    float v5[4]; // [sp+70h] [-20h] BYREF

    AngleVectors(cgGlob->refdefViewAngles, v5, v4, v3);
    g_mdlprv.viewer.freeModeOrigin[0] = (float)(v3[0] * (float)dz) + g_mdlprv.viewer.freeModeOrigin[0];
    g_mdlprv.viewer.freeModeOrigin[1] = (float)(v3[1] * (float)dz) + g_mdlprv.viewer.freeModeOrigin[1];
    g_mdlprv.viewer.freeModeOrigin[2] = (float)(v3[2] * (float)dz) + g_mdlprv.viewer.freeModeOrigin[2];
}

// local variable allocation has failed, the output may be wrong!
void __cdecl MdlPrvFreeRot(double yaw, double pitch)
{
    double v2; // fp1
    double v3; // fp30
    double v4; // fp1
    long double v5; // fp2
    double v6; // fp31
    long double v7; // fp2

    v3 = (float)((float)(g_mdlprv.viewer.freeModeAngles[1] + (float)v2) * (float)0.0027777778);
    g_mdlprv.viewer.freeModeAngles[1] = g_mdlprv.viewer.freeModeAngles[1] + (float)v2;
    g_mdlprv.viewer.freeModeAngles[0] = g_mdlprv.viewer.freeModeAngles[0] + (float)pitch;
    v4 = (float)((float)v3 + (float)0.5);
    v5 = floor(*(long double *)&pitch);
    v6 = (float)(g_mdlprv.viewer.freeModeAngles[0] * (float)0.0027777778);
    g_mdlprv.viewer.freeModeAngles[1] = (float)((float)v3 - (float)*(double *)&v5) * (float)360.0;
    *(double *)&v5 = (float)((float)(g_mdlprv.viewer.freeModeAngles[0] * (float)0.0027777778) + (float)0.5);
    v7 = floor(v5);
    g_mdlprv.viewer.freeModeAngles[0] = (float)((float)v6 - (float)*(double *)&v7) * (float)360.0;
}

// attributes: thunk
void __cdecl MdlPrvFreePlaceModel(float *pos)
{
    MdlPrvModelOriginSet(pos);
}

void __cdecl MdlPrvFreePlaceModelInFrontCamera(const cg_s *cgGlob)
{
    const float *refdefViewAngles; // r30
    double v3; // fp0
    double value; // fp8
    double v5; // fp7
    double v6; // fp6
    double v7; // fp11
    float v8; // [sp+50h] [-60h] BYREF
    float v9; // [sp+54h] [-5Ch]
    float v10; // [sp+58h] [-58h]
    float v11[4]; // [sp+60h] [-50h] BYREF
    float v12[4]; // [sp+70h] [-40h] BYREF
    float v13[6]; // [sp+80h] [-30h] BYREF

    refdefViewAngles = cgGlob->refdefViewAngles;
    AngleVectors(cgGlob->refdefViewAngles, &v8, v13, v12);
    v3 = (float)(g_mdlprv.viewer.centerRadius * v10);
    v10 = g_mdlprv.viewer.centerRadius * v10;
    v8 = g_mdlprv.viewer.centerRadius * v8;
    v9 = g_mdlprv.viewer.centerRadius * v9;
    value = modPrvCenterOffset->current.value;
    v5 = modPrvCenterOffset->current.vector[1];
    v6 = modPrvCenterOffset->current.vector[2];
    g_mdlprv.viewer.horizontal = cgGlob->refdefViewAngles[1];
    v7 = *refdefViewAngles;
    v11[0] = (float)(g_mdlprv.viewer.freeModeOrigin[0] + v8) - (float)value;
    v11[1] = (float)(g_mdlprv.viewer.freeModeOrigin[1] + v9) - (float)v5;
    v11[2] = (float)(g_mdlprv.viewer.freeModeOrigin[2] + (float)v3) - (float)v6;
    g_mdlprv.viewer.vertical = -v7;
    MdlPrvModelOriginSet(v11);
}

void MdlPrvModeToggle()
{
    //g_mdlprv.system.uiModeGPad = (_cntlzw(g_mdlprv.system.uiModeGPad - 1) & 0x20) == 0;
    g_mdlprv.system.uiModeGPad = (MdlPrvUiModeGamepad)(g_mdlprv.system.uiModeGPad == 1);
}

void MdlPrvRotModeToggle()
{
    float (*v0)[3]; // r3
    float v1[4]; // [sp+50h] [-50h] BYREF
    float v2[4][3]; // [sp+60h] [-40h] BYREF

    g_mdlprv.system.modelRotCamMode = (MdlPrvMRotCamMode)(g_mdlprv.system.modelRotCamMode == MROTCAMMODE_STATIC);
    UnitQuatToAxis(g_mdlprv.model.currentEntity.placement.base.quat, (mat3x3&)v2);
    AxisToAngles((const mat3x3&)v2, v1);
    YawToAxis(v1[1], (mat3x3&)v0);
    AxisToQuat(v2, g_mdlprv.model.currentEntity.placement.base.quat);
}

void MdlPrvDropToFloor()
{
    float v0[2]; // [sp+50h] [-80h] BYREF
    float v1; // [sp+58h] [-78h]
    float v2[4]; // [sp+60h] [-70h] BYREF
    float v3[4]; // [sp+70h] [-60h] BYREF
    float v4[4]; // [sp+80h] [-50h] BYREF
    trace_t v5; // [sp+90h] [-40h] BYREF

    MdlPrvGetBounds(v0, v2, v4);
    v2[2] = 5.0;
    v2[1] = 5.0;
    v2[0] = 5.0;
    v4[0] = g_mdlprv.model.currentEntity.placement.base.origin[0];
    v3[0] = g_mdlprv.model.currentEntity.placement.base.origin[0];
    v4[1] = g_mdlprv.model.currentEntity.placement.base.origin[1];
    v3[1] = g_mdlprv.model.currentEntity.placement.base.origin[1];
    v3[2] = g_mdlprv.model.currentEntity.placement.base.origin[2] - (float)3000.0;
    v0[1] = -5.0;
    v0[0] = -5.0;
    v4[2] = g_mdlprv.model.currentEntity.placement.base.origin[2] - (float)(v1 - (float)5.0);
    v1 = -5.0;
    CG_TraceCapsule(&v5, v4, v0, v2, v3, -1, 1);
    if (!v5.allsolid && v5.fraction != 1.0)
    {
        g_mdlprv.model.initialOrigin[2] = g_mdlprv.model.initialOrigin[2] + (float)(v5.fraction * (float)-3000.0);
        g_mdlprv.model.currentEntity.placement.base.origin[2] = g_mdlprv.model.currentEntity.placement.base.origin[2]
            + (float)(v5.fraction * (float)-3000.0);
    }
}

void __cdecl MdlPrvCloneClear(MdlPrvClone *clone)
{
    DObj_s *obj; // r3

    obj = clone->obj;
    if (obj)
    {
        DObjFree(obj);
        clone->obj = 0;
    }
}

void MdlPrvCloneClearAll()
{
    int v0; // r30
    DObj_s **p_obj; // r31

    v0 = 10;
    p_obj = &g_mdlprv.model.clones[0].obj;
    do
    {
        if (*p_obj)
        {
            DObjFree(*p_obj);
            *p_obj = 0;
        }
        --v0;
        p_obj += 78;
    } while (v0);
}

void __cdecl MdlPrvCloneModel(const cg_s *cgGlob)
{
    int cloneNextIdx; // r11
    MdlPrvClone *v3; // r30
    DObj_s *obj; // r3
    float v5; // [sp+50h] [-60h] BYREF
    float v6; // [sp+54h] [-5Ch]
    float v7[4]; // [sp+60h] [-50h] BYREF
    float v8; // [sp+70h] [-40h] BYREF

    cloneNextIdx = g_mdlprv.model.cloneNextIdx;
    if (g_mdlprv.model.cloneNextIdx >= 0xAu)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp",
            2812,
            0,
            "%s",
            "g_mdlprv.model.cloneNextIdx < CLONES_COUNT");
        cloneNextIdx = g_mdlprv.model.cloneNextIdx;
    }
    v3 = &g_mdlprv.model.clones[cloneNextIdx];
    obj = v3->obj;
    if (obj)
    {
        DObjFree(obj);
        v3->obj = 0;
    }
    DObjGetTree(g_mdlprv.model.currentObj);
    DObjClone(g_mdlprv.model.currentObj, (DObj_s*)v3->objBuf);
    v3->obj = (DObj_s *)v3->objBuf;
    DObjSetTree((DObj_s *)v3->objBuf, 0);
    memcpy(v3, &g_mdlprv.model.currentEntity, 0x7Cu);
    AngleVectors(cgGlob->refdefViewAngles, &v8, &v5, v7);
    ++g_mdlprv.model.cloneNextIdx;
    g_mdlprv.model.initialOrigin[1] = g_mdlprv.model.initialOrigin[1] + (float)(v6 * (float)16.0);
    g_mdlprv.model.initialOrigin[0] = g_mdlprv.model.initialOrigin[0] + (float)(v5 * (float)16.0);
    g_mdlprv.model.currentEntity.placement.base.origin[0] = g_mdlprv.model.currentEntity.placement.base.origin[0]
        + (float)(v5 * (float)16.0);
    g_mdlprv.model.currentEntity.placement.base.origin[1] = g_mdlprv.model.currentEntity.placement.base.origin[1]
        + (float)(v6 * (float)16.0);
    if (g_mdlprv.model.cloneNextIdx >= 0xAu)
        g_mdlprv.model.cloneNextIdx = 0;
}

void MdlPrvFreeSpeedToggle()
{
    if (g_mdlprv.viewer.freeModeSpeed)
    {
        if (g_mdlprv.viewer.freeModeSpeed == FREESPEED_SLOW)
        {
            g_mdlprv.viewer.freeModeSpeed = FREESPEED_NORMAL;
        }
        else if (g_mdlprv.viewer.freeModeSpeed < (unsigned int)(FREESPEED_FAST | FREESPEED_SLOW))
        {
            g_mdlprv.viewer.freeModeSpeed = FREESPEED_SLOW;
        }
    }
    else
    {
        g_mdlprv.viewer.freeModeSpeed = FREESPEED_FAST;
    }
}

void MdlPrvRagdollToggle()
{
    if (g_mdlprv.model.ragdoll)
    {
        Ragdoll_Remove(g_mdlprv.model.ragdoll);
        g_mdlprv.model.ragdoll = 0;
    }
    else
    {
        g_mdlprv.model.ragdollDef = g_mdlprv.model.ragdollDef == 0;
        g_mdlprv.model.ragdoll = Ragdoll_CreateRagdollForDObjRaw(
            0,
            g_mdlprv.model.ragdollDef,
            g_mdlprv.model.currentEntity.info.pose,
            g_mdlprv.model.currentObj);
    }
}

void __cdecl MdlPrvControlsGamepad(int localClientNum, double forward, double side, double pitch, double yaw)
{
    int v10; // r29
    __int64 v11; // r11
    double v12; // fp31
    double v13; // fp26
    double v14; // fp1
    char v15[312]; // [sp+60h] [-160h] BYREF

    if (DevGui_IsActive())
        return;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v10 = Sys_Milliseconds();
    LODWORD(v11) = cgArray[0].frametime;
    v12 = (float)((float)v11 * (float)0.001);
    if (v12 == 0.0)
        v12 = 0.015;
    if (cgArray[0].predictedPlayerState.pm_type != 4)
    {
        if (g_mdlprv.system.walkaboutActive
            && Key_IsDown(localClientNum, 16)
            && v10 - g_mdlprv.system.buttonTimes.walkabout > 250)
        {
            g_mdlprv.system.buttonTimes.walkabout = v10;
            g_mdlprv.viewer.freeModeOrigin[0] = cgArray[0].refdef.vieworg[0];
            g_mdlprv.viewer.freeModeOrigin[1] = cgArray[0].refdef.vieworg[1];
            g_mdlprv.system.walkaboutActive = 0;
            g_mdlprv.viewer.freeModeOrigin[2] = cgArray[0].refdef.vieworg[2];
            g_mdlprv.viewer.freeModeAngles[0] = cgArray[0].refdefViewAngles[0];
            g_mdlprv.viewer.freeModeAngles[1] = cgArray[0].refdefViewAngles[1];
            g_mdlprv.viewer.freeModeAngles[2] = cgArray[0].refdefViewAngles[2];
            G_SetPM_MPViewer(1);
        }
        return;
    }
    if (Key_IsDown(localClientNum, 4) && v10 - g_mdlprv.system.buttonTimes.mode > 500)
    {
        g_mdlprv.system.buttonTimes.mode = v10;
        //g_mdlprv.system.uiModeGPad = (_cntlzw(g_mdlprv.system.uiModeGPad - 1) & 0x20) == 0;
        g_mdlprv.system.uiModeGPad = (MdlPrvUiModeGamepad)(g_mdlprv.system.uiModeGPad == 1);
        return;
    }
    if (Key_IsDown(localClientNum, 3) && v10 - g_mdlprv.system.buttonTimes.dropToFloor > 250)
    {
        g_mdlprv.system.buttonTimes.dropToFloor = v10;
        MdlPrvDropToFloor();
        return;
    }
    if (Key_IsDown(localClientNum, 16) && v10 - g_mdlprv.system.buttonTimes.walkabout > 250)
    {
        g_mdlprv.system.buttonTimes.walkabout = v10;
        g_mdlprv.system.walkaboutActive = 1;
        G_SetPM_MPViewer(0);
        Com_sprintf(
            v15,
            256,
            "setviewpos %f %f %f %f\n",
            cgArray[0].refdef.vieworg[0],
            cgArray[0].refdef.vieworg[1],
            cgArray[0].refdef.vieworg[2]
        );
        Cbuf_InsertText(localClientNum, v15);
        return;
    }
    if (g_mdlprv.system.uiModeGPad == MDLPRVMODE_FREE)
    {
        if (Key_IsDown(localClientNum, 5) && v10 - g_mdlprv.system.buttonTimes.ragdollSpeed > 250)
        {
            g_mdlprv.system.buttonTimes.ragdollSpeed = v10;
            MdlPrvRagdollToggle();
        }
        if (Key_IsDown(localClientNum, 6) && v10 - g_mdlprv.system.buttonTimes.freeSpeed > 250)
        {
            g_mdlprv.system.buttonTimes.freeSpeed = v10;
            MdlPrvFreeSpeedToggle();
        }
        if (g_mdlprv.viewer.freeModeSpeed == FREESPEED_SLOW)
        {
            v13 = 20.0;
        }
        else if (g_mdlprv.viewer.freeModeSpeed == FREESPEED_FAST)
        {
            v13 = 1000.0;
        }
        else
        {
            v13 = 300.0;
        }
        if (Key_IsDown(localClientNum, 20) || Key_IsDown(localClientNum, 19))
        {
            v14 = (float)((float)v13 * (float)v12);
        }
        else
        {
            if (!Key_IsDown(localClientNum, 21) && !Key_IsDown(localClientNum, 18))
            {
            LABEL_39:
                MdlPrvFreeMove(
                    cgArray,
                    (float)((float)((float)v13 * (float)v12) * (float)forward),
                    (float)((float)((float)v13 * (float)v12) * (float)side));
                MdlPrvFreeRot(
                    (float)((float)((float)v12 * (float)yaw) * (float)90.0),
                    (float)((float)((float)v12 * (float)pitch) * (float)90.0));
                if (Key_IsDown(localClientNum, 2))
                {
                    MdlPrvModelOriginSet(g_mdlprv.viewer.freeModeOrigin);
                }
                else if (Key_IsDown(localClientNum, 1))
                {
                    MdlPrvFreePlaceModelInFrontCamera(cgArray);
                }
                return;
            }
            v14 = (float)((float)((float)v13 * (float)v12) * (float)-1.0);
        }
        MdlPrvFreeMoveVertical(cgArray, v14);
        goto LABEL_39;
    }
    if (Key_IsDown(localClientNum, 18) && Key_IsDown(localClientNum, 19))
    {
        g_mdlprv.system.focusedMode = FOCUSEDMODE_FOCALMOVE;
        Dvar_SetVec3(
            (dvar_s*)modPrvCenterOffset,
            modPrvCenterOffset->current.value,
            modPrvCenterOffset->current.vector[1],
            (float)-(float)((float)((float)((float)v12 * (float)pitch) * (float)100.0) - modPrvCenterOffset->current.vector[2]));
        MdlPrvMoveFocus2D(
            cgArray,
            (float)((float)((float)v12 * (float)forward) * (float)200.0),
            (float)((float)((float)v12 * (float)side) * (float)200.0));
        if (Key_IsDown(localClientNum, 1))
            MdlPrvMoveFocusReset();
    }
    else if (Key_IsDown(localClientNum, 18))
    {
        g_mdlprv.system.focusedMode = FOCUSEDMODE_MODELMOVE;
        g_mdlprv.model.initialOrigin[2] = g_mdlprv.model.initialOrigin[2]
            + (float)((float)((float)v12 * (float)pitch) * (float)-100.0);
        g_mdlprv.model.currentEntity.placement.base.origin[2] = g_mdlprv.model.currentEntity.placement.base.origin[2]
            + (float)((float)((float)v12 * (float)pitch) * (float)-100.0);
        MdlPrvMoveModel2D(
            cgArray,
            (float)((float)((float)v12 * (float)forward) * (float)200.0),
            (float)((float)((float)v12 * (float)side) * (float)200.0));
    }
    else if (Key_IsDown(localClientNum, 19))
    {
        g_mdlprv.system.focusedMode = FOCUSEDMODE_MODELROTATE;
        if (g_mdlprv.system.modelRotCamMode == MROTCAMMODE_TRAVEL)
        {
            g_mdlprv.viewer.horizontal = -(float)((float)((float)((float)v12 * (float)yaw) * (float)360.0)
                - g_mdlprv.viewer.horizontal);
            MdlPrvSpinYawOffset((float)((float)((float)v12 * (float)yaw) * (float)-360.0));
        }
        else
        {
            MdlPrvSpin_(1u, (float)((float)((float)v12 * (float)yaw) * (float)-360.0));
            MdlPrvSpin_(0, (float)((float)((float)v12 * (float)forward) * (float)-180.0));
            MdlPrvSpin_(2u, (float)((float)((float)v12 * (float)side) * (float)-180.0));
        }
        if (Key_IsDown(localClientNum, 5))
            CG_ModPrvModelResetRotation();
        if (Key_IsDown(localClientNum, 2) && v10 - g_mdlprv.system.buttonTimes.mdlRotMode > 250)
        {
            g_mdlprv.system.buttonTimes.mdlRotMode = v10;
            MdlPrvRotModeToggle();
        }
    }
    else
    {
        g_mdlprv.system.focusedMode = FOCUSEDMODE_CAMERA;
        if (Key_IsDown(localClientNum, 1) && v10 - g_mdlprv.system.buttonTimes.clone > 250)
        {
            g_mdlprv.system.buttonTimes.clone = v10;
            MdlPrvCloneModel(cgArray);
        }
        else if (Key_IsDown(localClientNum, 2) && v10 - g_mdlprv.system.buttonTimes.clearClones > 250)
        {
            g_mdlprv.system.buttonTimes.clearClones = v10;
            MdlPrvCloneClearAll();
        }
        else
        {
            g_mdlprv.viewer.vertical = -(float)((float)((float)((float)v12 * (float)pitch) * (float)-100.0)
                - g_mdlprv.viewer.vertical);
            g_mdlprv.viewer.horizontal = -(float)((float)((float)((float)v12 * (float)yaw) * (float)180.0)
                - g_mdlprv.viewer.horizontal);
            CG_ModelPreviewerZoomCamera((float)((float)((float)v12 * (float)forward) * (float)1800.0), 0.0);
        }
    }
}

// attributes: thunk
void __cdecl CG_ModelPreviewerHandleGamepadEvents(
    int localClientNum,
    double forward,
    double side,
    double pitch,
    double yaw)
{
    MdlPrvControlsGamepad(localClientNum, forward, side, pitch, yaw);
}

void __cdecl CG_ModelPreviewerHandleKeyEvents(int localClientNum, int key, int down, unsigned int time)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].predictedPlayerState.pm_type == 4)
    {
        switch (key)
        {
        case 'e':
            g_mdlprv.system.uiModePC = ROTATE_MODE;
            break;
        case 'q':
            g_mdlprv.system.uiModePC = SELECTION_MODE;
            break;
        case 'w':
            g_mdlprv.system.uiModePC = MOVE_MODE;
            break;
        case 'x':
            CG_ModPrvModelResetRotationXY();
            break;
        case 'z':
            CG_ModPrvModelResetRotation();
            break;
        default:
            return;
        }
    }
}

void __cdecl MdlPrvUpdateViewFocused(float *viewOrigin, float (*viewAxis)[3], float *viewAngles, float *zNear)
{
    const dvar_s *v8; // r11
    double v9; // fp0
    double v10; // fp13
    double v11; // fp12
    double v12; // fp12
    double v13; // fp0
    double v14; // fp13
    double v15; // fp11
    double v16; // fp10
    double v17; // fp9
    double v18; // fp12
    float v19[44]; // [sp+50h] [-1F0h] BYREF
    float v20[16]; // [sp+100h] [-140h] BYREF
    float v21[16]; // [sp+140h] [-100h] BYREF
    float v22[16]; // [sp+180h] [-C0h] BYREF
    float v23[8][4]; // [sp+1C0h] [-80h] BYREF

    if (!zNear)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp", 3112, 0, "%s", "zNear");
    MatrixIdentity44((mat4x4&)v22);
    MatrixIdentity44((mat4x4&)v21);
    MatrixIdentity44((mat4x4&)v20);
    MatrixRotationY((float (*)[3]) & v19[4], g_mdlprv.viewer.vertical);
    MatrixRotationZ((float (*)[3]) & v19[16], -g_mdlprv.viewer.horizontal);
    v21[0] = v19[4];
    v21[1] = v19[5];
    v21[2] = v19[6];
    v20[0] = v19[16];
    v20[1] = v19[17];
    v20[2] = v19[18];
    v21[4] = v19[7];
    v21[5] = v19[8];
    v21[6] = v19[9];
    v20[4] = v19[19];
    v20[5] = v19[20];
    v20[6] = v19[21];
    v21[8] = v19[10];
    v21[9] = v19[11];
    v21[10] = v19[12];
    v20[8] = v19[22];
    v20[9] = v19[23];
    v20[10] = v19[24];
    v22[12] = -g_mdlprv.viewer.centerRadius;
    v22[13] = 0.0;
    v22[14] = 0.0;
    MatrixMultiply44((const mat4x4&)v22, (const mat4x4&)v21, (mat4x4&)v23);
    MatrixMultiply44((const mat4x4&)v23, (const mat4x4&)v20, (mat4x4&)v19[28]);
    if (*(_BYTE *)(modPrvAnimApplyDelta + 12) && g_mdlprv.anim.isAnimPlaying)
    {
        v8 = modPrvCenterOffset;
        v9 = (float)(modPrvCenterOffset->current.value + g_mdlprv.model.initialOrigin[0]);
        v19[0] = modPrvCenterOffset->current.value + g_mdlprv.model.initialOrigin[0];
        v10 = (float)(modPrvCenterOffset->current.vector[1] + g_mdlprv.model.initialOrigin[1]);
        v11 = g_mdlprv.model.initialOrigin[2];
    }
    else
    {
        v8 = modPrvCenterOffset;
        v9 = (float)(modPrvCenterOffset->current.value + g_mdlprv.model.currentEntity.placement.base.origin[0]);
        v19[0] = modPrvCenterOffset->current.value + g_mdlprv.model.currentEntity.placement.base.origin[0];
        v10 = (float)(modPrvCenterOffset->current.vector[1] + g_mdlprv.model.currentEntity.placement.base.origin[1]);
        v11 = g_mdlprv.model.currentEntity.placement.base.origin[2];
    }
    v19[1] = v10;
    v12 = (float)(v8->current.vector[2] + (float)v11);
    *viewOrigin = v19[40] + (float)v9;
    v13 = v19[42];
    viewOrigin[1] = v19[41] + (float)v10;
    v14 = v19[30];
    v19[2] = v12;
    v15 = v19[36];
    v16 = v19[37];
    v17 = v19[38];
    viewOrigin[2] = (float)v13 + (float)v12;
    (*viewAxis)[0] = v19[28];
    (*viewAxis)[1] = v19[29];
    (*viewAxis)[2] = v14;
    (*viewAxis)[3] = v19[32];
    v18 = v19[34];
    (*viewAxis)[4] = v19[33];
    (*viewAxis)[5] = v18;
    (*viewAxis)[6] = v15;
    (*viewAxis)[7] = v16;
    (*viewAxis)[8] = v17;
    AxisToAngles((const mat3x3&)viewAxis, viewAngles);
    if (modPrvDrawAxis->current.enabled || g_mdlprv.system.focusedMode == FOCUSEDMODE_FOCALMOVE)
        CG_ModPrvDrawViewAxis(v19);
    *zNear = g_mdlprv.viewer.zNear;
}

void __cdecl MdlPrvUpdateViewFree(float *viewOrigin, float (*viewAxis)[3], float *viewAngles, float *zNear)
{
    *viewOrigin = g_mdlprv.viewer.freeModeOrigin[0];
    viewOrigin[1] = g_mdlprv.viewer.freeModeOrigin[1];
    viewOrigin[2] = g_mdlprv.viewer.freeModeOrigin[2];
    *viewAngles = g_mdlprv.viewer.freeModeAngles[0];
    viewAngles[1] = g_mdlprv.viewer.freeModeAngles[1];
    viewAngles[2] = g_mdlprv.viewer.freeModeAngles[2];
    AnglesToAxis(viewAngles, viewAxis);
}

void __cdecl CG_ModelPreviewerUpdateView(float *viewOrigin, float (*viewAxis)[3], float *viewAngles, float *zNear)
{
    if (g_mdlprv.system.uiModeGPad)
    {
        *viewOrigin = g_mdlprv.viewer.freeModeOrigin[0];
        viewOrigin[1] = g_mdlprv.viewer.freeModeOrigin[1];
        viewOrigin[2] = g_mdlprv.viewer.freeModeOrigin[2];
        *viewAngles = g_mdlprv.viewer.freeModeAngles[0];
        viewAngles[1] = g_mdlprv.viewer.freeModeAngles[1];
        viewAngles[2] = g_mdlprv.viewer.freeModeAngles[2];
        AnglesToAxis(viewAngles, viewAxis);
    }
    else
    {
        MdlPrvUpdateViewFocused(viewOrigin, viewAxis, viewAngles, zNear);
        g_mdlprv.viewer.freeModeOrigin[0] = *viewOrigin;
        g_mdlprv.viewer.freeModeOrigin[1] = viewOrigin[1];
        g_mdlprv.viewer.freeModeOrigin[2] = viewOrigin[2];
        g_mdlprv.viewer.freeModeAngles[0] = *viewAngles;
        g_mdlprv.viewer.freeModeAngles[1] = viewAngles[1];
        g_mdlprv.viewer.freeModeAngles[2] = viewAngles[2];
    }
}

bool __cdecl CG_ModelPreviewerNeedsVieworgInterpSkipped(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return cgArray[0].predictedPlayerState.pm_type == 4;
}

void __cdecl CG_AddModelPreviewerModel(int frametime)
{
    // KISAKTODO: pending rework to fix stack
#if 0
    double v7; // fp0
    bool isAnimPlaying; // r11
    double v9; // fp30
    bool v10; // r4
    double Radius; // fp1
    int v12; // r29
    float *v13; // r31
    const cpose_t *v14; // r4
    const DObj_s *v15; // r3
    __int64 v16; // [sp+50h] [-70h] BYREF
    float v17; // [sp+58h] [-68h]
    float v18[3][3]; // [sp+60h] [-60h] BYREF

    if (g_mdlprv.model.currentObj && !*(_BYTE *)(modPrvHideModel + 12))
    {
        LODWORD(a7) = frametime;
        v16 = a7;
        v7 = (float)-(float)((float)((float)a7 * (float)0.001) - g_mdlprv.anim.stepCounter);
        g_mdlprv.anim.stepCounter = -(float)((float)((float)a7 * (float)0.001) - g_mdlprv.anim.stepCounter);
        if (v7 >= 0.0)
        {
            isAnimPlaying = g_mdlprv.anim.isAnimPlaying;
        }
        else
        {
            isAnimPlaying = 0;
            g_mdlprv.anim.isAnimPlaying = 0;
        }
        if (isAnimPlaying)
            v9 = (float)((float)(*(float *)(modPrvAnimRate + 12) * (float)a7) * (float)0.001);
        else
            v9 = 0.0;
        if (*(_BYTE *)(modPrvAnimForceLoop + 12))
            CG_ModPrvLoopAnimation();
        CG_ModPrvApplyAnimationBlend(v9);
        CG_ModPrvApplyDelta(v9);
        DObjUpdateClientInfo(g_mdlprv.model.currentObj, v9, v10);
        memset(&g_mdlprv.model.pose, 0, sizeof(g_mdlprv.model.pose));
        g_mdlprv.model.pose.eType = 17;
        g_mdlprv.model.currentEntity.info.pose = &g_mdlprv.model.pose;
        g_mdlprv.model.pose.ragdollHandle = g_mdlprv.model.ragdoll;
        //g_mdlprv.model.pose.isRagdoll = (_cntlzw(g_mdlprv.model.ragdoll) & 0x20) == 0;
        g_mdlprv.model.pose.isRagdoll = g_mdlprv.model.ragdoll;
        UnitQuatToAxis(g_mdlprv.model.currentEntity.placement.base.quat, v18);
        AxisToAngles(v18, g_mdlprv.model.pose.angles);
        g_mdlprv.model.pose.origin[0] = g_mdlprv.model.currentEntity.placement.base.origin[0];
        g_mdlprv.model.pose.origin[1] = g_mdlprv.model.currentEntity.placement.base.origin[1];
        g_mdlprv.model.pose.origin[2] = g_mdlprv.model.currentEntity.placement.base.origin[2];
        if (modPrvDrawBoneInfo->current.integer > 0)
            CG_ModPrvDrawBones();
        *(float *)&v16 = g_mdlprv.model.currentEntity.info.pose->origin[0];
        *((float *)&v16 + 1) = g_mdlprv.model.currentEntity.info.pose->origin[1];
        v17 = g_mdlprv.model.currentEntity.info.pose->origin[2] + (float)4.0;
        R_AddDObjToScene(g_mdlprv.model.currentObj, g_mdlprv.model.currentEntity.info.pose, 0x880u, 0, (float *)&v16, 0.0);
        if (!g_mdlprv.model.inited)
        {
            g_mdlprv.model.inited = 1;
            g_mdlprv.model.currentEntity.cull.mins[0] = 0.0;
            g_mdlprv.model.currentEntity.cull.mins[1] = 0.0;
            g_mdlprv.model.currentEntity.cull.mins[2] = 0.0;
            g_mdlprv.model.currentEntity.cull.maxs[0] = 0.0;
            g_mdlprv.model.currentEntity.cull.maxs[1] = 0.0;
            g_mdlprv.model.currentEntity.cull.maxs[2] = 0.0;
            Radius = DObjGetRadius(g_mdlprv.model.currentObj);
            g_mdlprv.model.currentEntity.cull.mins[0] = g_mdlprv.model.currentEntity.info.pose->origin[0] + (float)-Radius;
            g_mdlprv.model.currentEntity.cull.mins[1] = g_mdlprv.model.currentEntity.info.pose->origin[1] + (float)-Radius;
            g_mdlprv.model.currentEntity.cull.mins[2] = g_mdlprv.model.currentEntity.info.pose->origin[2] + (float)-Radius;
            g_mdlprv.model.currentEntity.cull.maxs[0] = g_mdlprv.model.currentEntity.info.pose->origin[0] + (float)Radius;
            g_mdlprv.model.currentEntity.cull.maxs[1] = g_mdlprv.model.currentEntity.info.pose->origin[1] + (float)Radius;
            g_mdlprv.model.currentEntity.cull.maxs[2] = g_mdlprv.model.currentEntity.info.pose->origin[2] + (float)Radius;
            CG_ModPrvFrameModel();
        }
        v12 = 10;
        v13 = &g_mdlprv.model.clones[0].pose.origin[1];
        do
        {
            if (*((unsigned int *)v13 - 7))
            {
                memset(v13 - 6, 0, 0x54u);
                *((_BYTE *)v13 - 22) = 17;
                *((unsigned int *)v13 - 9) = (unsigned int)v13 - 6;
                UnitQuatToAxis(v13 - 35, v18);
                AxisToAngles(v18, v13 + 2);
                *(v13 - 1) = *(v13 - 31);
                *v13 = *(v13 - 30);
                v13[1] = *(v13 - 29);
                v14 = (const cpose_t *)*((unsigned int *)v13 - 9);
                v15 = (const DObj_s *)*((unsigned int *)v13 - 7);
                *(float *)&v16 = v14->origin[0];
                *((float *)&v16 + 1) = v14->origin[1];
                v17 = v14->origin[2] + (float)4.0;
                R_AddDObjToScene(v15, v14, 0x881u, 0, (float *)&v16, 0.0);
            }
            --v12;
            v13 += 78;
        } while (v12);
    }
#endif
}

void __cdecl CG_ModelPreviewerDestroyDevGui()
{
    Cmd_RemoveCommand("modPrvResetOrientation");
    Cmd_RemoveCommand("modPrvExit");
    CG_ModPrvShutdown();
}

void __cdecl CG_ModelPreviewerBuildInfoStr(char *buffer, int bufferSize)
{
    int modelCount; // r29
    int v5; // r27
    int animCount; // r29
    int v7; // r27
    const char *v8; // r3
    const char *v9; // r3
    const dvar_s *Var; // r3
    const char *v11; // r3
    const dvar_s *v12; // r3
    const char *v13; // r3
    const dvar_s *v14; // r3
    const char *v15; // r3
    const dvar_s *v16; // r3
    const char *v17; // r3
    char v18[320]; // [sp+50h] [-140h] BYREF

    I_strncat(buffer, bufferSize, "#MODELS,");
    modelCount = g_mdlprv.system.modelCount;
    if (g_mdlprv.system.modelCount > 0)
    {
        v5 = 0;
        do
        {
            I_strncat(buffer, bufferSize, g_mdlprv.system.modelNames[v5]);
            I_strncat(buffer, bufferSize, ",");
            --modelCount;
            ++v5;
        } while (modelCount);
    }
    I_strncat(buffer, bufferSize, "#ANIMS,");
    animCount = g_mdlprv.system.animCount;
    if (g_mdlprv.system.animCount > 0)
    {
        v7 = 0;
        do
        {
            I_strncat(buffer, bufferSize, g_mdlprv.system.animNames[v7]);
            I_strncat(buffer, bufferSize, ",");
            --animCount;
            ++v7;
        } while (animCount);
    }
    if (Dvar_FindVar("modPrvLoadModel"))
    {
        I_strncat(buffer, bufferSize, "#DVARS,");
        sprintf(v18, "modPrvLoadModel %s,", g_mdlprv.system.modelNames[modPrvLoadModel->current.integer]);
        I_strncat(buffer, bufferSize, v18);
        sprintf(v18, "modPrvLoadFromAnim %s,", g_mdlprv.system.animNames[modPrvLoadFromAnim->current.integer]);
        I_strncat(buffer, bufferSize, v18);
        sprintf(v18, "modPrvLoadToAnim %s,", g_mdlprv.system.animNames[modPrvLoadToAnim->current.integer]);
        I_strncat(buffer, bufferSize, v18);
        v8 = Dvar_EnumToString(modPrvDrawBoneInfo);
        sprintf(v18, "modPrvDrawBoneInfo %s,", v8);
        I_strncat(buffer, bufferSize, v18);
        //sprintf(v18, "modPrvDrawAxis %i,", (_cntlzw(modPrvDrawAxis->current.color[0]) & 0x20) == 0);
        sprintf(v18, "modPrvDrawAxis %i,", modPrvDrawAxis->current.color[0]);
        I_strncat(buffer, bufferSize, v18);
        sprintf(
            v18,
            "modPrvAnimCrossBlendTime %.2f",
            modPrvAnimCrossBlendTime->current.value);
        I_strncat(buffer, bufferSize, v18);
        sprintf(
            v18,
            "modPrvAnimCrossBlendDuration %.2f",
            modPrvAnimCrossBlendDuration->current.value
        );
        I_strncat(buffer, bufferSize, v18);
        sprintf(
            v18,
            "modPrvAnimRate %.2f",
            modPrvAnimRate->current.value);
        I_strncat(buffer, bufferSize, v18);
        //sprintf(v18, "modPrvAnimApplyDelta %i,", (_cntlzw(*(unsigned __int8 *)(modPrvAnimApplyDelta + 12)) & 0x20) == 0);
        sprintf(v18, "modPrvAnimApplyDelta %i,", modPrvAnimApplyDelta->current.value);
        I_strncat(buffer, bufferSize, v18);
        //sprintf(v18, "modPrvAnimForceLoop %i,", (_cntlzw(*(unsigned __int8 *)(modPrvAnimForceLoop + 12)) & 0x20) == 0);
        sprintf(v18, "modPrvAnimForceLoop %i,", modPrvAnimForceLoop->current.value);
        I_strncat(buffer, bufferSize, v18);
        v9 = Dvar_EnumToString((const dvar_s *)modPrvAnimBlendMode);
        sprintf(v18, "modPrvAnimBlendMode %s,", v9);
        I_strncat(buffer, bufferSize, v18); 
        sprintf(
            v18,
            "modPrvAnimBlendWeight %.2f",
            modPrvAnimBlendWeight->current.value);
        I_strncat(buffer, bufferSize, v18);
        Var = Dvar_FindVar("r_forceLod");
        if (Var)
        {
            v11 = Dvar_EnumToString(Var);
            sprintf(v18, "r_forceLod %s,", v11);
            I_strncat(buffer, bufferSize, v18);
        }
        v12 = Dvar_FindVar("r_colorMap");
        if (v12)
        {
            v13 = Dvar_EnumToString(v12);
            sprintf(v18, "r_colorMap %s,", v13);
            I_strncat(buffer, bufferSize, v18);
        }
        v14 = Dvar_FindVar("r_specularMap");
        if (v14)
        {
            v15 = Dvar_EnumToString(v14);
            sprintf(v18, "r_specularMap %s,", v15);
            I_strncat(buffer, bufferSize, v18);
        }
        v16 = Dvar_FindVar("r_normalMap");
        if (v16)
        {
            v17 = Dvar_EnumToString(v16);
            sprintf(v18, "r_normalMap %s,", v17);
            I_strncat(buffer, bufferSize, v18);
        }
    }
}

void __cdecl CG_ModelPreviewerBuildViewPosStr(char *buffer, int bufferSize)
{
    if (modPrvCenterOffset)
        Com_sprintf(
            buffer,
            bufferSize,
            "%.2f %.2f %.2f %.2f %.2f %.2f",
            g_mdlprv.model.currentEntity.placement.base.origin[0],
            g_mdlprv.model.currentEntity.placement.base.origin[1],
            g_mdlprv.model.currentEntity.placement.base.origin[2],
            g_mdlprv.viewer.centerRadius,
            g_mdlprv.viewer.vertical
        );
    else
        Com_sprintf(buffer, bufferSize, "#ERROR-NotInGame");
}

void __cdecl CG_ModPrvSaveDObjs()
{
    int v0; // r30
    DObj_s **p_obj; // r31

    v0 = 10;
    p_obj = &g_mdlprv.model.clones[0].obj;
    do
    {
        if (*p_obj)
            DObjArchive(*p_obj);
        --v0;
        p_obj += 78;
    } while (v0);
    if (g_mdlprv.model.currentObj)
        DObjArchive(g_mdlprv.model.currentObj);
}

void __cdecl CG_ModPrvLoadDObjs()
{
    int v0; // r30
    DObj_s **p_obj; // r31

    v0 = 10;
    p_obj = &g_mdlprv.model.clones[0].obj;
    do
    {
        if (*p_obj)
            DObjUnarchive(*p_obj);
        --v0;
        p_obj += 78;
    } while (v0);
    if (g_mdlprv.model.currentObj)
        DObjUnarchive(g_mdlprv.model.currentObj);
}

void CG_ModPrvExit_f()
{
    MdlPrvCloneClearAll();
    G_SetPM_MPViewer(0);
    g_mdlprv.system.walkaboutActive = 0;
    CG_ModPrvUnloadModel();
    R_ResetSunLightParseParams();
    DevGui_Toggle();
    DevGui_Reset();
}

void CG_ModPrvRegisterCmds()
{
    Cmd_AddCommandInternal("modPrvResetOrientation", CG_ModPrvResetOrientation_f, &CG_ModPrvResetOrientation_f_VAR);
    Cmd_AddCommandInternal("modPrvExit", CG_ModPrvExit_f, &CG_ModPrvExit_f_VAR);
}

void CG_ModPrvModelGetBoneNameList()
{
    DObj_s *currentObj; // r3
    unsigned int v1; // r28
    int NumModels; // r22
    int v3; // r27
    const XModel *Model; // r3
    int v5; // r26
    int v6; // r30
    const char **v7; // r29
    const char *BoneName; // r3
    const dvar_s *v9; // r3

    currentObj = g_mdlprv.model.currentObj;
    if (!g_mdlprv.model.currentObj)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp",
            971,
            0,
            "%s",
            "g_mdlprv.model.currentObj");
        currentObj = g_mdlprv.model.currentObj;
    }
    CG_ModPrvGetNumTotalBones(currentObj);
    v1 = 4;
    g_mdlprv.model.boneNameTable[0] = mpDefaultDrawBoneOptions[0];
    g_mdlprv.model.boneNameTable[1] = mpDefaultDrawBoneOptions[1];
    g_mdlprv.model.boneNameTable[2] = mpDefaultDrawBoneOptions[2];
    g_mdlprv.model.boneNameTable[3] = mpDefaultDrawBoneOptions[3];
    NumModels = DObjGetNumModels(g_mdlprv.model.currentObj);
    v3 = 0;
    if (NumModels > 0)
    {
        do
        {
            Model = DObjGetModel(g_mdlprv.model.currentObj, v3);
            v5 = XModelNumBones(Model);
            v6 = 0;
            if (v5 > 0)
            {
                v7 = &g_mdlprv.model.boneNameTable[v1];
                do
                {
                    if (v1 >= 0x85)
                        MyAssertHandler(
                            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp",
                            984,
                            0,
                            "%s",
                            "stringIndex < ARRAY_COUNT( g_mdlprv.model.boneNameTable )");
                    BoneName = CG_ModPrvModelGetBoneName(g_mdlprv.model.currentObj, v3, v6++);
                    *v7 = BoneName;
                    ++v1;
                    ++v7;
                } while (v6 < v5);
            }
            ++v3;
        } while (v3 < NumModels);
        if (v1 >= 0x85)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp",
                989,
                0,
                "%s",
                "stringIndex < ARRAY_COUNT( g_mdlprv.model.boneNameTable )");
    }
    v9 = modPrvDrawBoneInfo;
    g_mdlprv.model.boneNameTable[v1] = 0;
    Dvar_UpdateEnumDomain((dvar_s*)v9, g_mdlprv.model.boneNameTable);
}

void __cdecl CG_ModPrvLoadModel(const cg_s *cgameGlob, const char *modelFilename)
{
#if 0 // KISAKTODO: particularlly nasty
    __int64 v43; // r4
    int v44; // r30
    const char *v45; // r28
    double v46; // fp30
    double v47; // fp29
    double v48; // fp31
    double v49; // fp30
    long double v50; // fp2
    long double v51; // fp2
    double v52; // fp13
    int NumSurfaces; // r3
    int v54; // r30
    int v55; // r24
    int surfaceCount; // r9
    int v57; // r23
    int v58; // r26
    Material *v59; // r27
    char v60; // r8
    int v61; // r11
    Material **v62; // r10
    const dvar_s *v63; // r3
    int v64; // r8
    int v65; // r30
    int v66; // r25
    int v67; // r27
    int v68; // r26
    Material *v69; // r28
    char v70; // r9
    int v71; // r11
    Material **v72; // r10

    v43 = ((__int64 (*)(void))RtlCheckStack12)();
    v44 = HIDWORD(v43);
    v45 = (const char *)v43;
    if (g_mdlprv.model.currentObj)
    {
        v46 = g_mdlprv.model.initialOrigin[0];
        v47 = g_mdlprv.model.initialOrigin[1];
        v48 = g_mdlprv.model.initialOrigin[2];
    }
    else
    {
        v46 = (float)((float)(*(float *)(HIDWORD(v43) + 154368) * g_mdlprv.viewer.centerRadius)
            + *(float *)(HIDWORD(v43) + 154356));
        v48 = *(float *)(HIDWORD(v43) + 108092);
        v47 = (float)((float)(*(float *)(HIDWORD(v43) + 154372) * g_mdlprv.viewer.centerRadius)
            + *(float *)(HIDWORD(v43) + 154360));
        G_SetPM_MPViewer(1);
    }
    CG_ModPrvUnloadModel();
    g_mdlprv.system.gamePadRStickDeflect = gpad_button_rstick_deflect_max->current.value;
    Dvar_SetFloat(gpad_button_rstick_deflect_max, 1.0);
    R_SetIgnorePrecacheErrors(1);
    R_RegisterModel(v45);
    R_SetIgnorePrecacheErrors(0);
    HIWORD(a36) = 0;
    a35 = R_RegisterModel(v45);
    BYTE2(a36) = 0;
    if (!a35)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp", 1031, 0, "%s", "dobjModel.model");
    DObjCreate((DObjModel_s *)&a35, 1u, 0, g_mdlprv.model.objBuf, 0);
    g_mdlprv.model.currentEntity.gfxEntIndex = 0;
    g_mdlprv.model.currentEntity.placement.scale = 1.0;
    g_mdlprv.model.inited = 0;
    g_mdlprv.model.currentObj = (DObj_s *)g_mdlprv.model.objBuf;
    CG_ModPrvModelGetBoneNameList();
    g_mdlprv.model.initialOrigin[0] = v46;
    g_mdlprv.model.initialOrigin[1] = v47;
    g_mdlprv.model.initialOrigin[2] = v48;
    g_mdlprv.model.currentEntity.placement.base.origin[0] = v46;
    g_mdlprv.model.currentEntity.placement.base.origin[1] = v47;
    g_mdlprv.model.currentEntity.placement.base.origin[2] = v48;
    Dvar_SetVec3((dvar_s*)modPrvOrigin, 0.0, 0.0, 0.0);
    v49 = (float)(*(float *)(v44 + 170872) * (float)0.0027777778);
    *(double *)&v50 = (float)((float)(*(float *)(v44 + 170872) * (float)0.0027777778) + (float)0.5);
    v51 = floor(v50);
    g_mdlprv.model.initialYaw = (float)((float)v49 - (float)*(double *)&v51) * (float)360.0;
    Dvar_SetVec3((dvar_s*)modPrvCenterOffset, 0.0, 0.0, 0.0);
    Dvar_SetVec3((dvar_s*)modPrvRotationAngles, 0.0, g_mdlprv.model.initialYaw, 0.0);
    value = modPrvRotationAngles->current.value;
    v52 = (float)(g_mdlprv.anim.deltaYaw + modPrvRotationAngles->current.vector[1]);
    a38 = modPrvRotationAngles->current.vector[1];
    a39 = modPrvRotationAngles->current.vector[2];
    a38 = (float)v52 + (float)180.0;
    AnglesToQuat(&value, g_mdlprv.model.currentEntity.placement.base.quat);
    Dvar_Reset(modPrvLod, DVAR_SOURCE_INTERNAL);
    CG_ModPrvPushMruEntry(v45, g_mdlprv.model.mruNames, g_mdlprv.model.mruNameTable, modPrvModelMru);
    g_mdlprv.anim.fromCurrentIndex = -1;
    g_mdlprv.anim.toCurrentIndex = -1;
    Dvar_Reset((const dvar_s *)modPrvAnimRate, DVAR_SOURCE_INTERNAL);
    Dvar_Reset(modPrvAnimBlendWeight, DVAR_SOURCE_INTERNAL);
    Dvar_Reset((const dvar_s *)modPrvAnimBlendMode, DVAR_SOURCE_INTERNAL);
    Dvar_Reset(modPrvAnimCrossBlendDuration, DVAR_SOURCE_INTERNAL);
    Dvar_Reset(modPrvAnimCrossBlendTime, DVAR_SOURCE_INTERNAL);
    Dvar_Reset((const dvar_s *)modPrvAnimForceLoop, DVAR_SOURCE_INTERNAL);
    NumSurfaces = CG_ModPrvGetNumSurfaces(g_mdlprv.model.currentObj, modPrvLod->current.integer);
    g_mdlprv.model.surfaceCount = NumSurfaces;
    if (NumSurfaces > 0)
    {
        g_mdlprv.mat.surfMatHandles = (Material **)Z_VirtualAlloc(4 * NumSurfaces, "MODPRV_MaterialHandles", 0);
        R_DObjGetSurfMaterials(g_mdlprv.model.currentObj, modPrvLod->current.integer, g_mdlprv.mat.surfMatHandles);
        v54 = 0;
        v55 = 0;
        g_mdlprv.mat.nameTable[0] = "<None>";
        surfaceCount = g_mdlprv.model.surfaceCount;
        if (g_mdlprv.model.surfaceCount > 0)
        {
            v57 = 0;
            v58 = 0;
            do
            {
                v59 = g_mdlprv.mat.surfMatHandles[v57];
                if (v59)
                {
                    v60 = 0;
                    v61 = 0;
                    if (v54 > 0)
                    {
                        v62 = (Material **)&a43;
                        while (*v62 != v59)
                        {
                            ++v61;
                            ++v62;
                            if (v61 >= v54)
                                goto LABEL_16;
                        }
                        v60 = 1;
                    }
                LABEL_16:
                    if (!v60)
                    {
                        if ((unsigned int)(v54 + 1) >= 0x42)
                            MyAssertHandler(
                                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp",
                                1091,
                                0,
                                "%s",
                                "uniqueMatCount + 1 < ARRAY_COUNT( g_mdlprv.mat.nameTable )");
                        g_mdlprv.mat.nameTable[v58 + 1] = Material_GetName(v59);
                        if ((unsigned int)v54 >= 0x2200)
                            MyAssertHandler(
                                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp",
                                1094,
                                0,
                                "%s",
                                "uniqueMatCount < ARRAY_COUNT( uniqueHash )");
                        surfaceCount = g_mdlprv.model.surfaceCount;
                        ++v54;
                        *(int *)((char *)&a43 + v58 * 4) = (int)v59;
                        ++v58;
                    }
                }
                ++v55;
                ++v57;
            } while (v55 < surfaceCount);
        }
        v63 = modPrvMatSelect;
        g_mdlprv.mat.nameTable[v54 + 1] = 0;
        Dvar_UpdateEnumDomain((dvar_s*)v63, g_mdlprv.mat.nameTable);
        Dvar_UpdateEnumDomain((dvar_s*)modPrvMatReplace, g_mdlprv.mat.nameTable);
        g_mdlprv.mat.handleCount = v54;
        g_mdlprv.mat.handleArray = (Material **)Z_VirtualAlloc(4 * v54, "MODPRV_MaterialHandles", 0);
        v64 = g_mdlprv.model.surfaceCount;
        v65 = 0;
        v66 = 0;
        if (g_mdlprv.model.surfaceCount > 0)
        {
            v67 = 0;
            v68 = 0;
            do
            {
                v69 = g_mdlprv.mat.surfMatHandles[v68];
                if (v69)
                {
                    v70 = 0;
                    v71 = 0;
                    if (v65 > 0)
                    {
                        v72 = (Material **)&a43;
                        while (*v72 != v69)
                        {
                            ++v71;
                            ++v72;
                            if (v71 >= v65)
                                goto LABEL_32;
                        }
                        v70 = 1;
                    }
                LABEL_32:
                    if (!v70)
                    {
                        if (v65 >= g_mdlprv.mat.handleCount)
                            MyAssertHandler(
                                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp",
                                1126,
                                0,
                                "%s",
                                "uniqueMatCount < g_mdlprv.mat.handleCount");
                        g_mdlprv.mat.handleArray[v67] = g_mdlprv.mat.surfMatHandles[v68];
                        if ((unsigned int)v65 >= 0x2200)
                            MyAssertHandler(
                                "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_modelpreviewer.cpp",
                                1129,
                                0,
                                "%s",
                                "uniqueMatCount < ARRAY_COUNT( uniqueHash )");
                        v64 = g_mdlprv.model.surfaceCount;
                        ++v65;
                        *(int *)((char *)&a43 + v67 * 4) = (int)v69;
                        ++v67;
                    }
                }
                ++v66;
                ++v68;
            } while (v66 < v64);
        }
    }
    Dvar_Reset(modPrvMatSelect, DVAR_SOURCE_INTERNAL);
    g_mdlprv.mat.selectSliderIndex = 0;
    g_mdlprv.mat.replaceIndex = -1;
    g_mdlprv.mat.replaceSliderIndex = 0;
#endif
}

void __cdecl CG_ModPrvModelRecentAccepted(const cg_s *cgameGlob)
{
    const char *v2; // r3
    double v3; // fp8
    double v4; // fp7
    double v5; // fp6
    double v6; // fp5
    double v7; // fp4
    double v8; // fp3
    double v9; // fp2
    double v10; // fp1
    int v11; // r30
    const char **modelNames; // r7
    const char *v13; // r10
    const char *v14; // r11
    int v15; // r9
    int v16; // r8
    int v17; // [sp+8h] [-78h]
    int v18; // [sp+Ch] [-74h]
    int v19; // [sp+10h] [-70h]
    int v20; // [sp+14h] [-6Ch]
    int v21; // [sp+18h] [-68h]
    int v22; // [sp+1Ch] [-64h]
    int v23; // [sp+20h] [-60h]
    int v24; // [sp+24h] [-5Ch]
    int v25; // [sp+28h] [-58h]
    int v26; // [sp+2Ch] [-54h]
    int v27; // [sp+30h] [-50h]
    int v28; // [sp+34h] [-4Ch]
    int v29; // [sp+38h] [-48h]
    int v30; // [sp+3Ch] [-44h]
    int v31; // [sp+40h] [-40h]
    int v32; // [sp+44h] [-3Ch]
    int v33; // [sp+48h] [-38h]
    int v34; // [sp+4Ch] [-34h]
    XModel *v35; // [sp+50h] [-30h]
    int v36; // [sp+54h] [-2Ch]
    float v37; // [sp+58h] [-28h]
    float v38; // [sp+60h] [-20h]
    float v39; // [sp+68h] [-18h]
    int v40; // [sp+70h] [-10h]
    int v41; // [sp+74h] [-Ch]
    int v42; // [sp+78h] [-8h]
    int v43; // [sp+7Ch] [-4h]

    Dvar_ClearModified((dvar_s*)modPrvModelMru);
    v2 = Dvar_EnumToString(modPrvModelMru);
    v11 = 0;
    if (g_mdlprv.system.modelCount <= 0)
    {
    LABEL_8:
        Com_Printf(14, "Model previewer could not load <%s> because it does not exist.\n ", v2);
        CG_ModPrvRemoveMruEntry(g_mdlprv.model.mruNames, g_mdlprv.model.mruNameTable, modPrvModelMru);
    }
    else
    {
        modelNames = g_mdlprv.system.modelNames;
        while (1)
        {
            v13 = *modelNames;
            v14 = v2;
            do
            {
                v15 = *(unsigned __int8 *)v14;
                v16 = v15 - *(unsigned __int8 *)v13;
                if (!*v14)
                    break;
                ++v14;
                ++v13;
            } while (!v16);
            if (!v16)
                break;
            ++v11;
            ++modelNames;
            if (v11 >= g_mdlprv.system.modelCount)
                goto LABEL_8;
        }
        CG_ModPrvLoadModel(cgameGlob, v2);
        g_mdlprv.model.currentIndex = v11;
    }
}

static void CG_ModPrvModelLoadAccepted(const cg_s *cgameGlob)
{
    Dvar_ClearModified((dvar_s*)modPrvLoadModel);
    CG_ModPrvLoadModel(cgameGlob, g_mdlprv.system.modelNames[modPrvLoadModel->current.integer]);
    g_mdlprv.model.currentIndex = modPrvLoadModel->current.integer;
}

static void CG_ModPrvModelLoadUpdate(const cg_s *cgameGlob)
{
    int integer; // r11

    if (g_mdlprv.system.cachedAllModels)
    {
        integer = modPrvLoadModel->latched.integer;
        if (integer != g_mdlprv.system.lastLoadModel)
        {
            CG_ModPrvLoadModel(cgameGlob, g_mdlprv.system.modelNames[integer]);
            g_mdlprv.system.lastLoadModel = modPrvLoadModel->latched.integer;
        }
    }
}

void __cdecl CG_ModelPreviewerFrame(const cg_s *cgameGlob)
{
    if (modPrvLoadModel->modified)
        CG_ModPrvModelLoadAccepted(cgameGlob);
    else
        CG_ModPrvModelLoadUpdate(cgameGlob);
    if (modPrvModelMru->modified)
        CG_ModPrvModelRecentAccepted(cgameGlob);
    if (modPrvOrigin->modified)
        CG_ModPrvOriginUpdate();
    if (modPrvRotationAngles->modified)
        CG_ModPrvRotateUpdate();
    if (modPrvFromAnimMru->modified)
        CG_ModPrvAnimRecentAccept(modPrvFromAnimMru, &g_mdlprv.anim.fromCurrentIndex);
    if (modPrvToAnimMru->modified)
        CG_ModPrvAnimRecentAccept(modPrvToAnimMru, &g_mdlprv.anim.toCurrentIndex);
    if (modPrvLoadFromAnim->modified)
        CG_ModPrvLoadAnimAccept(modPrvLoadFromAnim, &g_mdlprv.anim.fromCurrentIndex);
    if (modPrvLoadToAnim->modified)
        CG_ModPrvLoadAnimAccept(modPrvLoadToAnim, &g_mdlprv.anim.toCurrentIndex);
    if (modPrvAnimCrossBlendTime->modified || modPrvAnimCrossBlendDuration->modified)
    {
        Dvar_ClearModified((dvar_s*)modPrvAnimCrossBlendTime);
        Dvar_ClearModified((dvar_s*)modPrvAnimCrossBlendDuration);
        CG_ModPrvLoadAnimations(0);
    }
    if (modPrvAnimBlendWeight->modified)
        CG_ModPrvAnimBlendWeightUpdate();
    if (modPrvMatReplace->modified)
        CG_ModPrvMatReplaceAccepted();
    else
        CG_ModPrvMatReplaceUpdate();
    if (modPrvLightSetup->modified)
        CG_ModPrvLightSetupModified();
    if (CG_ModPrvAnyLightValuesChanged())
        CG_ModPrvLightValuesUpdate();
}

void CG_ModPrvEnumerateModels_FastFile()
{
    HunkUser *v0; // r29
    unsigned int v1[12]; // [sp+50h] [-30h] BYREF

    if (g_mdlprv.system.modelNames)
        Dvar_UpdateEnumDomain((dvar_s*)modPrvLoadModel, g_emptyEnumList);
    g_mdlprv.system.modelCount = DB_GetAllXAssetOfType(ASSET_TYPE_XMODEL, 0, 0x7FFFFFFF);
    if (g_mdlprv.system.modelCount)
    {
        v0 = Hunk_UserCreate(0x20000, "CG_ModPrvEnumerateModels", 0, 0, 0);
        g_mdlprv.system.modelNames = (const char **)Hunk_UserAlloc(v0, 4 * (g_mdlprv.system.modelCount + 2), 4);
        *g_mdlprv.system.modelNames = (const char *)v0;
        ++g_mdlprv.system.modelNames;
        v1[2] = (int)"CG_ModPrvEnumerateModels";
        v1[1] = 3;
        v1[0] = 0;
        DB_EnumXAssets(
            ASSET_TYPE_XMODEL,
            (void(__cdecl *)(XAssetHeader, void *))CG_ModPrvGetAssetName,
            v1,
            0);

        //std::_Sort<int *, int, bool(__cdecl *)(int, int)>(
        //    (Material **)g_mdlprv.system.modelNames,
        //    (Material **)&g_mdlprv.system.modelNames[g_mdlprv.system.modelCount],
        //    (4 * g_mdlprv.system.modelCount) >> 2,
        //    (bool(__cdecl *)(const Material *, const Material *))CG_ModPrvCompareString);
        //std::sort(g_mdlprv.system.modelNames, &g_mdlprv.system.modelNames[g_mdlprv.system.modelCount], CG_ModPrvCompareString); // KISAKTODO: fix sorting of model names here, std::sort with raw pointers is annoying
    }
}

// attributes: thunk
void CG_ModPrvEnumerateModels()
{
    CG_ModPrvEnumerateModels_FastFile();
}

void CG_ModPrvEnumerateAnimations_FastFile()
{
    HunkUser *v0; // r29
    unsigned int v1[12]; // [sp+50h] [-30h] BYREF

    if (g_mdlprv.system.animNames)
    {
        Dvar_UpdateEnumDomain((dvar_s*)modPrvLoadFromAnim, g_emptyEnumList);
        Dvar_UpdateEnumDomain((dvar_s*)modPrvLoadToAnim, g_emptyEnumList);
    }
    g_mdlprv.system.animCount = DB_GetAllXAssetOfType(ASSET_TYPE_XANIMPARTS, 0, 0x7FFFFFFF);
    if (g_mdlprv.system.animCount)
    {
        v0 = Hunk_UserCreate(0x20000, "CG_ModPrvEnumerateAnimations", 0, 0, 0);
        g_mdlprv.system.animNames = (const char **)Hunk_UserAlloc(v0, 4 * (g_mdlprv.system.animCount + 2), 4);
        *g_mdlprv.system.animNames = (const char *)v0;
        v1[2] = (int)"CG_ModPrvEnumerateAnimations";
        ++g_mdlprv.system.animNames;
        v1[1] = 2;
        v1[0] = 0;
        DB_EnumXAssets(
            ASSET_TYPE_XANIMPARTS,
            (void(__cdecl *)(XAssetHeader, void *))CG_ModPrvGetAssetName,
            v1,
            0);
        //std::_Sort<int *, int, bool(__cdecl *)(int, int)>(
        //    (Material **)g_mdlprv.system.animNames,
        //    (Material **)&g_mdlprv.system.animNames[g_mdlprv.system.animCount],
        //    (4 * g_mdlprv.system.animCount) >> 2,
        //    (bool(__cdecl *)(const Material *, const Material *))CG_ModPrvCompareString);

        //std::sort(g_mdlprv.system.animNames, &g_mdlprv.system.animNames[g_mdlprv.system.animCount], CG_ModPrvCompareString); // KISAKTODO: fix sorting of model names here, std::sort with raw pointers is annoying

    }
}

// attributes: thunk
void CG_ModPrvEnumerateAnimations()
{
    CG_ModPrvEnumerateAnimations_FastFile();
}

void __cdecl CG_ModelPreviewerEnumerateAssets()
{
    if (g_mdlprv.inited)
    {
        CG_ModPrvEnumerateModels_FastFile();
        Dvar_UpdateEnumDomain((dvar_s*)modPrvLoadModel, g_mdlprv.system.modelNames);
        CG_ModPrvEnumerateAnimations_FastFile();
        Dvar_UpdateEnumDomain((dvar_s*)modPrvLoadFromAnim, g_mdlprv.system.animNames);
        Dvar_UpdateEnumDomain((dvar_s*)modPrvLoadToAnim, g_mdlprv.system.animNames);
    }
}

void __cdecl CG_ModPrvStartup(int localClientNum)
{
    CG_ModPrvEnumerateModels_FastFile();
    CG_ModPrvEnumerateAnimations_FastFile();
    CG_ModPrvRegisterDvars();
    Cmd_AddCommandInternal("modPrvResetOrientation", CG_ModPrvResetOrientation_f, &CG_ModPrvResetOrientation_f_VAR);
    Cmd_AddCommandInternal("modPrvExit", CG_ModPrvExit_f, &CG_ModPrvExit_f_VAR);
    g_mdlprv.inited = 1;
}

void __cdecl CG_ModelPreviewerCreateDevGui(int localClientNum)
{
    memset(&g_mdlprv, 0, sizeof(g_mdlprv));
    CG_ModPrvEnumerateModels_FastFile();
    CG_ModPrvEnumerateAnimations_FastFile();
    CG_ModPrvRegisterDvars();
    Cmd_AddCommandInternal("modPrvResetOrientation", CG_ModPrvResetOrientation_f, &CG_ModPrvResetOrientation_f_VAR);
    Cmd_AddCommandInternal("modPrvExit", CG_ModPrvExit_f, &CG_ModPrvExit_f_VAR);
    g_mdlprv.inited = 1;
    CG_ModPrvResetGlobals();
    Cbuf_InsertText(localClientNum, "exec devgui_modelpreviewer\n");
}

