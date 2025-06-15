#pragma once

#include <bgame/bg_local.h>

#include <gfx_d3d/fxprimitives.h>
#include <gfx_d3d/r_gfx.h>
#include <gfx_d3d/r_material.h>

struct cg_s;

struct Font_s;
struct Material;
struct ScreenPlacement;

#define ENTITYNUM_NONE 1023
#define SURF_TYPECOUNT 29

#define WEAPON_HINT_OFFSET 4
#define LAST_WEAPON_HINT 132

enum $73E480FCE7B67BAA29FC24DF5A08B1FF : __int32
{
    WEAP_ANIM_VIEWMODEL_START = 0x0,
    WEAP_ANIM_VIEWMODEL_END = 0x1F,
};

enum $53B7CF4E68BA96864516EAE91DEE3467 : __int32
{
    IMPACTEFFECT_HEADSHOT = 0x1,
    IMPACTEFFECT_FATAL = 0x2,
    IMPACTEFFECT_EXIT = 0x4,
};

 enum entityType_t : __int32
 {                                       // ...
     ET_GENERAL         = 0x0,
     ET_PLAYER          = 0x1,
     ET_PLAYER_CORPSE   = 0x2,
     ET_ITEM            = 0x3,
     ET_MISSILE         = 0x4,
     ET_INVISIBLE       = 0x5,
     ET_SCRIPTMOVER     = 0x6,
     ET_SOUND_BLEND     = 0x7,
     ET_FX              = 0x8,
     ET_LOOP_FX         = 0x9,
     ET_PRIMARY_LIGHT   = 0xA,
     ET_MG42            = 0xB,
     ET_HELICOPTER      = 0xC,
     ET_PLANE           = 0xD,
     ET_VEHICLE         = 0xE,
     ET_VEHICLE_COLLMAP = 0xF,
     ET_VEHICLE_CORPSE  = 0x10,
     ET_EVENTS          = 0x11,
 };

const float colorBlack[4] = { 0.0, 0.0, 0.0, 1.0 }; // idb
const float colorRed[4] = { 1.0, 0.0, 0.0, 1.0 }; // idb
const float colorGreen[4] = { 0.0, 1.0, 0.0, 1.0 }; // idb
const float colorLtGreen[4] = { 0.0, 0.69999999f, 0.0, 1.0 }; // idb
const float colorBlue[4] = { 0.0, 0.0, 1.0, 1.0 }; // idb
const float colorLtBlue[4] = { 0.5, 0.5, 1.0, 1.0 }; // idb
const float colorYellow[4] = { 1.0, 1.0, 0.0, 1.0 }; // idb
const float colorLtYellow[4] = { 0.75, 0.75f, 0.0f, 1.0f };
const float colorMdYellow[4] = { 0.5, 0.5, 0.0, 1.0 }; // idb
const float colorMagenta[4] = { 1.0, 0.0, 1.0, 1.0 }; // idb
const float colorCyan[4] = { 0.0f, 1.0f, 1.0f, 1.0f };
const float colorLtCyan[4] = { 0.0, 0.75, 0.75, 1.0 }; // idb
const float colorMdCyan[4] = { 0.0f, 0.5f, 0.5f, 1.0f };
const float colorDkCyan[4] = { 0.0f, 0.25f, 0.25f, 1.0f };
const float colorWhite[4] = { 1.0, 1.0, 1.0, 1.0 }; // idb
const float colorLtGrey[4] = { 0.75, 0.75, 0.75, 1.0 }; // idb
const float colorMdGrey[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
const float colorDkGrey[4] = { 0.25f, 0.25f, 0.25f, 1.0f };
const float colorOrange[4] = { 1.0, 0.69999999f, 0.0, 1.0 }; // idb
const float colorLtOrange[4] = { 0.75, 0.52499998f, 0.0, 1.0 }; // idb
const float colorWhiteFaded[4] = { 1.0, 1.0, 1.0, 0.75 }; // idb
const float colorGreenFaded[4] = { 0.0, 1.0, 0.0, 0.75 }; // idb
const float colorRedFaded[4] = { 0.75, 0.25, 0.0, 0.75 }; // idb


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
    const mat3x3 &rotation,
    const float *color,
    int depthTest,
    int duration);
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
void  AddDrawSurfForHudElemWaypoint(int localClientNum, const hudelem_s *elem);
double __cdecl HudElemWaypointHeight(int localClientNum, const hudelem_s *elem);


// cg_weapons
struct refdef_s;
struct weaponInfo_s // sizeof=0x44
{                                       // ...
    DObj_s *viewModelDObj;
    XModel *handModel;
    XModel *gogglesModel;
    XModel *rocketModel;
    XModel *knifeModel;
    unsigned __int8 weapModelIdx;
    // padding byte
    // padding byte
    // padding byte
    unsigned int partBits[4];
    int iPrevAnim;
    XAnimTree_s *tree;
    int registered;
    const gitem_s *item;
    const char *translatedDisplayName;
    const char *translatedModename;
    const char *translatedAIOverlayDescription;
};
bool __cdecl CG_JavelinADS(int localClientNum);
int __cdecl CG_WeaponDObjHandle(int weaponNum);
void __cdecl CG_RegisterWeapon(int localClientNum, unsigned int weaponNum);
XAnimTree_s *__cdecl CG_CreateWeaponViewModelXAnim(WeaponDef *weapDef);
void __cdecl CG_UpdateWeaponViewmodels(int localClientNum);
void __cdecl ChangeViewmodelDobj(
    int localClientNum,
    unsigned int weaponNum,
    unsigned __int8 weaponModel,
    XModel *newHands,
    XModel *newGoggles,
    XModel *newRocket,
    XModel *newKnife,
    bool updateClientInfo);
void __cdecl CG_UpdateHandViewmodels(int localClientNum, XModel *handModel);
void __cdecl CG_RegisterItemVisuals(int localClientNum, unsigned int weapIdx);
void __cdecl CG_RegisterItems(int localClientNum);
void __cdecl CG_HoldBreathInit(cg_s *cgameGlob);
void __cdecl CG_UpdateViewModelPose(const DObj_s *obj, int localClientNum);
bool __cdecl CG_IsPlayerCrouching(clientInfo_t *ci, const centity_s *cent);
bool __cdecl CG_IsPlayerProne(clientInfo_t *ci, const centity_s *cent);
bool __cdecl CG_IsPlayerADS(clientInfo_t *ci, const centity_s *cent);
void __cdecl CG_GuessSpreadForWeapon(
    int localClientNum,
    const centity_s *cent,
    const WeaponDef *weapDef,
    float *minSpread,
    float *maxSpread);
void __cdecl CG_GetPlayerViewOrigin(int localClientNum, const playerState_s *ps, float *origin);
void __cdecl CG_AddPlayerWeapon(
    int localClientNum,
    const GfxScaledPlacement *placement,
    const playerState_s *ps,
    centity_s *cent,
    int bDrawGun);
void __cdecl WeaponFlash(
    int localClientNum,
    unsigned int dobjHandle,
    unsigned int weaponNum,
    int bViewFlash,
    unsigned int flashTag);
void __cdecl HoldBreathUpdate(int localClientNum);
void __cdecl HoldBreathSoundLerp(int localClientNum, float lerp);
void __cdecl CG_UpdateViewWeaponAnim(int localClientNum);
void __cdecl WeaponRunXModelAnims(int localClientNum, const playerState_s *ps, weaponInfo_s *weapInfo);
void __cdecl StartWeaponAnim(
    int localClientNum,
    unsigned int weaponNum,
    DObj_s *obj,
    int animIndex,
    float transitionTime);
double __cdecl GetWeaponAnimRate(WeaponDef *weapDef, XAnim_s *anims, unsigned int animIndex);
void __cdecl PlayADSAnim(float weaponPosFrac, int weaponNum, DObj_s *obj, int animIndex);
void __cdecl ResetWeaponAnimTrees(int localClientNum, const playerState_s *ps);
char __cdecl UpdateViewmodelAttachments(
    int localClientNum,
    unsigned int weaponNum,
    unsigned __int8 weaponModel,
    weaponInfo_s *weapInfo);
bool __cdecl ViewmodelRocketShouldBeAttached(int localClientNum, WeaponDef *weapDef);
bool __cdecl ViewmodelKnifeShouldBeAttached(int localClientNum, WeaponDef *weapDef);
void __cdecl ProcessWeaponNoteTracks(int localClientNum, const playerState_s *predictedPlayerState);
void __cdecl PlayNoteMappedSoundAliases(int localClientNum, const char *noteName, const WeaponDef *weapDef);
void __cdecl CG_AddViewWeapon(int localClientNum);
void __cdecl CalculateWeaponPosition_Sway(cg_s *cgameGlob);
void __cdecl CalculateWeaponPosition(cg_s *cgameGlob, float *origin);
void __cdecl CalculateWeaponPosition_SwayMovement(const cg_s *cgameGlob, float *origin);
void __cdecl CalculateWeaponPosition_BasePosition(cg_s *cgameGlob, float *origin);
void __cdecl CalculateWeaponPosition_BasePosition_movement(cg_s *cgameGlob, float *origin);
void __cdecl CalculateWeaponPosition_ToWorldPosition(const cg_s *cgameGlob, float *origin);
void __cdecl CalculateWeaponPosition_SaveOffsetMovement(cg_s *cgameGlob, float *origin);
void __cdecl CalculateWeaponPostion_PositionToADS(cg_s *cgameGlob, playerState_s *ps);
void __cdecl CG_NextWeapon_f();
bool __cdecl WeaponCycleAllowed(cg_s *cgameGlob);
void __cdecl CG_PrevWeapon_f();
void __cdecl CG_OutOfAmmoChange(int localClientNum);
char __cdecl VerifyPlayerAltModeWeapon(int localClientNum, const WeaponDef *weapDef);
char __cdecl CycleWeapPrimary(int localClientNum, int cycleForward, int bIgnoreEmpty);
unsigned int __cdecl CG_AltWeaponToggleIndex(int localClientNum, const cg_s *cgameGlob);
int __cdecl NextWeapInCycle(
    int localClientNum,
    const playerState_s *ps,
    weapInventoryType_t type,
    unsigned int startWeaponIndex,
    bool cycleForward,
    bool skipEmpties,
    bool skipHaveNoAlts);
void __cdecl CG_ActionSlotDown_f();
char __cdecl ToggleWeaponAltMode(int localClientNum);
bool __cdecl ActionSlotUsageAllowed(cg_s *cgameGlob);
char __cdecl ActionParms(int *slotResult);
void __cdecl CG_ActionSlotUp_f();
void __cdecl CG_EjectWeaponBrass(int localClientNum, const entityState_s *ent, int event);
void __cdecl CG_FireWeapon(
    int localClientNum,
    centity_s *cent,
    int event,
    unsigned __int16 tagName,
    unsigned int weapon,
    const playerState_s *ps);
void __cdecl DrawBulletImpacts(
    int localClientNum,
    const centity_s *ent,
    const WeaponDef *weaponDef,
    unsigned __int16 boneName,
    const playerState_s *ps);
void __cdecl FireBulletPenetrate(
    int localClientNum,
    BulletFireParams *bp,
    const WeaponDef *weapDef,
    const centity_s *attacker,
    float *tracerStart,
    bool drawTracer);
char __cdecl BulletTrace(
    int localClientNum,
    const BulletFireParams *bp,
    const WeaponDef *weapDef,
    const centity_s *attacker,
    BulletTraceResults *br,
    unsigned int lastSurfaceType);
bool __cdecl ShouldIgnoreHitEntity(int attackerNum, int hitEntNum);
bool __cdecl IsEntityAPlayer(int localClientNum, unsigned int entityNum);
void __cdecl CG_BulletEndpos(
    int randSeed,
    float spread,
    const float *start,
    float *end,
    float *dir,
    const float *forwardDir,
    const float *rightDir,
    const float *upDir,
    float maxRange);
void __cdecl RandomBulletDir(int randSeed, float *x, float *y);
void __cdecl TakeClipOnlyWeaponIfEmpty(int localClientNum, playerState_s *ps);
void __cdecl CG_SpawnTracer(int localClientNum, const float *pstart, const float *pend);
void __cdecl CG_DrawTracer(const float *start, const float *finish, const refdef_s *refdef);
void __cdecl ScaleTracer(
    const float *start,
    const float *finish,
    const float *viewOrg,
    float *startWidth,
    float *finishWidth);
double __cdecl CalcTracerFinalScale(float tracerScaleDistRange, float dist, float tracerScale);
cg_s *__cdecl CG_GetLocalClientGlobalsForEnt(int localClientNum, int entityNum);
void __cdecl CG_GetViewDirection(int localClientNum, int entityNum, float *forward, float *right, float *up);
void __cdecl CG_CalcEyePoint(int localClientNum, int entityNum, float *eyePos);
void __cdecl CG_RandomEffectAxis(const float *forward, float *left, float *up);
void __cdecl CG_ImpactEffectForWeapon(
    unsigned int weaponIndex,
    unsigned int surfType,
    char impactFlags,
    const FxEffectDef **outFx,
    snd_alias_list_t **outSnd);
void __cdecl CG_BulletHitEvent(
    int localClientNum,
    int sourceEntityNum,
    unsigned int targetEntityNum,
    unsigned int weaponIndex,
    float *startPos,
    float *position,
    const float *normal,
    unsigned int surfType,
    int event,
    unsigned __int8 eventParam,
    int damage,
    __int16 hitContents);
int __cdecl CalcMuzzlePoint(int localClientNum, int entityNum, float *muzzle, unsigned int flashTag);
void __cdecl CG_BulletHitEvent_Internal(
    int localClientNum,
    int sourceEntityNum,
    unsigned int targetEntityNum,
    unsigned int weaponIndex,
    float *startPos,
    float *position,
    const float *normal,
    unsigned int surfType,
    int event,
    unsigned __int8 eventParam,
    int damage,
    __int16 hitContents);
void __cdecl BulletTrajectoryEffects(
    int localClientNum,
    int sourceEntityNum,
    float *startPos,
    float *position,
    int surfType,
    unsigned int flashTag,
    unsigned __int8 impactFlags,
    int damage);
void __cdecl WhizbySound(int localClientNum, const float *start, const float *end);
bool __cdecl ShouldSpawnTracer(int localClientNum, int sourceEntityNum);
void __cdecl CG_BulletHitClientEvent(
    int localClientNum,
    int sourceEntityNum,
    float *startPos,
    float *position,
    unsigned int surfType,
    int event,
    int damage);
void __cdecl CG_MeleeBloodEvent(int localClientNum, const centity_s *cent);
void __cdecl CG_SetupWeaponDef(int localClientNum);
void __cdecl ParseWeaponDefFiles(const char **ppszFiles, int iNumFiles);
unsigned int __cdecl ValidLatestPrimaryWeapIdx(unsigned int weaponIndex);
void __cdecl CG_SelectWeaponIndex(int localClientNum, unsigned int weaponIndex);
char __cdecl CG_ScopeIsOverlayed(int localClientNum);
int __cdecl CG_PlayerTurretWeaponIdx(int localClientNum);
bool __cdecl CG_PlayerUsingScopedTurret(int localClientNum);
char __cdecl Bullet_Trace(
    const BulletFireParams *bp,
    const WeaponDef *weapDef,
    gentity_s *attacker,
    BulletTraceResults *br,
    unsigned int lastSurfaceType);



// cg_localents
enum leType_t : __int32
{                                       // ...
    LE_MOVING_TRACER = 0x0,
};
struct localEntity_s // sizeof=0x50
{                                       // ...
    localEntity_s *prev;
    localEntity_s *next;                // ...
    leType_t leType;
    int endTime;
    trajectory_t pos;
    float color[4];
    float tracerClipDist;
    GfxEntity refEntity;
};
void __cdecl TRACK_cg_localents();
void __cdecl CG_InitLocalEntities(int localClientNum);
localEntity_s *__cdecl CG_AllocLocalEntity(int localClientNum);
void __cdecl CG_FreeLocalEntity(int localClientNum, localEntity_s *le);
void __cdecl CG_AddLocalEntityTracerBeams(int localClientNum);
void __cdecl CG_AddMovingTracer(const cg_s *cgameGlob, localEntity_s *le, const refdef_s *refdef);



// offhandweapons
void __cdecl CG_OffhandRegisterDvars();
void __cdecl CG_DrawOffHandIcon(
    int localClientNum,
    const rectDef_s *rect,
    float scale,
    const float *color,
    Material *material,
    OffhandClass weaponType);
int __cdecl GetBestOffhand(const playerState_s *predictedPlayerState, int offhandClass);
bool __cdecl IsOffHandDisplayVisible(const cg_s *cgameGlob);
void __cdecl CG_DrawOffHandHighlight(
    int localClientNum,
    const rectDef_s *rect,
    float scale,
    const float *color,
    Material *material,
    OffhandClass weaponType);
void __cdecl OffHandFlash(const cg_s *cgameGlob, const float *base_color, float *out_color);
int __cdecl CalcOffHandAmmo(const playerState_s *predictedPlayerState, int weaponType);
void __cdecl CG_DrawOffHandAmmo(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    const float *color,
    int textStyle,
    OffhandClass weaponType);
void __cdecl CG_DrawOffHandName(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    const float *color,
    int textStyle,
    OffhandClass weaponType);
void __cdecl CG_SwitchOffHandCmd(int localClientNum);
void __cdecl CG_PrepOffHand(int localClientNum, const entityState_s *ent, unsigned int weaponIndex);
void __cdecl CG_UseOffHand(int localClientNum, const centity_s *cent, unsigned int weaponIndex);
void __cdecl CG_SetEquippedOffHand(int localClientNum, unsigned int offHandIndex);



// cg_world
bool __cdecl CG_IsEntityLinked(int localClientNum, unsigned int entIndex);
bool __cdecl CG_EntityNeedsLinked(int localClientNum, unsigned int entIndex);
DObj_s *__cdecl CG_LocationalTraceDObj(int localClientNum, unsigned int entIndex);
void __cdecl CG_UnlinkEntity(int localClientNum, unsigned int entIndex);
void __cdecl CG_LinkEntity(int localClientNum, unsigned int entIndex);
void __cdecl CG_GetEntityBModelBounds(const centity_s *cent, float *mins, float *maxs, float *absMins, float *absMaxs);
void __cdecl CG_GetEntityDobjBounds(const centity_s *cent, const DObj_s *dobj, float *absMins, float *absMaxs);
void __cdecl CG_LocationalTrace(trace_t *results, float *start, float *end, int passEntityNum, int contentMask);
void __cdecl CG_Trace(
    trace_t *results,
    float *start,
    float *mins,
    float *maxs,
    float *end,
    int passEntityNum,
    int contentMask,
    bool locational,
    bool staticModels);
void __cdecl CG_ClipMoveToEntities(const moveclip_t *clip, trace_t *results);
void __cdecl CG_ClipMoveToEntities_r(
    const moveclip_t *clip,
    unsigned __int16 sectorIndex,
    const float *p1,
    const float *p2,
    trace_t *results);
void __cdecl CG_ClipMoveToEntity(const moveclip_t *clip, unsigned int entIndex, trace_t *results);
int __cdecl CG_GetEntityBModelContents(const centity_s *cent);
void __cdecl CG_PointTraceToEntities(const pointtrace_t *clip, trace_t *results);
void __cdecl CG_PointTraceToEntities_r(
    const pointtrace_t *clip,
    unsigned __int16 sectorIndex,
    const float *p1,
    const float *p2,
    trace_t *results);
void __cdecl CG_PointTraceToEntity(const pointtrace_t *clip, unsigned int entIndex, trace_t *results);
void __cdecl CG_LocationTraceDobjCalcPose(const DObj_s *dobj, const cpose_t *pose, int *partBits);
void __cdecl CG_LocationalTraceEntitiesOnly(
    trace_t *results,
    float *start,
    float *end,
    int passEntityNum,
    int contentMask);
void __cdecl CG_TraceCapsule(
    trace_t *results,
    const float *start,
    const float *mins,
    const float *maxs,
    const float *end,
    int passEntityNum,
    int contentMask);



// cg_visionsets
enum visionSetMode_t : __int32
{                                       // ...
    VISIONSETMODE_NAKED = 0x0,
    VISIONSETMODE_NIGHT = 0x1,
    VISIONSETMODECOUNT = 0x2,
};
enum visionSetLerpStyle_t : __int32
{                                       // ...
    VISIONSETLERP_UNDEFINED = 0x0,
    VISIONSETLERP_NONE = 0x1,
    VISIONSETLERP_TO_LINEAR = 0x2,
    VISIONSETLERP_TO_SMOOTH = 0x3,
    VISIONSETLERP_BACKFORTH_LINEAR = 0x4,
    VISIONSETLERP_BACKFORTH_SMOOTH = 0x5,
};
struct visionSetLerpData_t // sizeof=0xC
{                                       // ...
    int timeStart;
    int timeDuration;
    visionSetLerpStyle_t style;
};
struct visionSetVars_t // sizeof=0x50
{                                       // ...
    bool glowEnable;                    // ...
    // padding byte
    // padding byte
    // padding byte
    float glowBloomCutoff;              // ...
    float glowBloomDesaturation;        // ...
    float glowBloomIntensity0;          // ...
    float glowBloomIntensity1;          // ...
    float glowRadius0;                  // ...
    float glowRadius1;                  // ...
    float glowSkyBleedIntensity0;       // ...
    float glowSkyBleedIntensity1;       // ...
    bool filmEnable;                    // ...
    // padding byte
    // padding byte
    // padding byte
    float filmBrightness;               // ...
    float filmContrast;                 // ...
    float filmDesaturation;             // ...
    bool filmInvert;                    // ...
    // padding byte
    // padding byte
    // padding byte
    float filmLightTint[3];             // ...
    float filmDarkTint[3];              // ...
};
void __cdecl CG_RegisterVisionSetsDvars();
void __cdecl CG_InitVisionSetsMenu();
void __cdecl CG_AddVisionSetMenuItem(XAssetHeader header);
void __cdecl CG_VisionSetsUpdate(int localClientNum);
void __cdecl UpdateVarsLerp(
    int time,
    const visionSetVars_t *from,
    const visionSetVars_t *to,
    visionSetLerpData_t *lerpData,
    visionSetVars_t *result);
bool __cdecl LerpBool(bool from, bool to, float fraction, visionSetLerpStyle_t style);
double __cdecl LerpFloat(float from, float to, float fraction, visionSetLerpStyle_t style);
void __cdecl LerpVec3(float *from, float *to, float fraction, visionSetLerpStyle_t style, float *result);
char __cdecl CG_VisionSetStartLerp_To(
    int localClientNum,
    visionSetMode_t mode,
    visionSetLerpStyle_t style,
    char *nameTo,
    int duration);
char __cdecl GetVisionSet(int localClientNum, char *name, visionSetVars_t *resultSettings);
char __cdecl LoadVisionFile(const char *name, visionSetVars_t *resultSettings);
char *__cdecl RawBufferOpen(const char *name, const char *formatFullPath);
char __cdecl LoadVisionSettingsFromBuffer(const char *buffer, const char *filename, visionSetVars_t *settings);
char __cdecl ApplyTokenToField(unsigned int fieldNum, const char *token, visionSetVars_t *settings);
char __cdecl VisionSetCurrent(int localClientNum, visionSetMode_t mode, char *name);
void __cdecl SetDefaultVision(int localClientNum);
void __cdecl CG_VisionSetConfigString_Naked(int localClientNum);
void __cdecl CG_VisionSetConfigString_Night(int localClientNum);
void __cdecl CG_VisionSetMyChanges();
void __cdecl CG_VisionSetUpdateTweaksFromFile_Glow();
bool __cdecl LoadVisionFileForTweaks(visionSetVars_t *setVars);
void __cdecl CG_VisionSetUpdateTweaksFromFile_Film();
char __cdecl CG_LookingThroughNightVision(int localClientNum);
void __cdecl CG_VisionSetApplyToRefdef(int localClientNum);
double __cdecl VisionFadeValue(int localClientNum);
void __cdecl FadeRefDef(refdef_s *rd, float brightness);



// cg_shellshock
void __cdecl CG_PerturbCamera(cg_s *cgameGlob);
int __cdecl CG_DrawShellShockSavedScreenBlendBlurred(
    int localClientNum,
    const shellshock_parms_t *parms,
    int start,
    int duration);
void __cdecl SaveScreenToBuffer(int localClientNum);
int __cdecl CG_DrawShellShockSavedScreenBlendFlashed(
    int localClientNum,
    const shellshock_parms_t *parms,
    int start,
    int duration);
double __cdecl BlendSmooth(float percent);
void __cdecl CG_UpdateShellShock(int localClientNum, const shellshock_parms_t *parms, int start, int duration);
void __cdecl EndShellShock(int localClientNum);
void __cdecl EndShellShockSound(int localClientNum);
void __cdecl EndShellShockLookControl(int localClientNum);
void __cdecl EndShellShockCamera(int localClientNum);
void __cdecl EndShellShockScreen(int localClientNum);
void __cdecl UpdateShellShockSound(int localClientNum, const shellshock_parms_t *parms, int time, int duration);
void __cdecl UpdateShellShockLookControl(int localClientNum, const shellshock_parms_t *parms, int time, int duration);
void __cdecl UpdateShellShockCamera(int localClientNum, const shellshock_parms_t *parms, int time, int duration);
double __cdecl CubicInterpolate(float t, float x0, float x1, float x2, float x3);
void __cdecl CG_StartShellShock(cg_s *cgameGlob, const shellshock_parms_t *parms, int start, int duration);
bool __cdecl CG_Flashbanged(int localClientNum);



// cg_pose_utils
void __cdecl CG_UsedDObjCalcPose(cpose_t *pose);
void __cdecl CG_CullIn(cpose_t *pose);



// cg_playerstate
struct transPlayerState_t // sizeof=0x18
{                                       // ...
    int damageEvent;
    int eventSequence;
    int events[4];
};
void __cdecl CG_Respawn(int localClientNum);
int __cdecl CG_TransitionPlayerState(int localClientNum, playerState_s *ps, const transPlayerState_t *ops);
void __cdecl CG_DamageFeedback(int localClientNum, int yawByte, int pitchByte, int damage);
int __cdecl CG_CheckPlayerstateEvents(int localClientNum, playerState_s *ps, const transPlayerState_t *ops);



// cg_laser
enum LaserOwnerEnum : __int32
{                                       // ...
    LASER_OWNER_NON_PLAYER = 0x0,
    LASER_OWNER_PLAYER = 0x1,
};
void __cdecl CG_Laser_Add(
    centity_s *cent,
    DObj_s *obj,
    cpose_t *pose,
    const float *viewerPos,
    LaserOwnerEnum laserOwner);
void __cdecl CG_Laser_Add_Core(
    centity_s *cent,
    DObj_s *obj,
    orientation_t *orient,
    const float *viewerPos,
    LaserOwnerEnum laserOwner);



// cg_event
enum EquipmentSound_t : __int32
{                                       // ...
    EQS_WALKING = 0x0,
    EQS_RUNNING = 0x1,
    EQS_SPRINTING = 0x2,
    EQS_QWALKING = 0x3,
    EQS_QRUNNING = 0x4,
    EQS_QSPRINTING = 0x5,
};

enum InvalidCmdHintType : __int32
{                                       // ...
    INVALID_CMD_NONE = 0x0,
    INVALID_CMD_NO_AMMO_BULLETS = 0x1,
    INVALID_CMD_NO_AMMO_FRAG_GRENADE = 0x2,
    INVALID_CMD_NO_AMMO_SPECIAL_GRENADE = 0x3,
    INVALID_CMD_NO_AMMO_FLASH_GRENADE = 0x4,
    INVALID_CMD_STAND_BLOCKED = 0x5,
    INVALID_CMD_CROUCH_BLOCKED = 0x6,
    INVALID_CMD_TARGET_TOO_CLOSE = 0x7,
    INVALID_CMD_LOCKON_REQUIRED = 0x8,
    INVALID_CMD_NOT_ENOUGH_CLEARANCE = 0x9,
};

int __cdecl CG_GetBoneIndex(
    int localClientNum,
    unsigned int dobjHandle,
    unsigned int boneName,
    unsigned __int8 *boneIndex);
void __cdecl CG_PlayBoltedEffect(
    int localClientNum,
    const FxEffectDef *fxDef,
    unsigned int dobjHandle,
    unsigned int boneName);
void __cdecl CG_EntityEvent(int localClientNum, centity_s *cent, int event);
void __cdecl CG_Obituary(int localClientNum, const entityState_s *ent);
void __cdecl CG_ItemPickup(int localClientNum, int weapIndex);
void __cdecl CG_EquipmentSound(int localClientNum, int entNum, bool isPlayerView, EquipmentSound_t type);
void __cdecl CG_PlayFx(int localClientNum, centity_s *cent, const float *angles);
void __cdecl CG_PlayFxOnTag(int localClientNum, centity_s *cent, int eventParm);
void __cdecl CG_SetInvalidCmdHint(cg_s *cgameGlob, InvalidCmdHintType hintType);
void __cdecl CG_StopWeaponSound(
    int localClientNum,
    bool isPlayerView,
    const WeaponDef *weaponDef,
    int entitynum,
    weaponstate_t weaponstate);
void __cdecl CG_CheckEvents(int localClientNum, centity_s *cent);



// cg_draw_reticles
void __cdecl CG_CalcCrosshairPosition(const cg_s *cgameGlob, float *x, float *y);
char __cdecl CG_GetWeapReticleZoom(const cg_s *cgameGlob, float *zoom);
void __cdecl CG_DrawNightVisionOverlay(int localClientNum);
void __cdecl CG_DrawCrosshair(int localClientNum);
void __cdecl CG_DrawAdsOverlay(
    int localClientNum,
    const WeaponDef *weapDef,
    const float *color,
    const float *crosshairPos);
void __cdecl CG_DrawFrameOverlay(
    float innerLeft,
    float innerRight,
    float innerTop,
    float innerBottom,
    const float *color,
    Material *material);
bool __cdecl CG_UsingLowResViewPort(int localClientNum);
void __cdecl CG_UpdateScissorViewport(refdef_s *refdef, float *drawPos, float *drawSize);
double __cdecl CG_DrawWeapReticle(int localClientNum);
void __cdecl CG_CalcCrosshairColor(int localClientNum, float alpha, float *color);
void __cdecl CG_DrawTurretCrossHair(int localClientNum);
char __cdecl AllowedToDrawCrosshair(int localClientNum, const playerState_s *predictedPlayerState);
bool __cdecl CG_IsReticleTurnedOff();
void __cdecl CG_DrawAdsAimIndicator(
    int localClientNum,
    const WeaponDef *weapDef,
    const float *color,
    float centerX,
    float centerY,
    float transScale);
void __cdecl CG_TransitionToAds(
    const cg_s *cgameGlob,
    const WeaponDef *weapDef,
    float posLerp,
    float *transScale,
    float *transShift);
void __cdecl CG_DrawReticleCenter(
    int localClientNum,
    const WeaponDef *weapDef,
    const float *color,
    float centerX,
    float centerY,
    float transScale);
void __cdecl CG_DrawReticleSides(
    int localClientNum,
    const WeaponDef *weapDef,
    const float *baseColor,
    float centerX,
    float centerY,
    float transScale);
void __cdecl CG_CalcReticleSpread(
    const cg_s *cgameGlob,
    const WeaponDef *weapDef,
    const float *drawSize,
    float transScale,
    float *spread);
void __cdecl CG_CalcReticleColor(const float *baseColor, float alpha, float aimSpreadScale, float *reticleColor);
void __cdecl CG_CalcReticleImageOffset(const float *drawSize, float *imageTexelOffset);


// cg_draw_indicators
struct HudGrenade // sizeof=0x10
{                                       // ...
    float origin[3];
    Material *material;                 // ...
};
void __cdecl CG_DrawFlashDamage(const cg_s *cgameGlob);
void __cdecl CG_DrawDamageDirectionIndicators(int localClientNum);
void __cdecl CG_ClearHudGrenades();
char __cdecl CG_AddHudGrenade_PositionCheck(const cg_s *cgameGlob, const centity_s *grenadeEnt, WeaponDef *weapDef);
void __cdecl CG_AddHudGrenade(const cg_s *cgameGlob, const centity_s *grenadeEnt);
void __cdecl CG_DrawGrenadeIndicators(int localClientNum);
void __cdecl CG_DrawGrenadePointer(
    int localClientNum,
    float centerX,
    float centerY,
    const float *grenadeOffset,
    const float *color);
void __cdecl CG_DrawGrenadeIcon(
    int localClientNum,
    float centerX,
    float centerY,
    const float *grenadeOffset,
    const float *color,
    Material *material);



// cg_draw_debug
struct meminfo_t;
void __cdecl CG_CalculateFPS();
double __cdecl CG_DrawFPS(const ScreenPlacement *scrPlace, float y, meminfo_t *meminfo);
bool __cdecl CG_Flash(int timeMs);
double __cdecl CG_CornerDebugPrint(
    const ScreenPlacement *sP,
    float posX,
    float posY,
    float labelWidth,
    char *text,
    char *label,
    const float *color);
double __cdecl CG_CornerDebugPrintCaption(
    const ScreenPlacement *sP,
    float posX,
    float posY,
    float labelWidth,
    char *text,
    const float *color);
void __cdecl CG_DrawUpperRightDebugInfo(int localClientNum);
double __cdecl CG_DrawSnapshot(int localClientNum, float posY);
double __cdecl CG_DrawStatmon(const ScreenPlacement *scrPlace, float y, meminfo_t *meminfo);
void __cdecl CG_DrawPerformanceWarnings();
void __cdecl CG_DrawDebugOverlays(int localClientNum);
void __cdecl CG_DrawMaterial(int localClientNum, unsigned int drawMaterialType);
void __cdecl CG_DrawDebugPlayerHealth(int localClientNum);
void __cdecl CG_DrawFullScreenDebugOverlays(int localClientNum);
void __cdecl CG_DrawScriptUsage(const ScreenPlacement *scrPlace);
void CG_DrawVersion();
void __cdecl CG_DrawSoundEqOverlay(int localClientNum);
void __cdecl CG_DrawSoundOverlay(const ScreenPlacement *scrPlace);
void __cdecl CG_DrawFxProfile(int localClientNum);
void __cdecl CG_DrawFxText(char *text, float *profilePos);
void __cdecl CG_DrawFxMarkProfile(int localClientNum);
void __cdecl Vec4Set(float *v, float x, float y, float z, float w);



// cg_compass
enum CompassType : __int32
{                                       // ...
    COMPASS_TYPE_PARTIAL = 0x0,
    COMPASS_TYPE_FULL = 0x1,
};

void __cdecl CG_CompassRegisterDvars();
bool __cdecl CG_IsSelectingLocation(int localClientNum);
bool __cdecl CG_WorldPosToCompass(
    CompassType compassType,
    const cg_s *cgameGlob,
    const rectDef_s *mapRect,
    const float *north,
    const float *playerWorldPos,
    const float *in,
    float *out,
    float *outClipped);
void __cdecl CG_CompassCalcDimensions(
    CompassType compassType,
    const cg_s *cgameGlob,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    float *x,
    float *y,
    float *w,
    float *h);
double __cdecl CG_FadeCompass(int localClientNum, int displayStartTime, CompassType compassType);
void __cdecl CG_CompassDrawPlayerBack(
    int localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Material *material,
    float *color);
void __cdecl CG_CompassDrawPlayerNorthCoord(
    int localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Font_s *font,
    Material *material,
    float *const color,
    int style);
void __cdecl CG_CompassDrawPlayerEastCoord(
    int localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Font_s *font,
    Material *material,
    float *const color,
    int style);
void __cdecl CG_CompassDrawPlayerNCoordScroll(
    int localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Font_s *font,
    Material *material,
    float *color,
    int textStyle);
void __cdecl CG_CompassDrawPlayerECoordScroll(
    int localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Font_s *font,
    Material *material,
    float *color,
    int textStyle);
void __cdecl CG_CompassDrawPlayerMap(
    int localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Material *material,
    float *color);
void __cdecl CG_CompassDrawPlayerMapLocationSelector(
    int localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Material *material,
    float *color);
void __cdecl CG_CompassDrawPlayer(
    int localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    rectDef_s *rect,
    Material *material,
    float *color);
void __cdecl CG_CompassDrawBorder(
    int localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    rectDef_s *rect,
    Material *material,
    float *color);
void __cdecl CG_CompassUpYawVector(const cg_s *cgameGlob, float *result);
void __cdecl CG_CompassDrawTickertape(
    int localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Material *material,
    const float *color,
    Font_s *textFont,
    float textScale,
    int textStyle,
    bool drawObjectives);
void __cdecl CalcCompassPointerSize(CompassType compassType, float *w, float *h);
void __cdecl DrawIconDistanceText(
    int localClientNum,
    float distance,
    float iconX,
    float iconY,
    float iconH,
    const rectDef_s *rect,
    const float *color,
    Font_s *textFont,
    float textScale,
    int textStyle);
double __cdecl CutFloat(float original);
double __cdecl CG_GetHudAlphaCompass(int localClientNum);
void __cdecl CalcCompassFriendlySize(CompassType compassType, float *w, float *h);
void __cdecl CG_CompassDrawPlayerPointers_MP(
    int localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Material *material,
    const float *color);
double __cdecl GetObjectiveFade(const rectDef_s *clipRect, float x, float y, float width, float height);



// cg_colltree
struct CgEntCollNode // sizeof=0x14
{                                       // ...
    unsigned __int16 sector;
    unsigned __int16 nextEntInSector;
    float linkMins[2];
    float linkMaxs[2];
};
union CgEntCollTree_u // sizeof=0x2
{                                       // ...
    unsigned __int16 parent;
    unsigned __int16 nextFree;
};
struct CgEntCollTree // sizeof=0xC
{                                       // ...
    float dist;
    unsigned __int16 axis;
    CgEntCollTree_u u;
    unsigned __int16 child[2];
};
struct CgEntCollSector // sizeof=0x10
{                                       // ...
    CgEntCollTree tree;
    unsigned __int16 entListHead;
    // padding byte
    // padding byte
};
struct CgEntCollWorld // sizeof=0x401C
{                                       // ...
    float mins[3];
    float maxs[3];
    unsigned __int16 freeHead;
    // padding byte
    // padding byte
    CgEntCollSector sectors[1024];
};
void __cdecl TRACK_CG_CollWorld();
void __cdecl CG_SetCollWorldLocalClientNum(int localClientNum);
int __cdecl CG_GetCollWorldLocalClientNum();
void __cdecl CG_ClearEntityCollWorld(int localClientNum);
const CgEntCollSector *__cdecl CG_GetEntityCollSector(int localClientNum, unsigned __int16 sectorIndex);
const CgEntCollNode *__cdecl CG_GetEntityCollNode(int localClientNum, unsigned int entIndex);
CgEntCollNode *__cdecl CG_GetCollNode(int localClientNum, unsigned int entIndex);
void __cdecl CG_UnlinkEntityColl(int localClientNum, unsigned int entIndex);
void __cdecl CG_LinkEntityColl(int localClientNum, unsigned int entIndex, const float *absMins, const float *absMaxs);
void __cdecl CG_AddEntityToCollSector(int localClientNum, unsigned int entIndex, unsigned __int16 sectorIndex);
void __cdecl CG_SortEntityCollSector(
    int localClientNum,
    unsigned __int16 sectorIndex,
    const float *mins,
    const float *maxs);
unsigned __int16 __cdecl CG_AllocEntityCollSector(int localClientNum, const float *mins, const float *maxs);



// cg_camerashake
struct CameraShake // sizeof=0x24
{                                       // ...
    int time;                           // ...
    float scale;                        // ...
    float length;                       // ...
    float radius;                       // ...
    float src[3];                       // ...
    float size;                         // ...
    float rumbleScale;                  // ...
};
struct CameraShakeSet // sizeof=0x94
{                                       // ...
    CameraShake shakes[4];
    float phase;
};

void __cdecl TRACK_cg_camerashake();
void __cdecl CG_StartShakeCamera(int localClientNum, float p, int duration, float *src, float radius);
int __cdecl CG_UpdateCameraShake(const cg_s *cgameGlob, CameraShake *shake);
void __cdecl CG_ShakeCamera(int localClientNum);
void __cdecl CG_ClearCameraShakes(int localClientNum);



// cg_ammocounter
void __cdecl CG_AmmoCounterRegisterDvars();
void __cdecl CG_DrawPlayerWeaponAmmoStock(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    float *color,
    Material *material,
    int textStyle);
unsigned int __cdecl ClipCounterWeapIdx(const cg_s *cgameGlob, unsigned int weapIndex);
unsigned int __cdecl GetWeaponAltIndex(const cg_s *cgameGlob, const WeaponDef *weapDef);
double __cdecl AmmoCounterFadeAlpha(int localClientNum, cg_s *cgameGlob);
double __cdecl CG_GetHudAlphaDPad(int localClientNum);
double __cdecl DpadFadeAlpha(int localClientNum, cg_s *cgameGlob);
bool __cdecl ActionSlotIsActive(int localClientNum, unsigned int slotIdx);
double __cdecl CG_GetHudAlphaAmmoCounter(int localClientNum);
bool __cdecl CG_ActionSlotIsUsable(int localClientNum, unsigned int slotIdx);
void __cdecl CG_DrawPlayerActionSlotDpad(
    int localClientNum,
    const rectDef_s *rect,
    const float *color,
    Material *material);
void __cdecl CG_DrawPlayerActionSlot(
    int localClientNum,
    const rectDef_s *rect,
    unsigned int slotIdx,
    float *color,
    Font_s *textFont,
    float textScale,
    int textStyle);
void __cdecl DpadIconDims(
    const rectDef_s *rect,
    unsigned int slotIdx,
    WeaponDef *weapDef,
    float *x,
    float *y,
    float *w,
    float *h);
void __cdecl DpadTextPos(const rectDef_s *rect, unsigned int slotIdx, WeaponDef *weapDef, float *x, float *y);
void __cdecl CG_DrawPlayerWeaponBackground(
    int localClientNum,
    const rectDef_s *rect,
    const float *color,
    Material *material);
void __cdecl CG_DrawPlayerWeaponAmmoClipGraphic(int localClientNum, const rectDef_s *rect, const float *color);
void __cdecl GetBaseRectPos(int localClientNum, const rectDef_s *rect, float *base);
void __cdecl DrawClipAmmo(cg_s *cgameGlob, float *base, unsigned int weapIdx, const WeaponDef *weapDef, float *color);
void __cdecl DrawClipAmmoMagazine(
    cg_s *cgameGlob,
    const float *base,
    unsigned int weapIdx,
    const WeaponDef *weapDef,
    float *color);
void __cdecl AmmoColor(cg_s *cgameGlob, float *color, unsigned int weapIndex);
void __cdecl DrawClipAmmoShortMagazine(
    cg_s *cgameGlob,
    const float *base,
    unsigned int weapIdx,
    const WeaponDef *weapDef,
    float *color);
void __cdecl DrawClipAmmoShotgunShells(
    cg_s *cgameGlob,
    const float *base,
    unsigned int weapIdx,
    const WeaponDef *weapDef,
    float *color);
void __cdecl DrawClipAmmoRockets(
    cg_s *cgameGlob,
    const float *base,
    unsigned int weapIdx,
    const WeaponDef *weapDef,
    float *color);
void __cdecl DrawClipAmmoBeltfed(
    cg_s *cgameGlob,
    float *base,
    unsigned int weapIdx,
    const WeaponDef *weapDef,
    float *color);
void __cdecl CG_DrawPlayerWeaponIcon(int localClientNum, const rectDef_s *rect, const float *color);
void __cdecl DrawStretchPicGun(
    const ScreenPlacement *scrPlace,
    const rectDef_s *rect,
    const float *color,
    Material *material,
    weaponIconRatioType_t ratio);
void __cdecl CG_DrawPlayerWeaponLowAmmoWarning(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float textScale,
    int textStyle,
    float text_x,
    float text_y,
    char textAlignMode,
    Material *material);
unsigned int __cdecl GetWeaponIndex(const cg_s *cgameGlob);
void __cdecl Vec4Copy(const float *from, float *to);


// cg_effects_load_obj
struct EffectFile // sizeof=0x630
{                                       // ...
    const char *nonflesh[12][29];       // ...
    const char *flesh[12][4];           // ...
};
FxImpactTable *__cdecl CG_RegisterImpactEffects(const char *mapname);
FxImpactTable *__cdecl CG_RegisterImpactEffects_LoadObj(const char *mapname);



// cg_info
void __cdecl CG_LoadingString(int localClientNum, const char *s);
void __cdecl CG_DrawInformation(int localClientNum);
bool __cdecl CG_IsShowingProgress_FastFile();



extern const dvar_t *nightVisionFadeInOutTime;
extern const dvar_t *nightVisionPowerOnTime;
extern const dvar_t *nightVisionDisableEffects;