#pragma once

#include <bgame/bg_local.h>

#include <gfx_d3d/fxprimitives.h>
#include <gfx_d3d/r_gfx.h>
#include <gfx_d3d/r_material.h>

#include <cstdint>

struct cg_s;

struct Font_s;
struct Material;
struct ScreenPlacement;

#define MAX_EFFECT_NAMES 100

#define ACTIONSLOTS_NUM 3

#define ENTITYNUM_WORLD (MAX_GENTITIES - 2)
#define ENTITYNUM_NONE (MAX_GENTITIES - 1)
#define SURF_TYPECOUNT 29

#define WEAPON_HINT_OFFSET 4

#define FIRST_WEAPON_HINT 5
#define LAST_WEAPON_HINT 132

#define MYMODELCOUNT 4

#define MAX_WEAPONS 128

#define ITEM_WEAPMODEL(x) (MAX_WEAPONS * (x / MAX_WEAPONS))

#define PRIMARY_LIGHT_NONE 0

static const float up[3] = { 0.0f, 0.0f, 1.0f };

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
    int32_t horzAlign,
    int32_t vertAlign,
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
    int32_t horzAlign,
    int32_t vertAlign,
    const float *color,
    Material *material);
void __cdecl CG_DrawStringExt(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    char *string,
    const float *setColor,
    int32_t forceColor,
    int32_t shadow,
    float charHeight);
int32_t __cdecl CG_DrawDevString(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    float xScale,
    float yScale,
    char *s,
    const float *color,
    char align,
    Font_s *font);
int32_t __cdecl CG_DrawBigDevString(const ScreenPlacement *scrPlace, float x, float y, char *s, float alpha, char align);
int32_t __cdecl CG_DrawBigDevStringColor(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    char *s,
    const float *color,
    char align);
int32_t __cdecl CG_DrawSmallDevStringColor(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    char *s,
    const float *color,
    char align);
double __cdecl CG_FadeAlpha(int32_t timeNow, int32_t startMsec, int32_t totalMsec, int32_t fadeMsec);
float *__cdecl CG_FadeColor(int32_t timeNow, int32_t startMsec, int32_t totalMsec, int32_t fadeMsec);
void __cdecl CG_MiniMapChanged(int32_t localClientNum);
void __cdecl CG_NorthDirectionChanged(int32_t localClientNum);
void __cdecl CG_DebugLine(const float *start, const float *end, const float *color, int32_t depthTest, int32_t duration);
void __cdecl CG_DebugStar(const float *point, const float *color, int32_t duration);
void __cdecl CG_DebugStarWithText(
    const float *point,
    const float *starColor,
    const float *textColor,
    char *string,
    float fontsize,
    int32_t duration);
void __cdecl CG_DebugBox(
    const float *origin,
    const float *mins,
    const float *maxs,
    float yaw,
    const float *color,
    int32_t depthTest,
    int32_t duration);
void __cdecl CG_DebugBoxOriented(
    const float *origin,
    const float *mins,
    const float *maxs,
    const mat3x3 &rotation,
    const float *color,
    int32_t depthTest,
    int32_t duration);
void __cdecl CG_DebugCircle(
    const float *center,
    float radius,
    const float *dir,
    const float *color,
    int32_t depthTest,
    int32_t duration);
void __cdecl CG_TeamColor(int32_t team, const char *prefix, float *color);
void __cdecl CG_RelativeTeamColor(int32_t clientNum, const char *prefix, float *color, int32_t localClientNum);



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
    int32_t timeNow;                        // ...
};
void __cdecl CG_HudElemRegisterDvars();
void __cdecl CG_TranslateHudElemMessage(
    int32_t localClientNum,
    const char *message,
    const char *messageType,
    char *hudElemString);
char __cdecl ReplaceDirective(int32_t localClientNum, uint32_t *searchPos, uint32_t *dstLen, char *dstString);
void __cdecl GetHudelemDirective(int32_t localClientNum, char *directive, char *result);
void __cdecl DirectiveFakeIntroSeconds(int32_t localClientNum, const char *arg0, char *result);
void __cdecl ParseDirective(char *directive, char *resultName, char *resultArg0);
void __cdecl CG_Draw2dHudElems(int32_t localClientNum, int32_t foreground);
void __cdecl DrawSingleHudElem2d(int32_t localClientNum, const hudelem_s *elem);
void __cdecl GetHudElemInfo(int32_t localClientNum, const hudelem_s *elem, cg_hudelem_t *cghe, char *hudElemString);
void __cdecl SafeTranslateHudElemString(int32_t localClientNum, int32_t index, char *hudElemString);
double __cdecl HudElemStringWidth(const char *string, const cg_hudelem_t *cghe);
char *__cdecl HudElemTimerString(const hudelem_s *elem, int32_t timeNow);
int32_t __cdecl GetHudElemTime(const hudelem_s *elem, int32_t timeNow);
char *__cdecl HudElemTenthsTimerString(const hudelem_s *elem, int32_t timeNow);
double __cdecl HudElemWidth(const ScreenPlacement *scrPlace, const hudelem_s *elem, const cg_hudelem_t *cghe);
double __cdecl HudElemMaterialWidth(const ScreenPlacement *scrPlace, const hudelem_s *elem, const cg_hudelem_t *cghe);
double __cdecl HudElemMaterialSpecifiedWidth(
    const ScreenPlacement *scrPlace,
    char alignScreen,
    int32_t sizeVirtual,
    const cg_hudelem_t *cghe);
double __cdecl HudElemHeight(const ScreenPlacement *scrPlace, const hudelem_s *elem, const cg_hudelem_t *cghe);
double __cdecl HudElemMaterialHeight(const ScreenPlacement *scrPlace, const hudelem_s *elem, const cg_hudelem_t *cghe);
double __cdecl HudElemMaterialSpecifiedHeight(
    const ScreenPlacement *scrPlace,
    char alignScreen,
    int32_t sizeVirtual,
    const cg_hudelem_t *cghe);
void __cdecl SetHudElemPos(const ScreenPlacement *scrPlace, const hudelem_s *elem, cg_hudelem_t *cghe);
void __cdecl GetHudElemOrg(
    const ScreenPlacement *scrPlace,
    int32_t alignOrg,
    int32_t alignScreen,
    float xVirtual,
    float yVirtual,
    float width,
    float height,
    float *orgX,
    float *orgY);
double __cdecl AlignHudElemX(int32_t alignOrg, float x, float width);
double __cdecl AlignHudElemY(int32_t alignOrg, float y, float height);
double __cdecl HudElemMovementFrac(const hudelem_s *elem, int32_t timeNow);
void __cdecl ConsolidateHudElemText(cg_hudelem_t *cghe, char *hudElemString);
void __cdecl CopyStringToHudElemString(char *string, char *hudElemString);
void __cdecl HudElemColorToVec4(const hudelem_color_t *hudElemColor, float *resultColor);
void __cdecl DrawHudElemString(
    uint32_t localClientNum,
    const ScreenPlacement *scrPlace,
    char *text,
    const hudelem_s *elem,
    cg_hudelem_t *cghe);
double __cdecl OffsetHudElemY(const hudelem_s *elem, const cg_hudelem_t *cghe, float offsetY);
void __cdecl DrawHudElemClock(int32_t localClientNum, const hudelem_s *elem, const cg_hudelem_t *cghe);
void __cdecl DrawHudElemMaterial(int32_t localClientNum, const hudelem_s *elem, cg_hudelem_t *cghe);
void __cdecl DrawOffscreenViewableWaypoint(int32_t localClientNum, const hudelem_s *elem);
char __cdecl WorldPosToScreenPos(int32_t localClientNum, const float *worldPos, float *outScreenPos);
bool __cdecl ClampScreenPosToEdges(
    int32_t localClientNum,
    float *point,
    float padLeft,
    float padRight,
    float padTop,
    float padBottom,
    float *resultNormal,
    float *resultDist);
float __cdecl GetScaleForDistance(int32_t localClientNum, const float *worldPos);
int32_t __cdecl GetSortedHudElems(int32_t localClientNum, hudelem_s **elems);
void __cdecl CopyInUseHudElems(hudelem_s **elems, int32_t *elemCount, hudelem_s *elemSrcArray, int32_t elemSrcArrayCount);
int32_t __cdecl compare_hudelems(const void *pe0, const void *pe1);
void __cdecl CG_AddDrawSurfsFor3dHudElems(int32_t localClientNum);
void  AddDrawSurfForHudElemWaypoint(int32_t localClientNum, const hudelem_s *elem);
float __cdecl HudElemWaypointHeight(int32_t localClientNum, const hudelem_s *elem);


// cg_weapons
struct refdef_s;
struct weaponInfo_s // sizeof=0x44
{                                       // ...
    DObj_s *viewModelDObj;
    XModel *handModel;
    XModel *gogglesModel;
    XModel *rocketModel;
    XModel *knifeModel;
    uint8_t weapModelIdx;
    // padding byte
    // padding byte
    // padding byte
    uint32_t partBits[4];
    int32_t iPrevAnim;
#ifdef KISAK_SP
    int32_t hasAnimTree;
#endif
    XAnimTree_s *tree;
    int32_t registered;
    const gitem_s *item;
    const char *translatedDisplayName;
    const char *translatedModename;
    const char *translatedAIOverlayDescription;
};
bool __cdecl CG_JavelinADS(int32_t localClientNum);
int32_t __cdecl CG_WeaponDObjHandle(int32_t weaponNum);
void __cdecl CG_RegisterWeapon(int32_t localClientNum, uint32_t weaponNum);
XAnimTree_s *__cdecl CG_CreateWeaponViewModelXAnim(WeaponDef *weapDef);
void __cdecl CG_UpdateWeaponViewmodels(int32_t localClientNum);
void __cdecl ChangeViewmodelDobj(
    int32_t localClientNum,
    uint32_t weaponNum,
    uint8_t weaponModel,
    XModel *newHands,
    XModel *newGoggles,
    XModel *newRocket,
    XModel *newKnife,
    bool updateClientInfo);
void __cdecl CG_UpdateHandViewmodels(int32_t localClientNum, XModel *handModel);
void __cdecl CG_RegisterItemVisuals(int32_t localClientNum, uint32_t weapIdx);
void __cdecl CG_RegisterItems(int32_t localClientNum);
void __cdecl CG_HoldBreathInit(cg_s *cgameGlob);
void __cdecl CG_UpdateViewModelPose(const DObj_s *obj, int32_t localClientNum);
bool __cdecl CG_IsPlayerCrouching(clientInfo_t *ci, const centity_s *cent);
bool __cdecl CG_IsPlayerProne(clientInfo_t *ci, const centity_s *cent);
bool __cdecl CG_IsPlayerADS(clientInfo_t *ci, const centity_s *cent);
void __cdecl CG_GuessSpreadForWeapon(
    int32_t localClientNum,
    const centity_s *cent,
    const WeaponDef *weapDef,
    float *minSpread,
    float *maxSpread);
void __cdecl CG_GetPlayerViewOrigin(int32_t localClientNum, const playerState_s *ps, float *origin);
void __cdecl CG_AddPlayerWeapon(
    int32_t localClientNum,
    const GfxScaledPlacement *placement,
    const playerState_s *ps,
    centity_s *cent,
    int32_t bDrawGun);
void __cdecl WeaponFlash(
    int32_t localClientNum,
    uint32_t dobjHandle,
    uint32_t weaponNum,
    int32_t bViewFlash,
    uint32_t flashTag);
void __cdecl HoldBreathUpdate(int32_t localClientNum);
void __cdecl HoldBreathSoundLerp(int32_t localClientNum, float lerp);
void __cdecl CG_UpdateViewWeaponAnim(int32_t localClientNum);
void __cdecl WeaponRunXModelAnims(int32_t localClientNum, const playerState_s *ps, weaponInfo_s *weapInfo);
void __cdecl StartWeaponAnim(
    int32_t localClientNum,
    uint32_t weaponNum,
    DObj_s *obj,
    int32_t animIndex,
    float transitionTime);
double __cdecl GetWeaponAnimRate(WeaponDef *weapDef, XAnim_s *anims, uint32_t animIndex);
void __cdecl PlayADSAnim(float weaponPosFrac, int32_t weaponNum, DObj_s *obj, int32_t animIndex);
void __cdecl ResetWeaponAnimTrees(int32_t localClientNum, const playerState_s *ps);
char __cdecl UpdateViewmodelAttachments(
    int32_t localClientNum,
    uint32_t weaponNum,
    uint8_t weaponModel,
    weaponInfo_s *weapInfo);
bool __cdecl ViewmodelRocketShouldBeAttached(int32_t localClientNum, WeaponDef *weapDef);
bool __cdecl ViewmodelKnifeShouldBeAttached(int32_t localClientNum, WeaponDef *weapDef);
void __cdecl ProcessWeaponNoteTracks(int32_t localClientNum, const playerState_s *predictedPlayerState);
void __cdecl PlayNoteMappedSoundAliases(int32_t localClientNum, const char *noteName, const WeaponDef *weapDef);
void __cdecl CG_AddViewWeapon(int32_t localClientNum);
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
void __cdecl CG_OutOfAmmoChange(int32_t localClientNum);
char __cdecl VerifyPlayerAltModeWeapon(int32_t localClientNum, const WeaponDef *weapDef);
char __cdecl CycleWeapPrimary(int32_t localClientNum, int32_t cycleForward, int32_t bIgnoreEmpty);
uint32_t __cdecl CG_AltWeaponToggleIndex(int32_t localClientNum, const cg_s *cgameGlob);
int32_t __cdecl NextWeapInCycle(
    int32_t localClientNum,
    const playerState_s *ps,
    weapInventoryType_t type,
    uint32_t startWeaponIndex,
    bool cycleForward,
    bool skipEmpties,
    bool skipHaveNoAlts);
void __cdecl CG_ActionSlotDown_f();
char __cdecl ToggleWeaponAltMode(int32_t localClientNum);
bool __cdecl ActionSlotUsageAllowed(cg_s *cgameGlob);
char __cdecl ActionParms(int32_t *slotResult);
void __cdecl CG_ActionSlotUp_f();
void __cdecl CG_EjectWeaponBrass(int32_t localClientNum, const entityState_s *ent, int32_t event);
void __cdecl CG_FireWeapon(
    int32_t localClientNum,
    centity_s *cent,
    int32_t event,
    uint16_t tagName,
    uint32_t weapon,
    const playerState_s *ps);
void __cdecl DrawBulletImpacts(
    int32_t localClientNum,
    const centity_s *ent,
    const WeaponDef *weaponDef,
    uint16_t boneName,
    const playerState_s *ps);
void __cdecl FireBulletPenetrate(
    int32_t localClientNum,
    BulletFireParams *bp,
    const WeaponDef *weapDef,
    const centity_s *attacker,
    float *tracerStart,
    bool drawTracer);
char __cdecl BulletTrace(
    int32_t localClientNum,
    const BulletFireParams *bp,
    const WeaponDef *weapDef,
    const centity_s *attacker,
    BulletTraceResults *br,
    uint32_t lastSurfaceType);
bool __cdecl ShouldIgnoreHitEntity(int32_t attackerNum, int32_t hitEntNum);
bool __cdecl IsEntityAPlayer(int32_t localClientNum, uint32_t entityNum);
void __cdecl CG_BulletEndpos(
    int32_t randSeed,
    float spread,
    const float *start,
    float *end,
    float *dir,
    const float *forwardDir,
    const float *rightDir,
    const float *upDir,
    float maxRange);
void __cdecl RandomBulletDir(int32_t randSeed, float *x, float *y);
void __cdecl TakeClipOnlyWeaponIfEmpty(int32_t localClientNum, playerState_s *ps);
void __cdecl CG_SpawnTracer(int32_t localClientNum, const float *pstart, const float *pend);
void __cdecl CG_DrawTracer(const float *start, const float *finish, const refdef_s *refdef);
void __cdecl ScaleTracer(
    const float *start,
    const float *finish,
    const float *viewOrg,
    float *startWidth,
    float *finishWidth);
double __cdecl CalcTracerFinalScale(float tracerScaleDistRange, float dist, float tracerScale);
cg_s *__cdecl CG_GetLocalClientGlobalsForEnt(int32_t localClientNum, int32_t entityNum);
void __cdecl CG_GetViewDirection(int32_t localClientNum, int32_t entityNum, float *forward, float *right, float *up);
void __cdecl CG_CalcEyePoint(int32_t localClientNum, int32_t entityNum, float *eyePos);
void __cdecl CG_RandomEffectAxis(const float *forward, float *left, float *up);
void __cdecl CG_ImpactEffectForWeapon(
    uint32_t weaponIndex,
    uint32_t surfType,
    char impactFlags,
    const FxEffectDef **outFx,
    snd_alias_list_t **outSnd);
void __cdecl CG_BulletHitEvent(
    int32_t localClientNum,
    int32_t sourceEntityNum,
    uint32_t targetEntityNum,
    uint32_t weaponIndex,
    float *startPos,
    float *position,
    const float *normal,
    uint32_t surfType,
    int32_t event,
    uint8_t eventParam,
    int32_t damage,
    __int16 hitContents);
int32_t __cdecl CalcMuzzlePoint(int32_t localClientNum, int32_t entityNum, float *muzzle, uint32_t flashTag);
void __cdecl CG_BulletHitEvent_Internal(
    int32_t localClientNum,
    int32_t sourceEntityNum,
    uint32_t targetEntityNum,
    uint32_t weaponIndex,
    float *startPos,
    float *position,
    const float *normal,
    uint32_t surfType,
    int32_t event,
    uint8_t eventParam,
    int32_t damage,
    __int16 hitContents);
void __cdecl BulletTrajectoryEffects(
    int32_t localClientNum,
    int32_t sourceEntityNum,
    float *startPos,
    float *position,
    int32_t surfType,
    uint32_t flashTag,
    uint8_t impactFlags,
    int32_t damage);
void __cdecl WhizbySound(int32_t localClientNum, const float *start, const float *end);
bool __cdecl ShouldSpawnTracer(int32_t localClientNum, int32_t sourceEntityNum);
void __cdecl CG_BulletHitClientEvent(
    int32_t localClientNum,
    int32_t sourceEntityNum,
    float *startPos,
    float *position,
    uint32_t surfType,
    int32_t event,
    int32_t damage);
void __cdecl CG_MeleeBloodEvent(int32_t localClientNum, const centity_s *cent);
void __cdecl CG_SetupWeaponDef(int32_t localClientNum);
void __cdecl ParseWeaponDefFiles(const char **ppszFiles, int32_t iNumFiles);
uint32_t __cdecl ValidLatestPrimaryWeapIdx(uint32_t weaponIndex);
void __cdecl CG_SelectWeaponIndex(int32_t localClientNum, uint32_t weaponIndex);
char __cdecl CG_ScopeIsOverlayed(int32_t localClientNum);
int32_t __cdecl CG_PlayerTurretWeaponIdx(int32_t localClientNum);
bool __cdecl CG_PlayerUsingScopedTurret(int32_t localClientNum);
void CG_DisplayViewmodelAnim(int localClientNum);
char __cdecl Bullet_Trace(
    const BulletFireParams *bp,
    const WeaponDef *weapDef,
    gentity_s *attacker,
    BulletTraceResults *br,
    uint32_t lastSurfaceType);



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
    int32_t endTime;
    trajectory_t pos;
    float color[4];
    float tracerClipDist;
    GfxEntity refEntity;
};
void __cdecl TRACK_cg_localents();
void __cdecl CG_InitLocalEntities(int32_t localClientNum);
localEntity_s *__cdecl CG_AllocLocalEntity(int32_t localClientNum);
void __cdecl CG_FreeLocalEntity(int32_t localClientNum, localEntity_s *le);
void __cdecl CG_AddLocalEntityTracerBeams(int32_t localClientNum);
void __cdecl CG_AddMovingTracer(const cg_s *cgameGlob, localEntity_s *le, const refdef_s *refdef);



// offhandweapons
void __cdecl CG_OffhandRegisterDvars();
void __cdecl CG_DrawOffHandIcon(
    int32_t localClientNum,
    const rectDef_s *rect,
    float scale,
    const float *color,
    Material *material,
    OffhandClass weaponType);
int32_t __cdecl GetBestOffhand(const playerState_s *predictedPlayerState, int32_t offhandClass);
bool __cdecl IsOffHandDisplayVisible(const cg_s *cgameGlob);
void __cdecl CG_DrawOffHandHighlight(
    int32_t localClientNum,
    const rectDef_s *rect,
    float scale,
    const float *color,
    Material *material,
    OffhandClass weaponType);
void __cdecl OffHandFlash(const cg_s *cgameGlob, const float *base_color, float *out_color);
int32_t __cdecl CalcOffHandAmmo(const playerState_s *predictedPlayerState, int32_t weaponType);
void __cdecl CG_DrawOffHandAmmo(
    int32_t localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    const float *color,
    int32_t textStyle,
    OffhandClass weaponType);
void __cdecl CG_DrawOffHandName(
    int32_t localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    const float *color,
    int32_t textStyle,
    OffhandClass weaponType);
void __cdecl CG_SwitchOffHandCmd(int32_t localClientNum);
void __cdecl CG_PrepOffHand(int32_t localClientNum, const entityState_s *ent, uint32_t weaponIndex);
void __cdecl CG_UseOffHand(int32_t localClientNum, const centity_s *cent, uint32_t weaponIndex);
void __cdecl CG_SetEquippedOffHand(int32_t localClientNum, uint32_t offHandIndex);



// cg_world
bool __cdecl CG_IsEntityLinked(int32_t localClientNum, uint32_t entIndex);
bool __cdecl CG_EntityNeedsLinked(int32_t localClientNum, uint32_t entIndex);
DObj_s *__cdecl CG_LocationalTraceDObj(int32_t localClientNum, uint32_t entIndex);
void __cdecl CG_UnlinkEntity(int32_t localClientNum, uint32_t entIndex);
void __cdecl CG_LinkEntity(int32_t localClientNum, uint32_t entIndex);
void __cdecl CG_GetEntityBModelBounds(const centity_s *cent, float *mins, float *maxs, float *absMins, float *absMaxs);
void __cdecl CG_GetEntityDobjBounds(const centity_s *cent, const DObj_s *dobj, float *absMins, float *absMaxs);
void __cdecl CG_LocationalTrace(trace_t *results, float *start, float *end, int32_t passEntityNum, int32_t contentMask);
void __cdecl CG_Trace(
    trace_t *results,
    float *start,
    float *mins,
    float *maxs,
    float *end,
    int32_t passEntityNum,
    int32_t contentMask,
    bool locational,
    bool staticModels);
void __cdecl CG_ClipMoveToEntities(const moveclip_t *clip, trace_t *results);
void __cdecl CG_ClipMoveToEntities_r(
    const moveclip_t *clip,
    uint16_t sectorIndex,
    const float *p1,
    const float *p2,
    trace_t *results);
void __cdecl CG_ClipMoveToEntity(const moveclip_t *clip, uint32_t entIndex, trace_t *results);
int32_t __cdecl CG_GetEntityBModelContents(const centity_s *cent);
void __cdecl CG_PointTraceToEntities(const pointtrace_t *clip, trace_t *results);
void __cdecl CG_PointTraceToEntities_r(
    const pointtrace_t *clip,
    uint16_t sectorIndex,
    const float *p1,
    const float *p2,
    trace_t *results);
void __cdecl CG_PointTraceToEntity(const pointtrace_t *clip, uint32_t entIndex, trace_t *results);
void __cdecl CG_LocationTraceDobjCalcPose(const DObj_s *dobj, const cpose_t *pose, int32_t *partBits);
void __cdecl CG_LocationalTraceEntitiesOnly(
    trace_t *results,
    float *start,
    float *end,
    int32_t passEntityNum,
    int32_t contentMask);
void __cdecl CG_TraceCapsule(
    trace_t *results,
    const float *start,
    const float *mins,
    const float *maxs,
    const float *end,
    int32_t passEntityNum,
    int32_t contentMask);



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
    int32_t timeStart;
    int32_t timeDuration;
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
void __cdecl CG_VisionSetsUpdate(int32_t localClientNum);
void __cdecl UpdateVarsLerp(
    int32_t time,
    const visionSetVars_t *from,
    const visionSetVars_t *to,
    visionSetLerpData_t *lerpData,
    visionSetVars_t *result);
bool __cdecl LerpBool(bool from, bool to, float fraction, visionSetLerpStyle_t style);
double __cdecl LerpFloat(float from, float to, float fraction, visionSetLerpStyle_t style);
void __cdecl LerpVec3(float *from, float *to, float fraction, visionSetLerpStyle_t style, float *result);
char __cdecl CG_VisionSetStartLerp_To(
    int32_t localClientNum,
    visionSetMode_t mode,
    visionSetLerpStyle_t style,
    char *nameTo,
    int32_t duration);
char __cdecl GetVisionSet(int32_t localClientNum, char *name, visionSetVars_t *resultSettings);
char __cdecl LoadVisionFile(const char *name, visionSetVars_t *resultSettings);
char *__cdecl RawBufferOpen(const char *name, const char *formatFullPath);
char __cdecl LoadVisionSettingsFromBuffer(const char *buffer, const char *filename, visionSetVars_t *settings);
char __cdecl ApplyTokenToField(uint32_t fieldNum, const char *token, visionSetVars_t *settings);
char __cdecl VisionSetCurrent(int32_t localClientNum, visionSetMode_t mode, char *name);
void __cdecl SetDefaultVision(int32_t localClientNum);
void __cdecl CG_VisionSetConfigString_Naked(int32_t localClientNum);
void __cdecl CG_VisionSetConfigString_Night(int32_t localClientNum);
void __cdecl CG_VisionSetMyChanges();
void __cdecl CG_VisionSetUpdateTweaksFromFile_Glow();
bool __cdecl LoadVisionFileForTweaks(visionSetVars_t *setVars);
void __cdecl CG_VisionSetUpdateTweaksFromFile_Film();
char __cdecl CG_LookingThroughNightVision(int32_t localClientNum);
void __cdecl CG_VisionSetApplyToRefdef(int32_t localClientNum);
double __cdecl VisionFadeValue(int32_t localClientNum);
void __cdecl FadeRefDef(refdef_s *rd, float brightness);



// cg_shellshock
void __cdecl CG_PerturbCamera(cg_s *cgameGlob);
int32_t __cdecl CG_DrawShellShockSavedScreenBlendBlurred(
    int32_t localClientNum,
    const shellshock_parms_t *parms,
    int32_t start,
    int32_t duration);
void __cdecl SaveScreenToBuffer(int32_t localClientNum);
int32_t __cdecl CG_DrawShellShockSavedScreenBlendFlashed(
    int32_t localClientNum,
    const shellshock_parms_t *parms,
    int32_t start,
    int32_t duration);
double __cdecl BlendSmooth(float percent);
void __cdecl CG_UpdateShellShock(int32_t localClientNum, const shellshock_parms_t *parms, int32_t start, int32_t duration);
void __cdecl EndShellShock(int32_t localClientNum);
void __cdecl EndShellShockSound(int32_t localClientNum);
void __cdecl EndShellShockLookControl(int32_t localClientNum);
void __cdecl EndShellShockCamera(int32_t localClientNum);
void __cdecl EndShellShockScreen(int32_t localClientNum);
void __cdecl UpdateShellShockSound(int32_t localClientNum, const shellshock_parms_t *parms, int32_t time, int32_t duration);
void __cdecl UpdateShellShockLookControl(int32_t localClientNum, const shellshock_parms_t *parms, int32_t time, int32_t duration);
void __cdecl UpdateShellShockCamera(int32_t localClientNum, const shellshock_parms_t *parms, int32_t time, int32_t duration);
double __cdecl CubicInterpolate(float t, float x0, float x1, float x2, float x3);
void __cdecl CG_StartShellShock(cg_s *cgameGlob, const shellshock_parms_t *parms, int32_t start, int32_t duration);
bool __cdecl CG_Flashbanged(int32_t localClientNum);



// cg_pose_utils
void __cdecl CG_UsedDObjCalcPose(cpose_t *pose);
void __cdecl CG_CullIn(cpose_t *pose);



// cg_playerstate
struct transPlayerState_t // sizeof=0x18
{                                       // ...
    int32_t damageEvent;
    int32_t eventSequence;
    int32_t events[4];
};
void __cdecl CG_Respawn(int32_t localClientNum);
int32_t __cdecl CG_TransitionPlayerState(int32_t localClientNum, playerState_s *ps, const transPlayerState_t *ops);
void __cdecl CG_DamageFeedback(int32_t localClientNum, int32_t yawByte, int32_t pitchByte, int32_t damage);
int32_t __cdecl CG_CheckPlayerstateEvents(int32_t localClientNum, playerState_s *ps, const transPlayerState_t *ops);



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

int32_t __cdecl CG_GetBoneIndex(
    int32_t localClientNum,
    uint32_t dobjHandle,
    uint32_t boneName,
    uint8_t *boneIndex);
void __cdecl CG_PlayBoltedEffect(
    int32_t localClientNum,
    const FxEffectDef *fxDef,
    uint32_t dobjHandle,
    uint32_t boneName);
void __cdecl CG_EntityEvent(int32_t localClientNum, centity_s *cent, int32_t event);
void __cdecl CG_Obituary(int32_t localClientNum, const entityState_s *ent);
void __cdecl CG_ItemPickup(int32_t localClientNum, int32_t weapIndex);
void __cdecl CG_EquipmentSound(int32_t localClientNum, int32_t entNum, bool isPlayerView, EquipmentSound_t type);
void __cdecl CG_PlayFx(int32_t localClientNum, centity_s *cent, const float *angles);
void __cdecl CG_PlayFxOnTag(int32_t localClientNum, centity_s *cent, int32_t eventParm);
void __cdecl CG_SetInvalidCmdHint(cg_s *cgameGlob, InvalidCmdHintType hintType);
void __cdecl CG_StopWeaponSound(
    int32_t localClientNum,
    bool isPlayerView,
    const WeaponDef *weaponDef,
    int32_t entitynum,
    weaponstate_t weaponstate);
void __cdecl CG_CheckEvents(int32_t localClientNum, centity_s *cent);



// cg_draw_reticles
void __cdecl CG_CalcCrosshairPosition(const cg_s *cgameGlob, float *x, float *y);
char __cdecl CG_GetWeapReticleZoom(const cg_s *cgameGlob, float *zoom);
void __cdecl CG_DrawNightVisionOverlay(int32_t localClientNum);
void __cdecl CG_DrawCrosshair(int32_t localClientNum);
void __cdecl CG_DrawAdsOverlay(
    int32_t localClientNum,
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
bool __cdecl CG_UsingLowResViewPort(int32_t localClientNum);
void __cdecl CG_UpdateScissorViewport(refdef_s *refdef, float *drawPos, float *drawSize);
double __cdecl CG_DrawWeapReticle(int32_t localClientNum);
void __cdecl CG_CalcCrosshairColor(int32_t localClientNum, float alpha, float *color);
void __cdecl CG_DrawTurretCrossHair(int32_t localClientNum);
char __cdecl AllowedToDrawCrosshair(int32_t localClientNum, const playerState_s *predictedPlayerState);
bool __cdecl CG_IsReticleTurnedOff();
void __cdecl CG_DrawAdsAimIndicator(
    int32_t localClientNum,
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
    int32_t localClientNum,
    const WeaponDef *weapDef,
    const float *color,
    float centerX,
    float centerY,
    float transScale);
void __cdecl CG_DrawReticleSides(
    int32_t localClientNum,
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
void __cdecl CG_DrawDamageDirectionIndicators(int32_t localClientNum);
void __cdecl CG_ClearHudGrenades();
char __cdecl CG_AddHudGrenade_PositionCheck(const cg_s *cgameGlob, const centity_s *grenadeEnt, WeaponDef *weapDef);
void __cdecl CG_AddHudGrenade(const cg_s *cgameGlob, const centity_s *grenadeEnt);
void __cdecl CG_DrawGrenadeIndicators(int32_t localClientNum);
void __cdecl CG_DrawGrenadePointer(
    int32_t localClientNum,
    float centerX,
    float centerY,
    const float *grenadeOffset,
    const float *color);
void __cdecl CG_DrawGrenadeIcon(
    int32_t localClientNum,
    float centerX,
    float centerY,
    const float *grenadeOffset,
    const float *color,
    Material *material);



// cg_draw_debug
struct meminfo_t;
void __cdecl CG_CalculateFPS();
double __cdecl CG_DrawFPS(const ScreenPlacement *scrPlace, float y, meminfo_t *meminfo);
bool __cdecl CG_Flash(int32_t timeMs);
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
void __cdecl CG_DrawUpperRightDebugInfo(int32_t localClientNum);
double __cdecl CG_DrawSnapshot(int32_t localClientNum, float posY);
double __cdecl CG_DrawStatmon(const ScreenPlacement *scrPlace, float y, meminfo_t *meminfo);
void __cdecl CG_DrawPerformanceWarnings();
void __cdecl CG_DrawDebugOverlays(int32_t localClientNum);
void __cdecl CG_DrawMaterial(int32_t localClientNum, uint32_t drawMaterialType);
void __cdecl CG_DrawDebugPlayerHealth(int32_t localClientNum);
void __cdecl CG_DrawFullScreenDebugOverlays(int32_t localClientNum);
void __cdecl CG_DrawScriptUsage(const ScreenPlacement *scrPlace);
void CG_DrawVersion();
void __cdecl CG_DrawSoundEqOverlay(int32_t localClientNum);
void __cdecl CG_DrawSoundOverlay(const ScreenPlacement *scrPlace);
void __cdecl CG_DrawFxProfile(int32_t localClientNum);
void __cdecl CG_DrawFxText(char *text, float *profilePos);
void __cdecl CG_DrawFxMarkProfile(int32_t localClientNum);



// cg_compass
enum CompassType : __int32
{                                       // ...
    COMPASS_TYPE_PARTIAL = 0x0,
    COMPASS_TYPE_FULL = 0x1,
};

void __cdecl CG_CompassRegisterDvars();
bool __cdecl CG_IsSelectingLocation(int32_t localClientNum);
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
double __cdecl CG_FadeCompass(int32_t localClientNum, int32_t displayStartTime, CompassType compassType);
void __cdecl CG_CompassDrawPlayerBack(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Material *material,
    float *color);
void __cdecl CG_CompassDrawPlayerNorthCoord(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Font_s *font,
    Material *material,
    float *const color,
    int32_t style);
void __cdecl CG_CompassDrawPlayerEastCoord(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Font_s *font,
    Material *material,
    float *const color,
    int32_t style);
void __cdecl CG_CompassDrawPlayerNCoordScroll(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Font_s *font,
    Material *material,
    float *color,
    int32_t textStyle);
void __cdecl CG_CompassDrawPlayerECoordScroll(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Font_s *font,
    Material *material,
    float *color,
    int32_t textStyle);
void __cdecl CG_CompassDrawPlayerMap(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Material *material,
    float *color);
void __cdecl CG_CompassDrawPlayerMapLocationSelector(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Material *material,
    float *color);
void __cdecl CG_CompassDrawPlayer(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    rectDef_s *rect,
    Material *material,
    float *color);
void __cdecl CG_CompassDrawBorder(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    rectDef_s *rect,
    Material *material,
    float *color);
void __cdecl CG_CompassUpYawVector(const cg_s *cgameGlob, float *result);
void __cdecl CG_CompassDrawTickertape(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Material *material,
    const float *color,
    Font_s *textFont,
    float textScale,
    int32_t textStyle,
    bool drawObjectives);
void __cdecl CalcCompassPointerSize(CompassType compassType, float *w, float *h);
void __cdecl DrawIconDistanceText(
    int32_t localClientNum,
    float distance,
    float iconX,
    float iconY,
    float iconH,
    const rectDef_s *rect,
    const float *color,
    Font_s *textFont,
    float textScale,
    int32_t textStyle);
double __cdecl CutFloat(float original);
double __cdecl CG_GetHudAlphaCompass(int32_t localClientNum);
void __cdecl CalcCompassFriendlySize(CompassType compassType, float *w, float *h);
void __cdecl CG_CompassDrawPlayerPointers_MP(
    int32_t localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Material *material,
    const float *color);
double __cdecl GetObjectiveFade(const rectDef_s *clipRect, float x, float y, float width, float height);



// cg_colltree
struct CgEntCollNode // sizeof=0x14
{                                       // ...
    uint16_t sector;
    uint16_t nextEntInSector;
    float linkMins[2];
    float linkMaxs[2];
};
union CgEntCollTree_u // sizeof=0x2
{                                       // ...
    uint16_t parent;
    uint16_t nextFree;
};
struct CgEntCollTree // sizeof=0xC
{                                       // ...
    float dist;
    uint16_t axis;
    CgEntCollTree_u u;
    uint16_t child[2];
};
struct CgEntCollSector // sizeof=0x10
{                                       // ...
    CgEntCollTree tree;
    uint16_t entListHead;
    // padding byte
    // padding byte
};
struct CgEntCollWorld // sizeof=0x401C
{                                       // ...
    float mins[3];
    float maxs[3];
    uint16_t freeHead;
    // padding byte
    // padding byte
    CgEntCollSector sectors[1024];
};
void __cdecl TRACK_CG_CollWorld();
void __cdecl CG_SetCollWorldLocalClientNum(int32_t localClientNum);
int32_t __cdecl CG_GetCollWorldLocalClientNum();
void __cdecl CG_ClearEntityCollWorld(int32_t localClientNum);
const CgEntCollSector *__cdecl CG_GetEntityCollSector(int32_t localClientNum, uint16_t sectorIndex);
const CgEntCollNode *__cdecl CG_GetEntityCollNode(int32_t localClientNum, uint32_t entIndex);
CgEntCollNode *__cdecl CG_GetCollNode(int32_t localClientNum, uint32_t entIndex);
void __cdecl CG_UnlinkEntityColl(int32_t localClientNum, uint32_t entIndex);
void __cdecl CG_LinkEntityColl(int32_t localClientNum, uint32_t entIndex, const float *absMins, const float *absMaxs);
void __cdecl CG_AddEntityToCollSector(int32_t localClientNum, uint32_t entIndex, uint16_t sectorIndex);
void __cdecl CG_SortEntityCollSector(
    int32_t localClientNum,
    uint16_t sectorIndex,
    const float *mins,
    const float *maxs);
uint16_t __cdecl CG_AllocEntityCollSector(int32_t localClientNum, const float *mins, const float *maxs);



// cg_camerashake
struct CameraShake // sizeof=0x24
{                                       // ...
    int32_t time;                           // ...
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
void __cdecl CG_StartShakeCamera(int32_t localClientNum, float p, int32_t duration, float *src, float radius);
int32_t __cdecl CG_UpdateCameraShake(const cg_s *cgameGlob, CameraShake *shake);
void __cdecl CG_ShakeCamera(int32_t localClientNum);
void __cdecl CG_ClearCameraShakes(int32_t localClientNum);



// cg_ammocounter
void __cdecl CG_AmmoCounterRegisterDvars();
void __cdecl CG_DrawPlayerWeaponAmmoStock(
    int32_t localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    float *color,
    Material *material,
    int32_t textStyle);
uint32_t __cdecl ClipCounterWeapIdx(const cg_s *cgameGlob, uint32_t weapIndex);
uint32_t __cdecl GetWeaponAltIndex(const cg_s *cgameGlob, const WeaponDef *weapDef);
double __cdecl AmmoCounterFadeAlpha(int32_t localClientNum, cg_s *cgameGlob);
double __cdecl CG_GetHudAlphaDPad(int32_t localClientNum);
double __cdecl DpadFadeAlpha(int32_t localClientNum, cg_s *cgameGlob);
bool __cdecl ActionSlotIsActive(int32_t localClientNum, uint32_t slotIdx);
double __cdecl CG_GetHudAlphaAmmoCounter(int32_t localClientNum);
bool __cdecl CG_ActionSlotIsUsable(int32_t localClientNum, uint32_t slotIdx);
void __cdecl CG_DrawPlayerActionSlotDpad(
    int32_t localClientNum,
    const rectDef_s *rect,
    const float *color,
    Material *material);
void __cdecl CG_DrawPlayerActionSlot(
    int32_t localClientNum,
    const rectDef_s *rect,
    uint32_t slotIdx,
    float *color,
    Font_s *textFont,
    float textScale,
    int32_t textStyle);
void __cdecl DpadIconDims(
    const rectDef_s *rect,
    uint32_t slotIdx,
    WeaponDef *weapDef,
    float *x,
    float *y,
    float *w,
    float *h);
void __cdecl DpadTextPos(const rectDef_s *rect, uint32_t slotIdx, WeaponDef *weapDef, float *x, float *y);
void __cdecl CG_DrawPlayerWeaponBackground(
    int32_t localClientNum,
    const rectDef_s *rect,
    const float *color,
    Material *material);
void __cdecl CG_DrawPlayerWeaponAmmoClipGraphic(int32_t localClientNum, const rectDef_s *rect, const float *color);
void __cdecl GetBaseRectPos(int32_t localClientNum, const rectDef_s *rect, float *base);
void __cdecl DrawClipAmmo(cg_s *cgameGlob, float *base, uint32_t weapIdx, const WeaponDef *weapDef, float *color);
void __cdecl DrawClipAmmoMagazine(
    cg_s *cgameGlob,
    const float *base,
    uint32_t weapIdx,
    const WeaponDef *weapDef,
    float *color);
void __cdecl AmmoColor(cg_s *cgameGlob, float *color, uint32_t weapIndex);
void __cdecl DrawClipAmmoShortMagazine(
    cg_s *cgameGlob,
    const float *base,
    uint32_t weapIdx,
    const WeaponDef *weapDef,
    float *color);
void __cdecl DrawClipAmmoShotgunShells(
    cg_s *cgameGlob,
    const float *base,
    uint32_t weapIdx,
    const WeaponDef *weapDef,
    float *color);
void __cdecl DrawClipAmmoRockets(
    cg_s *cgameGlob,
    const float *base,
    uint32_t weapIdx,
    const WeaponDef *weapDef,
    float *color);
void __cdecl DrawClipAmmoBeltfed(
    cg_s *cgameGlob,
    float *base,
    uint32_t weapIdx,
    const WeaponDef *weapDef,
    float *color);
void __cdecl CG_DrawPlayerWeaponIcon(int32_t localClientNum, const rectDef_s *rect, const float *color);
void __cdecl DrawStretchPicGun(
    const ScreenPlacement *scrPlace,
    const rectDef_s *rect,
    const float *color,
    Material *material,
    weaponIconRatioType_t ratio);
void __cdecl CG_DrawPlayerWeaponLowAmmoWarning(
    int32_t localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float textScale,
    int32_t textStyle,
    float text_x,
    float text_y,
    char textAlignMode,
    Material *material);
uint32_t __cdecl GetWeaponIndex(const cg_s *cgameGlob);
int BG_PlayerHasWeapon(const playerState_s *ps, int weaponIndex);
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
void __cdecl CG_LoadingString(int32_t localClientNum, const char *s);
void __cdecl CG_DrawInformation(int32_t localClientNum);
bool __cdecl CG_IsShowingProgress_FastFile();



extern const dvar_t *nightVisionFadeInOutTime;
extern const dvar_t *nightVisionPowerOnTime;
extern const dvar_t *nightVisionDisableEffects;


#ifdef KISAK_SP
#include "cg_main.h"

inline int CG_GetEntityIndex(const centity_s *cent)
{
    iassert(cent->nextState.number == (cent - &cg_entitiesArray[0][0]) % MAX_GENTITIES);

    return cent->nextState.number;
}

inline int CG_GetLocalClientTime(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\cgame\\cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return cgArray[localClientNum].time;
}
#endif 