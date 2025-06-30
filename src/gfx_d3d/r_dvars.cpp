#include "r_dvars.h"

#include <qcommon/threads.h>
#include <qcommon/qcommon.h>
#include "r_material.h"
#include "r_sky.h"

 //const dvar_t **prof_probe;
 const dvar_t *prof_probe[5];
 //const dvar_t **r_smp_worker_thread;
 const dvar_t *r_smp_worker_thread[2];
 const dvar_t *r_envMapMaxIntensity;
 const dvar_t *r_portalMinRecurseDepth;
 const dvar_t *r_dof_nearEnd;
 const dvar_t *r_streamSize;
 const dvar_t *sm_showOverlay;
 const dvar_t *r_streamMaxDist;
 const dvar_t *r_desaturation;
 const dvar_t *sm_sunEnable;
 const dvar_t *r_lightTweakDiffuseFraction;
 const dvar_t *prof_probeMaxMsec;
 const dvar_t *sm_spotEnable;
 const dvar_t *r_norefresh;
 const dvar_t *r_lightTweakSunLight;
 const dvar_t *r_showCollisionDepthTest;
 const dvar_t *r_specular;
 const dvar_t *r_glow_allowed;
 const dvar_t *sc_offscreenCasterLodBias;
 const dvar_t *sm_spotShadowFadeTime;
 const dvar_t *r_showCollisionGroups;
 const dvar_t *r_dof_farStart;
 const dvar_t *r_outdoorAwayBias;
 const dvar_t *sc_length;
 const dvar_t *r_zfar;
 const dvar_t *sc_shadowInRate;
 const dvar_t *r_drawDynEnts;
 const dvar_t *r_gamma;
 const dvar_t *r_lowestLodDist;
 const dvar_t *r_specularColorScale;
 const dvar_t *r_staticModelDumpLodInfo;
 const dvar_t *profile_script;
 const dvar_t *profile_script_by_file;
 const dvar_t *r_showAabbTrees;
 const dvar_t *sm_maxLights;
 const dvar_t *r_showTriCounts;
 const dvar_t *sc_debugReceiverCount;
 const dvar_t *r_cacheSModelLighting;
 const dvar_t *r_drawEntities;
 const dvar_t *r_distortion;
 const dvar_t *r_filmUseTweaks;
 const dvar_t *r_drawBModels;
 const dvar_t *r_drawXModels;
 const dvar_t *r_vsync;
 const dvar_t *r_singleCell;
 const dvar_t *r_glowTweakBloomDesaturation;
 const dvar_t *sm_polygonOffsetScale;
 const dvar_t *r_ignore;
 const dvar_t *r_glowUseTweaks;
 const dvar_t *r_showCollisionPolyType;
 const dvar_t *r_polygonOffsetScale;
 const dvar_t *r_znear_depthhack;
 const dvar_t *r_stream;
 const dvar_t *r_drawSun;
 const dvar_t *r_portalWalkLimit;
 const dvar_t *r_contrast;
 const dvar_t *sm_strictCull;
 const dvar_t *r_smp_backend;
 const dvar_t *r_polygonOffsetBias;
 const dvar_t *r_spotLightBrightness;
 const dvar_t *r_spotLightSModelShadows;
 const dvar_t *prof_sortTime;
 const dvar_t *r_lowLodDist;
 const dvar_t *r_lodBiasSkinned;
 const dvar_t *r_portalBevels;
 const dvar_t *r_brightness;
 const dvar_t *r_drawWorld;
 const dvar_t *r_drawPrimHistogram;
 const dvar_t *r_debugLineWidth;
 const dvar_t *r_smp_worker;
 const dvar_t *r_showTess;
 const dvar_t *r_debugShader;
 const dvar_t *r_dof_viewModelStart;
 const dvar_t *r_dof_viewModelEnd;
 const dvar_t *r_matTwkPreviewSize;
 const dvar_t *r_outdoorFeather;
 const dvar_t *r_diffuseColorScale;
 const dvar_t *r_texFilterAnisoMax;
 const dvar_t *r_drawPrimCap;
 const dvar_t *r_showCullXModels;
 const dvar_t *r_optimizeLightmaps;
 const dvar_t *r_spotLightFovInnerFraction;
 const dvar_t *r_streamDebug;
 const dvar_t *r_spotLightShadows;
 const dvar_t *r_streamTaint;
 const dvar_t *r_fullbright;
 const dvar_t *r_vc_makelog;
 const dvar_t *r_lightTweakAmbientColor;
 const dvar_t *r_showPortals;
 const dvar_t *r_showTris;
 const dvar_t *r_filmTweakContrast;
 const dvar_t *r_filmTweakDesaturation;
 const dvar_t *r_texFilterAnisoMin;
 const dvar_t *r_cacheModelLighting;
 const dvar_t *r_clear;
 const dvar_t *r_pix_material;
 const dvar_t *r_ignoreHwGamma;
 const dvar_t *r_dlightLimit;
 const dvar_t *r_specularMap;
 const dvar_t *r_pixelShaderGPRReallocation;
 const dvar_t *r_filmTweakInvert;
 const dvar_t *r_drawPoly;
 const dvar_t *r_streamFakeLagMsec;
 const dvar_t *r_lockPvs;
 const dvar_t *r_detail;
 const dvar_t *r_lightMap;
 const dvar_t *sm_fastSunShadow;
 const dvar_t *r_envMapSpecular;
 const dvar_t *sc_wantCount;
 const dvar_t *r_optimize;
 const dvar_t *r_glow;
 const dvar_t *r_glowTweakBloomCutoff;
 const dvar_t *r_mediumLodDist;
 const dvar_t *r_lightTweakSunDirection;
 const dvar_t *r_filmTweakLightTint;
 const dvar_t *sc_wantCountMargin;
 const dvar_t *r_showCollision;
 const dvar_t *r_spotLightStartRadius;
 const dvar_t *sc_showOverlay;
 const dvar_t *r_filmTweakEnable;
 const dvar_t *r_texFilterMipMode;
 const dvar_t *r_glowTweakBloomIntensity;
 const dvar_t *sm_lightScore_spotProjectFrac;
 const dvar_t *sm_lightScore_eyeProjectDist;
 const dvar_t *r_texFilterMipBias;
 const dvar_t *sc_offscreenCasterLodScale;
 const dvar_t *r_showSModelNames;
 const dvar_t *sm_enable;
 const dvar_t *r_lightTweakAmbient;
 const dvar_t *r_streamClear;
 const dvar_t *r_showCullBModels;
 const dvar_t *r_lodScaleSkinned;
 const dvar_t *r_dof_enable;
 const dvar_t *r_lodBiasRigid;
 const dvar_t *r_showFbColorDebug;
 const dvar_t *r_showMissingLightGrid;
 const dvar_t *r_drawSModels;
 const dvar_t *r_drawPrimFloor;
 const dvar_t *r_smooth_vsync;
 const dvar_t *sm_sunSampleSizeNear;
 const dvar_t *r_lightTweakSunDiffuseColor;
 const dvar_t *sc_fadeRange;
 const dvar_t *sm_showOverlayDepthBounds;
 const dvar_t *r_showSurfCounts;
 const dvar_t *r_spotLightEntityShadows;
 const dvar_t *r_lightTweakSunColor;
 const dvar_t *r_clearColor2;
 const dvar_t *r_filmTweakDarkTint;
 const dvar_t *r_scaleViewport;
 const dvar_t *r_normal;
 const dvar_t *sm_sunShadowCenter;
 const dvar_t *r_glowTweakEnable;
 const dvar_t *r_envMapExponent;
 const dvar_t *r_dof_nearBlur;
 const dvar_t *r_portalMinClipArea;
 const dvar_t *r_skipPvs;
 const dvar_t *r_dof_bias;
 const dvar_t *sc_showDebug;
 const dvar_t *r_drawWater;
 const dvar_t *r_clearColor;
 const dvar_t *r_lodScaleRigid;
 const dvar_t *r_colorMap;
 const dvar_t *r_blur;
 const dvar_t *sc_count;
 const dvar_t *r_dof_tweak;
 const dvar_t *r_resampleScene;
 const dvar_t *sm_debugFastSunShadow;
 const dvar_t *r_showPixelCost;
 const dvar_t *r_dof_nearStart;
 const dvar_t *r_envMapOverride;
 const dvar_t *r_showFloatZDebug;
 const dvar_t *r_sun_from_dvars;
 const dvar_t *r_streamShowStats;
 const dvar_t *sc_enable;
 const dvar_t *r_xdebug;
 const dvar_t *r_vc_showlog;
 const dvar_t *r_filmTweakBrightness;
 const dvar_t *sc_shadowOutRate;
 const dvar_t *r_showCollisionDist;
 const dvar_t *developer;
 const dvar_t *r_envMapSunIntensity;
 const dvar_t *r_highLodDist;
 const dvar_t *r_forceLod;
 const dvar_t *r_logFile;
 const dvar_t *r_normalMap;
 const dvar_t *r_outdoorDownBias;
 const dvar_t *r_texFilterDisable;
 const dvar_t *profile_mode;
 const dvar_t *sm_qualitySpotShadow;
 const dvar_t *r_streamCheckAabb;
 const dvar_t *r_showVertCounts;
 const dvar_t *r_portalBevelsOnly;
 const dvar_t *r_showCullSModels;
 const dvar_t *r_skipDrawTris;
 const dvar_t *sc_debugCasterCount;
 const dvar_t *r_dof_farEnd;
 const dvar_t *r_znear;
 const dvar_t *r_showLightGrid;
 const dvar_t *r_spotLightEndRadius;
 const dvar_t *r_dof_farBlur;
 const dvar_t *r_streamShowVolumes;
 const dvar_t *r_glowTweakRadius;
 const dvar_t *sm_sunShadowScale;
 const dvar_t *r_streamShowList;
 const dvar_t *r_fog;
 const dvar_t *r_envMapMinIntensity;
 const dvar_t *sm_polygonOffsetBias;
 const dvar_t *sc_blur;

 const dvar_t *r_reflectionProbeGenerate;
 const dvar_t *r_reflectionProbeRegenerateAll;
 const dvar_t *r_reflectionProbeGenerateExit;
 //const dvar_t *r_warningRepeatDelay;

 const dvar_t *r_gpuSync;
 const dvar_t *r_multiGpu;
 const dvar_t *r_skinCache;
 const dvar_t *r_fastSkin;
 const dvar_t *r_smc_enable;
 const dvar_t *r_pretess;
 const dvar_t *r_picmip_manual;
 const dvar_t *r_picmip;
 const dvar_t *r_picmip_bump;
 const dvar_t *r_picmip_spec;
 const dvar_t *r_picmip_water;
 const dvar_t *r_useLayeredMaterials;
 const dvar_t *r_loadForRenderer;
 const dvar_t *r_aaAlpha;
 const dvar_t *r_aaSamples;
 const dvar_t *r_drawDecals;
 const dvar_t *r_floatz;
 const dvar_t *r_zFeather;
 const dvar_t *r_depthPrepass;
 const dvar_t *r_modelVertColor;
 const dvar_t *r_glow_allowed_script_forced;
 const dvar_t *r_outdoor;
 const dvar_t *r_sse_skinning;
 const dvar_t *r_monitor;
 const dvar_t *r_rendererPreference;
 const dvar_t *r_rendererInUse;
 const dvar_t *r_aspectRatio;
 const dvar_t *r_customMode;
 const dvar_t *r_altModelLightingUpdate;
 const dvar_t *r_preloadShaders;

 const char *g_profile_mode_values[2] =
 {
     "default",
     NULL
 };

 const char *prof_enumNames[433]{ ":)" }; // KISAKTODO

 const char *s_aspectRatioNames[5] =
 {
     "auto",
     "standard",
     "wide 16:10",
     "wide 16:9",
     NULL
 };
 const char *s_rendererNames[4] =
 {
     "Shader Model 2.0",
     "Shader Model 3.0",
     "Default",
     NULL
 };
 const char *sm_showOverlayNames[4] =
 {
     "off",
     "sun",
     "spot",
     NULL
 };
 const char *r_showTessNames[7] =
 {
     "off",
     "tech",
     "techset",
     "material",
     "vertexshader",
     "pixelshader",
     NULL
 };
 const char *r_forceLodNames[6] =
 {
     "high",
     "medium",
     "low",
     "lowest",
     "none",
     NULL
 };
 const char *showCollisionNames[11] =
 {
     "None",
     "All",
     "Player",
     "Bullet",
     "Missile",
     "Vehicle",
     "Monster",
     "Item",
     "Canshoot",
     "Ai Nosight",
     NULL
 };
 const char *showCollisionGroupsNames[4] =
 {
     "All",
     "Brush",
     "Terrain",
     NULL
 };
 const char *showCollisionPolyTypeNames[4] =
 {
     "All",
     "Wireframe",
     "Interior",
     NULL
 };

 const char *fbColorDebugNames[4] =
 {
     "None",
     "Screen",
     "Feedback",
     NULL
 };

 const char *s_aaAlphaNames[4] =
 {
     "off",
     "dither (fast)",
     "supersample (nice)",
     NULL
 };
 const char *r_clearNames[6] =
 {
     "never",
     "dev-only blink",
     "blink",
     "steady",
     "fog color",
     NULL
 };
 const char *mipFilterNames[5] =
 {
     "Unchanged",
     "Force Trilinear",
     "Force Bilinear",
     "Force MipMaps Off",
     NULL
 };

 const char *xdebugNames[5] =
 {
     "",
     "boxes",
     "axes",
     "both",
     NULL
 };

 const char *showPixelCostNames[5] =
 {
     "off",
     "timing",
     "use depth",
     "ignore depth",
     NULL
 };

 const char *colorMapNames[5] =
 {
     "Black",
     "Unchanged",
     "White",
     "Gray",
     NULL
 };

 const char *normalMapNames[3] =
 {
     "Flat",
     "Unchanged",
     NULL
 };

 const char *debugShaderNames[6] =
 {
     "none",
     "normal",
     "basisTangent",
     "basisBinormal",
     "basisNormal",
     NULL
 };

 const char *gpuSyncNames[4] =
 {
     "off",
     "adaptive",
     "aggressive",
     NULL
 };

 void __cdecl R_ReflectionProbeRegisterDvars()
 {
     r_reflectionProbeGenerate = Dvar_RegisterBool(
         "r_reflectionProbeGenerate",
         false,
         DVAR_NOFLAG,
         "Generate cube maps for reflection probes.");
     r_reflectionProbeRegenerateAll = Dvar_RegisterBool(
         "r_reflectionProbeRegenerateAll",
         false,
         DVAR_NOFLAG,
         "Regenerate cube maps for all reflection probes.");
     r_reflectionProbeGenerateExit = Dvar_RegisterBool(
         "r_reflectionProbeGenerateExit",
         false,
         DVAR_NOFLAG,
         "Exit when done generating reflection cubes.");
     if (r_reflectionProbeGenerate->current.enabled)
         Material_PreventOverrideTechniqueGeneration();
 }

 static void __cdecl R_WarnInitDvars()
 {
     DvarLimits min; // [esp+4h] [ebp-10h]

     min.value.max = 30.0;
     min.value.min = 0.0;
     r_warningRepeatDelay = Dvar_RegisterFloat(
         "r_warningRepeatDelay",
         5.0,
         min,
         DVAR_NOFLAG,
         "Number of seconds after displaying a \"per-frame\" warning before it will display again");
 }

 void __cdecl R_RegisterDvars()
 {
     DvarLimits min; // [esp+Ch] [ebp-10h]
     DvarLimits mina; // [esp+Ch] [ebp-10h]
     DvarLimits minb; // [esp+Ch] [ebp-10h]
     DvarLimits minc; // [esp+Ch] [ebp-10h]
     DvarLimits mind; // [esp+Ch] [ebp-10h]
     DvarLimits mine; // [esp+Ch] [ebp-10h]
     DvarLimits minf; // [esp+Ch] [ebp-10h]
     DvarLimits ming; // [esp+Ch] [ebp-10h]
     DvarLimits minh; // [esp+Ch] [ebp-10h]
     DvarLimits mini; // [esp+Ch] [ebp-10h]
     DvarLimits minj; // [esp+Ch] [ebp-10h]
     DvarLimits mink; // [esp+Ch] [ebp-10h]
     DvarLimits minl; // [esp+Ch] [ebp-10h]
     DvarLimits minm; // [esp+Ch] [ebp-10h]
     DvarLimits minn; // [esp+Ch] [ebp-10h]
     DvarLimits mino; // [esp+Ch] [ebp-10h]
     DvarLimits minp; // [esp+Ch] [ebp-10h]
     DvarLimits minq; // [esp+Ch] [ebp-10h]
     DvarLimits minr; // [esp+Ch] [ebp-10h]
     DvarLimits mins; // [esp+Ch] [ebp-10h]
     DvarLimits mint; // [esp+Ch] [ebp-10h]
     DvarLimits minu; // [esp+Ch] [ebp-10h]
     DvarLimits minv; // [esp+Ch] [ebp-10h]
     DvarLimits minw; // [esp+Ch] [ebp-10h]
     DvarLimits minx; // [esp+Ch] [ebp-10h]
     DvarLimits miny; // [esp+Ch] [ebp-10h]
     DvarLimits minz; // [esp+Ch] [ebp-10h]
     DvarLimits minba; // [esp+Ch] [ebp-10h]
     DvarLimits minbb; // [esp+Ch] [ebp-10h]
     DvarLimits minbc; // [esp+Ch] [ebp-10h]
     DvarLimits minbd; // [esp+Ch] [ebp-10h]
     DvarLimits minbe; // [esp+Ch] [ebp-10h]
     DvarLimits minbf; // [esp+Ch] [ebp-10h]
     DvarLimits minbg; // [esp+Ch] [ebp-10h]
     DvarLimits minbh; // [esp+Ch] [ebp-10h]
     DvarLimits minbi; // [esp+Ch] [ebp-10h]
     DvarLimits minbj; // [esp+Ch] [ebp-10h]
     DvarLimits minbk; // [esp+Ch] [ebp-10h]
     DvarLimits minbl; // [esp+Ch] [ebp-10h]
     DvarLimits minbm; // [esp+Ch] [ebp-10h]
     DvarLimits minbn; // [esp+Ch] [ebp-10h]
     DvarLimits minbo; // [esp+Ch] [ebp-10h]
     DvarLimits minbp; // [esp+Ch] [ebp-10h]
     DvarLimits minbq; // [esp+Ch] [ebp-10h]
     DvarLimits minbr; // [esp+Ch] [ebp-10h]
     DvarLimits minbs; // [esp+Ch] [ebp-10h]
     DvarLimits minbt; // [esp+Ch] [ebp-10h]
     DvarLimits minbu; // [esp+Ch] [ebp-10h]
     DvarLimits minbv; // [esp+Ch] [ebp-10h]
     DvarLimits minbw; // [esp+Ch] [ebp-10h]
     DvarLimits minbx; // [esp+Ch] [ebp-10h]
     DvarLimits minby; // [esp+Ch] [ebp-10h]
     DvarLimits minbz; // [esp+Ch] [ebp-10h]
     DvarLimits minca; // [esp+Ch] [ebp-10h]
     DvarLimits mincb; // [esp+Ch] [ebp-10h]
     DvarLimits mincc; // [esp+Ch] [ebp-10h]
     DvarLimits mincd; // [esp+Ch] [ebp-10h]
     DvarLimits mince; // [esp+Ch] [ebp-10h]
     DvarLimits mincf; // [esp+Ch] [ebp-10h]
     DvarLimits mincg; // [esp+Ch] [ebp-10h]
     DvarLimits minch; // [esp+Ch] [ebp-10h]
     DvarLimits minci; // [esp+Ch] [ebp-10h]
     DvarLimits mincj; // [esp+Ch] [ebp-10h]
     DvarLimits minck; // [esp+Ch] [ebp-10h]
     DvarLimits mincl; // [esp+Ch] [ebp-10h]
     DvarLimits mincm; // [esp+Ch] [ebp-10h]
     DvarLimits mincn; // [esp+Ch] [ebp-10h]
     DvarLimits minco; // [esp+Ch] [ebp-10h]
     DvarLimits mincp; // [esp+Ch] [ebp-10h]
     DvarLimits mincq; // [esp+Ch] [ebp-10h]
     DvarLimits mincr; // [esp+Ch] [ebp-10h]
     DvarLimits mincs; // [esp+Ch] [ebp-10h]
     DvarLimits minct; // [esp+Ch] [ebp-10h]
     DvarLimits mincu; // [esp+Ch] [ebp-10h]
     DvarLimits mincv; // [esp+Ch] [ebp-10h]

     R_RegisterSunDvars();
     r_ignore = Dvar_RegisterInt("r_ignore", 0, (DvarLimits)0x7FFFFFFF80000000LL, DVAR_NOFLAG, "used for debugging anything");
     vid_xpos = Dvar_RegisterInt("vid_xpos", 3, (DvarLimits)0x1000FFFFF000LL, DVAR_ARCHIVE, "Game window horizontal position");
     vid_ypos = Dvar_RegisterInt("vid_ypos", 22, (DvarLimits)0x1000FFFFF000LL, DVAR_ARCHIVE, "game window vertical position");
     r_fullscreen = Dvar_RegisterBool("r_fullscreen", 0, DVAR_LATCH | DVAR_ARCHIVE, "Display game full screen");
     min.value.max = 3.0;
     min.value.min = 0.5;
     r_gamma = Dvar_RegisterFloat("r_gamma", 0.80000001f, min, DVAR_ARCHIVE, "Gamma value");
     r_ignoreHwGamma = Dvar_RegisterBool("r_ignorehwgamma", 0, DVAR_LATCH | DVAR_ARCHIVE, "Ignore hardware gamma");
     r_texFilterAnisoMax = Dvar_RegisterInt(
         "r_texFilterAnisoMax",
         16,
         (DvarLimits)0x1000000001LL,
         DVAR_ARCHIVE,
         "Maximum anisotropy to use for texture filtering");
     r_texFilterDisable = Dvar_RegisterBool(
         "r_texFilterDisable",
         0,
         DVAR_CHEAT,
         "Disables all texture filtering (uses nearest only.)");
     r_texFilterAnisoMin = Dvar_RegisterInt(
         "r_texFilterAnisoMin",
         1,
         (DvarLimits)0x1000000001LL,
         DVAR_ARCHIVE,
         "Minimum anisotropy to use for texture filtering (overridden by max)");
     r_texFilterMipMode = Dvar_RegisterEnum(
         "r_texFilterMipMode",
         mipFilterNames,
         0,
         DVAR_ARCHIVE,
         "Forces all mipmaps to use a particular blend between levels (or disables mipping.)");
     mina.value.max = 15.99f;
     mina.value.min = -16.0f;
     r_texFilterMipBias = Dvar_RegisterFloat("r_texFilterMipBias", 0.0f, mina, DVAR_CHEAT, "Change the mipmap bias");
     r_fullbright = Dvar_RegisterBool("r_fullbright", 0, DVAR_CHEAT, "Toggles rendering without lighting");
     r_debugShader = Dvar_RegisterEnum("r_debugShader", debugShaderNames, 0, DVAR_CHEAT, "Enable shader debugging information");
     r_gpuSync = Dvar_RegisterEnum(
         "r_gpuSync",
         gpuSyncNames,
         1,
         DVAR_NOFLAG,
         "GPU synchronization type (used to improve mouse responsiveness)");
     r_multiGpu = Dvar_RegisterBool("r_multiGpu", false, DVAR_ARCHIVE, "Use multiple GPUs");
     r_skinCache = Dvar_RegisterBool("r_skinCache", true, DVAR_NOFLAG, "Enable cache for vertices of animated models");
     r_fastSkin = Dvar_RegisterBool("r_fastSkin", false, DVAR_ARCHIVE, "Enable fast model skinning");
     r_smc_enable = Dvar_RegisterBool("r_smc_enable", true, DVAR_NOFLAG, "Enable static model cache");
     r_pretess = Dvar_RegisterBool("r_pretess", true, DVAR_NOFLAG, "Batch surfaces to reduce primitive count");
     minb.value.max = FLT_MAX;
     minb.value.min = 0.0f;
     r_lodScaleRigid = Dvar_RegisterFloat(
         "r_lodScaleRigid",
         1.0f,
         minb,
         DVAR_ARCHIVE,
         "Scale the level of detail distance for rigid models (larger reduces detail)");
     minc.value.max = FLT_MAX;
     minc.value.min = -FLT_MAX;
     r_lodBiasRigid = Dvar_RegisterFloat(
         "r_lodBiasRigid",
         0.0f,
         minc,
         DVAR_ARCHIVE,
         "Bias the level of detail distance for rigid models (negative increases detail)");
     mind.value.max = FLT_MAX;
     mind.value.min = 0.0f;
     r_lodScaleSkinned = Dvar_RegisterFloat(
         "r_lodScaleSkinned",
         1.0f,
         mind,
         DVAR_ARCHIVE,
         "Scale the level of detail distance for skinned models (larger reduces detail)");
     mine.value.max = FLT_MAX;
     mine.value.min = -FLT_MAX;
     r_lodBiasSkinned = Dvar_RegisterFloat(
         "r_lodBiasSkinned",
         0.0f,
         mine,
         DVAR_ARCHIVE,
         "Bias the level of detail distance for skinned models (negative increases detail)");
     minf.value.max = 10000.0f;
     minf.value.min = 0.001f;
     r_znear = Dvar_RegisterFloat(
         "r_znear",
         4.0f,
         minf,
         DVAR_CHEAT,
         "Things closer than this aren't drawn.  Reducing this increases z-fighting in the distance.");
     ming.value.max = 16.0f;
     ming.value.min = 0.001f;
     r_znear_depthhack = Dvar_RegisterFloat("r_znear_depthhack", 0.1f, ming, DVAR_CHEAT, "Viewmodel near clip plane");
     minh.value.max = FLT_MAX;
     minh.value.min = 0.0f;
     r_zfar = Dvar_RegisterFloat(
         "r_zfar",
         0.0f,
         minh,
         DVAR_CHEAT,
         "Change the distance at which culling fog reaches 100% opacity; 0 is off");
     r_fog = Dvar_RegisterBool("r_fog", 1, DVAR_CHEAT, "Set to 0 to disable fog");
     mini.value.max = 0.0f;
     mini.value.min = -4.0f;
     r_polygonOffsetScale = Dvar_RegisterFloat(
         "r_polygonOffsetScale",
         -1.0f,
         mini,
         DVAR_ARCHIVE,
         "Offset scale for decal polygons; bigger values z-fight less but poke through walls more");
     minj.value.max = 0.0f;
     minj.value.min = -16.0f;
     r_polygonOffsetBias = Dvar_RegisterFloat(
         "r_polygonOffsetBias",
         -1.0f,
         minj,
         DVAR_ARCHIVE,
         "Offset bias for decal polygons; bigger values z-fight less but poke through walls more");
     r_picmip_manual = Dvar_RegisterBool(
         "r_picmip_manual",
         0,
         DVAR_ARCHIVE,
         "If 0, picmip is set automatically.  If 1, picmip is set based on the other r_picmip dvars.");
     r_picmip = Dvar_RegisterInt(
         "r_picmip",
         0,
         (DvarLimits)0x300000000LL,
         DVAR_ARCHIVE,
         "Picmip level of color maps.  If r_picmip_manual is 0, this is read-only.");
     r_picmip_bump = Dvar_RegisterInt(
         "r_picmip_bump",
         0,
         (DvarLimits)0x300000000LL,
         DVAR_ARCHIVE,
         "Picmip level of normal maps.  If r_picmip_manual is 0, this is read-only.");
     r_picmip_spec = Dvar_RegisterInt(
         "r_picmip_spec",
         0,
         (DvarLimits)0x300000000LL,
         DVAR_ARCHIVE,
         "Picmip level of specular maps.  If r_picmip_manual is 0, this is read-only.");
     r_picmip_water = Dvar_RegisterInt(
         "r_picmip_water",
         0,
         (DvarLimits)0x100000000LL,
         DVAR_LATCH | DVAR_ARCHIVE,
         "Picmip level of water maps.");
     r_detail = Dvar_RegisterBool("r_detail", true, DVAR_NOFLAG, "Allows shaders to use detail textures");
     r_normal = Dvar_RegisterBool("r_normal", true, DVAR_NOFLAG, "Allows shaders to use normal maps");
     r_specular = Dvar_RegisterBool("r_specular", true, DVAR_NOFLAG, "Allows shaders to use phong specular lighting");
     r_envMapSpecular = Dvar_RegisterBool("r_envMapSpecular", true, DVAR_NOFLAG, "Enables environment map specular lighting");
     r_lightMap = Dvar_RegisterEnum(
         "r_lightMap",
         colorMapNames,
         1,
         DVAR_CHEAT,
         "Replace all lightmaps with pure black or pure white");
     r_colorMap = Dvar_RegisterEnum(
         "r_colorMap",
         colorMapNames,
         1,
         DVAR_CHEAT,
         "Replace all color maps with pure black or pure white");
     r_normalMap = Dvar_RegisterEnum(
         "r_normalMap",
         normalMapNames,
         1,
         DVAR_CHEAT,
         "Replace all normal maps with a flat normal map");
     r_specularMap = Dvar_RegisterEnum(
         "r_specularMap",
         colorMapNames,
         1,
         DVAR_CHEAT,
         "Replace all specular maps with pure black (off) or pure white (super shiny)");
     mink.value.max = 100.0f;
     mink.value.min = 0.0f;
     r_specularColorScale = Dvar_RegisterFloat(
         "r_specularColorScale",
         1.0f,
         mink,
         DVAR_SAVED | DVAR_CHEAT,
         "Set greater than 1 to brighten specular highlights");
     minl.value.max = 100.0f;
     minl.value.min = 0.0f;
     r_diffuseColorScale = Dvar_RegisterFloat(
         "r_diffuseColorScale",
         1.0f,
         minl,
         DVAR_SAVED | DVAR_CHEAT,
         "Globally scale the diffuse color of all point lights");
     r_useLayeredMaterials = Dvar_RegisterBool(
         "r_useLayeredMaterials",
         0,
         DVAR_LATCH,
         "Set to true to use layered materials on shader model 3 hardware");
     r_loadForRenderer = Dvar_RegisterBool(
         "r_loadForRenderer",
         1,
         DVAR_LATCH,
         "Set to false to disable dx allocations (for dedicated server mode)");
     r_showTris = Dvar_RegisterInt("r_showTris", 0, (DvarLimits)0x200000000LL, DVAR_CHEAT, "Show triangle outlines");
     r_showTriCounts = Dvar_RegisterBool("r_showTriCounts", 0, DVAR_CHEAT, "Triangle count for each rendered entity");
     r_showSurfCounts = Dvar_RegisterBool("r_showSurfCounts", 0, DVAR_CHEAT, "Surface count for each rendered entity");
     r_showVertCounts = Dvar_RegisterBool("r_showVertCounts", 0, DVAR_CHEAT, "Vertex count for each entity");
     r_resampleScene = Dvar_RegisterBool(
         "r_resampleScene",
         1,
         DVAR_CHEAT,
         "Upscale the frame buffer with sharpen filter and color correction.");
     r_showPixelCost = Dvar_RegisterEnum(
         "r_showPixelCost",
         showPixelCostNames,
         0,
         DVAR_CHEAT,
         "Shows how expensive it is to draw every pixel on the screen");
     r_xdebug = Dvar_RegisterEnum("r_xdebug", xdebugNames, 0, DVAR_CHEAT, "xmodel/xanim debug rendering");
     minm.value.max = 16.0f;
     minm.value.min = 0.0f;
     r_debugLineWidth = Dvar_RegisterFloat("r_debugLineWidth", 1.0, minm, DVAR_ARCHIVE, "Width of server side debug lines");
     r_vc_makelog = Dvar_RegisterInt(
         "r_vc_makelog",
         0,
         (DvarLimits)0x200000000LL,
         DVAR_LATCH,
         "Enable logging of light grid points for the vis cache.  1 starts from scratch, 2 appends.");
     r_vc_showlog = Dvar_RegisterInt(
         "r_vc_showlog",
         0,
         (DvarLimits)0x40000000000LL,
         DVAR_NOFLAG,
         "Show this many rows of light grid points for the vis cache");
     r_showLightGrid = Dvar_RegisterBool("r_showLightGrid", 0, DVAR_CHEAT, "Show light grid debugging information");
     r_showMissingLightGrid = Dvar_RegisterBool(
         "r_showMissingLightGrid",
         true,
         DVAR_NOFLAG,
         "Use rainbow colors for entities that are outside the light grid");
     r_cacheSModelLighting = Dvar_RegisterBool(
         "r_cacheSModelLighting",
         true,
         DVAR_NOFLAG,
         "Speed up static model lighting by caching previous results");
     r_cacheModelLighting = Dvar_RegisterBool(
         "r_cacheModelLighting",
         true,
         DVAR_NOFLAG,
         "Speed up model lighting by caching previous results");
     minn.value.max = 4.0f;
     minn.value.min = 0.0f;
     r_lightTweakAmbient = Dvar_RegisterFloat("r_lightTweakAmbient", 0.1f, minn, DVAR_ROM | DVAR_AUTOEXEC, "Ambient light strength");
     mino.value.max = 1.0f;
     mino.value.min = 0.0f;
     r_lightTweakDiffuseFraction = Dvar_RegisterFloat(
         "r_lightTweakDiffuseFraction",
         0.5f,
         mino,
         DVAR_ROM | DVAR_AUTOEXEC,
         "diffuse light fraction");
     minp.value.max = 4.0f;
     minp.value.min = 0.0f;
     r_lightTweakSunLight = Dvar_RegisterFloat("r_lightTweakSunLight", 1.0f, minp, DVAR_CHEAT | DVAR_AUTOEXEC, "Sunlight strength");
     r_lightTweakAmbientColor = Dvar_RegisterColor(
         "r_lightTweakAmbientColor",
         1.0f,
         0.0f,
         0.0f,
         1.0f,
         DVAR_ROM | DVAR_AUTOEXEC,
         "Light ambient color");
     r_lightTweakSunColor = Dvar_RegisterColor("r_lightTweakSunColor", 0.0f, 1.0f, 0.0f, 1.0f, DVAR_CHEAT | DVAR_AUTOEXEC, "Sun color");
     r_lightTweakSunDiffuseColor = Dvar_RegisterColor(
         "r_lightTweakSunDiffuseColor",
         0.0f,
         0.0f,
         1.0f,
         1.0f,
         DVAR_ROM | DVAR_AUTOEXEC,
         "Sun diffuse color");
     minq.value.max = 360.0f;
     minq.value.min = -360.0f;
     r_lightTweakSunDirection = Dvar_RegisterVec3(
         "r_lightTweakSunDirection",
         0.0f,
         0.0f,
         0.0f,
         minq,
         DVAR_CHEAT | DVAR_AUTOEXEC | DVAR_SAVED,
         "Sun direction in degrees");
     r_envMapOverride = Dvar_RegisterBool("r_envMapOverride", false, DVAR_NOFLAG, "Min reflection intensity based on glancing angle.");
     minr.value.max = 2.0f;
     minr.value.min = 0.0f;
     r_envMapMinIntensity = Dvar_RegisterFloat(
         "r_envMapMinIntensity",
         0.2f,
         minr,
         DVAR_NOFLAG,
         "Min reflection intensity based on glancing angle.");
     mins.value.max = 2.0f;
     mins.value.min = 0.0099999998f;
     r_envMapMaxIntensity = Dvar_RegisterFloat(
         "r_envMapMaxIntensity",
         0.5f,
         mins,
         DVAR_NOFLAG,
         "Max reflection intensity based on glancing angle.");
     mint.value.max = 20.0f;
     mint.value.min = 0.050000001f;
     r_envMapExponent = Dvar_RegisterFloat("r_envMapExponent", 5.0f, mint, DVAR_NOFLAG, "Reflection exponent.");
     minu.value.max = 4.0f;
     minu.value.min = 0.0f;
     r_envMapSunIntensity = Dvar_RegisterFloat(
         "r_envMapSunIntensity",
         2.0f,
         minu,
         DVAR_NOFLAG,
         "Max sun specular intensity intensity with env map materials.");
     r_drawPrimHistogram = Dvar_RegisterBool(
         "r_drawPrimHistogram",
         0,
         DVAR_CHEAT,
         "Draws a histogram of the sizes of each primitive batch");
     r_logFile = Dvar_RegisterInt(
         "r_logFile",
         0,
         (DvarLimits)0x7FFFFFFF00000000LL,
         DVAR_NOFLAG,
         "Write all graphics hardware calls for this many frames to a logfile");
     r_norefresh = Dvar_RegisterBool("r_norefresh", 0, DVAR_CHEAT, "Skips all rendering.  Useful for benchmarking.");
     minv.value.max = 1.0f;
     minv.value.min = 0.0f;
     r_scaleViewport = Dvar_RegisterFloat(
         "r_scaleViewport",
         1.0f,
         minv,
         DVAR_CHEAT,
         "Scale 3D viewports by this fraction.  Use this to see if framerate is pixel shader bound.");
     r_smp_backend = Dvar_RegisterBool("r_smp_backend", true, DVAR_NOFLAG, "Process renderer back end in a separate thread");
     r_smp_worker = Dvar_RegisterBool("r_smp_worker", true, DVAR_NOFLAG, "Process renderer front end in a separate thread");
     r_smp_worker_thread[0] = R_RegisterWorkerThreadDvar("r_smp_worker_thread0", 0);
     r_smp_worker_thread[1] = R_RegisterWorkerThreadDvar("r_smp_worker_thread1", 1u);
     r_aaAlpha = Dvar_RegisterEnum("r_aaAlpha", s_aaAlphaNames, 1, DVAR_ARCHIVE, "Transparency anti-aliasing method");
     r_aaSamples = Dvar_RegisterInt(
         "r_aaSamples",
         1,
         (DvarLimits)0x1000000001LL,
         DVAR_ARCHIVE | DVAR_LATCH,
         "Anti-aliasing sample count; 1 disables anti-aliasing");
     r_vsync = Dvar_RegisterBool(
         "r_vsync",
         1,
         DVAR_ARCHIVE | DVAR_LATCH,
         "Enable v-sync before drawing the next frame to avoid 'tearing' artifacts.");
     r_clear = Dvar_RegisterEnum("r_clear", r_clearNames, 1, DVAR_NOFLAG, "Controls how the color buffer is cleared");
     r_clearColor = Dvar_RegisterColor(
         "r_clearColor",
         0.5f,
         0.75f,
         1.0f,
         1.0f,
         DVAR_NOFLAG,
         "Color to clear the screen to when clearing the frame buffer");
     r_clearColor2 = Dvar_RegisterColor(
         "r_clearColor2",
         1.0f,
         0.5f,
         0.0f,
         1.0f,
         DVAR_NOFLAG,
         "Color to clear every second frame to (for use during development)");
     r_drawSun = Dvar_RegisterBool("r_drawSun", true, DVAR_ARCHIVE, "Enable sun effects");
     r_drawDecals = Dvar_RegisterBool("r_drawDecals", true, DVAR_ARCHIVE, "Enable world decal rendering");
     r_drawWorld = Dvar_RegisterBool("r_drawWorld", true, DVAR_CHEAT, "Enable world rendering");
     r_drawEntities = Dvar_RegisterBool("r_drawEntities", true, DVAR_CHEAT, "Enable entity rendering");
     r_drawPoly = Dvar_RegisterBool("r_drawPoly", true, DVAR_CHEAT, "Enable poly rendering");
     r_drawDynEnts = Dvar_RegisterBool("r_drawDynEnts", true, DVAR_CHEAT, "Enable dynamic entity rendering");
     r_drawBModels = Dvar_RegisterBool("r_drawBModels", true, DVAR_CHEAT, "Enable brush model rendering");
     r_drawSModels = Dvar_RegisterBool("r_drawSModels", true, DVAR_CHEAT, "Enable static model rendering");
     r_drawXModels = Dvar_RegisterBool("r_drawXModels", true, DVAR_CHEAT, "Enable xmodel rendering");
     r_dlightLimit = Dvar_RegisterInt(
         "r_dlightLimit",
         4,
         (DvarLimits)0x400000000LL,
         DVAR_NOFLAG,
         "Maximum number of dynamic lights drawn simultaneously");
     minw.value.max = 0.99000001f;
     minw.value.min = 0.0f;
     r_spotLightFovInnerFraction = Dvar_RegisterFloat(
         "r_spotLightFovInnerFraction",
         0.69999999f,
         minw,
         DVAR_CHEAT,
         "Relative Inner FOV angle for the dynamic spot light. 0 is full fade 0.99 is almost no fade.");
     minx.value.max = 1200.0f;
     minx.value.min = 0.0f;
     r_spotLightStartRadius = Dvar_RegisterFloat(
         "r_spotLightStartRadius",
         36.0f,
         minx,
         DVAR_CHEAT | DVAR_SAVED,
         "Radius of the circle at the start of the spot light in inches.");
     miny.value.max = 1200.0f;
     miny.value.min = 1.0f;
     r_spotLightEndRadius = Dvar_RegisterFloat(
         "r_spotLightEndRadius",
         196.0f,
         miny,
         DVAR_CHEAT | DVAR_SAVED,
         "Radius of the circle at the end of the spot light in inches.");
     r_spotLightShadows = Dvar_RegisterBool("r_spotLightShadows", 1, DVAR_CHEAT, "Enable shadows for spot lights.");
     r_spotLightSModelShadows = Dvar_RegisterBool(
         "r_spotLightSModelShadows",
         1,
         DVAR_CHEAT,
         "Enable static model shadows for spot lights.");
     r_spotLightEntityShadows = Dvar_RegisterBool(
         "r_spotLightEntityShadows",
         1,
         DVAR_CHEAT,
         "Enable entity shadows for spot lights.");
     minz.value.max = 16.0f;
     minz.value.min = 0.0f;
     r_spotLightBrightness = Dvar_RegisterFloat(
         "r_spotLightBrightness",
         14.0f,
         minz,
         DVAR_CHEAT | DVAR_SAVED,
         "Brightness scale for spot light to get overbrightness from the 0-1 particle color range.");
     r_drawPrimCap = Dvar_RegisterInt(
         "r_drawPrimCap",
         0,
         (DvarLimits)0x2710FFFFFFFFLL,
         DVAR_CHEAT,
         "Only draw primitive batches with less than this many triangles");
     r_drawPrimFloor = Dvar_RegisterInt(
         "r_drawPrimFloor",
         0,
         (DvarLimits)0x271000000000LL,
         DVAR_CHEAT,
         "Only draw primitive batches with more than this many triangles");
     r_skipDrawTris = Dvar_RegisterBool("r_skipDrawTris", false, DVAR_CHEAT, "Skip drawing primitive tris.");
     r_drawWater = Dvar_RegisterBool("r_drawWater", true, DVAR_ARCHIVE, "Enable water animation");
     r_lockPvs = Dvar_RegisterBool(
         "r_lockPvs",
         false,
         DVAR_CHEAT,
         "Lock the viewpoint used for determining what is visible to the current position and direction");
     r_skipPvs = Dvar_RegisterBool(
         "r_skipPvs",
         false,
         DVAR_CHEAT,
         "Skipt the determination of what is in the potentially visible set (disables most drawing)");
     minba.value.max = 1.0f;
     minba.value.min = 0.0f;
     r_portalBevels = Dvar_RegisterFloat(
         "r_portalBevels",
         0.69999999f,
         minba,
         DVAR_ARCHIVE,
         "Helps cull geometry by angles of portals that are acute when projected onto the screen, value is th"
         "e cosine of the angle");
     r_portalBevelsOnly = Dvar_RegisterBool(
         "r_portalBevelsOnly",
         false,
         DVAR_NOFLAG,
         "Use screen-space bounding box of portals rather than the actual shape of the portal projected onto the screen");
     r_singleCell = Dvar_RegisterBool(
         "r_singleCell",
         false,
         DVAR_CHEAT,
         "Only draw things in the same cell as the camera.  Most useful for seeing how big the current cell is.");
     r_portalWalkLimit = Dvar_RegisterInt(
         "r_portalWalkLimit",
         0,
         (DvarLimits)0x6400000000LL,
         DVAR_CHEAT,
         "Stop portal recursion after this many iterations.  Useful for debugging portal errors.");
     minbb.value.max = 1.0f;
     minbb.value.min = 0.0f;
     r_portalMinClipArea = Dvar_RegisterFloat(
         "r_portalMinClipArea",
         0.02f,
         minbb,
         DVAR_NOFLAG,
         "Don't clip child portals by a parent portal smaller than this fraction of the screen area.");
     r_portalMinRecurseDepth = Dvar_RegisterInt(
         "r_portalMinRecurseDepth",
         2,
         (DvarLimits)0x6400000000LL,
         DVAR_CHEAT,
         "Ignore r_portalMinClipArea for portals with fewer than this many parent portals.");
     r_showPortals = Dvar_RegisterInt("r_showPortals", 0, (DvarLimits)0x300000000LL, DVAR_CHEAT, "Show portals for debugging");
     r_showAabbTrees = Dvar_RegisterInt(
         "r_showAabbTrees",
         0,
         (DvarLimits)0x200000000LL,
         DVAR_CHEAT,
         "Show axis-aligned bounding box trees used in potentially visibility set determination");
     r_showSModelNames = Dvar_RegisterBool("r_showSModelNames", false, DVAR_CHEAT, "Show static model names");
     r_showTess = Dvar_RegisterEnum("r_showTess", r_showTessNames, false, DVAR_CHEAT, "Show details for each surface");
     r_showCullBModels = Dvar_RegisterBool("r_showCullBModels", false, DVAR_CHEAT, "Show culled brush models");
     r_showCullSModels = Dvar_RegisterBool("r_showCullSModels", false, DVAR_CHEAT, "Show culled static models");
     r_showCullXModels = Dvar_RegisterBool("r_showCullXModels", false, DVAR_CHEAT, "Show culled xmodels");
     r_showFbColorDebug = Dvar_RegisterEnum(
         "r_showFbColorDebug",
         fbColorDebugNames,
         0,
         DVAR_CHEAT,
         "Show front buffer color debugging information");
     r_showFloatZDebug = Dvar_RegisterBool(
         "r_showFloatZDebug",
         false,
         DVAR_CHEAT,
         "Show float z buffer used to eliminate hard edges on particles near geometry");
     r_showCollision = Dvar_RegisterEnum(
         "r_showCollision",
         showCollisionNames,
         0,
         DVAR_CHEAT,
         "Show the collision surfaces for the selected mask types");
     r_showCollisionGroups = Dvar_RegisterEnum(
         "r_showCollisionGroups",
         showCollisionGroupsNames,
         0,
         DVAR_CHEAT,
         "Select whether to show the terrain, brush or all collision surface groups");
     r_showCollisionPolyType = Dvar_RegisterEnum(
         "r_showCollisionPolyType",
         showCollisionPolyTypeNames,
         0,
         DVAR_CHEAT,
         "Select whether to display the collision surfaces as wireframe, poly interiors, or both");
     r_showCollisionDepthTest = Dvar_RegisterBool(
         "r_showCollisionDepthTest",
         true,
         DVAR_CHEAT,
         "Select whether to use depth test in collision surfaces display");
     minbc.value.max = FLT_MAX;
     minbc.value.min = 1.0f;
     r_showCollisionDist = Dvar_RegisterFloat(
         "r_showCollisionDist",
         500.0f,
         minbc,
         DVAR_CHEAT,
         "Maximum distance to show collision surfaces");
     r_floatz = Dvar_RegisterBool(
         "r_floatz",
         1,
         DVAR_LATCH,
         "Allocate a float z buffer (required for effects such as floatz, dof, and laser light)");
     r_zFeather = Dvar_RegisterBool("r_zFeather", true, DVAR_ARCHIVE, "Enable z feathering (fixes particles clipping into geometry)");
     r_depthPrepass = Dvar_RegisterBool("r_depthPrepass", false, DVAR_ARCHIVE, "Enable depth prepass (usually improves performance)");
     minbd.value.max = FLT_MAX;
     minbd.value.min = -1.0f;
     r_highLodDist = Dvar_RegisterFloat("r_highLodDist", -1.0f, minbd, DVAR_CHEAT, "Distance for high level of detail");
     minbe.value.max = FLT_MAX;
     minbe.value.min = -1.0f;
     r_mediumLodDist = Dvar_RegisterFloat("r_mediumLodDist", -1.0f, minbe, DVAR_CHEAT, "Distance for medium level of detail");
     minbf.value.max = FLT_MAX;
     minbf.value.min = -1.0f;
     r_lowLodDist = Dvar_RegisterFloat("r_lowLodDist", -1.0f, minbf, DVAR_CHEAT, "Distance for low level of detail");
     minbg.value.max = FLT_MAX;
     minbg.value.min = -1.0f;
     r_lowestLodDist = Dvar_RegisterFloat("r_lowestLodDist", -1.0f, minbg, DVAR_CHEAT, "Distance for lowest level of detail");
     r_forceLod = Dvar_RegisterEnum("r_forceLod", r_forceLodNames, 4, DVAR_CHEAT, "Force all level of detail to this level");
     r_modelVertColor = Dvar_RegisterBool(
         "r_modelVertColor",
         1,
         0xA0u,
         "Set to 0 to replace all model vertex colors with white when loaded");
     sc_enable = Dvar_RegisterBool("sc_enable", false, DVAR_NOFLAG, "Enable shadow cookies");
     sc_blur = Dvar_RegisterInt("sc_blur", 2, (DvarLimits)0x400000000LL, DVAR_CHEAT, "Enable shadow cookie blur");
     sc_count = Dvar_RegisterInt("sc_count", 24, (DvarLimits)0x1800000000LL, DVAR_CHEAT, "Number of shadow cookies");
     sc_debugCasterCount = Dvar_RegisterInt(
         "sc_debugCasterCount",
         24,
         (DvarLimits)0x1800000000LL,
         DVAR_CHEAT,
         "Show debugging information for the shadow cookie caster count");
     sc_debugReceiverCount = Dvar_RegisterInt(
         "sc_debugReceiverCount",
         24,
         (DvarLimits)0x1800000000LL,
         DVAR_CHEAT,
         "Show debugging information for the shadow cookie receiver count");
     sc_showOverlay = Dvar_RegisterBool("sc_showOverlay", false, DVAR_CHEAT, "Show shadow overlay for shadow cookies");
     sc_showDebug = Dvar_RegisterBool("sc_showDebug", false, DVAR_CHEAT, "Show debug information for shadow cookies");
     sc_wantCount = Dvar_RegisterInt("sc_wantCount", 12, (DvarLimits)0x1800000000LL, DVAR_CHEAT, "Number of desired shadows");
     sc_wantCountMargin = Dvar_RegisterInt(
         "sc_wantCountMargin",
         1,
         (DvarLimits)0x1800000000LL,
         DVAR_CHEAT,
         "Margin of error on number of desired shadows");
     minbh.value.max = 1.0f;
     minbh.value.min = 0.0f;
     sc_fadeRange = Dvar_RegisterFloat("sc_fadeRange", 0.25, minbh, DVAR_CHEAT, "Shadow cookie fade range");
     minbi.value.max = 20.0f;
     minbi.value.min = 0.0f;
     sc_shadowInRate = Dvar_RegisterFloat(
         "sc_shadowInRate",
         2.0f,
         minbi,
         DVAR_CHEAT,
         "Rate at which the shadow cookie horizon moves inwards");
     minbj.value.max = 20.0f;
     minbj.value.min = 0.0f;
     sc_shadowOutRate = Dvar_RegisterFloat(
         "sc_shadowOutRate",
         5.0f,
         minbj,
         DVAR_CHEAT,
         "Rate at which the shadow cookie horizon moves outwards");
     minbk.value.max = 2000.0f;
     minbk.value.min = 1.0f;
     sc_length = Dvar_RegisterFloat("sc_length", 400.0f, minbk, DVAR_CHEAT, "Shadow cookie length");
     minbl.value.max = FLT_MAX;
     minbl.value.min = -FLT_MAX;
     sc_offscreenCasterLodBias = Dvar_RegisterFloat(
         "sc_offscreenCasterLodBias",
         0.0f,
         minbl,
         DVAR_CHEAT,
         "Shadow cookie off-screen caster level of detail bias");
     minbm.value.max = FLT_MAX;
     minbm.value.min = 0.0f;
     sc_offscreenCasterLodScale = Dvar_RegisterFloat(
         "sc_offscreenCasterLodScale",
         20.0f,
         minbm,
         DVAR_CHEAT,
         "Shadow cookie off-screen caster level of detail scale");
     sm_enable = Dvar_RegisterBool("sm_enable", 1, 1u, "Enable shadow mapping");
     sm_sunEnable = Dvar_RegisterBool("sm_sunEnable", 1, DVAR_SAVED | DVAR_CHEAT, "Enable sun shadow mapping from script");
     sm_spotEnable = Dvar_RegisterBool("sm_spotEnable", 1, DVAR_SAVED | DVAR_CHEAT, "Enable spot shadow mapping from script");
     sm_maxLights = Dvar_RegisterInt(
         "sm_maxLights",
         4,
         (DvarLimits)0x400000000LL,
         DVAR_ARCHIVE,
         "Limits how many primary lights can have shadow maps");
     minbn.value.max = 5.0f;
     minbn.value.min = 0.0099999998f;
     sm_spotShadowFadeTime = Dvar_RegisterFloat(
         "sm_spotShadowFadeTime",
         1.0f,
         minbn,
         DVAR_NOFLAG,
         "How many seconds it takes for a primary light shadow map to fade in or out");
     minbo.value.max = 1024.0f;
     minbo.value.min = 0.0f;
     sm_lightScore_eyeProjectDist = Dvar_RegisterFloat(
         "sm_lightScore_eyeProjectDist",
         64.0f,
         minbo,
         DVAR_NOFLAG,
         "When picking shadows for primary lights, measure distance from a point this far in fr"
         "ont of the camera.");
     minbp.value.max = 1.0f;
     minbp.value.min = 0.0f;
     sm_lightScore_spotProjectFrac = Dvar_RegisterFloat(
         "sm_lightScore_spotProjectFrac",
         0.125f,
         minbp,
         DVAR_NOFLAG,
         "When picking shadows for primary lights, measure distance to a point this fraction o"
         "f the light's radius along it's shadow direction.");
     sm_showOverlay = Dvar_RegisterEnum("sm_showOverlay", sm_showOverlayNames, 0, DVAR_CHEAT, "Show shadow map overlay");
     minbq.value.max = 1.0f;
     minbq.value.min = 0.0f;
     sm_showOverlayDepthBounds = Dvar_RegisterVec2(
         "sm_showOverlayDepthBounds",
         0.25f,
         0.75f,
         minbq,
         DVAR_CHEAT,
         "Near and far depth values for the shadow map overlay");
     minbr.value.max = 8.0f;
     minbr.value.min = 0.0f;
     sm_polygonOffsetScale = Dvar_RegisterFloat("sm_polygonOffsetScale", 2.0f, minbr, DVAR_NOFLAG, "Shadow map offset scale");
     minbs.value.max = 32.0f;
     minbs.value.min = 0.0f;
     sm_polygonOffsetBias = Dvar_RegisterFloat("sm_polygonOffsetBias", 0.5f, minbs, DVAR_NOFLAG, "Shadow map offset bias");
     minbt.value.max = 32.0f;
     minbt.value.min = 0.0625f;
     sm_sunSampleSizeNear = Dvar_RegisterFloat("sm_sunSampleSizeNear", 0.25f, minbt, DVAR_SAVED | DVAR_CHEAT, "Shadow sample size");
     minbu.value.max = FLT_MAX;
     minbu.value.min = -FLT_MAX;
     sm_sunShadowCenter = Dvar_RegisterVec3(
         "sm_sunShadowCenter",
         0.0f,
         0.0f,
         0.0f,
         minbu,
         DVAR_SAVED | DVAR_CHEAT,
         "Sun shadow center, 0 0 0 means don't override");
     minbv.value.max = 1.0f;
     minbv.value.min = 0.25f;
     sm_sunShadowScale = Dvar_RegisterFloat("sm_sunShadowScale", 1.0f, minbv, DVAR_SAVED | DVAR_CHEAT, "Sun shadow scale optimization");
     sm_strictCull = Dvar_RegisterBool("sm_strictCull", 1, DVAR_CHEAT, "Strict shadow map cull");
     sm_fastSunShadow = Dvar_RegisterBool("sm_fastSunShadow", 1, DVAR_CHEAT, "Fast sun shadow");
     sm_qualitySpotShadow = Dvar_RegisterBool("sm_qualitySpotShadow", 1, DVAR_CHEAT, "Fast spot shadow");
     sm_debugFastSunShadow = Dvar_RegisterBool("sm_debugFastSunShadow", 0, DVAR_CHEAT, "Debug fast sun shadow");
     minbw.value.max = 32.0f;
     minbw.value.min = 0.0f;
     r_blur = Dvar_RegisterFloat("r_blur", 0.0f, minbw, DVAR_CHEAT, "Dev tweak to blur the screen");
     r_distortion = Dvar_RegisterBool("r_distortion", true, DVAR_ARCHIVE, "Enable distortion");
     r_glow_allowed = Dvar_RegisterBool("r_glow_allowed", true, DVAR_ARCHIVE, "Allow glow.");
     r_glow_allowed_script_forced = Dvar_RegisterBool(
         "r_glow_allowed_script_forced",
         false,
         DVAR_SAVED,
         "Force 'allow glow' to be treated as true, by script.");
     r_glow = Dvar_RegisterBool("r_glow", 1, DVAR_CHEAT, "Enable glow.");
     r_glowUseTweaks = Dvar_RegisterBool("r_glowUseTweaks", 0, DVAR_CHEAT, "Overide glow with tweak dvar values.");
     r_glowTweakEnable = Dvar_RegisterBool("r_glowTweakEnable", 0, DVAR_CHEAT, "Tweak dev var; Enable glow");
     minbx.value.max = 32.0f;
     minbx.value.min = 0.0f;
     r_glowTweakRadius = Dvar_RegisterFloat(
         "r_glowTweakRadius0",
         5.0,
         minbx,
         DVAR_CHEAT,
         "Tweak dev var; Glow radius in pixels at 640x480");
     minby.value.max = 20.0f;
     minby.value.min = 0.0f;
     r_glowTweakBloomIntensity = Dvar_RegisterFloat(
         "r_glowTweakBloomIntensity0",
         1.0,
         minby,
         DVAR_CHEAT,
         "Tweak dev var; Glow bloom intensity");
     minbz.value.max = 1.0f;
     minbz.value.min = 0.0f;
     r_glowTweakBloomCutoff = Dvar_RegisterFloat(
         "r_glowTweakBloomCutoff",
         0.5,
         minbz,
         DVAR_CHEAT,
         "Tweak dev var; Glow bloom cut off fraction");
     minca.value.max = 1.0f;
     minca.value.min = 0.0f;
     r_glowTweakBloomDesaturation = Dvar_RegisterFloat(
         "r_glowTweakBloomDesaturation",
         0.0f,
         minca,
         DVAR_CHEAT,
         "Tweak dev var; Glow bloom desaturation");
     r_filmUseTweaks = Dvar_RegisterBool("r_filmUseTweaks", 0, DVAR_CHEAT, "Overide film effects with tweak dvar values.");
     r_filmTweakEnable = Dvar_RegisterBool("r_filmTweakEnable", 0, DVAR_SAVED, "Tweak dev var; enable film color effects");
     mincb.value.max = 4.0f;
     mincb.value.min = 0.0f;
     r_filmTweakContrast = Dvar_RegisterFloat(
         "r_filmTweakContrast",
         1.4f,
         mincb,
         DVAR_SAVED,
         "Tweak dev var; film color contrast");
     mincc.value.max = 4.0f;
     mincc.value.min = 0.0f;
     r_contrast = Dvar_RegisterFloat("r_contrast", 1.0f, mincc, DVAR_CHEAT, "Contrast adjustment");
     mincd.value.max = 1.0f;
     mincd.value.min = -1.0f;
     r_brightness = Dvar_RegisterFloat("r_brightness", 0.0f, mincd, DVAR_CHEAT, "Brightness adjustment");
     mince.value.max = 4.0f;
     mince.value.min = 0.0f;
     r_desaturation = Dvar_RegisterFloat("r_desaturation", 1.0f, mince, DVAR_CHEAT, "Desaturation adjustment");
     mincf.value.max = 1.0f;
     mincf.value.min = -1.0f;
     r_filmTweakBrightness = Dvar_RegisterFloat(
         "r_filmTweakBrightness",
         0.0f,
         mincf,
         DVAR_SAVED,
         "Tweak dev var; film color brightness");
     mincg.value.max = 1.0f;
     mincg.value.min = 0.0f;
     r_filmTweakDesaturation = Dvar_RegisterFloat(
         "r_filmTweakDesaturation",
         0.2f,
         mincg,
         DVAR_SAVED,
         "Tweak dev var; Desaturation applied after all 3D drawing");
     r_filmTweakInvert = Dvar_RegisterBool("r_filmTweakInvert", 0, DVAR_SAVED, "Tweak dev var; enable inverted video");
     minch.value.max = 2.0f;
     minch.value.min = 0.0f;
     r_filmTweakDarkTint = Dvar_RegisterVec3(
         "r_filmTweakDarkTint",
         0.69999999f,
         0.85000002f,
         1.0f,
         minch,
         DVAR_SAVED,
         "Tweak dev var; film color dark tint color");
     minci.value.max = 2.0f;
     minci.value.min = 0.0f;
     r_filmTweakLightTint = Dvar_RegisterVec3(
         "r_filmTweakLightTint",
         1.1f,
         1.05f,
         0.85000002f,
         minci,
         DVAR_SAVED,
         "Tweak dev var; film color light tint color");
     r_dof_enable = Dvar_RegisterBool("r_dof_enable", true, DVAR_ARCHIVE, "Enable the depth of field effect");
     r_dof_tweak = Dvar_RegisterBool(
         "r_dof_tweak",
         false,
         DVAR_CHEAT,
         "Use dvars to set the depth of field effect; overrides r_dof_enable");
     mincj.value.max = 10.0f;
     mincj.value.min = 4.0f;
     r_dof_nearBlur = Dvar_RegisterFloat(
         "r_dof_nearBlur",
         6.0f,
         mincj,
         DVAR_CHEAT,
         "Sets the radius of the gaussian blur used by depth of field, in pixels at 640x480");
     minck.value.max = 10.0f;
     minck.value.min = 0.0f;
     r_dof_farBlur = Dvar_RegisterFloat(
         "r_dof_farBlur",
         1.8f,
         minck,
         DVAR_CHEAT,
         "Sets the radius of the gaussian blur used by depth of field, in pixels at 640x480");
     mincl.value.max = 128.0f;
     mincl.value.min = 0.0f;
     r_dof_viewModelStart = Dvar_RegisterFloat(
         "r_dof_viewModelStart",
         2.0f,
         mincl,
         DVAR_CHEAT,
         "Depth of field viewmodel start distance, in inches");
     mincm.value.max = 128.0f;
     mincm.value.min = 0.0f;
     r_dof_viewModelEnd = Dvar_RegisterFloat(
         "r_dof_viewModelEnd",
         8.0f,
         mincm,
         DVAR_CHEAT,
         "Depth of field viewmodel end distance, in inches");
     mincn.value.max = 1000.0f;
     mincn.value.min = 0.0f;
     r_dof_nearStart = Dvar_RegisterFloat(
         "r_dof_nearStart",
         10.0f,
         mincn,
         DVAR_CHEAT,
         "Depth of field near start distance, in inches");
     minco.value.max = 1000.0f;
     minco.value.min = 0.0f;
     r_dof_nearEnd = Dvar_RegisterFloat("r_dof_nearEnd", 60.0, minco, DVAR_CHEAT, "Depth of field near end distance, in inches");
     mincp.value.max = 20000.0f;
     mincp.value.min = 0.0f;
     r_dof_farStart = Dvar_RegisterFloat(
         "r_dof_farStart",
         1000.0f,
         mincp,
         DVAR_CHEAT,
         "Depth of field far start distance, in inches");
     mincq.value.max = 20000.0f;
     mincq.value.min = 0.0f;
     r_dof_farEnd = Dvar_RegisterFloat("r_dof_farEnd", 7000.0, mincq, DVAR_CHEAT, "Depth of field far end distance, in inches");
     mincr.value.max = 3.0f;
     mincr.value.min = 0.1f;
     r_dof_bias = Dvar_RegisterFloat(
         "r_dof_bias",
         0.5f,
         mincr,
         DVAR_CHEAT,
         "Depth of field bias as a power function (like gamma); less than 1 is sharper");
     r_outdoor = Dvar_RegisterBool("r_outdoor", 1, DVAR_SAVED, "Prevents snow from going indoors");
     mincs.value.max = FLT_MAX;
     mincs.value.min = -FLT_MAX;
     r_outdoorAwayBias = Dvar_RegisterFloat(
         "r_outdoorAwayBias",
         32.0f,
         mincs,
         DVAR_SAVED,
         "Affects the height map lookup for making sure snow doesn't go indoors");
     minct.value.max = FLT_MAX;
     minct.value.min = -FLT_MAX;
     r_outdoorDownBias = Dvar_RegisterFloat(
         "r_outdoorDownBias",
         0.0f,
         minct,
         DVAR_SAVED,
         "Affects the height map lookup for making sure snow doesn't go indoors");
     mincu.value.max = FLT_MAX;
     mincu.value.min = -FLT_MAX;
     r_outdoorFeather = Dvar_RegisterFloat("r_outdoorFeather", 8.0f, mincu, DVAR_SAVED, "Outdoor z-feathering value");
     Dvar_SetModified((dvar_s*)r_outdoorFeather);
     r_sun_from_dvars = Dvar_RegisterBool(
         "r_sun_from_dvars",
         false,
         DVAR_CHEAT,
         "Set sun flare values from dvars rather than the level");
     developer = Dvar_RegisterInt("developer", 0, (DvarLimits)0x200000000LL, DVAR_NOFLAG, "Enable development options");
     sv_cheats = Dvar_RegisterBool("sv_cheats", 1, DVAR_SYSTEMINFO | DVAR_ROM, "Allow server side cheats");
     com_statmon = Dvar_RegisterBool("com_statmon", false, DVAR_NOFLAG, "Draw stats monitor");
     r_sse_skinning = Dvar_RegisterBool("r_sse_skinning", true, DVAR_NOFLAG, "Use Streaming SIMD Extensions for skinning");
     r_monitor = Dvar_RegisterInt(
         "r_monitor",
         0,
         (DvarLimits)0x800000000LL,
         DVAR_ARCHIVE | DVAR_LATCH,
         "Index of the monitor to use in a multi monitor system; 0 picks automatically.");
     r_rendererPreference = Dvar_RegisterEnum(
         "r_rendererPreference",
         s_rendererNames,
         2,
         DVAR_ARCHIVE | DVAR_LATCH,
         "Preferred renderer; unsupported renderers will never be used.");
     r_rendererInUse = Dvar_RegisterEnum("r_rendererInUse", s_rendererNames, 2, DVAR_ROM, "The renderer currently used");
     r_aspectRatio = Dvar_RegisterEnum(
         "r_aspectRatio",
         s_aspectRatioNames,
         0,
         DVAR_ARCHIVE | DVAR_LATCH,
         "Screen aspect ratio.  Most widescreen monitors are 16:10 instead of 16:9.");
     r_customMode = Dvar_RegisterString(
         "r_customMode",
         (char *)"",
         DVAR_ARCHIVE | DVAR_LATCH,
         "Special resolution mode for the remote debugger");
     R_WarnInitDvars();
     prof_probe[0] = Dvar_RegisterEnum("prof_probe0", prof_enumNames, 0, DVAR_NOFLAG, "Profile probe");
     prof_probe[1] = Dvar_RegisterEnum("prof_probe1", prof_enumNames, 0, DVAR_NOFLAG, "Profile probe");
     prof_probe[2] = Dvar_RegisterEnum("prof_probe2", prof_enumNames, 0, DVAR_NOFLAG, "Profile probe");
     prof_probe[3] = Dvar_RegisterEnum("prof_probe3", prof_enumNames, 0, DVAR_NOFLAG, "Profile probe");
     prof_probe[4] = Dvar_RegisterEnum("prof_probe4", prof_enumNames, 0, DVAR_NOFLAG, "Profile probe");
     prof_probeMaxMsec = Dvar_RegisterInt(
         "prof_probeMaxMsec",
         30,
         (DvarLimits)0x3E800000001LL,
         DVAR_NOFLAG,
         "Height of each profile probe graph, in milliseconds");
     mincv.value.max = 1000.0f;
     mincv.value.min = 0.1f;
     prof_sortTime = Dvar_RegisterFloat("prof_sortTime", 2.0f, mincv, DVAR_NOFLAG, "Time in seconds between resort profiles");
     profile_mode = Dvar_RegisterEnum("profile_mode", g_profile_mode_values, 0, DVAR_NOFLAG, "Profiler mode");
     profile_script = Dvar_RegisterBool("profile_script", false, DVAR_NOFLAG, "Enable profile scripts");
     profile_script_by_file = Dvar_RegisterBool("profile_script_by_file", false, DVAR_NOFLAG, "Enable profile scripts by source file");
     r_staticModelDumpLodInfo = Dvar_RegisterBool(
         "r_staticModelDumpLodInfo",
         false,
         DVAR_NOFLAG,
         "Dump static model info for the next frame.");
     r_altModelLightingUpdate = Dvar_RegisterBool(
         "r_altModelLightingUpdate",
         true,
         DVAR_ARCHIVE | DVAR_LATCH,
         "Use alternate model lighting update technique");
     r_preloadShaders = Dvar_RegisterBool(
         "r_preloadShaders",
         false,
         DVAR_ARCHIVE | DVAR_LATCH,
         "Force D3D to draw dummy geometry with all shaders during level load; may fix long pauses at level start.");
     R_ReflectionProbeRegisterDvars();
 }

 const dvar_t* R_RegisterWorkerThreadDvar(const char *name, unsigned int workerIndex)
 {
     const char *helpString; // [esp+4h] [ebp-Ch]
     bool defaultState; // [esp+Bh] [ebp-5h]
     unsigned __int16 flags; // [esp+Ch] [ebp-4h]

     flags = 0;
     defaultState = 1;
     helpString = "Enable worker thread";
     if (workerIndex + 2 >= Sys_GetCpuCount() || !sys_smp_allowed->current.enabled)
     {
         flags = 64;
         defaultState = 0;
         helpString = "Worker thread always disabled; not enough hardware threads";
     }
     return Dvar_RegisterBool(name, defaultState, flags, helpString);
 }

 char __cdecl R_CheckDvarModified(const dvar_s *dvar)
 {
     if (!dvar->modified)
         return 0;
     Dvar_ClearModified((dvar_s*)dvar);
     return 1;
 }

