#pragma once
#include <gfx_d3d/fxprimitives.h>

struct clientControllers_t // sizeof=0x60
{                                       // ...
    float angles[6][3];
    float tag_origin_angles[3];
    float tag_origin_offset[3];
};
 struct __declspec(align(4)) CEntPlayerInfo // sizeof=0xC
 {                                       // ...
     clientControllers_t *control;       // ...
     unsigned __int8 tag[6];             // ...
     // padding byte
     // padding byte
 };
 struct CEntTurretAngles // sizeof=0x8
 {                                       // ...
     float pitch;
     float yaw;
 };
 union $06707BB8B07567CD0B9AB50B15101D6E // sizeof=0x8
 {                                       // ...
     CEntTurretAngles angles;
     const float *viewAngles;
 };
 struct CEntTurretInfo // sizeof=0x10
 {                                       // ...
     $06707BB8B07567CD0B9AB50B15101D6E ___u0;
     float barrelPitch;
     bool playerUsing;
     unsigned __int8 tag_aim;
     unsigned __int8 tag_aim_animated;
     unsigned __int8 tag_flash;
 };
 struct __declspec(align(2)) CEntVehicleInfo // sizeof=0x24
 {                                       // ...
     __int16 pitch;
     __int16 yaw;
     __int16 roll;
     __int16 barrelPitch;
     float barrelRoll;
     __int16 steerYaw;
     // padding byte
     // padding byte
     float time;
     unsigned __int16 wheelFraction[4];
     unsigned __int8 wheelBoneIndex[4];
     unsigned __int8 tag_body;
     unsigned __int8 tag_turret;
     unsigned __int8 tag_barrel;
     // padding byte
 };
 
 struct CEntFx // sizeof=0x8
 {                                       // ...
     int triggerTime;
     FxEffect *effect;
 };

 struct GfxSkinCacheEntry // sizeof=0xC
 {                                       // ...
     unsigned int frameCount;
     int skinnedCachedOffset;
     unsigned __int16 numSkinnedVerts;
     unsigned __int16 ageCount;
 };

struct cpose_t // sizeof=0x64
{                                       // ...
    unsigned __int16 lightingHandle;
    unsigned __int8 eType;
    unsigned __int8 eTypeUnion;
    unsigned __int8 localClientNum;
    // padding byte
    // padding byte
    // padding byte
    int cullIn;
    unsigned __int8 isRagdoll;
    // padding byte
    // padding byte
    // padding byte
    int ragdollHandle;
    int killcamRagdollHandle;
    int physObjId;
    float origin[3];
    float angles[3];
    GfxSkinCacheEntry skinCacheEntry;
    //$9D88A49AD898204B3D6E378457DD8419 ___u12;
    union //$9D88A49AD898204B3D6E378457DD8419 // sizeof=0x24
    {                                       // ...
        CEntPlayerInfo player;
        CEntTurretInfo turret;
        CEntVehicleInfo vehicle;
        CEntFx fx;
    };
};

struct centity_s // sizeof=0x1DC
{                                       // ...
    cpose_t pose;
    LerpEntityState currentState;
    entityState_s nextState;
    bool nextValid;
    bool bMuzzleFlash;
    bool bTrailMade;
    // padding byte
    int previousEventSequence;
    int miscTime;
    float lightingOrigin[3];
    XAnimTree_s *tree;
};

void __cdecl CG_DrawRotatedPicPhysical(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    float width,
    float height,
    float angle,
    const float *color,
    Material *material);
void __cdecl CG_DrawRotatedPic(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    float width,
    float height,
    int horzAlign,
    int vertAlign,
    float angle,
    const float *color,
    Material *material);
void __cdecl CG_DrawRotatedQuadPic(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    const float (*verts)[2],
    float angle,
    const float *color,
    Material *material);
void __cdecl CG_DrawVLine(
    const ScreenPlacement *scrPlace,
    float x,
    float top,
    float lineWidth,
    float height,
    int horzAlign,
    int vertAlign,
    const float *color,
    Material *material);
void __cdecl CG_DrawStringExt(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    char *string,
    const float *setColor,
    int forceColor,
    int shadow,
    float charHeight);
int __cdecl CG_DrawDevString(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    float xScale,
    float yScale,
    char *s,
    const float *color,
    char align,
    Font_s *font);
int __cdecl CG_DrawBigDevString(const ScreenPlacement *scrPlace, float x, float y, char *s, float alpha, char align);
int __cdecl CG_DrawBigDevStringColor(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    char *s,
    const float *color,
    char align);
int __cdecl CG_DrawSmallDevStringColor(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    char *s,
    const float *color,
    char align);
double __cdecl CG_FadeAlpha(int timeNow, int startMsec, int totalMsec, int fadeMsec);
float *__cdecl CG_FadeColor(int timeNow, int startMsec, int totalMsec, int fadeMsec);
void __cdecl CG_MiniMapChanged(int localClientNum);
void __cdecl CG_NorthDirectionChanged(int localClientNum);
void __cdecl CG_DebugLine(const float *start, const float *end, const float *color, int depthTest, int duration);
void __cdecl CG_DebugStar(const float *point, const float *color, int duration);
void __cdecl CG_DebugStarWithText(
    const float *point,
    const float *starColor,
    const float *textColor,
    char *string,
    float fontsize,
    int duration);
void __cdecl CG_DebugBox(
    const float *origin,
    const float *mins,
    const float *maxs,
    float yaw,
    const float *color,
    int depthTest,
    int duration);
void __cdecl CG_DebugBoxOriented(
    const float *origin,
    const float *mins,
    const float *maxs,
    float (*rotation)[3],
    const float *color,
    int depthTest,
    int duration);
void __cdecl MatrixTransformVector(const float *in1, const float (*in2)[3], float *out);
void __cdecl CG_DebugCircle(
    const float *center,
    float radius,
    const float *dir,
    const float *color,
    int depthTest,
    int duration);
void __cdecl CG_TeamColor(int team, const char *prefix, float *color);
void __cdecl CG_RelativeTeamColor(int clientNum, const char *prefix, float *color, int localClientNum);



// cg_hudelem
struct cg_hudelem_t // sizeof=0x238
{                                       // ...
    float x;                            // ...
    float y;
    float width;
    float height;
    char hudElemLabel[256];             // ...
    float labelWidth;                   // ...
    char hudElemText[256];              // ...
    float textWidth;
    Font_s *font;
    float fontScale;
    float fontHeight;
    float color[4];                     // ...
    int timeNow;                        // ...
};
void __cdecl CG_HudElemRegisterDvars();
void __cdecl CG_TranslateHudElemMessage(
    int localClientNum,
    const char *message,
    const char *messageType,
    char *hudElemString);
char __cdecl ReplaceDirective(int localClientNum, unsigned int *searchPos, unsigned int *dstLen, char *dstString);
void __cdecl GetHudelemDirective(int localClientNum, char *directive, char *result);
void __cdecl DirectiveFakeIntroSeconds(int localClientNum, const char *arg0, char *result);
void __cdecl ParseDirective(char *directive, char *resultName, char *resultArg0);
void __cdecl CG_Draw2dHudElems(int localClientNum, int foreground);
void __cdecl DrawSingleHudElem2d(int localClientNum, const hudelem_s *elem);
void __cdecl GetHudElemInfo(int localClientNum, const hudelem_s *elem, cg_hudelem_t *cghe, char *hudElemString);
void __cdecl SafeTranslateHudElemString(int localClientNum, int index, char *hudElemString);
double __cdecl HudElemStringWidth(const char *string, const cg_hudelem_t *cghe);
char *__cdecl HudElemTimerString(const hudelem_s *elem, int timeNow);
int __cdecl GetHudElemTime(const hudelem_s *elem, int timeNow);
char *__cdecl HudElemTenthsTimerString(const hudelem_s *elem, int timeNow);
double __cdecl HudElemWidth(const ScreenPlacement *scrPlace, const hudelem_s *elem, const cg_hudelem_t *cghe);
double __cdecl HudElemMaterialWidth(const ScreenPlacement *scrPlace, const hudelem_s *elem, const cg_hudelem_t *cghe);
double __cdecl HudElemMaterialSpecifiedWidth(
    const ScreenPlacement *scrPlace,
    char alignScreen,
    int sizeVirtual,
    const cg_hudelem_t *cghe);
double __cdecl HudElemHeight(const ScreenPlacement *scrPlace, const hudelem_s *elem, const cg_hudelem_t *cghe);
double __cdecl HudElemMaterialHeight(const ScreenPlacement *scrPlace, const hudelem_s *elem, const cg_hudelem_t *cghe);
double __cdecl HudElemMaterialSpecifiedHeight(
    const ScreenPlacement *scrPlace,
    char alignScreen,
    int sizeVirtual,
    const cg_hudelem_t *cghe);
void __cdecl SetHudElemPos(const ScreenPlacement *scrPlace, const hudelem_s *elem, cg_hudelem_t *cghe);
void __cdecl GetHudElemOrg(
    const ScreenPlacement *scrPlace,
    int alignOrg,
    int alignScreen,
    float xVirtual,
    float yVirtual,
    float width,
    float height,
    float *orgX,
    float *orgY);
double __cdecl AlignHudElemX(int alignOrg, float x, float width);
double __cdecl AlignHudElemY(int alignOrg, float y, float height);
double __cdecl HudElemMovementFrac(const hudelem_s *elem, int timeNow);
void __cdecl ConsolidateHudElemText(cg_hudelem_t *cghe, char *hudElemString);
void __cdecl CopyStringToHudElemString(char *string, char *hudElemString);
void __cdecl HudElemColorToVec4(const hudelem_color_t *hudElemColor, float *resultColor);
void __cdecl DrawHudElemString(
    unsigned int localClientNum,
    const ScreenPlacement *scrPlace,
    char *text,
    const hudelem_s *elem,
    cg_hudelem_t *cghe);
double __cdecl OffsetHudElemY(const hudelem_s *elem, const cg_hudelem_t *cghe, float offsetY);
void __cdecl DrawHudElemClock(int localClientNum, const hudelem_s *elem, const cg_hudelem_t *cghe);
void __cdecl DrawHudElemMaterial(int localClientNum, const hudelem_s *elem, cg_hudelem_t *cghe);
void __cdecl DrawOffscreenViewableWaypoint(int localClientNum, const hudelem_s *elem);
char __cdecl WorldPosToScreenPos(int localClientNum, const float *worldPos, float *outScreenPos);
bool __cdecl ClampScreenPosToEdges(
    int localClientNum,
    float *point,
    float padLeft,
    float padRight,
    float padTop,
    float padBottom,
    float *resultNormal,
    float *resultDist);
double __cdecl GetScaleForDistance(int localClientNum, const float *worldPos);
int __cdecl GetSortedHudElems(int localClientNum, hudelem_s **elems);
void __cdecl CopyInUseHudElems(hudelem_s **elems, int *elemCount, hudelem_s *elemSrcArray, int elemSrcArrayCount);
int __cdecl compare_hudelems(const void *pe0, const void *pe1);
void __cdecl CG_AddDrawSurfsFor3dHudElems(int localClientNum);
void  AddDrawSurfForHudElemWaypoint(hudelem_color_t a1@<ebp>, int localClientNum, const hudelem_s *elem);
double __cdecl HudElemWaypointHeight(int localClientNum, const hudelem_s *elem);
