#include <universal/q_shared.h>
#include "rb_logfile.h"

struct $7272244CE635036285CD968BA6FC5DEE // sizeof=0x4
{                                       // ...
    FILE *fp;                          // ...
};

$7272244CE635036285CD968BA6FC5DEE r_logFileGlob;

void __cdecl RB_UpdateLogging()
{
    if (RB_IsLogging())
        Dvar_SetInt((dvar_s *)r_logFile, r_logFile->current.integer - 1);
    if (r_logFile->current.integer)
        RB_OpenLogFile();
    else
        RB_CloseLogFile();
}

void RB_CloseLogFile()
{
    if (r_logFileGlob.fp)
    {
        fclose(r_logFileGlob.fp);
        r_logFileGlob.fp = 0;
    }
}

void RB_OpenLogFile()
{
    const char *v0; // eax
    __int64 aclock; // [esp+0h] [ebp-10h] BYREF
    tm *newtime; // [esp+Ch] [ebp-4h]

    if (!r_logFileGlob.fp)
    {
        r_logFileGlob.fp = fopen("dx.log", "wt");
        if (r_logFileGlob.fp)
        {
            _time64(&aclock);
            newtime = _localtime64(&aclock);
            v0 = asctime(newtime);
            fprintf(r_logFileGlob.fp, "%s\n", v0);
            fflush(r_logFileGlob.fp);
        }
    }
}

bool __cdecl RB_IsLogging()
{
    return r_logFile->current.integer && r_logFileGlob.fp != 0;
}

void __cdecl RB_LogPrint(const char *text)
{
    iassert( r_logFile->current.integer );
    if (RB_IsLogging())
    {
        fprintf(r_logFileGlob.fp, "%s", text);
        fflush(r_logFileGlob.fp);
    }
}

const char *__cdecl RB_LogTechniqueType(MaterialTechniqueType techType)
{
    const char *techniqueNames[TECHNIQUE_TOTAL_COUNT]; // [esp+0h] [ebp-90h]

    techniqueNames[TECHNIQUE_DEPTH_PREPASS] = "TECHNIQUE_DEPTH_PREPASS";
    techniqueNames[TECHNIQUE_BUILD_FLOAT_Z] = "TECHNIQUE_BUILD_FLOAT_Z";
    techniqueNames[TECHNIQUE_BUILD_SHADOWMAP_DEPTH] = "TECHNIQUE_BUILD_SHADOWMAP_DEPTH";
    techniqueNames[TECHNIQUE_BUILD_SHADOWMAP_COLOR] = "TECHNIQUE_BUILD_SHADOWMAP_COLOR";
    techniqueNames[TECHNIQUE_UNLIT] = "TECHNIQUE_UNLIT";
    techniqueNames[TECHNIQUE_EMISSIVE] = "TECHNIQUE_EMISSIVE";
    techniqueNames[TECHNIQUE_EMISSIVE_SHADOW] = "TECHNIQUE_EMISSIVE_SHADOW";
    techniqueNames[TECHNIQUE_LIT] = "TECHNIQUE_LIT";
    techniqueNames[TECHNIQUE_LIT_SUN] = "TECHNIQUE_LIT_SUN";
    techniqueNames[TECHNIQUE_LIT_SUN_SHADOW] = "TECHNIQUE_LIT_SUN_SHADOW";
    techniqueNames[TECHNIQUE_LIT_SPOT] = "TECHNIQUE_LIT_SPOT";
    techniqueNames[TECHNIQUE_LIT_SPOT_SHADOW] = "TECHNIQUE_LIT_SPOT_SHADOW";
    techniqueNames[TECHNIQUE_LIT_OMNI] = "TECHNIQUE_LIT_OMNI";
    techniqueNames[TECHNIQUE_LIT_OMNI_SHADOW] = "TECHNIQUE_LIT_OMNI_SHADOW";
    techniqueNames[TECHNIQUE_LIT_INSTANCED] = "TECHNIQUE_LIT_INSTANCED";
    techniqueNames[TECHNIQUE_LIT_INSTANCED_SUN] = "TECHNIQUE_LIT_INSTANCED_SUN";
    techniqueNames[TECHNIQUE_LIT_INSTANCED_SUN_SHADOW] = "TECHNIQUE_LIT_INSTANCED_SUN_SHADOW";
    techniqueNames[TECHNIQUE_LIT_INSTANCED_SPOT] = "TECHNIQUE_LIT_INSTANCED_SPOT";
    techniqueNames[TECHNIQUE_LIT_INSTANCED_SPOT_SHADOW] = "TECHNIQUE_LIT_INSTANCED_SPOT_SHADOW";
    techniqueNames[TECHNIQUE_LIT_INSTANCED_OMNI] = "TECHNIQUE_LIT_INSTANCED_OMNI";
    techniqueNames[TECHNIQUE_LIT_INSTANCED_OMNI_SHADOW] = "TECHNIQUE_LIT_INSTANCED_OMNI_SHADOW";
    techniqueNames[TECHNIQUE_LIGHT_SPOT] = "TECHNIQUE_LIGHT_SPOT";
    techniqueNames[TECHNIQUE_LIGHT_OMNI] = "TECHNIQUE_LIGHT_OMNI";
    techniqueNames[TECHNIQUE_LIGHT_SPOT_SHADOW] = "TECHNIQUE_LIGHT_SPOT_SHADOW";
    techniqueNames[TECHNIQUE_FAKELIGHT_NORMAL] = "TECHNIQUE_FAKELIGHT_NORMAL";
    techniqueNames[TECHNIQUE_FAKELIGHT_VIEW] = "TECHNIQUE_FAKELIGHT_VIEW";
    techniqueNames[TECHNIQUE_SUNLIGHT_PREVIEW] = "TECHNIQUE_SUNLIGHT_PREVIEW";
    techniqueNames[TECHNIQUE_CASE_TEXTURE] = "TECHNIQUE_CASE_TEXTURE";
    techniqueNames[TECHNIQUE_WIREFRAME_SOLID] = "TECHNIQUE_WIREFRAME_SOLID";
    techniqueNames[TECHNIQUE_WIREFRAME_SHADED] = "TECHNIQUE_WIREFRAME_SHADED";
    techniqueNames[TECHNIQUE_SHADOWCOOKIE_CASTER] = "TECHNIQUE_SHADOWCOOKIE_CASTER";
    techniqueNames[TECHNIQUE_SHADOWCOOKIE_RECEIVER] = "TECHNIQUE_SHADOWCOOKIE_RECEIVER";
    techniqueNames[TECHNIQUE_DEBUG_BUMPMAP] = "TECHNIQUE_DEBUG_BUMPMAP";
    techniqueNames[TECHNIQUE_DEBUG_BUMPMAP_INSTANCED] = "TECHNIQUE_DEBUG_BUMPMAP_INSTANCED";
    techniqueNames[TECHNIQUE_COUNT] = "TECHNIQUE_COUNT";
    if ((uint32_t)techType > TECHNIQUE_COUNT)
        MyAssertHandler(
            ".\\rb_logfile.cpp",
            178,
            0,
            "%s\n\t(techType) = %i",
            "(techType >= 0 && techType < TECHNIQUE_TOTAL_COUNT)",
            techType);
    return techniqueNames[techType];
}

void __cdecl RB_LogPrintState_0(int stateBits0, int changedBits0)
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    StateBitsTable alphaTestTable[4]; // [esp+10h] [ebp-38h] BYREF
    StateBitsTable cullTable[3]; // [esp+30h] [ebp-18h] BYREF

    alphaTestTable[0].stateBits = 2048;
    alphaTestTable[0].name = "Disabled";
    alphaTestTable[1].stateBits = 4096;
    alphaTestTable[1].name = "GT0";
    alphaTestTable[2].stateBits = 0x2000;
    alphaTestTable[2].name = "LT128";
    alphaTestTable[3].stateBits = 12288;
    alphaTestTable[3].name = "GE128";
    cullTable[0].stateBits = 0x4000;
    cullTable[0].name = "None";
    cullTable[1].stateBits = 0x8000;
    cullTable[1].name = "Back";
    cullTable[2].stateBits = 49152;
    cullTable[2].name = "Front";
    iassert( r_logFile->current.integer );
    v2 = va("---------- (%c)Blend         : ", (changedBits0 & 0x7FF) != 0 ? 42 : 32);
    RB_LogPrint(v2);
    if ((stateBits0 & 0x700) != 0)
    {
        RB_LogBlendOp("%s( ", (stateBits0 & 0x700) >> 8);
        RB_LogBlend("%s, ", stateBits0 & 0xF);
        RB_LogBlend("%s )\n", (stateBits0 & 0xF0) >> 4);
    }
    else
    {
        RB_LogPrint("Disabled\n");
    }
    v3 = va("---------- (%c)SeparateAlpha : ", (changedBits0 & 0x7FF0000) != 0 ? 42 : 32);
    RB_LogPrint(v3);
    if ((stateBits0 & 0x7000000) != 0)
    {
        RB_LogBlendOp("%s( ", (stateBits0 & 0x7000000) >> 24);
        RB_LogBlend("%s, ", (stateBits0 & 0xF0000) >> 16);
        RB_LogBlend("%s )\n", (stateBits0 & 0xF00000) >> 20);
    }
    else
    {
        RB_LogPrint("Disabled\n");
    }
    RB_LogFromTable("AlphaTest", stateBits0, changedBits0, 14336, 0, alphaTestTable, 4);
    v4 = va("---------- (%c)Color Write   : ", (changedBits0 & 0x18000000) != 0 ? 42 : 32);
    RB_LogPrint(v4);
    if ((stateBits0 & 0x8000000) != 0)
        v5 = va("%s, ", "true");
    else
        v5 = va("%s, ", "false");
    RB_LogPrint(v5);
    if ((stateBits0 & 0x8000000) != 0)
        v6 = va("%s, ", "true");
    else
        v6 = va("%s, ", "false");
    RB_LogPrint(v6);
    if ((stateBits0 & 0x8000000) != 0)
        v7 = va("%s, ", "true");
    else
        v7 = va("%s, ", "false");
    RB_LogPrint(v7);
    if ((stateBits0 & 0x10000000) != 0)
        v8 = va("%s\n", "true");
    else
        v8 = va("%s\n", "false");
    RB_LogPrint(v8);
    RB_LogFromTable("Cull Face", stateBits0, changedBits0, 49152, 0, cullTable, 3);
}

void __cdecl RB_LogBlend(const char *format, uint32_t blend)
{
    const char *v2; // eax
    const char *blendNames[14]; // [esp+0h] [ebp-38h]

    blendNames[0] = "Disabled";
    blendNames[1] = "Zero";
    blendNames[2] = "One";
    blendNames[3] = "SrcColor";
    blendNames[4] = "InvSrcColor";
    blendNames[5] = "SrcAlpha";
    blendNames[6] = "InvSrcAlpha";
    blendNames[7] = "DestAlpha";
    blendNames[8] = "InvDestAlpha";
    blendNames[9] = "DestColor";
    blendNames[10] = "InvDestColor";
    blendNames[11] = "SrcAlphaSat";
    blendNames[12] = "BlendFactor";
    blendNames[13] = "InvBlendFactor";
    iassert( r_logFile->current.integer );
    if (blend >= 0xE)
        MyAssertHandler(
            ".\\rb_logfile.cpp",
            204,
            0,
            "blend doesn't index ARRAY_COUNT( blendNames )\n\t%i not in [0, %i)",
            blend,
            14);
    v2 = va(format, blendNames[blend]);
    RB_LogPrint(v2);
}

void __cdecl RB_LogBlendOp(const char *format, uint32_t blendOp)
{
    const char *v2; // eax
    const char *blendOpNames[6]; // [esp+0h] [ebp-18h]

    blendOpNames[0] = "Disabled";
    blendOpNames[1] = "Add";
    blendOpNames[2] = "Subtract";
    blendOpNames[3] = "RevSubtract";
    blendOpNames[4] = "Min";
    blendOpNames[5] = "Max";
    iassert( r_logFile->current.integer );
    if (blendOp >= 6)
        MyAssertHandler(
            ".\\rb_logfile.cpp",
            223,
            0,
            "blendOp doesn't index ARRAY_COUNT( blendOpNames )\n\t%i not in [0, %i)",
            blendOp,
            6);
    v2 = va(format, blendOpNames[blendOp]);
    RB_LogPrint(v2);
}

void __cdecl RB_LogFromTable(
    const char *keyName,
    int stateBits,
    int changedBits,
    int bitMask,
    char bitShift,
    const StateBitsTable *table,
    int tableCount)
{
    const char *v7; // eax
    const char *v8; // eax
    int tableIndex; // [esp+0h] [ebp-8h]
    int stateBitsa; // [esp+14h] [ebp+Ch]

    stateBitsa = (bitMask & stateBits) >> bitShift;
    for (tableIndex = 0; tableIndex < tableCount; ++tableIndex)
    {
        if (table[tableIndex].stateBits == stateBitsa)
        {
            v7 = va("---------- (%c)%-14s: %s\n", (bitMask & changedBits) != 0 ? 42 : 32, keyName, table[tableIndex].name);
            RB_LogPrint(v7);
            return;
        }
    }
    v8 = va("---------- (%c)%-14s: unknown - %08x\n", (bitMask & changedBits) != 0 ? 42 : 32, keyName, stateBitsa);
    RB_LogPrint(v8);
}

void __cdecl RB_LogPrintState_1(int stateBits1, int changedBits1)
{
    StencilLogBits stencilLogFront; // [esp+0h] [ebp-78h] BYREF
    StencilLogBits stencilLogBack; // [esp+18h] [ebp-60h] BYREF
    StateBitsTable depthTestTable[5]; // [esp+30h] [ebp-48h] BYREF
    StateBitsTable polygonOffsetTable[4]; // [esp+58h] [ebp-20h] BYREF

    depthTestTable[0].stateBits = 2;
    depthTestTable[0].name = "Disabled";
    depthTestTable[1].stateBits = 4;
    depthTestTable[1].name = "Less";
    depthTestTable[2].stateBits = 12;
    depthTestTable[2].name = "LessEqual";
    depthTestTable[3].stateBits = 8;
    depthTestTable[3].name = "Equal";
    depthTestTable[4].stateBits = 0;
    depthTestTable[4].name = "Always";
    polygonOffsetTable[0].stateBits = 0;
    polygonOffsetTable[0].name = "0";
    polygonOffsetTable[1].stateBits = 16;
    polygonOffsetTable[1].name = "1";
    polygonOffsetTable[2].stateBits = 32;
    polygonOffsetTable[2].name = "2";
    polygonOffsetTable[3].stateBits = 48;
    polygonOffsetTable[3].name = "shadowmap";
    stencilLogFront.description = "Front";
    stencilLogFront.enableMask = 64;
    stencilLogFront.passShift = 8;
    stencilLogFront.failShift = 11;
    stencilLogFront.zfailShift = 14;
    stencilLogFront.funcShift = 17;
    stencilLogBack.description = "Back";
    stencilLogBack.enableMask = 128;
    stencilLogBack.passShift = 20;
    stencilLogBack.failShift = 23;
    stencilLogBack.zfailShift = 26;
    stencilLogBack.funcShift = 29;
    iassert( r_logFile->current.integer );
    RB_LogBool("Depth Write", stateBits1, changedBits1, 1, "Enabled", "Disabled");
    RB_LogFromTable("Depth Test", stateBits1, changedBits1, 14, 0, depthTestTable, 5);
    RB_LogFromTable("Polygon Offset", stateBits1, changedBits1, 48, 0, polygonOffsetTable, 4);
    RB_LogStencilState(stateBits1, changedBits1, &stencilLogFront);
    RB_LogStencilState(stateBits1, changedBits1, &stencilLogBack);
}

void __cdecl RB_LogBool(
    const char *keyName,
    int stateBits,
    int changedBits,
    int bitMask,
    const char *trueName,
    const char *falseName)
{
    const char *v6; // eax
    int changedBitsa; // [esp+14h] [ebp+10h]

    changedBitsa = bitMask & changedBits;
    if ((bitMask & stateBits) != 0)
        v6 = va("---------- (%c)%-14s: %s\n", changedBitsa != 0 ? 42 : 32, keyName, trueName);
    else
        v6 = va("---------- (%c)%-14s: %s\n", changedBitsa != 0 ? 42 : 32, keyName, falseName);
    RB_LogPrint(v6);
}

void __cdecl RB_LogStencilState(int stateBits1, int changedBits1, const StencilLogBits *desc)
{
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    int enableMask; // [esp-Ch] [ebp-8Ch]
    int funcShift; // [esp-Ch] [ebp-8Ch]
    int passShift; // [esp-Ch] [ebp-8Ch]
    int failShift; // [esp-Ch] [ebp-8Ch]
    int zfailShift; // [esp-Ch] [ebp-8Ch]
    StateBitsTable stencilFuncNames[GFXS_STENCILFUNC_COUNT]; // [esp+0h] [ebp-80h] BYREF
    StateBitsTable stencilOpNames[GFXS_STENCILOP_COUNT]; // [esp+40h] [ebp-40h] BYREF

    stencilFuncNames[GFXS_STENCILFUNC_NEVER].stateBits = GFXS_STENCILFUNC_NEVER;
    stencilFuncNames[GFXS_STENCILFUNC_NEVER].name = "Never";
    stencilFuncNames[GFXS_STENCILFUNC_LESS].stateBits = GFXS_STENCILFUNC_LESS;
    stencilFuncNames[GFXS_STENCILFUNC_LESS].name = "Less";
    stencilFuncNames[GFXS_STENCILFUNC_EQUAL].stateBits = GFXS_STENCILFUNC_EQUAL;
    stencilFuncNames[GFXS_STENCILFUNC_EQUAL].name = "Equal";
    stencilFuncNames[GFXS_STENCILFUNC_LESSEQUAL].stateBits = GFXS_STENCILFUNC_LESSEQUAL;
    stencilFuncNames[GFXS_STENCILFUNC_LESSEQUAL].name = "LessEqual";
    stencilFuncNames[GFXS_STENCILFUNC_GREATER].stateBits = GFXS_STENCILFUNC_GREATER;
    stencilFuncNames[GFXS_STENCILFUNC_GREATER].name = "Greater";
    stencilFuncNames[GFXS_STENCILFUNC_NOTEQUAL].stateBits = GFXS_STENCILFUNC_NOTEQUAL;
    stencilFuncNames[GFXS_STENCILFUNC_NOTEQUAL].name = "NotEqual";
    stencilFuncNames[GFXS_STENCILFUNC_GREATEREQUAL].stateBits = GFXS_STENCILFUNC_GREATEREQUAL;
    stencilFuncNames[GFXS_STENCILFUNC_GREATEREQUAL].name = "GreaterEqual";
    stencilFuncNames[GFXS_STENCILFUNC_ALWAYS].stateBits = GFXS_STENCILFUNC_ALWAYS;
    stencilFuncNames[GFXS_STENCILFUNC_ALWAYS].name = "Always";
    stencilOpNames[GFXS_STENCILOP_KEEP].stateBits = GFXS_STENCILOP_KEEP;
    stencilOpNames[GFXS_STENCILOP_KEEP].name = "Keep";
    stencilOpNames[GFXS_STENCILOP_ZERO].stateBits = GFXS_STENCILOP_ZERO;
    stencilOpNames[GFXS_STENCILOP_ZERO].name = "Zero";
    stencilOpNames[GFXS_STENCILOP_REPLACE].stateBits = GFXS_STENCILOP_REPLACE;
    stencilOpNames[GFXS_STENCILOP_REPLACE].name = "Replace";
    stencilOpNames[GFXS_STENCILOP_INCRSAT].stateBits = GFXS_STENCILOP_INCRSAT;
    stencilOpNames[GFXS_STENCILOP_INCRSAT].name = "IncrSat";
    stencilOpNames[GFXS_STENCILOP_DECRSAT].stateBits = GFXS_STENCILOP_DECRSAT;
    stencilOpNames[GFXS_STENCILOP_DECRSAT].name = "DecrSat";
    stencilOpNames[GFXS_STENCILOP_INVERT].stateBits = GFXS_STENCILOP_INVERT;
    stencilOpNames[GFXS_STENCILOP_INVERT].name = "Invert";
    stencilOpNames[GFXS_STENCILOP_INCR].stateBits = GFXS_STENCILOP_INCR;
    stencilOpNames[GFXS_STENCILOP_INCR].name = "Incr";
    stencilOpNames[GFXS_STENCILOP_DECR].stateBits = GFXS_STENCILOP_DECR;
    stencilOpNames[GFXS_STENCILOP_DECR].name = "Decr";
    enableMask = desc->enableMask;
    v3 = va("Stencil %s", desc->description);
    RB_LogBool(v3, stateBits1, changedBits1, enableMask, "Enabled", "Disabled");
    if ((desc->enableMask & stateBits1) != 0)
    {
        funcShift = desc->funcShift;
        v4 = va("%s Func", desc->description);
        RB_LogFromTable(
            v4,
            stateBits1,
            changedBits1,
            (GFXS_STENCILFUNC_COUNT - 1) << funcShift,
            funcShift,
            stencilFuncNames,
            GFXS_STENCILFUNC_COUNT);
        passShift = desc->passShift;
        v5 = va("%s Pass", desc->description);
        RB_LogFromTable(
            v5,
            stateBits1,
            changedBits1,
            (GFXS_STENCILOP_COUNT - 1) << passShift,
            passShift,
            stencilOpNames,
            GFXS_STENCILOP_COUNT);
        failShift = desc->failShift;
        v6 = va("%s Fail", desc->description);
        RB_LogFromTable(
            v6,
            stateBits1,
            changedBits1,
            (GFXS_STENCILOP_COUNT - 1) << failShift,
            failShift,
            stencilOpNames,
            GFXS_STENCILOP_COUNT);
        zfailShift = desc->zfailShift;
        v7 = va("%s ZFail", desc->description);
        RB_LogFromTable(
            v7,
            stateBits1,
            changedBits1,
            (GFXS_STENCILOP_COUNT - 1) << zfailShift,
            zfailShift,
            stencilOpNames,
            GFXS_STENCILOP_COUNT);
    }
}

