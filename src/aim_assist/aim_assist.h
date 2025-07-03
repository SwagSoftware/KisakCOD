#pragma once
#include <universal/q_shared.h>
#include <cgame/cg_local.h>
#include <qcommon/graph.h>
#include <cstdint>

#define EF_AIM_ASSIST 0x800
#define SOLID_BMODEL 0xFFFFFF

struct AimTarget // sizeof=0x2C
{                                       // ...
    int32_t entIndex;
    float worldDistSqr;
    float mins[3];
    float maxs[3];
    float velocity[3];
};
static_assert(sizeof(AimTarget) == 0x2C);

struct AimTargetGlob // sizeof=0x1608
{                                       // ...
    AimTarget targets[64];
    int32_t targetCount;
    AimTarget clientTargets[64];
    int32_t clientTargetCount;
};
static_assert(sizeof(AimTargetGlob) == 0x1608);

struct AimTweakables // sizeof=0x20
{                                       // ...
    float slowdownRegionWidth;
    float slowdownRegionHeight;
    float autoAimRegionWidth;
    float autoAimRegionHeight;
    float autoMeleeRegionWidth;
    float autoMeleeRegionHeight;
    float lockOnRegionWidth;
    float lockOnRegionHeight;
};
static_assert(sizeof(AimTweakables) == 0x20);

struct AimScreenTarget // sizeof=0x34
{                                       // ...
    int32_t entIndex;                       // ...
    float clipMins[2];                  // ...
    float clipMaxs[2];                  // ...
    float aimPos[3];                    // ...
    float velocity[3];                  // ...
    float distSqr;                      // ...
    float crosshairDistSqr;             // ...
};
static_assert(sizeof(AimScreenTarget) == 0x34);

struct AimAssistGlobals // sizeof=0xE34
{                                       // ...
    bool initialized;
    // padding byte
    // padding byte
    // padding byte
    AimTweakables tweakables;
    float viewOrigin[3];
    float viewAngles[3];
    float viewAxis[3][3];
    float fovTurnRateScale;
    float fovScaleInv;
    float adsLerp;
    float pitchDelta;
    float yawDelta;
    float screenWidth;
    float screenHeight;
    float screenMtx[4][4];
    float invScreenMtx[4][4];
    AimScreenTarget screenTargets[64];
    int32_t screenTargetCount;
    int32_t autoAimTargetEnt;
    bool autoAimPressed;
    bool autoAimActive;
    // padding byte
    // padding byte
    float autoAimPitch;
    float autoAimPitchTarget;
    float autoAimYaw;
    float autoAimYawTarget;
    int32_t autoMeleeTargetEnt;
    bool autoMeleeActive;
    bool autoMeleePressed;
    // padding byte
    // padding byte
    float autoMeleePitch;
    float autoMeleePitchTarget;
    float autoMeleeYaw;
    float autoMeleeYawTarget;
    int32_t lockOnTargetEnt;
};
static_assert(sizeof(AimAssistGlobals) == 0xE34);

struct AimInput // sizeof=0x30
{                                       // ...
    float deltaTime;                    // ...
    float pitch;                        // ...
    float pitchAxis;                    // ...
    float pitchMax;                     // ...
    float yaw;                          // ...
    float yawAxis;                      // ...
    float yawMax;                       // ...
    float forwardAxis;                  // ...
    float rightAxis;                    // ...
    int32_t buttons;                        // ...
    int32_t localClientNum;                 // ...
    const struct playerState_s *ps;            // ...
};
static_assert(sizeof(AimInput) == 0x30);

struct AimOutput // sizeof=0x10
{                                       // ...
    float pitch;                        // ...
    float yaw;                          // ...
    float meleeChargeYaw;               // ...
    uint8_t meleeChargeDist;    // ...
    // padding byte
    // padding byte
    // padding byte
};
static_assert(sizeof(AimOutput) == 0x10);

void __cdecl TRACK_aim_assist();
void __cdecl AimAssist_Init(int32_t localClientNum);
void AimAssist_RegisterDvars();
void __cdecl AimAssist_Setup(int32_t localClientNum);
void __cdecl AimAssist_UpdateScreenTargets(
    int32_t localClientNum,
    const float *viewOrg,
    const float *viewAngles,
    float tanHalfFovX,
    float tanHalfFovY);
void __cdecl AimAssist_FovScale(AimAssistGlobals *aaGlob, float tanHalfFovY);
void __cdecl AimAssist_CreateScreenMatrix(AimAssistGlobals *aaGlob, float tanHalfFovX, float tanHalfFovY);
char __cdecl AimAssist_ConvertToClipBounds(
    const AimAssistGlobals *aaGlob,
    const float (*bounds)[3],
    const mat4x3& mtx,
    float (*clipBounds)[3]);
char __cdecl AimAssist_XfmWorldPointToClipSpace(const AimAssistGlobals *aaGlob, const float *in, float *out);
double __cdecl AimAssist_GetCrosshairDistSqr(const float *clipMins, const float *clipMaxs);
void __cdecl AimAssist_AddToTargetList(AimAssistGlobals *aaGlob, const AimScreenTarget *screenTarget);
int32_t __cdecl AimAssist_CompareTargets(const AimScreenTarget *screenTargetA, const AimScreenTarget *screenTargetB);
int32_t __cdecl AimAssist_CalcAimPos(
    int32_t localClientNum,
    const centity_s *targetEnt,
    const AimTarget *target,
    float *aimPos);
int32_t __cdecl AimTarget_GetTagPos(int32_t localClientNum, const centity_s *cent, uint32_t tagName, float *pos);
void __cdecl AimTarget_GetTagPos(const centity_s *ent, uint32_t tagName, float *pos);
int32_t __cdecl AimAssist_GetScreenTargetCount(int32_t localClientNum);
int32_t __cdecl AimAssist_GetScreenTargetEntity(int32_t localClientNum, uint32_t targetIndex);
void __cdecl AimAssist_ClearEntityReference(int32_t localClientNum, int32_t entIndex);
void __cdecl AimAssist_UpdateTweakables(const AimInput *input);
void __cdecl AimAssist_UpdateAdsLerp(const AimInput *input);
uint32_t __cdecl AimAssist_GetWeaponIndex(int32_t localClientNum, const playerState_s *ps);
const AimScreenTarget *__cdecl AimAssist_GetBestTarget(
    const AimAssistGlobals *aaGlob,
    float range,
    float regionWidth,
    float regionHeight);
bool __cdecl AimAssist_DoBoundsIntersectCenterBox(
    const float *clipMins,
    const float *clipMaxs,
    float clipHalfWidth,
    float clipHalfHeight);
const AimScreenTarget *__cdecl AimAssist_GetTargetFromEntity(const AimAssistGlobals *aaGlob, int32_t entIndex);
void __cdecl AimAssist_ApplyAutoMelee(const AimInput *input, AimOutput *output);
void __cdecl AimAssist_ClearAutoMeleeTarget(AimAssistGlobals *aaGlob);
char __cdecl AimAssist_UpdateAutoMeleeTarget(AimAssistGlobals *aaGlob);
void __cdecl AimAssist_SetAutoMeleeTarget(AimAssistGlobals *aaGlob, const AimScreenTarget *screenTarget);
void __cdecl AimAssist_ApplyMeleeCharge(const AimInput *input, AimOutput *output);
void __cdecl AimAssist_UpdateMouseInput(const AimInput *input, AimOutput *output);
void __cdecl AimAssist_DrawDebugOverlay(uint32_t localClientNum);
void __cdecl AimAssist_DrawCenterBox(
    const AimAssistGlobals *aaGlob,
    float clipHalfWidth,
    float clipHalfHeight,
    const float *color);
void __cdecl AimAssist_DrawTargets(int64_t localClientNum, const float *color);


// aim_target_mp
struct cg_s;
int32_t __cdecl AimTarget_GetTagPos(int32_t localClientNum, const centity_s *cent, uint32_t tagName, float *pos);
void __cdecl TRACK_aim_target();
void __cdecl AimTarget_Init(int32_t localClientNum);
const dvar_s *AimTarget_RegisterDvars();
void __cdecl AimTarget_ClearTargetList(int32_t localClientNum);
void __cdecl AimTarget_ProcessEntity(int32_t localClientNum, const centity_s *ent);
char __cdecl AimTarget_IsTargetValid(const cg_s *cgameGlob, const centity_s *targetEnt);
double __cdecl AimTarget_GetTargetRadius(const centity_s *targetEnt);
void __cdecl AimTarget_GetTargetBounds(const centity_s *targetEnt, float *mins, float *maxs);
char __cdecl AimTarget_IsTargetVisible(int32_t localClientNum, const centity_s *targetEnt, uint32_t visBone);
void __cdecl AimTarget_GetTargetCenter(const centity_s *targetEnt, float *center);
void __cdecl AimTarget_CreateTarget(int32_t localClientNum, const centity_s *targetEnt, AimTarget *target);
void __cdecl AimTarget_AddTargetToList(int32_t localClientNum, const AimTarget *target);
int __cdecl AimTarget_CompareTargets(const AimTarget *targetA, const AimTarget *targetB);
bool __cdecl AimTarget_PlayerInValidState(const playerState_s *ps);
void __cdecl AimTarget_UpdateClientTargets(int32_t localClientNum);
void __cdecl AimTarget_GetClientTargetList(int32_t localClientNum, AimTarget **targetList, int32_t*targetCount);
