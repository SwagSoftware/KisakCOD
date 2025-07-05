#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include <universal/q_shared.h>
#include <xanim/dobj.h>
#include <xanim/xanim.h>
#include "cg_local.h"
#include <gfx_d3d/r_scene.h>

enum ButtonNames : __int32
{
    BTN_MODESWITCH = 0x0,
    BTN_DROPMDL = 0x1,
    BTN_WALKABOUT_ENTER = 0x2,
    BTN_WALKABOUT_EXIT = 0x3,
    BTN_FREE_DROPFRONT = 0x4,
    BTN_FREE_DROPPOS = 0x5,
    BTN_FREE_TOGGLEMOVESPEED = 0x6,
    BTN_FREE_TOGGLERAGDOLL = 0x7,
    BTN_FREE_DOWN = 0x8,
    BTN_FREE_UP = 0x9,
    BTN_FREE_UPDOWN = 0xA,
    BTN_FOCUS_TOGGLEMOV = 0xB,
    BTN_FOCUS_TOGGLEROT = 0xC,
    BTN_FOCUS_TOGGLEFOCALMOVE = 0xD,
    BTN_FOCUS_DEFAULT_CLONEMODEL = 0xE,
    BTN_FOCUS_DEFAULT_CLEARCLONES = 0xF,
    BTN_FOCUS_DEFAULT_ZOOM = 0x10,
    BTN_FOCUS_DEFAULT_ORBIT = 0x11,
    BTN_FOCUS_MMOV_2D = 0x12,
    BTN_FOCUS_MMOV_UPDOWN = 0x13,
    BTN_FOCUS_MROT_TOGGLECAM = 0x14,
    BTN_FOCUS_MROT_PITCHROLL = 0x15,
    BTN_FOCUS_MROT_YAW = 0x16,
    BTN_FOCUS_MROT_RESET = 0x17,
    BTN_FOCUS_FOCALMOVE_2D = 0x18,
    BTN_FOCUS_FOCALMOVE_UPDOWN = 0x19,
    BTN_FOCUS_FOCALMOVE_RESET = 0x1A,
    TOTAL_BUTTONNAMES = 0x1B,
};

struct MdlPrvClone
{
    GfxSceneEntity ent;
    DObj_s *obj;
    cpose_t pose;
    char objBuf[100];
};

void __fastcall CG_ModPrvUpdateMru(const dvar_s **mruDvars, const char **stringTable, const dvar_s *dvar);
void __fastcall CG_ModPrvPushMruEntry(
    const char *entry,
    const dvar_s **mruDvars,
    const char **stringTable,
    const dvar_s *dvar);
void __fastcall CG_ModPrvRemoveMruEntry(const dvar_s **mruDvars, const char **stringTable, const dvar_s *dvar);
const dvar_s *CG_ModPrvRegisterDvars();
void __fastcall CG_ModPrvSetEntityAxis(float *angles, float *quat);
void __fastcall MdlPrvGetBounds(float *mins, float *maxs, float *center);
void CG_ModPrvFrameModel();
void CG_ModPrvResetOrientation_f();
// attributes: thunk
void __fastcall SetViewerActive(bool active);
void CG_ModPrvUnregisterCmds();
void *__fastcall CG_ModPrvAlloc(int size);
void __fastcall CG_ModPrvFree(void *allocated, int size);
void CG_ModPrvResetGlobals();
bool __fastcall CG_ModPrvCompareString(const char *string1, const char *string2);
void __fastcall CG_ModPrvGetAssetName(const XModel *header, unsigned int *data);
void CG_ModPrvUnloadModel();
void CG_ModPrvShutdown();
void __fastcall CG_ModPrvDrawViewAxis(const float *centerPos);
void CG_ModPrvOriginUpdate();
void CG_ModPrvRotateUpdate();
void CG_ModPrvModelResetRotation();
void CG_ModPrvModelResetRotationXY();
int __fastcall CG_ModPrvGetNumTotalBones(DObj_s *dobj);
int __fastcall CG_ModPrvGetNumSurfaces(DObj_s *obj, int lod);
const char *__fastcall CG_ModPrvModelGetBoneName(DObj_s *dobj, int modelIndex, int boneIndex);
void CG_ModPrvDrawBones();
void __fastcall CG_ModPrvLoadAnimations(const char *animationFilename);
void __fastcall CG_ModPrvApplyAnimationBlend(double deltaTime);
void __fastcall CG_ModPrvApplyDelta(double deltaTime);
int __fastcall CG_ModPrvLoopAnimation();
void __fastcall CG_ModPrvAnimRecentAccept(const dvar_s *dvar, int *currentIndex);
void __fastcall CG_ModPrvLoadAnimAccept(const dvar_s *dvar, int *currentIndex);
XAnimTree_s *CG_ModPrvAnimBlendWeightUpdate();
void CG_ModPrvMatReplaceAccepted();
void CG_ModPrvMatReplaceUpdate();
void CG_ModPrvLightSetupModified();
bool __fastcall CG_ModPrvAnyLightValuesChanged();
void CG_ModPrvLightValuesUpdate();
void __fastcall TRACK_cg_modelpreviewer();
void __fastcall CG_ModelPreviewerPauseAnim();
void __fastcall CG_ModelPreviewerStepAnim(double deltaTime);
int __fastcall MdlPrvPrint(double x, double y, const char *txt, const char *a4, const float *a5, int a6, Font_s *a7);
int __fastcall MdlPrvPrintColor(
    double x,
    double y,
    const char *txt,
    const float *color,
    const float *a5,
    int a6,
    Font_s *a7);
int __fastcall MdlPrvPrintHelpLine(ButtonNames idx, double vPos, const char *a3, const float *a4, int a5);
void DrawDistFromModel();
void __fastcall MdlPrvDrawOverlayGamepad();
// attributes: thunk
void __fastcall CG_ModelPreviewerDrawInfo();
void __fastcall CG_ModelPreviewerRotateCamera(double dx, double dy);
void __fastcall CG_ModelPreviewerZoomCamera(double dx, double dy);
void __fastcall MdlPrvModelOriginSet(float *origin);
void __fastcall MdlPrvModelOriginOffset(double dx, double dy, double dz);
void __fastcall MdlPrvSpin_(unsigned int yprIdx, double deg);
void __fastcall MdlPrvSpinYaw(double deg);
void __fastcall MdlPrvSpinPitch(double deg);
void __fastcall MdlPrvSpinRoll(double deg);
void __fastcall MdlPrvSpinYawOffset(double deg);
void MdlPrvSpinClearPitchRoll();
void __fastcall MdlPrvMoveModelUpDown(double dist);
void __fastcall MdlPrvMoveModel2D(const cg_s *cgGlob, double away, double left);
void __fastcall MdlPrvMoveFocusUpDown(double dist);
void __fastcall MdlPrvMoveFocus2D(const cg_s *cgGlob, double away, double left);
void MdlPrvMoveFocusReset();
void __fastcall MdlPrvFreeMove(const cg_s *cgGlob, double dx, double dy);
void __fastcall MdlPrvFreeMoveVertical(const cg_s *cgGlob, double dz);
// local variable allocation has failed, the output may be wrong!
void __fastcall MdlPrvFreeRot(double yaw, double pitch);
// attributes: thunk
void __fastcall MdlPrvFreePlaceModel(float *pos);
void __fastcall MdlPrvFreePlaceModelInFrontCamera(const cg_s *cgGlob);
void MdlPrvModeToggle();
void MdlPrvRotModeToggle();
void MdlPrvDropToFloor();
void __fastcall MdlPrvCloneClear(MdlPrvClone *clone);
void MdlPrvCloneClearAll();
void __fastcall MdlPrvCloneModel(const cg_s *cgGlob);
void MdlPrvFreeSpeedToggle();
void MdlPrvRagdollToggle();
void __fastcall MdlPrvControlsGamepad(int localClientNum, double forward, double side, double pitch, double yaw);
// attributes: thunk
void __fastcall CG_ModelPreviewerHandleGamepadEvents(
    int localClientNum,
    double forward,
    double side,
    double pitch,
    double yaw);
void __fastcall CG_ModelPreviewerHandleKeyEvents(int localClientNum, int key, int down, unsigned int time);
void __fastcall MdlPrvUpdateViewFocused(float *viewOrigin, float (*viewAxis)[3], float *viewAngles, float *zNear);
void __fastcall MdlPrvUpdateViewFree(float *viewOrigin, float (*viewAxis)[3], float *viewAngles, float *zNear);
void __fastcall CG_ModelPreviewerUpdateView(float *viewOrigin, float (*viewAxis)[3], float *viewAngles, float *zNear);
bool __fastcall CG_ModelPreviewerNeedsVieworgInterpSkipped(int localClientNum);
void __fastcall CG_AddModelPreviewerModel(int frametime, int a2, int a3, int a4, int a5, int a6, __int64 a7);
void __fastcall CG_ModelPreviewerDestroyDevGui();
void __fastcall CG_ModelPreviewerBuildInfoStr(char *buffer, int bufferSize);
void __fastcall CG_ModelPreviewerBuildViewPosStr(char *buffer, int bufferSize);
void __fastcall CG_ModPrvSaveDObjs();
void __fastcall CG_ModPrvLoadDObjs();
void CG_ModPrvExit_f();
void CG_ModPrvRegisterCmds();
void CG_ModPrvModelGetBoneNameList();
void __fastcall CG_ModPrvLoadModel(
    const cg_s *cgameGlob,
    const char *modelFilename,
    double a3,
    double a4,
    double a5,
    double a6,
    double a7,
    double a8,
    double a9,
    double a10,
    int a11,
    int a12,
    int a13,
    int a14,
    int a15,
    int a16,
    int a17,
    int a18,
    int a19,
    int a20,
    int a21,
    int a22,
    int a23,
    int a24,
    int a25,
    int a26,
    int a27,
    int a28,
    int a29,
    int a30,
    int a31,
    int a32,
    int a33,
    int a34,
    XModel *a35,
    int a36,
    float value,
    float a38,
    float a39,
    int a40,
    int a41,
    int a42,
    int a43);
void __fastcall CG_ModPrvModelRecentAccepted(const cg_s *cgameGlob);
void __fastcall CG_ModelPreviewerFrame(const cg_s *cgameGlob);
void CG_ModPrvEnumerateModels_FastFile();
// attributes: thunk
void CG_ModPrvEnumerateModels();
void CG_ModPrvEnumerateAnimations_FastFile();
// attributes: thunk
void CG_ModPrvEnumerateAnimations();
void __fastcall CG_ModelPreviewerEnumerateAssets();
void __fastcall CG_ModPrvStartup(int localClientNum);
void __fastcall CG_ModelPreviewerCreateDevGui(int localClientNum);
