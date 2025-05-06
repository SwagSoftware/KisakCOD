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
 const dvar_t *r_fullscreen;
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
 const dvar_t *vid_xpos;
 const dvar_t *vid_ypos;
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
         0,
         0,
         "Generate cube maps for reflection probes.");
     r_reflectionProbeRegenerateAll = Dvar_RegisterBool(
         "r_reflectionProbeRegenerateAll",
         0,
         0,
         "Regenerate cube maps for all reflection probes.");
     r_reflectionProbeGenerateExit = Dvar_RegisterBool(
         "r_reflectionProbeGenerateExit",
         0,
         0,
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
         0,
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
     r_ignore = Dvar_RegisterInt("r_ignore", 0, (DvarLimits)0x7FFFFFFF80000000LL, 0, "used for debugging anything");
     vid_xpos = Dvar_RegisterInt("vid_xpos", 3, (DvarLimits)0x1000FFFFF000LL, 1u, "Game window horizontal position");
     vid_ypos = Dvar_RegisterInt("vid_ypos", 22, (DvarLimits)0x1000FFFFF000LL, 1u, "game window vertical position");
     r_fullscreen = Dvar_RegisterBool("r_fullscreen", 1, 0x21u, "Display game full screen");
     min.value.max = 3.0;
     min.value.min = 0.5;
     r_gamma = Dvar_RegisterFloat("r_gamma", 0.80000001, min, 1u, "Gamma value");
     r_ignoreHwGamma = Dvar_RegisterBool("r_ignorehwgamma", 0, 0x21u, "Ignore hardware gamma");
     r_texFilterAnisoMax = Dvar_RegisterInt(
         "r_texFilterAnisoMax",
         16,
         (DvarLimits)0x1000000001LL,
         1u,
         "Maximum anisotropy to use for texture filtering");
     r_texFilterDisable = Dvar_RegisterBool(
         "r_texFilterDisable",
         0,
         0x80u,
         "Disables all texture filtering (uses nearest only.)");
     r_texFilterAnisoMin = Dvar_RegisterInt(
         "r_texFilterAnisoMin",
         1,
         (DvarLimits)0x1000000001LL,
         1u,
         "Minimum anisotropy to use for texture filtering (overridden by max)");
     r_texFilterMipMode = Dvar_RegisterEnum(
         "r_texFilterMipMode",
         mipFilterNames,
         0,
         1u,
         "Forces all mipmaps to use a particular blend between levels (or disables mipping.)");
     mina.value.max = 15.99;
     mina.value.min = -16.0;
     r_texFilterMipBias = Dvar_RegisterFloat("r_texFilterMipBias", 0.0, mina, 0x80u, "Change the mipmap bias");
     r_fullbright = Dvar_RegisterBool("r_fullbright", 0, 0x80u, "Toggles rendering without lighting");
     r_debugShader = Dvar_RegisterEnum("r_debugShader", debugShaderNames, 0, 0x80u, "Enable shader debugging information");
     r_gpuSync = Dvar_RegisterEnum(
         "r_gpuSync",
         gpuSyncNames,
         1,
         0,
         "GPU synchronization type (used to improve mouse responsiveness)");
     r_multiGpu = Dvar_RegisterBool("r_multiGpu", 0, 1u, "Use multiple GPUs");
     r_skinCache = Dvar_RegisterBool("r_skinCache", 1, 0, "Enable cache for vertices of animated models");
     r_fastSkin = Dvar_RegisterBool("r_fastSkin", 0, 1u, "Enable fast model skinning");
     r_smc_enable = Dvar_RegisterBool("r_smc_enable", 1, 0, "Enable static model cache");
     r_pretess = Dvar_RegisterBool("r_pretess", 1, 0, "Batch surfaces to reduce primitive count");
     minb.value.max = 3.4028235e38;
     minb.value.min = 0.0;
     r_lodScaleRigid = Dvar_RegisterFloat(
         "r_lodScaleRigid",
         1.0,
         minb,
         1u,
         "Scale the level of detail distance for rigid models (larger reduces detail)");
     minc.value.max = 3.4028235e38;
     minc.value.min = -3.4028235e38;
     r_lodBiasRigid = Dvar_RegisterFloat(
         "r_lodBiasRigid",
         0.0,
         minc,
         1u,
         "Bias the level of detail distance for rigid models (negative increases detail)");
     mind.value.max = 3.4028235e38;
     mind.value.min = 0.0;
     r_lodScaleSkinned = Dvar_RegisterFloat(
         "r_lodScaleSkinned",
         1.0,
         mind,
         1u,
         "Scale the level of detail distance for skinned models (larger reduces detail)");
     mine.value.max = 3.4028235e38;
     mine.value.min = -3.4028235e38;
     r_lodBiasSkinned = Dvar_RegisterFloat(
         "r_lodBiasSkinned",
         0.0,
         mine,
         1u,
         "Bias the level of detail distance for skinned models (negative increases detail)");
     minf.value.max = 10000.0;
     minf.value.min = 0.001;
     r_znear = Dvar_RegisterFloat(
         "r_znear",
         4.0,
         minf,
         0x80u,
         "Things closer than this aren't drawn.  Reducing this increases z-fighting in the distance.");
     ming.value.max = 16.0;
     ming.value.min = 0.001;
     r_znear_depthhack = Dvar_RegisterFloat("r_znear_depthhack", 0.1, ming, 0x80u, "Viewmodel near clip plane");
     minh.value.max = 3.4028235e38;
     minh.value.min = 0.0;
     r_zfar = Dvar_RegisterFloat(
         "r_zfar",
         0.0,
         minh,
         0x80u,
         "Change the distance at which culling fog reaches 100% opacity; 0 is off");
     r_fog = Dvar_RegisterBool("r_fog", 1, 0x80u, "Set to 0 to disable fog");
     mini.value.max = 0.0;
     mini.value.min = -4.0;
     r_polygonOffsetScale = Dvar_RegisterFloat(
         "r_polygonOffsetScale",
         -1.0,
         mini,
         1u,
         "Offset scale for decal polygons; bigger values z-fight less but poke through walls more");
     minj.value.max = 0.0;
     minj.value.min = -16.0;
     r_polygonOffsetBias = Dvar_RegisterFloat(
         "r_polygonOffsetBias",
         -1.0,
         minj,
         1u,
         "Offset bias for decal polygons; bigger values z-fight less but poke through walls more");
     r_picmip_manual = Dvar_RegisterBool(
         "r_picmip_manual",
         0,
         1u,
         "If 0, picmip is set automatically.  If 1, picmip is set based on the other r_picmip dvars.");
     r_picmip = Dvar_RegisterInt(
         "r_picmip",
         0,
         (DvarLimits)0x300000000LL,
         1u,
         "Picmip level of color maps.  If r_picmip_manual is 0, this is read-only.");
     r_picmip_bump = Dvar_RegisterInt(
         "r_picmip_bump",
         0,
         (DvarLimits)0x300000000LL,
         1u,
         "Picmip level of normal maps.  If r_picmip_manual is 0, this is read-only.");
     r_picmip_spec = Dvar_RegisterInt(
         "r_picmip_spec",
         0,
         (DvarLimits)0x300000000LL,
         1u,
         "Picmip level of specular maps.  If r_picmip_manual is 0, this is read-only.");
     r_picmip_water = Dvar_RegisterInt(
         "r_picmip_water",
         0,
         (DvarLimits)0x100000000LL,
         0x21u,
         "Picmip level of water maps.");
     r_detail = Dvar_RegisterBool("r_detail", 1, 0, "Allows shaders to use detail textures");
     r_normal = Dvar_RegisterBool("r_normal", 1, 0, "Allows shaders to use normal maps");
     r_specular = Dvar_RegisterBool("r_specular", 1, 0, "Allows shaders to use phong specular lighting");
     r_envMapSpecular = Dvar_RegisterBool("r_envMapSpecular", 1, 0, "Enables environment map specular lighting");
     r_lightMap = Dvar_RegisterEnum(
         "r_lightMap",
         colorMapNames,
         1,
         0x80u,
         "Replace all lightmaps with pure black or pure white");
     r_colorMap = Dvar_RegisterEnum(
         "r_colorMap",
         colorMapNames,
         1,
         0x80u,
         "Replace all color maps with pure black or pure white");
     r_normalMap = Dvar_RegisterEnum(
         "r_normalMap",
         normalMapNames,
         1,
         0x80u,
         "Replace all normal maps with a flat normal map");
     r_specularMap = Dvar_RegisterEnum(
         "r_specularMap",
         colorMapNames,
         1,
         0x80u,
         "Replace all specular maps with pure black (off) or pure white (super shiny)");
     mink.value.max = 100.0;
     mink.value.min = 0.0;
     r_specularColorScale = Dvar_RegisterFloat(
         "r_specularColorScale",
         1.0,
         mink,
         0x1080u,
         "Set greater than 1 to brighten specular highlights");
     minl.value.max = 100.0;
     minl.value.min = 0.0;
     r_diffuseColorScale = Dvar_RegisterFloat(
         "r_diffuseColorScale",
         1.0,
         minl,
         0x1080u,
         "Globally scale the diffuse color of all point lights");
     r_useLayeredMaterials = Dvar_RegisterBool(
         "r_useLayeredMaterials",
         0,
         0x20u,
         "Set to true to use layered materials on shader model 3 hardware");
     r_loadForRenderer = Dvar_RegisterBool(
         "r_loadForRenderer",
         1,
         0x20u,
         "Set to false to disable dx allocations (for dedicated server mode)");
     r_showTris = Dvar_RegisterInt("r_showTris", 0, (DvarLimits)0x200000000LL, 0x80u, "Show triangle outlines");
     r_showTriCounts = Dvar_RegisterBool("r_showTriCounts", 0, 0x80u, "Triangle count for each rendered entity");
     r_showSurfCounts = Dvar_RegisterBool("r_showSurfCounts", 0, 0x80u, "Surface count for each rendered entity");
     r_showVertCounts = Dvar_RegisterBool("r_showVertCounts", 0, 0x80u, "Vertex count for each entity");
     r_resampleScene = Dvar_RegisterBool(
         "r_resampleScene",
         1,
         0x80u,
         "Upscale the frame buffer with sharpen filter and color correction.");
     r_showPixelCost = Dvar_RegisterEnum(
         "r_showPixelCost",
         showPixelCostNames,
         0,
         0x80u,
         "Shows how expensive it is to draw every pixel on the screen");
     r_xdebug = Dvar_RegisterEnum("r_xdebug", xdebugNames, 0, 0x80u, "xmodel/xanim debug rendering");
     minm.value.max = 16.0;
     minm.value.min = 0.0;
     r_debugLineWidth = Dvar_RegisterFloat("r_debugLineWidth", 1.0, minm, 1u, "Width of server side debug lines");
     r_vc_makelog = Dvar_RegisterInt(
         "r_vc_makelog",
         0,
         (DvarLimits)0x200000000LL,
         0x20u,
         "Enable logging of light grid points for the vis cache.  1 starts from scratch, 2 appends.");
     r_vc_showlog = Dvar_RegisterInt(
         "r_vc_showlog",
         0,
         (DvarLimits)0x40000000000LL,
         0,
         "Show this many rows of light grid points for the vis cache");
     r_showLightGrid = Dvar_RegisterBool("r_showLightGrid", 0, 0x80u, "Show light grid debugging information");
     r_showMissingLightGrid = Dvar_RegisterBool(
         "r_showMissingLightGrid",
         1,
         0,
         "Use rainbow colors for entities that are outside the light grid");
     r_cacheSModelLighting = Dvar_RegisterBool(
         "r_cacheSModelLighting",
         1,
         0,
         "Speed up static model lighting by caching previous results");
     r_cacheModelLighting = Dvar_RegisterBool(
         "r_cacheModelLighting",
         1,
         0,
         "Speed up model lighting by caching previous results");
     minn.value.max = 4.0;
     minn.value.min = 0.0;
     r_lightTweakAmbient = Dvar_RegisterFloat("r_lightTweakAmbient", 0.1, minn, 0x240u, "Ambient light strength");
     mino.value.max = 1.0;
     mino.value.min = 0.0;
     r_lightTweakDiffuseFraction = Dvar_RegisterFloat(
         "r_lightTweakDiffuseFraction",
         0.5,
         mino,
         0x240u,
         "diffuse light fraction");
     minp.value.max = 4.0;
     minp.value.min = 0.0;
     r_lightTweakSunLight = Dvar_RegisterFloat("r_lightTweakSunLight", 1.0, minp, 0x280u, "Sunlight strength");
     r_lightTweakAmbientColor = Dvar_RegisterColor(
         "r_lightTweakAmbientColor",
         1.0,
         0.0,
         0.0,
         1.0,
         0x240u,
         "Light ambient color");
     r_lightTweakSunColor = Dvar_RegisterColor("r_lightTweakSunColor", 0.0, 1.0, 0.0, 1.0, 0x280u, "Sun color");
     r_lightTweakSunDiffuseColor = Dvar_RegisterColor(
         "r_lightTweakSunDiffuseColor",
         0.0,
         0.0,
         1.0,
         1.0,
         0x240u,
         "Sun diffuse color");
     minq.value.max = 360.0;
     minq.value.min = -360.0;
     r_lightTweakSunDirection = Dvar_RegisterVec3(
         "r_lightTweakSunDirection",
         0.0,
         0.0,
         0.0,
         minq,
         0x1280u,
         "Sun direction in degrees");
     r_envMapOverride = Dvar_RegisterBool("r_envMapOverride", 0, 0, "Min reflection intensity based on glancing angle.");
     minr.value.max = 2.0;
     minr.value.min = 0.0;
     r_envMapMinIntensity = Dvar_RegisterFloat(
         "r_envMapMinIntensity",
         0.2,
         minr,
         0,
         "Min reflection intensity based on glancing angle.");
     mins.value.max = 2.0;
     mins.value.min = 0.0099999998;
     r_envMapMaxIntensity = Dvar_RegisterFloat(
         "r_envMapMaxIntensity",
         0.5,
         mins,
         0,
         "Max reflection intensity based on glancing angle.");
     mint.value.max = 20.0;
     mint.value.min = 0.050000001;
     r_envMapExponent = Dvar_RegisterFloat("r_envMapExponent", 5.0, mint, 0, "Reflection exponent.");
     minu.value.max = 4.0;
     minu.value.min = 0.0;
     r_envMapSunIntensity = Dvar_RegisterFloat(
         "r_envMapSunIntensity",
         2.0,
         minu,
         0,
         "Max sun specular intensity intensity with env map materials.");
     r_drawPrimHistogram = Dvar_RegisterBool(
         "r_drawPrimHistogram",
         0,
         0x80u,
         "Draws a histogram of the sizes of each primitive batch");
     r_logFile = Dvar_RegisterInt(
         "r_logFile",
         0,
         (DvarLimits)0x7FFFFFFF00000000LL,
         0,
         "Write all graphics hardware calls for this many frames to a logfile");
     r_norefresh = Dvar_RegisterBool("r_norefresh", 0, 0x80u, "Skips all rendering.  Useful for benchmarking.");
     minv.value.max = 1.0;
     minv.value.min = 0.0;
     r_scaleViewport = Dvar_RegisterFloat(
         "r_scaleViewport",
         1.0,
         minv,
         0x80u,
         "Scale 3D viewports by this fraction.  Use this to see if framerate is pixel shader bound.");
     r_smp_backend = Dvar_RegisterBool("r_smp_backend", 1, 0, "Process renderer back end in a separate thread");
     r_smp_worker = Dvar_RegisterBool("r_smp_worker", 1, 0, "Process renderer front end in a separate thread");
     r_smp_worker_thread[0] = R_RegisterWorkerThreadDvar("r_smp_worker_thread0", 0);
     r_smp_worker_thread[1] = R_RegisterWorkerThreadDvar("r_smp_worker_thread1", 1u);
     r_aaAlpha = Dvar_RegisterEnum("r_aaAlpha", s_aaAlphaNames, 1, 1u, "Transparency anti-aliasing method");
     r_aaSamples = Dvar_RegisterInt(
         "r_aaSamples",
         1,
         (DvarLimits)0x1000000001LL,
         0x21u,
         "Anti-aliasing sample count; 1 disables anti-aliasing");
     r_vsync = Dvar_RegisterBool(
         "r_vsync",
         1,
         0x21u,
         "Enable v-sync before drawing the next frame to avoid 'tearing' artifacts.");
     r_clear = Dvar_RegisterEnum("r_clear", r_clearNames, 1, 0, "Controls how the color buffer is cleared");
     r_clearColor = Dvar_RegisterColor(
         "r_clearColor",
         0.5,
         0.75,
         1.0,
         1.0,
         0,
         "Color to clear the screen to when clearing the frame buffer");
     r_clearColor2 = Dvar_RegisterColor(
         "r_clearColor2",
         1.0,
         0.5,
         0.0,
         1.0,
         0,
         "Color to clear every second frame to (for use during development)");
     r_drawSun = Dvar_RegisterBool("r_drawSun", 1, 1u, "Enable sun effects");
     r_drawDecals = Dvar_RegisterBool("r_drawDecals", 1, 1u, "Enable world decal rendering");
     r_drawWorld = Dvar_RegisterBool("r_drawWorld", 1, 0x80u, "Enable world rendering");
     r_drawEntities = Dvar_RegisterBool("r_drawEntities", 1, 0x80u, "Enable entity rendering");
     r_drawPoly = Dvar_RegisterBool("r_drawPoly", 1, 0x80u, "Enable poly rendering");
     r_drawDynEnts = Dvar_RegisterBool("r_drawDynEnts", 1, 0x80u, "Enable dynamic entity rendering");
     r_drawBModels = Dvar_RegisterBool("r_drawBModels", 1, 0x80u, "Enable brush model rendering");
     r_drawSModels = Dvar_RegisterBool("r_drawSModels", 1, 0x80u, "Enable static model rendering");
     r_drawXModels = Dvar_RegisterBool("r_drawXModels", 1, 0x80u, "Enable xmodel rendering");
     r_dlightLimit = Dvar_RegisterInt(
         "r_dlightLimit",
         4,
         (DvarLimits)0x400000000LL,
         0,
         "Maximum number of dynamic lights drawn simultaneously");
     minw.value.max = 0.99000001;
     minw.value.min = 0.0;
     r_spotLightFovInnerFraction = Dvar_RegisterFloat(
         "r_spotLightFovInnerFraction",
         0.69999999,
         minw,
         0x80u,
         "Relative Inner FOV angle for the dynamic spot light. 0 is full fade 0.99 is almost no fade.");
     minx.value.max = 1200.0;
     minx.value.min = 0.0;
     r_spotLightStartRadius = Dvar_RegisterFloat(
         "r_spotLightStartRadius",
         36.0,
         minx,
         0x1080u,
         "Radius of the circle at the start of the spot light in inches.");
     miny.value.max = 1200.0;
     miny.value.min = 1.0;
     r_spotLightEndRadius = Dvar_RegisterFloat(
         "r_spotLightEndRadius",
         196.0,
         miny,
         0x1080u,
         "Radius of the circle at the end of the spot light in inches.");
     r_spotLightShadows = Dvar_RegisterBool("r_spotLightShadows", 1, 0x80u, "Enable shadows for spot lights.");
     r_spotLightSModelShadows = Dvar_RegisterBool(
         "r_spotLightSModelShadows",
         1,
         0x80u,
         "Enable static model shadows for spot lights.");
     r_spotLightEntityShadows = Dvar_RegisterBool(
         "r_spotLightEntityShadows",
         1,
         0x80u,
         "Enable entity shadows for spot lights.");
     minz.value.max = 16.0;
     minz.value.min = 0.0;
     r_spotLightBrightness = Dvar_RegisterFloat(
         "r_spotLightBrightness",
         14.0,
         minz,
         0x1080u,
         "Brightness scale for spot light to get overbrightness from the 0-1 particle color range.");
     r_drawPrimCap = Dvar_RegisterInt(
         "r_drawPrimCap",
         0,
         (DvarLimits)0x2710FFFFFFFFLL,
         0x80u,
         "Only draw primitive batches with less than this many triangles");
     r_drawPrimFloor = Dvar_RegisterInt(
         "r_drawPrimFloor",
         0,
         (DvarLimits)0x271000000000LL,
         0x80u,
         "Only draw primitive batches with more than this many triangles");
     r_skipDrawTris = Dvar_RegisterBool("r_skipDrawTris", 0, 0x80u, "Skip drawing primitive tris.");
     r_drawWater = Dvar_RegisterBool("r_drawWater", 1, 1u, "Enable water animation");
     r_lockPvs = Dvar_RegisterBool(
         "r_lockPvs",
         0,
         0x80u,
         "Lock the viewpoint used for determining what is visible to the current position and direction");
     r_skipPvs = Dvar_RegisterBool(
         "r_skipPvs",
         0,
         0x80u,
         "Skipt the determination of what is in the potentially visible set (disables most drawing)");
     minba.value.max = 1.0;
     minba.value.min = 0.0;
     r_portalBevels = Dvar_RegisterFloat(
         "r_portalBevels",
         0.69999999,
         minba,
         1u,
         "Helps cull geometry by angles of portals that are acute when projected onto the screen, value is th"
         "e cosine of the angle");
     r_portalBevelsOnly = Dvar_RegisterBool(
         "r_portalBevelsOnly",
         0,
         0,
         "Use screen-space bounding box of portals rather than the actual shape of the portal projected onto the screen");
     r_singleCell = Dvar_RegisterBool(
         "r_singleCell",
         0,
         0x80u,
         "Only draw things in the same cell as the camera.  Most useful for seeing how big the current cell is.");
     r_portalWalkLimit = Dvar_RegisterInt(
         "r_portalWalkLimit",
         0,
         (DvarLimits)0x6400000000LL,
         0x80u,
         "Stop portal recursion after this many iterations.  Useful for debugging portal errors.");
     minbb.value.max = 1.0;
     minbb.value.min = 0.0;
     r_portalMinClipArea = Dvar_RegisterFloat(
         "r_portalMinClipArea",
         0.02,
         minbb,
         0,
         "Don't clip child portals by a parent portal smaller than this fraction of the screen area.");
     r_portalMinRecurseDepth = Dvar_RegisterInt(
         "r_portalMinRecurseDepth",
         2,
         (DvarLimits)0x6400000000LL,
         0x80u,
         "Ignore r_portalMinClipArea for portals with fewer than this many parent portals.");
     r_showPortals = Dvar_RegisterInt("r_showPortals", 0, (DvarLimits)0x300000000LL, 0x80u, "Show portals for debugging");
     r_showAabbTrees = Dvar_RegisterInt(
         "r_showAabbTrees",
         0,
         (DvarLimits)0x200000000LL,
         0x80u,
         "Show axis-aligned bounding box trees used in potentially visibility set determination");
     r_showSModelNames = Dvar_RegisterBool("r_showSModelNames", 0, 0x80u, "Show static model names");
     r_showTess = Dvar_RegisterEnum("r_showTess", r_showTessNames, 0, 0x80u, "Show details for each surface");
     r_showCullBModels = Dvar_RegisterBool("r_showCullBModels", 0, 0x80u, "Show culled brush models");
     r_showCullSModels = Dvar_RegisterBool("r_showCullSModels", 0, 0x80u, "Show culled static models");
     r_showCullXModels = Dvar_RegisterBool("r_showCullXModels", 0, 0x80u, "Show culled xmodels");
     r_showFbColorDebug = Dvar_RegisterEnum(
         "r_showFbColorDebug",
         fbColorDebugNames,
         0,
         0x80u,
         "Show front buffer color debugging information");
     r_showFloatZDebug = Dvar_RegisterBool(
         "r_showFloatZDebug",
         0,
         0x80u,
         "Show float z buffer used to eliminate hard edges on particles near geometry");
     r_lightTweakSunDirection = Dvar_RegisterEnum(
         "r_showCollision",
         showCollisionNames,
         0,
         0x80u,
         "Show the collision surfaces for the selected mask types");
     r_showCollisionGroups = Dvar_RegisterEnum(
         "r_showCollisionGroups",
         showCollisionGroupsNames,
         0,
         0x80u,
         "Select whether to show the terrain, brush or all collision surface groups");
     r_showCollisionPolyType = Dvar_RegisterEnum(
         "r_showCollisionPolyType",
         showCollisionPolyTypeNames,
         0,
         0x80u,
         "Select whether to display the collision surfaces as wireframe, poly interiors, or both");
     r_showCollisionDepthTest = Dvar_RegisterBool(
         "r_showCollisionDepthTest",
         1,
         0x80u,
         "Select whether to use depth test in collision surfaces display");
     minbc.value.max = 3.4028235e38;
     minbc.value.min = 1.0;
     r_showCollisionDist = Dvar_RegisterFloat(
         "r_showCollisionDist",
         500.0,
         minbc,
         0x80u,
         "Maximum distance to show collision surfaces");
     r_floatz = Dvar_RegisterBool(
         "r_floatz",
         1,
         0x20u,
         "Allocate a float z buffer (required for effects such as floatz, dof, and laser light)");
     r_zFeather = Dvar_RegisterBool("r_zFeather", 1, 1u, "Enable z feathering (fixes particles clipping into geometry)");
     r_depthPrepass = Dvar_RegisterBool("r_depthPrepass", 0, 1u, "Enable depth prepass (usually improves performance)");
     minbd.value.max = 3.4028235e38;
     minbd.value.min = -1.0;
     r_highLodDist = Dvar_RegisterFloat("r_highLodDist", -1.0, minbd, 0x80u, "Distance for high level of detail");
     minbe.value.max = 3.4028235e38;
     minbe.value.min = -1.0;
     r_mediumLodDist = Dvar_RegisterFloat("r_mediumLodDist", -1.0, minbe, 0x80u, "Distance for medium level of detail");
     minbf.value.max = 3.4028235e38;
     minbf.value.min = -1.0;
     r_lowLodDist = Dvar_RegisterFloat("r_lowLodDist", -1.0, minbf, 0x80u, "Distance for low level of detail");
     minbg.value.max = 3.4028235e38;
     minbg.value.min = -1.0;
     r_lowestLodDist = Dvar_RegisterFloat("r_lowestLodDist", -1.0, minbg, 0x80u, "Distance for lowest level of detail");
     r_forceLod = Dvar_RegisterEnum("r_forceLod", r_forceLodNames, 4, 0x80u, "Force all level of detail to this level");
     r_modelVertColor = Dvar_RegisterBool(
         "r_modelVertColor",
         1,
         0xA0u,
         "Set to 0 to replace all model vertex colors with white when loaded");
     sc_enable = Dvar_RegisterBool("sc_enable", 0, 0, "Enable shadow cookies");
     sc_blur = Dvar_RegisterInt("sc_blur", 2, (DvarLimits)0x400000000LL, 0x80u, "Enable shadow cookie blur");
     sc_count = Dvar_RegisterInt("sc_count", 24, (DvarLimits)0x1800000000LL, 0x80u, "Number of shadow cookies");
     sc_debugCasterCount = Dvar_RegisterInt(
         "sc_debugCasterCount",
         24,
         (DvarLimits)0x1800000000LL,
         0x80u,
         "Show debugging information for the shadow cookie caster count");
     sc_debugReceiverCount = Dvar_RegisterInt(
         "sc_debugReceiverCount",
         24,
         (DvarLimits)0x1800000000LL,
         0x80u,
         "Show debugging information for the shadow cookie receiver count");
     sc_showOverlay = Dvar_RegisterBool("sc_showOverlay", 0, 0x80u, "Show shadow overlay for shadow cookies");
     sc_showDebug = Dvar_RegisterBool("sc_showDebug", 0, 0x80u, "Show debug information for shadow cookies");
     sc_wantCount = Dvar_RegisterInt("sc_wantCount", 12, (DvarLimits)0x1800000000LL, 0x80u, "Number of desired shadows");
     r_lightTweakSunDirection = Dvar_RegisterInt(
         "sc_wantCountMargin",
         1,
         (DvarLimits)0x1800000000LL,
         0x80u,
         "Margin of error on number of desired shadows");
     minbh.value.max = 1.0;
     minbh.value.min = 0.0;
     sc_fadeRange = Dvar_RegisterFloat("sc_fadeRange", 0.25, minbh, 0x80u, "Shadow cookie fade range");
     minbi.value.max = 20.0;
     minbi.value.min = 0.0;
     sc_shadowInRate = Dvar_RegisterFloat(
         "sc_shadowInRate",
         2.0,
         minbi,
         0x80u,
         "Rate at which the shadow cookie horizon moves inwards");
     minbj.value.max = 20.0;
     minbj.value.min = 0.0;
     sc_shadowOutRate = Dvar_RegisterFloat(
         "sc_shadowOutRate",
         5.0,
         minbj,
         0x80u,
         "Rate at which the shadow cookie horizon moves outwards");
     minbk.value.max = 2000.0;
     minbk.value.min = 1.0;
     sc_length = Dvar_RegisterFloat("sc_length", 400.0, minbk, 0x80u, "Shadow cookie length");
     minbl.value.max = 3.4028235e38;
     minbl.value.min = -3.4028235e38;
     sc_offscreenCasterLodBias = Dvar_RegisterFloat(
         "sc_offscreenCasterLodBias",
         0.0,
         minbl,
         0x80u,
         "Shadow cookie off-screen caster level of detail bias");
     minbm.value.max = 3.4028235e38;
     minbm.value.min = 0.0;
     sc_offscreenCasterLodScale = Dvar_RegisterFloat(
         "sc_offscreenCasterLodScale",
         20.0,
         minbm,
         0x80u,
         "Shadow cookie off-screen caster level of detail scale");
     sm_enable = Dvar_RegisterBool("sm_enable", 1, 1u, "Enable shadow mapping");
     sm_sunEnable = Dvar_RegisterBool("sm_sunEnable", 1, 0x1080u, "Enable sun shadow mapping from script");
     sm_spotEnable = Dvar_RegisterBool("sm_spotEnable", 1, 0x1080u, "Enable spot shadow mapping from script");
     sm_maxLights = Dvar_RegisterInt(
         "sm_maxLights",
         4,
         (DvarLimits)0x400000000LL,
         1u,
         "Limits how many primary lights can have shadow maps");
     minbn.value.max = 5.0;
     minbn.value.min = 0.0099999998;
     sm_spotShadowFadeTime = Dvar_RegisterFloat(
         "sm_spotShadowFadeTime",
         1.0,
         minbn,
         0,
         "How many seconds it takes for a primary light shadow map to fade in or out");
     minbo.value.max = 1024.0;
     minbo.value.min = 0.0;
     sm_lightScore_eyeProjectDist = Dvar_RegisterFloat(
         "sm_lightScore_eyeProjectDist",
         64.0,
         minbo,
         0,
         "When picking shadows for primary lights, measure distance from a point this far in fr"
         "ont of the camera.");
     minbp.value.max = 1.0;
     minbp.value.min = 0.0;
     sm_lightScore_spotProjectFrac = Dvar_RegisterFloat(
         "sm_lightScore_spotProjectFrac",
         0.125,
         minbp,
         0,
         "When picking shadows for primary lights, measure distance to a point this fraction o"
         "f the light's radius along it's shadow direction.");
     sm_showOverlay = Dvar_RegisterEnum("sm_showOverlay", sm_showOverlayNames, 0, 0x80u, "Show shadow map overlay");
     minbq.value.max = 1.0;
     minbq.value.min = 0.0;
     sm_showOverlayDepthBounds = Dvar_RegisterVec2(
         "sm_showOverlayDepthBounds",
         0.25,
         0.75,
         minbq,
         0x80u,
         "Near and far depth values for the shadow map overlay");
     minbr.value.max = 8.0;
     minbr.value.min = 0.0;
     sm_polygonOffsetScale = Dvar_RegisterFloat("sm_polygonOffsetScale", 2.0, minbr, 0, "Shadow map offset scale");
     minbs.value.max = 32.0;
     minbs.value.min = 0.0;
     sm_polygonOffsetBias = Dvar_RegisterFloat("sm_polygonOffsetBias", 0.5, minbs, 0, "Shadow map offset bias");
     minbt.value.max = 32.0;
     minbt.value.min = 0.0625;
     sm_sunSampleSizeNear = Dvar_RegisterFloat("sm_sunSampleSizeNear", 0.25, minbt, 0x1080u, "Shadow sample size");
     minbu.value.max = 3.4028235e38;
     minbu.value.min = -3.4028235e38;
     sm_sunShadowCenter = Dvar_RegisterVec3(
         "sm_sunShadowCenter",
         0.0,
         0.0,
         0.0,
         minbu,
         0x1080u,
         "Sun shadow center, 0 0 0 means don't override");
     minbv.value.max = 1.0;
     minbv.value.min = 0.25;
     sm_sunShadowScale = Dvar_RegisterFloat("sm_sunShadowScale", 1.0, minbv, 0x1080u, "Sun shadow scale optimization");
     sm_strictCull = Dvar_RegisterBool("sm_strictCull", 1, 0x80u, "Strict shadow map cull");
     sm_fastSunShadow = Dvar_RegisterBool("sm_fastSunShadow", 1, 0x80u, "Fast sun shadow");
     sm_qualitySpotShadow = Dvar_RegisterBool("sm_qualitySpotShadow", 1, 0x80u, "Fast spot shadow");
     sm_debugFastSunShadow = Dvar_RegisterBool("sm_debugFastSunShadow", 0, 0x80u, "Debug fast sun shadow");
     minbw.value.max = 32.0;
     minbw.value.min = 0.0;
     r_blur = Dvar_RegisterFloat("r_blur", 0.0, minbw, 0x80u, "Dev tweak to blur the screen");
     r_distortion = Dvar_RegisterBool("r_distortion", 1, 1u, "Enable distortion");
     r_glow_allowed = Dvar_RegisterBool("r_glow_allowed", 1, 1u, "Allow glow.");
     r_glow_allowed_script_forced = Dvar_RegisterBool(
         "r_glow_allowed_script_forced",
         0,
         0x1000u,
         "Force 'allow glow' to be treated as true, by script.");
     r_glow = Dvar_RegisterBool("r_glow", 1, 0x80u, "Enable glow.");
     r_glowUseTweaks = Dvar_RegisterBool("r_glowUseTweaks", 0, 0x80u, "Overide glow with tweak dvar values.");
     r_glowTweakEnable = Dvar_RegisterBool("r_glowTweakEnable", 0, 0x80u, "Tweak dev var; Enable glow");
     minbx.value.max = 32.0;
     minbx.value.min = 0.0;
     r_glowTweakRadius = Dvar_RegisterFloat(
         "r_glowTweakRadius0",
         5.0,
         minbx,
         0x80u,
         "Tweak dev var; Glow radius in pixels at 640x480");
     minby.value.max = 20.0;
     minby.value.min = 0.0;
     r_glowTweakBloomIntensity = Dvar_RegisterFloat(
         "r_glowTweakBloomIntensity0",
         1.0,
         minby,
         0x80u,
         "Tweak dev var; Glow bloom intensity");
     minbz.value.max = 1.0;
     minbz.value.min = 0.0;
     r_glowTweakBloomCutoff = Dvar_RegisterFloat(
         "r_glowTweakBloomCutoff",
         0.5,
         minbz,
         0x80u,
         "Tweak dev var; Glow bloom cut off fraction");
     minca.value.max = 1.0;
     minca.value.min = 0.0;
     r_glowTweakBloomDesaturation = Dvar_RegisterFloat(
         "r_glowTweakBloomDesaturation",
         0.0,
         minca,
         0x80u,
         "Tweak dev var; Glow bloom desaturation");
     r_filmUseTweaks = Dvar_RegisterBool("r_filmUseTweaks", 0, 0x80u, "Overide film effects with tweak dvar values.");
     r_filmTweakEnable = Dvar_RegisterBool("r_filmTweakEnable", 0, 0x1000u, "Tweak dev var; enable film color effects");
     mincb.value.max = 4.0;
     mincb.value.min = 0.0;
     r_filmTweakContrast = Dvar_RegisterFloat(
         "r_filmTweakContrast",
         1.4,
         mincb,
         0x1000u,
         "Tweak dev var; film color contrast");
     mincc.value.max = 4.0;
     mincc.value.min = 0.0;
     r_contrast = Dvar_RegisterFloat("r_contrast", 1.0, mincc, 0x80u, "Contrast adjustment");
     mincd.value.max = 1.0;
     mincd.value.min = -1.0;
     r_brightness = Dvar_RegisterFloat("r_brightness", 0.0, mincd, 0x80u, "Brightness adjustment");
     mince.value.max = 4.0;
     mince.value.min = 0.0;
     r_desaturation = Dvar_RegisterFloat("r_desaturation", 1.0, mince, 0x80u, "Desaturation adjustment");
     mincf.value.max = 1.0;
     mincf.value.min = -1.0;
     r_filmTweakBrightness = Dvar_RegisterFloat(
         "r_filmTweakBrightness",
         0.0,
         mincf,
         0x1000u,
         "Tweak dev var; film color brightness");
     mincg.value.max = 1.0;
     mincg.value.min = 0.0;
     r_filmTweakDesaturation = Dvar_RegisterFloat(
         "r_filmTweakDesaturation",
         0.2,
         mincg,
         0x1000u,
         "Tweak dev var; Desaturation applied after all 3D drawing");
     r_filmTweakInvert = Dvar_RegisterBool("r_filmTweakInvert", 0, 0x1000u, "Tweak dev var; enable inverted video");
     minch.value.max = 2.0;
     minch.value.min = 0.0;
     r_filmTweakDarkTint = Dvar_RegisterVec3(
         "r_filmTweakDarkTint",
         0.69999999,
         0.85000002,
         1.0,
         minch,
         0x1000u,
         "Tweak dev var; film color dark tint color");
     minci.value.max = 2.0;
     minci.value.min = 0.0;
     r_lightTweakSunDirection = Dvar_RegisterVec3(
         "r_filmTweakLightTint",
         1.1,
         1.05,
         0.85000002,
         minci,
         0x1000u,
         "Tweak dev var; film color light tint color");
     r_dof_enable = Dvar_RegisterBool("r_dof_enable", 1, 1u, "Enable the depth of field effect");
     r_dof_tweak = Dvar_RegisterBool(
         "r_dof_tweak",
         0,
         0x80u,
         "Use dvars to set the depth of field effect; overrides r_dof_enable");
     mincj.value.max = 10.0;
     mincj.value.min = 4.0;
     r_dof_nearBlur = Dvar_RegisterFloat(
         "r_dof_nearBlur",
         6.0,
         mincj,
         0x80u,
         "Sets the radius of the gaussian blur used by depth of field, in pixels at 640x480");
     minck.value.max = 10.0;
     minck.value.min = 0.0;
     r_dof_farBlur = Dvar_RegisterFloat(
         "r_dof_farBlur",
         1.8,
         minck,
         0x80u,
         "Sets the radius of the gaussian blur used by depth of field, in pixels at 640x480");
     mincl.value.max = 128.0;
     mincl.value.min = 0.0;
     r_dof_viewModelStart = Dvar_RegisterFloat(
         "r_dof_viewModelStart",
         2.0,
         mincl,
         0x80u,
         "Depth of field viewmodel start distance, in inches");
     mincm.value.max = 128.0;
     mincm.value.min = 0.0;
     r_dof_viewModelEnd = Dvar_RegisterFloat(
         "r_dof_viewModelEnd",
         8.0,
         mincm,
         0x80u,
         "Depth of field viewmodel end distance, in inches");
     mincn.value.max = 1000.0;
     mincn.value.min = 0.0;
     r_dof_nearStart = Dvar_RegisterFloat(
         "r_dof_nearStart",
         10.0,
         mincn,
         0x80u,
         "Depth of field near start distance, in inches");
     minco.value.max = 1000.0;
     minco.value.min = 0.0;
     r_dof_nearEnd = Dvar_RegisterFloat("r_dof_nearEnd", 60.0, minco, 0x80u, "Depth of field near end distance, in inches");
     mincp.value.max = 20000.0;
     mincp.value.min = 0.0;
     r_dof_farStart = Dvar_RegisterFloat(
         "r_dof_farStart",
         1000.0,
         mincp,
         0x80u,
         "Depth of field far start distance, in inches");
     mincq.value.max = 20000.0;
     mincq.value.min = 0.0;
     r_dof_farEnd = Dvar_RegisterFloat("r_dof_farEnd", 7000.0, mincq, 0x80u, "Depth of field far end distance, in inches");
     mincr.value.max = 3.0;
     mincr.value.min = 0.1;
     r_dof_bias = Dvar_RegisterFloat(
         "r_dof_bias",
         0.5,
         mincr,
         0x80u,
         "Depth of field bias as a power function (like gamma); less than 1 is sharper");
     r_outdoor = Dvar_RegisterBool("r_outdoor", 1, 0x1000u, "Prevents snow from going indoors");
     mincs.value.max = 3.4028235e38;
     mincs.value.min = -3.4028235e38;
     r_outdoorAwayBias = Dvar_RegisterFloat(
         "r_outdoorAwayBias",
         32.0,
         mincs,
         0x1000u,
         "Affects the height map lookup for making sure snow doesn't go indoors");
     minct.value.max = 3.4028235e38;
     minct.value.min = -3.4028235e38;
     r_outdoorDownBias = Dvar_RegisterFloat(
         "r_outdoorDownBias",
         0.0,
         minct,
         0x1000u,
         "Affects the height map lookup for making sure snow doesn't go indoors");
     mincu.value.max = 3.4028235e38;
     mincu.value.min = -3.4028235e38;
     r_outdoorFeather = Dvar_RegisterFloat("r_outdoorFeather", 8.0, mincu, 0x1000u, "Outdoor z-feathering value");
     Dvar_SetModified((dvar_s*)r_outdoorFeather);
     r_sun_from_dvars = Dvar_RegisterBool(
         "r_sun_from_dvars",
         0,
         0x80u,
         "Set sun flare values from dvars rather than the level");
     developer = Dvar_RegisterInt("developer", 0, (DvarLimits)0x200000000LL, 0, "Enable development options");
     sv_cheats = Dvar_RegisterBool("sv_cheats", 1, 0x48u, "Allow server side cheats");
     com_statmon = Dvar_RegisterBool("com_statmon", 0, 0, "Draw stats monitor");
     r_sse_skinning = Dvar_RegisterBool("r_sse_skinning", 1, 0, "Use Streaming SIMD Extensions for skinning");
     r_monitor = Dvar_RegisterInt(
         "r_monitor",
         0,
         (DvarLimits)0x800000000LL,
         0x21u,
         "Index of the monitor to use in a multi monitor system; 0 picks automatically.");
     r_rendererPreference = Dvar_RegisterEnum(
         "r_rendererPreference",
         s_rendererNames,
         2,
         0x21u,
         "Preferred renderer; unsupported renderers will never be used.");
     r_rendererInUse = Dvar_RegisterEnum("r_rendererInUse", s_rendererNames, 2, 0x40u, "The renderer currently used");
     r_aspectRatio = Dvar_RegisterEnum(
         "r_aspectRatio",
         s_aspectRatioNames,
         0,
         0x21u,
         "Screen aspect ratio.  Most widescreen monitors are 16:10 instead of 16:9.");
     r_customMode = Dvar_RegisterString(
         "r_customMode",
         (char *)"",
         0x21u,
         "Special resolution mode for the remote debugger");
     R_WarnInitDvars();
     prof_probe[0] = Dvar_RegisterEnum("prof_probe0", prof_enumNames, 0, 0, "Profile probe");
     prof_probe[1] = Dvar_RegisterEnum("prof_probe1", prof_enumNames, 0, 0, "Profile probe");
     prof_probe[2] = Dvar_RegisterEnum("prof_probe2", prof_enumNames, 0, 0, "Profile probe");
     prof_probe[3] = Dvar_RegisterEnum("prof_probe3", prof_enumNames, 0, 0, "Profile probe");
     prof_probe[4] = Dvar_RegisterEnum("prof_probe4", prof_enumNames, 0, 0, "Profile probe");
     prof_probeMaxMsec = Dvar_RegisterInt(
         "prof_probeMaxMsec",
         30,
         (DvarLimits)0x3E800000001LL,
         0,
         "Height of each profile probe graph, in milliseconds");
     mincv.value.max = 1000.0;
     mincv.value.min = 0.1;
     prof_sortTime = Dvar_RegisterFloat("prof_sortTime", 2.0, mincv, 0, "Time in seconds between resort profiles");
     profile_mode = Dvar_RegisterEnum("profile_mode", g_profile_mode_values, 0, 0, "Profiler mode");
     profile_script = Dvar_RegisterBool("profile_script", 0, 0, "Enable profile scripts");
     profile_script_by_file = Dvar_RegisterBool("profile_script_by_file", 0, 0, "Enable profile scripts by source file");
     r_staticModelDumpLodInfo = Dvar_RegisterBool(
         "r_staticModelDumpLodInfo",
         0,
         0,
         "Dump static model info for the next frame.");
     r_altModelLightingUpdate = Dvar_RegisterBool(
         "r_altModelLightingUpdate",
         1,
         0x21u,
         "Use alternate model lighting update technique");
     r_preloadShaders = Dvar_RegisterBool(
         "r_preloadShaders",
         0,
         0x21u,
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

