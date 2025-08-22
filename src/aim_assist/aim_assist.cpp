#include "aim_assist.h"
#include <qcommon/mem_track.h>
#include <universal/com_math.h>
#include <qcommon/cmd.h>
#include <client/client.h>
#include <script/scr_const.h>
#include <universal/profile.h>

#ifdef KISAK_MP
#include <cgame_mp/cg_local_mp.h>
#include <client_mp/client_mp.h>
#elif KISAK_SP
#include <cgame/cg_main.h>
#include <cgame/cg_ents.h>
#include <xanim/xanim.h>
#endif

//float const *const vec2_origin        82000d78     aim_assist.obj
//float const *const vec3_origin        82000d80     aim_assist.obj
//float const *const vec4_origin        82000d8c     aim_assist.obj

AimAssistGlobals aaGlobArray[1];
GraphFloat aaInputGraph[4];

const dvar_t *aim_input_graph_enabled;
const dvar_t *aim_input_graph_debug;
const dvar_t *aim_input_graph_index;
const dvar_t *aim_turnrate_pitch;
const dvar_t *aim_turnrate_pitch_ads;
const dvar_t *aim_turnrate_yaw;
const dvar_t *aim_turnrate_yaw_ads;
const dvar_t *aim_accel_turnrate_enabled;
const dvar_t *aim_accel_turnrate_debug;
const dvar_t *aim_accel_turnrate_lerp;
const dvar_t *aim_slowdown_enabled;
const dvar_t *aim_slowdown_debug;
const dvar_t *aim_slowdown_region_width;
const dvar_t *aim_slowdown_region_height;
const dvar_t *aim_slowdown_pitch_scale;
const dvar_t *aim_slowdown_pitch_scale_ads;
const dvar_t *aim_slowdown_yaw_scale;
const dvar_t *aim_slowdown_yaw_scale_ads;
const dvar_t *aim_autoaim_enabled;
const dvar_t *aim_autoaim_debug;
const dvar_t *aim_autoaim_lerp;
const dvar_t *aim_autoaim_region_width;
const dvar_t *aim_autoaim_region_height;
const dvar_t *aim_automelee_enabled;
const dvar_t *aim_automelee_debug;
const dvar_t *aim_automelee_lerp;
const dvar_t *aim_automelee_region_width;
const dvar_t *aim_automelee_region_height;
const dvar_t *aim_automelee_range;
const dvar_t *aim_lockon_enabled;
const dvar_t *aim_lockon_debug;
const dvar_t *aim_lockon_deflection;
const dvar_t *aim_lockon_strength;
const dvar_t *aim_lockon_region_width;
const dvar_t *aim_lockon_region_height;
const dvar_t *aim_scale_view_axis;

void __cdecl TRACK_aim_assist()
{
    TRACK_STATIC_ARR(aaGlobArray, 10);
    TRACK_STATIC_ARR(aaInputGraph, 10);
}

void __cdecl AimAssist_Init(int32_t localClientNum)
{
    char graphName[128]; // [esp+4h] [ebp-88h] BYREF
    int graphIndex; // [esp+88h] [ebp-4h]

    memset((uint8_t*)&aaGlobArray[localClientNum], 0, sizeof(AimAssistGlobals));
    AimAssist_RegisterDvars();
    for (graphIndex = 0; graphIndex < 4; ++graphIndex)
    {
        Com_sprintf(graphName, 0x80u, "aim_assist/view_input_%d.graph", graphIndex);
        GraphFloat_Load(&aaInputGraph[graphIndex], graphName, 1.0);
    }
    Cbuf_InsertText(0, "exec devgui_aimassist\n");
    for (graphIndex = 0; graphIndex < 4; ++graphIndex)
    {
        Com_sprintf(graphName, 0x80u, "Aim Assist/Input Graph:1/Graph %d", graphIndex);
        GraphFloat_CreateDevGui(&aaInputGraph[graphIndex], graphName);
    }
}

void AimAssist_RegisterDvars()
{
    DvarLimits min; // [esp+4h] [ebp-14h]
    DvarLimits mina; // [esp+4h] [ebp-14h]
    DvarLimits minb; // [esp+4h] [ebp-14h]
    DvarLimits minc; // [esp+4h] [ebp-14h]
    DvarLimits mind; // [esp+4h] [ebp-14h]
    DvarLimits mine; // [esp+4h] [ebp-14h]
    DvarLimits minf; // [esp+4h] [ebp-14h]
    DvarLimits ming; // [esp+4h] [ebp-14h]
    DvarLimits minh; // [esp+4h] [ebp-14h]
    DvarLimits mini; // [esp+4h] [ebp-14h]
    DvarLimits minj; // [esp+4h] [ebp-14h]
    DvarLimits mink; // [esp+4h] [ebp-14h]
    DvarLimits minl; // [esp+4h] [ebp-14h]
    DvarLimits minm; // [esp+4h] [ebp-14h]
    DvarLimits minn; // [esp+4h] [ebp-14h]
    DvarLimits mino; // [esp+4h] [ebp-14h]
    DvarLimits minp; // [esp+4h] [ebp-14h]
    DvarLimits minq; // [esp+4h] [ebp-14h]
    DvarLimits minr; // [esp+4h] [ebp-14h]
    DvarLimits mins; // [esp+4h] [ebp-14h]
    DvarLimits mint; // [esp+4h] [ebp-14h]
    DvarLimits minu; // [esp+4h] [ebp-14h]

    aim_input_graph_enabled = Dvar_RegisterBool("aim_input_graph_enabled", 1, DVAR_CHEAT, "Use graph for adjusting view input");
    aim_input_graph_debug = Dvar_RegisterBool("aim_input_graph_debug", 0, DVAR_CHEAT, "Debug the view input graphs");
    aim_input_graph_index = Dvar_RegisterInt(
        "aim_input_graph_index",
        3,
        (DvarLimits)0x300000000LL,
        DVAR_CHEAT,
        "Which input graph to use");
    min.value.max = 1080.0f;
    min.value.min = 0.0f;
    aim_turnrate_pitch = Dvar_RegisterFloat(
        "aim_turnrate_pitch",
        90.0f,
        min,
        DVAR_CHEAT,
        "The vertical turn rate for aim assist when firing from the hip");
    mina.value.max = 1080.0f;
    mina.value.min = 0.0f;
    aim_turnrate_pitch_ads = Dvar_RegisterFloat(
        "aim_turnrate_pitch_ads",
        55.0f,
        mina,
        DVAR_CHEAT,
        "The turn rate up and down for aim assist when aiming down the sight");
    minb.value.max = 1080.0f;
    minb.value.min = 0.0f;
    aim_turnrate_yaw = Dvar_RegisterFloat(
        "aim_turnrate_yaw",
        260.0f,
        minb,
        DVAR_CHEAT,
        "The horizontal turn rate for aim assist when firing from the hip");
    minc.value.max = 1080.0f;
    minc.value.min = 0.0f;
    aim_turnrate_yaw_ads = Dvar_RegisterFloat(
        "aim_turnrate_yaw_ads",
        90.0f,
        minc,
        DVAR_CHEAT,
        "The horizontal turn rate for aim assist when aiming down the sight");
    aim_accel_turnrate_enabled = Dvar_RegisterBool(
        "aim_accel_turnrate_enabled",
        1,
        DVAR_CHEAT,
        "Enable/disable acceleration of the turnrates");
    aim_accel_turnrate_debug = Dvar_RegisterBool(
        "aim_accel_turnrate_debug",
        0,
        DVAR_CHEAT,
        "Turn on debugging info for the acceleration");
    mind.value.max = 4000.0f;
    mind.value.min = 0.0f;
    aim_accel_turnrate_lerp = Dvar_RegisterFloat(
        "aim_accel_turnrate_lerp",
        1200.0f,
        mind,
        DVAR_CHEAT,
        "The acceleration of the turnrates");
    aim_slowdown_enabled = Dvar_RegisterBool(
        "aim_slowdown_enabled",
        1,
        DVAR_CHEAT,
        "Slowdown the turn rate when the cross hair passes over a target");
    aim_slowdown_debug = Dvar_RegisterBool("aim_slowdown_debug", 0, 0x80u, "Turn on debugging info for aim slowdown");
    mine.value.max = 640.0f;
    mine.value.min = 0.0f;
    aim_slowdown_region_width = Dvar_RegisterFloat(
        "aim_slowdown_region_width",
        90.0f,
        mine,
        DVAR_CHEAT,
        "The screen width of the aim slowdown region");
    minf.value.max = 480.0f;
    minf.value.min = 0.0f;
    aim_slowdown_region_height = Dvar_RegisterFloat(
        "aim_slowdown_region_height",
        90.0f,
        minf,
        DVAR_CHEAT,
        "The screen height of the aim assist slowdown region");
    ming.value.max = 1.0f;
    ming.value.min = 0.0f;
    aim_slowdown_pitch_scale = Dvar_RegisterFloat(
        "aim_slowdown_pitch_scale",
        0.40000001f,
        ming,
        DVAR_CHEAT,
        "The vertical aim assist slowdown ratio from the hip");
    minh.value.max = 1.0f;
    minh.value.min = 0.0f;
    aim_slowdown_pitch_scale_ads = Dvar_RegisterFloat(
        "aim_slowdown_pitch_scale_ads",
        0.5f,
        minh,
        DVAR_CHEAT,
        "The vertical aim assist slowdown ratio when aiming down the sight");
    mini.value.max = 1.0f;
    mini.value.min = 0.0f;
    aim_slowdown_yaw_scale = Dvar_RegisterFloat(
        "aim_slowdown_yaw_scale",
        0.40000001f,
        mini,
        DVAR_CHEAT,
        "The horizontal aim assist slowdown ratio from the hip");
    minj.value.max = 1.0f;
    minj.value.min = 0.0f;
    aim_slowdown_yaw_scale_ads = Dvar_RegisterFloat(
        "aim_slowdown_yaw_scale_ads",
        0.5f,
        minj,
        DVAR_CHEAT,
        "The horizontal aim assist slowdown ratio when aiming down the sight");
    aim_autoaim_enabled = Dvar_RegisterBool("aim_autoaim_enabled", 0, 0x80u, "Turn on auto aim");
    aim_autoaim_debug = Dvar_RegisterBool("aim_autoaim_debug", 0, 0x80u, "Turn on auto aim debugging");
    mink.value.max = 100.0f;
    mink.value.min = 0.0f;
    aim_autoaim_lerp = Dvar_RegisterFloat(
        "aim_autoaim_lerp",
        40.0f,
        mink,
        DVAR_CHEAT,
        "The rate in degrees per second that the auto aim will converge to its target");
    minl.value.max = 640.0f;
    minl.value.min = 0.0f;
    aim_autoaim_region_width = Dvar_RegisterFloat(
        "aim_autoaim_region_width",
        160.0f,
        minl,
        DVAR_CHEAT,
        "The width of the auto aim region in virtual screen coordinates (0 - 640)");
    minm.value.max = 480.0f;
    minm.value.min = 0.0f;
    aim_autoaim_region_height = Dvar_RegisterFloat(
        "aim_autoaim_region_height",
        120.0f,
        minm,
        DVAR_CHEAT,
        "The height of the auto aim region in virtual screen coordinates (0 - 480)");
    aim_automelee_enabled = Dvar_RegisterBool("aim_automelee_enabled", 1, 0x80u, "Turn on auto melee");
    aim_automelee_debug = Dvar_RegisterBool("aim_automelee_debug", 0, 0x80u, "Turn on auto melee debugging");
    minn.value.max = 100.0f;
    minn.value.min = 0.0f;
    aim_automelee_lerp = Dvar_RegisterFloat(
        "aim_automelee_lerp",
        40.0f,
        minn,
        DVAR_CHEAT,
        "The rate in degrees per second that the auto melee will converge to its target");
    mino.value.max = 640.0f;
    mino.value.min = 0.0f;
    aim_automelee_region_width = Dvar_RegisterFloat(
        "aim_automelee_region_width",
        320.0f,
        mino,
        DVAR_CHEAT,
        "The width of the auto melee region in virtual screen coordinates (0 - 640)");
    minp.value.max = 480.0f;
    minp.value.min = 0.0f;
    aim_automelee_region_height = Dvar_RegisterFloat(
        "aim_automelee_region_height",
        240.0f,
        minp,
        DVAR_CHEAT,
        "The height of the auto melee region in virtual screen coordinates (0 - 480)");
    minq.value.max = 255.0f;
    minq.value.min = 0.0f;
    aim_automelee_range = Dvar_RegisterFloat("aim_automelee_range", 128.0f, minq, 0x80u, "The range of the auto melee");
    aim_lockon_enabled = Dvar_RegisterBool(
        "aim_lockon_enabled",
        1,
        DVAR_CHEAT,
        "Aim lock on helps the player to stay on target");
    aim_lockon_debug = Dvar_RegisterBool("aim_lockon_debug", 0, 0x80u, "Turn on debugging info for aim lock on");
    minr.value.max = 1.0f;
    minr.value.min = 0.0f;
    aim_lockon_deflection = Dvar_RegisterFloat(
        "aim_lockon_deflection",
        0.050000001f,
        minr,
        DVAR_CHEAT,
        "The amount of stick deflection for the lockon to activate");
    mins.value.max = 1.0f;
    mins.value.min = 0.0f;
    aim_lockon_strength = Dvar_RegisterFloat(
        "aim_lockon_strength",
        0.60000002f,
        mins,
        DVAR_CHEAT,
        "The amount of aim assistance given by the target lock on");
    mint.value.max = 640.0f;
    mint.value.min = 0.0f;
    aim_lockon_region_width = Dvar_RegisterFloat(
        "aim_lockon_region_width",
        90.0f,
        mint,
        DVAR_CHEAT,
        "The width of the auto aim region in virtual screen coordinates(0-640)");
    minu.value.max = 480.0f;
    minu.value.min = 0.0f;
    aim_lockon_region_height = Dvar_RegisterFloat(
        "aim_lockon_region_height",
        90.0f,
        minu,
        DVAR_CHEAT,
        "The height of the auto aim region in virtual screen coordinates(0-480)");
    aim_scale_view_axis = Dvar_RegisterBool(
        "aim_scale_view_axis",
        1,
        DVAR_CHEAT,
        "Scale the influence of each input axis so that the major axis has more influence on the control");
}

void __cdecl AimAssist_Setup(int32_t localClientNum)
{
    AimAssistGlobals *aaGlob; // [esp+0h] [ebp-4h]

    aaGlob = &aaGlobArray[localClientNum];
    memset((uint8_t*)aaGlob, 0, sizeof(AimAssistGlobals));
    aaGlob->initialized = 1;
    aaGlob->fovTurnRateScale = 1.0;
    aaGlob->fovScaleInv = 1.0;
    aaGlob->screenWidth = scrPlaceView[localClientNum].realViewportSize[0];
    aaGlob->screenHeight = scrPlaceView[localClientNum].realViewportSize[1];
    aaGlob->autoAimTargetEnt = ENTITYNUM_NONE;
    aaGlob->autoMeleeTargetEnt = ENTITYNUM_NONE;
    aaGlob->lockOnTargetEnt = ENTITYNUM_NONE;
}

void __cdecl AimAssist_UpdateScreenTargets(
    int32_t localClientNum,
    const float *viewOrg,
    const float *viewAngles,
    float tanHalfFovX,
    float tanHalfFovY)
{
    float *v5; // [esp+40h] [ebp-C8h]
    float *viewOrigin; // [esp+44h] [ebp-C4h]
    AimScreenTarget screenTarget; // [esp+5Ch] [ebp-ACh] BYREF
    int targetCount; // [esp+90h] [ebp-78h] BYREF
    const AimTarget *target; // [esp+94h] [ebp-74h]
    const AimTarget *targetList; // [esp+98h] [ebp-70h] BYREF
    centity_s *cent; // [esp+9Ch] [ebp-6Ch]
    float entMtx[4][3]; // [esp+A0h] [ebp-68h] BYREF
    int targetIndex; // [esp+D0h] [ebp-38h]
    float bounds[2][3]; // [esp+D4h] [ebp-34h] BYREF
    float clipBounds[2][3]; // [esp+ECh] [ebp-1Ch] BYREF
    AimAssistGlobals *aaGlob; // [esp+104h] [ebp-4h]

    PROF_SCOPED("AimAssist_UpdateScreenTargets");

    aaGlob = &aaGlobArray[localClientNum];
    if (aaGlob->initialized)
    {
        viewOrigin = aaGlob->viewOrigin;
        aaGlob->viewOrigin[0] = *viewOrg;
        viewOrigin[1] = viewOrg[1];
        viewOrigin[2] = viewOrg[2];
        v5 = aaGlob->viewAngles;
        aaGlob->viewAngles[0] = *viewAngles;
        v5[1] = viewAngles[1];
        v5[2] = viewAngles[2];
        AnglesToAxis(viewAngles, aaGlob->viewAxis);
        AimAssist_FovScale(aaGlob, tanHalfFovY);
        AimAssist_CreateScreenMatrix(aaGlob, tanHalfFovX, tanHalfFovY);
        aaGlob->screenTargetCount = 0;
        AimTarget_GetClientTargetList(localClientNum, (AimTarget **)&targetList, &targetCount);
        for (targetIndex = 0; targetIndex < targetCount; ++targetIndex)
        {
            target = &targetList[targetIndex];
            cent = CG_GetEntity(localClientNum, target->entIndex);
            if (cent->nextValid)
            {
                bounds[0][0] = target->mins[0];
                bounds[0][1] = target->mins[1];
                bounds[0][2] = target->mins[2];
                bounds[1][0] = target->maxs[0];
                bounds[1][1] = target->maxs[1];
                bounds[1][2] = target->maxs[2];
                AnglesToAxis(cent->pose.angles, entMtx);
                entMtx[3][0] = cent->pose.origin[0];
                entMtx[3][1] = cent->pose.origin[1];
                entMtx[3][2] = cent->pose.origin[2];
                if (AimAssist_ConvertToClipBounds(aaGlob, bounds, entMtx, clipBounds))
                {
                    screenTarget.entIndex = target->entIndex;
                    screenTarget.clipMins[0] = clipBounds[0][0];
                    screenTarget.clipMins[1] = clipBounds[0][1];
                    screenTarget.clipMaxs[0] = clipBounds[1][0];
                    screenTarget.clipMaxs[1] = clipBounds[1][1];
                    screenTarget.velocity[0] = target->velocity[0];
                    screenTarget.velocity[1] = target->velocity[1];
                    screenTarget.velocity[2] = target->velocity[2];
                    if (AimAssist_CalcAimPos(localClientNum, cent, target, screenTarget.aimPos))
                    {
                        screenTarget.distSqr = target->worldDistSqr;
                        screenTarget.crosshairDistSqr = AimAssist_GetCrosshairDistSqr(clipBounds[0], clipBounds[1]);
                        AimAssist_AddToTargetList(aaGlob, &screenTarget);
                    }
                }
            }
        }
    }
}

void __cdecl AimAssist_FovScale(AimAssistGlobals *aaGlob, float tanHalfFovY)
{
    float v2; // [esp+8h] [ebp-Ch]
    float v3; // [esp+Ch] [ebp-8h]
    float tanHalfBaseFovY; // [esp+10h] [ebp-4h]

    if (!aaGlob)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 390, 0, "%s", "aaGlob");
    aaGlob->fovTurnRateScale = tanHalfFovY / (float)0.47780272;
    v3 = cg_fov->current.value * 0.01745329238474369 * 0.5;
    v2 = tan(v3);
    tanHalfBaseFovY = v2 * 0.75;
    if (tanHalfBaseFovY == 0.0)
        MyAssertHandler(
            ".\\aim_assist\\aim_assist.cpp",
            399,
            0,
            "%s\n\t(tanHalfBaseFovY) = %g",
            "(tanHalfBaseFovY != 0.0f)",
            tanHalfBaseFovY);
    aaGlob->fovScaleInv = tanHalfBaseFovY / tanHalfFovY;
}

void __cdecl AimAssist_CreateScreenMatrix(AimAssistGlobals *aaGlob, float tanHalfFovX, float tanHalfFovY)
{
    float viewMtx[4][4]; // [esp+Ch] [ebp-C0h] BYREF
    float projMtx[4][4]; // [esp+4Ch] [ebp-80h] BYREF
    float screenMtx[4][4]; // [esp+8Ch] [ebp-40h] BYREF

    if (!aaGlob)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 413, 0, "%s", "aaGlob");
    MatrixForViewer(viewMtx, aaGlob->viewOrigin, aaGlob->viewAxis);
    InfinitePerspectiveMatrix(projMtx, tanHalfFovX, tanHalfFovY, 1.0);
    MatrixMultiply44(viewMtx, projMtx, screenMtx);
    MatrixTranspose44(screenMtx, aaGlob->screenMtx);
    MatrixInverse44(aaGlob->screenMtx, aaGlob->invScreenMtx);
}

char __cdecl AimAssist_ConvertToClipBounds(
    const AimAssistGlobals *aaGlob,
    const float (*bounds)[3],
    const mat4x3 &mtx,
    float (*clipBounds)[3])
{
    float v5; // [esp+0h] [ebp-A0h]
    float v6; // [esp+4h] [ebp-9Ch]
    float v7; // [esp+8h] [ebp-98h]
    float v8; // [esp+Ch] [ebp-94h]
    float v9; // [esp+10h] [ebp-90h]
    float v10; // [esp+14h] [ebp-8Ch]
    float v11; // [esp+18h] [ebp-88h]
    float v12; // [esp+1Ch] [ebp-84h]
    float v13; // [esp+20h] [ebp-80h]
    float v14; // [esp+24h] [ebp-7Ch]
    float v15; // [esp+28h] [ebp-78h]
    float v16; // [esp+2Ch] [ebp-74h]
    float v17; // [esp+30h] [ebp-70h]
    float v18; // [esp+34h] [ebp-6Ch]
    float v19; // [esp+38h] [ebp-68h]
    float v20; // [esp+3Ch] [ebp-64h]
    float v21; // [esp+40h] [ebp-60h]
    float v22; // [esp+44h] [ebp-5Ch]
    float v23; // [esp+48h] [ebp-58h]
    float v24; // [esp+4Ch] [ebp-54h]
    float v25; // [esp+50h] [ebp-50h]
    float v26; // [esp+54h] [ebp-4Ch]
    float v27; // [esp+58h] [ebp-48h]
    float v28; // [esp+5Ch] [ebp-44h]
    float v29; // [esp+60h] [ebp-40h]
    float v30; // [esp+64h] [ebp-3Ch]
    float v31; // [esp+68h] [ebp-38h]
    float v32; // [esp+6Ch] [ebp-34h]
    float v33; // [esp+70h] [ebp-30h]
    float v34; // [esp+74h] [ebp-2Ch]
    float worldCorner[3]; // [esp+78h] [ebp-28h] BYREF
    float clipCorner[3]; // [esp+84h] [ebp-1Ch] BYREF
    int ptIndex; // [esp+90h] [ebp-10h]
    float worldCornerRotated[3]; // [esp+94h] [ebp-Ch] BYREF

    if (!bounds)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 464, 0, "%s", "bounds");
    if (!mtx)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 465, 0, "%s", "mtx");
    if (!clipBounds)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 466, 0, "%s", "clipBounds");
    ClearBounds((float *)clipBounds, &(*clipBounds)[3]);
    for (ptIndex = 0; ptIndex < 8; ++ptIndex)
    {
        worldCorner[0] = (*bounds)[3 * (ptIndex & 1)];
        worldCorner[1] = (*bounds)[3 * ((ptIndex >> 1) & 1) + 1];
        worldCorner[2] = (*bounds)[3 * ((ptIndex >> 2) & 1) + 2];
        MatrixTransformVector43(worldCorner, mtx, worldCornerRotated);
        if (AimAssist_XfmWorldPointToClipSpace(aaGlob, worldCornerRotated, clipCorner))
            AddPointToBounds(clipCorner, (float *)clipBounds, &(*clipBounds)[3]);
    }
    if ((*clipBounds)[3] <= (double)(*clipBounds)[0]
        || (*clipBounds)[4] <= (double)(*clipBounds)[1]
        || (*clipBounds)[5] <= (double)(*clipBounds)[2])
    {
        return 0;
    }
    if ((*clipBounds)[0] > 1.0 || (*clipBounds)[1] > 1.0 || (*clipBounds)[2] > 1.0)
        return 0;
    if ((*clipBounds)[3] < -1.0 || (*clipBounds)[4] < -1.0 || (*clipBounds)[5] < 0.0)
        return 0;
    v33 = (*clipBounds)[0];
    v22 = v33 - 1.0;
    if (v22 < 0.0)
        v34 = v33;
    else
        v34 = 1.0;
    v21 = -1.0 - v33;
    if (v21 < 0.0)
        v20 = v34;
    else
        v20 = -1.0;
    (*clipBounds)[0] = v20;
    v31 = (*clipBounds)[1];
    v19 = v31 - 1.0;
    if (v19 < 0.0)
        v32 = v31;
    else
        v32 = 1.0;
    v18 = -1.0 - v31;
    if (v18 < 0.0)
        v17 = v32;
    else
        v17 = -1.0;
    (*clipBounds)[1] = v17;
    v29 = (*clipBounds)[2];
    v16 = v29 - 1.0;
    if (v16 < 0.0)
        v30 = v29;
    else
        v30 = 1.0;
    v15 = 0.0 - v29;
    if (v15 < 0.0)
        v14 = v30;
    else
        v14 = 0.0;
    (*clipBounds)[2] = v14;
    v27 = (*clipBounds)[3];
    v13 = v27 - 1.0;
    if (v13 < 0.0)
        v28 = v27;
    else
        v28 = 1.0;
    v12 = -1.0 - v27;
    if (v12 < 0.0)
        v11 = v28;
    else
        v11 = -1.0;
    (*clipBounds)[3] = v11;
    v25 = (*clipBounds)[4];
    v10 = v25 - 1.0;
    if (v10 < 0.0)
        v26 = v25;
    else
        v26 = 1.0;
    v9 = -1.0 - v25;
    if (v9 < 0.0)
        v8 = v26;
    else
        v8 = -1.0;
    (*clipBounds)[4] = v8;
    v23 = (*clipBounds)[5];
    v7 = v23 - 1.0;
    if (v7 < 0.0)
        v24 = v23;
    else
        v24 = 1.0;
    v6 = 0.0 - v23;
    if (v6 < 0.0)
        v5 = v24;
    else
        v5 = 0.0;
    (*clipBounds)[5] = v5;
    return 1;
}

char __cdecl AimAssist_XfmWorldPointToClipSpace(const AimAssistGlobals *aaGlob, const float *in, float *out)
{
    float clip; // [esp+0h] [ebp-20h]
    float clip_4; // [esp+4h] [ebp-1Ch]
    float clip_8; // [esp+8h] [ebp-18h]
    float clip_12; // [esp+Ch] [ebp-14h]
    float xyzw[4]; // [esp+10h] [ebp-10h] BYREF

    if (!aaGlob)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 431, 0, "%s", "aaGlob");
    if (!in)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 432, 0, "%s", "in");
    if (!out)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 433, 0, "%s", "out");
    xyzw[0] = *in;
    xyzw[1] = in[1];
    xyzw[2] = in[2];
    xyzw[3] = 1.0;
    clip_12 = Vec4Dot(aaGlob->screenMtx[3], xyzw);
    if (clip_12 <= 0.0)
        return 0;
    clip = Vec4Dot(aaGlob->screenMtx[0], xyzw);
    clip_4 = Vec4Dot(aaGlob->screenMtx[1], xyzw);
    clip_8 = Vec4Dot(aaGlob->screenMtx[2], xyzw);
    *out = clip / clip_12;
    out[1] = clip_4 / clip_12 * -1.0;
    out[2] = clip_8 / clip_12;
    return 1;
}

double __cdecl AimAssist_GetCrosshairDistSqr(const float *clipMins, const float *clipMaxs)
{
    float center; // [esp+4h] [ebp-8h]
    float centera; // [esp+4h] [ebp-8h]
    float center_4; // [esp+8h] [ebp-4h]
    float center_4a; // [esp+8h] [ebp-4h]

    if (!clipMins)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 508, 0, "%s", "clipMins");
    if (!clipMaxs)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 509, 0, "%s", "clipMaxs");
    center = *clipMins + *clipMaxs;
    center_4 = clipMins[1] + clipMaxs[1];
    centera = 0.5 * center;
    center_4a = 0.5 * center_4;
    return (float)(center_4a * center_4a + centera * centera);
}

void __cdecl AimAssist_AddToTargetList(AimAssistGlobals *aaGlob, const AimScreenTarget *screenTarget)
{
    int low; // [esp+8h] [ebp-Ch]
    int high; // [esp+10h] [ebp-4h]

    if (!aaGlob)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 609, 0, "%s", "aaGlob");
    if (!screenTarget)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 610, 0, "%s", "screenTarget");
    low = 0;
    high = aaGlob->screenTargetCount;
    while (low < high)
    {
        if (AimAssist_CompareTargets(screenTarget, &aaGlob->screenTargets[(high + low) / 2]) <= 0)
            low = (high + low) / 2 + 1;
        else
            high = (high + low) / 2;
    }
    if (low < 64)
    {
        if (aaGlob->screenTargetCount == 64)
            --aaGlob->screenTargetCount;
        memmove(
            (unsigned __int8 *)&aaGlob->screenTargets[low + 1],
            (unsigned __int8 *)&aaGlob->screenTargets[low],
            52 * (aaGlob->screenTargetCount - low));
        memcpy(&aaGlob->screenTargets[low], screenTarget, sizeof(aaGlob->screenTargets[low]));
        ++aaGlob->screenTargetCount;
    }
}

int32_t __cdecl AimAssist_CompareTargets(const AimScreenTarget *screenTargetA, const AimScreenTarget *screenTargetB)
{
    if (!screenTargetA)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 586, 0, "%s", "screenTargetA");
    if (!screenTargetB)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 587, 0, "%s", "screenTargetB");
    if (screenTargetB->crosshairDistSqr > (double)screenTargetA->crosshairDistSqr)
        return 1;
    if (screenTargetB->crosshairDistSqr >= (double)screenTargetA->crosshairDistSqr)
        return 0;
    return -1;
}

int32_t __cdecl AimAssist_CalcAimPos(
    int32_t localClientNum,
    const centity_s *targetEnt,
    const AimTarget *target,
    float *aimPos)
{
    float center[3]; // [esp+0h] [ebp-Ch] BYREF

    if (!targetEnt)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 667, 0, "%s", "targetEnt");
    if (!target)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 668, 0, "%s", "target");
    if (!aimPos)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 669, 0, "%s", "aimPos");
    if (targetEnt->nextState.eType == ET_PLAYER)
        return AimTarget_GetTagPos(localClientNum, targetEnt, scr_const.aim_bone, aimPos);
    Vec3Avg(target->mins, target->maxs, center);
    Vec3Add(targetEnt->pose.origin, center, aimPos);
    return 1;
}

int32_t __cdecl AimTarget_GetTagPos(int32_t localClientNum, const centity_s *cent, uint32_t tagName, float *pos)
{
    DObj_s *dobj; // [esp+0h] [ebp-4h]

    if (!cent)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 646, 0, "%s", "cent");
    if (!pos)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 647, 0, "%s", "pos");
    dobj = Com_GetClientDObj(cent->nextState.number, localClientNum);
    if (!dobj)
        return 0;
    if (!CG_DObjGetWorldTagPos(&cent->pose, dobj, tagName, pos))
    {
        Com_Error(ERR_DROP, "AimTarget_GetTagPos: Cannot find tag [%s] on entity\n", SL_ConvertToString(tagName));
    }
    return 1;
}

int32_t __cdecl AimAssist_GetScreenTargetCount(int32_t localClientNum)
{
    return aaGlobArray[localClientNum].screenTargetCount;
}

int32_t __cdecl AimAssist_GetScreenTargetEntity(int32_t localClientNum, uint32_t targetIndex)
{
    const AimAssistGlobals *aaGlob; // [esp+0h] [ebp-4h]

    aaGlob = &aaGlobArray[localClientNum];
    if (targetIndex >= aaGlob->screenTargetCount)
        MyAssertHandler(
            ".\\aim_assist\\aim_assist.cpp",
            784,
            0,
            "targetIndex doesn't index aaGlob->screenTargetCount\n\t%i not in [0, %i)",
            targetIndex,
            aaGlob->screenTargetCount);
    return aaGlob->screenTargets[targetIndex].entIndex;
}

void __cdecl AimAssist_ClearEntityReference(int32_t localClientNum, int32_t entIndex)
{
    AimAssistGlobals *aaGlob; // [esp+0h] [ebp-4h]

    aaGlob = &aaGlobArray[localClientNum];
    if (aaGlob->autoAimTargetEnt == entIndex)
        aaGlob->autoAimTargetEnt = ENTITYNUM_NONE;
    if (aaGlob->autoMeleeTargetEnt == entIndex)
        aaGlob->autoMeleeTargetEnt = ENTITYNUM_NONE;
    if (aaGlob->lockOnTargetEnt == entIndex)
        aaGlob->lockOnTargetEnt = ENTITYNUM_NONE;
}

void __cdecl AimAssist_UpdateTweakables(const AimInput *input)
{
    float v1; // [esp+0h] [ebp-58h]
    float v2; // [esp+4h] [ebp-54h]
    float v3; // [esp+8h] [ebp-50h]
    float v4; // [esp+Ch] [ebp-4Ch]
    float v5; // [esp+10h] [ebp-48h]
    float v6; // [esp+14h] [ebp-44h]
    float v7; // [esp+18h] [ebp-40h]
    float v8; // [esp+1Ch] [ebp-3Ch]
    float v9; // [esp+20h] [ebp-38h]
    float v10; // [esp+24h] [ebp-34h]
    float v11; // [esp+28h] [ebp-30h]
    float v12; // [esp+2Ch] [ebp-2Ch]
    int v13; // [esp+30h] [ebp-28h]
    int v14; // [esp+34h] [ebp-24h]
    int v15; // [esp+38h] [ebp-20h]
    int v16; // [esp+3Ch] [ebp-1Ch]
    int v17; // [esp+40h] [ebp-18h]
    int v18; // [esp+44h] [ebp-14h]
    int v19; // [esp+48h] [ebp-10h]
    int localClientNum; // [esp+4Ch] [ebp-Ch]
    AimAssistGlobals *aaGlob; // [esp+50h] [ebp-8h]

    if (!input)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 236, 0, "%s", "input");
    aaGlob = &aaGlobArray[input->localClientNum];
    localClientNum = input->localClientNum;
    v12 = scrPlaceView[localClientNum].scaleVirtualToReal[0] * aim_slowdown_region_width->current.value;
    aaGlob->tweakables.slowdownRegionWidth = v12 / aaGlobArray[localClientNum].screenWidth;
    v19 = input->localClientNum;
    v11 = scrPlaceView[v19].scaleVirtualToReal[1] * aim_slowdown_region_height->current.value;
    aaGlob->tweakables.slowdownRegionHeight = v11 / aaGlobArray[v19].screenHeight;
    v18 = input->localClientNum;
    v10 = scrPlaceView[v18].scaleVirtualToReal[0] * aim_autoaim_region_width->current.value;
    v9 = v10 / aaGlobArray[v18].screenWidth;
    aaGlob->tweakables.autoAimRegionWidth = v9 * aaGlob->fovScaleInv;
    v17 = input->localClientNum;
    v8 = scrPlaceView[v17].scaleVirtualToReal[1] * aim_autoaim_region_height->current.value;
    v7 = v8 / aaGlobArray[v17].screenHeight;
    aaGlob->tweakables.autoAimRegionHeight = v7 * aaGlob->fovScaleInv;
    v16 = input->localClientNum;
    v6 = scrPlaceView[v16].scaleVirtualToReal[0] * aim_automelee_region_width->current.value;
    v5 = v6 / aaGlobArray[v16].screenWidth;
    aaGlob->tweakables.autoMeleeRegionWidth = v5 * aaGlob->fovScaleInv;
    v15 = input->localClientNum;
    v4 = scrPlaceView[v15].scaleVirtualToReal[1] * aim_automelee_region_height->current.value;
    v3 = v4 / aaGlobArray[v15].screenHeight;
    aaGlob->tweakables.autoMeleeRegionHeight = v3 * aaGlob->fovScaleInv;
    v14 = input->localClientNum;
    v2 = scrPlaceView[v14].scaleVirtualToReal[0] * aim_lockon_region_width->current.value;
    aaGlob->tweakables.lockOnRegionWidth = v2 / aaGlobArray[v14].screenWidth;
    v13 = input->localClientNum;
    v1 = scrPlaceView[v13].scaleVirtualToReal[1] * aim_lockon_region_height->current.value;
    aaGlob->tweakables.lockOnRegionHeight = v1 / aaGlobArray[v13].screenHeight;
}

void __cdecl AimAssist_UpdateAdsLerp(const AimInput *input)
{
    AimAssistGlobals *aaGlob; // [esp+0h] [ebp-4h]

    if (!input)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 262, 0, "%s", "input");
    aaGlob = &aaGlobArray[input->localClientNum];
    aaGlob->adsLerp = input->ps->fWeaponPosFrac;
    if ((input->ps->eFlags & 0x300) != 0 && (input->buttons & 0x800) != 0)
        aaGlob->adsLerp = 1.0;
}

uint32_t __cdecl AimAssist_GetWeaponIndex(int32_t localClientNum, const playerState_s *ps)
{
    uint32_t NumWeapons; // eax
    uint32_t weapIndex; // [esp+0h] [ebp-8h]

    if ((ps->eFlags & 0x300) != 0)
    {
        iassert(ps->viewlocked_entNum != ENTITYNUM_NONE);
        iassert(((ps->viewlocked_entNum >= 0) && (ps->viewlocked_entNum < (1 << 10))));
        weapIndex = CG_GetEntity(localClientNum, ps->viewlocked_entNum)->nextState.weapon;
    }
    else
    {
        weapIndex = BG_GetViewmodelWeaponIndex(ps);
    }

    bcassert(weapIndex, BG_GetNumWeapons());

    return weapIndex;
}

const AimScreenTarget *__cdecl AimAssist_GetBestTarget(
    const AimAssistGlobals *aaGlob,
    float range,
    float regionWidth,
    float regionHeight)
{
    float v5; // [esp+8h] [ebp-Ch]
    int targetIndex; // [esp+10h] [ebp-4h]

    if (!aaGlob)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 843, 0, "%s", "aaGlob");
    if (range < 0.0)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 844, 0, "%s\n\t(range) = %g", "((range >= 0.0f))", range);
    for (targetIndex = 0; targetIndex < aaGlob->screenTargetCount; ++targetIndex)
    {
        v5 = range * range;
        if (v5 >= (double)aaGlob->screenTargets[targetIndex].distSqr
            && AimAssist_DoBoundsIntersectCenterBox(
                aaGlob->screenTargets[targetIndex].clipMins,
                aaGlob->screenTargets[targetIndex].clipMaxs,
                regionWidth,
                regionHeight))
        {
            return &aaGlob->screenTargets[targetIndex];
        }
    }
    return 0;
}

bool __cdecl AimAssist_DoBoundsIntersectCenterBox(
    const float *clipMins,
    const float *clipMaxs,
    float clipHalfWidth,
    float clipHalfHeight)
{
    bool v6; // [esp+4h] [ebp-Ch]
    bool v7; // [esp+8h] [ebp-8h]

    if (!clipMins)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 528, 0, "%s", "clipMins");
    if (!clipMaxs)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 529, 0, "%s", "clipMaxs");
    v7 = clipHalfWidth >= (double)*clipMins && *clipMaxs >= -clipHalfWidth;
    v6 = clipHalfHeight >= (double)clipMins[1] && clipMaxs[1] >= -clipHalfHeight;
    return v7 && v6;
}

const AimScreenTarget *__cdecl AimAssist_GetTargetFromEntity(const AimAssistGlobals *aaGlob, int32_t entIndex)
{
    int32_t targetIndex; // [esp+4h] [ebp-4h]

    iassert(aaGlob);

    if (entIndex == ENTITYNUM_NONE)
        return 0;

    for (targetIndex = 0; targetIndex < aaGlob->screenTargetCount; ++targetIndex)
    {
        if (aaGlob->screenTargets[targetIndex].entIndex == entIndex)
            return &aaGlob->screenTargets[targetIndex];
    }

    return 0;
}

void __cdecl AimAssist_ApplyAutoMelee(const AimInput *input, AimOutput *output)
{
    const AimScreenTarget *screenTarget; // [esp+18h] [ebp-28h]
    bool meleeing; // [esp+1Fh] [ebp-21h]
    float yawDelta; // [esp+20h] [ebp-20h]
    float newPitch; // [esp+24h] [ebp-1Ch]
    float pitchDelta; // [esp+28h] [ebp-18h]
    unsigned int weapIndex; // [esp+2Ch] [ebp-14h]
    AimAssistGlobals *aaGlob; // [esp+30h] [ebp-10h]
    float newYaw; // [esp+34h] [ebp-Ch]

    if (!input)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 1339, 0, "%s", "input");
    if (!output)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 1340, 0, "%s", "output");
    aaGlob = &aaGlobArray[input->localClientNum];
    meleeing = input->ps->weaponstate == 12;
    weapIndex = AimAssist_GetWeaponIndex(input->localClientNum, input->ps);
    if (aim_automelee_enabled->current.enabled && meleeing && weapIndex)
    {
        if (!aaGlob->autoMeleePressed)
        {
            screenTarget = AimAssist_GetBestTarget(
                aaGlob,
                aim_automelee_range->current.value,
                aaGlob->tweakables.autoMeleeRegionWidth,
                aaGlob->tweakables.autoMeleeRegionHeight);
            if (screenTarget)
                AimAssist_SetAutoMeleeTarget(aaGlob, screenTarget);
            aaGlob->autoMeleePressed = 1;
        }
        if (aaGlob->autoMeleeActive)
        {
            if (AimAssist_UpdateAutoMeleeTarget(aaGlob))
            {
                newPitch = DiffTrackAngle(
                    aaGlob->autoMeleePitchTarget,
                    aaGlob->autoMeleePitch,
                    aim_automelee_lerp->current.value,
                    input->deltaTime);
                newYaw = DiffTrackAngle(
                    aaGlob->autoMeleeYawTarget,
                    aaGlob->autoMeleeYaw,
                    aim_automelee_lerp->current.value,
                    input->deltaTime);
                pitchDelta = AngleDelta(newPitch, aaGlob->autoMeleePitch);
                yawDelta = AngleDelta(newYaw, aaGlob->autoMeleeYaw);
                aaGlob->autoMeleePitch = newPitch;
                aaGlob->autoMeleeYaw = newYaw;
                output->pitch = output->pitch + pitchDelta;
                output->yaw = output->yaw + yawDelta;
            }
            else
            {
                AimAssist_ClearAutoMeleeTarget(aaGlob);
            }
        }
    }
    else
    {
        AimAssist_ClearAutoMeleeTarget(aaGlob);
        aaGlob->autoMeleePressed = 0;
    }
}

void __cdecl AimAssist_ClearAutoMeleeTarget(AimAssistGlobals *aaGlob)
{
    iassert(aaGlob);

    aaGlob->autoMeleeTargetEnt = ENTITYNUM_NONE;
    aaGlob->autoMeleeActive = 0;
    aaGlob->autoMeleePitch = 0.0;
    aaGlob->autoMeleePitchTarget = 0.0;
    aaGlob->autoMeleeYaw = 0.0;
    aaGlob->autoMeleeYawTarget = 0.0;
}

char __cdecl AimAssist_UpdateAutoMeleeTarget(AimAssistGlobals *aaGlob)
{
    float targetDir[3]; // [esp+0h] [ebp-1Ch] BYREF
    const AimScreenTarget *screenTarget; // [esp+Ch] [ebp-10h]
    float targetAngles[3]; // [esp+10h] [ebp-Ch] BYREF

    if (!aaGlob)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 1286, 0, "%s", "aaGlob");
    if (!aaGlob->autoMeleeActive)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 1287, 0, "%s", "aaGlob->autoMeleeActive");
    screenTarget = AimAssist_GetTargetFromEntity(aaGlob, aaGlob->autoMeleeTargetEnt);
    if (!screenTarget)
        return 0;
    Vec3Sub(screenTarget->aimPos, aaGlob->viewOrigin, targetDir);
    vectoangles(targetDir, targetAngles);
    aaGlob->autoMeleePitchTarget = targetAngles[0];
    aaGlob->autoMeleeYawTarget = targetAngles[1];
    return 1;
}

void __cdecl AimAssist_SetAutoMeleeTarget(AimAssistGlobals *aaGlob, const AimScreenTarget *screenTarget)
{
    if (!aaGlob)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 1308, 0, "%s", "aaGlob");
    if (!screenTarget)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 1309, 0, "%s", "screenTarget");
    AimAssist_ClearAutoMeleeTarget(aaGlob);
    aaGlob->autoMeleeTargetEnt = screenTarget->entIndex;
    aaGlob->autoMeleeActive = 1;
    aaGlob->autoMeleePitch = aaGlob->viewAngles[0];
    aaGlob->autoMeleeYaw = aaGlob->viewAngles[1];
    AimAssist_UpdateAutoMeleeTarget(aaGlob);
}

void __cdecl AimAssist_ApplyMeleeCharge(const AimInput *input, AimOutput *output)
{
    float v2; // [esp+18h] [ebp-30h]
    float v3; // [esp+1Ch] [ebp-2Ch]
    float v4; // [esp+20h] [ebp-28h]
    float targetDir[2]; // [esp+34h] [ebp-14h] BYREF
    const AimScreenTarget *screenTarget; // [esp+3Ch] [ebp-Ch]
    AimAssistGlobals *aaGlob; // [esp+40h] [ebp-8h]
    const AimTweakables *tweaks; // [esp+44h] [ebp-4h]

    if (!input)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 1479, 0, "%s", "input");
    if (!output)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 1480, 0, "%s", "output");
    aaGlob = &aaGlobArray[input->localClientNum];
    tweaks = &aaGlob->tweakables;
    output->meleeChargeYaw = 0.0;
    output->meleeChargeDist = 0;
    if ((input->buttons & 4) != 0 && (input->ps->pm_flags & 1) == 0)
    {
        screenTarget = AimAssist_GetBestTarget(
            aaGlob,
            aim_automelee_range->current.value,
            tweaks->autoMeleeRegionWidth,
            tweaks->autoMeleeRegionHeight);
        if (screenTarget)
        {
            v4 = player_meleeRange->current.value * player_meleeRange->current.value;
            if (v4 <= (double)screenTarget->distSqr)
            {
                v3 = 255.0 * 255.0;
                if (v3 < (double)screenTarget->distSqr)
                    MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 1501, 0, "%s", "screenTarget->distSqr <= SQR( 255.0f )");
                targetDir[0] = screenTarget->aimPos[0] - aaGlob->viewOrigin[0];
                targetDir[1] = screenTarget->aimPos[1] - aaGlob->viewOrigin[1];
                output->meleeChargeYaw = vectoyaw(targetDir);
                v2 = sqrt(screenTarget->distSqr);
                output->meleeChargeDist = (int)v2;
            }
        }
    }
}

void __cdecl AimAssist_UpdateMouseInput(const AimInput *input, AimOutput *output)
{
    PROF_SCOPED("AimAssist_UpdateMouseInput");

    iassert(input);
    iassert(input->ps);
    iassert(output);

    output->pitch = input->pitch;
    output->yaw = input->yaw;

    if (aaGlobArray[input->localClientNum].initialized)
    {
        AimAssist_UpdateTweakables(input);
        AimAssist_UpdateAdsLerp(input);
        AimAssist_ApplyAutoMelee(input, output);
        AimAssist_ApplyMeleeCharge(input, output);
    }
}

void __cdecl AimAssist_DrawDebugOverlay(uint32_t localClientNum)
{
    float green[4]; // [esp+Ch] [ebp-2Ch] BYREF
    float red[4]; // [esp+1Ch] [ebp-1Ch] BYREF
    const AimAssistGlobals *aaGlob; // [esp+2Ch] [ebp-Ch]
    const playerState_s *ps; // [esp+30h] [ebp-8h]
    const AimTweakables *tweaks; // [esp+34h] [ebp-4h]

    red[0] = 1.0;
    red[1] = 0.0;
    red[2] = 0.0;
    red[3] = 0.25;
    green[0] = 0.0;
    green[1] = 1.0;
    green[2] = 0.0;
    green[3] = 0.25;
    aaGlob = &aaGlobArray[localClientNum];
    if (aaGlob->initialized)
    {
        tweaks = &aaGlob->tweakables;
        ps = &CG_GetLocalClientGlobals(localClientNum)->predictedPlayerState;

        if (aim_slowdown_debug->current.enabled)
        {
            AimAssist_DrawTargets(localClientNum, red);
            AimAssist_DrawCenterBox(aaGlob, tweaks->slowdownRegionWidth, tweaks->slowdownRegionHeight, green);
        }
        if (aim_autoaim_debug->current.enabled)
        {
            AimAssist_DrawTargets(localClientNum, red);
            AimAssist_DrawCenterBox(aaGlob, tweaks->autoAimRegionWidth, tweaks->autoAimRegionHeight, green);
        }
        if (aim_automelee_debug->current.enabled)
        {
            AimAssist_DrawTargets(localClientNum, red);
            AimAssist_DrawCenterBox(aaGlob, tweaks->autoMeleeRegionWidth, tweaks->autoMeleeRegionHeight, green);
        }
        if (aim_lockon_debug->current.enabled)
        {
            AimAssist_DrawTargets(localClientNum, red);
            AimAssist_DrawCenterBox(aaGlob, tweaks->lockOnRegionWidth, tweaks->lockOnRegionHeight, green);
        }
    }
}

void __cdecl AimAssist_DrawCenterBox(
    const AimAssistGlobals *aaGlob,
    float clipHalfWidth,
    float clipHalfHeight,
    const float *color)
{
    float width; // [esp+28h] [ebp-10h]
    float height; // [esp+2Ch] [ebp-Ch]
    float x; // [esp+30h] [ebp-8h]
    float y; // [esp+34h] [ebp-4h]

    if (!aaGlob)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 1597, 0, "%s", "aaGlob");
    if (!color)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 1598, 0, "%s", "color");
    width = clipHalfWidth * aaGlob->screenWidth;
    height = clipHalfHeight * aaGlob->screenHeight;
    x = (aaGlob->screenWidth - width) * 0.5;
    y = (aaGlob->screenHeight - height) * 0.5;
    CL_DrawStretchPicPhysical(x, y, width, height, 0.0, 0.0, 1.0, 1.0, color, cgMedia.whiteMaterial);
}

void __cdecl AimAssist_DrawTargets(int64_t localClientNum, const float *color)
{
    char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    double v5; // st7
    char *v6; // eax
    float v7; // [esp+28h] [ebp-6Ch]
    float v8; // [esp+2Ch] [ebp-68h]
    float v9; // [esp+30h] [ebp-64h]
    float v10; // [esp+34h] [ebp-60h]
    float v11; // [esp+38h] [ebp-5Ch]
    float v12; // [esp+3Ch] [ebp-58h]
    float range; // [esp+64h] [ebp-30h]
    float width; // [esp+68h] [ebp-2Ch]
    float height; // [esp+6Ch] [ebp-28h]
    char *msg; // [esp+70h] [ebp-24h]
    int targetIndex; // [esp+74h] [ebp-20h]
    int weapIndex; // [esp+78h] [ebp-1Ch]
    const AimAssistGlobals *aaGlob; // [esp+7Ch] [ebp-18h]
    const WeaponDef *weapDef; // [esp+80h] [ebp-14h]
    float x; // [esp+84h] [ebp-10h]
    float x_4; // [esp+88h] [ebp-Ch]
    float y; // [esp+8Ch] [ebp-8h]
    float ya; // [esp+8Ch] [ebp-8h]
    float yb; // [esp+8Ch] [ebp-8h]
    float yc; // [esp+8Ch] [ebp-8h]
    float yd; // [esp+8Ch] [ebp-8h]
    float y_4; // [esp+90h] [ebp-4h]

    if (!HIDWORD(localClientNum))
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 1627, 0, "%s", "ps");
    if (!color)
        MyAssertHandler(".\\aim_assist\\aim_assist.cpp", 1628, 0, "%s", "color");
    weapIndex = AimAssist_GetWeaponIndex(localClientNum, (const playerState_s *)HIDWORD(localClientNum));
    if (weapIndex)
    {
        weapDef = BG_GetWeaponDef(weapIndex);
        aaGlob = &aaGlobArray[localClientNum];
        range = aaGlob->adsLerp * weapDef->aimAssistRangeAds + (1.0 - aaGlob->adsLerp) * weapDef->aimAssistRange;
        for (targetIndex = 0; targetIndex < aaGlob->screenTargetCount; ++targetIndex)
        {
            if (aim_autoaim_debug->current.enabled)
            {
                v12 = weapDef->autoAimRange * weapDef->autoAimRange;
                if (v12 < (double)aaGlob->screenTargets[targetIndex].distSqr)
                    continue;
            }
            if (aim_automelee_debug->current.enabled)
            {
                v11 = aim_automelee_range->current.value * aim_automelee_range->current.value;
                if (v11 < (double)aaGlob->screenTargets[targetIndex].distSqr)
                    continue;
            }
            if (aim_slowdown_debug->current.enabled)
            {
                v10 = range * range;
                if (v10 < (double)aaGlob->screenTargets[targetIndex].distSqr)
                    continue;
            }
            if (aim_lockon_debug->current.enabled)
            {
                v9 = range * range;
                if (v9 < (double)aaGlob->screenTargets[targetIndex].distSqr)
                    continue;
            }
            x = (aaGlob->screenTargets[targetIndex].clipMins[0] + 1.0) * (aaGlob->screenWidth * 0.5);
            y = (aaGlob->screenTargets[targetIndex].clipMins[1] + 1.0) * (aaGlob->screenHeight * 0.5);
            x_4 = (aaGlob->screenTargets[targetIndex].clipMaxs[0] + 1.0) * (aaGlob->screenWidth * 0.5);
            y_4 = (aaGlob->screenTargets[targetIndex].clipMaxs[1] + 1.0) * (aaGlob->screenHeight * 0.5);
            width = x_4 - x;
            height = y_4 - y;
            CL_DrawStretchPicPhysical(x, y, width, height, 0.0, 0.0, 1.0, 1.0, color, cgMedia.whiteMaterial);
            v2 = va("Pri: %i", targetIndex);
            CL_DrawText(
                &scrPlaceView[localClientNum],
                v2,
                0x7FFFFFFF,
                cgMedia.smallDevFont,
                x,
                y,
                5,
                5,
                1.0,
                1.0,
                colorYellow,
                0);
            ya = y - 10.0;
            v3 = va("Ent: %i", aaGlob->screenTargets[targetIndex].entIndex);
            CL_DrawText(
                &scrPlaceView[localClientNum],
                v3,
                0x7FFFFFFF,
                cgMedia.smallDevFont,
                x,
                ya,
                5,
                5,
                1.0,
                1.0,
                colorYellow,
                0);
            yb = ya - 10.0;
            v8 = sqrt(aaGlob->screenTargets[targetIndex].distSqr);
            v4 = va("Dist: %.2f", v8);
            CL_DrawText(
                &scrPlaceView[localClientNum],
                v4,
                0x7FFFFFFF,
                cgMedia.smallDevFont,
                x,
                yb,
                5,
                5,
                1.0,
                1.0,
                colorWhite,
                0);
            yc = yb - 10.0;
            v5 = Vec3Length(aaGlob->screenTargets[targetIndex].velocity);
            v6 = va("Speed: %.2f", v5);
            CL_DrawText(
                &scrPlaceView[localClientNum],
                v6,
                0x7FFFFFFF,
                cgMedia.smallDevFont,
                x,
                yc,
                5,
                5,
                1.0,
                1.0,
                colorWhite,
                0);
            yd = yc - 10.0;
            v7 = sqrt(aaGlob->screenTargets[targetIndex].crosshairDistSqr);
            msg = va("XHairDist: %.4f", v7);
            CL_DrawText(
                &scrPlaceView[localClientNum],
                msg,
                0x7FFFFFFF,
                cgMedia.smallDevFont,
                x,
                yd,
                5,
                5,
                1.0,
                1.0,
                colorWhite,
                0);
        }
    }
}
