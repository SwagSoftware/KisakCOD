#include "r_material.h"
#include "r_utils.h"
#include <universal/com_files.h>

#include <algorithm>
#include <database/database.h>
#include <qcommon/com_bsp.h>
#include "r_bsp.h"
#include "r_dvars.h"
#include "r_water.h"
#include <universal/q_parse.h>

#include <d3d9.h>
#include <d3dx9shader.h>

#include "r_image.h"
#include <win32/win_local.h>
#include <devgui/devgui.h>

int g_vertexNamesCount;
ShaderBinNames *g_vertexNamesList;
int g_pixelNamesCount;
ShaderBinNames *g_pixelNamesList;

const CodeSamplerSource s_lightmapSamplers[3] =
{
    { "primary", TEXTURE_SRC_CODE_LIGHTMAP_PRIMARY, NULL, 0, 0 },
    { "secondary", TEXTURE_SRC_CODE_LIGHTMAP_SECONDARY, NULL, 0, 0 },
    {0}
};

const CodeSamplerSource s_lightSamplers[2] =
{
    { "attenuation", TEXTURE_SRC_CODE_LIGHT_ATTENUATION, NULL, 0, 0 },
    {0}
};

const CodeSamplerSource s_codeSamplers[20] =
{
  { "white", TEXTURE_SRC_CODE_WHITE, NULL, 0, 0 },
  { "black", TEXTURE_SRC_CODE_BLACK, NULL, 0, 0 },
  { "identityNormalMap", TEXTURE_SRC_CODE_IDENTITY_NORMAL_MAP, NULL, 0, 0 },
  { "lightmap", TEXTURE_SRC_CODE_LIGHTMAP_PRIMARY, s_lightmapSamplers, 0, 0 },
  { "outdoor", TEXTURE_SRC_CODE_OUTDOOR, NULL, 0, 0 },
  { "shadowmapSun", TEXTURE_SRC_CODE_SHADOWMAP_SUN, NULL, 0, 0 },
  { "shadowmapSpot", TEXTURE_SRC_CODE_SHADOWMAP_SPOT, NULL, 0, 0 },
  { "shadowCookie", TEXTURE_SRC_CODE_SHADOWCOOKIE, NULL, 0, 0 },
  { "dynamicShadow", TEXTURE_SRC_CODE_DYNAMIC_SHADOWS, NULL, 0, 0 },
  { "feedback", TEXTURE_SRC_CODE_FEEDBACK, NULL, 0, 0 },
  { "resolvedPostSun", TEXTURE_SRC_CODE_RESOLVED_POST_SUN, NULL, 0, 0 },
  { "resolvedScene", TEXTURE_SRC_CODE_RESOLVED_SCENE, NULL, 0, 0 },
  { "postEffect0", TEXTURE_SRC_CODE_POST_EFFECT_0, NULL, 0, 0 },
  { "postEffect1", TEXTURE_SRC_CODE_POST_EFFECT_1, NULL, 0, 0 },
  { "sky", TEXTURE_SRC_CODE_SKY, NULL, 0, 0 },
  { "light", TEXTURE_SRC_CODE_LIGHT_ATTENUATION, s_lightSamplers, 0, 0 },
  { "floatZ", TEXTURE_SRC_CODE_FLOATZ, NULL, 0, 0 },
  { "processedFloatZ", TEXTURE_SRC_CODE_PROCESSED_FLOATZ, NULL, 0, 0 },
  { "rawFloatZ", TEXTURE_SRC_CODE_RAW_FLOATZ, NULL, 0, 0 },
  { NULL, TEXTURE_SRC_CODE_BLACK, NULL, 0, 0 }
}; // idb

const MaterialUpdateFrequency s_codeSamplerUpdateFreq[27] =
{
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_CUSTOM,
  MTL_UPDATE_CUSTOM,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_CUSTOM
}; // idb

const CodeSamplerSource s_defaultCodeSamplers[18] =
{
  { "shadowmapSamplerSun", TEXTURE_SRC_CODE_SHADOWMAP_SUN, NULL, 0, 0 },
  { "shadowmapSamplerSpot", TEXTURE_SRC_CODE_SHADOWMAP_SPOT, NULL, 0, 0 },
  { "shadowCookieSampler", TEXTURE_SRC_CODE_SHADOWCOOKIE, NULL, 0, 0 },
  { "feedbackSampler", TEXTURE_SRC_CODE_FEEDBACK, NULL, 0, 0 },
  { "dynamicShadowSampler", TEXTURE_SRC_CODE_DYNAMIC_SHADOWS, NULL, 0, 0 },
  { "floatZSampler", TEXTURE_SRC_CODE_FLOATZ, NULL, 0, 0 },
  { "processedFloatZSampler", TEXTURE_SRC_CODE_PROCESSED_FLOATZ, NULL, 0, 0 },
  { "rawFloatZSampler", TEXTURE_SRC_CODE_RAW_FLOATZ, NULL, 0, 0 },
  { "attenuationSampler", TEXTURE_SRC_CODE_LIGHT_ATTENUATION, NULL, 0, 0 },
  { "lightmapSamplerPrimary", TEXTURE_SRC_CODE_LIGHTMAP_PRIMARY, NULL, 0, 0 },
  {
    "lightmapSamplerSecondary",
    TEXTURE_SRC_CODE_LIGHTMAP_SECONDARY,
    NULL,
    0,
    0
  },
  { "modelLightingSampler", TEXTURE_SRC_CODE_MODEL_LIGHTING, NULL, 0, 0 },
  { "cinematicYSampler", TEXTURE_SRC_CODE_CINEMATIC_Y, NULL, 0, 0 },
  { "cinematicCrSampler", TEXTURE_SRC_CODE_CINEMATIC_CR, NULL, 0, 0 },
  { "cinematicCbSampler", TEXTURE_SRC_CODE_CINEMATIC_CB, NULL, 0, 0 },
  { "cinematicASampler", TEXTURE_SRC_CODE_CINEMATIC_A, NULL, 0, 0 },
  { "reflectionProbeSampler", TEXTURE_SRC_CODE_REFLECTION_PROBE, NULL, 0, 0 },
  { NULL, TEXTURE_SRC_CODE_BLACK, NULL, 0, 0 }
}; // idb

const CodeConstantSource s_sunConsts[4] = 
{
    { "position", 0x23, 0, 0, 0 },
    { "diffuse", 0x24, 0, 0, 0 },
    { "specular", 0x25, 0, 0, 0 },
    {0}
};

const CodeConstantSource s_lightConsts[7] =
{
    { "position", 0, 0, 0, 0 },
    { "diffuse", 1, 0, 0, 0 },
    { "specular", 2, 0, 0, 0 },
    { "spotDir", 3, 0, 0, 0 },
    { "spotFactors", 4, 0, 0, 0 },
    { "falloffPlacement", 0xB, 0, 0, 0 },
    {0}
};

const CodeConstantSource s_nearPlaneConsts[4] =
{
    { "org", 5, 0, 0, 0 },
    { "dx", 6, 0, 0, 0 },
    { "dy", 7, 0, 0, 0 }
};

const CodeConstantSource s_defaultCodeConsts[14] =
{
    { "nearPlaneOrg", 5, 0, 0, 0 },
    { "nearPlaneDx", 6, 0, 0, 0 },
    { "nearPlaneDy", 7, 0, 0, 0 },
    { "sunPosition", 0x23, 0, 0, 0 },
    { "sunDiffuse", 0x24, 0, 0, 0 },
    { "sunSpecular", 0x25, 0, 0, 0 },
    { "lightPosition", 0, 0, 0, 0 },
    { "lightDiffuse", 1, 0, 0, 0 },
    { "lightSpecular", 2, 0, 0, 0 },
    { "lightSpotDir", 3, 0, 0, 0 },
    { "lightSpotFactors", 4, 0, 0, 0 },
    { "lightFalloffPlacement", 0xB, 0, 0, 0 },
    { "spotShadowmapPixelAdjust", 0x32, 0, 0, 0 },
    {0}
};

const CodeConstantSource s_codeConsts[73] =
{
  { "nearPlane", 91u, s_nearPlaneConsts, 0, 0 },
  { "sun", 91u, s_sunConsts, 0, 0 },
  { "light", 91u, s_lightConsts, 0, 0 },
  { "baseLightingCoords", 57u, NULL, 0, 0 },
  { "lightingLookupScale", 38u, NULL, 0, 0 },
  { "debugBumpmap", 39u, NULL, 0, 0 },
  { "pixelCostFracs", 19u, NULL, 0, 0 },
  { "pixelCostDecode", 20u, NULL, 0, 0 },
  { "materialColor", 40u, NULL, 0, 0 },
  { "fogConsts", 41u, NULL, 0, 0 },
  { "fogColor", 42u, NULL, 0, 0 },
  { "glowSetup", 43u, NULL, 0, 0 },
  { "glowApply", 44u, NULL, 0, 0 },
  { "filterTap", 21u, NULL, 8, 1 },
  { "codeMeshArg", 55u, NULL, 2, 1 },
  { "renderTargetSize", 10u, NULL, 0, 0 },
  { "shadowmapSwitchPartition", 32u, NULL, 0, 0 },
  { "shadowmapScale", 33u, NULL, 0, 0 },
  { "shadowmapPolygonOffset", 9u, NULL, 0, 0 },
  { "shadowParms", 8u, NULL, 0, 0 },
  { "zNear", 34u, NULL, 0, 0 },
  { "clipSpaceLookupScale", 51u, NULL, 0, 0 },
  { "clipSpaceLookupOffset", 52u, NULL, 0, 0 },
  { "dofEquationViewModelAndFarBlur", 12u, NULL, 0, 0 },
  { "dofEquationScene", 13u, NULL, 0, 0 },
  { "dofLerpScale", 14u, NULL, 0, 0 },
  { "dofLerpBias", 15u, NULL, 0, 0 },
  { "dofRowDelta", 16u, NULL, 0, 0 },
  { "depthFromClip", 54u, NULL, 0, 0 },
  { "outdoorFeatherParms", 48u, NULL, 0, 0 },
  { "envMapParms", 49u, NULL, 0, 0 },
  { "colorMatrixR", 29u, NULL, 0, 0 },
  { "colorMatrixG", 30u, NULL, 0, 0 },
  { "colorMatrixB", 31u, NULL, 0, 0 },
  { "colorBias", 45u, NULL, 0, 0 },
  { "colorTintBase", 46u, NULL, 0, 0 },
  { "colorTintDelta", 47u, NULL, 0, 0 },
  { "gameTime", 18u, NULL, 0, 0 },
  { "particleCloudColor", 17u, NULL, 0, 0 },
  { "particleCloudMatrix", 53u, NULL, 0, 0 },
  { "worldMatrix", 58u, NULL, 0, 0 },
  { "inverseWorldMatrix", 59u, NULL, 0, 0 },
  { "transposeWorldMatrix", 60u, NULL, 0, 0 },
  { "inverseTransposeWorldMatrix", 61u, NULL, 0, 0 },
  { "viewMatrix", 62u, NULL, 0, 0 },
  { "inverseViewMatrix", 63u, NULL, 0, 0 },
  { "transposeViewMatrix", 64u, NULL, 0, 0 },
  { "inverseTransposeViewMatrix", 65u, NULL, 0, 0 },
  { "projectionMatrix", 66u, NULL, 0, 0 },
  { "inverseProjectionMatrix", 67u, NULL, 0, 0 },
  { "transposeProjectionMatrix", 68u, NULL, 0, 0 },
  { "inverseTransposeProjectionMatrix", 69u, NULL, 0, 0 },
  { "worldViewMatrix", 70u, NULL, 0, 0 },
  { "inverseWorldViewMatrix", 71u, NULL, 0, 0 },
  { "transposeWorldViewMatrix", 72u, NULL, 0, 0 },
  { "inverseTransposeWorldViewMatrix", 73u, NULL, 0, 0 },
  { "viewProjectionMatrix", 74u, NULL, 0, 0 },
  { "inverseViewProjectionMatrix", 75u, NULL, 0, 0 },
  { "transposeViewProjectionMatrix", 76u, NULL, 0, 0 },
  { "inverseTransposeViewProjectionMatrix", 77u, NULL, 0, 0 },
  { "worldViewProjectionMatrix", 78u, NULL, 0, 0 },
  { "inverseWorldViewProjectionMatrix", 79u, NULL, 0, 0 },
  { "transposeWorldViewProjectionMatrix", 80u, NULL, 0, 0 },
  { "inverseTransposeWorldViewProjectionMatrix", 81u, NULL, 0, 0 },
  { "shadowLookupMatrix", 82u, NULL, 0, 0 },
  { "inverseShadowLookupMatrix", 83u, NULL, 0, 0 },
  { "transposeShadowLookupMatrix", 84u, NULL, 0, 0 },
  { "inverseTransposeShadowLookupMatrix", 85u, NULL, 0, 0 },
  { "worldOutdoorLookupMatrix", 86u, NULL, 0, 0 },
  { "inverseWorldOutdoorLookupMatrix", 87u, NULL, 0, 0 },
  { "transposeWorldOutdoorLookupMatrix", 88u, NULL, 0, 0 },
  { "inverseTransposeWorldOutdoorLookupMatrix", 89u, NULL, 0, 0 },
  { NULL, 0u, NULL, 0, 0 }
}; // idb

const LayeredTechniqueSetName s_lyrTechSetNames[33] =
{
  { "l_sm_a0c0", "l_sm_", "l_[hsm|sm]_", "a0c0" },
  { "l_sm_a0c0d0", "l_sm_", "l_[hsm|sm]_", "a0c0d0" },
  { "l_sm_a0c0d0n0", "l_sm_", "l_[hsm|sm]_", "a0c0d0n0" },
  { "l_sm_a0c0d0n0s0", "l_sm_", "l_[hsm|sm]_", "a0c0d0n0s0" },
  { "l_sm_a0c0d0s0", "l_sm_", "l_[hsm|sm]_", "a0c0d0s0" },
  { "l_sm_a0c0n0", "l_sm_", "l_[hsm|sm]_", "a0c0n0" },
  { "l_sm_a0c0n0s0", "l_sm_", "l_[hsm|sm]_", "a0c0n0s0" },
  { "l_sm_a0c0s0", "l_sm_", "l_[hsm|sm]_", "a0c0s0" },
  { "l_sm_b0c0", "l_sm_", "l_[hsm|sm]_", "b0c0" },
  { "l_sm_b0c0d0", "l_sm_", "l_[hsm|sm]_", "b0c0d0" },
  { "l_sm_b0c0d0n0", "l_sm_", "l_[hsm|sm]_", "b0c0d0n0" },
  { "l_sm_b0c0d0n0s0", "l_sm_", "l_[hsm|sm]_", "b0c0d0n0s0" },
  { "l_sm_b0c0d0s0", "l_sm_", "l_[hsm|sm]_", "b0c0d0s0" },
  { "l_sm_b0c0n0", "l_sm_", "l_[hsm|sm]_", "b0c0n0" },
  { "l_sm_b0c0n0s0", "l_sm_", "l_[hsm|sm]_", "b0c0n0s0" },
  { "l_sm_b0c0s0", "l_sm_", "l_[hsm|sm]_", "b0c0s0" },
  { "l_sm_r0c0", "l_sm_", "l_[hsm|sm]_", "r0c0" },
  { "l_sm_r0c0d0", "l_sm_", "l_[hsm|sm]_", "r0c0d0" },
  { "l_sm_r0c0d0n0", "l_sm_", "l_[hsm|sm]_", "r0c0d0n0" },
  { "l_sm_r0c0d0n0s0", "l_sm_", "l_[hsm|sm]_", "r0c0d0n0s0" },
  { "l_sm_r0c0d0s0", "l_sm_", "l_[hsm|sm]_", "r0c0d0s0" },
  { "l_sm_r0c0n0", "l_sm_", "l_[hsm|sm]_", "r0c0n0" },
  { "l_sm_r0c0n0s0", "l_sm_", "l_[hsm|sm]_", "r0c0n0s0" },
  { "l_sm_r0c0s0", "l_sm_", "l_[hsm|sm]_", "r0c0s0" },
  { "l_sm_t0c0", "l_sm_", "l_[hsm|sm]_", "t0c0" },
  { "l_sm_t0c0d0", "l_sm_", "l_[hsm|sm]_", "t0c0d0" },
  { "l_sm_t0c0d0n0", "l_sm_", "l_[hsm|sm]_", "t0c0d0n0" },
  { "l_sm_t0c0d0n0s0", "l_sm_", "l_[hsm|sm]_", "t0c0d0n0s0" },
  { "l_sm_t0c0d0s0", "l_sm_", "l_[hsm|sm]_", "t0c0d0s0" },
  { "l_sm_t0c0n0", "l_sm_", "l_[hsm|sm]_", "t0c0n0" },
  { "l_sm_t0c0n0s0", "l_sm_", "l_[hsm|sm]_", "t0c0n0s0" },
  { "l_sm_t0c0s0", "l_sm_", "l_[hsm|sm]_", "t0c0s0" },
  { "unlit_multiply", NULL, NULL, "m0c0" }
}; // idb

unsigned int g_customSamplerSrc[3] = { 0x1a, 4, 5 };
unsigned int g_customSamplerDest[3] = { 1, 2, 3 };

const MtlStateMapBitName s_alphaTestBitNames[5] =
{
    { "Always", 0x800 },
    { "GE128", 0x3000 },
    { "GT0", 0x1000 },
    { "LT128", 0x2000 },
    { 0 }
};

const MtlStateMapBitName s_blendOpRgbBitNames[7] =
{
    { "Disable", 0 },
    { "Add", 0x100  },
    { "Subtract", 0x200 },
    { "RevSubtract", 0x300 },
    { "Min", 0x400 },
    { "Max", 0x500 },
    {0}
};

const MtlStateMapBitName s_srcBlendRgbBitNames[11] =
{
    { "Zero", 1 },
    { "One", 2 },
    { "SrcColor", 3 },
    { "InvSrcColor", 4 },
    { "SrcAlpha", 5 },
    { "InvSrcAlpha", 6 },
    { "DestAlpha", 7 },
    { "InvDestAlpha", 8 },
    { "DestColor", 9 },
    { "InvDestColor", 10 },
    {0}
};

const MtlStateMapBitName s_dstBlendRgbBitNames[11] =
{
    { "Zero", 0x10 },
    { "One", 0x20 },
    { "SrcColor", 0x30 },
    { "InvSrcColor", 0x40 },
    { "SrcAlpha", 0x50 },
    { "InvSrcAlpha", 0x60 },
    { "DestAlpha", 0x70 },
    { "InvDestAlpha", 0x80 },
    { "DestColor", 0x90 },
    { "InvDestColor", 0xA0 },
    {0}
};

const MtlStateMapBitName s_blendOpAlphaBitNames[7] =
{
    { "Disable", 0 },
    { "Add", 0x1000000  },
    { "Subtract", 0x2000000 },
    { "RevSubtract", 0x3000000 },
    { "Min", 0x4000000 },
    { "Max", 0x5000000 },
    {0}
};

const MtlStateMapBitName s_srcBlendAlphaBitNames[11] =
{
    { "Zero",           0x10000 },
    { "One",            0x20000 },
    { "SrcColor",       0x30000 },
    { "InvSrcColor",    0x40000 },
    { "SrcAlpha",       0x50000 },
    { "InvSrcAlpha",    0x60000 },
    { "DestAlpha",      0x70000 },
    { "InvDestAlpha",   0x80000 },
    { "DestColor",      0x90000 },
    { "InvDestColor",   0xA0000 },
    {0}
};

const MtlStateMapBitName s_dstBlendAlphaBitNames[11] =
{
    { "Zero",           0x100000 },
    { "One",            0x200000 },
    { "SrcColor",       0x300000 },
    { "InvSrcColor",    0x400000 },
    { "SrcAlpha",       0x500000 },
    { "InvSrcAlpha",    0x600000 },
    { "DestAlpha",      0x700000 },
    { "InvDestAlpha",   0x800000 },
    { "DestColor",      0x900000 },
    { "InvDestColor",   0xA00000 },
    {0}
};

const MtlStateMapBitName s_cullFaceBitNames[4] =
{
    { "None", 0x4000 },
    { "Back", 0x8000 },
    { "Front",0xC000 },
    {0}
};

const MtlStateMapBitName s_colorWriteRgbBitNames[3] =
{
    { "Enable", 0x8000000 },
    { "Disable", 0 },
    {0}
};

const MtlStateMapBitName s_colorWriteAlphaBitNames[3] =
{
    { "Enable", 0x10000000 },
    { "Disable", 0 },
    {0}
};

const MtlStateMapBitName s_depthTestBitNames[6] =
{
    { "Disable", 2 },
    { "Less", 4 },
    { "LessEqual", 0xC },
    { "Equal", 8 },
    { "Always", 0 },
    { 0 }
};

const MtlStateMapBitName s_depthWriteBitNames[3] =
{
    { "Enable", 1 },
    { "Disable", 0 },
    {0}
};

const MtlStateMapBitName s_polygonOffsetBitNames[5] =
{
    { "0", 0 },
    { "1", 0x10 },
    { "2", 0x20 },
    { "shadowmap", 0x30 },
    {0}
};

const MtlStateMapBitName s_stencilBitNames[4] =
{
    { "Disable", 0 },
    { "OneSided", 0x40 },
    { "TwoSided", 0xC0 },
    {0}
};

const MtlStateMapBitName s_stencilOpFrontPassBitNames[9] =
{
    { "Keep", 0 },
    { "Zero", 0x100 },
    { "Replace", 0x200 },
    { "IncrSat", 0x300 },
    { "DecrSat", 0x400 },
    { "Invert", 0x500 },
    { "Incr", 0x600 },
    { "Decr", 0x700 },
    { 0 }
};

const MtlStateMapBitName s_stencilFuncFrontBitNames[9] =
{
    { "Never",          0 },
    { "Less",           0x20000 },
    { "Equal",          0x40000 },
    { "LessEqual",      0x60000 },
    { "Greater",        0x80000 },
    { "NotEqual",       0xA0000 },
    { "GreaterEqual",   0xC0000 },
    { "Always",         0xE0000 },
    {0}
};

const MtlStateMapBitName s_stencilOpFrontFailBitNames[9] =
{
    { "Keep", 0 },
    { "Zero", 0x800 },
    { "Replace", 0x1000 },
    { "IncrSat", 0x1800 },
    { "DecrSat", 0x2000 },
    { "Invert", 0x2800 },
    { "Incr", 0x3000 },
    { "Decr", 0x3800 },
    {0}
};

const MtlStateMapBitName s_stencilOpFrontZFailBitNames[9] =
{
    { "Keep",    0 },
    { "Zero",   0x4000 },
    { "Replace", 0x8000 },
    { "IncrSat", 0xC000 },
    { "DecrSat", 0x10000 },
    { "Invert",  0x14000 },
    { "Incr",    0x18000 },
    { "Decr",    0x1C000 },
    {0}
};

const MtlStateMapBitName s_stencilFuncBackBitNames[9] =
{
    { "Never", 0 },
    { "Less", 0x20000000 },
    { "Equal", 0x40000000 },
    { "LessEqual", 0x60000000 },
    { "Greater", 0x80000000 },
    { "NotEqual", 0x0A0000000 },
    { "GreaterEqual", 0x0C0000000 },
    { "Always", 0x0E0000000 },
    {0}
};

const MtlStateMapBitName s_stencilOpBackPassBitNames[9] =
{
    { "Keep", 0 },
    { "Zero",       0x100000 },
    { "Replace",    0x200000 },
    { "IncrSat",    0x300000 },
    { "DecrSat",    0x400000 },
    { "Invert",     0x500000 },
    { "Incr",       0x600000 },
    { "Decr",       0x700000 },
    {0}
};

const MtlStateMapBitName s_stencilOpBackFailBitNames[9] =
{
    { "Keep",       0        },
    { "Zero",       0x800000 },
    { "Replace",    0x1000000 },
    { "IncrSat",    0x1800000 },
    { "DecrSat",    0x2000000 },
    { "Invert",     0x2800000 },
    { "Incr",       0x3000000 },
    { "Decr",       0x3800000 },
    {0}
};

const MtlStateMapBitName s_stencilOpBackZFailBitNames[9] =
{
    { "Keep",       0        },
    { "Zero",       0x4000000 },
    { "Replace",    0x8000000 },
    { "IncrSat",    0xC000000 },
    { "DecrSat",    0x10000000 },
    { "Invert",     0x14000000 },
    { "Incr",       0x18000000 },
    { "Decr",       0x1C000000 },
    {0}
};

const MtlStateMapBitName s_wireframeBitNames[3] =
{
    { "Enable", 0x80000000 },
    { "Disable", 0 },
    {0}
};

const MtlStateMapBitGroup s_stateMapSrcBitGroup[23] =
{
  { "mtlAlphaTest", s_alphaTestBitNames, { 14336, 0 } },
  { "mtlBlendOp", s_blendOpRgbBitNames, { 1792, 0 } },
  { "mtlSrcBlend", s_srcBlendRgbBitNames, { 15, 0 } },
  { "mtlDestBlend", s_dstBlendRgbBitNames, { 240, 0 } },
  { "mtlBlendOpAlpha", s_blendOpAlphaBitNames, { 117440512, 0 } },
  { "mtlSrcBlendAlpha", s_srcBlendAlphaBitNames, { 983040, 0 } },
  { "mtlDestBlendAlpha", s_dstBlendAlphaBitNames, { 15728640, 0 } },
  { "mtlCullFace", s_cullFaceBitNames, { 49152, 0 } },
  { "mtlColorWriteRgb", s_colorWriteRgbBitNames, { 134217728, 0 } },
  { "mtlColorWriteAlpha", s_colorWriteAlphaBitNames, { 268435456, 0 } },
  { "mtlDepthTest", s_depthTestBitNames, { 0, 14 } },
  { "mtlDepthWrite", s_depthWriteBitNames, { 0, 1 } },
  { "mtlPolygonOffset", s_polygonOffsetBitNames, { 0, 48 } },
  { "mtlStencil", s_stencilBitNames, { 0, 192 } },
  { "mtlStencilFuncFront", s_stencilFuncFrontBitNames, { 0, 917504 } },
  { "mtlStencilOpFrontPass", s_stencilOpFrontPassBitNames, { 0, 1792 } },
  { "mtlStencilOpFrontFail", s_stencilOpFrontFailBitNames, { 0, 14336 } },
  { "mtlStencilOpFrontZFail", s_stencilOpFrontZFailBitNames, { 0, 114688 } },
  { "mtlStencilFuncBack", s_stencilFuncBackBitNames, { 0, 917504 } },
  { "mtlStencilOpBackPass", s_stencilOpBackPassBitNames, { 0, 1792 } },
  { "mtlStencilOpBackFail", s_stencilOpBackFailBitNames, { 0, 14336 } },
  { "mtlStencilOpBackZFail", s_stencilOpBackZFailBitNames, { 0, 114688 } },
  { NULL, NULL, { 0, 0 } }
}; // idb
const MtlStateMapBitGroup s_stateMapDstAlphaTestBitGroup[2] =
{
  { "alphaTest", s_alphaTestBitNames, { 14336, 0 } },
  { NULL, NULL, { 0, 0 } }
}; // idb
const MtlStateMapBitGroup s_stateMapDstBlendFuncRgbBitGroup[4] =
{
  { "blendFuncRgb", s_blendOpRgbBitNames, { 1792, 0 } },
  { "blendFuncRgb", s_srcBlendRgbBitNames, { 1807, 0 } },
  { "blendFuncRgb", s_dstBlendRgbBitNames, { 2032, 0 } },
  { NULL, NULL, { 0, 0 } }
}; // idb
const MtlStateMapBitGroup s_stateMapDstBlendFuncAlphaBitGroup[4] =
{
  { "blendFuncAlpha", s_blendOpAlphaBitNames, { 117440512, 0 } },
  { "blendFuncAlpha", s_srcBlendAlphaBitNames, { 983040, 0 } },
  { "blendFuncAlpha", s_dstBlendAlphaBitNames, { 15728640, 0 } },
  { NULL, NULL, { 0, 0 } }
}; // idb
const MtlStateMapBitGroup s_stateMapDstCullFaceBitGroup[2] = { { "cullFace", s_cullFaceBitNames, { 49152, 0 } }, { NULL, NULL, { 0, 0 } } }; // idb
const MtlStateMapBitGroup s_stateMapDstDepthTestBitGroup[2] = { { "depthTest", s_depthTestBitNames, { 0, 14 } }, { NULL, NULL, { 0, 0 } } }; // idb
const MtlStateMapBitGroup s_stateMapDstDepthWriteBitGroup[2] = { { "depthWrite", s_depthWriteBitNames, { 0, 1 } }, { NULL, NULL, { 0, 0 } } }; // idb
const MtlStateMapBitGroup s_stateMapDstColorWriteBitGroup[3] =
{
  { "colorWrite", s_colorWriteRgbBitNames, { 134217728, 0 } },
  { "colorWrite", s_colorWriteAlphaBitNames, { 268435456, 0 } },
  { NULL, NULL, { 0, 0 } }
}; // idb
const MtlStateMapBitGroup s_stateMapDstPolygonOffsetBitGroup[2] =
{
  { "polygonOffset", s_polygonOffsetBitNames, { 0, 48 } },
  { NULL, NULL, { 0, 0 } }
}; // idb
const MtlStateMapBitGroup s_stateMapDstWireframeBitGroup[2] =
{
  { "wireframe", s_wireframeBitNames, { 2147483648, 0 } },
  { NULL, NULL, { 0, 0 } }
}; // idb
const MtlStateMapBitGroup s_stateMapDstStencilBitGroup[10] =
{
  { "stencil", s_stencilBitNames, { 0, 192 } },
  { "stencil", s_stencilFuncFrontBitNames, { 0, 917504 } },
  { "stencil", s_stencilOpFrontPassBitNames, { 0, 1792 } },
  { "stencil", s_stencilOpFrontFailBitNames, { 0, 14336 } },
  { "stencil", s_stencilOpFrontZFailBitNames, { 0, 114688 } },
  { "stencil", s_stencilFuncBackBitNames, { 0, 917504 } },
  { "stencil", s_stencilOpBackPassBitNames, { 0, 1792 } },
  { "stencil", s_stencilOpBackFailBitNames, { 0, 14336 } },
  { "stencil", s_stencilOpBackZFailBitNames, { 0, 114688 } },
  { NULL, NULL, { 0, 0 } }
}; // idb

$8E67C8D28114E56A26FBAF05ACADB66A mtlLoadGlob;

char __cdecl MaterialTechniqueSet_FindHashLocation(const char *name, int *foundHashIndex)
{
    int hashIndex; // [esp+0h] [ebp-4h]

    iassert( name );
    for (hashIndex = R_HashAssetName(name) & 0x3FF;
        materialGlobals.techniqueSetHashTable[hashIndex];
        hashIndex = ((_WORD)hashIndex + 1) & 0x3FF)
    {
        if (!I_stricmp(materialGlobals.techniqueSetHashTable[hashIndex]->name, name))
        {
            *foundHashIndex = hashIndex;
            return 1;
        }
    }
    *foundHashIndex = hashIndex;
    return 0;
}

bool __cdecl Material_MatchToken(const char **text, const char *match)
{
    return Com_MatchToken(text, match, 1) != 0;
}

int __cdecl Material_TechniqueTypeForName(const char *name)
{
    const char *techniqueNames[34]; // [esp+14h] [ebp-90h]
    int techniqueIndex; // [esp+A0h] [ebp-4h]

    techniqueNames[0] = "\"depth prepass\"";
    techniqueNames[1] = "\"build floatz\"";
    techniqueNames[2] = "\"build shadowmap depth\"";
    techniqueNames[3] = "\"build shadowmap color\"";
    techniqueNames[4] = "\"unlit\"";
    techniqueNames[5] = "\"emissive\"";
    techniqueNames[6] = "\"emissive shadow\"";
    techniqueNames[7] = "\"lit\"";
    techniqueNames[8] = "\"lit sun\"";
    techniqueNames[9] = "\"lit sun shadow\"";
    techniqueNames[10] = "\"lit spot\"";
    techniqueNames[11] = "\"lit spot shadow\"";
    techniqueNames[12] = "\"lit omni\"";
    techniqueNames[13] = "\"lit omni shadow\"";
    techniqueNames[14] = "\"lit instanced\"";
    techniqueNames[15] = "\"lit instanced sun\"";
    techniqueNames[16] = "\"lit instanced sun shadow\"";
    techniqueNames[17] = "\"lit instanced spot\"";
    techniqueNames[18] = "\"lit instanced spot shadow\"";
    techniqueNames[19] = "\"lit instanced omni\"";
    techniqueNames[20] = "\"lit instanced omni shadow\"";
    techniqueNames[21] = "\"light spot\"";
    techniqueNames[22] = "\"light omni\"";
    techniqueNames[23] = "\"light spot shadow\"";
    techniqueNames[24] = "\"fakelight normal\"";
    techniqueNames[25] = "\"fakelight view\"";
    techniqueNames[26] = "\"sunlight preview\"";
    techniqueNames[27] = "\"case texture\"";
    techniqueNames[28] = "\"solid wireframe\"";
    techniqueNames[29] = "\"shaded wireframe\"";
    techniqueNames[30] = "\"shadowcookie caster\"";
    techniqueNames[31] = "\"shadowcookie receiver\"";
    techniqueNames[32] = "\"debug bumpmap\"";
    techniqueNames[33] = "\"debug bumpmap instanced\"";
    for (techniqueIndex = 0; techniqueIndex < 0x22; ++techniqueIndex)
    {
        if (!strcmp(name, techniqueNames[techniqueIndex]))
            return techniqueIndex;
    }
    return 34;
}

const bool g_useTechnique[34] =
{
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  false,
  false,
  false,
  false,
  true,
  false,
  true,
  true,
  true,
  true
}; // idb
bool __cdecl Material_UsingTechnique(unsigned int techType)
{
    if (techType >= 0x22)
        MyAssertHandler(
            ".\\r_material_load_obj.cpp",
            1824,
            0,
            "techType doesn't index TECHNIQUE_COUNT\n\t%i not in [0, %i)",
            techType,
            34);
    return g_useTechnique[techType];
}

char __cdecl Material_HasMatchingParameter(
    unsigned __int8 find,
    const ShaderVaryingDef *paramTable,
    unsigned int paramCount)
{
    unsigned int paramIndex; // [esp+0h] [ebp-4h]

    for (paramIndex = 0; paramIndex < paramCount; ++paramIndex)
    {
        if (paramTable[paramIndex].resourceDest == find)
            return 1;
    }
    return 0;
}

char __cdecl Material_HasMatchingParameter_BuggySdkWorkaround(
    unsigned __int8 find,
    const ShaderVaryingDef *paramTable,
    unsigned int paramCount)
{
    if (find == 2)
        return Material_HasMatchingParameter(3u, paramTable, paramCount);
    if (find == 3)
        return Material_HasMatchingParameter(2u, paramTable, paramCount);
    return 0;
}

bool __cdecl Material_ValidateShaderLinkage(
    const ShaderVaryingDef *vertexOutputs,
    unsigned int vertexOutputCount,
    const ShaderVaryingDef *pixelInputs,
    unsigned int pixelInputCount)
{
    unsigned int paramIndex; // [esp+0h] [ebp-8h]
    unsigned int paramIndexa; // [esp+0h] [ebp-8h]
    bool isValid; // [esp+7h] [ebp-1h]

    isValid = 1;
    for (paramIndex = 0; paramIndex < pixelInputCount; ++paramIndex)
    {
        if (!Material_HasMatchingParameter(pixelInputs[paramIndex].resourceDest, vertexOutputs, vertexOutputCount)
            && !Material_HasMatchingParameter_BuggySdkWorkaround(
                pixelInputs[paramIndex].resourceDest,
                vertexOutputs,
                vertexOutputCount))
        {
            Com_ScriptError(
                "Pixel shader input '%s' doesn't have a corresponding vertex shader output.\n",
                pixelInputs[paramIndex].name);
            isValid = 0;
        }
    }
    for (paramIndexa = 0; paramIndexa < vertexOutputCount; ++paramIndexa)
    {
        if (!Material_HasMatchingParameter(vertexOutputs[paramIndexa].resourceDest, pixelInputs, pixelInputCount)
            && !Material_HasMatchingParameter_BuggySdkWorkaround(
                vertexOutputs[paramIndexa].resourceDest,
                pixelInputs,
                pixelInputCount))
        {
            Com_ScriptError(
                "Vertex shader output '%s' doesn't have a corresponding pixel shader input.\n",
                vertexOutputs[paramIndexa].name);
            isValid = 0;
        }
    }
    return isValid;
}

char __cdecl MaterialTechnique_FindHashLocation(const char *name, GfxRenderer renderer, unsigned int *foundHashIndex)
{
    unsigned int hashIndex; // [esp+0h] [ebp-8h]
    MaterialTechnique **hashTable; // [esp+4h] [ebp-4h]

    hashTable = mtlLoadGlob.techniqueHashTable[renderer];
    for (hashIndex = R_HashAssetName(name) & 0xFFF; hashTable[hashIndex]; hashIndex = (hashIndex + 1) & 0xFFF)
    {
        if (!I_stricmp(hashTable[hashIndex]->name, name))
        {
            *foundHashIndex = hashIndex;
            return 1;
        }
    }
    *foundHashIndex = hashIndex;
    return 0;
}

MaterialTechnique *__cdecl Material_FindTechnique(const char *name, GfxRenderer renderer)
{
    unsigned int hashIndex; // [esp+0h] [ebp-4h] BYREF

    iassert( name );
    if (MaterialTechnique_FindHashLocation(name, renderer, &hashIndex))
        return mtlLoadGlob.techniqueHashTable[renderer][hashIndex];
    else
        return 0;
}

char __cdecl Material_HashStateMap(const char *name, unsigned int *foundHashIndex)
{
    unsigned int hashIndex; // [esp+14h] [ebp-4h]

    for (hashIndex = R_HashAssetName(name) & 0x1F;
        mtlLoadGlob.stateMapHashTable[hashIndex];
        hashIndex = (hashIndex + 1) & 0x1F)
    {
        if (!strcmp(mtlLoadGlob.stateMapHashTable[hashIndex]->name, name))
        {
            *foundHashIndex = hashIndex;
            return 1;
        }
    }
    *foundHashIndex = hashIndex;
    return 0;
}

MaterialStateMap *__cdecl Material_FindStateMap(const char *name)
{
    unsigned int hashIndex; // [esp+0h] [ebp-4h] BYREF

    if (Material_HashStateMap(name, &hashIndex))
        return mtlLoadGlob.stateMapHashTable[hashIndex];
    else
        return 0;
}

const MtlStateMapBitName *__cdecl Material_ParseValueForState(const char **text, const MtlStateMapBitName *bitNames)
{
    parseInfo_t *v2; // eax
    int valueIndex; // [esp+14h] [ebp-110h]
    parseInfo_t *token; // [esp+18h] [ebp-10Ch]
    char signedToken[260]; // [esp+1Ch] [ebp-108h] BYREF

    token = Com_Parse(text);
    if (token->token[0] == 45)
    {
        v2 = Com_Parse(text);
        Com_sprintf(signedToken, 0x100u, "-%s", v2->token);
        //token = signedToken;
        strcpy(token->token, signedToken);
    }
    for (valueIndex = 0; bitNames[valueIndex].name; ++valueIndex)
    {
        if (!strcmp(token->token, bitNames[valueIndex].name))
            return &bitNames[valueIndex];
    }
    Com_ScriptError("%s is not a valid state value\n", token->token);
    return 0;
}

int __cdecl Material_ParseRuleSetConditionTest(const char **text, const char *token, MaterialStateMapRule *rule)
{
    int sourceIndex; // [esp+18h] [ebp-Ch]
    const MtlStateMapBitName *bitName; // [esp+1Ch] [ebp-8h]
    int stateBitsIndex; // [esp+20h] [ebp-4h]

    for (sourceIndex = 0; ; ++sourceIndex)
    {
        if (!s_stateMapSrcBitGroup[sourceIndex].name)
            return 1;
        if (!strcmp(token, s_stateMapSrcBitGroup[sourceIndex].name))
            break;
    }
    if (!Material_MatchToken(text, "=="))
        return 2;
    bitName = Material_ParseValueForState(text, s_stateMapSrcBitGroup[sourceIndex].bitNames);
    if (!bitName)
        return 2;
    stateBitsIndex = 0;
    while (!s_stateMapSrcBitGroup[sourceIndex].stateBitsMask[stateBitsIndex])
    {
        if (++stateBitsIndex >= 2)
            MyAssertHandler(
                ".\\r_material_load_obj.cpp",
                1889,
                0,
                "%s\n\t(s_stateMapSrcBitGroup[sourceIndex].name) = %s",
                "(stateBitsIndex < 2)",
                s_stateMapSrcBitGroup[sourceIndex].name);
    }
    rule->stateBitsMask[stateBitsIndex] |= s_stateMapSrcBitGroup[sourceIndex].stateBitsMask[stateBitsIndex];
    rule->stateBitsValue[stateBitsIndex] |= bitName->bits;
    return 0;
}

int __cdecl Material_ParseRuleSetCondition(const char **text, const char *token, MaterialStateMapRule *rule)
{
    MtlParseSuccess success; // [esp+3Ch] [ebp-4h]
    MtlParseSuccess successa; // [esp+3Ch] [ebp-4h]
    parseInfo_t *tokena; // [esp+4Ch] [ebp+Ch]
    parseInfo_t *tokenb; // [esp+4Ch] [ebp+Ch]

    if (rule->stateBitsMask[0])
        MyAssertHandler(
            ".\\r_material_load_obj.cpp",
            1901,
            1,
            "%s\n\t(rule->stateBitsMask[0]) = %i",
            "(rule->stateBitsMask[0] == 0)",
            rule->stateBitsMask[0]);
    if (rule->stateBitsMask[1])
        MyAssertHandler(
            ".\\r_material_load_obj.cpp",
            1902,
            1,
            "%s\n\t(rule->stateBitsMask[1]) = %i",
            "(rule->stateBitsMask[1] == 0)",
            rule->stateBitsMask[1]);
    if (rule->stateBitsValue[0])
        MyAssertHandler(
            ".\\r_material_load_obj.cpp",
            1903,
            1,
            "%s\n\t(rule->stateBitsValue[0]) = %i",
            "(rule->stateBitsValue[0] == 0)",
            rule->stateBitsValue[0]);
    if (rule->stateBitsValue[1])
        MyAssertHandler(
            ".\\r_material_load_obj.cpp",
            1904,
            1,
            "%s\n\t(rule->stateBitsValue[1]) = %i",
            "(rule->stateBitsValue[1] == 0)",
            rule->stateBitsValue[1]);
    if (!strcmp(token, "default"))
        return Material_MatchToken(text, ":") ? 0 : 2;
    success = (MtlParseSuccess)Material_ParseRuleSetConditionTest(text, token, rule);
    if (success)
        return success;
    do
    {
        tokena = Com_Parse(text);
        if (!strcmp(tokena->token, ":"))
            return 0;
        if (strcmp(tokena->token, "&&"))
        {
            Com_ScriptError("expected ':' or '&&', found '%s'\n", tokena->token);
            return 2;
        }
        tokenb = Com_Parse(text);
        successa = (MtlParseSuccess)Material_ParseRuleSetConditionTest(text, tokenb->token, rule);
    } while (successa == MTL_PARSE_SUCCESS);
    if (successa == MTL_PARSE_NO_MATCH)
        Com_ScriptError("failed parsing conditional after '&&'\n");
    return 2;
}

bool __cdecl Material_ParseRuleSetValue(
    const char **text,
    const char *token,
    const MtlStateMapBitGroup *stateSet,
    MaterialStateMapRule *rule)
{
    int stateIndex; // [esp+18h] [ebp-Ch]
    const MtlStateMapBitName *bitName; // [esp+1Ch] [ebp-8h]
    int stateBitsIndex; // [esp+20h] [ebp-4h]

    if (rule->stateBitsClear[0])
        MyAssertHandler(
            ".\\r_material_load_obj.cpp",
            1940,
            1,
            "%s\n\t(rule->stateBitsClear[0]) = %i",
            "(rule->stateBitsClear[0] == 0)",
            rule->stateBitsClear[0]);
    if (rule->stateBitsClear[1])
        MyAssertHandler(
            ".\\r_material_load_obj.cpp",
            1941,
            1,
            "%s\n\t(rule->stateBitsClear[1]) = %i",
            "(rule->stateBitsClear[1] == 0)",
            rule->stateBitsClear[1]);
    if (rule->stateBitsSet[0])
        MyAssertHandler(
            ".\\r_material_load_obj.cpp",
            1942,
            1,
            "%s\n\t(rule->stateBitsSet[0]) = %i",
            "(rule->stateBitsSet[0] == 0)",
            rule->stateBitsSet[0]);
    if (rule->stateBitsSet[1])
        MyAssertHandler(
            ".\\r_material_load_obj.cpp",
            1943,
            1,
            "%s\n\t(rule->stateBitsSet[1]) = %i",
            "(rule->stateBitsSet[1] == 0)",
            rule->stateBitsSet[1]);
    if (!strcmp(token, "passthrough"))
        return Material_MatchToken(text, ";");
    Com_UngetToken();
    stateIndex = 0;
    do
    {
        bitName = Material_ParseValueForState(text, stateSet[stateIndex].bitNames);
        if (!bitName)
            return 0;
        stateBitsIndex = 0;
        while (!stateSet[stateIndex].stateBitsMask[stateBitsIndex])
        {
            if (++stateBitsIndex >= 2)
                MyAssertHandler(
                    ".\\r_material_load_obj.cpp",
                    1966,
                    0,
                    "%s\n\t(stateSet[stateIndex].name) = %s",
                    "(stateBitsIndex < 2)",
                    stateSet[stateIndex].name);
        }
        rule->stateBitsSet[stateBitsIndex] |= bitName->bits;
        rule->stateBitsClear[stateBitsIndex] |= stateSet[stateIndex++].stateBitsMask[stateBitsIndex];
        if (!stateSet[stateIndex].name)
            return Material_MatchToken(text, ";");
    } while (Material_MatchToken(text, ","));
    return 0;
}

MaterialStateMapRuleSet *__cdecl Material_AssembleRuleSet(int ruleCount, MaterialStateMapRule *rules)
{
    int stateBitsIndex; // [esp+0h] [ebp-Ch]
    unsigned __int8 *ruleSet; // [esp+4h] [ebp-8h]
    int ruleIndex; // [esp+8h] [ebp-4h]

    ruleSet = Material_Alloc(32 * ruleCount + 4);
    *(_DWORD *)ruleSet = ruleCount;
    Com_Memcpy((char *)ruleSet + 4, (char *)rules, 32 * ruleCount);
    for (ruleIndex = 0; ruleIndex < ruleCount; ++ruleIndex)
    {
        for (stateBitsIndex = 0; stateBitsIndex < 2; ++stateBitsIndex)
            *(_DWORD *)&ruleSet[32 * ruleIndex + 28 + 4 * stateBitsIndex] = ~*(_DWORD *)&ruleSet[32 * ruleIndex + 28 + 4 * stateBitsIndex];
    }
    return (MaterialStateMapRuleSet *)ruleSet;
}

char __cdecl Material_ParseRuleSet(
    char **text,
    char *ruleSetName,
    const MtlStateMapBitGroup *stateSet,
    MaterialStateMapRuleSet **ruleSet)
{
    int v5; // edx
    int v6; // eax
    int v7; // [esp+0h] [ebp-2034h]
    MaterialStateMapRule dst[257]; // [esp+4h] [ebp-2030h] BYREF
    int v9; // [esp+2024h] [ebp-10h]
    int ruleCount; // [esp+2028h] [ebp-Ch]
    char *token; // [esp+202Ch] [ebp-8h]
    int i; // [esp+2030h] [ebp-4h]

    if (!Material_MatchToken((const char**)text, (const char*)ruleSetName))
        return 0;
    if (!Material_MatchToken((const char**)text, "{"))
        return 0;
    memset(dst, 0, sizeof(dst));
    v7 = 0;
    ruleCount = 0;
    while (1)
    {
        if (ruleCount > 256)
        {
            Com_ScriptError("state %s has more than %i rules\n", ruleSetName, 256);
            return 0;
        }
        token = Com_Parse((const char**)text)->token;
        if (*token == 125)
            break;
        v9 = Material_ParseRuleSetCondition((const char **)text, token, &dst[ruleCount]);
        if (v9)
        {
            if (v9 == 2)
                return 0;
            if (v7 == ruleCount)
            {
                Com_ScriptError("missing rule condition for state %s\n", ruleSetName);
                return 0;
            }
            if (!Material_ParseRuleSetValue((const char **)text, token, stateSet, &dst[v7]))
                return 0;
            for (i = v7 + 1; i < ruleCount; ++i)
            {
                v5 = i;
                dst[v5].stateBitsSet[0] = dst[v7].stateBitsSet[0];
                dst[v5].stateBitsSet[1] = dst[v7].stateBitsSet[1];
                v6 = i;
                dst[v6].stateBitsClear[0] = dst[v7].stateBitsClear[0];
                dst[v6].stateBitsClear[1] = dst[v7].stateBitsClear[1];
            }
            v7 = ruleCount;
        }
        else
        {
            ++ruleCount;
        }
    }
    if (ruleCount)
    {
        if (v7 == ruleCount)
        {
            *ruleSet = Material_AssembleRuleSet(ruleCount, dst);
            return 1;
        }
        else
        {
            Com_ScriptError("missing value for state %s\n", ruleSetName);
            return 0;
        }
    }
    else
    {
        Com_ScriptError("no entries for state %s: you may want to do 'default: passthrough;'\n", ruleSetName);
        return 0;
    }
}

bool __cdecl Material_ParseStateMap(char **text, MaterialStateMap *stateMap)
{
    if (!Material_ParseRuleSet(text, (char*)"alphaTest", s_stateMapDstAlphaTestBitGroup, stateMap->ruleSet))
        return 0;
    if (!Material_ParseRuleSet(text, (char *)"blendFunc", s_stateMapDstBlendFuncRgbBitGroup, &stateMap->ruleSet[1]))
        return 0;
    if (!Material_ParseRuleSet(text, (char *)"separateAlphaBlendFunc", s_stateMapDstBlendFuncAlphaBitGroup, &stateMap->ruleSet[2]))
        return 0;
    if (!Material_ParseRuleSet(text, (char *)"cullFace", s_stateMapDstCullFaceBitGroup, &stateMap->ruleSet[3]))
        return 0;
    if (!Material_ParseRuleSet(text, (char *)"depthTest", s_stateMapDstDepthTestBitGroup, &stateMap->ruleSet[4]))
        return 0;
    if (!Material_ParseRuleSet(text, (char *)"depthWrite", s_stateMapDstDepthWriteBitGroup, &stateMap->ruleSet[5]))
        return 0;
    if (!Material_ParseRuleSet(text, (char *)"colorWrite", s_stateMapDstColorWriteBitGroup, &stateMap->ruleSet[6]))
        return 0;
    if (!Material_ParseRuleSet(text, (char *)"polygonOffset", s_stateMapDstPolygonOffsetBitGroup, &stateMap->ruleSet[7]))
        return 0;
    if (Material_ParseRuleSet(text, (char *)"stencil", s_stateMapDstStencilBitGroup, &stateMap->ruleSet[8]))
        return Material_ParseRuleSet(text, (char *)"wireframe", s_stateMapDstWireframeBitGroup, &stateMap->ruleSet[9]) != 0;
    return 0;
}

MaterialStateMap *__cdecl Material_LoadStateMap(char *name)
{
    unsigned int v2; // [esp+0h] [ebp-12Ch]
    MaterialStateMap *stateMap; // [esp+10h] [ebp-11Ch]
    char filename[260]; // [esp+14h] [ebp-118h] BYREF
    int nameSize; // [esp+11Ch] [ebp-10h]
    int fileSize; // [esp+120h] [ebp-Ch]
    void *file; // [esp+124h] [ebp-8h] BYREF
    char *text; // [esp+128h] [ebp-4h] BYREF

    Com_sprintf(filename, 0x100u, "statemaps/%s.sm", name);
    fileSize = FS_ReadFile(filename, &file);
    if (fileSize >= 0)
    {
        text = (char*)file;
        Com_BeginParseSession(filename);
        Com_SetScriptWarningPrefix("^1ERROR: ");
        Com_SetSpaceDelimited(0);
        v2 = strlen(name);
        nameSize = v2 + 1;
        stateMap = (MaterialStateMap*)Material_Alloc(v2 + 45);
        stateMap->name = (const char*)&stateMap[1]; // (skip past statemap struct and use rest of buffer for name)
        memcpy((void*)stateMap->name, name, nameSize);
        if (!Material_ParseStateMap(&text, stateMap))
            stateMap = 0;
        Com_EndParseSession();
        FS_FreeFile((char*)file);
        return stateMap;
    }
    else
    {
        Com_ScriptError("Couldn't open statemap '%s'\n", filename);
        return 0;
    }
}

void __cdecl Material_SetStateMap(const char *name, MaterialStateMap *stateMap)
{
    unsigned int hashIndex; // [esp+0h] [ebp-4h] BYREF

    Material_HashStateMap(name, &hashIndex);
    mtlLoadGlob.stateMapHashTable[hashIndex] = stateMap;
}

MaterialStateMap *__cdecl Material_RegisterStateMap(char *name)
{
    MaterialStateMap *stateMap; // [esp+0h] [ebp-4h]
    MaterialStateMap *stateMapa; // [esp+0h] [ebp-4h]

    stateMap = Material_FindStateMap(name);
    if (stateMap)
        return stateMap;
    stateMapa = Material_LoadStateMap(name);
    if (!stateMapa)
        return 0;
    Material_SetStateMap(name, stateMapa);
    return stateMapa;
}

bool __cdecl Material_LoadPassStateMap(const char **text, MaterialStateMap **stateMap)
{
    parseInfo_t *token; // [esp+0h] [ebp-4h]

    if (!Material_MatchToken(text, "stateMap"))
        return 0;
    token = Com_Parse(text);
    if (token->token[0] && token->token[0] != 59)
    {
        *stateMap = Material_RegisterStateMap(token->token);
        return *stateMap && Material_MatchToken(text, ";");
    }
    else
    {
        Com_ScriptError("missing stateMap filename\n");
        return 0;
    }
}

unsigned __int8 __cdecl Material_ParseShaderVersion(const char **text)
{
    float versionNumber; // [esp+Ch] [ebp-4h]

    versionNumber = Com_ParseFloat(text);
    return (versionNumber * 10.0 + 0.5);
}

char __cdecl Material_GetVertexShaderHashIndex(
    const char *shaderName,
    GfxRenderer renderer,
    unsigned int *foundHashIndex)
{
    unsigned int hashIndex; // [esp+14h] [ebp-8h]
    MaterialVertexShader **hashTable; // [esp+18h] [ebp-4h]

    iassert( shaderName );
    iassert( foundHashIndex );
    hashTable = mtlLoadGlob.vertexShaderHashTable[renderer];
    for (hashIndex = R_HashAssetName(shaderName) & 0x7FF; hashTable[hashIndex]; hashIndex = (hashIndex + 1) & 0x7FF)
    {
        if (!strcmp(hashTable[hashIndex]->name, shaderName))
        {
            *foundHashIndex = hashIndex;
            return 1;
        }
    }
    *foundHashIndex = hashIndex;
    return 0;
}

void __cdecl Material_GetShaderTargetString(
    char *target,
    unsigned int maxChars,
    const char *prefix,
    int shaderVersion,
    GfxRenderer renderer)
{
    if (renderer)
    {
        if (renderer != GFX_RENDERER_SHADER_3)
            MyAssertHandler(
                ".\\r_material_load_obj.cpp",
                3765,
                0,
                "%s\n\t(renderer) = %i",
                "(renderer == GFX_RENDERER_SHADER_3)",
                renderer);
        Com_sprintf(target, maxChars, "%s_3_0", prefix);
    }
    else
    {
        Com_sprintf(target, maxChars, "%s_%i_%i", prefix, shaderVersion / 10, shaderVersion % 10);
    }
}

int __cdecl Material_MemCopyAndReturnLines(char *destString, const char *srcString, int len)
{
    int i; // [esp+0h] [ebp-8h]
    int iLines; // [esp+4h] [ebp-4h]

    iLines = 0;
    for (i = 0; i < len; ++i)
    {
        destString[i] = srcString[i];
        if (srcString[i] == 10)
            ++iLines;
    }
    return iLines;
}

void __cdecl Material_EmitShaderString(GfxAssembledShaderText *prog, const char *string)
{
    int v2; // [esp+0h] [ebp-14h]

    if (!prog->overflowed)
    {
        v2 = strlen(string);
        if (v2 + prog->used < prog->total)
        {
            prog->currentDestLine += Material_MemCopyAndReturnLines(&prog->string[prog->used], string, v2);
            prog->used += v2;
        }
        else
        {
            Com_ScriptError("Shader text overflowed. You may need to increase R_SHADER_TEXT_SIZE_LIMIT.\n");
            prog->overflowed = 1;
        }
    }
}

void __cdecl Material_AddShaderFile(GfxAssembledShaderText *prog, char *shaderFileName, unsigned int srcLine)
{
    if (prog->fileCount < 0x80)
    {
        KISAK_NULLSUB();
        I_strncpyz(prog->files[prog->fileCount].fileName, shaderFileName, 256);
        prog->files[prog->fileCount].srcLine = srcLine;
        prog->files[prog->fileCount++].destLine = prog->currentDestLine;
    }
    else
    {
        Com_ScriptError("Max shader include files exceeded.  Increase R_SHADER_MAX_INCLUDE_FILES.\n");
    }
}

void __cdecl Material_EmitShaderChar(GfxAssembledShaderText *prog, char ch)
{
    if (prog->overflowed)
    {
        Com_ScriptError("Shader text overflowed. You may need to increase R_SHADER_TEXT_SIZE_LIMIT.\n");
    }
    else
    {
        prog->string[prog->used++] = ch;
        if (ch == 10)
            ++prog->currentDestLine;
        prog->overflowed = prog->used == prog->total;
    }
}

char __cdecl Material_FindCachedShaderText(const char *filename, const char **data, unsigned int *byteCount)
{
    int top; // [esp+0h] [ebp-10h]
    int bot; // [esp+4h] [ebp-Ch]
    int comparison; // [esp+8h] [ebp-8h]
    int mid; // [esp+Ch] [ebp-4h]

    bot = 0;
    top = mtlLoadGlob.cachedShaderCount - 1;
    while (bot <= top)
    {
        mid = (top + bot) >> 1;
        comparison = I_stricmp(filename, mtlLoadGlob.cachedShaderText[mid].name);
        if (!comparison)
        {
            *byteCount = mtlLoadGlob.cachedShaderText[mid].textSize;
            *data = mtlLoadGlob.cachedShaderText[mid].text;
            return 1;
        }
        if (comparison >= 0)
            bot = mid + 1;
        else
            top = mid - 1;
    }
    return 0;
}

bool __cdecl Material_IncludeShader(GfxAssembledShaderText *prog, char *includeName, bool isInLibDir)
{
    char extendedName[64]; // [esp+0h] [ebp-50h] BYREF
    const char *file; // [esp+44h] [ebp-Ch] BYREF
    unsigned int fileSize; // [esp+48h] [ebp-8h] BYREF
    bool hasLibPrefix; // [esp+4Fh] [ebp-1h]

    //hasLibPrefix = strnicmp(includeName, "lib/", 4u) == 0;
    hasLibPrefix = _strnicmp(includeName, "lib/", 4u) == 0;
    if (isInLibDir)
    {
        if (hasLibPrefix)
        {
            Com_ScriptError(
                "Shaders in the shader library folder shouldn't include the library directory when including other shaders: %s\n",
                includeName);
            return 0;
        }
        Com_sprintf(extendedName, 0x40u, "lib/%s", includeName);
        includeName = extendedName;
    }
    else if (!hasLibPrefix)
    {
        Com_ScriptError("Shader is trying to include '%s' instead of 'lib/%s'\n", includeName, includeName);
        return 0;
    }
    if (Material_FindCachedShaderText(includeName, &file, &fileSize))
        return Material_GenerateShaderString_r(prog, includeName, file, fileSize, 1);
    Com_ScriptError("Didn't preload shader file '%s'\n", includeName);
    return 0;
}

bool __cdecl Material_GenerateShaderString_r(
    GfxAssembledShaderText *prog,
    char *shaderName,
    const char *file,
    unsigned int fileSize,
    bool isInLibDir)
{
    bool atStartOfLine; // [esp+3h] [ebp-411h]
    char includeName[1024]; // [esp+4h] [ebp-410h] BYREF
    const char *parse; // [esp+408h] [ebp-Ch]
    int includeNameLen; // [esp+40Ch] [ebp-8h]
    unsigned int includeLine; // [esp+410h] [ebp-4h]

    atStartOfLine = 1;
    parse = file;
    while (*parse)
    {
        if (*parse == 47 && parse[1] == 42)
        {
            Material_EmitShaderChar(prog, 32);
            for (parse += 2; *parse != 42 || parse[1] != 47; ++parse)
            {
                if (!*parse)
                    return 0;
            }
            parse += 2;
        }
        else if (*parse == 47 && parse[1] == 47)
        {
            Material_EmitShaderChar(prog, 10);
            for (parse += 2; *parse != 10; ++parse)
            {
                if (!*parse)
                    return !prog->overflowed;
            }
            ++parse;
            atStartOfLine = 1;
        }
        else if (atStartOfLine && !strncmp(parse, "#include", 8u) && isspace(parse[8]))
        {
            for (parse += 9; isspace(*parse); ++parse)
                ;
            if (*parse != 34)
                Com_Error(ERR_DROP, "Found '%c' instead of '\"' in #include for shader %s\n", *parse, shaderName);
            ++parse;
            for (includeNameLen = 0; parse[includeNameLen] != 34; ++includeNameLen)
            {
                if (parse[includeNameLen] == 10 || !parse[includeNameLen])
                    Com_Error(ERR_DROP, "Missing ending '\"' in #include for shader %s\n", shaderName);
                includeName[includeNameLen] = parse[includeNameLen];
            }
            includeName[includeNameLen] = 0;
            includeLine = prog->files[prog->fileCount - 1].srcLine
                + prog->currentDestLine
                - prog->files[prog->fileCount - 1].destLine;
            Material_AddShaderFile(prog, includeName, 1u);
            if (!Material_IncludeShader(prog, includeName, isInLibDir))
                return 0;
            Material_AddShaderFile(prog, shaderName, includeLine);
            Material_EmitShaderChar(prog, 10);
            for (parse += includeNameLen + 1; *parse != 10; ++parse)
            {
                if (!*parse)
                    return !prog->overflowed;
            }
            ++parse;
            atStartOfLine = 1;
        }
        else
        {
            Material_EmitShaderChar(prog, *parse);
            if (*parse == 10)
            {
                atStartOfLine = 1;
            }
            else if (!isspace(*parse))
            {
                atStartOfLine = 0;
            }
            ++parse;
        }
    }
    return !prog->overflowed;
}

unsigned int __cdecl Material_GenerateShaderString(
    GfxAssembledShaderText *prog,
    char *shaderName,
    MaterialShaderType shaderType,
    char *shaderString,
    unsigned int sizeofShaderString)
{
    bool wasGenerated; // [esp+3h] [ebp-10Dh]
    signed int textSize; // [esp+4h] [ebp-10Ch]
    char filepath[256]; // [esp+8h] [ebp-108h] BYREF
    char *text; // [esp+10Ch] [ebp-4h] BYREF

    Com_sprintf(filepath, 0x100u, "shaders/%s", shaderName);
    textSize = FS_ReadFile(filepath, (void**)&text);
    if (textSize >= 0)
    {
        prog->string = shaderString;
        prog->used = 0;
        prog->currentDestLine = 1;
        prog->fileCount = 0;
        prog->total = sizeofShaderString;
        prog->overflowed = 0;
        Material_EmitShaderString(prog, "#define PC\n");
        if (shaderType)
        {
            Material_EmitShaderString(prog, "#define IS_VERTEX_SHADER 0\n");
            Material_EmitShaderString(prog, "#define IS_PIXEL_SHADER 1\n");
        }
        else
        {
            Material_EmitShaderString(prog, "#define IS_VERTEX_SHADER 1\n");
            Material_EmitShaderString(prog, "#define IS_PIXEL_SHADER 0\n");
        }
        Material_AddShaderFile(prog, shaderName, 1u);
        wasGenerated = Material_GenerateShaderString_r(prog, shaderName, text, textSize, 0);
        FS_FreeFile(text);
        if (wasGenerated)
        {
            if (prog->overflowed)
            {
                return 0;
            }
            else
            {
                prog->string[prog->used] = 0;
                return prog->used;
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_PrintWarning(8, "Couldn't read shader '%s'\n", filepath);
        return 0;
    }
}

void __cdecl Material_DeleteDirectory(const char *dirname)
{
    DWORD errorCode; // [esp+0h] [ebp-360h]
    HANDLE handle; // [esp+4h] [ebp-35Ch]
    _WIN32_FIND_DATAA findData; // [esp+8h] [ebp-358h] BYREF
    char fullfilename[264]; // [esp+148h] [ebp-218h] BYREF
    char fulldirname[268]; // [esp+250h] [ebp-110h] BYREF

    Com_sprintf(fulldirname, 0x104u, "%s/*", dirname);
    handle = FindFirstFileA(fulldirname, &findData);
    if (handle != (HANDLE)-1)
    {
        do
        {
            if ((findData.dwFileAttributes & 0x11) == 0)
            {
                Com_sprintf(fullfilename, 0x104u, "%s/%s", dirname, findData.cFileName);
                if (!DeleteFileA(fullfilename))
                {
                    errorCode = GetLastError();
                    Com_PrintError(1, "ERROR: Failed to delete %s errorCode %d\n", fullfilename, errorCode);
                }
            }
        } while (FindNextFileA(handle, &findData));
        FindClose(handle);
    }
    RemoveDirectoryA(dirname);
}

void __cdecl Material_SubtractDays(_SYSTEMTIME *sysTime, unsigned __int16 daysOld)
{
    iassert( daysOld <= 20 );
    if (sysTime->wDay <= daysOld)
    {
        sysTime->wDay = sysTime->wDay + 28 - daysOld;
        if (sysTime->wMonth <= 1u)
        {
            sysTime->wMonth = 12;
            --sysTime->wYear;
        }
        else
        {
            --sysTime->wMonth;
        }
    }
    else
    {
        sysTime->wDay -= daysOld;
    }
}

void __cdecl Material_DeleteOldFilesInDirectory(const char *dirname, unsigned __int16 daysOld)
{
    DWORD errorCode; // [esp+0h] [ebp-378h]
    HANDLE handle; // [esp+4h] [ebp-374h]
    _WIN32_FIND_DATAA findData; // [esp+8h] [ebp-370h] BYREF
    char fullfilename[264]; // [esp+148h] [ebp-230h] BYREF
    char fulldirname[268]; // [esp+250h] [ebp-128h] BYREF
    _SYSTEMTIME sysTime; // [esp+360h] [ebp-18h] BYREF
    _FILETIME fileTime; // [esp+370h] [ebp-8h] BYREF

    GetSystemTime(&sysTime);
    Material_SubtractDays(&sysTime, daysOld);
    SystemTimeToFileTime(&sysTime, &fileTime);
    Com_sprintf(fulldirname, 0x104u, "%s/*", dirname);
    handle = FindFirstFileA(fulldirname, &findData);
    if (handle != (HANDLE)-1)
    {
        do
        {
            if ((findData.dwFileAttributes & 0x11) == 0 && CompareFileTime(&findData.ftLastAccessTime, &fileTime) < 0)
            {
                Com_sprintf(fullfilename, 0x104u, "%s/%s", dirname, findData.cFileName);
                if (!DeleteFileA(fullfilename))
                {
                    errorCode = GetLastError();
                    Com_PrintError(1, "ERROR: Failed to delete %s errorCode %d\n", fullfilename, errorCode);
                }
            }
        } while (FindNextFileA(handle, &findData));
        FindClose(handle);
    }
}

bool once;
void Material_DeleteOldCachedShaders()
{
    char dirname[264]; // [esp+0h] [ebp-110h] BYREF
    unsigned int oldShaderCacheVersion; // [esp+10Ch] [ebp-4h]

    if (!once)
    {
        once = 1;
        for (oldShaderCacheVersion = 1; oldShaderCacheVersion < 2; ++oldShaderCacheVersion)
        {
            Com_sprintf(dirname, 0x104u, "../shadercache%i", oldShaderCacheVersion);
            Material_DeleteDirectory(dirname);
        }
        Com_sprintf(dirname, 0x104u, "../shadercache%i", 2);
        Material_DeleteOldFilesInDirectory(dirname, 0xAu);
    }
}

char __cdecl Material_FindCachedShader(
    const char *shaderText,
    unsigned int shaderTextLen,
    const char *filename,
    void **cachedShader,
    unsigned int *shaderLen)
{
    unsigned int v6; // eax
    unsigned int cachedShaderTextLen; // [esp+8h] [ebp-Ch] BYREF
    FILE *cacheFile; // [esp+Ch] [ebp-8h]
    char *cachedShaderText; // [esp+10h] [ebp-4h]

    Material_DeleteOldCachedShaders();
    *cachedShader = 0;
    *shaderLen = 0;
    cacheFile = fopen(filename, "rb");

    if (!cacheFile)
        return 0;

    if (fread(&cachedShaderTextLen, 4u, 1u, cacheFile) == 1 && cachedShaderTextLen == shaderTextLen)
    {
        cachedShaderText = (char*)Z_Malloc(cachedShaderTextLen, "Material_FindCachedShader", 31);
        v6 = fread(cachedShaderText, 1u, cachedShaderTextLen, cacheFile);
        if (cachedShaderTextLen == v6
            && !memcmp(cachedShaderText, shaderText, cachedShaderTextLen)
            && fread(shaderLen, 4u, 1u, cacheFile) == 1)
        {
            *cachedShader = Z_Malloc(*shaderLen, "Material_FindCachedShader", 31);
            iassert( *cachedShader );
            fread(*cachedShader, 1u, *shaderLen, cacheFile);
            Z_Free(cachedShaderText, 31);
            fclose(cacheFile);
            return 1;
        }
        else
        {
            Z_Free(cachedShaderText, 31);
            fclose(cacheFile);
            return 0;
        }
    }
    else
    {
        fclose(cacheFile);
        return 0;
    }
}

#ifdef KISAK_NO_FASTFILES
static bool Material_FindCachedShader2(unsigned int *shaderLen, void **cachedShader, const char *filename)
{
    Material_DeleteOldCachedShaders();

    *cachedShader = NULL;

    FILE *cacheFile = fopen(filename, "rb");
    
    if (!cacheFile)
        return false;

    // read in length
    if (fread(shaderLen, 4, 1, cacheFile) != 1)
    {
        fclose(cacheFile);
        return false;
    }

    *cachedShader = Z_Malloc(*shaderLen, "Material_FindCachedShader2", 31);

    iassert(*cachedShader);

    fread(*cachedShader, 1, *shaderLen, cacheFile);
    fclose(cacheFile);

    return true;
}

static bool Material_CopyTextToDXBuffer2(unsigned int shaderHash, ID3DXBuffer **shader, const char *targetprefix)
{
    const char *v3; // eax
    unsigned __int8 *v5; // eax
    int hr; // [esp+0h] [ebp-4h]

    char buffer[260];
    //Com_sprintf(buffer, 260, "%s/raw/shader_bin/%s_%8.8x", fs_basepath->current.string, targetprefix, shaderHash);
    Com_sprintf(buffer, 260, "%s/main/shader_bin/%s_%8.8x", fs_basepath->current.string, targetprefix, shaderHash);

    unsigned int shaderLen;
    void *cachedShader;
    if (!Material_FindCachedShader2(&shaderLen, &cachedShader, buffer))
    {
        return false;
    }

    hr = D3DXCreateBuffer(shaderLen, shader);

    if (hr < 0)
    {
        Com_PrintError(8, "ERROR: Material_CopyTextToDXBuffer: D3DXCreateBuffer(%d) failed: %s (0x%08x)\n", shaderLen, R_ErrorDescription(hr), hr);
        free(cachedShader);
        return false;
    }

    memcpy((*shader)->GetBufferPointer(), cachedShader, shaderLen);
    free(cachedShader);
    return true;
}
#endif

char __cdecl Material_CopyTextToDXBuffer(unsigned __int8 *cachedShader, unsigned int shaderLen, ID3DXBuffer **shader)
{
    const char *v3; // eax
    unsigned __int8 *v5; // eax
    int hr; // [esp+0h] [ebp-4h]

    hr = D3DXCreateBuffer(shaderLen, shader);
    if (hr >= 0)
    {
        v5 = (unsigned char*)(*shader)->GetBufferPointer();
        memcpy(v5, cachedShader, shaderLen);
        return 1;
    }
    else
    {
        v3 = R_ErrorDescription(hr);
        Com_PrintError(
            8,
            "ERROR: Material_CopyTextToDXBuffer: D3DXCreateBuffer(%d) failed: %s (0x%08x)\n",
            shaderLen,
            v3,
            hr);
        free(cachedShader);
        return 0;
    }
}

char __cdecl Material_FindCachedShaderDX(
    const char *shaderText,
    unsigned int shaderTextLen,
    const char *entryPoint,
    const char *target,
    ID3DXBuffer **shader)
{
    unsigned int shaderLen; // [esp+0h] [ebp-11Ch] BYREF
    char filename[268]; // [esp+4h] [ebp-118h] BYREF
    int checksum; // [esp+114h] [ebp-8h]
    void *cachedShader; // [esp+118h] [ebp-4h] BYREF

    checksum = Com_BlockChecksumKey32((const unsigned char*)shaderText, shaderTextLen, 0);
    Com_sprintf(filename, 0x104u, "../shadercache%d/%s_%s_%8.8x", 2, entryPoint, target, checksum);
    if (!Material_FindCachedShader(shaderText, shaderTextLen, filename, &cachedShader, &shaderLen))
        return 0;
    if (Material_CopyTextToDXBuffer((unsigned char*)cachedShader, shaderLen, shader))
    {
        Z_Free(cachedShader, 31);
        return 1;
    }
    else
    {
        Z_Free(cachedShader, 31);
        return 0;
    }
}

bool __cdecl Material_ParseLineNumber(char *errorMessage, unsigned int *lineNumber)
{
    const char *v2; // eax
    const char *lineNumberStart; // [esp+0h] [ebp-8h]
    int charNumber; // [esp+4h] [ebp-4h] BYREF

    v2 = strchr(errorMessage, 0x28u);
    lineNumberStart = v2;
    if (!v2)
        return 0;
    if (sscanf(v2, "(%d,%d)", lineNumber, &charNumber) == 2)
        return 1;
    return sscanf(lineNumberStart, "(%d)", lineNumber) == 1;
}

void __cdecl Material_FileIncludeFileAndLineNumber(
    GfxAssembledShaderText *prog,
    char *errorMessage,
    char **fileName,
    unsigned int *lineNumber)
{
    unsigned int i; // [esp+0h] [ebp-4h]

    if (Material_ParseLineNumber(errorMessage, lineNumber))
    {
        if (*lineNumber < prog->files[0].destLine)
            MyAssertHandler(
                ".\\r_material_load_obj.cpp",
                3604,
                0,
                "*lineNumber >= prog->files[0].destLine\n\t%i, %i",
                *lineNumber,
                prog->files[0].destLine);
        for (i = 1; i < prog->fileCount && *lineNumber >= prog->files[i].destLine; ++i)
            ;
        *fileName = prog->files[i - 1].fileName;
        *lineNumber = prog->files[i - 1].srcLine + *lineNumber - prog->files[i - 1].destLine;
    }
    else
    {
        *fileName = (char*)"Message format changed.  Update Material_FileIncludeFileAndLineNumber.";
        *lineNumber = 0;
    }
}

void __cdecl Material_CacheShader(
    const char *shaderText,
    unsigned int shaderTextLen,
    const char *filename,
    const void *shaderBuffer,
    unsigned int shaderLen)
{
    FILE *cacheFile; // [esp+0h] [ebp-4h]

    cacheFile = fopen(filename, "wb");
    if (cacheFile)
    {
        fwrite(&shaderTextLen, 4u, 1u, cacheFile);
        fwrite(shaderText, 1u, shaderTextLen, cacheFile);
        fwrite(&shaderLen, 4u, 1u, cacheFile);
        fwrite(shaderBuffer, 1u, shaderLen, cacheFile);
        fclose(cacheFile);
    }
    else
    {
        Com_PrintWarning(10, "Material_CacheShader: Failed to open '%s'", filename);
    }
}

void __cdecl Material_CacheShaderDX(
    const char *shaderText,
    unsigned int shaderTextLen,
    const char *entryPoint,
    const char *target,
    ID3DXBuffer *shader)
{
    const void *v5; // eax
    char filename[268]; // [esp+0h] [ebp-220h] BYREF
    int checksum; // [esp+10Ch] [ebp-114h]
    char dirname[268]; // [esp+110h] [ebp-110h] BYREF

    checksum = Com_BlockChecksumKey32((const unsigned char*)shaderText, shaderTextLen, 0);
    Com_sprintf(dirname, 0x104u, "../shadercache%d", 2);
    Sys_Mkdir(dirname);
    Com_sprintf(filename, 0x104u, "%s/%s_%s_%8.8x", dirname, entryPoint, target, checksum);
    Material_CacheShader(shaderText, shaderTextLen, filename, shader->GetBufferPointer(), shader->GetBufferSize());
}

ID3DXBuffer *__cdecl Material_CompileShader(
    char *shaderName,
    MaterialShaderType shaderType,
    char *entryPoint,
    char *target)
{
    char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // [esp-10h] [ebp-8498h]
    char **v9; // [esp-Ch] [ebp-8494h]
    int v10; // [esp-Ch] [ebp-8494h]
    int v11; // [esp-8h] [ebp-8490h]
    unsigned int lineNumber; // [esp+0h] [ebp-8488h] BYREF
    int v13; // [esp+4h] [ebp-8484h] BYREF
    char dest[68]; // [esp+8h] [ebp-8480h] BYREF
    unsigned int shaderTextLen; // [esp+4Ch] [ebp-843Ch]
    ID3DXConstantTable *v16; // [esp+50h] [ebp-8438h] BYREF
    HRESULT hr; // [esp+54h] [ebp-8434h]
    GfxAssembledShaderText prog; // [esp+58h] [ebp-8430h] BYREF
    char *shaderString; // [esp+8478h] [ebp-10h]
    LPD3DXBUFFER fileName; // [esp+847Ch] [ebp-Ch] BYREF
    ID3DXBuffer *shader[2]; // [esp+8480h] [ebp-8h] BYREF

    Com_sprintf(dest, 0x40u, "shaders/%s", shaderName);
    shaderString = (char*)Hunk_AllocateTempMemory(0x10000, "Material_CompileShader");
    shaderTextLen = Material_GenerateShaderString(&prog, shaderName, shaderType, shaderString, 0x10000u);
    if (!shaderTextLen)
    {
        Hunk_FreeTempMemory(shaderString);
        return 0;
    }
    if (Material_FindCachedShaderDX(shaderString, shaderTextLen, entryPoint, target, shader))
    {
    LABEL_15:
        Hunk_FreeTempMemory(shaderString);
        return shader[0];
    }
    shader[1] = 0;
    hr = D3DXCompileShader(shaderString, shaderTextLen, 0, 0, entryPoint, target, 0, shader, &fileName, &v16);
    // __asm { fnclex }
    // KISAKTODO: cancerous error handle
    //if (fileName)
    //{
    //    v9 = fileName;
    //    v5 = (*(*fileName + 3))(fileName);
    //    Material_FileIncludeFileAndLineNumber(&prog, v5, v9, &lineNumber);
    //    v6 = (*(*fileName + 3))(fileName, lineNumber, v13);
    //    Com_ScriptError(
    //        "compiler message(s) for %s (entryPoint=\"%s\", target=\"%s\"):\n%s\n%s(%d)\n",
    //        dest,
    //        entryPoint,
    //        target,
    //        v6,
    //        v8,
    //        v10);
    //    (*(*fileName + 2))(fileName, v11, &v13);
    //}
    if (hr < 0)
    {
        v7 = R_ErrorDescription(hr);
        Com_ScriptError("%s compilation failed - %s\n", dest, v7);
        Hunk_FreeTempMemory(shaderString);
        return 0;
    }
    if (shader[0])
    {
        if (!v16)
        {
            shader[0]->Release();
            Com_ScriptError("%s compilation failed - NULL constants\n", dest);
            Hunk_FreeTempMemory(shaderString);
            return 0;
        }
        v16->Release();
        Material_CacheShaderDX(shaderString, shaderTextLen, entryPoint, target, shader[0]);
        goto LABEL_15;
    }
    if (v16)
        v16->Release();
    Com_ScriptError("%s compilation failed - NULL shader\n", dest);
    Hunk_FreeTempMemory(shaderString);
    return 0;
}

#ifdef KISAK_NO_FASTFILES
static int GetHashedFilename(int shaderType, const char *shaderName)
{
    iassert(shaderType == MTL_PIXEL_SHADER || shaderType == MTL_VERTEX_SHADER); // lwss add

    ShaderBinNames *pList = shaderType ? g_pixelNamesList : g_vertexNamesList;
    int listSize = shaderType ? g_pixelNamesCount : g_vertexNamesCount;

    unsigned int hash = R_HashAssetName(shaderName);

    int itr = 0;
    int end = listSize - 1;

    if (end < 0)
        return 0;

    int v7;
    unsigned int key;

    while (1)
    {
        v7 = (end + itr) >> 1;
        //key = *(_DWORD *)(pList + 8 * v7);
        key = pList[v7].key;
        if (key <= hash)
            break;
        end = v7 - 1;
    LABEL_11:
        if (itr > end)
            return 0;
    }
    if (key < hash)
    {
        itr = v7 + 1;
        goto LABEL_11;
    }
    //return *(_DWORD *)(pList + 8 * v7 + 4);
    return pList[v7].val;
}
#endif

MaterialVertexShader *__cdecl Material_LoadVertexShader(char *shaderName, int shaderVersion, GfxRenderer renderer)
{
    unsigned int programSize; // [esp+10h] [ebp-34h]
    int hr; // [esp+18h] [ebp-2Ch]
    char target[16]; // [esp+1Ch] [ebp-28h] BYREF
    unsigned int *program; // [esp+30h] [ebp-14h]
    unsigned int nameSize; // [esp+34h] [ebp-10h]
    ID3DXBuffer *shader = NULL; // [esp+38h] [ebp-Ch]
    unsigned int totalSize; // [esp+3Ch] [ebp-8h]
    MaterialVertexShader *mtlShader; // [esp+40h] [ebp-4h]

    Material_GetShaderTargetString(target, 0x10u, "vs", shaderVersion, renderer);

    // We are missing the shader/ folder with hlsl, (see Material_PreLoadAllShaderText())
#ifdef KISAK_NO_FASTFILES
    int hashedName = GetHashedFilename(MTL_VERTEX_SHADER, shaderName);

    if (!hashedName || !Material_CopyTextToDXBuffer2(hashedName, &shader, target))
    {
        Com_Error(ERR_DROP, "Can't find shader: shader_bin/%s_%8.8x\n", target, hashedName);
        return 0;
    }
#else
    shader = Material_CompileShader(shaderName, MTL_VERTEX_SHADER, (char *)"vs_main", target);
#endif

    if (!shader)
        return 0;

    programSize = shader->GetBufferSize();
    iassert( (programSize > 0) );
    nameSize = strlen(shaderName) + 1;
    totalSize = sizeof(MaterialVertexShader) + programSize + nameSize;
    mtlShader = (MaterialVertexShader*)Material_Alloc(totalSize);
    program = (uint*)&mtlShader[1];
    mtlShader->name = (const char*)&mtlShader[1] + programSize;
    memcpy((void*)mtlShader->name, shaderName, nameSize);
    memcpy(program, shader->GetBufferPointer(), programSize);
    hr = dx.device->CreateVertexShader((const DWORD*)program, &mtlShader->prog.vs);
    if (hr >= 0)
    {
        mtlShader->prog.loadDef.loadForRenderer = renderer;
        mtlShader->prog.loadDef.programSize = programSize >> 2;
        //iassert(mtlShader->prog.loadDef.programSize * sizeof(mtlShader->prog.loadDef.program[0]) == programSize);
        iassert(mtlShader->prog.loadDef.programSize * 4 == programSize);

        mtlShader->prog.loadDef.program = program;
        shader->Release();
        return mtlShader;
    }
    else
    {
        Com_ScriptError("vertex shader creation failed for %s: %s\n", shaderName, R_ErrorDescription(hr));
        return 0;
    }
}

MaterialVertexShader *__cdecl Material_RegisterVertexShader(
    char *shaderName,
    unsigned __int8 shaderVersion,
    GfxRenderer renderer)
{
    unsigned int hashIndex; // [esp+0h] [ebp-Ch] BYREF
    MaterialVertexShader *mtlShader; // [esp+8h] [ebp-4h]

    if (Material_GetVertexShaderHashIndex(shaderName, renderer, &hashIndex))
        return mtlLoadGlob.vertexShaderHashTable[renderer][hashIndex];
    ProfLoad_Begin("Load vertex shader");
    mtlShader = Material_LoadVertexShader(shaderName, shaderVersion, renderer);
    ProfLoad_End();
    if (mtlShader)
    {
        mtlLoadGlob.vertexShaderHashTable[renderer][hashIndex] = mtlShader;
        if (++mtlLoadGlob.vertexShaderCount == 2048)
            Com_Error(ERR_DROP, "More than %i unique vertex shaders", 2047);
    }
    return mtlShader;
}

char __cdecl Material_LoadPassVertexShader(
    const char **text,
    GfxRenderer renderer,
    unsigned __int16 *techFlags,
    ShaderParameterSet *paramSet,
    MaterialPass *pass,
    unsigned int argLimit,
    unsigned int *argCount,
    MaterialShaderArgument *args)
{
    unsigned __int8 shaderVersion; // [esp+3h] [ebp-Dh]
    char *shaderName; // [esp+8h] [ebp-8h]
    MaterialVertexShader *mtlShader; // [esp+Ch] [ebp-4h]

    memset(paramSet, 0, sizeof(ShaderParameterSet));
    if (!Material_MatchToken(text, "vertexShader"))
        return 0;
    shaderVersion = Material_ParseShaderVersion(text);
    shaderName = Com_Parse(text)->token;
    mtlShader = Material_RegisterVertexShader(shaderName, shaderVersion, renderer);
    if (!mtlShader)
        return 0;
    pass->vertexShader = mtlShader;
    return Material_SetPassShaderArguments_DX(
        text,
        mtlShader->name,
        MTL_VERTEX_SHADER,
        (uint*)&mtlShader[1],
        techFlags,
        paramSet,
        argLimit,
        argCount,
        args);
}

int __cdecl Material_GetArgUpdateFrequency(const MaterialShaderArgument *arg)
{
    int type; // [esp+0h] [ebp-8h]
    MaterialUpdateFrequency updateFreq; // [esp+4h] [ebp-4h]
    MaterialUpdateFrequency updateFreqa; // [esp+4h] [ebp-4h]

    iassert( arg );
    type = arg->type;
    switch (type)
    {
    case 3:
        return s_codeConstUpdateFreq[arg->u.codeConst.index];
    case 4:
        updateFreqa = (MaterialUpdateFrequency)s_codeSamplerUpdateFreq[arg->u.codeSampler];
        if (updateFreqa != MTL_UPDATE_PER_OBJECT && updateFreqa != MTL_UPDATE_RARELY && updateFreqa != MTL_UPDATE_CUSTOM)
            MyAssertHandler(
                ".\\r_material_load_obj.cpp",
                5038,
                0,
                "%s\n\t(arg->u.codeSampler) = %i",
                "((updateFreq == MTL_UPDATE_PER_OBJECT) || (updateFreq == MTL_UPDATE_RARELY) || (updateFreq == MTL_UPDATE_CUSTOM))",
                arg->u.codeSampler);
        return updateFreqa;
    case 5:
        updateFreq = (MaterialUpdateFrequency)s_codeConstUpdateFreq[arg->u.codeConst.index];
        if (updateFreq != MTL_UPDATE_RARELY)
            MyAssertHandler(
                ".\\r_material_load_obj.cpp",
                5033,
                0,
                "%s\n\t(arg->u.codeConst.index) = %i",
                "(updateFreq == MTL_UPDATE_RARELY)",
                arg->u.codeConst.index);
        return updateFreq;
    default:
        return 2;
    }
}

unsigned __int8 __cdecl Material_CountArgsWithUpdateFrequency(
    MaterialUpdateFrequency updateFreq,
    const MaterialShaderArgument *args,
    unsigned int argCount,
    unsigned int *firstArg)
{
    unsigned int matchCount; // [esp+0h] [ebp-4h]
    const MaterialShaderArgument *argsa; // [esp+10h] [ebp+Ch]
    unsigned int argCounta; // [esp+14h] [ebp+10h]

    argsa = &args[*firstArg];
    argCounta = argCount - *firstArg;
    for (matchCount = 0;
        matchCount < argCounta && Material_GetArgUpdateFrequency(&argsa[matchCount]) == updateFreq;
        ++matchCount)
    {
        ;
    }
    *firstArg += matchCount;
    return matchCount;
}

char __cdecl Material_GetPixelShaderHashIndex(
    const char *shaderName,
    GfxRenderer renderer,
    unsigned int *foundHashIndex)
{
    unsigned int hashIndex; // [esp+14h] [ebp-8h]
    MaterialPixelShader **hashTable; // [esp+18h] [ebp-4h]

    iassert( shaderName );
    iassert( foundHashIndex );
    hashTable = mtlLoadGlob.pixelShaderHashTable[renderer];
    for (hashIndex = R_HashAssetName(shaderName) & 0x7FF; hashTable[hashIndex]; hashIndex = (hashIndex + 1) & 0x7FF)
    {
        if (!strcmp(hashTable[hashIndex]->name, shaderName))
        {
            *foundHashIndex = hashIndex;
            return 1;
        }
    }
    *foundHashIndex = hashIndex;
    return 0;
}

MaterialPixelShader *__cdecl Material_LoadPixelShader(char *shaderName, int shaderVersion, GfxRenderer renderer)
{
    unsigned int programSize; // [esp+10h] [ebp-34h]
    int hr; // [esp+18h] [ebp-2Ch]
    char target[16]; // [esp+1Ch] [ebp-28h] BYREF
    unsigned int *program; // [esp+30h] [ebp-14h]
    unsigned int nameSize; // [esp+34h] [ebp-10h]
    ID3DXBuffer *shader = NULL; // [esp+38h] [ebp-Ch]
    unsigned int totalSize; // [esp+3Ch] [ebp-8h]
    MaterialPixelShader *mtlShader; // [esp+40h] [ebp-4h]

    Material_GetShaderTargetString(target, 0x10u, "ps", shaderVersion, renderer);

#ifdef KISAK_NO_FASTFILES
    int hashedName = GetHashedFilename(MTL_PIXEL_SHADER, shaderName);

    if (!hashedName || !Material_CopyTextToDXBuffer2(hashedName, &shader, target))
    {
        Com_Error(ERR_DROP, "Can't find shader: shader_bin/%s_%8.8x\n", target, hashedName);
        return 0;
    }
#else
    shader = Material_CompileShader(shaderName, MTL_PIXEL_SHADER, (char*)"ps_main", target);
#endif

    if (!shader)
        return 0;

    programSize = shader->GetBufferSize();
    iassert( (programSize > 0) );
    nameSize = strlen(shaderName) + 1;
    totalSize = sizeof(MaterialPixelShader) + programSize + nameSize;
    mtlShader = (MaterialPixelShader*)Material_Alloc(totalSize);
    program = (unsigned int *)&mtlShader[1];
    mtlShader->name = (const char*)&mtlShader[1] + programSize;
    memcpy((void*)mtlShader->name, shaderName, nameSize);
    memcpy(program, shader->GetBufferPointer(), programSize);

    hr = dx.device->CreatePixelShader((const DWORD*)program, &mtlShader->prog.ps);

    if (hr >= 0)
    {
        mtlShader->prog.loadDef.loadForRenderer = renderer;
        mtlShader->prog.loadDef.programSize = programSize >> 2;

        //iassert(mtlShader->prog.loadDef.programSize * sizeof(mtlShader->prog.loadDef.program[0]) == programSize);
        iassert(mtlShader->prog.loadDef.programSize * 4 == programSize);

        mtlShader->prog.loadDef.program = program;
        shader->Release();
        return mtlShader;
    }
    else
    {
        Com_ScriptError("pixel shader creation failed for %s: %s\n", shaderName, R_ErrorDescription(hr));
        return 0;
    }
}

MaterialPixelShader *__cdecl Material_RegisterPixelShader(
    char *shaderName,
    unsigned __int8 shaderVersion,
    GfxRenderer renderer)
{
    unsigned int hashIndex; // [esp+0h] [ebp-Ch] BYREF
    MaterialPixelShader *mtlShader; // [esp+8h] [ebp-4h]

    if (Material_GetPixelShaderHashIndex(shaderName, renderer, &hashIndex))
        return mtlLoadGlob.pixelShaderHashTable[renderer][hashIndex];
    ProfLoad_Begin("Load pixel shader");
    mtlShader = Material_LoadPixelShader(shaderName, shaderVersion, renderer);
    ProfLoad_End();
    if (mtlShader)
    {
        mtlLoadGlob.pixelShaderHashTable[renderer][hashIndex] = mtlShader;
        if (++mtlLoadGlob.pixelShaderCount == 2048)
            Com_Error(ERR_DROP, "More than %i unique pixel shaders", 2047);
    }
    return mtlShader;
}

char *__cdecl BufferOffset(char *buffer, int offset)
{
    return &buffer[offset];
}

unsigned int __cdecl R_SetParameterDefArray(
    _D3DXSHADER_CONSTANTTABLE *constantTable,
    unsigned int constantIndex,
    ShaderUniformDef *paramDef)
{
    unsigned int result; // eax
    char *typeInfo; // [esp+4h] [ebp-18h]
    char *name; // [esp+8h] [ebp-14h]
    unsigned int paramDefIndex; // [esp+Ch] [ebp-10h]
    bool isTransposed; // [esp+13h] [ebp-9h]
    const _D3DXSHADER_CONSTANTINFO *constantInfo; // [esp+14h] [ebp-8h]
    ShaderParamType type; // [esp+18h] [ebp-4h]

    constantInfo = (const _D3DXSHADER_CONSTANTINFO*)&BufferOffset((char*)constantTable, constantTable->ConstantInfo)[20 * constantIndex];
    typeInfo = BufferOffset((char *)constantTable, constantInfo->TypeInfo);
    name = BufferOffset((char *)constantTable, constantInfo->Name);
    isTransposed = *(_WORD *)typeInfo == 3;
    switch (*((_WORD *)typeInfo + 1))
    {
    case 3:
        type = SHADER_PARAM_FLOAT4;
        goto LABEL_7;
    case 0xC:
        type = SHADER_PARAM_SAMPLER_2D;
        goto LABEL_7;
    case 0xD:
        type = SHADER_PARAM_SAMPLER_3D;
        goto LABEL_7;
    case 0xE:
        type = SHADER_PARAM_SAMPLER_CUBE;
    LABEL_7:
        for (paramDefIndex = 0; paramDefIndex < constantInfo->RegisterCount; ++paramDefIndex)
        {
            paramDef->type = type;
            paramDef->name = name;
            paramDef->index = paramDefIndex;
            paramDef->resourceDest = paramDefIndex + constantInfo->RegisterIndex;
            paramDef->isTransposed = isTransposed;
            paramDef->isAssigned = 0;
            ++paramDef;
        }
        result = paramDefIndex;
        break;
    default:
        iassert(0); // lwss add
        Com_ScriptError("Unknown constant type '%i'", *((_WORD *)typeInfo + 1));
        result = 0;
        break;
    }
    return result;
}

unsigned int __cdecl Material_PrepareToParseShaderArguments(
    _D3DXSHADER_CONSTANTTABLE *constantTable,
    ShaderUniformDef *paramTable)
{
    unsigned int constantIndex; // [esp+0h] [ebp-8h]
    unsigned int usedCount; // [esp+4h] [ebp-4h]

    usedCount = 0;
    for (constantIndex = 0; constantIndex < constantTable->Constants; ++constantIndex)
        usedCount += R_SetParameterDefArray(constantTable, constantIndex, &paramTable[usedCount]);
    return usedCount;
}

int __cdecl Material_CompareShaderArgumentsForCombining(unsigned __int16 *e0, unsigned __int16 *e1)
{
    int v3; // [esp+0h] [ebp-18h]
    int v4; // [esp+4h] [ebp-14h]

    v4 = *e0 == 4 || *e0 == 2;
    v3 = *e1 == 4 || *e1 == 2;
    if (v4 == v3)
        return e0[1] - e1[1];
    else
        return v4 - v3;
}

char __cdecl Material_AttemptCombineShaderArguments(MaterialShaderArgument *arg0, const MaterialShaderArgument *arg1)
{
    if (arg0->type != arg1->type)
        return 0;
    if (arg0->type != 3 && arg0->type != 5)
        return 0;
    if (arg0->u.codeConst.rowCount + arg0->dest != arg1->dest)
        return 0;
    if (arg0->u.codeConst.index < 0x3Au)
        return 0;
    if (arg0->u.codeConst.index != arg1->u.codeConst.index)
        return 0;
    if (arg0->u.codeConst.rowCount + arg0->u.codeConst.firstRow != arg1->u.codeConst.firstRow)
        return 0;
    if (arg1->u.codeConst.rowCount + arg0->u.codeConst.rowCount + arg0->u.codeConst.firstRow < 2
        || arg1->u.codeConst.rowCount + arg0->u.codeConst.rowCount + arg0->u.codeConst.firstRow > 4)
    {
        MyAssertHandler(
            ".\\r_material_load_obj.cpp",
            2971,
            0,
            "arg0->u.codeConst.firstRow + arg0->u.codeConst.rowCount + arg1->u.codeConst.rowCount not in [2, 4]\n"
            "\t%i not in [%i, %i]",
            arg1->u.codeConst.rowCount + arg0->u.codeConst.rowCount + arg0->u.codeConst.firstRow,
            2,
            4);
    }
    arg0->u.codeConst.rowCount += arg1->u.codeConst.rowCount;
    return 1;
}

unsigned int __cdecl Material_CombineShaderArguments(unsigned int usedCount, MaterialShaderArgument *localArgs)
{
    MaterialArgumentDef v2; // ecx
    unsigned int srcIndex; // [esp+4h] [ebp-8h]
    unsigned int dstIndex; // [esp+8h] [ebp-4h]

    dstIndex = 0;
    for (srcIndex = 1; srcIndex < usedCount; ++srcIndex)
    {
        if (!Material_AttemptCombineShaderArguments(&localArgs[dstIndex], &localArgs[srcIndex]))
        {
            ++dstIndex;
            v2.nameHash = localArgs[srcIndex].u.nameHash;
            //*&localArgs[dstIndex].type = *&localArgs[srcIndex].type;
            localArgs[dstIndex].type = localArgs[srcIndex].type;
            localArgs[dstIndex].dest = localArgs[srcIndex].dest;
            localArgs[dstIndex].u = v2;
        }
    }
    return dstIndex + 1;
}

char __cdecl Material_SetShaderArguments(
    unsigned int usedCount,
    MaterialShaderArgument *localArgs,
    unsigned int argLimit,
    unsigned int *argCount,
    MaterialShaderArgument *args)
{
    unsigned int usedCounta; // [esp+8h] [ebp+8h]

    iassert( args );
    iassert( argCount );

    if (!usedCount)
        return 1;

    if (*argCount + usedCount <= argLimit)
    {
        qsort(localArgs, usedCount, 8u, (int(*)(const void*, const void*))Material_CompareShaderArgumentsForCombining);
        usedCounta = Material_CombineShaderArguments(usedCount, localArgs);
        memcpy(&args[*argCount], localArgs, 8 * usedCounta);
        *argCount += usedCounta;
        return 1;
    }
    else
    {
        Com_ScriptError("more than %i total shader arguments", argLimit);
        return 0;
    }
}

char __cdecl Material_DefaultIndexRange(
    const ShaderIndexRange *indexRangeRef,
    unsigned int arrayCount,
    ShaderIndexRange *indexRangeSet)
{
    if (arrayCount)
    {
        if (indexRangeRef->count + indexRangeRef->first > arrayCount)
            return 0;
    }
    else if (indexRangeRef->first || indexRangeRef->count != 1)
    {
        return 0;
    }
    indexRangeSet->first = indexRangeRef->first;
    indexRangeSet->count = indexRangeRef->count;
    indexRangeSet->isImplicit = 0;
    return 1;
}

char __cdecl Material_DefaultConstantSourceFromTable(
    MaterialShaderType shaderType,
    const char *constantName,
    const ShaderIndexRange *indexRange,
    const CodeConstantSource *sourceTable,
    ShaderArgumentSource *argSource)
{
    char v5; // al
    unsigned int sourceIndex; // [esp+1Ch] [ebp-8h]

    for (sourceIndex = 0; ; ++sourceIndex)
    {
        if (!sourceTable[sourceIndex].name)
            return 0;
        if (!sourceTable[sourceIndex].subtable && !strcmp(constantName, sourceTable[sourceIndex].name))
        {
            if (sourceTable[sourceIndex].source < 0x3Au)
            {
                v5 = sourceTable[sourceIndex].arrayCount > 1
                    ? Material_DefaultIndexRange(indexRange, sourceTable[sourceIndex].arrayCount, &argSource->indexRange)
                    : Material_DefaultIndexRange(indexRange, 1u, &argSource->indexRange);
            }
            else
            {
                if (sourceTable[sourceIndex].arrayCount)
                    MyAssertHandler(
                        ".\\r_material_load_obj.cpp",
                        2765,
                        0,
                        "%s\n\t(constantName) = %s",
                        "(sourceTable[sourceIndex].arrayCount == 0)",
                        constantName);
                v5 = Material_DefaultIndexRange(indexRange, 4u, &argSource->indexRange);
            }
            if (v5)
                break;
        }
    }
    argSource->type = 2 * (shaderType != MTL_VERTEX_SHADER) + 3;
    argSource->u.codeIndex = sourceTable[sourceIndex].source;
    if (argSource->type != 3 && !s_codeConstUpdateFreq[argSource->u.codeIndex])
        MyAssertHandler(
            ".\\r_material_load_obj.cpp",
            2777,
            0,
            "%s\n\t(argSource->u.codeIndex) = %i",
            "((argSource->type == MTL_ARG_CODE_VERTEX_CONST) || s_codeConstUpdateFreq[argSource->u.codeIndex] != MTL_UPDATE_PER_PRIM)",
            argSource->u.codeIndex);
    return 1;
}


bool __cdecl Material_DefaultConstantSource(
    MaterialShaderType shaderType,
    const char *constantName,
    const ShaderIndexRange *indexRange,
    ShaderArgumentSource *argSource)
{
    return Material_DefaultConstantSourceFromTable(shaderType, constantName, indexRange, s_codeConsts, argSource)
        || Material_DefaultConstantSourceFromTable(shaderType, constantName, indexRange, s_defaultCodeConsts, argSource) != 0;
}

char __cdecl Material_DefaultSamplerSourceFromTable(
    const char *constantName,
    const ShaderIndexRange *indexRange,
    const CodeSamplerSource *sourceTable,
    ShaderArgumentSource *argSource)
{
    int sourceIndex; // [esp+14h] [ebp-4h]

    iassert( constantName );
    iassert( sourceTable );
    iassert( indexRange );
    iassert( argSource );
    for (sourceIndex = 0; sourceTable[sourceIndex].name; ++sourceIndex)
    {
        if (!sourceTable[sourceIndex].subtable
            && !strcmp(constantName, sourceTable[sourceIndex].name)
            && Material_DefaultIndexRange(indexRange, sourceTable[sourceIndex].arrayCount, &argSource->indexRange))
        {
            argSource->type = 4;
            argSource->u.codeIndex = sourceTable[sourceIndex].source;
            return 1;
        }
    }
    return 0;
}

bool __cdecl Material_DefaultSamplerSource(
    const char *constantName,
    const ShaderIndexRange *indexRange,
    ShaderArgumentSource *argSource)
{
    return Material_DefaultSamplerSourceFromTable(constantName, indexRange, s_defaultCodeSamplers, argSource) != 0;
}

bool __cdecl Material_DefaultArgumentSource(
    MaterialShaderType shaderType,
    const char *constantName,
    ShaderParamType paramType,
    const ShaderIndexRange *indexRange,
    ShaderArgumentSource *argSource)
{
    iassert( constantName );
    iassert( argSource );
    if (paramType == SHADER_PARAM_FLOAT4)
        return Material_DefaultConstantSource(shaderType, constantName, indexRange, argSource);
    if (paramType > SHADER_PARAM_FLOAT4 && paramType <= SHADER_PARAM_SAMPLER_CUBE)
        return Material_DefaultSamplerSource(constantName, indexRange, argSource);
    return 0;
}

ShaderUniformDef *__cdecl Material_GetShaderArgumentDest(
    const char *paramName,
    unsigned int paramIndex,
    ShaderUniformDef *paramTable,
    unsigned int paramCount)
{
    unsigned int tableIndex; // [esp+14h] [ebp-4h]

    for (tableIndex = 0; tableIndex < paramCount; ++tableIndex)
    {
        if (paramTable[tableIndex].index == paramIndex && !strcmp(paramTable[tableIndex].name, paramName))
        {
            if (paramTable[tableIndex].isAssigned)
            {
                Com_ScriptError("parameter %s index %i already assigned", paramName, paramIndex);
                return 0;
            }
            else
            {
                paramTable[tableIndex].isAssigned = 1;
                return &paramTable[tableIndex];
            }
        }
    }
    if (!alwaysfails)
        MyAssertHandler(".\\r_material_load_obj.cpp", 3036, 1, "unfound name should be caught earlier");
    return 0;
}

char __cdecl MaterialAddShaderArgument(
    const char *shaderName,
    char *paramName,
    MaterialShaderArgument *arg,
    char (*registerUsage)[64])
{
    if (arg->type > 1u && arg->type != 3)
        return 1;
    if (arg->dest < 0x20u)
    {
        if ((*registerUsage)[64 * arg->dest])
        {
            Com_ScriptError(
                "Vertex register collision at index %d in '%s' between '%s' and '%s'\n",
                arg->dest,
                shaderName,
                &(*registerUsage)[64 * arg->dest],
                paramName);
            return 0;
        }
        else
        {
            I_strncpyz(&(*registerUsage)[64 * arg->dest], paramName, 64);
            return 1;
        }
    }
    else
    {
        Com_ScriptError("Invalid vertex register index %d in '%s' for '%s'\n", arg->dest, shaderName, paramName);
        return 0;
    }
}

char __cdecl Material_AddShaderArgumentFromMaterial(
    const char *shaderName,
    char *paramName,
    unsigned __int16 type,
    char *name,
    ShaderUniformDef *dest,
    MaterialShaderArgument *arg,
    char (*registerUsage)[64])
{
    Material_RegisterString(name);
    arg->type = type;
    arg->dest = dest->resourceDest;
    if (type == 6 && arg->dest >= 0x100u)
        MyAssertHandler(
            ".\\r_material_load_obj.cpp",
            3141,
            0,
            "arg->dest doesn't index R_MAX_PIXEL_SHADER_CONSTS\n\t%i not in [0, %i)",
            arg->dest,
            256);
    arg->u.codeSampler = (MaterialTextureSource)R_HashString(name);
    return MaterialAddShaderArgument(shaderName, paramName, arg, registerUsage);
}

char __cdecl Material_AddShaderArgumentFromLiteral(
    const char *shaderName,
    char *paramName,
    unsigned __int16 type,
    const float *literal,
    ShaderUniformDef *dest,
    MaterialShaderArgument *arg,
    char (*registerUsage)[64])
{
    arg->type = type;
    arg->dest = dest->resourceDest;
    if (type == 7 && arg->dest >= 0x100u)
        MyAssertHandler(
            ".\\r_material_load_obj.cpp",
            3082,
            0,
            "arg->dest doesn't index R_MAX_PIXEL_SHADER_CONSTS\n\t%i not in [0, %i)",
            arg->dest,
            256);
    arg->u.literalConst = literal;
    return MaterialAddShaderArgument(shaderName, paramName, arg, registerUsage);
}

void __cdecl Material_AddShaderArgumentFromCodeSampler(
    unsigned __int16 type,
    MaterialTextureSource codeSampler,
    ShaderUniformDef *dest,
    MaterialShaderArgument *arg)
{
    arg->type = type;
    arg->dest = dest->resourceDest;
    arg->u.codeSampler = codeSampler;
}

char __cdecl Material_AddShaderArgumentFromCodeConst(
    const char *shaderName,
    char *paramName,
    unsigned __int16 type,
    unsigned int codeIndex,
    __int16 offset,
    ShaderUniformDef *dest,
    MaterialShaderArgument *arg,
    char (*registerUsage)[64])
{
    arg->type = type;
    arg->dest = dest->resourceDest;
    if (type == 5 && arg->dest >= 0x100u)
        MyAssertHandler(
            ".\\r_material_load_obj.cpp",
            3100,
            0,
            "arg->dest doesn't index R_MAX_PIXEL_SHADER_CONSTS\n\t%i not in [0, %i)",
            arg->dest,
            256);
    if (codeIndex < 0x3A)
    {
        arg->u.codeConst.index = offset + codeIndex;
        arg->u.codeConst.firstRow = 0;
    }
    else
    {
        if (dest->isTransposed)
            arg->u.codeConst.index = ((codeIndex - 58) ^ 2) + 58;
        else
            arg->u.codeConst.index = codeIndex;
        arg->u.codeConst.firstRow = offset;
    }
    arg->u.codeConst.rowCount = 1;
    return MaterialAddShaderArgument(shaderName, paramName, arg, registerUsage);
}

bool __cdecl Material_AddShaderArgument(
    const char *shaderName,
    MaterialShaderType shaderType,
    ShaderArgumentSource *argSource,
    const ShaderArgumentDest *argDest,
    ShaderUniformDef *paramTable,
    unsigned int paramCount,
    unsigned int *usedCount,
    MaterialShaderArgument *argTable,
    char (*registerUsage)[64])
{
    bool result; // al
    const char *v10; // eax
    ShaderUniformDef *dest; // [esp+4h] [ebp-8h]
    ShaderUniformDef *desta; // [esp+4h] [ebp-8h]
    ShaderUniformDef *destb; // [esp+4h] [ebp-8h]
    ShaderUniformDef *destc; // [esp+4h] [ebp-8h]
    unsigned int indexOffset; // [esp+8h] [ebp-4h]
    unsigned int indexOffseta; // [esp+8h] [ebp-4h]

    if (argSource->indexRange.isImplicit)
    {
        iassert( argSource->indexRange.first == 0 );
        if (argDest->indexRange.count > argSource->indexRange.count)
        {
            Com_ScriptError(
                "The destination needs %i entries, but the source can only provide %i",
                argDest->indexRange.count,
                argSource->indexRange.count);
            return 0;
        }
        argSource->indexRange.count = argDest->indexRange.count;
    }
    else if (argDest->indexRange.count != argSource->indexRange.count)
    {
        Com_ScriptError(
            "The destination needs %i entries, but the source provides %i",
            argDest->indexRange.count,
            argSource->indexRange.count);
        return 0;
    }
    switch (argSource->type)
    {
    case 0u:
    case 2u:
    case 6u:
        if (argDest->indexRange.count == 1)
        {
            iassert( argSource->indexRange.first == 0 );
            iassert( argSource->indexRange.count == 1 );
            destc = Material_GetShaderArgumentDest(argDest->paramName, argDest->indexRange.first, paramTable, paramCount);
            if (destc)
            {
                if (Material_AddShaderArgumentFromMaterial(
                    shaderName,
                    (char *)argDest->paramName,
                    argSource->type,
                    (char *)argSource->u.literalConst,
                    destc,
                    &argTable[*usedCount],
                    registerUsage))
                {
                    ++*usedCount;
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else
            {
                result = 0;
            }
        }
        else
        {
            Com_ScriptError("Must assign material values one at a time");
            result = 0;
        }
        break;
    case 1u:
    case 7u:
        if (argDest->indexRange.count == 1)
        {
            dest = Material_GetShaderArgumentDest(argDest->paramName, argDest->indexRange.first, paramTable, paramCount);
            if (dest)
            {
                if (Material_AddShaderArgumentFromLiteral(
                    shaderName,
                    (char*)argDest->paramName,
                    argSource->type,
                    argSource->u.literalConst,
                    dest,
                    &argTable[*usedCount],
                    registerUsage))
                {
                    ++*usedCount;
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else
            {
                result = 0;
            }
        }
        else
        {
            Com_ScriptError("Must assign literals to a constant one row at a time");
            result = 0;
        }
        break;
    case 3u:
    case 5u:
        for (indexOffset = 0; indexOffset < argDest->indexRange.count; ++indexOffset)
        {
            desta = Material_GetShaderArgumentDest(
                argDest->paramName,
                indexOffset + argDest->indexRange.first,
                paramTable,
                paramCount);
            if (!desta)
                return 0;
            if (!Material_AddShaderArgumentFromCodeConst(
                shaderName,
                (char *)argDest->paramName,
                argSource->type,
                argSource->u.codeIndex,
                indexOffset + LOWORD(argSource->indexRange.first),
                desta,
                &argTable[*usedCount],
                registerUsage))
                return 0;
            ++*usedCount;
        }
        result = 1;
        break;
    case 4u:
        for (indexOffseta = 0; indexOffseta < argDest->indexRange.count; ++indexOffseta)
        {
            destb = Material_GetShaderArgumentDest(
                argDest->paramName,
                indexOffseta + argDest->indexRange.first,
                paramTable,
                paramCount);
            if (!destb)
                return 0;
            Material_AddShaderArgumentFromCodeSampler(
                argSource->type,
                (MaterialTextureSource)(indexOffseta + argSource->indexRange.first + argSource->u.codeIndex),
                destb,
                &argTable[*usedCount]);
            ++*usedCount;
        }
        result = 1;
        break;
    default:
        if (!alwaysfails)
        {
            v10 = va("unhandled case %i", argSource->type);
            MyAssertHandler(".\\r_material_load_obj.cpp", 3235, 1, v10);
        }
        result = 0;
        break;
    }
    return result;
}

unsigned int __cdecl Material_ElemCountForParamName(
    const char *shaderName,
    const ShaderUniformDef *paramTable,
    unsigned int paramCount,
    const char *name,
    ShaderParamType *paramType)
{
    unsigned int paramIndex; // [esp+14h] [ebp-8h]
    unsigned int count; // [esp+18h] [ebp-4h]

    count = 0;
    for (paramIndex = 0; paramIndex < paramCount; ++paramIndex)
    {
        if (!strcmp(name, paramTable[paramIndex].name))
        {
            if (count && paramTable[paramIndex].type != *paramType)
                Com_Error(ERR_DROP, "param type changed from %i to %i", paramTable[paramIndex].type, *paramType);
            *paramType = paramTable[paramIndex].type;
            if (count <= paramTable[paramIndex].index)
                count = paramTable[paramIndex].index + 1;
            iassert( count > 0 );
        }
    }
    return count;
}

bool __cdecl Material_ParseIndexRange(const char **text, unsigned int arrayCount, ShaderIndexRange *indexRange)
{
    unsigned int last; // [esp+4h] [ebp-4h]

    if (Com_Parse(text)->token[0] == 91)
    {
        indexRange->isImplicit = 0;
        indexRange->first = Com_ParseInt(text);
        if (indexRange->first < arrayCount)
        {
            if (Com_Parse(text)->token[0] == 44)
            {
                last = Com_ParseInt(text);
                if (last >= indexRange->first && last < arrayCount)
                {
                    return Material_MatchToken(text, "]");
                }
                else
                {
                    Com_ScriptError("ending index %i is not in the range [%i, %i]\n", last, indexRange->first, arrayCount - 1);
                    return 0;
                }
            }
            else
            {
                Com_UngetToken();
                indexRange->count = 1;
                return 1;
            }
        }
        else
        {
            Com_ScriptError("index %i is not in the range [0, %i]\n", indexRange->first, arrayCount - 1);
            return 0;
        }
    }
    else
    {
        Com_UngetToken();
        indexRange->first = 0;
        indexRange->count = arrayCount;
        indexRange->isImplicit = 1;
        return 1;
    }
}

bool __cdecl Material_ParseVector(const char **text, int elemCount, float *vector)
{
    int elemIndex; // [esp+0h] [ebp-4h]

    if (!Material_MatchToken(text, "("))
        return 0;
    elemIndex = 0;
    while (1)
    {
        vector[elemIndex++] = Com_ParseFloat(text);
        if (elemIndex == elemCount)
            break;
        if (!Material_MatchToken(text, ","))
            return 0;
    }
    return Material_MatchToken(text, ")");
}

char __cdecl Material_ParseLiteral(const char **text, const char *token, float *literal)
{
    iassert( text );
    iassert( token );
    iassert( literal );
    *literal = 0.0;
    literal[1] = 0.0;
    literal[2] = 0.0;
    literal[3] = 1.0;
    if (!strcmp(token, "float1"))
    {
        Material_ParseVector(text, 1, literal);
    }
    else if (!strcmp(token, "float2"))
    {
        Material_ParseVector(text, 2, literal);
    }
    else if (!strcmp(token, "float3"))
    {
        Material_ParseVector(text, 3, literal);
    }
    else
    {
        if (strcmp(token, "float4"))
            return 0;
        Material_ParseVector(text, 4, literal);
    }
    return 1;
}

char __cdecl Material_ParseArrayOffset(const char **text, int arrayCount, int arrayStride, int *offset)
{
    int arrayIndex; // [esp+0h] [ebp-4h]

    if (!Material_MatchToken(text, "["))
        return 0;
    arrayIndex = Com_ParseInt(text);
    if (arrayIndex >= 0 && arrayIndex < arrayCount)
    {
        if (Material_MatchToken(text, "]"))
        {
            *offset = arrayStride * arrayIndex;
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_ScriptError("array index must be in range [0, %i]\n", arrayCount - 1);
        return 0;
    }
}

bool __cdecl Material_ParseCodeConstantSource_r(
    MaterialShaderType shaderType,
    const char **text,
    int offset,
    const CodeConstantSource *sourceTable,
    ShaderArgumentSource *argSource)
{
    int sourceIndex; // [esp+14h] [ebp-Ch]
    int additionalOffset; // [esp+18h] [ebp-8h] BYREF
    const char *token; // [esp+1Ch] [ebp-4h]

    iassert( text );
    iassert( sourceTable );
    iassert( argSource );
    if (!Material_MatchToken(text, "."))
        return 0;
    token = Com_Parse(text)->token;
    for (sourceIndex = 0; ; ++sourceIndex)
    {
        if (!sourceTable[sourceIndex].name)
        {
            Com_ScriptError("unknown constant source '%s'\n", token);
            return 0;
        }
        if (!strcmp(token, sourceTable[sourceIndex].name))
            break;
    }
    if (sourceTable[sourceIndex].arrayCount)
    {
        if (!sourceTable[sourceIndex].subtable
            && (sourceTable[sourceIndex].source >= 0x3Au || sourceTable[sourceIndex].arrayStride != 1))
        {
            MyAssertHandler(
                ".\\r_material_load_obj.cpp",
                2671,
                0,
                "%s\n\t(sourceTable[sourceIndex].name) = %s",
                "(sourceTable[sourceIndex].subtable || (sourceTable[sourceIndex].source < CONST_SRC_FIRST_CODE_MATRIX && sourceTa"
                "ble[sourceIndex].arrayStride == 1))",
                sourceTable[sourceIndex].name);
        }
        if (sourceTable[sourceIndex].subtable)
        {
            if (!Material_ParseArrayOffset(
                text,
                sourceTable[sourceIndex].arrayCount,
                sourceTable[sourceIndex].arrayStride,
                &additionalOffset))
                return 0;
            offset += additionalOffset;
        }
        else
        {
            if (sourceTable[sourceIndex].arrayStride != 1)
                MyAssertHandler(
                    ".\\r_material_load_obj.cpp",
                    2674,
                    1,
                    "%s\n\t(sourceIndex) = %i",
                    "(sourceTable[sourceIndex].arrayStride == 1)",
                    sourceIndex);
            if (!Material_ParseIndexRange(text, sourceTable[sourceIndex].arrayCount, &argSource->indexRange))
                return 0;
        }
    }
    if (sourceTable[sourceIndex].subtable)
        return Material_ParseCodeConstantSource_r(shaderType, text, offset, sourceTable[sourceIndex].subtable, argSource);
    argSource->type = 2 * (shaderType != MTL_VERTEX_SHADER) + 3;
    argSource->u.codeIndex = offset + sourceTable[sourceIndex].source;
    if (argSource->type != 3 && !s_codeConstUpdateFreq[argSource->u.codeIndex])
        MyAssertHandler(
            ".\\r_material_load_obj.cpp",
            2691,
            0,
            "%s\n\t(argSource->u.codeIndex) = %i",
            "((argSource->type == MTL_ARG_CODE_VERTEX_CONST) || s_codeConstUpdateFreq[argSource->u.codeIndex] != MTL_UPDATE_PER_PRIM)",
            argSource->u.codeIndex);
    if (!sourceTable[sourceIndex].arrayCount)
    {
        if (argSource->u.codeIndex >= 0x3Au)
        {
            if (!Material_ParseIndexRange(text, 4u, &argSource->indexRange))
                return 0;
        }
        else
        {
            argSource->indexRange.first = 0;
            argSource->indexRange.count = 1;
            argSource->indexRange.isImplicit = 0;
        }
    }
    return 1;
}

const char *__cdecl Material_RegisterString(char *string)
{
    const char *v1; // eax
    unsigned int v3; // [esp+0h] [ebp-34h]
    unsigned __int8 *buffer; // [esp+24h] [ebp-10h]
    unsigned int hash; // [esp+28h] [ebp-Ch]
    unsigned int hashIndex; // [esp+2Ch] [ebp-8h]

    hash = R_HashString(string);
    for (hashIndex = hash & 0x3F; mtlLoadGlob.stringHashTable[hashIndex].string; hashIndex = (hashIndex + 1) & 0x3F)
    {
        if (mtlLoadGlob.stringHashTable[hashIndex].hash == hash)
        {
            if (strcmp(mtlLoadGlob.stringHashTable[hashIndex].string, string))
            {
                v1 = va("%s != %s", mtlLoadGlob.stringHashTable[hashIndex].string, string);
                MyAssertHandler(
                    ".\\r_material_load_obj.cpp",
                    1317,
                    0,
                    "%s\n\t%s",
                    "!strcmp( mtlLoadGlob.stringHashTable[hashIndex].string, string )",
                    v1);
            }
            return mtlLoadGlob.stringHashTable[hashIndex].string;
        }
    }
    if (++mtlLoadGlob.stringCount == 64)
        Com_Error(ERR_DROP, "More than %i string identifiers used by shaders", 63);
    v3 = strlen(string);
    buffer = Material_Alloc(v3 + 1);
    memcpy(buffer, string, v3 + 1);
    mtlLoadGlob.stringHashTable[hashIndex].string = (const char*)buffer;
    mtlLoadGlob.stringHashTable[hashIndex].hash = hash;
    return mtlLoadGlob.stringHashTable[hashIndex].string;
}

float *__cdecl Material_RegisterLiteral(const float *literal)
{
    float *v2; // [esp+0h] [ebp-8h]
    unsigned int literalIndex; // [esp+4h] [ebp-4h]

    for (literalIndex = 0; literalIndex < mtlLoadGlob.literalCount; ++literalIndex)
    {
        if (Vec4Compare(literal, mtlLoadGlob.literalTable[literalIndex]))
            return mtlLoadGlob.literalTable[literalIndex];
    }
    if (literalIndex == 16)
        Com_Error(ERR_DROP, "more than %i shader literals used", 16);
    ++mtlLoadGlob.literalCount;
    v2 = mtlLoadGlob.literalTable[literalIndex];
    *v2 = *literal;
    v2[1] = literal[1];
    v2[2] = literal[2];
    v2[3] = literal[3];
    return v2;
}

bool __cdecl Material_ParseConstantSource(
    MaterialShaderType shaderType,
    const char **text,
    ShaderArgumentSource *argSource)
{
    float literal[4]; // [esp+28h] [ebp-14h] BYREF
    const char *token; // [esp+38h] [ebp-4h]

    token = Com_Parse(text)->token;
    if (Material_ParseLiteral(text, token, literal))
    {
        argSource->type = shaderType != MTL_VERTEX_SHADER ? 7 : 1;
        argSource->u.literalConst = Material_RegisterLiteral(literal);
        argSource->indexRange.first = 0;
        argSource->indexRange.count = 1;
        argSource->indexRange.isImplicit = 1;
        return argSource->u.literalConst != 0;
    }
    else if (!strcmp(token, "constant"))
    {
        return Material_ParseCodeConstantSource_r(shaderType, text, 0, s_codeConsts, argSource);
    }
    else if (!strcmp(token, "material"))
    {
        if (Material_MatchToken(text, "."))
        {
            token = Com_Parse(text)->token;
            argSource->type = shaderType != MTL_VERTEX_SHADER ? 6 : 0;
            argSource->u.literalConst = (const float*)Material_RegisterString((char*)token);
            argSource->indexRange.first = 0;
            argSource->indexRange.count = 1;
            argSource->indexRange.isImplicit = 1;
            return argSource->u.literalConst != 0;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_ScriptError("expected 'sampler' or 'material', found '%s' instead\n", token);
        return 0;
    }
}

bool __cdecl Material_CodeSamplerSource_r(
    const char **text,
    int offset,
    const CodeSamplerSource *sourceTable,
    ShaderArgumentSource *argSource)
{
    int sourceIndex; // [esp+14h] [ebp-Ch]
    int additionalOffset; // [esp+18h] [ebp-8h] BYREF
    const char *token; // [esp+1Ch] [ebp-4h]

    iassert( text );
    iassert( sourceTable );
    if (!Material_MatchToken(text, "."))
        return 0;
    token = Com_Parse(text)->token;
    for (sourceIndex = 0; ; ++sourceIndex)
    {
        if (!sourceTable[sourceIndex].name)
        {
            Com_ScriptError("unknown sampler source '%s'\n", token);
            return 0;
        }
        if (!strcmp(token, sourceTable[sourceIndex].name))
            break;
    }
    if (sourceTable[sourceIndex].subtable)
    {
        if (sourceTable[sourceIndex].arrayCount)
        {
            if (!Material_ParseArrayOffset(
                text,
                sourceTable[sourceIndex].arrayCount,
                sourceTable[sourceIndex].arrayStride,
                &additionalOffset))
                return 0;
            offset += additionalOffset;
        }
        return Material_CodeSamplerSource_r(text, offset, sourceTable[sourceIndex].subtable, argSource);
    }
    else
    {
        argSource->type = 4;
        argSource->u.codeIndex = offset + sourceTable[sourceIndex].source;
        if (sourceTable[sourceIndex].arrayCount)
        {
            if (sourceTable[sourceIndex].arrayStride != 1)
                MyAssertHandler(
                    ".\\r_material_load_obj.cpp",
                    2526,
                    0,
                    "%s\n\t(sourceIndex) = %i",
                    "(sourceTable[sourceIndex].arrayStride == 1)",
                    sourceIndex);
            return Material_ParseIndexRange(text, sourceTable[sourceIndex].arrayCount, &argSource->indexRange);
        }
        else
        {
            argSource->indexRange.first = 0;
            argSource->indexRange.count = 1;
            argSource->indexRange.isImplicit = 1;
            return 1;
        }
    }
}

bool __cdecl Material_ParseSamplerSource(const char **text, ShaderArgumentSource *argSource)
{
    parseInfo_t *v3; // eax
    parseInfo_t *token; // [esp+28h] [ebp-4h]

    token = Com_Parse(text);
    if (!strcmp(token->token, "sampler"))
        return Material_CodeSamplerSource_r(text, 0, s_codeSamplers, argSource);
    if (!strcmp(token->token, "material"))
    {
        if (Material_MatchToken(text, "."))
        {
            v3 = Com_Parse(text);
            argSource->type = 2;
            argSource->u.literalConst = (const float*)Material_RegisterString(v3->token);
            argSource->indexRange.first = 0;
            argSource->indexRange.count = 1;
            argSource->indexRange.isImplicit = 1;
            return argSource->u.literalConst != 0;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_ScriptError("expected 'sampler' or 'material', found '%s' instead\n", token->token);
        return 0;
    }
}

bool __cdecl Material_ParseArgumentSource(
    MaterialShaderType shaderType,
    const char **text,
    const char *shaderName,
    ShaderParamType paramType,
    ShaderArgumentSource *argSource)
{
    const char *v6; // eax

    iassert( text );
    iassert( *text );
    iassert( shaderName );
    iassert( argSource );
    if (!Material_MatchToken(text, "="))
        return 0;
    if (paramType == SHADER_PARAM_FLOAT4)
        return Material_ParseConstantSource(shaderType, text, argSource);
    if (paramType > SHADER_PARAM_FLOAT4 && paramType <= SHADER_PARAM_SAMPLER_CUBE)
        return Material_ParseSamplerSource(text, argSource);
    if (!alwaysfails)
    {
        v6 = va("unknown constant type '%i'\n", paramType);
        MyAssertHandler(".\\r_material_load_obj.cpp", 2842, 1, v6);
    }
    return 0;
}

char __cdecl Material_ParseShaderArguments(
    const char **text,
    const char *shaderName,
    MaterialShaderType shaderType,
    ShaderUniformDef *paramTable,
    unsigned int paramCount,
    unsigned __int16 *techFlags,
    unsigned int argLimit,
    unsigned int *argCount,
    MaterialShaderArgument *args)
{
    ShaderArgumentSource argSource; // [esp+0h] [ebp-A40h] BYREF
    unsigned int usedCount; // [esp+14h] [ebp-A2Ch] BYREF
    char paramName[256]; // [esp+18h] [ebp-A28h] BYREF
    unsigned int paramIndex; // [esp+118h] [ebp-928h]
    ShaderArgumentDest argDest; // [esp+11Ch] [ebp-924h] BYREF
    ShaderParamType paramType; // [esp+12Ch] [ebp-914h] BYREF
    unsigned int registerCount; // [esp+130h] [ebp-910h]
    const char *token; // [esp+134h] [ebp-90Ch]
    MaterialShaderArgument localArgs[32]; // [esp+138h] [ebp-908h] BYREF
    char registerUsage[32][64]; // [esp+238h] [ebp-808h] BYREF

    memset(registerUsage, 0, sizeof(registerUsage));
    iassert( techFlags );
    iassert( paramTable );
    usedCount = 0;
    if (!Material_MatchToken(text, "{"))
        return 0;

    while (1)
    {
        token = Com_Parse(text)->token;
        if (!*token)
        {
            Com_ScriptError("unexpected end-of-file\n");
            return 0;
        }
        if (*token == 125)
            break;
        I_strncpyz(paramName, token, 256);
        registerCount = Material_ElemCountForParamName(shaderName, paramTable, paramCount, paramName, &paramType);
        if (registerCount)
        {
            if (!Material_ParseIndexRange(text, registerCount, &argDest.indexRange))
                return 0;
            argDest.paramName = paramName;
            if (!Material_ParseArgumentSource(shaderType, text, shaderName, paramType, &argSource))
                return 0;
            if (!Material_MatchToken(text, ";"))
                return 0;
            if (!Material_AddShaderArgument(
                shaderName,
                shaderType,
                &argSource,
                &argDest,
                paramTable,
                paramCount,
                &usedCount,
                localArgs,
                registerUsage))
                return 0;
            if (argSource.type == 4)
            {
                switch (argSource.u.codeIndex)
                {
                case 0xAu:
                    *techFlags |= 1u;
                    break;
                case 0xBu:
                    *techFlags |= 2u;
                    break;
                case 0x12u:
                case 0x13u:
                case 0x14u:
                    *techFlags |= 0x20u;
                    break;
                }
            }
        }
        else
        {
            Com_SetScriptWarningPrefix("^3WARNING: ");
            Com_ScriptError("'%s' is not referenced by %s\n", paramName, shaderName);
            Com_SetScriptWarningPrefix("^1ERROR: ");
            if (!Material_MatchToken(text, "="))
                return 0;
            Com_SkipRestOfLine(text);
        }
    }
    if (usedCount == paramCount)
        return Material_SetShaderArguments(usedCount, localArgs, argLimit, argCount, args);
    for (paramIndex = 0; paramIndex < paramCount; ++paramIndex)
    {
        if (!paramTable[paramIndex].isAssigned)
        {
            argDest.paramName = paramTable[paramIndex].name;
            argDest.indexRange.first = paramTable[paramIndex].index;
            argDest.indexRange.count = 1;
            argDest.indexRange.isImplicit = 0;
            if (Material_DefaultArgumentSource(
                shaderType,
                paramTable[paramIndex].name,
                paramTable[paramIndex].type,
                &argDest.indexRange,
                &argSource))
            {
                if (argSource.type == 5)
                {
                    if (argSource.u.codeIndex == 4)
                        *techFlags |= 0x10u;
                }
                else if (argSource.type == 4
                    && (argSource.u.codeIndex == 18 || argSource.u.codeIndex == 19 || argSource.u.codeIndex == 20))
                {
                    *techFlags |= 0x20u;
                }
                if (!Material_AddShaderArgument(
                    shaderName,
                    shaderType,
                    &argSource,
                    &argDest,
                    paramTable,
                    paramCount,
                    &usedCount,
                    localArgs,
                    registerUsage))
                    return 0;
            }
        }
    }
    if (usedCount == paramCount)
        return Material_SetShaderArguments(usedCount, localArgs, argLimit, argCount, args);
    Com_PrintWarning(8, "Undefined shader parameter(s) in %s\n", shaderName);
    for (paramIndex = 0; paramIndex < paramCount; ++paramIndex)
    {
        if (!paramTable[paramIndex].isAssigned)
            Com_PrintWarning(8, "  %s\n", paramTable[paramIndex].name);
    }
    Com_PrintWarning(8, "%i parameter(s) were undefined\n", paramCount - usedCount);
    return 0;
}

unsigned __int8 __cdecl Material_GetStreamDestForSemantic(const _D3DXSEMANTIC *semantic)
{
    unsigned int v1; // eax

    switch (semantic->Usage)
    {
    case 0u:
        if (semantic->UsageIndex)
            goto LABEL_13;
        LOBYTE(v1) = 0;
        break;
    case 3u:
        if (semantic->UsageIndex)
            goto LABEL_13;
        LOBYTE(v1) = 1;
        break;
    case 5u:
        if (semantic->UsageIndex >= 8)
            goto LABEL_13;
        v1 = semantic->UsageIndex + 4;
        break;
    case 0xAu:
        if (semantic->UsageIndex >= 2)
            goto LABEL_13;
        v1 = semantic->UsageIndex + 2;
        break;
    default:
    LABEL_13:
        Com_Error(ERR_DROP, "Unknown shader input/output usage %i:%i\n", semantic->Usage, semantic->UsageIndex);
        LOBYTE(v1) = 0;
        break;
    }
    return v1;
}

void __cdecl Material_SetVaryingParameterDef(const _D3DXSEMANTIC *semantic, ShaderVaryingDef *paramDef)
{
    paramDef->streamDest = Material_GetStreamDestForSemantic(semantic);
    paramDef->resourceDest = paramDef->streamDest;
    paramDef->name = Material_NameForStreamDest(paramDef->streamDest);
    paramDef->isAssigned = 0;
}

char __cdecl Material_SetPassShaderArguments_DX(
    const char **text,
    const char *shaderName,
    MaterialShaderType shaderType,
    unsigned int *program,
    unsigned __int16 *techFlags,
    ShaderParameterSet *paramSet,
    unsigned int argLimit,
    unsigned int *argCount,
    MaterialShaderArgument *args)
{
    const char *v9; // eax
    HRESULT v11; // [esp-4h] [ebp-1A8h]
    _D3DXSHADER_CONSTANTTABLE *constantTable; // [esp+0h] [ebp-1A4h]
    _D3DXSEMANTIC inputSemantics[32]; // [esp+4h] [ebp-1A0h] BYREF
    ID3DXConstantTable *constants; // [esp+108h] [ebp-9Ch] BYREF
    unsigned int inputCount; // [esp+10Ch] [ebp-98h] BYREF
    HRESULT hr; // [esp+110h] [ebp-94h]
    unsigned int outputCount; // [esp+114h] [ebp-90h] BYREF
    bool success; // [esp+11Bh] [ebp-89h]
    _D3DXSEMANTIC outputSemantics[16]; // [esp+11Ch] [ebp-88h] BYREF
    unsigned int semanticIndex; // [esp+1A0h] [ebp-4h]

    hr = D3DXGetShaderConstantTable((const DWORD*)program, &constants);
    if (hr >= 0)
    {
        iassert( constants );
        constantTable = (_D3DXSHADER_CONSTANTTABLE*)constants->GetBufferPointer();
        paramSet->uniformInputCount = Material_PrepareToParseShaderArguments(constantTable, paramSet->uniformInputs);
        success = Material_ParseShaderArguments(
            text,
            shaderName,
            shaderType,
            paramSet->uniformInputs,
            paramSet->uniformInputCount,
            techFlags,
            argLimit,
            argCount,
            args);
        constants->Release();
        if (success)
        {
            hr = D3DXGetShaderInputSemantics((const DWORD*)program, inputSemantics, &inputCount);
            paramSet->varyingInputCount = 0;
            for (semanticIndex = 0; semanticIndex < inputCount; ++semanticIndex)
            {
                Material_SetVaryingParameterDef(
                    &inputSemantics[semanticIndex],
                    &paramSet->varyingInputs[paramSet->varyingInputCount]);
                ++paramSet->varyingInputCount;
            }
            hr = D3DXGetShaderOutputSemantics((const DWORD *)program, outputSemantics, &outputCount);
            paramSet->outputCount = 0;
            for (semanticIndex = 0; semanticIndex < outputCount; ++semanticIndex)
            {
                if (outputSemantics[semanticIndex].Usage)
                {
                    if (outputSemantics[semanticIndex].Usage != 11)
                    {
                        Material_SetVaryingParameterDef(&outputSemantics[semanticIndex], &paramSet->outputs[paramSet->outputCount]);
                        ++paramSet->outputCount;
                    }
                }
            }
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        v11 = hr;
        v9 = R_ErrorDescription(hr);
        Com_ScriptError("Couldn't get the constant table: %s (%08x)\n", v9, v11);
        return 0;
    }
}

char __cdecl Material_LoadPassPixelShader(
    const char **text,
    GfxRenderer renderer,
    unsigned __int16 *techFlags,
    ShaderParameterSet *paramSet,
    MaterialPass *pass,
    unsigned int argLimit,
    unsigned int *argCount,
    MaterialShaderArgument *args)
{
    unsigned __int8 shaderVersion; // [esp+3h] [ebp-Dh]
    parseInfo_t *shaderName; // [esp+8h] [ebp-8h]
    MaterialPixelShader *mtlShader; // [esp+Ch] [ebp-4h]

    memset(paramSet, 0, sizeof(ShaderParameterSet));
    if (!Material_MatchToken(text, "pixelShader"))
        return 0;
    shaderVersion = Material_ParseShaderVersion(text);
    shaderName = Com_Parse(text);
    mtlShader = Material_RegisterPixelShader(shaderName->token, shaderVersion, renderer);
    if (!mtlShader)
        return 0;
    pass->pixelShader = mtlShader;
    return Material_SetPassShaderArguments_DX(
        text,
        mtlShader->name,
        MTL_PIXEL_SHADER,
        (unsigned int*)&mtlShader[1],
        techFlags,
        paramSet,
        argLimit,
        argCount,
        args);
}

int __cdecl Material_CompareShaderArgumentsForRuntime(
    const MaterialShaderArgument *e0,
    const MaterialShaderArgument *e1)
{
    int updateFreq; // [esp+0h] [ebp-10h]
    int updateFreq_4; // [esp+4h] [ebp-Ch]

    updateFreq = Material_GetArgUpdateFrequency(e0);
    updateFreq_4 = Material_GetArgUpdateFrequency(e1);
    if (updateFreq != updateFreq_4)
        return updateFreq - updateFreq_4;
    if (e0->type != e1->type)
        return e0->type - e1->type;
    if (!e0->type || e0->type == 6 || e0->type == 2)
        return e0->u.codeSampler < e1->u.codeSampler ? -1 : 1;
    return e0->dest - e1->dest;
}

bool __cdecl Material_ParseIndex(const char **text, int indexCount, int *index)
{
    if (!Material_MatchToken(text, "["))
        return 0;
    *index = Com_ParseInt(text);
    if (*index < 0 || *index >= indexCount)
        Com_ScriptError("index '%i' is not in the range [0, %i]\n", *index, indexCount - 1);
    return Material_MatchToken(text, "]");
}

char __cdecl Material_StreamDestForName(const char **text, const char *destName, unsigned __int8 *dest)
{
    int index; // [esp+50h] [ebp-4h] BYREF

    if (!strcmp(destName, "position"))
    {
        *dest = 0;
        return 1;
    }
    else if (!strcmp(destName, "normal"))
    {
        *dest = 1;
        return 1;
    }
    else if (!strcmp(destName, "color"))
    {
        if (Material_ParseIndex(text, 2, &index))
        {
            *dest = index + 2;
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else if (!strcmp(destName, "texcoord"))
    {
        if (Material_ParseIndex(text, 8, &index))
        {
            *dest = index + 4;
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_ScriptError("unknown stream destination '%s'\n", destName);
        return 0;
    }
}

const char *__cdecl Material_NameForStreamDest(unsigned __int8 dest)
{
    const char *result; // eax
    const char *v2; // eax

    switch (dest)
    {
    case 0u:
        result = "position";
        break;
    case 1u:
        result = "normal";
        break;
    case 2u:
        result = "color[0]";
        break;
    case 3u:
        result = "color[1]";
        break;
    case 4u:
        result = "texcoord[0]";
        break;
    case 5u:
        result = "texcoord[1]";
        break;
    case 6u:
        result = "texcoord[2]";
        break;
    case 7u:
        result = "texcoord[3]";
        break;
    case 8u:
        result = "texcoord[4]";
        break;
    case 9u:
        result = "texcoord[5]";
        break;
    case 0xAu:
        result = "texcoord[6]";
        break;
    case 0xBu:
        result = "texcoord[7]";
        break;
    default:
        if (!alwaysfails)
        {
            v2 = va("unhandled case %i", dest);
            MyAssertHandler(".\\r_material_load_obj.cpp", 2215, 0, v2);
        }
        result = "";
        break;
    }
    return result;
}

char __cdecl Material_ResourceDestForStreamDest(
    unsigned __int8 streamDest,
    ShaderVaryingDef *inputTable,
    unsigned int inputCount,
    unsigned __int8 *resourceDest)
{
    const char *v5; // eax
    unsigned int inputIndex; // [esp+0h] [ebp-4h]

    for (inputIndex = 0; ; ++inputIndex)
    {
        if (inputIndex >= inputCount)
        {
            v5 = Material_NameForStreamDest(streamDest);
            Com_ScriptError("vertex shader doesn't use input '%s'.\n", v5);
            return 0;
        }
        if (inputTable[inputIndex].streamDest == streamDest)
            break;
    }
    if (inputTable[inputIndex].isAssigned)
    {
        Com_ScriptError("vertex input '%s' specified more than once.\n", inputTable[inputIndex].name);
        return 0;
    }
    else
    {
        inputTable[inputIndex].isAssigned = 1;
        *resourceDest = inputTable[inputIndex].resourceDest;
        return 1;
    }
}

char __cdecl Material_StreamSourceForName(const char **text, const char *sourceName, unsigned __int8 *source)
{
    int index; // [esp+78h] [ebp-4h] BYREF

    if (!strcmp(sourceName, "position"))
    {
        *source = 0;
        return 1;
    }
    else if (!strcmp(sourceName, "normal"))
    {
        *source = 3;
        return 1;
    }
    else if (!strcmp(sourceName, "tangent"))
    {
        *source = 4;
        return 1;
    }
    else if (!strcmp(sourceName, "color"))
    {
        *source = 1;
        return 1;
    }
    else if (!strcmp(sourceName, "texcoord"))
    {
        if (Material_ParseIndex(text, 3, &index))
        {
            if (index)
                *source = index + 4;
            else
                *source = 2;
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else if (!strcmp(sourceName, "normalTransform"))
    {
        if (Material_ParseIndex(text, 2, &index))
        {
            *source = index + 7;
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_ScriptError("unknown stream source '%s'\n", sourceName);
        return 0;
    }
}

bool __cdecl Material_CheckUnspecifiedVertexInputs(const ShaderVaryingDef *inputTable, unsigned int inputCount)
{
    unsigned int inputIndex; // [esp+0h] [ebp-8h]
    bool isValid; // [esp+7h] [ebp-1h]

    isValid = 1;
    for (inputIndex = 0; inputIndex < inputCount; ++inputIndex)
    {
        if (!inputTable[inputIndex].isAssigned)
        {
            Com_ScriptError("vertex input '%s' is not specified.\n", inputTable[inputIndex].name);
            isValid = 0;
        }
    }
    return isValid;
}

int __cdecl Material_HashVertexDecl(const MaterialStreamRouting *routingData, int streamCount)
{
    char hash; // [esp+0h] [ebp-10h]
    unsigned int byteIndex; // [esp+Ch] [ebp-4h]

    hash = 0;
    for (byteIndex = 0; byteIndex < 2 * streamCount; ++byteIndex)
        hash += (byteIndex + 119) * *(&routingData->source + byteIndex);
    return hash & 0x1F;
}

MaterialVertexDeclaration *__cdecl Material_AllocVertexDecl(
    MaterialStreamRouting *routingData,
    unsigned int streamCount,
    bool *existing)
{
    unsigned int hashIndex; // [esp+8h] [ebp-Ch]
    MaterialVertexDeclaration *mvd; // [esp+Ch] [ebp-8h]
    int routingIndex; // [esp+10h] [ebp-4h]

    iassert( streamCount );
    hashIndex = Material_HashVertexDecl(routingData, streamCount);
    for (mvd = &mtlLoadGlob.vertexDeclHashTable[hashIndex];
        mvd->streamCount;
        mvd = &mtlLoadGlob.vertexDeclHashTable[hashIndex])
    {
        if (mvd->streamCount == streamCount && !memcmp(&mvd->routing, routingData, 2 * streamCount))
        {
            *existing = 1;
            return mvd;
        }
        hashIndex = (hashIndex + 1) & 0x1F;
    }
    if (mtlLoadGlob.vertexDeclCount == 31)
        Com_Error(ERR_DROP, "More than %i vertex declarations in use", 31);
    ++mtlLoadGlob.vertexDeclCount;
    memset(&mvd->streamCount, 0, sizeof(MaterialVertexDeclaration));
    memcpy(&mvd->routing, &routingData->source, 2 * streamCount);
    if (streamCount >= 0x10)
        MyAssertHandler(
            ".\\r_material_load_obj.cpp",
            1245,
            0,
            "streamCount doesn't index ARRAY_COUNT( mvd->routing.data )\n\t%i not in [0, %i)",
            streamCount,
            16);
    mvd->streamCount = streamCount;
    if (mvd->streamCount != streamCount)
        MyAssertHandler(
            ".\\r_material_load_obj.cpp",
            1247,
            0,
            "%s\n\t(streamCount) = %i",
            "(mvd->streamCount == streamCount)",
            streamCount);
    for (routingIndex = 0; routingIndex < streamCount; ++routingIndex)
    {
        if (mvd->routing.data[routingIndex].source >= 5u)
        {
            mvd->hasOptionalSource = 1;
            break;
        }
    }
    *existing = 0;
    return mvd;
}

char __cdecl Material_LoadPassVertexDecl(
    const char **text,
    ShaderVaryingDef *inputTable,
    unsigned int inputCount,
    MaterialPass *pass)
{
    unsigned __int8 source; // [esp+16h] [ebp-3Ah] BYREF
    unsigned __int8 resourceDest; // [esp+17h] [ebp-39h] BYREF
    int insertIndex; // [esp+18h] [ebp-38h]
    bool existing; // [esp+1Fh] [ebp-31h] BYREF
    const char *token; // [esp+20h] [ebp-30h]
    int routingIndex; // [esp+24h] [ebp-2Ch]
    unsigned __int8 dest[2]; // [esp+2Ah] [ebp-26h] BYREF
    MaterialStreamRouting routing[16]; // [esp+2Ch] [ebp-24h] BYREF

    for (routingIndex = 0; ; ++routingIndex)
    {
        if (routingIndex >= 0x10)
        {
            Com_ScriptError("More than %i vertex mappings\n", routingIndex);
            return 0;
        }
        token = Com_Parse(text)->token;
        if (strcmp(token, "vertex"))
            break;
        if (!Material_MatchToken(text, "."))
            return 0;
        token = Com_Parse(text)->token;
        if (!Material_StreamDestForName(text, token, &dest[1]))
            return 0;
        if (!Material_ResourceDestForStreamDest(dest[1], inputTable, inputCount, &resourceDest))
            return 0;
        if (!Material_MatchToken(text, "="))
            return 0;
        if (!Material_MatchToken(text, "code"))
            return 0;
        if (!Material_MatchToken(text, "."))
            return 0;
        token = Com_Parse(text)->token;
        if (!Material_StreamSourceForName(text, token, &source))
            return 0;
        if (!Material_MatchToken(text, ";"))
            return 0;
        for (insertIndex = routingIndex;
            insertIndex > 0
            && routing[insertIndex - 1].source >= source
            && (routing[insertIndex - 1].source != source || routing[insertIndex - 1].dest >= resourceDest);
            --insertIndex)
        {
            routing[insertIndex] = routing[insertIndex - 1];
        }
        routing[insertIndex].source = source;
        routing[insertIndex].dest = resourceDest;
    }
    Com_UngetToken();
    if (!Material_CheckUnspecifiedVertexInputs(inputTable, inputCount))
        return 0;
    pass->vertexDecl = Material_AllocVertexDecl(routing, routingIndex, &existing);
    if (!existing)
        Load_BuildVertexDecl(&pass->vertexDecl);
    return 1;
}

char __cdecl Material_LoadDeclTypes(const char **text, MaterialPass *pass)
{
    iassert( pass->vertexShader );
    return 1;
}

bool __cdecl Material_LoadPass(
    const char **text,
    GfxRenderer renderer,
    unsigned __int16 *techFlags,
    MaterialPass *pass,
    MaterialStateMap **stateMap)
{
    MaterialShaderArgument *customArg; // [esp+0h] [ebp-950h]
    int argIndex; // [esp+8h] [ebp-948h]
    int argIndexa; // [esp+8h] [ebp-948h]
    unsigned int customCount; // [esp+Ch] [ebp-944h]
    ShaderParameterSet pixelParamSet; // [esp+10h] [ebp-940h] BYREF
    bool success; // [esp+3A7h] [ebp-5A9h]
    ShaderParameterSet vertexParamSet; // [esp+3A8h] [ebp-5A8h] BYREF
    unsigned int argCount; // [esp+73Ch] [ebp-214h] BYREF
    unsigned int firstArg; // [esp+740h] [ebp-210h] BYREF
    unsigned int customArgIndex; // [esp+744h] [ebp-20Ch]
    unsigned int customSamplerIndex; // [esp+748h] [ebp-208h]
    MaterialShaderArgument *arg; // [esp+74Ch] [ebp-204h]
    MaterialShaderArgument args[64]; // [esp+750h] [ebp-200h] BYREF

    pass->vertexDecl = 0;
    pass->vertexShader = 0;
    pass->pixelShader = 0;
    pass->perPrimArgCount = 0;
    pass->perObjArgCount = 0;
    pass->stableArgCount = 0;
    pass->customSamplerFlags = 0;
    pass->args = 0;

    if (!Material_LoadPassStateMap(text, stateMap))
        return 0;

    argCount = 0;
    if (Material_LoadPassVertexShader(text, renderer, techFlags, &vertexParamSet, pass, 0x40u, &argCount, args))
    {
        if (argCount)
        {
            if (!Material_LoadPassPixelShader(text, renderer, techFlags, &pixelParamSet, pass, 0x40u, &argCount, args))
                goto LABEL_8;
            qsort(args, argCount, 8u, (int(*)(const void*, const void*))Material_CompareShaderArgumentsForRuntime);
            firstArg = 0;
            pass->perPrimArgCount = Material_CountArgsWithUpdateFrequency(MTL_UPDATE_PER_PRIM, args, argCount, &firstArg);
            pass->perObjArgCount = Material_CountArgsWithUpdateFrequency(MTL_UPDATE_PER_OBJECT, args, argCount, &firstArg);
            pass->stableArgCount = Material_CountArgsWithUpdateFrequency(MTL_UPDATE_RARELY, args, argCount, &firstArg);
            customArg = &args[firstArg];
            customCount = Material_CountArgsWithUpdateFrequency(MTL_UPDATE_CUSTOM, args, argCount, &firstArg);
            pass->customSamplerFlags = 0;

            customArgIndex = 0;
            while (customArgIndex < customCount)
            {
                iassert(customArg->type == MTL_ARG_CODE_PIXEL_SAMPLER);
                iassert( customArg->dest != SAMPLER_INDEX_INVALID );

                for (customSamplerIndex = 0; customSamplerIndex < 3; ++customSamplerIndex)
                {
                    if (customArg->u.codeSampler == g_customSamplerSrc[customSamplerIndex])
                    {
                        iassert(!(pass->customSamplerFlags & (1 << customSamplerIndex)));
                        iassert(customArg->dest == g_customSamplerDest[customSamplerIndex]);
                        
                        pass->customSamplerFlags |= 1 << customSamplerIndex;
                        break;
                    }
                }

                iassert( customSamplerIndex != CUSTOM_SAMPLER_COUNT );
                ++customArgIndex;
                ++customArg;
            }
            argCount = pass->stableArgCount + pass->perObjArgCount + pass->perPrimArgCount;
            pass->args = (MaterialShaderArgument*)Material_Alloc(8 * argCount);
            memcpy(pass->args, args, 8 * argCount);
            arg = pass->args;
            for (argIndex = 0; argIndex < pass->perPrimArgCount; ++argIndex)
            {
                iassert( arg->type >= MTL_ARG_CODE_PRIM_BEGIN );
                iassert( arg->type < MTL_ARG_CODE_PRIM_END );
                ++arg;
            }
            arg = &pass->args[pass->perPrimArgCount];
            for (argIndexa = 0; argIndexa < pass->perObjArgCount; ++argIndexa)
            {
                iassert( arg->type >= MTL_ARG_CODE_PRIM_BEGIN );
                iassert( arg->type < MTL_ARG_CODE_PRIM_END );
                ++arg;
            }
            if (Material_ValidateShaderLinkage(
                vertexParamSet.outputs,
                vertexParamSet.outputCount,
                pixelParamSet.varyingInputs,
                pixelParamSet.varyingInputCount))
            {
                success = Material_LoadPassVertexDecl(
                    text,
                    vertexParamSet.varyingInputs,
                    vertexParamSet.varyingInputCount,
                    pass);
                KISAK_NULLSUB();
                KISAK_NULLSUB();
                Material_LoadDeclTypes(text, pass);
                return success;
            }
            else
            {
            LABEL_8:
                KISAK_NULLSUB();
                KISAK_NULLSUB();
                return 0;
            }
        }
        else
        {
            Com_ScriptError("material has no vertex arguments; it should at least have a transform matrix.\n");
            return 0;
        }
    }
    else
    {
        KISAK_NULLSUB();
        return 0;
    }
}

MaterialTechnique *__cdecl Material_LoadTechnique(char *name, GfxRenderer renderer)
{
    unsigned __int8 *technique; // [esp+24h] [ebp-1A0h]
    int stateMapSize; // [esp+28h] [ebp-19Ch]
    MaterialStateMap *stateMap[4]; // [esp+2Ch] [ebp-198h] BYREF
    char filename[260]; // [esp+3Ch] [ebp-188h] BYREF
    MaterialVertexDeclaration *vertexDecl; // [esp+144h] [ebp-80h]
    unsigned int nameSize; // [esp+148h] [ebp-7Ch]
    unsigned __int16 techFlags; // [esp+14Ch] [ebp-78h] BYREF
    int fileSize; // [esp+150h] [ebp-74h]
    void *file; // [esp+154h] [ebp-70h] BYREF
    bool error; // [esp+15Bh] [ebp-69h]
    MaterialPass passes[4]; // [esp+15Ch] [ebp-68h] BYREF
    const char *token; // [esp+1ACh] [ebp-18h]
    MaterialStateMap **stateMapForPass; // [esp+1B0h] [ebp-14h]
    unsigned __int16 passCount; // [esp+1B4h] [ebp-10h]
    const char *formatString; // [esp+1B8h] [ebp-Ch]
    const char *text; // [esp+1BCh] [ebp-8h] BYREF
    int passIndex; // [esp+1C0h] [ebp-4h]

    formatString = "techniques/%s.tech";
    Com_sprintf(filename, 0x100u, "techniques/%s.tech", name);
    fileSize = FS_ReadFile(filename, &file);
    if (fileSize >= 0)
    {
        text = (const char*)file;
        Com_BeginParseSession(filename);
        Com_SetScriptWarningPrefix("^1ERROR: ");
        Com_SetSpaceDelimited(0);
        error = 0;
        techFlags = 0;
        for (passCount = 0; passCount < 4u; ++passCount)
        {
            token = Com_Parse(&text)->token;
            if (!*token)
                break;
            if (*token != 123)
            {
                Com_ScriptError("expected '{' but found '%s'\n", token);
                error = 1;
                break;
            }
            if (!Material_LoadPass(&text, renderer, &techFlags, &passes[passCount], &stateMap[passCount]))
            {
                Com_ScriptError("Error loading pass for dx9 technique '%s'\n", name);
                error = 1;
                break;
            }
            if (!Material_MatchToken(&text, "}"))
            {
                error = 1;
                break;
            }
        }
        Com_EndParseSession();
        FS_FreeFile((char*)file);
        if (error)
        {
            return 0;
        }
        else if (passCount)
        {
            stateMapSize = 4 * passCount;
            nameSize = strlen(name) + 1;
            technique = Material_Alloc(nameSize + 24 * passCount + 8);
            stateMapForPass = (MaterialStateMap**)&technique[20 * passCount + 8];
            *(DWORD*)technique = (DWORD)&stateMapForPass[passCount];
            memcpy(*(unsigned char**)technique, name, nameSize);
            *((_WORD *)technique + 2) = techFlags;
            if (!strcmp(*(const char**)technique, "zprepass"))
                *((_WORD *)technique + 2) |= 4u;
            for (passIndex = 0; passIndex < passCount; ++passIndex)
            {
                vertexDecl = passes[passIndex].vertexDecl;
                iassert( vertexDecl );
                if (vertexDecl->hasOptionalSource)
                {
                    *((_WORD *)technique + 2) |= 8u;
                    break;
                }
            }
            *((_WORD *)technique + 3) = passCount;
            memcpy(technique + 8, passes, 20 * passCount);
            memcpy(stateMapForPass, stateMap, stateMapSize);
            return (MaterialTechnique*)technique;
        }
        else
        {
            Com_ScriptError(
                "Technique '%s' has no passes.  The technique should be left blank in the techniqueset instead.\n",
                name);
            return 0;
        }
    }
    else
    {
        Com_ScriptError("Couldn't open technique '%s'\n", filename);
        return 0;
    }
}

void __cdecl Material_SetTechnique(const char *name, GfxRenderer renderer, MaterialTechnique *technique)
{
    unsigned int hashIndex; // [esp+0h] [ebp-4h] BYREF

    if (mtlLoadGlob.techniqueCount == 4095)
        Com_Error(ERR_DROP, "More than %i techniques in use", 4095);
    MaterialTechnique_FindHashLocation(name, renderer, &hashIndex);
    ++mtlLoadGlob.techniqueCount;
    mtlLoadGlob.techniqueHashTable[renderer][hashIndex] = technique;
}

MaterialTechnique *__cdecl Material_RegisterTechnique(char *name, GfxRenderer renderer)
{
    MaterialTechnique *technique; // [esp+0h] [ebp-4h]
    MaterialTechnique *techniquea; // [esp+0h] [ebp-4h]

    technique = Material_FindTechnique(name, renderer);
    if (technique)
        return technique;
    ProfLoad_Begin("Load technique");
    techniquea = Material_LoadTechnique(name, renderer);
    ProfLoad_End();
    if (techniquea)
        Material_SetTechnique(name, renderer, techniquea);
    return techniquea;
}

MaterialTechniqueSet *__cdecl Material_LoadTechniqueSet(char *name, GfxRenderer renderer)
{
    unsigned int v3; // [esp+0h] [ebp-1CCh]
    MaterialTechnique *technique; // [esp+10h] [ebp-1BCh]
    int techTypeCount; // [esp+14h] [ebp-1B8h]
    char filename[256]; // [esp+1Ch] [ebp-1B0h] BYREF
    int techTypeIndex; // [esp+120h] [ebp-ACh]
    _DWORD techType[35]; // [esp+124h] [ebp-A8h]
    bool usingTechnique; // [esp+1B3h] [ebp-19h]
    int nameSize; // [esp+1B4h] [ebp-18h]
    int fileSize; // [esp+1B8h] [ebp-14h]
    void *file; // [esp+1BCh] [ebp-10h] BYREF
    const char *token; // [esp+1C0h] [ebp-Ch]
    MaterialTechniqueSet *techniqueSet; // [esp+1C4h] [ebp-8h]
    const char *text; // [esp+1C8h] [ebp-4h] BYREF

    Com_sprintf(filename, 0x100u, "techsets/%s.techset", name);
    fileSize = FS_ReadFile(filename, &file);
    if (fileSize >= 0)
    {
        v3 = strlen(name);
        nameSize = v3 + 1;
        techniqueSet = (MaterialTechniqueSet*)Material_Alloc(v3 + 149);
        techniqueSet->name = (const char*)&techniqueSet[1];
        techniqueSet->worldVertFormat = 0;
        memcpy((void*)techniqueSet->name, name, nameSize);
        techniqueSet->remappedTechniqueSet = techniqueSet;
        Material_DirtyTechniqueSetOverrides();
        text = (const char*)file;
        Com_BeginParseSession(filename);
        Com_SetScriptWarningPrefix("^1ERROR: ");
        Com_SetSpaceDelimited(0);
        Com_SetKeepStringQuotes(1);
        techTypeCount = 0;
        usingTechnique = 0;
        while (1)
        {
            token = Com_Parse(&text)->token;
            if (!*token)
                break;
            if (*token == 34)
            {
                if (techTypeCount == 34)
                {
                    Com_ScriptError("Too many labels in technique set\n");
                    techniqueSet = 0;
                    break;
                }
                techType[techTypeCount] = Material_TechniqueTypeForName(token);
                if (techType[techTypeCount] == 34)
                {
                LABEL_9:
                    Com_ScriptError("Unknown technique type '%s'\n", token);
                    techniqueSet = 0;
                    break;
                }
                if (Material_UsingTechnique(techType[techTypeCount]))
                    usingTechnique = 1;
                ++techTypeCount;
                if (!Material_MatchToken(&text, ":"))
                {
                    techniqueSet = 0;
                    break;
                }
            }
            else
            {
                if (!techTypeCount)
                    goto LABEL_9;
                if (usingTechnique)
                {
                    technique = Material_RegisterTechnique((char*)token, renderer);
                    if (!technique)
                    {
                        techniqueSet = 0;
                        break;
                    }
                    for (techTypeIndex = 0; techTypeIndex < techTypeCount; ++techTypeIndex)
                        techniqueSet->techniques[techType[techTypeIndex]] = technique;
                }
                techTypeCount = 0;
                usingTechnique = 0;
                if (!Material_MatchToken(&text, ";"))
                {
                    techniqueSet = 0;
                    break;
                }
            }
        }
        Com_EndParseSession();
        FS_FreeFile((char*)file);
        return techniqueSet;
    }
    else
    {
        Com_PrintError(8, "^1ERROR: Couldn't open techniqueSet '%s'\n", filename);
        return 0;
    }
}

void __cdecl Material_SetTechniqueSet(const char *name, MaterialTechniqueSet *techniqueSet)
{
    int hashIndex; // [esp+0h] [ebp-4h] BYREF

    iassert( name );
    MaterialTechniqueSet_FindHashLocation(name, &hashIndex);
    iassert( techniqueSet->name );
    materialGlobals.techniqueSetHashTable[hashIndex] = techniqueSet;
}

const GfxMtlFeatureMap s_materialFeatures[20] =
{
  { "s0", 4u, 0u, false },
  { "s1", 4u, 0u, false },
  { "s2", 4u, 0u, false },
  { "s3", 4u, 0u, false },
  { "s4", 4u, 0u, false },
  { "d0", 8u, 0u, false },
  { "d1", 8u, 0u, false },
  { "d2", 8u, 0u, false },
  { "d3", 8u, 0u, false },
  { "d4", 8u, 0u, false },
  { "n0", 16u, 0u, false },
  { "n1", 16u, 0u, false },
  { "n2", 16u, 0u, false },
  { "n3", 16u, 0u, false },
  { "n4", 16u, 0u, false },
  { "zfeather", 1u, 0u, false },
  { "outdoor", 2u, 0u, false },
  { "sm", 384u, 128u, true },
  { "hsm", 384u, 256u, true },
  { "twk", 32u, 0u, false }
}; // idb

void __cdecl Material_RegisterOverriddenTechniqueSets_r(
    char *name,
    unsigned int nameLen,
    const char *parse,
    unsigned int unsetMask,
    unsigned int setValues)
{
    char v5; // [esp+23h] [ebp-291h]
    char *v6; // [esp+28h] [ebp-28Ch]
    char *v7; // [esp+2Ch] [ebp-288h]
    bool v8; // [esp+30h] [ebp-284h]
    char v9; // [esp+37h] [ebp-27Dh]
    char *v10; // [esp+3Ch] [ebp-278h]
    char nameExtended[260]; // [esp+4Ch] [ebp-268h] BYREF
    unsigned int featureIndex; // [esp+150h] [ebp-164h]
    bool prependUnderscore; // [esp+157h] [ebp-15Dh]
    const GfxMtlFeatureMap *feature; // [esp+158h] [ebp-15Ch]
    const GfxMtlFeatureMap *altFeature; // [esp+15Ch] [ebp-158h]
    unsigned int tokenLen; // [esp+160h] [ebp-154h]
    unsigned int extendedLen; // [esp+164h] [ebp-150h]
    bool featureIsNew; // [esp+16Bh] [ebp-149h]
    char token[64]; // [esp+16Ch] [ebp-148h] BYREF
    char nameSoFar[260]; // [esp+1ACh] [ebp-108h] BYREF

    v10 = nameSoFar;
    do
    {
        v9 = *name;
        *v10++ = *name++;
    } while (v9);
    while (1)
    {
        v8 = nameLen && *(parse - 1) == 95;
        prependUnderscore = v8;
        tokenLen = Material_NextTechniqueSetNameToken(&parse, token);
        if (!tokenLen)
            break;
        feature = Material_FindFeature(token, s_materialFeatures, 0x14u);
        if (feature)
        {
            featureIsNew = (unsetMask & feature->mask) != 0;
            unsetMask &= ~feature->mask;
            if (feature->value)
            {
                for (featureIndex = 0; featureIndex < 0x14; ++featureIndex)
                {
                    altFeature = &s_materialFeatures[featureIndex];
                    if (altFeature->mask == feature->mask)
                    {
                        if (featureIsNew)
                        {
                            v7 = nameSoFar;
                            v6 = nameExtended;
                            do
                            {
                                v5 = *v7;
                                *v6++ = *v7++;
                            } while (v5);
                            extendedLen = Material_ExtendTechniqueSetName(
                                nameExtended,
                                nameLen,
                                (char*)altFeature->name,
                                strlen(altFeature->name),
                                prependUnderscore);
                            Material_RegisterOverriddenTechniqueSets_r(
                                nameExtended,
                                extendedLen,
                                parse,
                                unsetMask,
                                altFeature->value | setValues);
                        }
                        else if (altFeature->value == (altFeature->mask & setValues))
                        {
                            nameLen = Material_ExtendTechniqueSetName(
                                nameSoFar,
                                nameLen,
                                (char*)altFeature->name,
                                strlen(altFeature->name),
                                prependUnderscore);
                            break;
                        }
                    }
                }
                if (featureIsNew && feature->valueRequired)
                    return;
            }
            else
            {
                if (featureIsNew)
                {
                    Material_RegisterOverriddenTechniqueSets_r(nameSoFar, nameLen, parse, unsetMask, setValues);
                    setValues |= feature->mask;
                }
                if ((feature->mask & setValues) != 0)
                    nameLen = Material_ExtendTechniqueSetName(nameSoFar, nameLen, token, tokenLen, prependUnderscore);
            }
        }
        else
        {
            nameLen = Material_ExtendTechniqueSetName(nameSoFar, nameLen, token, tokenLen, prependUnderscore);
        }
    }
    Material_RegisterTechniqueSet(nameSoFar);
}

void __cdecl Material_RegisterOverriddenTechniqueSets(const char *baseTechSetName)
{
    char nameSoFar[260]; // [esp+0h] [ebp-108h] BYREF

    if (!mtlOverrideGlob.isRegisteringOverrides)
    {
        mtlOverrideGlob.isRegisteringOverrides = 1;
        nameSoFar[0] = 0;
        Material_RegisterOverriddenTechniqueSets_r(nameSoFar, 0, baseTechSetName, 0xFFFFFFFF, 0);
        mtlOverrideGlob.isRegisteringOverrides = 0;
    }
}

MaterialTechniqueSet *__cdecl Material_RegisterTechniqueSet(const char *name)
{
    GfxRenderer renderer; // [esp+0h] [ebp-8h]
    MaterialTechniqueSet *techniqueSet; // [esp+4h] [ebp-4h]
    MaterialTechniqueSet *techniqueSeta; // [esp+4h] [ebp-4h]

    techniqueSet = Material_FindTechniqueSet(name, MTL_TECHSET_NOT_FOUND_RETURN_NULL);
    if (techniqueSet)
        return techniqueSet;
    renderer = (GfxRenderer)r_rendererInUse->current.integer;
    ProfLoad_Begin("Load technique set");
    techniqueSeta = Material_LoadTechniqueSet((char*)name, renderer);
    ProfLoad_End();
    if (!techniqueSeta)
        return 0;
    Material_SetTechniqueSet(name, techniqueSeta);
    if (g_generateOverrideTechniques)
        Material_RegisterOverriddenTechniqueSets(name);
    return techniqueSeta;
}

MaterialTechniqueSet *__cdecl Material_FindTechniqueSet_LoadObj(
    const char *name,
    MtlTechSetNotFoundBehavior notFoundBehavior)
{
    MaterialTechniqueSet *defaultTechSet; // [esp+0h] [ebp-8h]
    int hashIndex; // [esp+4h] [ebp-4h] BYREF

    iassert( name );
    if (MaterialTechniqueSet_FindHashLocation(name, &hashIndex))
        return materialGlobals.techniqueSetHashTable[hashIndex];
    if (notFoundBehavior == MTL_TECHSET_NOT_FOUND_RETURN_NULL)
        return 0;
    defaultTechSet = Material_RegisterTechniqueSet("default");
    iassert( defaultTechSet );
    return defaultTechSet;
}

void __cdecl Material_GetInfo(Material *handle, MaterialInfo *matInfo)
{
    iassert( handle );
    iassert( matInfo );
    *matInfo = Material_FromHandle(handle)->info;
}

Material *__cdecl Material_Duplicate(Material *mtlCopy, char *name)
{
    unsigned int v3; // [esp+8h] [ebp-30h]
    const char *nameBackup; // [esp+18h] [ebp-20h]
    Material *mtlNewa; // [esp+1Ch] [ebp-1Ch]
    unsigned __int8 *mtlNew; // [esp+1Ch] [ebp-1Ch]
    int constantTableSize; // [esp+24h] [ebp-14h]
    unsigned __int16 hashIndex[3]; // [esp+28h] [ebp-10h] BYREF
    bool exists; // [esp+2Fh] [ebp-9h] BYREF
    unsigned int textureTableSize; // [esp+30h] [ebp-8h]
    unsigned int stateBitsTableSize; // [esp+34h] [ebp-4h]

    iassert( mtlCopy );
    iassert( name );
    Material_GetHashIndex(name, hashIndex, &exists);
    if (exists)
    {
        mtlNewa = rg.materialHashTable[hashIndex[0]];
        nameBackup = mtlNewa->info.name;
        memcpy(mtlNewa, mtlCopy, sizeof(Material));
        mtlNewa->info.name = nameBackup;
        rgp.needSortMaterials = 1;
        return mtlNewa;
    }
    else
    {
        v3 = strlen(name);
        mtlNew = Material_Alloc(v3 + 81);
        memcpy(mtlNew, mtlCopy, 0x50u);
        *(_DWORD *)mtlNew = (uint32)mtlNew + 80;
        memcpy(*(unsigned __int8 **)mtlNew, (unsigned __int8 *)name, v3 + 1);
        stateBitsTableSize = 8 * mtlCopy->stateBitsCount;
        *((_DWORD *)mtlNew + 19) = (uint32)Material_Alloc(stateBitsTableSize);
        memcpy(*((unsigned __int8 **)mtlNew + 19), (unsigned __int8 *)mtlCopy->stateBitsTable, stateBitsTableSize);
        if (mtlCopy->textureTable)
        {
            textureTableSize = 12 * mtlCopy->textureCount;
            *((_DWORD *)mtlNew + 17) = (uint32)Material_Alloc(textureTableSize);
            memcpy(*((unsigned __int8 **)mtlNew + 17), (unsigned __int8 *)mtlCopy->textureTable, textureTableSize);
        }
        if (mtlCopy->constantTable)
        {
            constantTableSize = 32 * mtlCopy->constantCount;
            *((_DWORD *)mtlNew + 18) = (uint32)Material_Alloc(constantTableSize);
            memcpy(*((unsigned __int8 **)mtlNew + 18), (unsigned __int8 *)mtlCopy->constantTable, constantTableSize);
        }
        Material_Add((Material *)mtlNew, hashIndex[0]);
        return (Material *)mtlNew;
    }
}

Material *__cdecl R_GetBspMaterial(unsigned int materialIndex)
{
    const dmaterial_t *name; // [esp+2Ch] [ebp-110h]
    char materialName[260]; // [esp+34h] [ebp-108h] BYREF

    if (materialIndex >= 0x4C8)
        MyAssertHandler(
            ".\\r_bsp_load_obj.cpp",
            226,
            0,
            "materialIndex doesn't index MAX_MAP_MATERIALS\n\t%i not in [0, %i)",
            materialIndex,
            1224);
    name = &rgl.load.diskMaterials[materialIndex];
    //iassert( name[0] );
    if (!strcmp(name->material, "noshader"))
        MyAssertHandler(".\\r_bsp_load_obj.cpp", 230, 0, "%s", "strcmp( name, \"noshader\" )");
    if (!strcmp(name->material, "$default"))
    {
        strcpy((char*)&name->material[0], "$default3d");
        //name = "$default3d";
    }
    if (name->material[0] == 42)
        Com_sprintf(materialName, 0x100u, "%s%s", "", name->material);
    else
        Com_sprintf(materialName, 0x100u, "%s%s", "wc/", name->material);
    return Material_Register(materialName, 9);
}

bool __cdecl Material_HasNormalMap(const Material *mtl)
{
    unsigned int texIndex; // [esp+14h] [ebp-8h]
    unsigned int normalMapNameHash; // [esp+18h] [ebp-4h]

    iassert( mtl );
    normalMapNameHash = R_HashString("normalMap");
    for (texIndex = 0; ; ++texIndex)
    {
        if (texIndex >= mtl->textureCount)
            return 0;
        if (mtl->textureTable[texIndex].nameHash == normalMapNameHash)
            break;
    }
    iassert( mtl->textureTable[texIndex].nameStart == 'n' );
    iassert( mtl->textureTable[texIndex].nameEnd == 'p' );
    if (mtl->textureTable[texIndex].semantic != 5)
        MyAssertHandler(
            ".\\r_material_load_obj.cpp",
            6404,
            0,
            "%s",
            "mtl->textureTable[texIndex].semantic == TS_NORMAL_MAP");
    return strcmp(mtl->textureTable[texIndex].u.image->name, "$identitynormalmap") != 0;
}

unsigned int __cdecl Material_AppendTechniqueSetName(char *name, unsigned int nameLen, char *append, char lyrToken)
{
    char v5; // [esp+3h] [ebp-1h]

    while (*append)
    {
        if (nameLen == 63)
        {
            name[63] = 0;
            Com_Error(ERR_FATAL, "technique set name %s%s is too long", name, append);
        }
        if (*append == 48)
            v5 = lyrToken;
        else
            v5 = *append;
        name[nameLen++] = v5;
        ++append;
    }
    name[nameLen] = 0;
    return nameLen;
}

const LayeredTechniqueSetName *__cdecl Material_GetLayeredTechniqueSetName(const char *techSetName)
{
    unsigned int top; // [esp+0h] [ebp-10h]
    unsigned int bot; // [esp+4h] [ebp-Ch]
    int comparison; // [esp+8h] [ebp-8h]
    unsigned int mid; // [esp+Ch] [ebp-4h]
    const char *techSetNamea; // [esp+18h] [ebp+8h]

    if (!strncmp(techSetName, "w_", 2u))
    {
        techSetNamea = techSetName + 2;
    }
    else
    {
        if (strncmp(techSetName, "wc_", 3u))
            return 0;
        techSetNamea = techSetName + 3;
    }
    bot = 0;
    top = 33;
    do
    {
        mid = (top + bot) >> 1;
        comparison = I_strcmp(techSetNamea, s_lyrTechSetNames[mid].inputName);
        if (comparison >= 0)
        {
            if (comparison <= 0)
                return &s_lyrTechSetNames[mid];
            bot = mid + 1;
        }
        else
        {
            top = (top + bot) >> 1;
        }
    } while (bot < top);
    return 0;
}

MaterialTechniqueSet *__cdecl Material_RegisterLayeredTechniqueSet(const Material **mtl, unsigned int layerCount)
{
    const char *v3; // eax
    unsigned int newTechSetNameLen; // [esp+0h] [ebp-64h]
    MaterialWorldVertexFormat worldVertFormat; // [esp+4h] [ebp-60h]
    char layerToken; // [esp+Bh] [ebp-59h]
    unsigned int normalMapCount; // [esp+Ch] [ebp-58h]
    MaterialTechniqueSet *techSet; // [esp+10h] [ebp-54h]
    char newTechSetName[68]; // [esp+14h] [ebp-50h] BYREF
    unsigned int layerIndex; // [esp+5Ch] [ebp-8h]
    const LayeredTechniqueSetName *lyrTechSetName; // [esp+60h] [ebp-4h]

    if (layerCount - 1 >= 5)
        MyAssertHandler(
            ".\\r_material_load_obj.cpp",
            6480,
            0,
            "layerCount - 1 doesn't index ARRAY_COUNT( s_worldVertFormatForLayerCount )\n\t%i not in [0, %i)",
            layerCount - 1,
            5);
    worldVertFormat = (MaterialWorldVertexFormat)s_stateMapDstStencilBitGroup[9].stateBitsMask[layerCount + 1];
    normalMapCount = 0;
    newTechSetNameLen = 0;
    for (layerIndex = 0; layerIndex < layerCount; ++layerIndex)
    {
        lyrTechSetName = Material_GetLayeredTechniqueSetName(mtl[layerIndex]->techniqueSet->name);
        if (!lyrTechSetName)
        {
            Com_PrintWarning(
                8,
                "Material '%s' uses technique set '%s' which cannot be used in a layered material; using default instead.  Recomp"
                "ile the bsp to fix.\n",
                mtl[layerIndex]->info.name,
                mtl[layerIndex]->techniqueSet->name);
            return 0;
        }
        layerToken = layerIndex + 48;
        if ((layerIndex + 48) == 48)
        {
            if (lyrTechSetName->namePrefixRegister)
                newTechSetNameLen = Material_AppendTechniqueSetName(
                    newTechSetName,
                    newTechSetNameLen,
                    (char*)lyrTechSetName->namePrefixRegister,
                    layerToken);
        }
        else
        {
            newTechSetNameLen = Material_AppendTechniqueSetName(newTechSetName, newTechSetNameLen, (char*)"_", layerToken);
        }
        if (lyrTechSetName)
        {
            if (strchr(lyrTechSetName->nameChunk, 110))
                ++normalMapCount;
        }
        newTechSetNameLen = Material_AppendTechniqueSetName(
            newTechSetName,
            newTechSetNameLen,
            (char*)lyrTechSetName->nameChunk,
            layerToken);
    }
    techSet = Material_RegisterTechniqueSet(newTechSetName);
    if (techSet)
    {
        if (normalMapCount > 1)
            worldVertFormat = (MaterialWorldVertexFormat)(worldVertFormat + normalMapCount - 1);
        if (techSet->worldVertFormat && techSet->worldVertFormat != worldVertFormat)
        {
            v3 = va("%i, %i", techSet->worldVertFormat, worldVertFormat);
            MyAssertHandler(
                ".\\r_material_load_obj.cpp",
                6513,
                0,
                "%s\n\t%s",
                "techSet->worldVertFormat == MTL_WORLDVERT_TEX_1_NRM_1 || techSet->worldVertFormat == worldVertFormat",
                v3);
        }
        if (worldVertFormat != worldVertFormat)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
                281,
                0,
                "i == static_cast< Type >( i )\n\t%i, %i",
                worldVertFormat,
                worldVertFormat);
        techSet->worldVertFormat = worldVertFormat;
    }
    return techSet;
}

char __cdecl Material_HasConstant(const Material *mtl, unsigned int nameHash)
{
    unsigned int constantIndex; // [esp+0h] [ebp-4h]

    for (constantIndex = 0; constantIndex < mtl->constantCount; ++constantIndex)
    {
        if (mtl->constantTable[constantIndex].nameHash == nameHash)
            return 1;
    }
    return 0;
}

void __cdecl Material_GetLayeredStateBits(
    const Material **layerMtl,
    unsigned int layerCount,
    unsigned int techType,
    unsigned int *stateBits)
{
    const GfxStateBits *srcStateBitsa; // [esp+0h] [ebp-Ch]
    const GfxStateBits *srcStateBits; // [esp+0h] [ebp-Ch]
    unsigned int layerEntry; // [esp+4h] [ebp-8h]
    unsigned int layerIndex; // [esp+8h] [ebp-4h]

    iassert( layerMtl[0]->stateBitsEntry[techType] != UCHAR_MAX );
    srcStateBitsa = &(*layerMtl)->stateBitsTable[(*layerMtl)->stateBitsEntry[techType]];
    *stateBits = srcStateBitsa->loadBits[0];
    stateBits[1] = srcStateBitsa->loadBits[1];
    if ((*stateBits & 0xF0) == 0x10)
    {
        if ((*stateBits & 0x3800) != 0x800)
        {
            for (layerIndex = 0; layerIndex < layerCount; ++layerIndex)
            {
                layerEntry = layerMtl[layerIndex]->stateBitsEntry[techType];
                if (layerEntry != 255)
                {
                    srcStateBits = &layerMtl[layerIndex]->stateBitsTable[layerEntry];
                    if ((srcStateBits->loadBits[0] & 0xF0) != 0x10)
                    {
                        *stateBits &= 0xFFFFC000;
                        *stateBits |= 0x800u;
                        *stateBits |= 0x100u;
                        *stateBits |= 2u;
                        *stateBits |= srcStateBits->loadBits[0] & 0xF0;
                        return;
                    }
                }
            }
        }
    }
    else if ((*stateBits & 0xF) != 1)
    {
        *stateBits &= 0xFFFFFFF0;
        *stateBits |= 2u;
    }
}

unsigned __int8 __cdecl Material_AddStateBitsArrayToTable(
    const unsigned int (*stateBitsForPass)[2],
    unsigned int passCount,
    unsigned int (*stateBitsTable)[2],
    unsigned int *stateBitsCount)
{
    unsigned int scan; // [esp+8h] [ebp-8h]
    unsigned int partialMatchCount; // [esp+Ch] [ebp-4h]

    for (scan = 0; ; ++scan)
    {
        partialMatchCount = *stateBitsCount - scan;
        if (!partialMatchCount)
            break;
        if (partialMatchCount > passCount)
            partialMatchCount = passCount;
        if (!memcmp(&(*stateBitsTable)[2 * scan], stateBitsForPass, 8 * partialMatchCount))
            break;
    }
    memcpy(
        &(*stateBitsTable)[2 * *stateBitsCount],
        &(*stateBitsForPass)[2 * partialMatchCount],
        8 * (passCount - partialMatchCount));
    *stateBitsCount += passCount - partialMatchCount;
    iassert(scan == static_cast<byte>(scan));
    return scan;
}

unsigned int __cdecl Material_CreateLayeredStateBitsTable(
    const Material **layerMtl,
    unsigned int layerCount,
    const MaterialTechniqueSet *techSet,
    unsigned __int8 *stateBitsEntry,
    unsigned int (*stateBitsTable)[2])
{
    unsigned int techType; // [esp+0h] [ebp-10h]
    unsigned int derivedStateBits[2]; // [esp+4h] [ebp-Ch] BYREF
    unsigned int stateBitsCount; // [esp+Ch] [ebp-4h] BYREF

    stateBitsCount = 0;
    for (techType = 0; techType < 0x22; ++techType)
    {
        if (techSet->techniques[techType])
        {
            if (!(*layerMtl)->techniqueSet->techniques[techType])
                MyAssertHandler(
                    ".\\r_material_load_obj.cpp",
                    6224,
                    0,
                    "%s",
                    "layerMtl[0]->techniqueSet->techniques[techType] != NULL");
            if ((*layerMtl)->techniqueSet->techniques[techType]->passCount != 1)
                MyAssertHandler(
                    ".\\r_material_load_obj.cpp",
                    6225,
                    0,
                    "%s",
                    "layerMtl[0]->techniqueSet->techniques[techType]->passCount == 1");
            Material_GetLayeredStateBits(layerMtl, layerCount, techType, derivedStateBits);
            if ((derivedStateBits[0] & 0x3800) == 0)
                MyAssertHandler(
                    ".\\r_material_load_obj.cpp",
                    6227,
                    0,
                    "%s",
                    "derivedStateBits[0] & (GFXS0_ATEST_MASK | GFXS0_ATEST_DISABLE)");
            stateBitsEntry[techType] = Material_AddStateBitsArrayToTable(
                (const unsigned int(*)[2])derivedStateBits,
                1u,
                stateBitsTable,
                &stateBitsCount);
        }
        else
        {
            stateBitsEntry[techType] = -1;
        }
    }
    return stateBitsCount;
}

void __cdecl Material_AppendCharToConstName(char *name, char ch)
{
    unsigned int nameIndex; // [esp+4h] [ebp-4h]

    for (nameIndex = 0; nameIndex < 0xC; ++nameIndex)
    {
        if (!name[nameIndex])
        {
            name[nameIndex] = ch;
            if (nameIndex + 1 < 0xC)
                name[nameIndex + 1] = 0;
            return;
        }
    }
}

int __cdecl CompareHashedMaterialTextures(_DWORD *e0, _DWORD *e1)
{
    return *e0 < *e1 ? -1 : 1;
}

Material *__cdecl Material_CreateLayered(
    char *name,
    const Material **layerMtl,
    unsigned int layerCount,
    MaterialTechniqueSet *techSet)
{
    unsigned __int8 *v4; // edi
    const MaterialTextureDef *v5; // eax
    char v7; // [esp+Bh] [ebp-1ADh]
    MaterialConstantDef *v8; // [esp+Ch] [ebp-1ACh]
    MaterialTextureDef *v9; // [esp+10h] [ebp-1A8h]
    unsigned int v10; // [esp+14h] [ebp-1A4h]
    float *literal; // [esp+24h] [ebp-194h]
    unsigned __int8 *memory; // [esp+28h] [ebp-190h]
    unsigned int oredSurfaceTypeBits; // [esp+2Ch] [ebp-18Ch]
    unsigned int texIndex; // [esp+30h] [ebp-188h]
    unsigned int texTableSize; // [esp+34h] [ebp-184h]
    unsigned __int8 andedGameFlags; // [esp+43h] [ebp-175h]
    unsigned int constTableSize; // [esp+44h] [ebp-174h]
    MaterialTextureDef *newTexEntry; // [esp+48h] [ebp-170h]
    unsigned __int8 oredGameFlags; // [esp+4Fh] [ebp-169h]
    unsigned int stateBitsTable[34][2]; // [esp+50h] [ebp-168h] BYREF
    const MaterialConstantDef *oldConstTable; // [esp+164h] [ebp-54h]
    unsigned int tintConstNameHash; // [esp+168h] [ebp-50h]
    MaterialConstantDef *newConstEntry; // [esp+16Ch] [ebp-4Ch]
    bool isTintSpecified; // [esp+172h] [ebp-46h]
    unsigned __int8 constantCount; // [esp+173h] [ebp-45h]
    const MaterialTextureDef *oldTexTable; // [esp+174h] [ebp-44h]
    unsigned __int8 stateBitsEntry[34]; // [esp+178h] [ebp-40h] BYREF
    Material *newMtl; // [esp+1A0h] [ebp-18h]
    unsigned int layerIndex; // [esp+1A4h] [ebp-14h]
    unsigned int constIndex; // [esp+1A8h] [ebp-10h]
    unsigned __int8 textureCount; // [esp+1AFh] [ebp-9h]
    unsigned int stateBitsCount; // [esp+1B0h] [ebp-8h]
    char layerChar; // [esp+1B7h] [ebp-1h]

    andedGameFlags = -1;
    oredGameFlags = 0;
    textureCount = 0;
    constantCount = 0;
    oredSurfaceTypeBits = 0;
    tintConstNameHash = R_HashString("colorTint");
    for (layerIndex = 0; layerIndex < layerCount; ++layerIndex)
    {
        andedGameFlags &= layerMtl[layerIndex]->info.gameFlags;
        oredGameFlags |= layerMtl[layerIndex]->info.gameFlags;
        textureCount += layerMtl[layerIndex]->textureCount;
        constantCount += layerMtl[layerIndex]->constantCount;
        oredSurfaceTypeBits |= layerMtl[layerIndex]->info.surfaceTypeBits;
        if (!Material_HasConstant(layerMtl[layerIndex], tintConstNameHash))
            ++constantCount;
    }
    stateBitsCount = Material_CreateLayeredStateBitsTable(layerMtl, layerCount, techSet, stateBitsEntry, stateBitsTable);
    v10 = strlen(name);
    texTableSize = 12 * textureCount;
    constTableSize = 32 * constantCount;
    memory = Material_Alloc(v10 + 1 + texTableSize + constTableSize + 80);
    memset(memory, 0, v10 + 1 + texTableSize + constTableSize + 80);
    newMtl = (Material*)memory;
    if (texTableSize)
        v9 = (MaterialTextureDef*)(memory + 80);
    else
        v9 = 0;
    newMtl->textureTable = v9;
    if (constTableSize)
        v8 = (MaterialConstantDef*)&memory[texTableSize + 80];
    else
        v8 = 0;
    newMtl->constantTable = v8;
    newMtl->techniqueSet = techSet;
    newMtl->info.name = (const char*)&memory[texTableSize + 80 + constTableSize];
    memcpy((void*)newMtl->info.name, name, v10 + 1);
    newMtl->info.gameFlags = oredGameFlags & 0xFB | andedGameFlags & 4;
    newMtl->info.sortKey = (*layerMtl)->info.sortKey;
    newMtl->info.surfaceTypeBits = oredSurfaceTypeBits;
    newMtl->textureCount = textureCount;
    newMtl->constantCount = constantCount;
    v4 = newMtl->stateBitsEntry;
    qmemcpy(newMtl->stateBitsEntry, stateBitsEntry, 0x20u);
    *((_WORD *)v4 + 16) = *(_WORD *)&stateBitsEntry[32];
    Material_SetStateBits(newMtl, stateBitsTable, stateBitsCount);
    newTexEntry = newMtl->textureTable;
    newConstEntry = newMtl->constantTable;
    for (layerIndex = 0; layerIndex < layerCount; ++layerIndex)
    {
        if (layerIndex)
            v7 = layerIndex + 48;
        else
            v7 = 0;
        layerChar = v7;
        oldTexTable = layerMtl[layerIndex]->textureTable;
        for (texIndex = 0; texIndex < layerMtl[layerIndex]->textureCount; ++texIndex)
        {
            v5 = &oldTexTable[texIndex];
            newTexEntry->nameHash = v5->nameHash;
            newTexEntry->nameStart = v5->nameStart;
            newTexEntry->nameEnd = v5->nameEnd;
            newTexEntry->samplerState= v5->samplerState;
            newTexEntry->semantic= v5->semantic;
            newTexEntry->u.image = v5->u.image;
            if ((newTexEntry->samplerState & 0x18) == 8 && (newTexEntry->semantic == 2 || newTexEntry->semantic == 5))
            {
                newTexEntry->samplerState &= 0xE7u;
                newTexEntry->samplerState |= 0x10u;
            }
            if (layerChar)
            {
                newTexEntry->nameEnd = layerChar;
                newTexEntry->nameHash = layerChar ^ (33 * newTexEntry->nameHash);
            }
            ++newTexEntry;
        }
        oldConstTable = layerMtl[layerIndex]->constantTable;
        isTintSpecified = 0;
        for (constIndex = 0; constIndex < layerMtl[layerIndex]->constantCount; ++constIndex)
        {
            qmemcpy(newConstEntry, &oldConstTable[constIndex], sizeof(MaterialConstantDef));
            if (layerChar)
            {
                Material_AppendCharToConstName(newConstEntry->name, layerChar);
                newConstEntry->nameHash = layerChar ^ (33 * newConstEntry->nameHash);
            }
            if (oldConstTable[constIndex].nameHash == tintConstNameHash)
                isTintSpecified = 1;
            ++newConstEntry;
        }
        if (!isTintSpecified)
        {
            strncpy(newConstEntry->name, "colorTint", 0xCu);
            newConstEntry->nameHash = tintConstNameHash;
            literal = newConstEntry->literal;
            newConstEntry->literal[0] = 1.0;
            literal[1] = 1.0;
            literal[2] = 1.0;
            literal[3] = 1.0;
            if (layerChar)
            {
                Material_AppendCharToConstName(newConstEntry->name, layerChar);
                newConstEntry->nameHash = layerChar ^ (33 * newConstEntry->nameHash);
            }
            ++newConstEntry;
        }
    }
    if (newTexEntry - newMtl->textureTable != newMtl->textureCount)
        MyAssertHandler(
            ".\\r_material_load_obj.cpp",
            6368,
            0,
            "newTexEntry - newMtl->textureTable == newMtl->textureCount\n\t%i, %i",
            newTexEntry - newMtl->textureTable,
            newMtl->textureCount);
    if (newConstEntry - newMtl->constantTable != newMtl->constantCount)
        MyAssertHandler(
            ".\\r_material_load_obj.cpp",
            6369,
            0,
            "newConstEntry - newMtl->constantTable == newMtl->constantCount\n\t%i, %i",
            newConstEntry - newMtl->constantTable,
            newMtl->constantCount);
    qsort(newMtl->textureTable, newMtl->textureCount, 0xCu, (int(*)(const void*, const void*))CompareHashedMaterialTextures);
    qsort(newMtl->constantTable, newMtl->constantCount, 0x20u, (int(*)(const void *, const void *))CompareHashedMaterialTextures);
    Material_SetMaterialDrawRegion(newMtl);
    if (Material_Validate(newMtl))
        return newMtl;
    else
        return 0;
}

Material *__cdecl Material_LoadLayered(char *assetName)
{
    int v1; // edx
    int bspMaterialIndex; // [esp+8h] [ebp-30h]
    bool hasError; // [esp+Fh] [ebp-29h]
    int bspVersion; // [esp+10h] [ebp-28h]
    const char *name; // [esp+14h] [ebp-24h]
    unsigned int layerCount; // [esp+18h] [ebp-20h]
    bool expectNormal; // [esp+1Fh] [ebp-19h]
    const Material *mtl[5]; // [esp+20h] [ebp-18h] BYREF
    MaterialTechniqueSet *techSet; // [esp+34h] [ebp-4h]

    iassert( assetName[0] == '*' );
    name = assetName + 1;
    layerCount = 0;
    hasError = 0;
    bspVersion = Com_GetBspVersion();
    while (1)
    {
        bspMaterialIndex = 0;
        while (isdigit(*name))
            bspMaterialIndex = 10 * bspMaterialIndex + *name++ - 48;
        iassert( layerCount < MTL_LAYER_LIMIT );
        mtl[layerCount] = R_GetBspMaterial(bspMaterialIndex);
        if (bspVersion < 10)
        {
            if (*name != 110 && *name != 120)
                MyAssertHandler(
                    ".\\r_material_load_obj.cpp",
                    6561,
                    0,
                    "%s\n\t(assetName) = %s",
                    "(*name == 'n' || *name == 'x')",
                    assetName);
            expectNormal = *name == 110;
            name += 7;
        }
        else
        {
            if (*name != 110 && *name != 95 && *name)
                MyAssertHandler(
                    ".\\r_material_load_obj.cpp",
                    6553,
                    0,
                    "%s\n\t(assetName) = %s",
                    "(*name == 'n' || *name == '_' || *name == '\\0')",
                    assetName);
            v1 = *name;
            expectNormal = v1 == 110;
            if (v1 == 110)
                ++name;
        }
        if (Material_IsDefault(mtl[layerCount]))
        {
            hasError = 1;
        }
        else if (expectNormal != Material_HasNormalMap(mtl[layerCount]))
        {
            if (expectNormal)
                Com_PrintError(
                    1,
                    "In layered material, expected material '%s' %s; using default instead.  Recompile the bsp to fix.\n",
                    mtl[layerCount]->info.name,
                    "without a normal map to have one");
            else
                Com_PrintError(
                    1,
                    "In layered material, expected material '%s' %s; using default instead.  Recompile the bsp to fix.\n",
                    mtl[layerCount]->info.name,
                    "with a normal map to not have one");
            hasError = 1;
        }
        ++layerCount;
        if (!*name)
            break;
        iassert( *name == '_' );
        ++name;
    }
    if (hasError)
        return 0;
    techSet = Material_RegisterLayeredTechniqueSet(mtl, layerCount);
    if (!techSet)
        return 0;
    else
        return Material_CreateLayered(assetName, mtl, layerCount, techSet);
}

MaterialTypeInfo g_materialTypeInfo[5] =
{
  { "", "", 0u},
  { "m/", "m_", 2u },
  { "mc/", "mc_", 3u },
  { "w/", "w_", 2u },
  { "wc/", "wc_", 3u }
}; // idb

unsigned int __cdecl Material_LoadFile(const char *filename, int *file)
{
    char fullFilename[68]; // [esp+0h] [ebp-48h] BYREF

    Com_sprintf(fullFilename, 0x40u, "materials/%s", filename);
    return FS_FOpenFileRead(fullFilename, file);
}

char __cdecl Material_HasTexture(const Material *mtl, unsigned int nameHash)
{
    unsigned int textureIndex; // [esp+0h] [ebp-4h]

    for (textureIndex = 0; textureIndex < mtl->textureCount; ++textureIndex)
    {
        if (mtl->textureTable[textureIndex].nameHash == nameHash)
            return 1;
    }
    return 0;
}

const char *__cdecl Material_StringFromHash(unsigned int hash)
{
    unsigned int hashIndex; // [esp+0h] [ebp-4h]

    for (hashIndex = hash & 0x3F; mtlLoadGlob.stringHashTable[hashIndex].string; hashIndex = (hashIndex + 1) & 0x3F)
    {
        if (mtlLoadGlob.stringHashTable[hashIndex].hash == hash)
            return mtlLoadGlob.stringHashTable[hashIndex].string;
    }
    return 0;
}

char __cdecl Material_ValidatePassArguments(
    const Material *mtl,
    const char *techniqueName,
    unsigned int argCount,
    const MaterialShaderArgument *args)
{
    unsigned int argIndex; // [esp+0h] [ebp-8h]
    const char *argName; // [esp+4h] [ebp-4h]
    const char *argNamea; // [esp+4h] [ebp-4h]

    for (argIndex = 0; argIndex < argCount; ++argIndex)
    {
        if (args[argIndex].type && args[argIndex].type != 6)
        {
            if (args[argIndex].type == 2 && !Material_HasTexture(mtl, args[argIndex].u.codeSampler))
            {
                argNamea = Material_StringFromHash(args[argIndex].u.codeSampler);
                Com_PrintError(
                    8,
                    "material '%s' using technique '%s' from techniqueSet '%s' doesn't expose a '%s' texture\n",
                    mtl->info.name,
                    techniqueName,
                    mtl->techniqueSet->name,
                    argNamea);
                return 0;
            }
        }
        else if (!Material_HasConstant(mtl, args[argIndex].u.codeSampler))
        {
            argName = Material_StringFromHash(args[argIndex].u.codeSampler);
            Com_PrintError(
                8,
                "material '%s' using technique '%s' from techniqueSet '%s' doesn't expose a '%s' constant\n",
                mtl->info.name,
                techniqueName,
                mtl->techniqueSet->name,
                argName);
            return 0;
        }
    }
    return 1;
}

char __cdecl Material_ValidateTechnique(const Material *material, const MaterialTechnique *technique)
{
    unsigned int passIndex; // [esp+8h] [ebp-4h]

    for (passIndex = 0; passIndex < technique->passCount; ++passIndex)
    {
        if (!Material_ValidatePassArguments(
            material,
            technique->name,
            technique->passArray[passIndex].stableArgCount
            + technique->passArray[passIndex].perObjArgCount
            + technique->passArray[passIndex].perPrimArgCount,
            technique->passArray[passIndex].args))
            return 0;
    }
    return 1;
}

char __cdecl Material_Validate(const Material *material)
{
    int techType; // [esp+0h] [ebp-4h]

    for (techType = 0; techType < 34; ++techType)
    {
        if (material->techniqueSet->techniques[techType]
            && !Material_ValidateTechnique(material, material->techniqueSet->techniques[techType]))
        {
            return 0;
        }
    }
    return 1;
}

BOOL __cdecl R_IsWorldMaterialType(unsigned int materialType)
{
    return materialType == 3 || materialType == 4;
}

water_t *__cdecl Material_RegisterWaterImage(const MaterialWaterDef *water)
{
    int v2; // [esp+0h] [ebp-5Ch]
    int v3; // [esp+4h] [ebp-58h]
    int textureWidth; // [esp+10h] [ebp-4Ch]
    water_t setup; // [esp+14h] [ebp-48h] BYREF

    textureWidth = water->textureWidth;
    if (textureWidth != textureWidth)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
            281,
            0,
            "i == static_cast< Type >( i )\n\t%i, %i",
            textureWidth,
            textureWidth);
    setup.M = textureWidth;
    setup.N = textureWidth;
    setup.Lx = water->horizontalWorldLength;
    setup.Lz = water->verticalWorldLength;
    setup.gravity = 800.0;
    setup.windvel = water->windSpeed;
    setup.winddir[0] = water->windDirection[0];
    setup.winddir[1] = water->windDirection[1];
    setup.amplitude = water->amplitude;
    setup.image = 0;
    if (textureWidth >> r_picmip_water->current.integer < 4)
        v3 = 4;
    else
        v3 = textureWidth >> r_picmip_water->current.integer;
    setup.M = v3;
    if (setup.N >> r_picmip_water->current.integer < 4)
        v2 = 4;
    else
        v2 = setup.N >> r_picmip_water->current.integer;
    setup.N = v2;
    return R_LoadWaterSetup(&setup);
}

int __cdecl CompareRawMaterialTextures(_DWORD *e0, _DWORD *e1)
{
    unsigned int v2; // esi
    const char *name_4; // [esp+8h] [ebp-4h]

    name_4 = (const char*)mtlLoadGlob.sortMtlRaw + *e1;
    v2 = R_HashString((const char*)mtlLoadGlob.sortMtlRaw + *e0);
    return v2 < R_HashString(name_4) ? -1 : 1;
}

BOOL __cdecl Material_RegisterImage(
    const MaterialRaw *material,
    int imageNameOffset,
    unsigned __int8 semantic,
    int imageTrack)
{
    return Image_Register((const char*)material + imageNameOffset, semantic, imageTrack) != 0;
}

BOOL __cdecl Material_FinishLoadingTexdef(
    const MaterialRaw *material,
    MaterialTextureDefRaw *texdef,
    unsigned int materialType,
    int imageTrack)
{
    iassert( texdef );
    if (material->info.sortKey == 4
        && R_IsWorldMaterialType(materialType)
        && (texdef->samplerState & 0x18) == 8
        && (texdef->semantic == 2 || texdef->semantic == 5))
    {
        texdef->samplerState &= 0xE7u;
        texdef->samplerState |= 0x10u;
    }
    if (texdef->semantic == 11)
        return Material_RegisterWaterImage((const MaterialWaterDef*)(material + texdef->u.imageNameOffset)) != 0;
    else
        return Material_RegisterImage(material, texdef->u.imageNameOffset, texdef->semantic, imageTrack);
}

bool __cdecl Material_FinishLoadingInstance(
    const MaterialRaw *mtlRaw,
    const char *techniqueSetVertDeclPrefix,
    MaterialTechniqueSet **techniqueSet,
    unsigned int materialType,
    int imageTrack)
{
    MaterialConstantDefRaw *constantTable; // [esp+0h] [ebp-118h]
    int constantIndex; // [esp+4h] [ebp-114h]
    char techniqueSetName[260]; // [esp+8h] [ebp-110h] BYREF
    MaterialTextureDefRaw *textureTable; // [esp+110h] [ebp-8h]
    int textureIndex; // [esp+114h] [ebp-4h]

    iassert( mtlRaw );
    if (mtlRaw->info.sortKey >= 0x40u)
        MyAssertHandler(
            ".\\r_material_load_obj.cpp",
            6112,
            0,
            "mtlRaw->info.sortKey doesn't index 1 << MTL_SORT_PRIMARY_SORT_KEY_BITS\n\t%i not in [0, %i)",
            mtlRaw->info.sortKey,
            64);
    textureTable = (MaterialTextureDefRaw*)((char*)mtlRaw + mtlRaw->textureTableOffset);
    for (textureIndex = 0; textureIndex < mtlRaw->textureCount; ++textureIndex)
    {
        if (!Material_FinishLoadingTexdef(mtlRaw, &textureTable[textureIndex], materialType, imageTrack))
            return 0;
    }
    mtlLoadGlob.sortMtlRaw = mtlRaw;
    qsort(textureTable, mtlRaw->textureCount, 0xCu, (int(*)(const void*, const void*))CompareRawMaterialTextures);
    mtlLoadGlob.sortMtlRaw = 0;
    constantTable = (MaterialConstantDefRaw*)((char*)mtlRaw + mtlRaw->constantTableOffset);
    for (constantIndex = 0; constantIndex < mtlRaw->constantCount; ++constantIndex)
    {
        if (!constantTable[constantIndex].nameOffset)
            return 0;
    }
    mtlLoadGlob.sortMtlRaw = mtlRaw;
    qsort(constantTable, mtlRaw->constantCount, 0x14u, (int(*)(const void *, const void *))CompareRawMaterialTextures);
    mtlLoadGlob.sortMtlRaw = 0;
    Com_sprintf(techniqueSetName, 0x100u, "%s%s", techniqueSetVertDeclPrefix, (char*) mtlRaw + mtlRaw->techSetNameOffset);
    *techniqueSet = Material_RegisterTechniqueSet(techniqueSetName);
    return *techniqueSet != 0;
}

void __cdecl Material_ApplyStateBitsRemapRuleSet(
    const Material *material,
    const MaterialStateMap *stateMap,
    unsigned int ruleSetIndex,
    const unsigned int *refStateBits,
    unsigned int *stateBitsOut)
{
    const MaterialStateMapRuleSet *ruleSet; // [esp+4h] [ebp-8h]
    int ruleIndex; // [esp+8h] [ebp-4h]

    ruleSet = stateMap->ruleSet[ruleSetIndex];
    for (ruleIndex = 0; ruleIndex < ruleSet->ruleCount; ++ruleIndex)
    {
        if ((ruleSet->rules[ruleIndex].stateBitsMask[0] & *refStateBits) == ruleSet->rules[ruleIndex].stateBitsValue[0]
            && (ruleSet->rules[ruleIndex].stateBitsMask[1] & refStateBits[1]) == ruleSet->rules[ruleIndex].stateBitsValue[1])
        {
            *stateBitsOut = ruleSet->rules[ruleIndex].stateBitsSet[0]
                | ruleSet->rules[ruleIndex].stateBitsClear[0] & *stateBitsOut;
            stateBitsOut[1] = ruleSet->rules[ruleIndex].stateBitsSet[1]
                | ruleSet->rules[ruleIndex].stateBitsClear[1] & stateBitsOut[1];
            return;
        }
    }
    Com_Error(
        ERR_FATAL,
        "No rule in stateMap '%s' rule set %i matched the current material state for material '%s'",
        stateMap->name,
        ruleSetIndex,
        material->info.name);
}

void __cdecl Material_RemapStateBits(
    const Material *material,
    __int16 toolFlags,
    const MaterialStateMap *stateMap,
    const unsigned int *refStateBits,
    unsigned int *stateBitsOut)
{
    unsigned int ruleSetIndex; // [esp+Ch] [ebp-4h]

    *stateBitsOut = *refStateBits;
    stateBitsOut[1] = refStateBits[1];
    for (ruleSetIndex = 0; ruleSetIndex < 0xA; ++ruleSetIndex)
        Material_ApplyStateBitsRemapRuleSet(material, stateMap, ruleSetIndex, refStateBits, stateBitsOut);
    if ((toolFlags & 0x200) == 0 && (stateBitsOut[1] & 0x30) == 0x10)
    {
        stateBitsOut[1] &= 0xFFFFFFCF;
        stateBitsOut[1] = stateBitsOut[1];
    }
}

unsigned int __cdecl Material_GetCullFlags(Material *material)
{
    unsigned int techType; // [esp+4h] [ebp-18h]
    unsigned int cullBits; // [esp+8h] [ebp-14h]
    unsigned int cullFlags; // [esp+Ch] [ebp-10h]
    unsigned int techTypeCullFlags; // [esp+10h] [ebp-Ch]
    MaterialTechniqueSet *techniqueSet; // [esp+14h] [ebp-8h]

    cullFlags = -1;
    techniqueSet = material->techniqueSet;
    iassert( techniqueSet );
    for (techType = 7; techType < 0x15; ++techType)
    {
        if (techniqueSet->techniques[techType])
        {
            cullBits = material->stateBitsTable[material->stateBitsEntry[techType]].loadBits[0] & 0xC000;
            if (cullBits == 0x8000)
            {
                techTypeCullFlags = 1;
            }
            else if (cullBits == 49152)
            {
                techTypeCullFlags = 2;
            }
            else
            {
                techTypeCullFlags = 0;
            }
            if (cullFlags != -1 && cullFlags != techTypeCullFlags)
                MyAssertHandler(
                    ".\\r_material_load_obj.cpp",
                    5738,
                    0,
                    "%s",
                    "(cullFlags == ~0u) || (cullFlags == techTypeCullFlags)");
            cullFlags = techTypeCullFlags;
        }
    }
    if (cullFlags == -1)
        return 0;
    return cullFlags;
}

unsigned int __cdecl Material_GetCullShadowFlags(Material *material)
{
    unsigned int cullBits; // [esp+4h] [ebp-10h]
    MaterialTechniqueSet *techniqueSet; // [esp+Ch] [ebp-8h]

    techniqueSet = material->techniqueSet;
    iassert( techniqueSet );
    if (!techniqueSet->techniques[2])
        return 0;
    cullBits = material->stateBitsTable[material->stateBitsEntry[2]].loadBits[0] & 0xC000;
    if (cullBits == 0x8000)
        return 64;
    if (cullBits == 49152)
        return 128;
    return 0;
}

int __cdecl Material_GetDecalFlags(const Material *mtl)
{
    int v2; // [esp+0h] [ebp-Ch]
    MaterialTechniqueSet *techniqueSet; // [esp+8h] [ebp-4h]

    if (!mtl->stateBitsTable)
        return 0;
    techniqueSet = mtl->techniqueSet;
    iassert( techniqueSet );
    if (techniqueSet->techniques[4])
        v2 = mtl->stateBitsEntry[4];
    else
        v2 = 0;
    return (mtl->stateBitsTable[v2].loadBits[1] & 0x30) != 0 ? 4 : 0;
}

int __cdecl Material_GetWritesDepthFlags(const Material *mtl)
{
    int v2; // [esp+0h] [ebp-Ch]
    MaterialTechniqueSet *techniqueSet; // [esp+8h] [ebp-4h]

    if (!mtl->stateBitsTable)
        return 0;
    techniqueSet = mtl->techniqueSet;
    iassert( techniqueSet );
    if (techniqueSet->techniques[4])
        v2 = mtl->stateBitsEntry[4];
    else
        v2 = 0;
    return (mtl->stateBitsTable[v2].loadBits[1] & 1) != 0 ? 8 : 0;
}

unsigned int __cdecl Material_GetUsesDepthBufferFlags(const Material *mtl)
{
    const MaterialTechnique *technique; // [esp+0h] [ebp-14h]
    unsigned int techType; // [esp+4h] [ebp-10h]
    MaterialTechniqueSet *techniqueSet; // [esp+8h] [ebp-Ch]
    const GfxStateBits *refStateBits; // [esp+Ch] [ebp-8h]
    unsigned int passIndex; // [esp+10h] [ebp-4h]

    techniqueSet = mtl->techniqueSet;
    iassert( techniqueSet );
    for (techType = 0; techType < 0x22; ++techType)
    {
        technique = techniqueSet->techniques[techType];
        if (technique)
        {
            for (passIndex = 0; passIndex < technique->passCount; ++passIndex)
            {
                refStateBits = &mtl->stateBitsTable[passIndex + mtl->stateBitsEntry[techType]];
                if ((refStateBits->loadBits[1] & 0x3D) != 0)
                    return 16;
                if ((refStateBits->loadBits[1] & 2) == 0)
                    return 16;
            }
        }
    }
    return 0;
}

unsigned int __cdecl Material_GetUsesStencilBufferFlags(const Material *mtl)
{
    const MaterialTechnique *technique; // [esp+0h] [ebp-14h]
    unsigned int techType; // [esp+4h] [ebp-10h]
    MaterialTechniqueSet *techniqueSet; // [esp+8h] [ebp-Ch]
    unsigned int passIndex; // [esp+10h] [ebp-4h]

    techniqueSet = mtl->techniqueSet;
    iassert( techniqueSet );
    for (techType = 0; techType < 0x22; ++techType)
    {
        technique = techniqueSet->techniques[techType];
        if (technique)
        {
            for (passIndex = 0; passIndex < technique->passCount; ++passIndex)
            {
                if ((mtl->stateBitsTable[passIndex + mtl->stateBitsEntry[techType]].loadBits[1] & 0xC0) != 0)
                    return 32;
            }
        }
    }
    return 0;
}

void __cdecl Material_UpdateStateFlags(Material *mtl)
{
    unsigned int CullFlags; // esi
    unsigned int v2; // esi
    int v3; // esi
    int v4; // esi
    unsigned int v5; // esi
    unsigned int stateFlags; // [esp+4h] [ebp-8h]

    if (mtl->techniqueSet)
    {
        CullFlags = Material_GetCullFlags(mtl);
        v2 = Material_GetCullShadowFlags(mtl) | CullFlags;
        v3 = Material_GetDecalFlags(mtl) | v2;
        v4 = Material_GetWritesDepthFlags(mtl) | v3;
        v5 = Material_GetUsesDepthBufferFlags(mtl) | v4;
        stateFlags = Material_GetUsesStencilBufferFlags(mtl) | v5;
    }
    else
    {
        LOBYTE(stateFlags) = 0;
    }
    mtl->stateFlags = stateFlags;
}

void __cdecl Material_SetStateBits(Material *material, unsigned int (*stateBitsTable)[2], unsigned int stateBitsCount)
{
    unsigned __int8 *v3; // [esp+0h] [ebp-4h]

    material->stateBitsCount = stateBitsCount;
    if (stateBitsCount)
        v3 = Material_Alloc(8 * stateBitsCount);
    else
        v3 = 0;
    material->stateBitsTable = (GfxStateBits*)v3;
    memcpy(material->stateBitsTable, stateBitsTable, 8 * stateBitsCount);
    Material_UpdateStateFlags(material);
}

void __cdecl Material_BuildStateBitsTable(Material *material, __int16 toolFlags, const unsigned int *refStateBits)
{
    MaterialTechnique *technique; // [esp+0h] [ebp-474h]
    unsigned int techType; // [esp+4h] [ebp-470h]
    const MaterialStateMap **stateMapTable; // [esp+8h] [ebp-46Ch]
    unsigned int stateBitsTable[136][2]; // [esp+Ch] [ebp-468h] BYREF
    unsigned int stateBitsForPass[4][2]; // [esp+44Ch] [ebp-28h] BYREF
    unsigned int stateBitsCount; // [esp+46Ch] [ebp-8h] BYREF
    unsigned int passIndex; // [esp+470h] [ebp-4h]

    stateBitsCount = 0;
    for (techType = 0; techType < 0x22; ++techType)
    {
        technique = material->techniqueSet->techniques[techType];
        if (technique)
        {
            stateMapTable = (const MaterialStateMap **)&technique->passArray[technique->passCount];
            for (passIndex = 0; passIndex < technique->passCount; ++passIndex)
                Material_RemapStateBits(
                    material,
                    toolFlags,
                    stateMapTable[passIndex],
                    refStateBits,
                    stateBitsForPass[passIndex]);
            material->stateBitsEntry[techType] = Material_AddStateBitsArrayToTable(
                stateBitsForPass,
                technique->passCount,
                stateBitsTable,
                &stateBitsCount);
        }
        else
        {
            material->stateBitsEntry[techType] = -1;
        }
    }
    Material_SetStateBits(material, stateBitsTable, stateBitsCount);
}

unsigned int __cdecl Material_GetTechniqueSetDrawRegion(MaterialTechniqueSet *techniqueSet)
{
    int techTypeIter; // [esp+4h] [ebp-Ch]
    unsigned int cameraRegion; // [esp+Ch] [ebp-4h]

    iassert( techniqueSet );
    if (techniqueSet->techniques[7])
    {
        cameraRegion = 0;
    }
    else if (techniqueSet->techniques[5])
    {
        cameraRegion = 2;
    }
    else
    {
        cameraRegion = 3;
    }
    for (techTypeIter = 7; techTypeIter != 14; ++techTypeIter)
    {
        if (g_useTechnique[techTypeIter] && (cameraRegion == 0) != (techniqueSet->techniques[techTypeIter] != 0))
            MyAssertHandler(
                ".\\r_material_load_obj.cpp",
                5504,
                0,
                "%s\n\t(techniqueSet->name) = %s",
                "(!g_useTechnique[techTypeIter] || (cameraRegion == CAMERA_REGION_LIT) == (techniqueSet->techniques[techTypeIter] != 0))",
                techniqueSet->name);
    }
    return cameraRegion;
}

void __cdecl Material_SetMaterialDrawRegion(Material *material)
{
    unsigned int cameraRegion; // [esp+0h] [ebp-4h]

    cameraRegion = Material_GetTechniqueSetDrawRegion(material->techniqueSet);
    if (!cameraRegion)
        LOBYTE(cameraRegion) = material->info.sortKey >= 0x18u;
    material->cameraRegion = cameraRegion;
}

Material *__cdecl Material_LoadRaw(const MaterialRaw *mtlRaw, unsigned int materialType, int imageTrack)
{
    int v4; // edx
    unsigned int v5; // eax
    float *literal; // [esp+34h] [ebp-48h]
    float *v8; // [esp+38h] [ebp-44h]
    unsigned int texIndex; // [esp+40h] [ebp-3Ch]
    char *constName; // [esp+44h] [ebp-38h]
    char *strDest; // [esp+48h] [ebp-34h]
    const MaterialConstantDefRaw *constantTableRaw; // [esp+4Ch] [ebp-30h]
    char surfIndex; // [esp+50h] [ebp-2Ch]
    Material *material; // [esp+58h] [ebp-24h]
    const char *tableEntryName; // [esp+60h] [ebp-1Ch]
    char *name; // [esp+64h] [ebp-18h]
    unsigned int prefixLen; // [esp+68h] [ebp-14h]
    MaterialTechniqueSet *techniqueSet; // [esp+6Ch] [ebp-10h] BYREF
    const MaterialTextureDefRaw *textureTableRaw; // [esp+70h] [ebp-Ch]
    unsigned int constIndex; // [esp+74h] [ebp-8h]
    void *materialMem; // [esp+78h] [ebp-4h]

    if (!Material_FinishLoadingInstance(
        mtlRaw,
        g_materialTypeInfo[materialType].techniqueSetPrefix,
        &techniqueSet,
        materialType,
        imageTrack))
        return 0;
    name = (char*)mtlRaw + mtlRaw->info.nameOffset;
    prefixLen = g_materialTypeInfo[materialType].prefixLen;
    materialMem = Material_Alloc(prefixLen + strlen(name) + 1 + sizeof(Material));
    material = (Material*)materialMem;
    strDest = (char*)materialMem + sizeof(Material);
    memcpy(strDest, g_materialTypeInfo[materialType].prefix, prefixLen);
    memcpy(&strDest[prefixLen], name, strlen(name) + 1);
    material->info.name = strDest;
    iassert(!material->info.drawSurf.fields.materialSortedIndex);
    material->info.gameFlags = mtlRaw->info.gameFlags;
    v4 = (mtlRaw->info.surfaceFlags & 0x1F00000) >> 20;
    surfIndex = v4;
    if (v4)
    {
        if (v4 >= 0x1Du)
            MyAssertHandler(
                ".\\r_material_load_obj.cpp",
                6685,
                0,
                "%s\n\t(surfIndex) = %i",
                "(surfIndex > 0 && surfIndex < 29)",
                v4);
        material->info.surfaceTypeBits = 1 << (surfIndex - 1);
    }
    else
    {
        material->info.surfaceTypeBits = 0;
    }
    if (materialType != 3 && materialType != 4)
        material->info.gameFlags &= ~2u;
    material->info.sortKey = mtlRaw->info.sortKey;
    material->info.textureAtlasRowCount = mtlRaw->info.textureAtlasRowCount;
    material->info.textureAtlasColumnCount = mtlRaw->info.textureAtlasColumnCount;
    material->textureCount = mtlRaw->textureCount;
    material->constantCount = mtlRaw->constantCount;
    material->techniqueSet = techniqueSet;
    if (mtlRaw->textureCount)
    {
        material->textureTable = (MaterialTextureDef*)Material_Alloc(12 * mtlRaw->textureCount);
        textureTableRaw = (const MaterialTextureDefRaw*)((char*)mtlRaw + mtlRaw->textureTableOffset);
        for (texIndex = 0; texIndex < mtlRaw->textureCount; ++texIndex)
        {
            tableEntryName = (const char*)mtlRaw + textureTableRaw[texIndex].nameOffset;
            material->textureTable[texIndex].nameHash = R_HashString(tableEntryName);
            material->textureTable[texIndex].nameStart = *tableEntryName;
            material->textureTable[texIndex].nameEnd = tableEntryName[strlen(tableEntryName) - 1];
            material->textureTable[texIndex].samplerState = textureTableRaw[texIndex].samplerState;
            if ((material->textureTable[texIndex].samplerState & 7) == 0)
                MyAssertHandler(
                    ".\\r_material_load_obj.cpp",
                    6733,
                    0,
                    "%s",
                    "material->textureTable[texIndex].samplerState & SAMPLER_FILTER_MASK");
            material->textureTable[texIndex].semantic = textureTableRaw[texIndex].semantic;
            if (material->textureTable[texIndex].semantic == 11)
                material->textureTable[texIndex].u.image = (GfxImage*)Material_RegisterWaterImage((const MaterialWaterDef*)(mtlRaw + textureTableRaw[texIndex].u.imageNameOffset));
            else
                material->textureTable[texIndex].u.image = Image_Register(
                    (const char*)mtlRaw + textureTableRaw[texIndex].u.imageNameOffset,
                    textureTableRaw[texIndex].semantic,
                    imageTrack);
        }
    }
    if (mtlRaw->constantCount)
    {
        material->constantTable = (MaterialConstantDef*)Material_Alloc(32 * mtlRaw->constantCount);
        constantTableRaw = (const MaterialConstantDefRaw*)((char*)mtlRaw + mtlRaw->constantTableOffset);
        for (constIndex = 0; constIndex < mtlRaw->constantCount; ++constIndex)
        {
            constName = (char*)mtlRaw + constantTableRaw[constIndex].nameOffset;
            v5 = R_HashString(constName);
            material->constantTable[constIndex].nameHash = v5;
            strncpy(material->constantTable[constIndex].name, constName, 0xCu);
            literal = material->constantTable[constIndex].literal;
            v8 = (float*)constantTableRaw[constIndex].literal;
            *literal = *v8;
            literal[1] = v8[1];
            literal[2] = v8[2];
            literal[3] = v8[3];
        }
    }
    Material_BuildStateBitsTable(material, mtlRaw->info.toolFlags, mtlRaw->refStateBits);
    Material_SetMaterialDrawRegion(material);
    if (!Material_Validate(material))
        return 0;
    iassert( material->techniqueSet );
    if ((material->info.gameFlags & 0x40) != 0)
        MyAssertHandler(
            ".\\r_material_load_obj.cpp",
            6779,
            0,
            "%s",
            "!(material->info.gameFlags & MTL_GAMEFLAG_CASTS_SHADOW)");
    if ((mtlRaw->info.surfaceFlags & 0x40000) == 0
        && Material_GetTechnique(material, TECHNIQUE_BUILD_SHADOWMAP_DEPTH)
        && (material->stateFlags & 4) == 0)
    {
        material->info.gameFlags |= 0x40u;
    }
    return material;
}

Material *__cdecl Material_Load(char *assetName, int imageTrack)
{
    Material *material; // [esp+0h] [ebp-18h]
    MaterialRaw *mtlRaw; // [esp+4h] [ebp-14h]
    int fileSize; // [esp+8h] [ebp-10h]
    int fileHandle; // [esp+Ch] [ebp-Ch] BYREF
    unsigned int materialType; // [esp+10h] [ebp-8h]
    unsigned int prefixLen; // [esp+14h] [ebp-4h]

    iassert( assetName );
    iassert( assetName[0] );
    if (*assetName == 42)
        return Material_LoadLayered(assetName);
    for (materialType = 1; materialType < 5; ++materialType)
    {
        prefixLen = g_materialTypeInfo[materialType].prefixLen;
        if (!strncmp(assetName, g_materialTypeInfo[materialType].prefix, prefixLen))
        {
            assetName += prefixLen;
            break;
        }
    }
    if (materialType == 5)
        materialType = 0;
    fileSize = Material_LoadFile(assetName, &fileHandle);
    if (fileSize >= 0)
    {
        if (fileSize)
        {
            mtlRaw = (MaterialRaw*)Hunk_AllocateTempMemory(fileSize, "Material_Load");
            FS_Read((unsigned char*)mtlRaw, fileSize, fileHandle);
            FS_FCloseFile(fileHandle);
            material = Material_LoadRaw(mtlRaw, materialType, imageTrack);
            Hunk_FreeTempMemory((char*)mtlRaw);
            return material;
        }
        else
        {
            FS_FCloseFile(fileHandle);
            Com_PrintError(8, "^1ERROR: material '%s' has zero length\n", assetName);
            return 0;
        }
    }
    else
    {
        if (*assetName != 36)
            Com_PrintError(8, "^1ERROR: Couldn't find material '%s'\n", assetName);
        return 0;
    }
}

void __cdecl Material_PreLoadSingleShaderText(const char *filename, const char *subdir, GfxCachedShaderText *cached)
{
    char *buffer; // [esp+20h] [ebp-58h]
    char *buffera; // [esp+20h] [ebp-58h]
    int fileHandle; // [esp+28h] [ebp-50h] BYREF
    int fileSize; // [esp+2Ch] [ebp-4Ch]
    char filepath[68]; // [esp+30h] [ebp-48h] BYREF

    Com_sprintf(filepath, 0x40u, "shaders/%s%s", subdir, filename);
    fileSize = FS_FOpenFileRead(filepath, &fileHandle);
    iassert( fileSize >= 0 );
    buffer = (char*)Hunk_AllocAlign(strlen(subdir) + fileSize + strlen(filename) + 2, 1, "Material_PreLoadSingleShaderText", 22);
    cached->name = buffer;
    buffera = &buffer[sprintf(buffer, "%s%s", subdir, filename) + 1];
    cached->text = buffera;
    FS_Read((unsigned char*)buffera, fileSize, fileHandle);
    FS_FCloseFile(fileHandle);
    buffera[fileSize] = 0;
    cached->textSize = fileSize;
}

void __cdecl Material_PreLoadShaderTextList(
    const char **fileList,
    int fileCount,
    const char *subdir,
    GfxCachedShaderText *cached)
{
    int fileIndex; // [esp+0h] [ebp-4h]

    for (fileIndex = 0; fileIndex < fileCount; ++fileIndex)
        Material_PreLoadSingleShaderText(fileList[fileIndex], subdir, &cached[fileIndex]);
}

static bool __cdecl Material_CachedShaderTextLess(const GfxCachedShaderText &cached0, const GfxCachedShaderText &cached1)
{
    return I_stricmp(cached0.name, cached1.name) < 0;
}

void __cdecl Material_PreLoadAllShaderText()
{
    // LWSS: we are actually missing the entire shaders/ folder afaik. Instead, we have the shader_bin/ folder which are precompiled pixel and vertex shaders.
    // Therefore, I have RE'd the "CoD4EffectsEd.exe" to see what it does, and that's where this code comes from.

    // What we have in shader_bin/ is basically a list of compiled shaders, however the names are hashed and stored in this shader_names file.
#ifdef KISAK_NO_FASTFILES

    int file;
    int fileLen = FS_FOpenFileRead("shader_bin/shader_names", &file);

    if (fileLen >= 0)
    {
        FS_Read((unsigned char*)&g_vertexNamesCount, sizeof(int), file);
        g_vertexNamesList = (ShaderBinNames *)Z_Malloc(sizeof(ShaderBinNames) * g_vertexNamesCount, "shader_names list", 69);
        FS_Read((unsigned char *)g_vertexNamesList, sizeof(ShaderBinNames) * g_vertexNamesCount, file);

        FS_Read((unsigned char *)&g_pixelNamesCount, sizeof(int), file);
        g_pixelNamesList = (ShaderBinNames *)Z_Malloc(sizeof(ShaderBinNames) * g_pixelNamesCount, "shader_names list", 69);
        FS_Read((unsigned char *)g_pixelNamesList, sizeof(ShaderBinNames) * g_pixelNamesCount, file);

        FS_FCloseFile(file);
    }
#else
    int fileCountLib; // [esp+154h] [ebp-8h] BYREF
    const char **shaderListLib; // [esp+158h] [ebp-4h]

    shaderListLib = FS_ListFiles("shaders/lib/", "hlsl", FS_LIST_PURE_ONLY, &fileCountLib);
    mtlLoadGlob.cachedShaderCount = fileCountLib;
    mtlLoadGlob.cachedShaderText = (GfxCachedShaderText *)Hunk_Alloc(sizeof(GfxCachedShaderText) * fileCountLib, "Material_PreLoadShaderText", 22);
    Material_PreLoadShaderTextList(shaderListLib, fileCountLib, "lib/", mtlLoadGlob.cachedShaderText);
    //std::_Sort<GfxCachedShaderText *, int, bool(__cdecl *)(GfxCachedShaderText const &, GfxCachedShaderText const &)>(
    //    mtlLoadGlob.cachedShaderText,
    //    &mtlLoadGlob.cachedShaderText[mtlLoadGlob.cachedShaderCount],
    //    (12 * mtlLoadGlob.cachedShaderCount) / 12,
    //    Material_CachedShaderTextLess);
    std::sort(&mtlLoadGlob.cachedShaderText[0], &mtlLoadGlob.cachedShaderText[mtlLoadGlob.cachedShaderCount], Material_CachedShaderTextLess);
    FS_FreeFileList(shaderListLib);
#endif
}

void Material_FreeAllLiterals()
{
    mtlLoadGlob.literalCount = 0;
}
void Material_FreeAllStrings()
{
    memset(mtlLoadGlob.stringHashTable, 0, sizeof(mtlLoadGlob.stringHashTable));
    mtlLoadGlob.stringCount = 0;
}
void Material_FreeAllStateMaps()
{
    memset(mtlLoadGlob.stateMapHashTable, 0, sizeof(mtlLoadGlob.stateMapHashTable));
    mtlLoadGlob.stateMapCount = 0;
}
void __cdecl Material_FreeAllTechniqueSets()
{
    DB_EnumXAssets(ASSET_TYPE_TECHNIQUE_SET, Material_ReleaseTechniqueSet, 0, 1);
    if (!IsFastFileLoad())
        memset(materialGlobals.techniqueSetHashTable, 0, sizeof(materialGlobals.techniqueSetHashTable));
}
void __cdecl Material_FreeAll()
{
    Material_FreeAllLiterals();
    Material_FreeAllStrings();
    Material_FreeAllStateMaps();
    Material_FreeAllTechniqueSets();
    if (!IsFastFileLoad())
    {
        memset(mtlLoadGlob.techniqueHashTable, 0, sizeof(mtlLoadGlob.techniqueHashTable));
        mtlLoadGlob.techniqueCount = 0;
        memset(&mtlLoadGlob.vertexDeclHashTable[0].streamCount, 0, 0xC80u);
        mtlLoadGlob.vertexDeclCount = 0;
        memset(mtlLoadGlob.vertexShaderHashTable, 0, sizeof(mtlLoadGlob.vertexShaderHashTable));
        mtlLoadGlob.vertexShaderCount = 0;
        memset(mtlLoadGlob.pixelShaderHashTable, 0, sizeof(mtlLoadGlob.pixelShaderHashTable));
        mtlLoadGlob.pixelShaderCount = 0;
    }
}

void __cdecl Material_GetVertexShaderName(char *dest, const MaterialPass *pass, int destsize)
{
    iassert( pass->vertexShader );
    I_strncpyz(dest, pass->vertexShader->name, destsize);
}

unsigned int __cdecl R_DrawSurfStandardPrepassSortKey(const Material *material)
{
    MaterialTechniqueSet *techSet; // [esp+0h] [ebp-8h]
    const MaterialTechnique *prepassTech; // [esp+4h] [ebp-4h]

    techSet = material->techniqueSet;
    iassert( techSet );
    prepassTech = techSet->techniques[0];
    if (prepassTech)
    {
        if ((material->stateFlags & 4) != 0)
            return 3;
        else
            return (prepassTech->flags & 4) == 0;
    }
    else if (techSet->techniques[1])
    {
        return 2;
    }
    else
    {
        return 3;
    }
}

void __cdecl R_RegisterShaderConst(unsigned int dest, const float *value, GfxShaderConstantBlock *consts)
{
    unsigned int sortedIndex; // [esp+4h] [ebp-4h]

    if (consts->count >= 0x10)
        MyAssertHandler(
            ".\\r_material_consts.cpp",
            15,
            1,
            "consts->count doesn't index ARRAY_COUNT( consts->dest )\n\t%i not in [0, %i)",
            consts->count,
            16);
    for (sortedIndex = consts->count;
        sortedIndex && *(&consts->count + sortedIndex + 1) > dest;
        consts->value[sortedIndex + 1] = consts->value[sortedIndex])
    {
        --sortedIndex;
        consts->dest[sortedIndex + 1] = consts->dest[sortedIndex];
    }
    consts->dest[sortedIndex] = dest;
    consts->value[sortedIndex] = value;
    ++consts->count;
}

void __cdecl R_GetPixelLiteralConsts(
    const Material *mtl,
    const MaterialPass *pass,
    GfxShaderConstantBlock *pixelLiteralConsts)
{
    const char *v3; // eax
    MaterialConstantDef *constDef; // [esp+0h] [ebp-Ch]
    unsigned int argCount; // [esp+4h] [ebp-8h]
    const MaterialShaderArgument *arg; // [esp+8h] [ebp-4h]

    pixelLiteralConsts->count = 0;
    argCount = pass->stableArgCount;
    if (pass->stableArgCount)
    {
        for (arg = &pass->args[pass->perPrimArgCount + pass->perObjArgCount]; arg->type < 6u; ++arg)
        {
            if (!--argCount)
                return;
        }
        constDef = mtl->constantTable;
        while (arg->type == 6)
        {
            while (constDef->nameHash != arg->u.codeSampler)
            {
                if (++constDef == &mtl->constantTable[mtl->constantCount])
                {
                    v3 = va("material '%s' is missing a required named constant", mtl->info.name);
                    MyAssertHandler(
                        ".\\r_material_consts.cpp",
                        59,
                        0,
                        "%s\n\t%s",
                        "constDef != &mtl->constantTable[mtl->constantCount]",
                        v3);
                }
            }
            R_RegisterShaderConst(arg->dest, constDef->literal, pixelLiteralConsts);
            ++arg;
            if (!--argCount)
                return;
        }
        do
        {
            if (arg->type != 7)
                break;
            R_RegisterShaderConst(arg->dest, arg->u.literalConst, pixelLiteralConsts);
            ++arg;
            --argCount;
        } while (argCount);
    }
}

int __cdecl R_ComparePixelConsts(const Material **material, const MaterialPass **pass)
{
    int j; // [esp+0h] [ebp-4ECh]
    GfxShaderConstantBlock pixelLiteralConsts[2]; // [esp+4h] [ebp-4E8h] BYREF
    unsigned __int16 pixelConsts[2][256]; // [esp+CCh] [ebp-420h] BYREF
    unsigned int argCount; // [esp+4D0h] [ebp-1Ch]
    const MaterialShaderArgument *arg; // [esp+4D4h] [ebp-18h]
    int i; // [esp+4D8h] [ebp-14h]
    int comparison; // [esp+4DCh] [ebp-10h]
    unsigned int constIndex; // [esp+4E0h] [ebp-Ch]
    unsigned int pixelConstsCount[2]; // [esp+4E4h] [ebp-8h]

    for (i = 0; i < 2; ++i)
    {
        pixelConstsCount[i] = 0;
        pixelLiteralConsts[i].count = 0;
        arg = &pass[i]->args[pass[i]->perPrimArgCount + pass[i]->perObjArgCount];
        argCount = pass[i]->stableArgCount;
        if (argCount)
        {
            while (arg->type < 5u)
            {
                ++arg;
                if (!--argCount)
                    goto done_2;
            }
            while (arg->type == 5)
            {
                if (pixelConstsCount[i] >= 0x100)
                    MyAssertHandler(
                        ".\\r_material_consts.cpp",
                        148,
                        0,
                        "pixelConstsCount[i] doesn't index ARRAY_COUNT( pixelConsts[i] )\n\t%i not in [0, %i)",
                        pixelConstsCount[i],
                        256);
                pixelConsts[i][pixelConstsCount[i]++] = arg->u.codeConst.index;
                ++arg;
                if (!--argCount)
                    goto done_2;
            }
            R_GetPixelLiteralConsts(material[i], pass[i], &pixelLiteralConsts[i]);
        }
    done_2:
        ;
    }
    comparison = pixelConstsCount[0] - pixelConstsCount[1];
    if (pixelConstsCount[0] != pixelConstsCount[1])
        return comparison;
    for (constIndex = 0; constIndex < pixelConstsCount[0]; ++constIndex)
    {
        comparison = pixelConsts[0][constIndex] - pixelConsts[1][constIndex];
        if (comparison)
            return comparison;
    }
    comparison = pixelLiteralConsts[0].count - pixelLiteralConsts[1].count;
    if (pixelLiteralConsts[0].count != pixelLiteralConsts[1].count)
        return comparison;
    for (constIndex = 0; constIndex < pixelLiteralConsts[0].count; ++constIndex)
    {
        comparison = pixelLiteralConsts[0].dest[constIndex] - pixelLiteralConsts[1].dest[constIndex];
        if (comparison)
            return comparison;
        for (j = 0; j < 4; ++j)
        {
            if (pixelLiteralConsts[1].value[constIndex][j] > pixelLiteralConsts[0].value[constIndex][j])
                return -1;
            if (pixelLiteralConsts[1].value[constIndex][j] < pixelLiteralConsts[0].value[constIndex][j])
                return 1;
        }
    }
    return 0;
}

int __cdecl Material_ComparePixelConsts(const Material *mtl0, const Material *mtl1, MaterialTechniqueType techType)
{
    const MaterialPass *pass[2]; // [esp+10h] [ebp-18h] BYREF
    const MaterialTechnique *techniqueLit[2]; // [esp+18h] [ebp-10h]
    const Material *mtl[2]; // [esp+20h] [ebp-8h] BYREF

    techniqueLit[0] = Material_GetTechnique(mtl0, techType);
    techniqueLit[1] = Material_GetTechnique(mtl1, techType);
    iassert( techniqueLit[0] );
    iassert( techniqueLit[1] );
    mtl[0] = mtl0;
    mtl[1] = mtl1;
    pass[0] = techniqueLit[0]->passArray;
    pass[1] = techniqueLit[1]->passArray;
    return R_ComparePixelConsts(mtl, pass);
}

INT __cdecl Material_Compare(const void *arg0, const void *arg1)
{
    int writesDepth; // [esp+98h] [ebp-148h]
    int writesDepth_4; // [esp+9Ch] [ebp-144h]
    const MaterialTechnique *techniqueEmissive; // [esp+A0h] [ebp-140h]
    const MaterialTechnique *techniqueEmissive_4; // [esp+A4h] [ebp-13Ch]
    const MaterialTechnique *techniqueLit; // [esp+A8h] [ebp-138h]
    const MaterialTechnique *techniqueLit_4; // [esp+ACh] [ebp-134h]
    char name0[128]; // [esp+B0h] [ebp-130h] BYREF
    int hasTechniqueEmissive[2]; // [esp+130h] [ebp-B0h]
    int hasTechniqueLit[2]; // [esp+138h] [ebp-A8h]
    int prepass[2]; // [esp+140h] [ebp-A0h]
    char name1[128]; // [esp+148h] [ebp-98h] BYREF
    const MaterialTechniqueSet *techSet[2]; // [esp+1CCh] [ebp-14h]
    int comparison; // [esp+1D4h] [ebp-Ch]
    int hasLightmap[2]; // [esp+1D8h] [ebp-8h]

    Material *mtl0 = *(Material **)arg0;
    Material *mtl1 = *(Material **)arg1;

    iassert( mtl0 );
    iassert( mtl1 );
    techSet[0] = Material_GetTechniqueSet(mtl0);
    techSet[1] = Material_GetTechniqueSet(mtl1);
    iassert( techSet[0] && techSet[1] );
    techniqueLit = Material_GetTechnique(mtl0, TECHNIQUE_LIT_BEGIN);
    techniqueLit_4 = Material_GetTechnique(mtl1, TECHNIQUE_LIT_BEGIN);
    hasTechniqueLit[0] = techniqueLit != 0;
    hasTechniqueLit[1] = techniqueLit_4 != 0;
    comparison = hasTechniqueLit[1] - hasTechniqueLit[0];
    if (hasTechniqueLit[1] != hasTechniqueLit[0])
        return comparison < 0;
    hasLightmap[0] = (mtl0->info.gameFlags & 2) != 0;
    hasLightmap[1] = (mtl1->info.gameFlags & 2) != 0;
    techniqueEmissive = Material_GetTechnique(mtl0, TECHNIQUE_EMISSIVE);
    techniqueEmissive_4 = Material_GetTechnique(mtl1, TECHNIQUE_EMISSIVE);
    hasTechniqueEmissive[0] = techniqueEmissive != 0;
    hasTechniqueEmissive[1] = techniqueEmissive_4 != 0;
    if (hasTechniqueLit[0])
    {
        iassert( !hasTechniqueEmissive[0] );
        iassert( !hasTechniqueEmissive[1] );
        comparison = mtl0->info.sortKey - mtl1->info.sortKey;
        if (comparison)
            return comparison < 0;
        comparison = hasLightmap[1] - hasLightmap[0];
        if (hasLightmap[1] != hasLightmap[0])
            return comparison < 0;
    }
    else
    {
        iassert( !hasLightmap[0] );
        iassert( !hasLightmap[1] );
        comparison = hasTechniqueEmissive[1] - hasTechniqueEmissive[0];
        if (hasTechniqueEmissive[1] != hasTechniqueEmissive[0])
            return comparison < 0;
        comparison = mtl0->info.sortKey - mtl1->info.sortKey;
        if (comparison)
            return comparison < 0;
    }
    prepass[0] = R_DrawSurfStandardPrepassSortKey(mtl0);
    prepass[1] = R_DrawSurfStandardPrepassSortKey(mtl1);
    comparison = prepass[0] - prepass[1];
    if (prepass[0] != prepass[1])
        return comparison < 0;
    writesDepth = (mtl0->stateFlags & 8) != 0;
    writesDepth_4 = (mtl1->stateFlags & 8) != 0;
    comparison = writesDepth_4 - writesDepth;
    if (writesDepth_4 != writesDepth)
        return comparison < 0;
    if (hasTechniqueLit[0])
    {
        comparison = strcmp(techniqueLit->passArray[0].pixelShader->name, techniqueLit_4->passArray[0].pixelShader->name);
        if (comparison)
            return comparison < 0;
        if (writesDepth)
        {
            comparison = Material_ComparePixelConsts(mtl0, mtl1, TECHNIQUE_LIT_BEGIN);
            if (comparison)
                return comparison < 0;
        }
        Material_GetVertexShaderName(name0, techniqueLit->passArray, 128);
        Material_GetVertexShaderName(name1, techniqueLit_4->passArray, 128);
        comparison = strcmp(name0, name1);
        if (comparison)
            return comparison < 0;
    }
    else if (hasTechniqueEmissive[0])
    {
        comparison = strcmp(
            techniqueEmissive->passArray[0].pixelShader->name,
            techniqueEmissive_4->passArray[0].pixelShader->name);
        if (comparison)
            return comparison < 0;
        comparison = Material_ComparePixelConsts(mtl0, mtl1, TECHNIQUE_EMISSIVE);
        if (comparison)
            return comparison < 0;
        Material_GetVertexShaderName(name0, techniqueEmissive->passArray, 128);
        Material_GetVertexShaderName(name1, techniqueEmissive_4->passArray, 128);
        comparison = strcmp(name0, name1);
        if (comparison)
            return comparison < 0;
    }
    comparison = strcmp(techSet[0]->name, techSet[1]->name);
    if (comparison)
        return comparison < 0;
    iassert( mtl0 != mtl1 );
    comparison = strcmp(mtl0->info.name, mtl1->info.name);
    iassert( comparison );
    return comparison < 0;
}

unsigned int __cdecl R_DrawSurfPrimarySortKey(const Material *material)
{
    if (material->info.sortKey >= 0x40u)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\gfx_d3d\\r_drawsurf.h",
            23,
            0,
            "material->info.sortKey doesn't index 1 << MTL_SORT_PRIMARY_SORT_KEY_BITS\n\t%i not in [0, %i)",
            material->info.sortKey,
            64);
    return material->info.sortKey;
}

void __cdecl Material_SortInternal(Material **sortedMaterials, unsigned int materialCount)
{
    unsigned __int64 v2; // rax
    unsigned int v3; // ecx
    unsigned __int64 v4; // rax
    unsigned int v5; // ecx
    unsigned __int64 v6; // rax
    int v7; // ecx
    unsigned __int64 v8; // rax
    unsigned int v9; // ecx
    unsigned int sortedIndex; // [esp+98h] [ebp-Ch]
    Material *material; // [esp+A0h] [ebp-4h]

    //std::_Sort<int *, int, bool(__cdecl *)(int, int)>(
    //    sortedMaterials,
    //    &sortedMaterials[materialCount],
    //    (4 * materialCount) >> 2,
    //    Material_Compare);
    qsort(sortedMaterials, materialCount, sizeof(Material *), Material_Compare);
    //std::sort(sortedMaterials + 0, sortedMaterials + materialCount, Material_Compare);
    for (sortedIndex = 0; sortedIndex < materialCount; ++sortedIndex)
    {
        material = sortedMaterials[sortedIndex];
        material->info.drawSurf.packed = 0;
        material->info.drawSurf.fields.primarySortKey = R_DrawSurfPrimarySortKey(material);
        material->info.drawSurf.fields.prepass = R_DrawSurfStandardPrepassSortKey(material);
        material->info.drawSurf.fields.customIndex = (material->info.gameFlags & 0x40) != 0;
        material->info.drawSurf.fields.materialSortedIndex = sortedIndex;
    }
}

void __cdecl Material_Sort()
{
    if (IsFastFileLoad())
        rgp.materialCount = DB_GetAllXAssetOfType(ASSET_TYPE_MATERIAL, (XAssetHeader *)&rgp, 2048);
    Material_SortInternal(rgp.sortedMaterials, rgp.materialCount);
}