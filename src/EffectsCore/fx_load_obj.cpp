#include "fx_system.h"
#include <universal/com_files.h>
#include <universal/q_parse.h>
#include <gfx_d3d/r_model.h>
#include <universal/com_memory.h>
#include <physics/phys_local.h>
#include <database/database.h>

struct $5E78DEAD8FFD2AA25C77996D083B001E // sizeof=0x808
{                                       // ...
    int effectDefCount;
    const FxEffectDef *effectDefs[512]; // ...
    const FxEffectDef *defaultEffect;   // ...
};

$5E78DEAD8FFD2AA25C77996D083B001E fx_load;

const FxFlagDef s_allFlagDefs[42] =
{
  { "looping", 0, 1, 1 },
  { "useRandColor", 0, 2, 2 },
  { "useRandAlpha", 0, 4, 4 },
  { "useRandSize0", 0, 8, 8 },
  { "useRandSize1", 0, 16, 16 },
  { "useRandScale", 0, 32, 32 },
  { "useRandRotDelta", 0, 64, 64 },
  { "modColorByAlpha", 0, 128, 128 },
  { "useRandVel0", 0, 256, 256 },
  { "useRandVel1", 0, 512, 512 },
  { "useBackCompatVel", 0, 1024, 1024 },
  { "absVel0", 0, 2048, 2048 },
  { "absVel1", 0, 4096, 4096 },
  { "playOnTouch", 0, 8192, 8192 },
  { "playOnDeath", 0, 16384, 16384 },
  { "playOnRun", 0, 32768, 32768 },
  { "boundingSphere", 0, 65536, 65536 },
  { "useItemClip", 0, 131072, 131072 },
  { "disabled", 0, INT_MIN, INT_MIN },
  { "spawnRelative", 1, 2, 2 },
  { "spawnFrustumCull", 1, 4, 4 },
  { "runnerUsesRandRot", 1, 8, 8 },
  { "spawnOffsetNone", 1, 48, 0 },
  { "spawnOffsetSphere", 1, 48, 16 },
  { "spawnOffsetCylinder", 1, 48, 32 },
  { "runRelToWorld", 1, 192, 0 },
  { "runRelToSpawn", 1, 192, 64 },
  { "runRelToEffect", 1, 192, 128 },
  { "runRelToOffset", 1, 192, 192 },
  { "useCollision", 1, 256, 256 },
  { "dieOnTouch", 1, 512, 512 },
  { "drawPastFog", 1, 1024, 1024 },
  { "drawWithViewModel", 1, 2048, 2048 },
  { "blocksSight", 1, 4096, 4096 },
  { "modelUsesPhysics", 1, 134217728, 134217728 },
  { "nonUniformScale", 1, 268435456, 268435456 },
  { "startFixed", 2, 3, 0 },
  { "startRandom", 2, 3, 1 },
  { "startIndexed", 2, 3, 2 },
  { "playOverLife", 2, 4, 4 },
  { "loopOnlyNTimes", 2, 8, 8 },
  { NULL, 0, 0, 0 }
}; // idb

//unsigned __int8 *__cdecl Hunk_AllocPhysPresetPrecache(int size)
//{
//    if (size <= 0)
//        MyAssertHandler(".\\EffectsCore\\fx_load_obj.cpp", 237, 0, "%s", "size > 0");
//    return Hunk_Alloc(size, "Hunk_AllocPhysPresetPrecache", 21);
//}

void FX_UnregisterAll()
{
    memset(&fx_load, 0, sizeof(fx_load));
}

char __cdecl FX_ParseSingleFlag(const char *token, FxFlagOutputSet *flagOutputSet)
{
    int *outputFlag; // [esp+0h] [ebp-8h]
    const FxFlagDef *flagDef; // [esp+4h] [ebp-4h]
    const FxFlagDef *flagDefa; // [esp+4h] [ebp-4h]

    for (flagDef = s_allFlagDefs; flagDef->name; ++flagDef)
    {
        outputFlag = flagOutputSet->flags[flagDef->flagType];
        if (outputFlag && !I_strcmp(token, flagDef->name))
        {
            *outputFlag &= ~flagDef->mask;
            *outputFlag |= flagDef->value;
            return 1;
        }
    }
    Com_Printf(21, "Valid flags:\n");
    for (flagDefa = s_allFlagDefs; flagDefa->name; ++flagDefa)
    {
        if (flagOutputSet->flags[flagDefa->flagType])
            Com_Printf(21, "  %s\n", flagDefa->name);
    }
    Com_ScriptError("Unknown flag '%s'\n", token);
    return 0;
}

char __cdecl FX_ParseFlagsField(const char **parse, FxFlagOutputSet *flagOutputSet)
{
    parseInfo_t *token; // [esp+0h] [ebp-4h]

    do
    {
        token = Com_Parse(parse);
        if (token->token[0] == 59)
        {
            Com_UngetToken();
            return 1;
        }
    } while (FX_ParseSingleFlag(token->token, flagOutputSet));
    return 0;
}

bool __cdecl FX_ParseName(const char **parse, FxEditorElemDef *edElemDef)
{
    parseInfo_t *token; // [esp+0h] [ebp-4h]

    token = Com_Parse(parse);
    I_strncpyz(edElemDef->name, token->token, 48);
    return 1;
}

bool __cdecl FX_ParseNonAtlasFlags(const char **parse, FxEditorElemDef *edElemDef)
{
    FxFlagOutputSet flagOutputSet; // [esp+0h] [ebp-Ch] BYREF

    flagOutputSet.flags[2] = 0;
    flagOutputSet.flags[0] = &edElemDef->editorFlags;
    flagOutputSet.flags[1] = &edElemDef->flags;
    return FX_ParseFlagsField(parse, &flagOutputSet);
}

char __cdecl FX_ParseFloat(const char **parse, float *value)
{
    *value = Com_ParseFloat(parse);
    return 1;
}

bool __cdecl FX_ParseFloatRange(const char **parse, FxFloatRange *range)
{
    return FX_ParseFloat(parse, &range->base) && FX_ParseFloat(parse, &range->amplitude) != 0;
}

bool __cdecl FX_ParseSpawnRange(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseFloatRange(parse, &edElemDef->spawnRange);
}

bool __cdecl FX_ParseFadeInRange(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseFloatRange(parse, &edElemDef->fadeInRange);
}

bool __cdecl FX_ParseFadeOutRange(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseFloatRange(parse, &edElemDef->fadeOutRange);
}

bool __cdecl FX_ParseSpawnFrustumCullRadius(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseFloat(parse, &edElemDef->spawnFrustumCullRadius);
}

char __cdecl FX_ParseInt(const char **parse, int *value)
{
    *value = Com_ParseInt(parse);
    return 1;
}

bool __cdecl FX_ParseSpawnDefLooping(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseInt(parse, &edElemDef->spawnLooping.intervalMsec)
        && FX_ParseInt(parse, &edElemDef->spawnLooping.count) != 0;
}

bool __cdecl FX_ParseIntRange(const char **parse, FxIntRange *range)
{
    return FX_ParseInt(parse, &range->base) && FX_ParseInt(parse, &range->amplitude) != 0;
}

bool __cdecl FX_ParseSpawnDefOneShot(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseIntRange(parse, &edElemDef->spawnOneShot.count);
}

bool __cdecl FX_ParseSpawnDelayMsec(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseIntRange(parse, &edElemDef->spawnDelayMsec);
}

bool __cdecl FX_ParseLifeSpanMsec(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseIntRange(parse, &edElemDef->lifeSpanMsec);
}

bool __cdecl FX_ParseSpawnOrgX(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseFloatRange(parse, edElemDef->spawnOrigin);
}

bool __cdecl FX_ParseSpawnOrgY(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseFloatRange(parse, &edElemDef->spawnOrigin[1]);
}

bool __cdecl FX_ParseSpawnOrgZ(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseFloatRange(parse, &edElemDef->spawnOrigin[2]);
}

bool __cdecl FX_ParseSpawnOffsetRadius(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseFloatRange(parse, &edElemDef->spawnOffsetRadius);
}

bool __cdecl FX_ParseSpawnOffsetHeight(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseFloatRange(parse, &edElemDef->spawnOffsetHeight);
}

bool __cdecl FX_ParseSpawnAnglePitch(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseFloatRange(parse, edElemDef->spawnAngles);
}

bool __cdecl FX_ParseSpawnAngleYaw(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseFloatRange(parse, &edElemDef->spawnAngles[1]);
}

bool __cdecl FX_ParseSpawnAngleRoll(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseFloatRange(parse, &edElemDef->spawnAngles[2]);
}

bool __cdecl FX_ParseAngleVelPitch(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseFloatRange(parse, edElemDef->angularVelocity);
}

bool __cdecl FX_ParseAngleVelYaw(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseFloatRange(parse, &edElemDef->angularVelocity[1]);
}

bool __cdecl FX_ParseAngleVelRoll(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseFloatRange(parse, &edElemDef->angularVelocity[2]);
}

bool __cdecl FX_ParseInitialRot(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseFloatRange(parse, &edElemDef->initialRotation);
}

bool __cdecl FX_ParseGravity(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseFloatRange(parse, &edElemDef->gravity);
}

bool __cdecl FX_ParseElasticity(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseFloatRange(parse, &edElemDef->elasticity);
}

bool __cdecl FX_ParseAtlasBehavior(const char **parse, FxEditorElemDef *edElemDef)
{
    FxFlagOutputSet flagOutputSet; // [esp+0h] [ebp-Ch] BYREF

    flagOutputSet.flags[2] = &edElemDef->atlas.behavior;
    flagOutputSet.flags[0] = 0;
    flagOutputSet.flags[1] = 0;
    return FX_ParseFlagsField(parse, &flagOutputSet);
}

bool __cdecl FX_ParseAtlasIndex(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseInt(parse, &edElemDef->atlas.index);
}

bool __cdecl FX_ParseAtlasFps(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseInt(parse, &edElemDef->atlas.fps);
}

bool __cdecl FX_ParseAtlasLoopCount(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseInt(parse, &edElemDef->atlas.loopCount);
}

bool __cdecl FX_ParseAtlasColIndexBits(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseInt(parse, &edElemDef->atlas.colIndexBits);
}

bool __cdecl FX_ParseAtlasRowIndexBits(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseInt(parse, &edElemDef->atlas.rowIndexBits);
}

bool __cdecl FX_ParseAtlasEntryCount(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseInt(parse, &edElemDef->atlas.entryCount);
}

char __cdecl FX_ParseCurve(const char **parse, int dimCount, float minValue, float maxValue, const FxCurve **shape)
{
    long double v6; // st7
    float v7; // [esp+10h] [ebp-828h]
    float v8; // [esp+14h] [ebp-824h]
    float v9; // [esp+18h] [ebp-820h]
    float v10; // [esp+1Ch] [ebp-81Ch]
    float v11; // [esp+20h] [ebp-818h]
    int keyCount; // [esp+24h] [ebp-814h]
    float keys[513]; // [esp+28h] [ebp-810h] BYREF
    int keyIndex; // [esp+82Ch] [ebp-Ch]
    const char *token; // [esp+830h] [ebp-8h]
    int valCount; // [esp+834h] [ebp-4h]

    if (!Com_MatchToken(parse, "{", 1))
        return 0;
    for (valCount = 0; ; ++valCount)
    {
        token = (const char *)Com_Parse(parse);
        if (*token == 125)
            break;
        if (valCount == 512)
        {
            Com_ScriptError("%i-dimensional values cannot have more than %i keys.\n", dimCount, 0x200u / (dimCount + 1));
            return 0;
        }
        v6 = atof(token);
        keys[valCount] = v6;
        if (valCount % (dimCount + 1))
        {
            v10 = keys[valCount];
            v9 = v10 - maxValue;
            if (v9 < 0.0)
                v11 = v10;
            else
                v11 = maxValue;
            v8 = minValue - v10;
            if (v8 < 0.0)
                v7 = v11;
            else
                v7 = minValue;
            keys[valCount] = v7;
        }
    }
    if (valCount % (dimCount + 1))
    {
        Com_ScriptError("Curve has a partial key.\n");
        return 0;
    }
    else
    {
        keyCount = valCount / (dimCount + 1);
        if (keyCount >= 2)
        {
            if (keys[0] == 0.0 && keys[(dimCount + 1) * (keyCount - 1)] == 1.0)
            {
                for (keyIndex = 1; keyIndex < keyCount; ++keyIndex)
                {
                    if (keys[(dimCount + 1) * (keyIndex - 1)] >= (double)keys[keyIndex * (dimCount + 1)])
                    {
                        Com_ScriptError(
                            "Curves times must be monotonically increasing (key %i is at %g <= %g).\n",
                            keyIndex,
                            keys[keyIndex * (dimCount + 1)],
                            keys[(dimCount + 1) * (keyIndex - 1)]);
                        return 0;
                    }
                }
                *shape = FxCurve_AllocAndCreateWithKeys(keys, dimCount, keyCount);
                return 1;
            }
            else
            {
                Com_ScriptError("Curves must always start at time 0 and end at time 1.\n");
                return 0;
            }
        }
        else
        {
            Com_ScriptError("Curves must always have at least 2 keys.\n");
            return 0;
        }
    }
}

bool __cdecl FX_ParseGraphRange(
    const char **parse,
    int dimCount,
    float minValue,
    float maxValue,
    float *scale,
    const FxCurve **shape)
{
    if (!FX_ParseFloat(parse, scale))
        return 0;
    if (!Com_MatchToken(parse, "{", 1))
        return 0;
    if (!FX_ParseCurve(parse, dimCount, minValue, maxValue, shape))
        return 0;
    if (FX_ParseCurve(parse, dimCount, minValue, maxValue, shape + 1))
        return Com_MatchToken(parse, "}", 1) != 0;
    return 0;
}

bool __cdecl FX_ParseVelGraph0X(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseGraphRange(parse, 1, -0.5, 0.5, edElemDef->velScale[0], edElemDef->velShape[0][0]);
}

bool __cdecl FX_ParseVelGraph0Y(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseGraphRange(parse, 1, -0.5, 0.5, &edElemDef->velScale[0][1], edElemDef->velShape[0][1]);
}

bool __cdecl FX_ParseVelGraph0Z(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseGraphRange(parse, 1, -0.5, 0.5, &edElemDef->velScale[0][2], edElemDef->velShape[0][2]);
}

bool __cdecl FX_ParseVelGraph1X(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseGraphRange(parse, 1, -0.5, 0.5, edElemDef->velScale[1], edElemDef->velShape[1][0]);
}

bool __cdecl FX_ParseVelGraph1Y(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseGraphRange(parse, 1, -0.5, 0.5, &edElemDef->velScale[1][1], edElemDef->velShape[1][1]);
}

bool __cdecl FX_ParseVelGraph1Z(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseGraphRange(parse, 1, -0.5, 0.5, &edElemDef->velScale[1][2], edElemDef->velShape[1][2]);
}

bool __cdecl FX_ParseRotGraph(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseGraphRange(parse, 1, -0.5, 0.5, &edElemDef->rotationScale, edElemDef->rotationShape);
}

bool __cdecl FX_ParseSizeGraph0(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseGraphRange(parse, 1, 0.0, 1.0, edElemDef->sizeScale, edElemDef->sizeShape[0]);
}

bool __cdecl FX_ParseSizeGraph1(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseGraphRange(parse, 1, 0.0, 1.0, &edElemDef->sizeScale[1], edElemDef->sizeShape[1]);
}

bool __cdecl FX_ParseScaleGraph(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseGraphRange(parse, 1, 0.0, 1.0, &edElemDef->scaleScale, edElemDef->scaleShape);
}

bool __cdecl FX_ParseColorGraph(const char **parse, FxEditorElemDef *edElemDef)
{
    float scale; // [esp+10h] [ebp-4h] BYREF

    return FX_ParseGraphRange(parse, 3, 0.0, 1.0, &scale, edElemDef->color);
}

bool __cdecl FX_ParseAlphaGraph(const char **parse, FxEditorElemDef *edElemDef)
{
    float scale; // [esp+10h] [ebp-4h] BYREF

    return FX_ParseGraphRange(parse, 1, 0.0, 1.0, &scale, edElemDef->alpha);
}

bool __cdecl FX_ParseLightingFrac(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseFloat(parse, &edElemDef->lightingFrac);
}

bool __cdecl FX_ParseVector(const char **parse, float *value)
{
    if (!FX_ParseFloat(parse, value))
        return 0;
    if (FX_ParseFloat(parse, value + 1))
        return FX_ParseFloat(parse, value + 2) != 0;
    return 0;
}

bool __cdecl FX_ParseCollOffset(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseVector(parse, edElemDef->collOffset);
}

bool __cdecl FX_ParseCollRadius(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseFloat(parse, &edElemDef->collRadius);
}

char __cdecl FX_ParseEffectRef(const char **parse, const FxEffectDef **fx)
{
    parseInfo_t *token; // [esp+0h] [ebp-4h]

    token = Com_Parse(parse);
    if (token->token[0])
        *fx = FX_Register(token->token);
    else
        *fx = 0;
    return 1;
}

bool __cdecl FX_ParseFxOnImpact(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseEffectRef(parse, &edElemDef->effectOnImpact);
}

bool __cdecl FX_ParseFxOnDeath(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseEffectRef(parse, &edElemDef->effectOnDeath);
}

bool __cdecl FX_ParseSortOrder(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseInt(parse, &edElemDef->sortOrder);
}

bool __cdecl FX_ParseEmission(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseEffectRef(parse, &edElemDef->emission);
}

bool __cdecl FX_ParseEmitDist(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseFloatRange(parse, &edElemDef->emitDist);
}

bool __cdecl FX_ParseEmitDistVariance(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseFloatRange(parse, &edElemDef->emitDistVariance);
}

bool __cdecl FX_ParseTrailRepeatTime(const char **parse, FxEditorElemDef *edElemDef)
{
    float deprecated; // [esp+0h] [ebp-4h] BYREF

    return FX_ParseFloat(parse, &deprecated);
}

bool __cdecl FX_ParseTrailSplitDist(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseInt(parse, &edElemDef->trailSplitDist);
}

bool __cdecl FX_ParseTrailScrollTime(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseFloat(parse, &edElemDef->trailScrollTime);
}

bool __cdecl FX_ParseTrailRepeatDist(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseInt(parse, &edElemDef->trailRepeatDist);
}

bool __cdecl FX_ParseTrailDef(const char **parse, FxEditorElemDef *edElemDef)
{
    int index; // [esp+0h] [ebp-Ch] BYREF
    FxTrailVertex *trailVertex; // [esp+4h] [ebp-8h]
    const char *token; // [esp+8h] [ebp-4h]

    if (!Com_MatchToken(parse, "{", 1))
        return 0;
    for (edElemDef->trailDef.vertCount = 0; ; ++edElemDef->trailDef.vertCount)
    {
        token = (const char *)Com_Parse(parse);
        if (*token == 125)
            break;
        Com_UngetToken();
        if (edElemDef->trailDef.vertCount == 64)
        {
            Com_ScriptError("ran out of trail verts in edElemDef->trailDef\n");
            return 0;
        }
        trailVertex = &edElemDef->trailDef.verts[edElemDef->trailDef.vertCount];
        if (!FX_ParseFloat(parse, trailVertex->pos))
            return 0;
        if (!FX_ParseFloat(parse, &trailVertex->pos[1]))
            return 0;
        if (!FX_ParseFloat(parse, &trailVertex->texCoord))
            return 0;
    }
    if (!Com_MatchToken(parse, "{", 1))
        return 0;
    for (edElemDef->trailDef.indCount = 0; ; ++edElemDef->trailDef.indCount)
    {
        if (edElemDef->trailDef.indCount == 128)
        {
            Com_ScriptError("ran out of trail inds in edElemDef->trailDef\n");
            return 0;
        }
        token = (const char *)Com_Parse(parse);
        if (*token == 125)
            break;
        Com_UngetToken();
        if (!FX_ParseInt(parse, &index))
            return 0;
        edElemDef->trailDef.inds[edElemDef->trailDef.indCount] = index;
        if (index != edElemDef->trailDef.inds[edElemDef->trailDef.indCount])
            MyAssertHandler(
                ".\\EffectsCore\\fx_load_obj.cpp",
                912,
                0,
                "%s\n\t(index) = %i",
                "(index == edElemDef->trailDef.inds[edElemDef->trailDef.indCount])",
                index);
    }
    return 1;
}

bool __cdecl FX_SetEditorElemType(FxEditorElemDef *edElemDef, unsigned __int8 type)
{
    if (edElemDef->elemType == 11)
    {
        edElemDef->elemType = type;
        return 1;
    }
    else
    {
        Com_ScriptError("More than one type of visuals present in effect element\n");
        return 0;
    }
}

bool __cdecl FX_ParseAssetArray_FxElemVisuals_32_(
    const char **parse,
    unsigned __int8 elemType,
    FxEditorElemDef *edElemDef,
    FxElemVisuals(*visualsArray)[32],
    bool(__cdecl *RegisterAsset)(const char *, FxElemVisuals *))
{
    char name[264]; // [esp+0h] [ebp-110h] BYREF
    const char *token; // [esp+10Ch] [ebp-4h]

    if (!FX_SetEditorElemType(edElemDef, elemType))
        return 0;
    if (!Com_MatchToken(parse, "{", 1))
        return 0;
    for (edElemDef->visualCount = 0; ; ++edElemDef->visualCount)
    {
        token = (const char *)Com_Parse(parse);
        if (*token == 125)
            return 1;
        if (edElemDef->visualCount == 32)
        {
            Com_ScriptError("More than %i visuals in array\n", 32);
            return 0;
        }
        I_strncpyz(name, (char *)token, 260);
        if (!RegisterAsset(name, &(*visualsArray)[edElemDef->visualCount]))
            break;
    }
    return 0;
}

Material *__cdecl FX_RegisterMaterial(char *material)
{
    if (!strcmp(material, "$default"))
        material = (char*)"$default3d";
    return Material_RegisterHandle(material, 6);
}

BOOL __cdecl FX_RegisterAsset_Material(char *name, FxElemVisuals *visuals)
{
    visuals->anonymous = FX_RegisterMaterial(name);
    return visuals->anonymous != 0;
}

bool __cdecl FX_ParseBillboardSprite(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseAssetArray_FxElemVisuals_32_(
        parse,
        0,
        edElemDef,
        (FxElemVisuals(*)[32]) & edElemDef->visualunion,
        (bool(__cdecl *)(const char *, FxElemVisuals *))FX_RegisterAsset_Material);
}

bool __cdecl FX_ParseOrientedSprite(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseAssetArray_FxElemVisuals_32_(
        parse,
        1u,
        edElemDef,
        (FxElemVisuals(*)[32]) & edElemDef->visualunion,
        (bool(__cdecl *)(const char *, FxElemVisuals *))FX_RegisterAsset_Material);
}

bool __cdecl FX_ParseCloud(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseAssetArray_FxElemVisuals_32_(
        parse,
        4u,
        edElemDef,
        (FxElemVisuals(*)[32]) & edElemDef->visualunion,
        (bool(__cdecl *)(const char *, FxElemVisuals *))FX_RegisterAsset_Material);
}

bool __cdecl FX_ParseTail(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseAssetArray_FxElemVisuals_32_(
        parse,
        2u,
        edElemDef,
        (FxElemVisuals(*)[32]) & edElemDef->visualunion,
        (bool(__cdecl *)(const char *, FxElemVisuals *))FX_RegisterAsset_Material);
}

bool __cdecl FX_ParseTrail(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseAssetArray_FxElemVisuals_32_(
        parse,
        3u,
        edElemDef,
        (FxElemVisuals(*)[32]) & edElemDef->visualunion,
        (bool(__cdecl *)(const char *, FxElemVisuals *))FX_RegisterAsset_Material);
}

bool __cdecl FX_ParseAssetArray_FxElemMarkVisuals_16_(
    const char **parse,
    unsigned __int8 elemType,
    FxEditorElemDef *edElemDef,
    FxElemMarkVisuals(*visualsArray)[16],
    bool(__cdecl *RegisterAsset)(const char *, FxElemMarkVisuals *))
{
    char name[264]; // [esp+0h] [ebp-110h] BYREF
    const char *token; // [esp+10Ch] [ebp-4h]

    if (!FX_SetEditorElemType(edElemDef, elemType))
        return 0;
    if (!Com_MatchToken(parse, "{", 1))
        return 0;
    for (edElemDef->visualCount = 0; ; ++edElemDef->visualCount)
    {
        token = (const char *)Com_Parse(parse);
        if (*token == 125)
            return 1;
        if (edElemDef->visualCount == 16)
        {
            Com_ScriptError("More than %i visuals in array\n", 16);
            return 0;
        }
        I_strncpyz(name, (char *)token, 260);
        if (!RegisterAsset(name, &(*visualsArray)[edElemDef->visualCount]))
            break;
    }
    return 0;
}

bool __cdecl FX_RegisterMarkMaterials(const char *materialName, Material **materials)
{
    Material *v2; // eax
    char materialNameWithPrefix[260]; // [esp+14h] [ebp-118h] BYREF
    bool success; // [esp+11Fh] [ebp-Dh]
    const char *typePrefixes[2]; // [esp+120h] [ebp-Ch]
    int typeIndex; // [esp+128h] [ebp-4h]

    typePrefixes[0] = "mc";
    typePrefixes[1] = "wc";
    if (!strcmp(materialName, "$default"))
        materialName = "$default3d";
    success = 1;
    for (typeIndex = 0; typeIndex != 2; ++typeIndex)
    {
        Com_sprintf(materialNameWithPrefix, 0x100u, "%s/%s", typePrefixes[typeIndex], materialName);
        v2 = FX_RegisterMaterial(materialNameWithPrefix);
        materials[typeIndex] = v2;
        if (!materials[typeIndex])
            success = 0;
    }
    return success;
}

bool __cdecl FX_RegisterAsset_DecalMaterials(const char *name, FxElemMarkVisuals *visuals)
{
    return FX_RegisterMarkMaterials(name, visuals->materials);
}

bool __cdecl FX_ParseDecal(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseAssetArray_FxElemMarkVisuals_16_(
        parse,
        9u,
        edElemDef,
        (FxElemMarkVisuals(*)[16]) & edElemDef->visualunion,
        FX_RegisterAsset_DecalMaterials);
}

BOOL __cdecl FX_RegisterAsset_Model(const char *name, FxElemVisuals *visuals)
{
    visuals->anonymous = FX_RegisterModel(name);
    return visuals->anonymous != 0;
}

bool __cdecl FX_ParseModel(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseAssetArray_FxElemVisuals_32_(
        parse,
        5u,
        edElemDef,
        (FxElemVisuals(*)[32]) & edElemDef->visualunion,
        (bool(__cdecl *)(const char *, FxElemVisuals *))FX_RegisterAsset_Model);
}

bool __cdecl FX_ParseLight(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_SetEditorElemType(edElemDef, 6u);
}


bool __cdecl FX_ParseSpotLight(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_SetEditorElemType(edElemDef, 7u);
}

BOOL __cdecl FX_RegisterAsset_EffectDef(char *name, FxElemVisuals *visuals)
{
    visuals->anonymous = FX_Register(name);
    return visuals->anonymous != 0;
}

bool __cdecl FX_ParseRunner(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseAssetArray_FxElemVisuals_32_(
        parse,
        0xAu,
        edElemDef,
        (FxElemVisuals(*)[32]) & edElemDef->visualunion,
        (bool(__cdecl *)(const char *, FxElemVisuals *))FX_RegisterAsset_EffectDef);
}

char __cdecl FX_RegisterAsset_SoundAliasName(char *name, FxElemVisuals *visuals)
{
    ReplaceString((const char **)visuals, name);
    return 1;
}

bool __cdecl FX_ParseSound(const char **parse, FxEditorElemDef *edElemDef)
{
    return FX_ParseAssetArray_FxElemVisuals_32_(
        parse,
        8u,
        edElemDef,
        (FxElemVisuals(*)[32]) & edElemDef->visualunion,
        (bool(__cdecl *)(const char *, FxElemVisuals *))FX_RegisterAsset_SoundAliasName);
}

const FxElemField s_elemFields[69] =
{
  { "name", &FX_ParseName },
  { "editorFlags", &FX_ParseNonAtlasFlags },
  { "flags", &FX_ParseNonAtlasFlags },
  { "spawnRange", &FX_ParseSpawnRange },
  { "fadeInRange", &FX_ParseFadeInRange },
  { "fadeOutRange", &FX_ParseFadeOutRange },
  { "spawnFrustumCullRadius", &FX_ParseSpawnFrustumCullRadius },
  { "spawnLooping", &FX_ParseSpawnDefLooping },
  { "spawnOneShot", &FX_ParseSpawnDefOneShot },
  { "spawnDelayMsec", &FX_ParseSpawnDelayMsec },
  { "lifeSpanMsec", &FX_ParseLifeSpanMsec },
  { "spawnOrgX", &FX_ParseSpawnOrgX },
  { "spawnOrgY", &FX_ParseSpawnOrgY },
  { "spawnOrgZ", &FX_ParseSpawnOrgZ },
  { "spawnOffsetRadius", &FX_ParseSpawnOffsetRadius },
  { "spawnOffsetHeight", &FX_ParseSpawnOffsetHeight },
  { "spawnAnglePitch", &FX_ParseSpawnAnglePitch },
  { "spawnAngleYaw", &FX_ParseSpawnAngleYaw },
  { "spawnAngleRoll", &FX_ParseSpawnAngleRoll },
  { "angleVelPitch", &FX_ParseAngleVelPitch },
  { "angleVelYaw", &FX_ParseAngleVelYaw },
  { "angleVelRoll", &FX_ParseAngleVelRoll },
  { "initialRot", &FX_ParseInitialRot },
  { "gravity", &FX_ParseGravity },
  { "elasticity", &FX_ParseElasticity },
  { "atlasBehavior", &FX_ParseAtlasBehavior },
  { "atlasIndex", &FX_ParseAtlasIndex },
  { "atlasFps", &FX_ParseAtlasFps },
  { "atlasLoopCount", &FX_ParseAtlasLoopCount },
  { "atlasColIndexBits", &FX_ParseAtlasColIndexBits },
  { "atlasRowIndexBits", &FX_ParseAtlasRowIndexBits },
  { "atlasEntryCount", &FX_ParseAtlasEntryCount },
  { "velGraph0X", &FX_ParseVelGraph0X },
  { "velGraph0Y", &FX_ParseVelGraph0Y },
  { "velGraph0Z", &FX_ParseVelGraph0Z },
  { "velGraph1X", &FX_ParseVelGraph1X },
  { "velGraph1Y", &FX_ParseVelGraph1Y },
  { "velGraph1Z", &FX_ParseVelGraph1Z },
  { "rotGraph", &FX_ParseRotGraph },
  { "sizeGraph0", &FX_ParseSizeGraph0 },
  { "sizeGraph1", &FX_ParseSizeGraph1 },
  { "scaleGraph", &FX_ParseScaleGraph },
  { "colorGraph", &FX_ParseColorGraph },
  { "alphaGraph", &FX_ParseAlphaGraph },
  { "lightingFrac", &FX_ParseLightingFrac },
  { "collOffset", &FX_ParseCollOffset },
  { "collRadius", &FX_ParseCollRadius },
  { "fxOnImpact", &FX_ParseFxOnImpact },
  { "fxOnDeath", &FX_ParseFxOnDeath },
  { "sortOrder", &FX_ParseSortOrder },
  { "emission", &FX_ParseEmission },
  { "emitDist", &FX_ParseEmitDist },
  { "emitDistVariance", &FX_ParseEmitDistVariance },
  { "trailRepeatTime", &FX_ParseTrailRepeatTime },
  { "trailSplitDist", &FX_ParseTrailSplitDist },
  { "trailScrollTime", &FX_ParseTrailScrollTime },
  { "trailRepeatDist", &FX_ParseTrailRepeatDist },
  { "trailDef", &FX_ParseTrailDef },
  { "billboardSprite", &FX_ParseBillboardSprite },
  { "orientedSprite", &FX_ParseOrientedSprite },
  { "cloud", &FX_ParseCloud },
  { "tail", &FX_ParseTail },
  { "trail", &FX_ParseTrail },
  { "decal", &FX_ParseDecal },
  { "model", &FX_ParseModel },
  { "light", &FX_ParseLight },
  { "spotLight", &FX_ParseSpotLight },
  { "runner", &FX_ParseRunner },
  { "sound", &FX_ParseSound }
}; // idb

bool __cdecl FX_ParseEditorElemField(const char **parse, FxEditorElemDef *edElemDef, const char *token)
{
    unsigned int fieldIndex; // [esp+14h] [ebp-4h]
    unsigned int fieldIndexa; // [esp+14h] [ebp-4h]

    for (fieldIndex = 0; fieldIndex < 0x45; ++fieldIndex)
    {
        if (!strcmp(token, s_elemFields[fieldIndex].keyName))
            return s_elemFields[fieldIndex].handler(parse, edElemDef) && Com_MatchToken(parse, ";", 1) != 0;
    }
    Com_Printf(21, "Valid effects element fields:\n");
    for (fieldIndexa = 0; fieldIndexa < 0x45; ++fieldIndexa)
        Com_Printf(21, "  %s\n", s_elemFields[fieldIndexa].keyName);
    Com_ScriptError("unkown field '%s'\n", token);
    return 0;
}

char __cdecl FX_ParseEditorElem(int version, const char **parse, FxEditorElemDef *edElemDef)
{
    parseInfo_t *token; // [esp+0h] [ebp-4h]

    memset((unsigned __int8 *)edElemDef, 0, sizeof(FxEditorElemDef));
    if (edElemDef->flags)
        MyAssertHandler(".\\EffectsCore\\fx_load_obj.cpp", 1072, 0, "%s", "edElemDef->flags == 0");
    if (edElemDef->editorFlags)
        MyAssertHandler(".\\EffectsCore\\fx_load_obj.cpp", 1073, 0, "%s", "edElemDef->editorFlags == 0");
    if (edElemDef->lightingFrac != 0.0)
        MyAssertHandler(".\\EffectsCore\\fx_load_obj.cpp", 1074, 0, "%s", "edElemDef->lightingFrac == 0.0f");
    if (version < 2)
        edElemDef->editorFlags = 1024;
    if (edElemDef->atlas.behavior)
        MyAssertHandler(".\\EffectsCore\\fx_load_obj.cpp", 1079, 0, "%s", "edElemDef->atlas.behavior == 0");
    edElemDef->elemType = 11;
    edElemDef->sortOrder = 5;
    while (1)
    {
        token = Com_Parse(parse);
        if (token->token[0] == 125)
            break;
        if (!FX_ParseEditorElemField(parse, edElemDef, token->token))
            return 0;
    }
    if (edElemDef->elemType != 11)
        return 1;
    Com_ScriptError("no visual type specified\n");
    return 0;
}

char __cdecl FX_ParseEditorEffect(const char **parse, FxEditorEffectDef *edEffectDef)
{
    int version; // [esp+0h] [ebp-8h]
    parseInfo_t *token; // [esp+4h] [ebp-4h]

    version = Com_ParseInt(parse);
    if (version <= 2)
    {
        for (edEffectDef->elemCount = 0; ; ++edEffectDef->elemCount)
        {
            token = Com_Parse(parse);
            if (!*parse)
                return 1;
            if (token->token[0] != 123)
            {
                Com_ScriptError("Expected '{' to start a new segment, found '%s' instead.\n", token->token);
                return 0;
            }
            if (edEffectDef->elemCount == 32)
            {
                Com_ScriptError("Cannot have more than %i segments.\n", edEffectDef->elemCount);
                return 0;
            }
            if (!FX_ParseEditorElem(version, parse, &edEffectDef->elems[edEffectDef->elemCount]))
                break;
        }
        return 0;
    }
    else
    {
        Com_ScriptError("Version %i is too high. I can only handle up to %i.\n", version, 2);
        return 0;
    }
}

char __cdecl FX_LoadEditorEffectFromBuffer(
    const char *buffer,
    const char *parseSessionName,
    FxEditorEffectDef *edEffectDef)
{
    char success; // [esp+3h] [ebp-9h]
    const char *parse; // [esp+4h] [ebp-8h] BYREF
    const char *token; // [esp+8h] [ebp-4h]

    Com_BeginParseSession(parseSessionName);
    Com_SetSpaceDelimited(0);
    Com_SetParseNegativeNumbers(1);
    parse = buffer;
    token = (const char *)Com_Parse(&parse);
    if (I_stricmp(token, "iwfx"))
    {
        Com_ScriptError("Effect needs to be updated from the legacy format.\n");
        success = 0;
    }
    else
    {
        success = FX_ParseEditorEffect(&parse, edEffectDef);
    }
    Com_EndParseSession();
    return success;
}

bool __cdecl FX_LoadEditorEffect(const char *name, FxEditorEffectDef *edEffectDef)
{
    char v3; // [esp+3h] [ebp-5Dh]
    FxEditorEffectDef *v4; // [esp+8h] [ebp-58h]
    const char *v5; // [esp+Ch] [ebp-54h]
    char filename[64]; // [esp+10h] [ebp-50h] BYREF
    int fileSize; // [esp+54h] [ebp-Ch]
    bool success; // [esp+5Bh] [ebp-5h]
    void *fileData; // [esp+5Ch] [ebp-4h] BYREF

    if (!name)
        MyAssertHandler(".\\EffectsCore\\fx_load_obj.cpp", 1178, 0, "%s", "name");
    if (!edEffectDef)
        MyAssertHandler(".\\EffectsCore\\fx_load_obj.cpp", 1179, 0, "%s", "edEffectDef");
    Com_sprintf(filename, 0x40u, "fx/%s.efx", name);
    fileSize = FS_ReadFile(filename, &fileData);
    if (fileSize >= 0)
    {
        v5 = name;
        v4 = edEffectDef;
        do
        {
            v3 = *v5;
            v4->name[0] = *v5++;
            v4 = (FxEditorEffectDef *)((char *)v4 + 1);
        } while (v3);
        success = FX_LoadEditorEffectFromBuffer((const char *)fileData, filename, edEffectDef);
        FS_FreeFile((char *)fileData);
        return success;
    }
    else
    {
        Com_PrintError(21, "%s not found\n", filename);
        return 0;
    }
}

void* FX_AllocMem(unsigned int size)
{
    return Hunk_AllocAlign(size, 4, "FX_Alloc", 8);
}

PhysPreset *__cdecl FX_RegisterPhysPreset(const char *name)
{
    if (!name)
        MyAssertHandler(".\\EffectsCore\\fx_load_obj.cpp", 246, 0, "%s", "name");
    return PhysPresetPrecache(name, (void *(__cdecl *)(int))Hunk_AllocPhysPresetPrecache);
}

const FxEffectDef *__cdecl FX_LoadFailed(const char *name)
{
    char v2; // [esp+3h] [ebp-55h]
    _BYTE *v3; // [esp+8h] [ebp-50h]
    const char *v4; // [esp+Ch] [ebp-4Ch]
    const char *v5; // [esp+2Ch] [ebp-2Ch]
    _DWORD *v6; // [esp+30h] [ebp-28h]
    _DWORD *v7; // [esp+34h] [ebp-24h]
    unsigned int baseBytesNeeded; // [esp+40h] [ebp-18h]
    byte *effectDef; // [esp+48h] [ebp-10h]
    int relocationDistance; // [esp+4Ch] [ebp-Ch]
    int elemIndex; // [esp+54h] [ebp-4h]

    if (!fx_load.defaultEffect)
    {
        if (!I_stricmp(name, "misc/missing_fx"))
            Com_Error(ERR_FATAL, "Couldn't load default effect");
        FX_RegisterDefaultEffect();
        if (!fx_load.defaultEffect)
            MyAssertHandler(".\\EffectsCore\\fx_load_obj.cpp", 1220, 1, "%s", "fx_load.defaultEffect");
        if (!fx_load.defaultEffect->name)
            MyAssertHandler(".\\EffectsCore\\fx_load_obj.cpp", 1221, 1, "%s", "fx_load.defaultEffect->name");
        if (I_strcmp(fx_load.defaultEffect->name, "misc/missing_fx"))
            MyAssertHandler(
                ".\\EffectsCore\\fx_load_obj.cpp",
                1222,
                1,
                "%s",
                "!I_strcmp( fx_load.defaultEffect->name, FX_DEFAULT_EFFECT_NAME )");
    }
    v5 = &fx_load.defaultEffect->name[strlen(fx_load.defaultEffect->name) + 1];
    baseBytesNeeded = fx_load.defaultEffect->totalSize - (v5 - fx_load.defaultEffect->name);
    effectDef = (byte *)FX_AllocMem(fx_load.defaultEffect->totalSize - (v5 - (fx_load.defaultEffect->name + 1)) + strlen(name));
    memcpy(effectDef, (unsigned __int8 *)fx_load.defaultEffect, baseBytesNeeded);
    *(_DWORD *)effectDef = (_DWORD)&effectDef[baseBytesNeeded];
    v4 = name;
    v3 = *(_BYTE **)effectDef;
    do
    {
        v2 = *v4;
        *v3++ = *v4++;
    } while (v2);
    relocationDistance = effectDef - (unsigned __int8 *)fx_load.defaultEffect;
    *((_DWORD *)effectDef + 7) += effectDef - (unsigned __int8 *)fx_load.defaultEffect;
    for (elemIndex = 0; elemIndex < *((_DWORD *)effectDef + 5) + *((_DWORD *)effectDef + 4); ++elemIndex)
    {
        v7 = (_DWORD *)(*((_DWORD *)effectDef + 7) + 252 * elemIndex + 180);
        *v7 += relocationDistance;
        v6 = (_DWORD *)(*((_DWORD *)effectDef + 7) + 252 * elemIndex + 184);
        *v6 += relocationDistance;
    }
    return (const FxEffectDef *)effectDef;
}

const FxEffectDef *__cdecl FX_Load(const char *name)
{
    char v2; // [esp+3h] [ebp-10B61h]
    const char *v4; // [esp+Ch] [ebp-10B58h]
    const FxEffectDef *v5; // [esp+10h] [ebp-10B54h]
    FxEditorEffectDef edEffectDef; // [esp+14h] [ebp-10B50h] BYREF

    strcpy_s(edEffectDef.name, name);
    if (FX_LoadEditorEffect(name, &edEffectDef)
        && (v5 = FX_Convert(&edEffectDef, &FX_AllocMem)) != 0)
    {
        return v5;
    }
    else
    {
        return FX_LoadFailed(name);
    }
}

int __cdecl FX_HashName(const char *name)
{
    __int16 hash; // [esp+4h] [ebp-Ch]
    char letter; // [esp+Bh] [ebp-5h]
    int scale; // [esp+Ch] [ebp-4h]

    hash = 0;
    scale = 'w';
    while (*name)
    {
        letter = *name;
        if (*name == '\\')
        {
            letter = '/';
        }
        else if (letter >= 'A' && letter <= 'Z')
        {
            letter += ' ';
        }
        hash += scale * letter;
        ++name;
        ++scale;
    }
    return hash & 0x1FF;
}

int __cdecl FX_GetHashIndex(const char *name, bool *exists)
{
    int hashIndex; // [esp+0h] [ebp-4h]

    if (!name)
        MyAssertHandler(".\\EffectsCore\\fx_load_obj.cpp", 1302, 0, "%s", "name");
    if (!exists)
        MyAssertHandler(".\\EffectsCore\\fx_load_obj.cpp", 1303, 0, "%s", "exists");
    for (hashIndex = FX_HashName(name); fx_load.effectDefs[hashIndex]; hashIndex = ((_WORD)hashIndex + 1) & 0x1FF)
    {
        if (!I_stricmp(name, fx_load.effectDefs[hashIndex]->name))
        {
            *exists = 1;
            return hashIndex;
        }
    }
    *exists = 0;
    return hashIndex;
}

const FxEffectDef *__cdecl FX_Register_LoadObj(const char *name)
{
    int hashIndex; // [esp+0h] [ebp-8h]
    bool exists; // [esp+7h] [ebp-1h] BYREF

    hashIndex = FX_GetHashIndex(name, &exists);
    if (!exists)
        fx_load.effectDefs[hashIndex] = FX_Load(name);
    if (!fx_load.effectDefs[hashIndex])
        MyAssertHandler(".\\EffectsCore\\fx_load_obj.cpp", 1346, 1, "%s", "fx_load.effectDefs[hashIndex]");
    return fx_load.effectDefs[hashIndex];
}

const FxEffectDef *__cdecl FX_Register(const char *name)
{
    if (useFastFile->current.enabled)
        return FX_Register_FastFile(name);
    else
        return FX_Register_LoadObj(name);
}

const FxEffectDef *__cdecl FX_Register_FastFile(const char *name)
{
    if (!I_strncmp(name, "fx/", 3))
        MyAssertHandler(
            ".\\EffectsCore\\fx_load_obj.cpp",
            1330,
            0,
            "%s\n\t(name) = %s",
            "(I_strncmp( name, \"fx/\", 3 ))",
            name);
    return DB_FindXAssetHeader(ASSET_TYPE_FX, name).fx;
}

void __cdecl FX_RegisterDefaultEffect()
{
    fx_load.defaultEffect = FX_Register("misc/missing_fx");
}

void __cdecl FX_ForEachEffectDef(void(__cdecl* callback)(const FxEffectDef*, void*), void* data)
{
    int hashIndex; // [esp+0h] [ebp-4h]

    for (hashIndex = 0; hashIndex < 512; ++hashIndex)
    {
        if (fx_load.effectDefs[hashIndex])
            callback(fx_load.effectDefs[hashIndex], data);
    }
}