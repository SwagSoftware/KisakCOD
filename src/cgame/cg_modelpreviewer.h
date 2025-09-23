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

enum MdlPrvFreeSpeed : __int32
{
    FREESPEED_NORMAL = 0x0,
    FREESPEED_SLOW = 0x1,
    FREESPEED_FAST = 0x2,
};

enum ModPrvUiModePC : __int32
{
    SELECTION_MODE = 0x0,
    MOVE_MODE = 0x1,
    ROTATE_MODE = 0x2,
    SCALE_MODE = 0x3,
};

enum MdlPrvUiModeGamepad : __int32
{
    MDLPRVMODE_FOCUSED = 0x0,
    MDLPRVMODE_FREE = 0x1,
    MDLPRVMODE_WALKABOUT = 0x2,
};

struct MdlPrvClone
{
    GfxSceneEntity ent;
    DObj_s *obj;
    cpose_t pose;
    char objBuf[100];
};

enum MdlPrvFocusedMode : __int32
{
    FOCUSEDMODE_CAMERA = 0x0,
    FOCUSEDMODE_MODELROTATE = 0x1,
    FOCUSEDMODE_MODELMOVE = 0x2,
    FOCUSEDMODE_FOCALMOVE = 0x3,
};

enum MdlPrvMRotCamMode : __int32
{
    MROTCAMMODE_STATIC = 0x0,
    MROTCAMMODE_TRAVEL = 0x1,
};

struct MdlPrvBtnTimes
{
    int mode;
    int mdlRotMode;
    int camUp;
    int camDown;
    int dropToFloor;
    int clone;
    int clearClones;
    int walkabout;
    int freeSpeed;
    int ragdollSpeed;
};

struct __declspec(align(4)) ModelPreviewer_System
{
    int modelCount;
    const char **modelNames;
    int animCount;
    const char **animNames;
    bool cachedAllModels;
    bool startedCaching;
    int cachedModelCount;
    int lastLoadModel;
    int lastMatReplace;
    ModPrvUiModePC uiModePC;
    float gamePadRStickDeflect;
    MdlPrvUiModeGamepad uiModeGPad;
    MdlPrvFocusedMode focusedMode;
    MdlPrvMRotCamMode modelRotCamMode;
    MdlPrvBtnTimes buttonTimes;
    bool walkaboutActive;
};

struct ModelPreviewer_Viewer
{
    float vertical;
    float horizontal;
    float centerRadius;
    float zNear;
    float zNearChangeLimit;
    MdlPrvFreeSpeed freeModeSpeed;
    float freeModeOrigin[3];
    float freeModeAngles[3];
};

struct ModelPreviewer_Material
{
    int handleCount;
    Material **handleArray;
    Material **surfMatHandles;
    const char *nameTable[66];
    int replaceIndex;
    Material *prevReplaced;
    int selectSliderIndex;
    int replaceSliderIndex;
};

struct ModelPreviewer_Light
{
    int setupCount;
    const char *nameTable[18];
    SunLightParseParams parsedSunLight[16];
    SunLightParseParams tweakableSunLight;
};

struct ModelPreviewer_Anim
{
    int fromCurrentIndex;
    int toCurrentIndex;
    bool isAnimPlaying;
    const dvar_s *mruNames[4];
    const char *mruNameTable[5];
    bool isToAnimPlaying;
    float stepCounter;
    bool isFromLooped;
    bool isToLooped;
    __int16 fromSliderID;
    __int16 toSliderID;
    __int16 fromMRUSliderID;
    __int16 toMRUSliderID;
    float deltaYaw;
};

struct ModelPreviewer_Model
{
    bool inited;
    int currentIndex;
    GfxSceneEntity currentEntity;
    cpose_t pose;
    DObj_s *currentObj;
    char objBuf[100];
    float initialOrigin[3];
    float initialYaw;
    int lodDist[4];
    int surfaceCount;
    __int16 boneInfoSliderID;
    __int16 loadSliderID;
    const dvar_s *mruNames[4];
    const char *mruNameTable[5];
    const char *boneNameTable[133];
    MdlPrvClone clones[100];
    int cloneNextIdx;
    int ragdoll;
    int ragdollDef;
};

struct ModelPreviewer
{
    bool inited;
    ModelPreviewer_System system;
    ModelPreviewer_Viewer viewer;
    ModelPreviewer_Model model;
    ModelPreviewer_Anim anim;
    ModelPreviewer_Material mat;
    ModelPreviewer_Light light;
};

void __cdecl CG_ModPrvUpdateMru(const dvar_s **mruDvars, const char **stringTable, const dvar_s *dvar);
void __cdecl CG_ModPrvPushMruEntry(const char *entry, const dvar_s **mruDvars, const char **stringTable, const dvar_s *dvar);
void __cdecl CG_ModPrvRemoveMruEntry(const dvar_s **mruDvars, const char **stringTable, const dvar_s *dvar);
void CG_ModPrvRegisterDvars();
void __cdecl CG_ModPrvSetEntityAxis(float *angles, float *quat);
void __cdecl MdlPrvGetBounds(float *mins, float *maxs, float *center);
void CG_ModPrvFrameModel();
void CG_ModPrvResetOrientation_f();
void __cdecl SetViewerActive(bool active);
void CG_ModPrvUnregisterCmds();
void *__cdecl CG_ModPrvAlloc(int size);
void __cdecl CG_ModPrvFree(void *allocated, int size);
void CG_ModPrvResetGlobals();
bool __cdecl CG_ModPrvCompareString(const char *string1, const char *string2);
void __cdecl CG_ModPrvGetAssetName(const XModel *header, unsigned int *data);
void CG_ModPrvUnloadModel();
void CG_ModPrvShutdown();
void __cdecl CG_ModPrvDrawViewAxis(const float *centerPos);
void CG_ModPrvOriginUpdate();
void CG_ModPrvRotateUpdate();
void CG_ModPrvModelResetRotation();
void CG_ModPrvModelResetRotationXY();
int __cdecl CG_ModPrvGetNumTotalBones(DObj_s *dobj);
int __cdecl CG_ModPrvGetNumSurfaces(DObj_s *obj, int lod);
const char *__cdecl CG_ModPrvModelGetBoneName(DObj_s *dobj, int modelIndex, int boneIndex);
void CG_ModPrvDrawBones();
void __cdecl CG_ModPrvLoadAnimations(const char *animationFilename);
void __cdecl CG_ModPrvApplyAnimationBlend(double deltaTime);
void __cdecl CG_ModPrvApplyDelta(double deltaTime);
int __cdecl CG_ModPrvLoopAnimation();
void __cdecl CG_ModPrvAnimRecentAccept(const dvar_s *dvar, int *currentIndex);
void __cdecl CG_ModPrvLoadAnimAccept(const dvar_s *dvar, int *currentIndex);
XAnimTree_s *CG_ModPrvAnimBlendWeightUpdate();
void CG_ModPrvMatReplaceAccepted();
void CG_ModPrvMatReplaceUpdate();
void CG_ModPrvLightSetupModified();
bool __cdecl CG_ModPrvAnyLightValuesChanged();
void CG_ModPrvLightValuesUpdate();
void __cdecl TRACK_cg_modelpreviewer();
void __cdecl CG_ModelPreviewerPauseAnim();
void __cdecl CG_ModelPreviewerStepAnim(float deltaTime);
//int __cdecl MdlPrvPrint(double x, double y, const char *txt, const char *a4, const float *a5, int a6, Font_s *a7);
//int __cdecl MdlPrvPrintColor(
//    double x,
//    double y,
//    const char *txt,
//    const float *color,
//    const float *a5,
//    int a6,
//    Font_s *a7);
void __cdecl MdlPrvPrintHelpLine(ButtonNames idx, float vPos);
void DrawDistFromModel();
void __cdecl MdlPrvDrawOverlayGamepad();
void __cdecl CG_ModelPreviewerDrawInfo();
void __cdecl CG_ModelPreviewerRotateCamera(double dx, double dy);
void __cdecl CG_ModelPreviewerZoomCamera(double dx, double dy);
void __cdecl MdlPrvModelOriginSet(float *origin);
void __cdecl MdlPrvModelOriginOffset(double dx, double dy, double dz);
void __cdecl MdlPrvSpin_(unsigned int yprIdx, double deg);
void __cdecl MdlPrvSpinYaw(double deg);
void __cdecl MdlPrvSpinPitch(double deg);
void __cdecl MdlPrvSpinRoll(double deg);
void __cdecl MdlPrvSpinYawOffset(double deg);
void MdlPrvSpinClearPitchRoll();
void __cdecl MdlPrvMoveModelUpDown(double dist);
void __cdecl MdlPrvMoveModel2D(const cg_s *cgGlob, float away, float left);
void __cdecl MdlPrvMoveFocusUpDown(double dist);
void __cdecl MdlPrvMoveFocus2D(const cg_s *cgGlob, double away, double left);
void MdlPrvMoveFocusReset();
void __cdecl MdlPrvFreeMove(const cg_s *cgGlob, double dx, double dy);
void __cdecl MdlPrvFreeMoveVertical(const cg_s *cgGlob, double dz);
void __cdecl MdlPrvFreeRot(double yaw, double pitch);
void __cdecl MdlPrvFreePlaceModel(float *pos);
void __cdecl MdlPrvFreePlaceModelInFrontCamera(const cg_s *cgGlob);
void MdlPrvModeToggle();
void MdlPrvRotModeToggle();
void MdlPrvDropToFloor();
void __cdecl MdlPrvCloneClear(MdlPrvClone *clone);
void MdlPrvCloneClearAll();
void __cdecl MdlPrvCloneModel(const cg_s *cgGlob);
void MdlPrvFreeSpeedToggle();
void MdlPrvRagdollToggle();
void __cdecl MdlPrvControlsGamepad(int localClientNum, double forward, double side, double pitch, double yaw);
void __cdecl CG_ModelPreviewerHandleGamepadEvents(int localClientNum, double forward, double side, double pitch, double yaw);
void __cdecl CG_ModelPreviewerHandleKeyEvents(int localClientNum, int key, int down, unsigned int time);
void __cdecl MdlPrvUpdateViewFocused(float *viewOrigin, float (*viewAxis)[3], float *viewAngles, float *zNear);
void __cdecl MdlPrvUpdateViewFree(float *viewOrigin, float (*viewAxis)[3], float *viewAngles, float *zNear);
void __cdecl CG_ModelPreviewerUpdateView(float *viewOrigin, float (*viewAxis)[3], float *viewAngles, float *zNear);
bool __cdecl CG_ModelPreviewerNeedsVieworgInterpSkipped(int localClientNum);
void __cdecl CG_AddModelPreviewerModel(int frametime);
void __cdecl CG_ModelPreviewerDestroyDevGui();
void __cdecl CG_ModelPreviewerBuildInfoStr(char *buffer, int bufferSize);
void __cdecl CG_ModelPreviewerBuildViewPosStr(char *buffer, int bufferSize);
void __cdecl CG_ModPrvSaveDObjs();
void __cdecl CG_ModPrvLoadDObjs();
void CG_ModPrvExit_f();
void CG_ModPrvRegisterCmds();
void CG_ModPrvModelGetBoneNameList();
void __cdecl CG_ModPrvLoadModel(const cg_s *cgameGlob, const char *modelFilename);
void __cdecl CG_ModPrvModelRecentAccepted(const cg_s *cgameGlob);
void __cdecl CG_ModelPreviewerFrame(const cg_s *cgameGlob);
void CG_ModPrvEnumerateModels_FastFile();
void CG_ModPrvEnumerateModels();
void CG_ModPrvEnumerateAnimations_FastFile();
void CG_ModPrvEnumerateAnimations();
void __cdecl CG_ModelPreviewerEnumerateAssets();
void __cdecl CG_ModPrvStartup(int localClientNum);
void __cdecl CG_ModelPreviewerCreateDevGui(int localClientNum);
