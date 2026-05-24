#pragma once

// LWSS: This file has way too many structs. KISAKTODO: move out later.
#include "xanim_public.h"
#include <script/scr_stringlist.h>
#include <gfx_d3d/r_font.h>
#include <gfx_d3d/r_bsp.h>
#include <universal/com_math.h>
#include <bgame/bg_weapons.h>
#include <xanim/xanim.h>
#include <sound/snd_public.h>
#include "dobj.h"
#include "xmodel.h"
#include <gfx_d3d/r_material.h>
#include <gfx_d3d/r_gfx.h>

#include <game/pathnode.h>

#include <ui/ui_shared.h>

#define ANIM_FLAG_COMPLETE 1

enum WeapAccuracyType : __int32
{
    WEAP_ACCURACY_AI_VS_AI = 0x0,
    WEAP_ACCURACY_AI_VS_PLAYER = 0x1,
    WEAP_ACCURACY_COUNT = 0x2,
};

union XAnimIndices // sizeof=0x4
{                                       // ...
    unsigned __int8 *_1;
    uint16_t *_2;
    void *data;
};
static_assert(sizeof(XAnimIndices) == 4);
struct XAnimNotifyInfo // sizeof=0x8
{
    uint16_t name;
    // padding byte
    // padding byte
    float time;
};
union XAnimDynamicIndices // sizeof=0x2
{                                       // ...
    unsigned __int8 _1[1];
    uint16_t _2[1];
};
struct XAnimDeltaPartQuatDataFrames // sizeof=0x8
{                                       // ...
    __int16 (*frames)[2];
    XAnimDynamicIndices indices;
    // padding byte
    // padding byte
};

union XAnimDynamicFrames // sizeof=0x4
{                                       // ...
    unsigned __int8 (*_1)[3];
    uint16_t (*_2)[3];
};
struct XAnimPartTransFrames // sizeof=0x20
{                                       // ...
    float mins[3];
    float size[3];
    XAnimDynamicFrames frames;
    XAnimDynamicIndices indices;
    // padding byte
    // padding byte
};

union XAnimPartTransData // sizeof=0x20
{                                       // ...
    XAnimPartTransFrames frames;
    float frame0[3];
};
struct XAnimPartTrans // sizeof=0x24
{
    uint16_t size;
    unsigned __int8 smallTrans;
    // padding byte
    XAnimPartTransData u;
};
union XAnimDeltaPartQuatData // sizeof=0x8
{                                       // ...
    XAnimDeltaPartQuatDataFrames frames;
    __int16 frame0[2];
};
union XAnimPartQuatFrames // sizeof=0x4
{                                       // ...
    __int16 (*frames)[4];
    __int16 (*frames2)[2];
};
struct XAnimPartQuatDataFrames // sizeof=0x8
{                                       // ...
    XAnimPartQuatFrames u;
    XAnimDynamicIndices indices;
    // padding byte
    // padding byte
};
union XAnimPartQuatData // sizeof=0x8
{                                       // ...
    XAnimPartQuatDataFrames frames;
    __int16 frame0[4];
    __int16 frame02[2];
};
struct XAnimPartQuat // sizeof=0xC
{
    uint16_t size;
    // padding byte
    // padding byte
    XAnimPartQuatData u;
};
struct XAnimPartQuatPtr // sizeof=0x8
{                                       // ...
    XAnimPartQuat *quat;                // ...
    unsigned __int8 partIndex;          // ...
    // padding byte
    // padding byte
    // padding byte
};
struct XAnimPartTransPtr // sizeof=0x8
{                                       // ...
    XAnimPartTrans *trans;              // ...
    unsigned __int8 partIndex;          // ...
    // padding byte
    // padding byte
    // padding byte
};

struct XAnimDeltaPartQuat // sizeof=0xC
{
    uint16_t size;
    // padding byte
    // padding byte
    XAnimDeltaPartQuatData u;
};
struct XAnimDeltaPart // sizeof=0x8
{
    XAnimPartTrans *trans;
    XAnimDeltaPartQuat *quat;
};
struct XAnimTime // sizeof=0xC
{                                       // ...
    float time;
    float frameFrac;
    int frameIndex;
};
struct XAnimParts // sizeof=0x58
{                                       // ...
    const char *name;
    uint16_t dataByteCount;
    uint16_t dataShortCount;
    uint16_t dataIntCount;
    uint16_t randomDataByteCount;
    uint16_t randomDataIntCount;
    uint16_t numframes;
    bool bLoop;
    bool bDelta;
    unsigned __int8 boneCount[10];
    unsigned __int8 notifyCount;
    unsigned __int8 assetType;
    bool isDefault;
    // padding byte
    uint32_t randomDataShortCount;
    uint32_t indexCount;
    float framerate;
    float frequency;
    uint16_t *names;
    unsigned __int8 *dataByte;
    __int16 *dataShort;
    int *dataInt;
    __int16 *randomDataShort;
    unsigned __int8 *randomDataByte;
    int *randomDataInt;
    XAnimIndices indices;
    XAnimNotifyInfo *notify;
    XAnimDeltaPart *deltaPart;
};
static_assert(sizeof(XAnimParts) == 88);

struct XModelNameMap // sizeof=0x4
{                                       // ...
    uint16_t name;              // ...
    uint16_t index;
};

struct XAnimParent // sizeof=0x4
{                                       // ...
    uint16_t flags;
    uint16_t children;
};
struct XAnimEntry // sizeof=0x8
{                                       // ...
    uint16_t numAnims;
    uint16_t parent;
    union //$7F333398CC08E12E110886895274CBFC
    {
        XAnimParts *parts;
        XAnimParent animParent;
    };
};
struct XAnim_s // sizeof=0x14
{
    const char *debugName;
    uint32_t size;
    const char **debugAnimNames;
    XAnimEntry entries[1];
};

struct XAnimTree_s // sizeof=0x14
{
    XAnim_s *anims;
    int info_usage;
    volatile long calcRefCount;
    volatile long modifyRefCount;
    uint16_t children;
    // padding byte
    // padding byte
};
struct mnode_t // sizeof=0x4
{
    uint16_t cellIndex;
    uint16_t rightChildOffset;
};

struct XAnimState // sizeof=0x20
{                                       // ...
    float currentAnimTime;              // ...
    float oldTime;                      // ...
    __int16 cycleCount;                 // ...
    __int16 oldCycleCount;              // ...
    float goalTime;                     // ...
    float goalWeight;                   // ...
    float weight;                       // ...
    float rate;                         // ...
    bool instantWeightChange;           // ...
    // padding byte
    // padding byte
    // padding byte
};

struct XAnimInfo // sizeof=0x40
{                                       // ...
    uint16_t notifyChild;
    __int16 notifyIndex;
    uint16_t notifyName;
    uint16_t notifyType;
    uint16_t prev;              // ...
    uint16_t next;              // ...
    uint16_t children;          // ...
    uint16_t parent;            // ...
    uint16_t animIndex;         // ...
    uint16_t animToModel;
    bool inuse;                         // ...
    // padding byte
    // padding byte
    // padding byte
    XAnimTree_s* tree;
    //$7F333398CC08E12E110886895274CBFC ___u12;
    union
    {                                       // ...
        XAnimParts* parts;
        XAnimParent animParent;
    };
    XAnimState state;                   // ...
};

struct XAnimSimpleRotPos // sizeof=0x18
{                                       // ...
    float rot[2];                       // ...
    float posWeight;                    // ...
    float pos[3];                       // ...
};

struct XAnimDeltaInfo // sizeof=0x4
{                                       // ...
    bool bClear;                        // ...
    bool bNormQuat;                     // ...
    bool bAbs;                          // ...
    bool bUseGoalWeight;                // ...
};

struct XAnimNotify_s // sizeof=0xC
{                                       // ...
    const char* name;
    uint32_t type;
    float timeFrac;
};

struct cStaticModelWritable // sizeof=0x2
{                                       // ...
    uint16_t nextModelInWorldSector;
};
struct cStaticModel_s // sizeof=0x50
{
    cStaticModelWritable writable;
    // padding byte
    // padding byte
    XModel* xmodel;
    float origin[3];
    float invScaledAxis[3][3];
    float absmin[3];
    float absmax[3];
};
struct dmaterial_t // sizeof=0x48
{
    char material[64];
    int surfaceFlags;
    int contentFlags;
};
struct cNode_t // sizeof=0x8
{
    cplane_s* plane;
    __int16 children[2];
};

struct cLeafBrushNodeLeaf_t // sizeof=0x4
{                                       // ...
    uint16_t* brushes;
};
struct cLeafBrushNodeChildren_t // sizeof=0xC
{                                       // ...
    float dist;
    float range;
    uint16_t childOffset[2];
};
union cLeafBrushNodeData_t // sizeof=0xC
{                                       // ...
    cLeafBrushNodeLeaf_t leaf;
    cLeafBrushNodeChildren_t children;
};
struct cLeafBrushNode_s // sizeof=0x14
{
    unsigned __int8 axis;
    // padding byte
    __int16 leafBrushCount;
    int contents;
    cLeafBrushNodeData_t data;
};
struct CollisionBorder // sizeof=0x1C
{
    float distEq[3];
    float zBase;
    float zSlope;
    float start;
    float length;
};
struct CollisionPartition // sizeof=0xC
{
    unsigned __int8 triCount;
    unsigned __int8 borderCount;
    // padding byte
    // padding byte
    int firstTri;
    CollisionBorder* borders;
};
union CollisionAabbTreeIndex // sizeof=0x4
{                                       // ...
    int firstChildIndex;
    int partitionIndex;
};
struct CollisionAabbTree // sizeof=0x20
{
    float origin[3];
    float halfSize[3];
    uint16_t materialIndex;
    uint16_t childCount;
    CollisionAabbTreeIndex u;
};

struct cbrushside_t // sizeof=0xC
{                                       // ...
    cplane_s* plane;                    // ...
    uint32_t materialNum;           // ...
    __int16 firstAdjacentSideOffset;
    unsigned __int8 edgeCount;
    // padding byte
};

struct __declspec(align(16)) cbrush_t // sizeof=0x50
{                                       // ...
    float mins[3];
    int contents;
    float maxs[3];
    uint32_t numsides;
    cbrushside_t* sides;
    __int16 axialMaterialNum[2][3];
    unsigned __int8* baseAdjacentSide;
    __int16 firstAdjacentSideOffsets[2][3];
    unsigned __int8 edgeCount[2][3];
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct MapEnts // sizeof=0xC
{                                       // ...
    const char* name;
    char* entityString;
    int numEntityChars;
};

struct DynEntityDef;
struct DynEntityPose;
struct DynEntityClient;
struct DynEntityColl;

struct ComPrimaryLight // sizeof=0x44 (SP/MP Same)
{
    unsigned __int8 type;
    unsigned __int8 canUseShadowMap;
    unsigned __int8 exponent;
    unsigned __int8 unused;
    float color[3];
    float dir[3];
    float origin[3];
    float radius;
    float cosHalfFovOuter;
    float cosHalfFovInner;
    float cosHalfFovExpanded;
    float rotationLimit;
    float translationLimit;
    const char* defName;
};
struct ComWorld // sizeof=0x10 (SP/MP Same)
{                                       // ...
    const char* name;                   // ...
    int isInUse;                        // ...
    uint32_t primaryLightCount;     // ...
    ComPrimaryLight* primaryLights;     // ...
};

struct XModelDrawInfo // sizeof=0x4
{                                       // ...
    uint16_t lod;
    uint16_t surfId;
};
struct GfxSceneDynModel // sizeof=0x6
{
    XModelDrawInfo info;
    uint16_t dynEntId;
};
struct BModelDrawInfo // sizeof=0x2
{                                       // ...
    uint16_t surfId;
};
struct GfxSceneDynBrush // sizeof=0x4
{
    BModelDrawInfo info;
    uint16_t dynEntId;
};

struct WeaponDef // sizeof=0x878
{                                       // ...
    const char* szInternalName;
    const char* szDisplayName;
    const char* szOverlayName;
    XModel* gunXModel[16];
    XModel* handXModel;
    const char* szXAnims[33];
    const char* szModeName;
    uint16_t hideTags[8];
    uint16_t notetrackSoundMapKeys[16];
    uint16_t notetrackSoundMapValues[16];
    int playerAnimType;
    weapType_t weapType;
    weapClass_t weapClass;
    PenetrateType penetrateType;
    ImpactType impactType;
    weapInventoryType_t inventoryType;
    weapFireType_t fireType;
    OffhandClass offhandClass;
    weapStance_t stance;
    const FxEffectDef* viewFlashEffect;
    const FxEffectDef* worldFlashEffect;
    snd_alias_list_t* pickupSound;
    snd_alias_list_t* pickupSoundPlayer;
    snd_alias_list_t* ammoPickupSound;
    snd_alias_list_t* ammoPickupSoundPlayer;
    snd_alias_list_t* projectileSound;
    snd_alias_list_t* pullbackSound;
    snd_alias_list_t* pullbackSoundPlayer;
    snd_alias_list_t* fireSound;
    snd_alias_list_t* fireSoundPlayer;
    snd_alias_list_t* fireLoopSound;
    snd_alias_list_t* fireLoopSoundPlayer;
    snd_alias_list_t* fireStopSound;
    snd_alias_list_t* fireStopSoundPlayer;
    snd_alias_list_t* fireLastSound;
    snd_alias_list_t* fireLastSoundPlayer;
    snd_alias_list_t* emptyFireSound;
    snd_alias_list_t* emptyFireSoundPlayer;
    snd_alias_list_t* meleeSwipeSound;
    snd_alias_list_t* meleeSwipeSoundPlayer;
    snd_alias_list_t* meleeHitSound;
    snd_alias_list_t* meleeMissSound;
    snd_alias_list_t* rechamberSound;
    snd_alias_list_t* rechamberSoundPlayer;
    snd_alias_list_t* reloadSound;
    snd_alias_list_t* reloadSoundPlayer;
    snd_alias_list_t* reloadEmptySound;
    snd_alias_list_t* reloadEmptySoundPlayer;
    snd_alias_list_t* reloadStartSound;
    snd_alias_list_t* reloadStartSoundPlayer;
    snd_alias_list_t* reloadEndSound;
    snd_alias_list_t* reloadEndSoundPlayer;
    snd_alias_list_t* detonateSound;
    snd_alias_list_t* detonateSoundPlayer;
    snd_alias_list_t* nightVisionWearSound;
    snd_alias_list_t* nightVisionWearSoundPlayer;
    snd_alias_list_t* nightVisionRemoveSound;
    snd_alias_list_t* nightVisionRemoveSoundPlayer;
    snd_alias_list_t* altSwitchSound;
    snd_alias_list_t* altSwitchSoundPlayer;
    snd_alias_list_t* raiseSound;
    snd_alias_list_t* raiseSoundPlayer;
    snd_alias_list_t* firstRaiseSound;
    snd_alias_list_t* firstRaiseSoundPlayer;
    snd_alias_list_t* putawaySound;
    snd_alias_list_t* putawaySoundPlayer;
    snd_alias_list_t** bounceSound;
    const FxEffectDef* viewShellEjectEffect;
    const FxEffectDef* worldShellEjectEffect;
    const FxEffectDef* viewLastShotEjectEffect;
    const FxEffectDef* worldLastShotEjectEffect;
    Material* reticleCenter;
    Material* reticleSide;
    int iReticleCenterSize;
    int iReticleSideSize;
    int iReticleMinOfs;
    activeReticleType_t activeReticleType;
    float vStandMove[3];
    float vStandRot[3];
    float vDuckedOfs[3];
    float vDuckedMove[3];
    float vDuckedRot[3];
    float vProneOfs[3];
    float vProneMove[3];
    float vProneRot[3];
    float fPosMoveRate;
    float fPosProneMoveRate;
    float fStandMoveMinSpeed;
    float fDuckedMoveMinSpeed;
    float fProneMoveMinSpeed;
    float fPosRotRate;
    float fPosProneRotRate;
    float fStandRotMinSpeed;
    float fDuckedRotMinSpeed;
    float fProneRotMinSpeed;
    XModel* worldModel[16];
    XModel* worldClipModel;
    XModel* rocketModel;
    XModel* knifeModel;
    XModel* worldKnifeModel;
    Material* hudIcon;
    weaponIconRatioType_t hudIconRatio;
    Material* ammoCounterIcon;
    weaponIconRatioType_t ammoCounterIconRatio;
    ammoCounterClipType_t ammoCounterClip;
    int iStartAmmo;
    const char* szAmmoName;
    int iAmmoIndex;
    const char* szClipName;
    int iClipIndex;
    int iMaxAmmo;
    int iClipSize;
    int shotCount;
    const char* szSharedAmmoCapName;
    int iSharedAmmoCapIndex;
    int iSharedAmmoCap;
    int damage;
    int playerDamage;
    int iMeleeDamage;
    int iDamageType;
    int iFireDelay;
    int iMeleeDelay;
    int meleeChargeDelay;
    int iDetonateDelay;
    int iFireTime;
    int iRechamberTime;
    int iRechamberBoltTime;
    int iHoldFireTime;
    int iDetonateTime;
    int iMeleeTime;
    int meleeChargeTime;
    int iReloadTime;
    int reloadShowRocketTime;
    int iReloadEmptyTime;
    int iReloadAddTime;
    int iReloadStartTime;
    int iReloadStartAddTime;
    int iReloadEndTime;
    int iDropTime;
    int iRaiseTime;
    int iAltDropTime;
    int iAltRaiseTime;
    int quickDropTime;
    int quickRaiseTime;
    int iFirstRaiseTime;
    int iEmptyRaiseTime;
    int iEmptyDropTime;
    int sprintInTime;
    int sprintLoopTime;
    int sprintOutTime;
    int nightVisionWearTime;
    int nightVisionWearTimeFadeOutEnd;
    int nightVisionWearTimePowerUp;
    int nightVisionRemoveTime;
    int nightVisionRemoveTimePowerDown;
    int nightVisionRemoveTimeFadeInStart;
    int fuseTime;
    int aiFuseTime;
    int requireLockonToFire;
    int noAdsWhenMagEmpty;
    int avoidDropCleanup;
    float autoAimRange;
    float aimAssistRange;
    float aimAssistRangeAds;
    float aimPadding;
    float enemyCrosshairRange;
    int crosshairColorChange;
    float moveSpeedScale;
    float adsMoveSpeedScale;
    float sprintDurationScale;
    float fAdsZoomFov;
    float fAdsZoomInFrac;
    float fAdsZoomOutFrac;
    Material* overlayMaterial;
    Material* overlayMaterialLowRes;
    weapOverlayReticle_t overlayReticle;
    WeapOverlayInteface_t overlayInterface;
    float overlayWidth;
    float overlayHeight;
    float fAdsBobFactor;
    float fAdsViewBobMult;
    float fHipSpreadStandMin;
    float fHipSpreadDuckedMin;
    float fHipSpreadProneMin;
    float hipSpreadStandMax;
    float hipSpreadDuckedMax;
    float hipSpreadProneMax;
    float fHipSpreadDecayRate;
    float fHipSpreadFireAdd;
    float fHipSpreadTurnAdd;
    float fHipSpreadMoveAdd;
    float fHipSpreadDuckedDecay;
    float fHipSpreadProneDecay;
    float fHipReticleSidePos;
    int iAdsTransInTime;
    int iAdsTransOutTime;
    float fAdsIdleAmount;
    float fHipIdleAmount;
    float adsIdleSpeed;
    float hipIdleSpeed;
    float fIdleCrouchFactor;
    float fIdleProneFactor;
    float fGunMaxPitch;
    float fGunMaxYaw;
    float swayMaxAngle;
    float swayLerpSpeed;
    float swayPitchScale;
    float swayYawScale;
    float swayHorizScale;
    float swayVertScale;
    float swayShellShockScale;
    float adsSwayMaxAngle;
    float adsSwayLerpSpeed;
    float adsSwayPitchScale;
    float adsSwayYawScale;
    float adsSwayHorizScale;
    float adsSwayVertScale;
    int bRifleBullet;
    int armorPiercing;
    int bBoltAction;
    int aimDownSight;
    int bRechamberWhileAds;
    float adsViewErrorMin;
    float adsViewErrorMax;
    int bCookOffHold;
    int bClipOnly;
    int adsFireOnly;
    int cancelAutoHolsterWhenEmpty;
    int suppressAmmoReserveDisplay;
    int enhanced;
    int laserSightDuringNightvision;
    Material* killIcon;
    weaponIconRatioType_t killIconRatio;
    int flipKillIcon;
    Material* dpadIcon;
    weaponIconRatioType_t dpadIconRatio;
    int bNoPartialReload;
    int bSegmentedReload;
    int iReloadAmmoAdd;
    int iReloadStartAdd;
    const char* szAltWeaponName;
    uint32_t altWeaponIndex;
    int iDropAmmoMin;
    int iDropAmmoMax;
    int blocksProne;
    int silenced;
    int iExplosionRadius;
    int iExplosionRadiusMin;
    int iExplosionInnerDamage;
    int iExplosionOuterDamage;
    float damageConeAngle;
    int iProjectileSpeed;
    int iProjectileSpeedUp;
    int iProjectileSpeedForward;
    int iProjectileActivateDist;
    float projLifetime;
    float timeToAccelerate;
    float projectileCurvature;
    XModel* projectileModel;
    weapProjExposion_t projExplosion;
    const FxEffectDef* projExplosionEffect;
    int projExplosionEffectForceNormalUp;
    const FxEffectDef* projDudEffect;
    snd_alias_list_t* projExplosionSound;
    snd_alias_list_t* projDudSound;
    int bProjImpactExplode;
    WeapStickinessType stickiness;
    int hasDetonator;
    int timedDetonation;
    int rotate;
    int holdButtonToThrow;
    int freezeMovementWhenFiring;
    float lowAmmoWarningThreshold;
    float parallelBounce[29];
    float perpendicularBounce[29];
    const FxEffectDef* projTrailEffect;
    float vProjectileColor[3];
    guidedMissileType_t guidedMissileType;
    float maxSteeringAccel;
    int projIgnitionDelay;
    const FxEffectDef* projIgnitionEffect;
    snd_alias_list_t* projIgnitionSound;
    float fAdsAimPitch;
    float fAdsCrosshairInFrac;
    float fAdsCrosshairOutFrac;
    int adsGunKickReducedKickBullets;
    float adsGunKickReducedKickPercent;
    float fAdsGunKickPitchMin;
    float fAdsGunKickPitchMax;
    float fAdsGunKickYawMin;
    float fAdsGunKickYawMax;
    float fAdsGunKickAccel;
    float fAdsGunKickSpeedMax;
    float fAdsGunKickSpeedDecay;
    float fAdsGunKickStaticDecay;
    float fAdsViewKickPitchMin;
    float fAdsViewKickPitchMax;
    float fAdsViewKickYawMin;
    float fAdsViewKickYawMax;
    float fAdsViewKickCenterSpeed;
    float fAdsViewScatterMin;
    float fAdsViewScatterMax;
    float fAdsSpread;
    int hipGunKickReducedKickBullets;
    float hipGunKickReducedKickPercent;
    float fHipGunKickPitchMin;
    float fHipGunKickPitchMax;
    float fHipGunKickYawMin;
    float fHipGunKickYawMax;
    float fHipGunKickAccel;
    float fHipGunKickSpeedMax;
    float fHipGunKickSpeedDecay;
    float fHipGunKickStaticDecay;
    float fHipViewKickPitchMin;
    float fHipViewKickPitchMax;
    float fHipViewKickYawMin;
    float fHipViewKickYawMax;
    float fHipViewKickCenterSpeed;
    float fHipViewScatterMin;
    float fHipViewScatterMax;
    float fightDist;
    float maxDist;
    const char* accuracyGraphName[WEAP_ACCURACY_COUNT];
    float (*accuracyGraphKnots[2])[WEAP_ACCURACY_COUNT];
    float (*originalAccuracyGraphKnots[2])[WEAP_ACCURACY_COUNT];
    int accuracyGraphKnotCount[WEAP_ACCURACY_COUNT];
    int originalAccuracyGraphKnotCount[WEAP_ACCURACY_COUNT];
    int iPositionReloadTransTime;
    float leftArc;
    float rightArc;
    float topArc;
    float bottomArc;
    float accuracy;
    float aiSpread;
    float playerSpread;
    float minTurnSpeed[2];
    float maxTurnSpeed[2];
    float pitchConvergenceTime;
    float yawConvergenceTime;
    float suppressTime;
    float maxRange;
    float fAnimHorRotateInc;
    float fPlayerPositionDist;
    const char* szUseHintString;
    const char* dropHintString;
    int iUseHintStringIndex;
    int dropHintStringIndex;
    float horizViewJitter;
    float vertViewJitter;
    const char* szScript;
    float fOOPosAnimLength[2];
    int minDamage;
    int minPlayerDamage;
    float fMaxDamageRange;
    float fMinDamageRange;
    float destabilizationRateTime;
    float destabilizationCurvatureMax;
    int destabilizeDistance;
    float locationDamageMultipliers[19];
    const char* fireRumble;
    const char* meleeImpactRumble;
    float adsDofStart;
    float adsDofEnd;
};
static_assert(sizeof(WeaponDef) == 2168);

struct SndDriverGlobals // sizeof=0x4
{                                       // ...
    const char* name;
};

struct RawFile // sizeof=0xC
{                                       // ...
    const char* name;
    int len;
    const char* buffer;
};
static_assert(sizeof(RawFile) == 12);

struct PhysPreset // sizeof=0x2C
{                                       // ...
    const char *name;                   // ...
    int type;                           // ...
    float mass;                         // ...
    float bounce;                       // ...
    float friction;                     // ...
    float bulletForceScale;             // ...
    float explosiveForceScale;          // ...
    const char *sndAliasPrefix;         // ...
    float piecesSpreadFraction;
    float piecesUpwardVelocity;
    bool tempDefaultToCylinder;
    // padding byte
    // padding byte
    // padding byte
};


extern "C" {
    // win32
    struct _OVERLAPPED;
}

union XAssetHeader // sizeof=0x4
{                                       // ...
    XAssetHeader() { data = NULL; }
    XAssetHeader(void *arg) { data = arg; }

    struct XModelPieces *xmodelPieces;
    struct PhysPreset *physPreset;
    struct XAnimParts *parts;
    struct XModel *model;
    struct Material *material;
    struct MaterialPixelShader *pixelShader;
    struct MaterialVertexShader *vertexShader;
    struct MaterialTechniqueSet *techniqueSet;
    struct GfxImage *image;
    struct snd_alias_list_t *sound;
    struct SndCurve *sndCurve;
    struct LoadedSound *loadSnd;
    struct clipMap_t *clipMap;
    struct ComWorld *comWorld;
    struct GameWorldSp *gameWorldSp;
    struct GameWorldMp *gameWorldMp;
    struct MapEnts *mapEnts;
    struct GfxWorld *gfxWorld;
    struct GfxLightDef *lightDef;
    struct Font_s *font;
    struct MenuList *menuList;
    struct menuDef_t *menu;
    struct LocalizeEntry *localize;
    struct WeaponDef *weapon;
    struct SndDriverGlobals *sndDriverGlobals;
    const struct FxEffectDef *fx;
    struct FxImpactTable *impactFx;
    struct RawFile *rawfile;
    struct StringTable *stringTable;

    void *data;
};

enum XAssetType : __int32 // Accurate to SP/MP (Win32)
{
//#ifdef KISAK_MP 
    ASSET_TYPE_XMODELPIECES = 0x0,
    ASSET_TYPE_PHYSPRESET = 0x1,
    ASSET_TYPE_XANIMPARTS = 0x2,
    ASSET_TYPE_XMODEL = 0x3,
    ASSET_TYPE_MATERIAL = 0x4,
    // PIXELSHADER?
    ASSET_TYPE_TECHNIQUE_SET = 0x5,
    ASSET_TYPE_IMAGE = 0x6,
    ASSET_TYPE_SOUND = 0x7,
    ASSET_TYPE_SOUND_CURVE = 0x8,
    ASSET_TYPE_LOADED_SOUND = 0x9,
    ASSET_TYPE_CLIPMAP = 0xA,
    ASSET_TYPE_CLIPMAP_PVS = 0xB,
    ASSET_TYPE_COMWORLD = 0xC,
    ASSET_TYPE_GAMEWORLD_SP = 0xD,
    ASSET_TYPE_GAMEWORLD_MP = 0xE,
    ASSET_TYPE_MAP_ENTS = 0xF,
    ASSET_TYPE_GFXWORLD = 0x10,
    ASSET_TYPE_LIGHT_DEF = 0x11,
    ASSET_TYPE_UI_MAP = 0x12,
    ASSET_TYPE_FONT = 0x13,
    ASSET_TYPE_MENULIST = 0x14,
    ASSET_TYPE_MENU = 0x15,
    ASSET_TYPE_LOCALIZE_ENTRY = 0x16,
    ASSET_TYPE_WEAPON = 0x17,
    ASSET_TYPE_SNDDRIVER_GLOBALS = 0x18,
    ASSET_TYPE_FX = 0x19,
    ASSET_TYPE_IMPACT_FX = 0x1A,
    ASSET_TYPE_AITYPE = 0x1B,
    ASSET_TYPE_MPTYPE = 0x1C,
    ASSET_TYPE_CHARACTER = 0x1D,
    ASSET_TYPE_XMODELALIAS = 0x1E,
    ASSET_TYPE_RAWFILE = 0x1F,
    ASSET_TYPE_STRINGTABLE = 0x20,
    ASSET_TYPE_COUNT = 0x21,
    ASSET_TYPE_STRING = 0x21,
    ASSET_TYPE_ASSETLIST = 0x22,

//#elif KISAK_SP // LWSS: this is accurate to xbox and not PC
//    ASSET_TYPE_XMODELPIECES = 0x0,
//    ASSET_TYPE_PHYSPRESET = 0x1,
//    ASSET_TYPE_XANIMPARTS = 0x2,
//    ASSET_TYPE_XMODEL = 0x3,
//    ASSET_TYPE_MATERIAL = 0x4,
//    ASSET_TYPE_PIXELSHADER = 0x5,
//    ASSET_TYPE_TECHNIQUE_SET = 0x6,
//    ASSET_TYPE_IMAGE = 0x7,
//    ASSET_TYPE_SOUND = 0x8,
//    ASSET_TYPE_SOUND_CURVE = 0x9,
//    ASSET_TYPE_LOADED_SOUND = 0xA,
//    ASSET_TYPE_CLIPMAP = 0xB,
//    ASSET_TYPE_CLIPMAP_PVS = 0xC,
//    ASSET_TYPE_COMWORLD = 0xD,
//    ASSET_TYPE_GAMEWORLD_SP = 0xE,
//    ASSET_TYPE_GAMEWORLD_MP = 0xF,
//    ASSET_TYPE_MAP_ENTS = 0x10,
//    ASSET_TYPE_GFXWORLD = 0x11,
//    ASSET_TYPE_LIGHT_DEF = 0x12,
//    ASSET_TYPE_UI_MAP = 0x13,
//    ASSET_TYPE_FONT = 0x14,
//    ASSET_TYPE_MENULIST = 0x15,
//    ASSET_TYPE_MENU = 0x16,
//    ASSET_TYPE_LOCALIZE_ENTRY = 0x17,
//    ASSET_TYPE_WEAPON = 0x18,
//    ASSET_TYPE_SNDDRIVER_GLOBALS = 0x19,
//    ASSET_TYPE_FX = 0x1A,
//    ASSET_TYPE_IMPACT_FX = 0x1B,
//    ASSET_TYPE_AITYPE = 0x1C,
//    ASSET_TYPE_MPTYPE = 0x1D,
//    ASSET_TYPE_CHARACTER = 0x1E,
//    ASSET_TYPE_XMODELALIAS = 0x1F,
//    ASSET_TYPE_RAWFILE = 0x20,
//    ASSET_TYPE_STRINGTABLE = 0x21,
//    ASSET_TYPE_COUNT = 0x22,
//    ASSET_TYPE_STRING = 0x22,
//    ASSET_TYPE_ASSETLIST = 0x23,
//#endif
};
inline XAssetType &operator++(XAssetType &e) {
    e = static_cast<XAssetType>(static_cast<int>(e) + 1);
    return e;
}
inline XAssetType &operator++(XAssetType &e, int i)
{
    ++e;
    return e;
}

struct XAsset // sizeof=0x8
{                                       // ...
    XAssetType type;                    // ...
    XAssetHeader header;                // ...
};
static_assert(sizeof(XAsset) == 8);

union XAssetSize // sizeof=0x878
{                                       // ...
    XAssetSize()
    {
        fx = NULL;
    }
    XAnimParts parts;
    XModel model;
    Material material;
    MaterialPixelShader pixelShader;
    MaterialVertexShader vertexShader;
    MaterialTechniqueSet techniqueSet;
    GfxImage image;
    snd_alias_list_t sound;
    SndCurve sndCurve;
    clipMap_t clipMap;
    ComWorld comWorld;
    MapEnts mapEnts;
    GfxWorld gfxWorld;
    GfxLightDef lightDef;
    Font_s font;
    MenuList menuList;
    menuDef_t menu;
    LocalizeEntry localize;
    WeaponDef weapon;
    SndDriverGlobals sndDriverGlobals;
    const FxEffectDef *fx;
    FxImpactTable impactFx;
    RawFile rawfile;
    StringTable stringTable;
};

template <typename T>
union XAssetPoolEntry // sizeof=0x10
{                                       // ...
    XAssetPoolEntry()
    {
        next = NULL;
    }
    T entry;
    XAssetPoolEntry<T> *next;
};

template <typename T, int LEN>
struct XAssetPool
{
    XAssetPoolEntry<T> *freeHead;
    XAssetPoolEntry<T> entries[LEN];
};

struct XAssetEntry // sizeof=0x10
{                                       // ...
    XAsset asset;                       // ...
    unsigned __int8 zoneIndex;
    bool inuse;
    uint16_t nextHash;
    uint16_t nextOverride;
    uint16_t usageFrame;
};

union XAssetEntryPoolEntry // sizeof=0x10
{                                       // ...
    XAssetEntryPoolEntry()
    {
    }
    XAssetEntry entry;
    XAssetEntryPoolEntry *next;
};

struct XZoneInfo // sizeof=0xC
{                                       // ...
    const char *name;                   // ...
    int allocFlags;                     // ...
    int freeFlags;                      // ...
};

struct XBlock // sizeof=0x8
{                                       // ...
    unsigned __int8 *data;
    uint32_t size;
};

struct XZoneMemory // sizeof=0x58
{                                       // ...
    XBlock blocks[9];
    unsigned __int8 *lockedVertexData;
    unsigned __int8 *lockedIndexData;
    void *vertexBuffer;
    void *indexBuffer;
};

struct XZone // sizeof=0xA8
{                                       // ...
    char name[64];                      // ...
    int flags;                          // ...
    int allocType;
    XZoneMemory mem;                    // ...
    int fileSize;                       // ...
    bool modZone;                       // ...
    // padding byte
    // padding byte
    // padding byte
};

struct ScriptStringList // sizeof=0x8
{                                       // ...
    int count;
    const char **strings;
};
static_assert(sizeof(ScriptStringList) == 8);

struct XAssetList // sizeof=0x10
{                                       // ...
    ScriptStringList stringList;
    int assetCount;
    XAsset *assets;
};
static_assert(sizeof(XAssetList) == 16);

struct XFile // sizeof=0x2C
{                                       // ...
    uint32_t size;
    uint32_t externalSize;          // ...
    uint32_t blockSize[9];          // ...
};
static_assert(sizeof(XFile) == 44);

struct XSurfaceCollisionAabb // sizeof=0xC
{                                       // ...
    uint16_t mins[3];
    uint16_t maxs[3];
};

struct XSurfaceCollisionNode // sizeof=0x10
{
    XSurfaceCollisionAabb aabb;
    uint16_t childBeginIndex;
    uint16_t childCount;
};

struct XSurfaceCollisionLeaf // sizeof=0x2
{
    uint16_t triangleBeginIndex;
};

struct XSurfaceCollisionTree // sizeof=0x28
{
    float trans[3];
    float scale[3];
    uint32_t nodeCount;
    XSurfaceCollisionNode *nodes;
    uint32_t leafCount;
    XSurfaceCollisionLeaf *leafs;
};
struct XRigidVertList // sizeof=0xC
{                                       // ...
    uint16_t boneOffset;        // ...
    uint16_t vertCount;         // ...
    uint16_t triOffset;         // ...
    uint16_t triCount;          // ...
    XSurfaceCollisionTree *collisionTree;
};
static_assert(sizeof(XRigidVertList) == 12);

struct XSurfaceVertexInfo // sizeof=0xC
{                                       // ...
    __int16 vertCount[4];
    uint16_t *vertsBlend;
};
static_assert(sizeof(XSurfaceVertexInfo) == 12);

struct XSurface // sizeof=0x38
{
    unsigned __int8 tileMode;
    bool deformed;
    uint16_t vertCount;
    uint16_t triCount;
    unsigned __int8 zoneHandle;
    // padding byte
    uint16_t baseTriIndex;
    uint16_t baseVertIndex;
    uint16_t *triIndices;
    XSurfaceVertexInfo vertInfo;
    GfxPackedVertex *verts0;
    uint32_t vertListCount;
    XRigidVertList *vertList;
    int partBits[4];
};
static_assert(sizeof(XSurface) == 56);

struct DObj_s;

struct gentity_s;

int __cdecl XAnimGetTreeHighMemUsage();
int __cdecl XAnimGetTreeMemUsage();
void __cdecl TRACK_xanim();
int __cdecl XAnimGetTreeMaxMemUsage();
XAnimInfo *XAnimAllocInfo(DObj_s *obj, uint32_t animIndex, int after);
void __cdecl XAnimInit();
void __cdecl XAnimShutdown();
XAnimParts* __cdecl XAnimFindData_LoadObj(const char* name);
XAnimParts* __cdecl XAnimFindData_FastFile(const char* name);
void __cdecl XAnimCreate(XAnim_s* anims, uint32_t animIndex, const char* name);
XAnimParts *__cdecl XAnimPrecache(const char *name, void *(__cdecl *Alloc)(int));
void __cdecl XAnimBlend(
    XAnim_s* anims,
    uint32_t animIndex,
    const char* name,
    uint32_t children,
    uint32_t num,
    uint32_t flags);
bool __cdecl IsNodeAdditive(const XAnimEntry* node);
bool __cdecl IsLeafNode(const XAnimEntry* anim);
XAnim_s* __cdecl XAnimCreateAnims(const char* debugName, uint32_t size, void* (__cdecl* Alloc)(int));
void __cdecl XAnimFreeList(XAnim_s* anims);
void __cdecl XAnimFree(XAnimParts *parts);
XAnimTree_s* __cdecl XAnimCreateTree(XAnim_s* anims, void* (__cdecl* Alloc)(int));
void __cdecl XAnimFreeTree(XAnimTree_s* tree, void(__cdecl* Free)(void*, int));
void XAnimCheckTreeLeak();
int XAnimGetAssetType(XAnimTree_s *tree, uint32_t index);
XAnim_s* __cdecl XAnimGetAnims(const XAnimTree_s* tree);
bool XAnimIsLeafNode(const XAnim_s *anims, uint32_t animIndex);
void XAnimResetAnimMap(const DObj_s* obj, uint32_t infoIndex);
void __cdecl XAnimInitModelMap(XModel* const* models, uint32_t numModels, XModelNameMap* modelMap);
void __cdecl XAnimResetAnimMap_r(XModelNameMap* modelMap, uint32_t infoIndex);
void __cdecl XAnimResetAnimMapLeaf(const XModelNameMap* modelMap, uint32_t infoIndex);
uint32_t __cdecl XAnimGetAnimMap(const XAnimParts* parts, const XModelNameMap* modelMap);
double __cdecl XAnimGetLength(const XAnim_s* anims, uint32_t animIndex);
int __cdecl XAnimGetLengthMsec(const XAnim_s* anims, uint32_t anim);
double __cdecl XAnimGetTime(const XAnimTree_s* tree, uint32_t animIndex);
uint32_t __cdecl XAnimGetInfoIndex(const XAnimTree_s* tree, uint32_t animIndex);
uint32_t __cdecl XAnimGetInfoIndex_r(const XAnimTree_s* tree, uint32_t animIndex, uint32_t infoIndex);
double __cdecl XAnimGetWeight(const XAnimTree_s* tree, uint32_t animIndex);
bool __cdecl XAnimHasFinished(const XAnimTree_s* tree, uint32_t animIndex);
int __cdecl XAnimGetNumChildren(const XAnim_s* anims, uint32_t animIndex);
uint32_t __cdecl XAnimGetChildAt(const XAnim_s* anims, uint32_t animIndex, uint32_t childIndex);
const char* __cdecl XAnimGetAnimName(const XAnim_s* anims, uint32_t animIndex);
char* __cdecl XAnimGetAnimDebugName(const XAnim_s* anims, uint32_t animIndex);
const char* __cdecl XAnimGetAnimTreeDebugName(const XAnim_s* anims);
uint32_t __cdecl XAnimGetAnimTreeSize(const XAnim_s* anims);
void __cdecl XAnimInitInfo(XAnimInfo* info);
void __cdecl XAnimUpdateOldTime(
    DObj_s* obj,
    uint32_t infoIndex,
    XAnimState* syncState,
    float dtime,
    bool parentHasWeight,
    bool* childHasTimeForParent);
uint32_t __cdecl XAnimInitTime(XAnimTree_s* tree, uint32_t infoIndex, float goalTime);
void __cdecl XAnimResetTime(uint32_t infoIndex);
void __cdecl XAnimResetTimeInternal(uint32_t infoIndex);
uint32_t __cdecl XAnimCloneInitTime(XAnimTree_s* tree, uint32_t infoIndex, uint32_t parentIndex);
void __cdecl DObjInitServerTime(DObj_s* obj, float dtime);
void __cdecl DObjUpdateClientInfo(DObj_s* obj, float dtime, bool notify);
void __cdecl XAnimUpdateTimeAndNotetrack(const DObj_s* obj, uint32_t infoIndex, float dtime, bool bNotify);
void __cdecl XAnimCheckFreeInfo(XAnimTree_s* tree, uint32_t infoIndex, int hasWeight);
void __cdecl XAnimFreeInfo(XAnimTree_s* tree, uint32_t infoIndex);
void __cdecl XAnimClearServerNotify(XAnimInfo* info);
double __cdecl XAnimGetAverageRateFrequency(const XAnimTree_s *tree, uint32_t infoIndex);
void __cdecl XAnimUpdateTimeAndNotetrackLeaf(
    const DObj_s* obj,
    const XAnimParts* parts,
    uint32_t infoIndex,
    float dtime,
    bool bNotify);
void __cdecl XAnimProcessClientNotify(XAnimInfo* info, float dtime);
uint16_t __cdecl XAnimGetNextNotifyIndex(const XAnimParts* parts, float time);
double __cdecl XAnimGetNotifyFracLeaf(const XAnimState* state, const XAnimState* nextState, float time, float dtime);
void __cdecl XAnimAddClientNotify(uint32_t notetrackName, float frac, uint32_t notifyType);
void __cdecl XAnimUpdateTimeAndNotetrackSyncSubTree(
    const DObj_s* obj,
    uint32_t infoIndex,
    float dtime,
    bool bNotify);
void __cdecl XAnimUpdateInfoSync(
    const DObj_s* obj,
    uint32_t infoIndex,
    bool bNotify,
    XAnimState* syncState,
    float dtime);
void __cdecl XAnimProcessServerNotify(const DObj_s* obj, XAnimInfo* info, float time);
XAnimParts* __cdecl XAnimGetParts(const XAnimTree_s* tree, XAnimInfo* info);
void __cdecl NotifyServerNotetrack(const DObj_s* obj, uint32_t notifyName, uint32_t notetrackName);
int __cdecl DObjUpdateServerInfo(DObj_s* obj, float dtime, int bNotify);
double __cdecl XAnimFindServerNoteTrack(const DObj_s* obj, uint32_t infoIndex, float dtime);
double __cdecl XAnimFindServerNoteTrackLeafNode(const DObj_s* obj, XAnimInfo* info, float dtime);
double __cdecl XAnimGetNextServerNotifyFrac(
    const DObj_s* obj,
    XAnimInfo* info,
    const XAnimState* syncState,
    const XAnimState* nextSyncState,
    float dtime);
double __cdecl XAnimFindServerNoteTrackSyncSubTree(const DObj_s* obj, XAnimInfo* info, float dtime);
double __cdecl XAnimGetServerNotifyFracSyncTotal(
    const DObj_s* obj,
    XAnimInfo* info,
    const XAnimState* syncState,
    const XAnimState* nextSyncState,
    float dtime);
int __cdecl DObjGetClientNotifyList(XAnimNotify_s** notifyList);
void __cdecl DObjDisplayAnimToBuffer(const DObj_s* obj, const char* header, char* buffer, int bufferSize);
void __cdecl XAnimDisplay(
    const XAnimTree_s *tree,
    uint32_t infoIndex,
    int depth,
    char *buffer,
    int bufferSize,
    int *bufferPos);
void __cdecl DObjDisplayAnim(const DObj_s* obj, const char* header);
void __cdecl XAnimCalcDelta(DObj_s* obj, uint32_t animIndex, float* rot, float* trans, bool bUseGoalWeight);
void __cdecl XAnimCalcDeltaTree(
    const DObj_s* obj,
    uint32_t infoIndex,
    float weightScale,
    XAnimDeltaInfo deltaInfo,
    XAnimSimpleRotPos* rotPos);
void __cdecl XAnimCalcRelDeltaParts(
    const XAnimParts* parts,
    float weightScale,
    float time1,
    float time2,
    XAnimSimpleRotPos* rotPos,
    int quatIndex);
void __cdecl TransformToQuatRefFrame(const float* rot, float* trans);
void __cdecl XAnimCalcAbsDeltaParts(const XAnimParts* parts, float weightScale, float time, XAnimSimpleRotPos* rotPos);
void __cdecl XAnimCalcAbsDelta(DObj_s* obj, uint32_t animIndex, float* rot, float* trans);
void __cdecl XAnimGetRelDelta(
    const XAnim_s* anims,
    uint32_t animIndex,
    float* rot,
    float* trans,
    float time1,
    float time2);
void __cdecl XAnimGetAbsDelta(const XAnim_s* anims, uint32_t animIndex, float* rot, float* trans, float time);
uint32_t __cdecl XAnimAllocInfoWithParent(
    XAnimTree_s* tree,
    uint16_t animToModel,
    uint32_t animIndex,
    uint32_t parentInfoIndex,
    int after);
uint32_t XAnimAllocInfoIndex(DObj_s *obj, uint32_t animIndex, int after);
uint32_t __cdecl XAnimEnsureGoalWeightParent(DObj_s* obj, uint32_t animIndex);
void __cdecl XAnimClearGoalWeightInternal(
    XAnimTree_s* tree,
    uint32_t infoIndex,
    float blendTime,
    int forceBlendTime);
void __cdecl XAnimClearTreeGoalWeightsInternal(
    XAnimTree_s* tree,
    uint32_t infoIndex,
    float blendTime,
    int forceBlendTime);
void __cdecl XAnimClearTreeGoalWeights(XAnimTree_s* tree, uint32_t animIndex, float blendTime);
void __cdecl XAnimClearTreeGoalWeightsStrict(XAnimTree_s* tree, uint32_t animIndex, float blendTime);
void __cdecl XAnimClearGoalWeightKnobInternal(
    XAnimTree_s* tree,
    uint32_t infoIndex,
    float goalWeight,
    float goalTime);
int __cdecl XAnimSetCompleteGoalWeightNode(
    XAnimTree_s* tree,
    uint32_t infoIndex,
    float goalWeight,
    float goalTime,
    float rate,
    uint32_t notifyName,
    uint32_t notifyType);
int XAnimSetCompleteGoalWeightKnobAll(
    DObj_s *obj,
    uint32_t animIndex,
    uint32_t rootIndex,
    float goalWeight,
    float goalTime,
    float rate,
    int notifyName,
    int notifyType,
    int bRestart);
int __cdecl XAnimSetGoalWeightKnobAll(
    DObj_s* obj,
    uint32_t animIndex,
    uint32_t rootIndex,
    float goalWeight,
    float goalTime,
    float rate,
    uint32_t notifyName,
    uint32_t notifyType,
    int bRestart);
int XAnimSetCompleteGoalWeightKnob(
    DObj_s *obj,
    uint32_t animIndex,
    double goalWeight,
    double goalTime,
    double rate,
    uint32_t notifyName,
    uint32_t notifyType,
    int bRestart);
int __cdecl XAnimSetGoalWeightKnob(
    DObj_s* obj,
    uint32_t animIndex,
    float goalWeight,
    float goalTime,
    float rate,
    uint32_t notifyName,
    uint32_t notifyType,
    int bRestart);
void __cdecl XAnimClearTree(XAnimTree_s* tree);
int __cdecl XAnimSetGoalWeightNode(
    XAnimTree_s* tree,
    uint32_t infoIndex,
    float goalWeight,
    float goalTime,
    float rate,
    uint32_t notifyName,
    uint32_t notifyType);
uint32_t __cdecl XAnimGetDescendantWithGreatestWeight(const XAnimTree_s* tree, uint32_t infoIndex);
void __cdecl XAnimSetupSyncNodes(XAnim_s* anims);
void __cdecl XAnimSetupSyncNodes_r(XAnim_s* anims, uint32_t animIndex);
void __cdecl XAnimFillInSyncNodes_r(XAnim_s* anims, uint32_t animIndex, bool bLoop);
bool __cdecl XAnimHasTime(const XAnim_s* anims, uint32_t animIndex);
BOOL __cdecl XAnimIsPrimitive(XAnim_s* anims, uint32_t animIndex);
void __cdecl XAnimSetTime(XAnimTree_s *tree, uint32_t animIndex, float time);
void __cdecl XAnimUpdateServerNotifyIndex(XAnimInfo* info, const XAnimParts* parts);
uint32_t __cdecl XAnimRestart(XAnimTree_s* tree, uint32_t infoIndex, float goalTime);
int __cdecl XAnimSetGoalWeight(
    DObj_s* obj,
    uint32_t animIndex,
    float goalWeight,
    float goalTime,
    float rate,
    uint32_t notifyName,
    uint32_t notifyType,
    int bRestart);
void __cdecl XAnimSetAnimRate(XAnimTree_s* tree, uint32_t animIndex, float rate);
bool __cdecl XAnimIsLooped(const XAnim_s* anims, uint32_t animIndex);
char __cdecl XAnimNotetrackExists(const XAnim_s* anims, uint32_t animIndex, uint32_t name);
void __cdecl XAnimAddNotetrackTimesToScriptArray(const XAnim_s* anims, uint32_t animIndex, uint32_t name);
int __cdecl XAnimSetCompleteGoalWeight(
    DObj_s* obj,
    uint32_t animIndex,
    float goalWeight,
    float goalTime,
    float rate,
    uint32_t notifyName,
    uint32_t notifyType,
    int bRestart);
void __cdecl XAnimCloneAnimInfo(const XAnimInfo* from, XAnimInfo* to);
void __cdecl XAnimCloneAnimTree(const XAnimTree_s* from, XAnimTree_s* to);
void __cdecl XAnimCloneAnimTree_r(
    const XAnimTree_s* from,
    XAnimTree_s* to,
    uint32_t fromInfoIndex,
    uint32_t toInfoParentIndex);
XAnimInfo* __cdecl GetAnimInfo(int infoIndex);
void XAnimDisableLeakCheck();
void XAnimFreeAnims(XAnim_s *anims, void(*Free)(void *, int));
void XAnimCloneClientAnimTree(const XAnimTree_s *from, XAnimTree_s *to);
void DObjTransfer(const DObj_s *fromObj, DObj_s *toObj, double dtime);


// xanim_load_obj
XModelPieces *__cdecl XModelPiecesPrecache(const char *name, void *(__cdecl *Alloc)(int));
XAnimParts *__cdecl XAnimLoadFile(char *name, void *(__cdecl *Alloc)(int));


// KISAK HACK: These are for gfx_d3d/r_material.h
struct TechniqueSetList
{
    MaterialTechniqueSet *hashTable[1024];
    int count;
};
void __cdecl R_GetMaterialList(XAssetHeader header, char *data);
void __cdecl Material_CollateTechniqueSets(XAssetHeader header, TechniqueSetList *techSetList);
void __cdecl Material_ReleaseTechniqueSet(XAssetHeader header, void *crap);