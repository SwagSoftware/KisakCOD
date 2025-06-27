#include "bg_public.h"
#include "bg_local.h"
#include <qcommon/mem_track.h>
#include <universal/q_parse.h>
#include <script/scr_animtree.h>

#include <string.h>
#include <game_mp/g_main_mp.h>
#include <xanim/dobj.h>
#include <universal/com_memory.h>
#include <script/scr_main.h>

animStringItem_t animParseModesStr[6] =
{
  { "defines", -1 },
  { "animations", -1 },
  { "canned_animations", -1 },
  { "statechanges", -1 },
  { "events", -1 },
  { NULL, -1 }
}; // idb

int numDefines[10];
char defineStrings[10000];
animStringItem_t defineStr[10][16];
unsigned int defineBits[10][16][2];
animStringItem_t weaponStrings[128];

animStringItem_t animStateStr[2] = { { "COMBAT", -1 }, { NULL, -1 } };

animStringItem_t animMoveTypesStr[44] =
{
  { "** UNUSED **", -1 },
  { "IDLE", -1 },
  { "IDLECR", -1 },
  { "IDLEPRONE", -1 },
  { "WALK", -1 },
  { "WALKBK", -1 },
  { "WALKCR", -1 },
  { "WALKCRBK", -1 },
  { "WALKPRONE", -1 },
  { "WALKPRONEBK", -1 },
  { "RUN", -1 },
  { "RUNBK", -1 },
  { "RUNCR", -1 },
  { "RUNCRBK", -1 },
  { "TURNRIGHT", -1 },
  { "TURNLEFT", -1 },
  { "TURNRIGHTCR", -1 },
  { "TURNLEFTCR", -1 },
  { "CLIMBUP", -1 },
  { "CLIMBDOWN", -1 },
  { "SPRINT", -1 },
  { "MANTLE_ROOT", -1 },
  { "MANTLE_UP_57", -1 },
  { "MANTLE_UP_51", -1 },
  { "MANTLE_UP_45", -1 },
  { "MANTLE_UP_39", -1 },
  { "MANTLE_UP_33", -1 },
  { "MANTLE_UP_27", -1 },
  { "MANTLE_UP_21", -1 },
  { "MANTLE_OVER_HIGH", -1 },
  { "MANTLE_OVER_MID", -1 },
  { "MANTLE_OVER_LOW", -1 },
  { "FLINCH_FORWARD", -1 },
  { "FLINCH_BACKWARD", -1 },
  { "FLINCH_LEFT", -1 },
  { "FLINCH_RIGHT", -1 },
  { "STUMBLE_FORWARD", -1 },
  { "STUMBLE_BACKWARD", -1 },
  { "STUMBLE_WALK_FORWARD", -1 },
  { "STUMBLE_WALK_BACKWARD", -1 },
  { "STUMBLE_CROUCH_FORWARD", -1 },
  { "STUMBLE_CROUCH_BACKWARD", -1 },
  { "STUMBLE_SPRINT_FORWARD", -1 },
  { NULL, -1 }
};

animStringItem_t animEventTypesStr[22] =
{
  { "PAIN", -1 },
  { "DEATH", -1 },
  { "FIREWEAPON", -1 },
  { "JUMP", -1 },
  { "JUMPBK", -1 },
  { "LAND", -1 },
  { "DROPWEAPON", -1 },
  { "RAISEWEAPON", -1 },
  { "CLIMBMOUNT", -1 },
  { "CLIMBDISMOUNT", -1 },
  { "RELOAD", -1 },
  { "CROUCH_TO_PRONE", -1 },
  { "PRONE_TO_CROUCH", -1 },
  { "STAND_TO_CROUCH", -1 },
  { "CROUCH_TO_STAND", -1 },
  { "STAND_TO_PRONE", -1 },
  { "PRONE_TO_STAND", -1 },
  { "MELEEATTACK", -1 },
  { "KNIFE_MELEE", -1 },
  { "KNIFE_MELEE_CHARGE", -1 },
  { "SHELLSHOCK", -1 },
  { NULL, -1 }
};

animStringItem_t animBodyPartsStr[5] =
{
  { "** UNUSED **", -1 },
  { "LEGS", -1 },
  { "TORSO", -1 },
  { "BOTH", -1 },
  { NULL, -1 }
};

animStringItem_t animConditionMountedStr[3] = { { "** UNUSED **", -1 }, { "MG42", -1 }, { NULL, -1 } };

animStringItem_t animWeaponClassStr[11] =
{
  { "RIFLE", -1 },
  { "MG", -1 },
  { "SMG", -1 },
  { "SPREAD", -1 },
  { "PISTOL", -1 },
  { "GRENADE", -1 },
  { "ROCKETLAUNCHER", -1 },
  { "TURRET", -1 },
  { "NON-PLAYER", -1 },
  { "ITEM", -1 },
  { NULL, -1 }
};

animStringItem_t animWeaponPositionStr[3] = { { "HIP", -1 }, { "ADS", -1 }, { NULL, -1 } };

animStringItem_t animStrafeStateStr[4] = { { "NOT", -1 }, { "LEFT", -1 }, { "RIGHT", -1 }, { NULL, -1 } };

animStringItem_t animPerkStateStr[4] =
{
  { "** UNUSED **", -1 },
  { "LASTSTAND", -1 },
  { "GRENADEDEATH", -1 },
  { NULL, -1 }
};
animStringItem_t animConditionsStr[11] =
{
  { "PLAYERANIMTYPE", -1 },
  { "WEAPONCLASS", -1 },
  { "MOUNTED", -1 },
  { "MOVETYPE", -1 },
  { "UNDERHAND", -1 },
  { "CROUCHING", -1 },
  { "FIRING", -1 },
  { "WEAPON_POSITION", -1 },
  { "STRAFING", -1 },
  { "PERK", -1 },
  { NULL, -1 }
};

animConditionTable_t animConditionsTable[10] =
{
  { ANIM_CONDTYPE_BITFLAGS, weaponStrings },
  { ANIM_CONDTYPE_BITFLAGS, animWeaponClassStr },
  { ANIM_CONDTYPE_VALUE, animConditionMountedStr },
  { ANIM_CONDTYPE_BITFLAGS, animMoveTypesStr },
  { ANIM_CONDTYPE_VALUE, NULL },
  { ANIM_CONDTYPE_VALUE, NULL },
  { ANIM_CONDTYPE_VALUE, NULL },
  { ANIM_CONDTYPE_VALUE, animWeaponPositionStr },
  { ANIM_CONDTYPE_VALUE, animStrafeStateStr },
  { ANIM_CONDTYPE_VALUE, animPerkStateStr }
};


const char *globalFilename = "mp/playeranim.script";

bgs_t *bgs;

loadAnim_t *g_pLoadAnims;
unsigned int *g_piNumLoadAnims;
animScriptData_t *globalScriptData;
scriptAnimMoveTypes_t parseMovetype;
int parseEvent;

unsigned int defineStringsOffset;

void __cdecl TRACK_bg_animation_mp()
{
    TRACK_STATIC_ARR(numDefines, 9);
    TRACK_STATIC_ARR(defineStrings, 9);
    TRACK_STATIC_ARR(defineStr, 9);
    TRACK_STATIC_ARR(defineBits, 9);
    TRACK_STATIC_ARR(weaponStrings, 9);
    TRACK_STATIC_ARR(animStateStr, 9);
    TRACK_STATIC_ARR(animMoveTypesStr, 9);
    TRACK_STATIC_ARR(animEventTypesStr, 9);
    TRACK_STATIC_ARR(animBodyPartsStr, 9);
    TRACK_STATIC_ARR(animConditionMountedStr, 9);
    TRACK_STATIC_ARR(animWeaponClassStr, 9);
    TRACK_STATIC_ARR(animWeaponPositionStr, 9);
    TRACK_STATIC_ARR(animStrafeStateStr, 9);
    TRACK_STATIC_ARR(animPerkStateStr, 9);
    TRACK_STATIC_ARR(animConditionsStr, 9);
    TRACK_STATIC_ARR(animConditionsTable, 9);
}

void BG_AnimParseError(const char *msg, ...)
{
    int CurrentParseLine; // eax
    char text[1028]; // [esp+4h] [ebp-408h] BYREF
    va_list va; // [esp+418h] [ebp+Ch] BYREF

    va_start(va, msg);
    _vsnprintf_s(text, 0x400u, _TRUNCATE, msg, va);
    if (globalFilename)
    {
        CurrentParseLine = Com_GetCurrentParseLine();
        Com_Error(ERR_DROP, "%s: (%s, line %i)", text, globalFilename, CurrentParseLine + 1);
    }
    else
    {
        Com_Error(ERR_DROP, "%s", text);
    }
}
unsigned int __cdecl BG_AnimationIndexForString(const char *string)
{
    char v2; // [esp+3h] [ebp-1Dh]
    char *szAnimName; // [esp+8h] [ebp-18h]
    const char *v4; // [esp+Ch] [ebp-14h]
    int hash; // [esp+10h] [ebp-10h]
    int hasha; // [esp+10h] [ebp-10h]
    loadAnim_t *loadAnim; // [esp+14h] [ebp-Ch]
    loadAnim_t *loadAnima; // [esp+14h] [ebp-Ch]
    animScriptData_t *anim; // [esp+18h] [ebp-8h]
    unsigned int i; // [esp+1Ch] [ebp-4h]
    unsigned int ia; // [esp+1Ch] [ebp-4h]

    BG_CheckThread();

    if (!bgs)
        MyAssertHandler(".\\bgame\\bg_animation_mp.cpp", 424, 0, "%s", "bgs");

    if (g_pLoadAnims)
    {
        hasha = BG_StringHashValue(string);
        ia = 0;
        loadAnim = g_pLoadAnims;
        while (ia < *g_piNumLoadAnims)
        {
            if (hasha == loadAnim->iNameHash && !I_stricmp(string, loadAnim->szAnimName))
                return ia;
            ++ia;
            ++loadAnim;
        }
        loadAnima = &g_pLoadAnims[*g_piNumLoadAnims];
        Scr_FindAnim("multiplayer", string, &loadAnima->anim, bgs->anim_user);
        v4 = string;
        szAnimName = loadAnima->szAnimName;
        do
        {
            v2 = *v4;
            *szAnimName++ = *v4++;
        } while (v2);
        loadAnima->iNameHash = hasha;
        return (*g_piNumLoadAnims)++;
    }
    else
    {
        hash = BG_StringHashValue(string);
        i = 0;
        anim = globalScriptData;
        while (i < globalScriptData->numAnimations)
        {
            if (hash == anim->animations[0].nameHash && !I_stricmp(string, (const char *)anim))
                return i;
            ++i;
            anim = (animScriptData_t *)((char *)anim + 104);
        }
        BG_AnimParseError("BG_AnimationIndexForString: unknown player animation '%s'", string);
        return -1;
    }
}

int __cdecl BG_StringHashValue(const char *fname)
{
    int hash; // [esp+0h] [ebp-Ch]
    int i; // [esp+8h] [ebp-4h]

    hash = 0;
    for (i = 0; fname[i]; ++i)
        hash += (i + 119) * (char)tolower(fname[i]);
    if (hash == -1)
        return 0;
    return hash;
}

animScriptParseMode_t __cdecl BG_IndexForString(const char *token, animStringItem_t *strings, int allowFail)
{
    int hash; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    hash = BG_StringHashValue(token);
    i = 0;
    while (strings->string)
    {
        if (strings->hash == -1)
            strings->hash = BG_StringHashValue(strings->string);
        if (hash == strings->hash && !I_stricmp(token, strings->string))
            return (animScriptParseMode_t)i;
        ++strings;
        ++i;
    }
    if (!allowFail)
        BG_AnimParseError("BG_IndexForString: unknown token '%s'", token);
    return (animScriptParseMode_t)-1;
}

void __cdecl BG_InitWeaponString(int index, const char *name)
{
    weaponStrings[index].string = name;
    weaponStrings[index].hash = BG_StringHashValue(name);
}

void __cdecl BG_InitWeaponStrings()
{
    memset((unsigned __int8 *)weaponStrings, 0, sizeof(weaponStrings));
    BG_LoadWeaponStrings();
}

void __cdecl BG_ParseCommands(const char **input, animScriptItem_t *scriptItem, animScriptData_t *scriptData)
{
    int v3; // eax
    int v4; // eax
    parseInfo_t *v5; // [esp+24h] [ebp-18h]
    int partIndex; // [esp+28h] [ebp-14h]
    parseInfo_t *token; // [esp+2Ch] [ebp-10h]
    parseInfo_t *tokena; // [esp+2Ch] [ebp-10h]
    parseInfo_t *tokenb; // [esp+2Ch] [ebp-10h]
    parseInfo_t *tokenc; // [esp+2Ch] [ebp-10h]
    parseInfo_t *tokend; // [esp+2Ch] [ebp-10h]
    parseInfo_t *tokene; // [esp+2Ch] [ebp-10h]
    animScriptCommand_t *command; // [esp+30h] [ebp-Ch]
    int i; // [esp+34h] [ebp-8h]
    int bCommandFound; // [esp+38h] [ebp-4h]

    command = 0;
    partIndex = 0;
    while (1)
    {
        v5 = partIndex >= 1 ? Com_ParseOnLine(input) : Com_Parse(input);
        if (!v5 || !v5->token[0])
            break;
        if (!I_stricmp(v5->token, "}"))
        {
            *input -= strlen(v5->token);
            return;
        }
        if (!partIndex)
        {
            if (scriptItem->numCommands >= 8)
                BG_AnimParseError("BG_ParseCommands: exceeded maximum number of animations (%i)", 8);
            command = &scriptItem->commands[scriptItem->numCommands++];
            *(_DWORD *)command->bodyPart = 0;
        }
        command->bodyPart[partIndex] = BG_IndexForString(v5->token, animBodyPartsStr, 1);
        if (command->bodyPart[partIndex] <= 0)
        {
            *input -= strlen(v5->token);
            goto LABEL_72;
        }
        token = Com_ParseOnLine(input);
        if (!token || !token->token[0])
            BG_AnimParseError("BG_ParseCommands: expected animation");
        command->animIndex[partIndex] = BG_AnimationIndexForString(token->token);
        command->animDuration[partIndex] = scriptData->animations[command->animIndex[partIndex]].duration;
        if (!g_pLoadAnims)
        {
            if (parseMovetype && command->bodyPart[partIndex] != 2)
            {
                scriptData->animations[command->animIndex[partIndex]].movetype |= 1LL << parseMovetype;
                if ((parseMovetype == ANIM_MT_CLIMBUP || parseMovetype == ANIM_MT_CLIMBDOWN)
                    && scriptData->animations[command->animIndex[partIndex]].moveSpeed != 0.0)
                {
                    scriptData->animations[command->animIndex[partIndex]].flags |= 2u;
                }
                for (i = 0; ; ++i)
                {
                    if (i >= scriptItem->numConditions)
                        goto LABEL_34;
                    if (scriptItem->conditions[i].index == 8)
                        break;
                }
                if (scriptItem->conditions[i].value[0] == 1)
                {
                    scriptData->animations[command->animIndex[partIndex]].flags |= 0x10u;
                }
                else if (scriptItem->conditions[i].value[0] == 2)
                {
                    scriptData->animations[command->animIndex[partIndex]].flags |= 0x20u;
                }
            }
        LABEL_34:
            switch (parseEvent)
            {
            case 2:
                scriptData->animations[command->animIndex[partIndex]].flags |= 8u;
                scriptData->animations[command->animIndex[partIndex]].initialLerp = 30;
                break;
            case 18:
            case 19:
                scriptData->animations[command->animIndex[partIndex]].flags |= 0x100u;
                break;
            case 1:
                scriptData->animations[command->animIndex[partIndex]].moveSpeed = 0.0;
                scriptData->animations[command->animIndex[partIndex]].flags |= 0x40u;
                break;
            case 10:
                scriptData->animations[command->animIndex[partIndex]].moveSpeed = 0.0;
                break;
            default:
                if (parseMovetype >= ANIM_MT_MANTLE_UP_57 && parseMovetype <= ANIM_MT_MANTLE_OVER_LOW)
                    scriptData->animations[command->animIndex[partIndex]].moveSpeed = 0.0;
                break;
            }
            goto LABEL_46;
        }
        do
        {
        LABEL_46:
            bCommandFound = 0;
            tokena = Com_ParseOnLine(input);
            if (!tokena || !tokena->token[0])
            {
            LABEL_66:
                Com_UngetToken();
                continue;
            }
            if (I_stricmp(tokena->token, "duration"))
            {
                if (I_stricmp(tokena->token, "turretanim"))
                {
                    if (I_stricmp(tokena->token, "blendtime"))
                        goto LABEL_66;
                    bCommandFound = 1;
                    tokenc = Com_ParseOnLine(input);
                    if (!tokenc || !tokenc->token[0])
                        BG_AnimParseError("BG_ParseCommands: expected blendtime value");
                    if (!g_pLoadAnims)
                        scriptData->animations[command->animIndex[partIndex]].initialLerp = atoi(tokenc->token);
                }
                else
                {
                    bCommandFound = 1;
                    if (!g_pLoadAnims)
                        scriptData->animations[command->animIndex[partIndex]].flags |= 4u;
                    if (command->bodyPart[partIndex] != 3)
                        BG_AnimParseError("BG_ParseCommands: Turret animations can only be played on the 'both' body part");
                }
            }
            else
            {
                bCommandFound = 1;
                tokenb = Com_ParseOnLine(input);
                if (!tokenb || !tokenb->token[0])
                    BG_AnimParseError("BG_ParseCommands: expected duration value");
                command->animDuration[partIndex] = atoi(tokenb->token);
            }
        } while (bCommandFound);
        if (command->bodyPart[partIndex] != 3)
        {
            v3 = partIndex++;
            if (v3 < 1)
                continue;
        }
    LABEL_72:
        while (1)
        {
            tokend = Com_ParseOnLine(input);
            if (!tokend || !tokend->token[0])
                break;
            if (I_stricmp(tokend->token, "sound"))
            {
                BG_AnimParseError("BG_ParseCommands: unknown parameter '%s'", tokend->token);
            }
            else
            {
                tokene = Com_ParseOnLine(input);
                if (!tokene || !tokene->token[0])
                    BG_AnimParseError("BG_ParseCommands: expected sound");
                if (strstr((const char*)tokene, ".wav"))
                    BG_AnimParseError("BG_ParseCommands: wav files not supported, only sound scripts");
                command->soundAlias = globalScriptData->soundAlias((const char*)tokene);
            }
        }
        partIndex = 0;
    }
}

int __cdecl GetValueForBitfield(unsigned int bitfield)
{
    int i; // [esp+0h] [ebp-8h]

    for (i = 0; i < 32; ++i)
    {
        if (Com_BitCheckAssert(&bitfield, i, 0xFFFFFFF))
            return i;
    }
    return 0;
}

bool __cdecl Com_BitCheckAssert(const unsigned int *array, int bitNum, int size)
{
    if (!array)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../universal/q_shared.h", 704, 0, "%s", "array");
    if (bitNum >= (unsigned int)(8 * size))
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\bgame\\../universal/q_shared.h",
            705,
            0,
            "bitNum doesn't index size * 8\n\t%i not in [0, %i)",
            bitNum,
            8 * size);
    return (array[bitNum >> 5] & (1 << (bitNum & 0x1F))) != 0;
}

int __cdecl BG_PlayAnim(
    playerState_s *ps,
    int animNum,
    animBodyPart_t bodyPart,
    int forceDuration,
    int setTimer,
    int isContinue,
    int force)
{
    int wasSet; // [esp+14h] [ebp-8h]
    int duration; // [esp+18h] [ebp-4h]

    wasSet = 0;
    if (forceDuration)
        duration = forceDuration;
    else
        duration = globalScriptData->animations[animNum].duration + 50;
    if (bodyPart != ANIM_BP_LEGS)
    {
        if (bodyPart == ANIM_BP_TORSO)
        {
        LABEL_32:
            if (ps->torsoTimer < 50 || force)
            {
                if (isContinue && (ps->torsoAnim & 0xFFFFFDFF) == animNum)
                {
                    if (setTimer && (globalScriptData->animations[animNum].flags & 0x80) != 0)
                        ps->torsoTimer = duration;
                }
                else
                {
                    ps->torsoAnim = animNum | ps->torsoAnim & 0x200 ^ 0x200;
                    if (setTimer)
                        ps->torsoTimer = duration;
                    ps->torsoAnimDuration = duration;
                    if (xanim_debug->current.enabled)
                    {
                        if (bodyPart == ANIM_BP_BOTH)
                            Com_Printf(
                                19,
                                "Playing (client %i) %s on %s\n",
                                ps->clientNum,
                                globalScriptData->animations[animNum].name,
                                "body");
                        else
                            Com_Printf(
                                19,
                                "Playing (client %i) %s on %s\n",
                                ps->clientNum,
                                globalScriptData->animations[animNum].name,
                                "torso");
                    }
                }
            }
            goto LABEL_46;
        }
        if (bodyPart != ANIM_BP_BOTH)
            goto LABEL_46;
    }
    if (ps->legsTimer < 50 || force)
    {
        if (isContinue && (ps->legsAnim & 0xFFFFFDFF) == animNum)
        {
            if (setTimer && (globalScriptData->animations[animNum].flags & 0x80) != 0)
            {
                ps->legsTimer = duration;
            }
            else if (xanim_debug->current.enabled && (ps->legsAnim & 0xFFFFFDFF) != animNum)
            {
                Com_Printf(19, "anim failed because");
                if ((ps->legsAnim & 0xFFFFFDFF) == animNum)
                {
                    Com_Printf(19, ", isContinue is true");
                    Com_Printf(
                        19,
                        ", legsAnim is %s, asking to play %s",
                        globalScriptData->animations[ps->legsAnim].name,
                        globalScriptData->animations[animNum].name);
                }
                if (setTimer)
                {
                    if ((globalScriptData->animations[animNum].flags & 0x80) == 0)
                        Com_Printf(19, ", on a non-looped anim");
                }
                else
                {
                    Com_Printf(19, ", setTimer is false");
                }
                Com_Printf(19, "\n");
            }
        }
        else
        {
            wasSet = 1;
            ps->legsAnimDuration = duration;
            ps->legsAnim = animNum | ps->legsAnim & 0x200 ^ 0x200;
            if (setTimer)
                ps->legsTimer = duration;
            if (xanim_debug->current.enabled)
            {
                if (bodyPart == ANIM_BP_BOTH)
                    Com_Printf(
                        19,
                        "Playing (client %i) %s on %s\n",
                        ps->clientNum,
                        globalScriptData->animations[animNum].name,
                        "body");
                else
                    Com_Printf(
                        19,
                        "Playing (client %i) %s on %s\n",
                        ps->clientNum,
                        globalScriptData->animations[animNum].name,
                        "legs");
            }
        }
    }
    if (bodyPart == ANIM_BP_BOTH)
    {
        animNum = 0;
        goto LABEL_32;
    }
LABEL_46:
    if (wasSet)
        return duration;
    else
        return -1;
}

int __cdecl BG_ExecuteCommand(
    playerState_s *ps,
    animScriptCommand_t *scriptCommand,
    int setTimer,
    int isContinue,
    int force)
{
    int duration; // [esp+0h] [ebp-8h]
    bool playedLegsAnim; // [esp+4h] [ebp-4h]

    duration = -1;
    playedLegsAnim = 0;
    if (scriptCommand->bodyPart[0])
    {
        duration = scriptCommand->animDuration[0] + 50;
        if (scriptCommand->bodyPart[0] == 1 || scriptCommand->bodyPart[0] == 3)
            playedLegsAnim = BG_PlayAnim(
                ps,
                scriptCommand->animIndex[0],
                (animBodyPart_t)scriptCommand->bodyPart[0],
                duration,
                setTimer,
                isContinue,
                force) > -1;
        else
            BG_PlayAnim(
                ps,
                scriptCommand->animIndex[0],
                (animBodyPart_t)scriptCommand->bodyPart[0],
                duration,
                setTimer,
                isContinue,
                force);
    }
    if (scriptCommand->bodyPart[1])
    {
        duration = scriptCommand->animDuration[0] + 50;
        if (scriptCommand->bodyPart[0] == 1 || scriptCommand->bodyPart[0] == 3)
            playedLegsAnim = BG_PlayAnim(
                ps,
                scriptCommand->animIndex[1],
                (animBodyPart_t)scriptCommand->bodyPart[1],
                duration,
                setTimer,
                isContinue,
                force) > -1;
        else
            BG_PlayAnim(
                ps,
                scriptCommand->animIndex[1],
                (animBodyPart_t)scriptCommand->bodyPart[1],
                duration,
                setTimer,
                isContinue,
                force);
    }
    if (scriptCommand->soundAlias)
        globalScriptData->playSoundAlias(ps->clientNum, scriptCommand->soundAlias);
    if (playedLegsAnim)
        return duration;
    else
        return -1;
}

int __cdecl BG_AnimScriptAnimation(playerState_s *ps, aistateEnum_t state, scriptAnimMoveTypes_t movetype, int force)
{
    animScriptItem_t *scriptItem; // [esp+8h] [ebp-4h]

    scriptItem = 0;
    BG_CheckThread();
    if (!bgs)
        MyAssertHandler(".\\bgame\\bg_animation_mp.cpp", 2048, 0, "%s", "bgs");
    if (movetype < ANIM_MT_UNUSED)
        MyAssertHandler(".\\bgame\\bg_animation_mp.cpp", 2050, 0, "%s", "movetype >= 0");
    if (movetype >= NUM_ANIM_MOVETYPES)
        MyAssertHandler(".\\bgame\\bg_animation_mp.cpp", 2051, 0, "%s", "movetype < NUM_ANIM_MOVETYPES");
    if (ps->clientNum >= 0x40u)
        MyAssertHandler(
            ".\\bgame\\bg_animation_mp.cpp",
            2053,
            0,
            "ps->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            ps->clientNum,
            64);
    if ((BG_GetConditionBit(&bgs->clientinfo[ps->clientNum], 3u) & 0x80000000) != 0)
        MyAssertHandler(
            ".\\bgame\\bg_animation_mp.cpp",
            2054,
            0,
            "%s",
            "BG_GetConditionBit( &bgs->clientinfo[ps->clientNum], ANIM_COND_MOVETYPE ) >= ANIM_MT_UNUSED");
    if ((int)BG_GetConditionBit(&bgs->clientinfo[ps->clientNum], 3u) >= 43)
        MyAssertHandler(
            ".\\bgame\\bg_animation_mp.cpp",
            2055,
            0,
            "%s",
            "BG_GetConditionBit( &bgs->clientinfo[ps->clientNum], ANIM_COND_MOVETYPE ) < NUM_ANIM_MOVETYPES");
    if (ps->pm_type >= 7)
        return -1;
    while (!scriptItem && state >= AISTATE_COMBAT)
    {
        if (globalScriptData->scriptAnims[state][movetype].numItems)
        {
            scriptItem = BG_FirstValidItem(ps->clientNum, &globalScriptData->scriptAnims[state][movetype]);
            if (!scriptItem)
                state = state--;
        }
        else
        {
            state = state--;
        }
    }
    if (scriptItem)
    {
        if (scriptItem->numCommands)
        {
            BG_SetConditionBit(ps->clientNum, 3, movetype);
            if (ps->clientNum >= 0x40u)
                MyAssertHandler(
                    ".\\bgame\\bg_animation_mp.cpp",
                    2100,
                    0,
                    "ps->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    ps->clientNum,
                    64);
            if ((BG_GetConditionBit(&bgs->clientinfo[ps->clientNum], 3u) & 0x80000000) != 0)
                MyAssertHandler(
                    ".\\bgame\\bg_animation_mp.cpp",
                    2101,
                    0,
                    "%s",
                    "BG_GetConditionBit( &bgs->clientinfo[ps->clientNum], ANIM_COND_MOVETYPE ) >= ANIM_MT_UNUSED");
            if ((int)BG_GetConditionBit(&bgs->clientinfo[ps->clientNum], 3u) >= 43)
                MyAssertHandler(
                    ".\\bgame\\bg_animation_mp.cpp",
                    2102,
                    0,
                    "%s",
                    "BG_GetConditionBit( &bgs->clientinfo[ps->clientNum], ANIM_COND_MOVETYPE ) < NUM_ANIM_MOVETYPES");
            if (!scriptItem->numCommands)
                MyAssertHandler(".\\bgame\\bg_animation_mp.cpp", 2105, 0, "%s", "scriptItem->numCommands");
            return BG_ExecuteCommand(ps, &scriptItem->commands[ps->clientNum % scriptItem->numCommands], 0, 1, force) != -1;
        }
        else
        {
            if (xanim_debug->current.enabled)
                Com_Printf(19, "Animation has no commands associated, finding new animation\n");
            return -1;
        }
    }
    else
    {
        if (xanim_debug->current.enabled)
            Com_Printf(19, "Failed playing animation, finding new animation\n");
        return -1;
    }
}

animScriptItem_t *__cdecl BG_FirstValidItem(unsigned int client, animScript_t *script)
{
    const char *BodyPart; // eax
    int command; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    animScriptItem_t **ppScriptItem; // [esp+8h] [ebp-4h]

    BG_CheckThread();
    if (!bgs)
        MyAssertHandler(".\\bgame\\bg_animation_mp.cpp", 1841, 0, "%s", "bgs");
    i = 0;
    ppScriptItem = script->items;
    while (i < script->numItems)
    {
        if (animscript_debug->current.enabled)
        {
            Com_Printf(19, "Evaluating whether to play: ");
            for (command = 0; command < (*ppScriptItem)->numCommands; ++command)
            {
                BodyPart = GetBodyPart((*ppScriptItem)->commands[command].bodyPart[0]);
                Com_Printf(
                    19,
                    "%s on %s",
                    globalScriptData->animations[(*ppScriptItem)->commands[command].animIndex[0]].name,
                    BodyPart);
                if (command > 0)
                    Com_Printf(19, ", ");
            }
            Com_Printf(19, "\n");
        }
        if (client >= 0x40)
            MyAssertHandler(
                ".\\bgame\\bg_animation_mp.cpp",
                1859,
                0,
                "client doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                client,
                64);
        if (BG_EvaluateConditions(&bgs->clientinfo[client], *ppScriptItem))
            return *ppScriptItem;
        ++i;
        ++ppScriptItem;
    }
    return 0;
}

int __cdecl BG_EvaluateConditions(clientInfo_t *ci, animScriptItem_t *scriptItem)
{
    int ValueForBitfield; // eax
    const char *WeaponTypeName; // eax
    int v4; // eax
    const char *MoveTypeName; // eax
    animScriptConditionTypes_t type; // [esp+0h] [ebp-14h]
    animScriptCondition_t *cond; // [esp+8h] [ebp-Ch]
    int index; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    if ((BG_GetConditionBit(ci, 3u) & 0x80000000) != 0)
        MyAssertHandler(
            ".\\bgame\\bg_animation_mp.cpp",
            1720,
            0,
            "%s",
            "BG_GetConditionBit( ci, ANIM_COND_MOVETYPE ) >= ANIM_MT_UNUSED");
    if ((int)BG_GetConditionBit(ci, 3u) >= 43)
        MyAssertHandler(
            ".\\bgame\\bg_animation_mp.cpp",
            1721,
            0,
            "%s",
            "BG_GetConditionBit( ci, ANIM_COND_MOVETYPE ) < NUM_ANIM_MOVETYPES");
    i = 0;
    cond = scriptItem->conditions;
    while (i < scriptItem->numConditions)
    {
        if (animscript_debug->current.enabled)
        {
            switch (scriptItem->conditions[i].index)
            {
            case 0:
                index = GetValueForBitfield(scriptItem->conditions[i].value[0]);
                Com_Printf(19, "Checking to see if weapon animtype is %i...\n", index);
                break;
            case 1:
                ValueForBitfield = GetValueForBitfield(scriptItem->conditions[i].value[0]);
                WeaponTypeName = GetWeaponTypeName(ValueForBitfield);
                Com_Printf(19, "Checking to see if weapon type is %s...\n", WeaponTypeName);
                break;
            case 2:
                Com_Printf(19, "Checking to see if player is mounted...\n");
                break;
            case 3:
                v4 = GetValueForBitfield(scriptItem->conditions[i].value[0]);
                MoveTypeName = GetMoveTypeName(v4);
                Com_Printf(19, "Checking to see if movetype is %s...\n", MoveTypeName);
                break;
            case 4:
                Com_Printf(19, "Checking to see if player using underhand...\n");
                break;
            case 5:
                Com_Printf(19, "Checking to see if player is crouching...\n");
                break;
            case 6:
                Com_Printf(19, "Checking to see if player is firing...\n");
                break;
            case 7:
                Com_Printf(19, "Checking weapon position...\n");
                break;
            case 8:
                Com_Printf(19, "Checking to see if player is strafing...\n");
                break;
            default:
                break;
            }
        }
        type = animConditionsTable[cond->index].type;
        if (type)
        {
            if (type == ANIM_CONDTYPE_VALUE && ci->clientConditions[cond->index][0] != cond->value[0])
            {
                if (animscript_debug->current.enabled)
                    Com_Printf(19, "failed\n");
                return 0;
            }
        }
        else if ((cond->value[0] & ci->clientConditions[cond->index][0]) == 0
            && (cond->value[1] & ci->clientConditions[cond->index][1]) == 0)
        {
            if (animscript_debug->current.enabled)
                Com_Printf(19, "failed\n");
            return 0;
        }
        ++i;
        ++cond;
    }
    if (animscript_debug->current.enabled)
        Com_Printf(19, "Success\n");
    return 1;
}

const char *__cdecl GetMoveTypeName(int type)
{
    const char *result; // eax

    switch (type)
    {
    case 0:
        result = "ANIM_MT_UNUSED";
        break;
    case 1:
        result = "Idle";
        break;
    case 2:
        result = "Crouching Idle";
        break;
    case 3:
        result = "Prone Idle";
        break;
    case 4:
        result = "Walk";
        break;
    case 5:
        result = "Walk Backward";
        break;
    case 6:
        result = "Crouching Walk";
        break;
    case 7:
        result = "Crouching Walk Backward";
        break;
    case 8:
        result = "Prone Crawl";
        break;
    case 9:
        result = "Prone Crawl Backward";
        break;
    case 10:
        result = "Run";
        break;
    case 11:
        result = "Run Backward";
        break;
    case 12:
        result = "Crouching Run";
        break;
    case 13:
        result = "Crouching Run Backward";
        break;
    case 14:
        result = "Turning Right";
        break;
    case 15:
        result = "Turning Left";
        break;
    case 16:
        result = "Turning Right Crouching";
        break;
    case 17:
        result = "Turning Left Crouching";
        break;
    case 18:
        result = "Climbing Up";
        break;
    case 19:
        result = "Climbing Down";
        break;
    case 20:
        result = "Sprinting";
        break;
    default:
        result = "Unknown";
        break;
    }
    return result;
}

const char *__cdecl GetWeaponTypeName(int type)
{
    const char *result; // eax

    switch (type)
    {
    case 0:
        result = "Rifle";
        break;
    case 1:
        result = "Machine Gun";
        break;
    case 2:
        result = "Submachine Gun";
        break;
    case 4:
        result = "Pistol";
        break;
    case 5:
        result = "Grenade";
        break;
    case 6:
        result = "Rocket Launcher";
        break;
    case 7:
        result = "Turret";
        break;
    case 8:
        result = "Weapon that's not meant for use by the player";
        break;
    case 9:
        result = "Item";
        break;
    default:
        result = "Unknown";
        break;
    }
    return result;
}

const char *__cdecl GetBodyPart(int bodypart)
{
    switch (bodypart)
    {
    case 1:
        return "legs";
    case 2:
        return "torso";
    case 3:
        return "fullbody";
    }
    return "unknown body part";
}

int __cdecl BG_AnimScriptEvent(playerState_s *ps, scriptAnimEventTypes_t event, int isContinue, int force)
{
    int v5; // eax
    animScriptItem_t *scriptItem; // [esp+8h] [ebp-4h]

    if (event != ANIM_ET_DEATH && ps->pm_type >= 7)
        return -1;
    if (G_IsServerGameSystem(ps->clientNum))
        Com_Printf(19, "event: %s\n", animEventTypesStr[event].string);
    if (!globalScriptData->scriptEvents[event].numItems)
        return -1;
    scriptItem = BG_FirstValidItem(ps->clientNum, &globalScriptData->scriptEvents[event]);
    if (!scriptItem)
        return -1;
    if (!scriptItem->numCommands)
        return -1;
    v5 = rand();
    return BG_ExecuteCommand(ps, &scriptItem->commands[v5 % scriptItem->numCommands], 1, isContinue, force);
}

void __cdecl BG_SetConditionValue(unsigned int client, unsigned int condition, unsigned __int64 value)
{
    unsigned int ConditionBit; // eax
    const char *ConditionString; // eax
    unsigned int *conditions; // [esp+18h] [ebp-4h]

    BG_CheckThread();
    if (!bgs)
        MyAssertHandler(".\\bgame\\bg_animation_mp.cpp", 2225, 0, "%s", "bgs");
    if (condition >= 0xA)
        MyAssertHandler(
            ".\\bgame\\bg_animation_mp.cpp",
            2226,
            0,
            "%s\n\t(condition) = %i",
            "(condition < NUM_ANIM_CONDITIONS && condition >= 0)",
            condition);
    if (client >= 0x40)
        MyAssertHandler(
            ".\\bgame\\bg_animation_mp.cpp",
            2230,
            0,
            "client doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            client,
            64);
    conditions = bgs->clientinfo[client].clientConditions[condition];
    *(_QWORD *)conditions = value;
    if ((*conditions != (unsigned int)value || conditions[1] != HIDWORD(value)) && G_IsServerGameSystem(client))
    {
        if (client >= 0x40)
            MyAssertHandler(
                ".\\bgame\\bg_animation_mp.cpp",
                2241,
                0,
                "client doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                client,
                64);
        ConditionBit = BG_GetConditionBit(&bgs->clientinfo[client], condition);
        ConditionString = BG_GetConditionString(condition, ConditionBit);
        Com_Printf(19, "condition: %s: %s\n", animConditionsStr[condition].string, ConditionString);
    }
}

const char *__cdecl BG_GetConditionString(int condition, unsigned int value)
{
    const char *result; // eax
    const char *v3; // eax
    const char *v4; // [esp+0h] [ebp-8h]

    BG_CheckThread();
    switch (condition)
    {
    case 0:
        result = BG_GetPlayerAnimTypeName(value);
        break;
    case 1:
        result = animWeaponClassStr[value].string;
        break;
    case 2:
        result = animConditionMountedStr[value].string;
        break;
    case 3:
        if (value > 0x2A)
            MyAssertHandler(
                ".\\bgame\\bg_animation_mp.cpp",
                2193,
                0,
                "%s\n\t(value) = %i",
                "(value >= 0 && value < NUM_ANIM_MOVETYPES)",
                value);
        result = animMoveTypesStr[value].string;
        break;
    case 4:
    case 5:
    case 6:
        if (value)
            v4 = "true";
        else
            v4 = "false";
        result = v4;
        break;
    case 7:
        result = animWeaponPositionStr[value].string;
        break;
    case 8:
        result = animStrafeStateStr[value].string;
        break;
    default:
        if (!alwaysfails)
        {
            v3 = va("BG_GetConditionString: unhandled case: %d", condition);
            MyAssertHandler(".\\bgame\\bg_animation_mp.cpp", 2208, 0, v3);
        }
        result = 0;
        break;
    }
    return result;
}

void __cdecl BG_SetConditionBit(unsigned int client, int condition, int value)
{
    const char *ConditionString; // eax

    BG_CheckThread();
    if (!bgs)
        MyAssertHandler(".\\bgame\\bg_animation_mp.cpp", 2259, 0, "%s", "bgs");
    if (animConditionsTable[condition].type)
        MyAssertHandler(
            ".\\bgame\\bg_animation_mp.cpp",
            2261,
            0,
            "%s",
            "animConditionsTable[condition].type == ANIM_CONDTYPE_BITFLAGS");
    if (client >= 0x40)
        MyAssertHandler(
            ".\\bgame\\bg_animation_mp.cpp",
            2263,
            0,
            "client doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            client,
            64);
    if (value >= 64)
        MyAssertHandler(".\\bgame\\bg_animation_mp.cpp", 2265, 0, "%s", "value < 64");
    if (client >= 0x40)
        MyAssertHandler(
            ".\\bgame\\bg_animation_mp.cpp",
            2268,
            0,
            "client doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            client,
            64);
    if (!Com_BitCheckAssert(bgs->clientinfo[client].clientConditions[condition], value, 8)
        && G_IsServerGameSystem(client))
    {
        ConditionString = BG_GetConditionString(condition, value);
        Com_Printf(19, "condition: %s: %s\n", animConditionsStr[condition].string, ConditionString);
    }
    if (client >= 0x40)
        MyAssertHandler(
            ".\\bgame\\bg_animation_mp.cpp",
            2279,
            0,
            "client doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
            client,
            64);
    bgs->clientinfo[client].clientConditions[condition][0] = 0;
    bgs->clientinfo[client].clientConditions[condition][1] = 0;
    Com_BitSetAssert(bgs->clientinfo[client].clientConditions[condition], value, 8);
}

void __cdecl Com_BitSetAssert(unsigned int *array, int bitNum, int size)
{
    if (!array)
        MyAssertHandler("c:\\trees\\cod3\\src\\bgame\\../universal/q_shared.h", 711, 0, "%s", "array");
    if (bitNum >= (unsigned int)(8 * size))
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\bgame\\../universal/q_shared.h",
            712,
            0,
            "bitNum doesn't index size * 8\n\t%i not in [0, %i)",
            bitNum,
            8 * size);
    array[bitNum >> 5] |= 1 << (bitNum & 0x1F);
}

unsigned int __cdecl BG_GetConditionBit(const clientInfo_t *ci, unsigned int condition)
{
    unsigned int i; // [esp+0h] [ebp-8h]

    if (condition >= 0xA)
        MyAssertHandler(
            ".\\bgame\\bg_animation_mp.cpp",
            2312,
            0,
            "%s\n\t(condition) = %i",
            "(condition < NUM_ANIM_CONDITIONS && condition >= 0)",
            condition);
    if (animConditionsTable[condition].type)
        MyAssertHandler(
            ".\\bgame\\bg_animation_mp.cpp",
            2313,
            0,
            "%s\n\t(animConditionsTable[condition].type) = %i",
            "(animConditionsTable[condition].type == ANIM_CONDTYPE_BITFLAGS)",
            animConditionsTable[condition].type);
    for (i = 0; i < 0x40; ++i)
    {
        if (Com_BitCheckAssert(ci->clientConditions[condition], i, 8))
            return i;
    }
    return 0;
}

animScriptData_t *__cdecl BG_GetAnimationForIndex(int client, unsigned int index)
{
    if (index >= globalScriptData->numAnimations)
        Com_Error(ERR_DROP, "BG_GetAnimationForIndex: index out of bounds");
    return (animScriptData_t *)((char *)globalScriptData + 104 * index);
}

void __cdecl BG_AnimUpdatePlayerStateConditions(pmove_t *pmove)
{
    unsigned int ViewmodelWeaponIndex; // eax
    WeaponDef *weaponDef; // [esp+8h] [ebp-8h]
    playerState_s *ps; // [esp+Ch] [ebp-4h]

    ps = pmove->ps;
    ViewmodelWeaponIndex = BG_GetViewmodelWeaponIndex(pmove->ps);
    weaponDef = BG_GetWeaponDef(ViewmodelWeaponIndex);
    if (!weaponDef)
        MyAssertHandler(".\\bgame\\bg_animation_mp.cpp", 2356, 0, "%s", "weaponDef");
    BG_SetConditionBit(ps->clientNum, 0, weaponDef->playerAnimType);
    BG_SetConditionBit(ps->clientNum, 1, weaponDef->weapClass);
    if ((ps->eFlags & 0x40000) != 0)
        BG_SetConditionValue(ps->clientNum, 7u, 1u);
    else
        BG_SetConditionValue(ps->clientNum, 7u, 0);
    if ((ps->eFlags & 0x300) != 0)
        BG_SetConditionValue(ps->clientNum, 2u, 1u);
    else
        BG_SetConditionValue(ps->clientNum, 2u, 0);
    BG_SetConditionValue(ps->clientNum, 4u, ps->viewangles[0] > 0.0);
    if ((pmove->cmd.buttons & 1) != 0)
        BG_SetConditionValue(ps->clientNum, 6u, 1u);
    else
        BG_SetConditionValue(ps->clientNum, 6u, 0);
    if (ps->pm_type == 6)
        BG_SetConditionValue(ps->clientNum, 9u, 1u);
    else
        BG_SetConditionValue(ps->clientNum, 9u, 0);
}

bool __cdecl BG_IsCrouchingAnim(const clientInfo_t *ci, int animNum)
{
    __int64 v2; // rax
    animScriptData_t *anim; // [esp+8h] [ebp-4h]

    anim = BG_GetAnimationForIndex(ci->clientNum, animNum & 0xFFFFFDFF);
    HIDWORD(v2) = anim->animations[0].movetype & 0xC4;
    LODWORD(v2) = HIDWORD(anim->animations[0].movetype) & 0x300;
    return v2 != 0;
}

bool __cdecl BG_IsAds(const clientInfo_t *ci, int animNum)
{
    return (BG_GetAnimationForIndex(ci->clientNum, animNum & 0xFFFFFDFF)->animations[0].movetype & 0x3F0) != 0;
}

bool __cdecl BG_IsProneAnim(const clientInfo_t *ci, int animNum)
{
    return (BG_GetAnimationForIndex(ci->clientNum, animNum & 0xFFFFFDFF)->animations[0].movetype & 0x308) != 0;
}

bool __cdecl BG_IsKnifeMeleeAnim(const clientInfo_t *ci, int animNum)
{
    return (BG_GetAnimationForIndex(ci->clientNum, animNum & 0xFFFFFDFF)->animations[0].flags & 0x100) != 0;
}

void __cdecl BG_LerpOffset(float *offset_goal, float maxOffsetChange, float *offset)
{
    float diff[3]; // [esp+20h] [ebp-10h] BYREF
    int error; // [esp+2Ch] [ebp-4h]

    Vec3Sub(offset_goal, offset, diff);
    *(float *)&error = Vec3LengthSq(diff);
    if (*(float *)&error != 0.0)
    {
        *(float *)&error = I_rsqrt(error) * maxOffsetChange;
        if (*(float *)&error >= 1.0)
        {
            *offset = *offset_goal;
            offset[1] = offset_goal[1];
            offset[2] = offset_goal[2];
        }
        else
        {
            Vec3Mad(offset, *(float *)&error, diff, offset);
        }
    }
}

void __cdecl Vec3Mad(const float *start, float scale, const float *dir, float *result)
{
    *result = scale * *dir + *start;
    result[1] = scale * dir[1] + start[1];
    result[2] = scale * dir[2] + start[2];
}

void __cdecl Vec3Accum(const float *subTotal, const float *weight, const float *added, float *total)
{
    *total = *weight * *added + *subTotal;
    total[1] = weight[1] * added[1] + subTotal[1];
    total[2] = weight[2] * added[2] + subTotal[2];
}

void __cdecl BG_Player_DoControllersSetup(const entityState_s *es, clientInfo_t *ci, int frametime)
{
    float maxOffsetChange; // [esp+8h] [ebp-6Ch]
    controller_info_t info; // [esp+Ch] [ebp-68h] BYREF
    float maxAngleChange; // [esp+6Ch] [ebp-8h]
    int i; // [esp+70h] [ebp-4h]

    BG_Player_DoControllersInternal(es, ci, &info);
    maxAngleChange = (double)frametime * 0.3600000143051147;
    for (i = 0; i < 6; ++i)
        BG_LerpAngles(info.angles[i], maxAngleChange, ci->control.angles[i]);
    BG_LerpAngles(info.tag_origin_angles, maxAngleChange, ci->control.tag_origin_angles);
    maxOffsetChange = (double)frametime * 0.1000000014901161;
    BG_LerpOffset(info.tag_origin_offset, maxOffsetChange, ci->control.tag_origin_offset);
}

void __cdecl BG_Player_DoControllersInternal(const entityState_s *es, const clientInfo_t *ci, controller_info_t *info)
{
    double v3; // st7
    double v4; // st7
    double v5; // st7
    double v6; // st7
    double v7; // st7
    double v8; // st7
    double v9; // st7
    float v10; // [esp+8h] [ebp-D0h]
    float v11; // [esp+Ch] [ebp-CCh]
    float *v12; // [esp+18h] [ebp-C0h]
    float *v13; // [esp+1Ch] [ebp-BCh]
    float v14; // [esp+38h] [ebp-A0h]
    float v15; // [esp+44h] [ebp-94h]
    float tag_origin_offset; // [esp+4Ch] [ebp-8Ch]
    float tag_origin_offset_4; // [esp+50h] [ebp-88h]
    float c; // [esp+58h] [ebp-80h]
    float vHeadAngles[3]; // [esp+5Ch] [ebp-7Ch] BYREF
    float vTorsoAngles[3]; // [esp+68h] [ebp-70h] BYREF
    float tag_origin_angles[3]; // [esp+74h] [ebp-64h] BYREF
    float angles[6][3]; // [esp+80h] [ebp-58h] BYREF
    float s; // [esp+C8h] [ebp-10h]
    float fLeanFrac; // [esp+CCh] [ebp-Ch]
    int i; // [esp+D0h] [ebp-8h]
    int clientNum; // [esp+D4h] [ebp-4h]

    if ((es->lerp.eFlags & 0x300) != 0)
    {
        memset((unsigned __int8 *)info, 0, sizeof(controller_info_t));
    }
    else
    {
        clientNum = es->clientNum;
        tag_origin_angles[0] = 0.0f;
        tag_origin_angles[1] = 0.0f;
        tag_origin_angles[2] = 0.0f;
        vTorsoAngles[0] = 0.0f;
        vTorsoAngles[1] = 0.0f;
        vTorsoAngles[2] = 0.0f;
        vHeadAngles[0] = ci->playerAngles[0];
        vHeadAngles[1] = ci->playerAngles[1];
        vHeadAngles[2] = ci->playerAngles[2];
        tag_origin_angles[1] = ci->legs.yawAngle;
        vTorsoAngles[1] = ci->torso.yawAngle;
        if ((BG_GetConditionBit(ci, 3u) & 0x80000000) != 0)
            MyAssertHandler(
                ".\\bgame\\bg_animation_mp.cpp",
                3250,
                0,
                "%s",
                "BG_GetConditionBit( ci, ANIM_COND_MOVETYPE ) >= ANIM_MT_UNUSED");
        if ((int)BG_GetConditionBit(ci, 3u) >= 43)
            MyAssertHandler(
                ".\\bgame\\bg_animation_mp.cpp",
                3251,
                0,
                "%s",
                "BG_GetConditionBit( ci, ANIM_COND_MOVETYPE ) < NUM_ANIM_MOVETYPES");
        if ((BG_GetConditionValue(ci, 3u) & 0xC0000) == 0)
        {
            vTorsoAngles[0] = ci->torso.pitchAngle;
            if ((es->lerp.eFlags & 8) != 0)
            {
                v15 = vTorsoAngles[0] * 0.002777777845039964f;
                v11 = v15 + 0.5f;
                v10 = floor(v11);
                vTorsoAngles[0] = (v15 - v10) * 360.0f;
                if (vTorsoAngles[0] <= 0.0f)
                    v3 = vTorsoAngles[0] * 0.25f;
                else
                    v3 = vTorsoAngles[0] * 0.5f;
                vTorsoAngles[0] = v3;
            }
        }
        AnglesSubtract(vHeadAngles, vTorsoAngles, vHeadAngles);
        AnglesSubtract(vTorsoAngles, tag_origin_angles, vTorsoAngles);
        tag_origin_offset = 0.0f;
        tag_origin_offset_4 = 0.0f;
        fLeanFrac = GetLeanFraction(ci->lerpLean);
        vTorsoAngles[2] = fLeanFrac * 50.0f * 0.925000011920929f;
        vHeadAngles[2] = vTorsoAngles[2];
        if (fLeanFrac != 0.0f)
        {
            if ((es->lerp.eFlags & 4) != 0)
            {
                if (fLeanFrac <= 0.0)
                    v4 = -fLeanFrac * player_lean_shift_crouch_left->current.value + tag_origin_offset_4;
                else
                    v4 = -fLeanFrac * player_lean_shift_crouch_right->current.value + tag_origin_offset_4;
                tag_origin_offset_4 = v4;
            }
            else
            {
                if (fLeanFrac <= 0.0)
                    v5 = -fLeanFrac * player_lean_shift_left->current.value + tag_origin_offset_4;
                else
                    v5 = -fLeanFrac * player_lean_shift_right->current.value + tag_origin_offset_4;
                tag_origin_offset_4 = v5;
            }
        }
        if ((es->lerp.eFlags & 0x20000) == 0)
            tag_origin_angles[1] = AngleDelta(tag_origin_angles[1], ci->playerAngles[1]);
        if ((es->lerp.eFlags & 8) != 0)
        {
            if (fLeanFrac != 0.0)
                vHeadAngles[2] = vHeadAngles[2] * 0.5;
            tag_origin_angles[0] = tag_origin_angles[0] + es->fTorsoPitch;
            v14 = vTorsoAngles[1] * 0.01745329238474369f;
            c = cos(v14);
            s = sin(v14);
            tag_origin_offset = (1.0f - c) * -24.0f + tag_origin_offset;
            tag_origin_offset_4 = s * -12.0f + tag_origin_offset_4;
            if (fLeanFrac * s > 0.0f)
                tag_origin_offset_4 = -fLeanFrac * (1.0f - c) * 16.0f + tag_origin_offset_4;
            angles[0][0] = 0.0f;
            angles[0][1] = vTorsoAngles[2] * -1.200000047683716f;
            angles[0][2] = vTorsoAngles[2] * 0.300000011920929f;
            if (es->fTorsoPitch != 0.0 || es->fWaistPitch != 0.0)
            {
                v6 = AngleDelta(es->fTorsoPitch, es->fWaistPitch);
                angles[0][0] = v6 + angles[0][0];
            }
            angles[1][0] = 0.0f;
            angles[1][1] = vTorsoAngles[1] * 0.1000000014901161f - vTorsoAngles[2] * 0.2000000029802322f;
            angles[1][2] = vTorsoAngles[2] * 0.2000000029802322f;
            angles[2][0] = vTorsoAngles[0];
            angles[2][1] = vTorsoAngles[1] * 0.800000011920929f - vTorsoAngles[2] * -1.0;
            angles[2][2] = vTorsoAngles[2] * -0.2000000029802322f;
        }
        else
        {
            if (fLeanFrac != 0.0)
            {
                if ((es->lerp.eFlags & 4) != 0)
                {
                    if (fLeanFrac <= 0.0)
                    {
                        vTorsoAngles[2] = vTorsoAngles[2] * player_lean_rotate_crouch_left->current.value;
                        v7 = vHeadAngles[2] * player_lean_rotate_crouch_left->current.value;
                    }
                    else
                    {
                        vTorsoAngles[2] = vTorsoAngles[2] * player_lean_rotate_crouch_right->current.value;
                        v7 = vHeadAngles[2] * player_lean_rotate_crouch_right->current.value;
                    }
                    vHeadAngles[2] = v7;
                }
                else
                {
                    if (fLeanFrac <= 0.0)
                    {
                        vTorsoAngles[2] = vTorsoAngles[2] * player_lean_rotate_left->current.value;
                        v8 = vHeadAngles[2] * player_lean_rotate_left->current.value;
                    }
                    else
                    {
                        vTorsoAngles[2] = vTorsoAngles[2] * player_lean_rotate_right->current.value;
                        v8 = vHeadAngles[2] * player_lean_rotate_right->current.value;
                    }
                    vHeadAngles[2] = v8;
                }
            }
            tag_origin_angles[2] = fLeanFrac * 50.0f * 0.07500000298023224f + tag_origin_angles[2];
            angles[0][0] = vTorsoAngles[0] * 0.2000000029802322f;
            angles[0][1] = vTorsoAngles[1] * 0.4000000059604645f;
            angles[0][2] = vTorsoAngles[2] * 0.5f;
            if (es->fTorsoPitch != 0.0 || es->fWaistPitch != 0.0)
            {
                v9 = AngleDelta(es->fTorsoPitch, es->fWaistPitch);
                angles[0][0] = v9 + angles[0][0];
            }
            angles[1][0] = vTorsoAngles[0] * 0.300000011920929f;
            angles[1][1] = vTorsoAngles[1] * 0.4000000059604645f;
            angles[1][2] = vTorsoAngles[2] * 0.5f;
            angles[2][0] = vTorsoAngles[0] * 0.5f;
            angles[2][1] = vTorsoAngles[1] * 0.2000000029802322f;
            angles[2][2] = vTorsoAngles[2] * -0.6000000238418579f;
        }
        angles[3][0] = vHeadAngles[0] * 0.300000011920929f;
        angles[3][1] = vHeadAngles[1] * 0.300000011920929f;
        angles[3][2] = 0.0f;
        angles[4][0] = vHeadAngles[0] * 0.699999988079071f;
        angles[4][1] = vHeadAngles[1] * 0.699999988079071f;
        angles[4][2] = vHeadAngles[2] * -0.300000011920929f;
        angles[5][0] = 0.0f;
        angles[5][1] = 0.0f;
        angles[5][2] = 0.0f;
        if (es->fWaistPitch != 0.0 || es->fTorsoPitch != 0.0)
            angles[5][0] = AngleDelta(es->fWaistPitch, es->fTorsoPitch);
        for (i = 0; i < 6; ++i)
        {
            v12 = info->angles[i];
            v13 = angles[i];
            *v12 = *v13;
            v12[1] = v13[1];
            v12[2] = v13[2];
        }
        info->tag_origin_angles[0] = tag_origin_angles[0];
        info->tag_origin_angles[1] = tag_origin_angles[1];
        info->tag_origin_angles[2] = tag_origin_angles[2];
        info->tag_origin_offset[0] = tag_origin_offset;
        info->tag_origin_offset[1] = tag_origin_offset_4;
        info->tag_origin_offset[2] = 0.0f;
    }
}

unsigned int __cdecl BG_GetConditionValue(const clientInfo_t *ci, unsigned int condition)
{
    if (condition >= 0xA)
        MyAssertHandler(
            ".\\bgame\\bg_animation_mp.cpp",
            2296,
            0,
            "%s\n\t(condition) = %i",
            "(condition < NUM_ANIM_CONDITIONS && condition >= 0)",
            condition);
    return ci->clientConditions[condition][0];
}

void __cdecl BG_LerpAngles(float *angles_goal, float maxAngleChange, float *angles)
{
    float diff; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    for (i = 0; i < 3; ++i)
    {
        diff = angles_goal[i] - angles[i];
        if (maxAngleChange >= (double)diff)
        {
            if (diff >= -maxAngleChange)
                angles[i] = angles_goal[i];
            else
                angles[i] = angles[i] - maxAngleChange;
        }
        else
        {
            angles[i] = angles[i] + maxAngleChange;
        }
    }
}

void __cdecl BG_PlayerAnimation(int localClientNum, const entityState_s *es, clientInfo_t *ci)
{
    XAnimTree_s *pAnimTree; // [esp+0h] [ebp-4h]

    BG_PlayerAngles(es, ci);
    BG_AnimPlayerConditions(es, ci);
    pAnimTree = ci->pXAnimTree;
    BG_PlayerAnimation_VerifyAnim(pAnimTree, &ci->legs);
    BG_PlayerAnimation_VerifyAnim(pAnimTree, &ci->torso);
    if (ci->leftHandGun && (ci->torso.animationNumber & 0xFFFFFDFF) == 0)
    {
        ci->leftHandGun = 0;
        ci->dobjDirty = 1;
    }
    BG_RunLerpFrameRate(localClientNum, ci, &ci->legs, es->legsAnim, es);
    BG_RunLerpFrameRate(localClientNum, ci, &ci->torso, es->torsoAnim, es);
}

void __cdecl BG_RunLerpFrameRate(
    int localClientNum,
    clientInfo_t *ci,
    lerpFrame_t *lf,
    int newAnimation,
    const entityState_s *es)
{
    float v5; // [esp+8h] [ebp-4Ch]
    bool v6; // [esp+Ch] [ebp-48h]
    float diff[3]; // [esp+24h] [ebp-30h] BYREF
    float v8; // [esp+30h] [ebp-24h]
    float fScaleMax; // [esp+34h] [ebp-20h]
    int bNewAnim; // [esp+38h] [ebp-1Ch]
    float moveSpeed; // [esp+3Ch] [ebp-18h]
    animation_s *anim; // [esp+40h] [ebp-14h]
    animation_s *oldAnim; // [esp+44h] [ebp-10h]
    int isLadderAnim; // [esp+48h] [ebp-Ch]
    XAnimTree_s *pAnimTree; // [esp+4Ch] [ebp-8h]
    XAnim_s *pXAnims; // [esp+50h] [ebp-4h]

    bNewAnim = 0;
    BG_CheckThread();
    if (!bgs)
        MyAssertHandler(".\\bgame\\bg_animation_mp.cpp", 2701, 0, "%s", "bgs");
    v6 = lf->animation && (lf->animation->flags & 2) != 0;
    isLadderAnim = v6;
    oldAnim = lf->animation;
    pAnimTree = ci->pXAnimTree;
    pXAnims = bgs->animScriptData.animTree.anims;
    if (newAnimation != lf->animationNumber || !lf->animation && (newAnimation & 0xFFFFFDFF) != 0)
    {
        BG_SetNewAnimation(localClientNum, ci, lf, newAnimation, es);
        bNewAnim = 1;
    }
    if ((newAnimation & 0xFFFFFDFF) != 0)
    {
        anim = lf->animation;
        if (anim && anim->moveSpeed != 0.0f && lf->oldFrameSnapshotTime)
        {
            if (bgs->latestSnapshotTime != lf->oldFrameSnapshotTime)
            {
                if (isLadderAnim)
                {
                    v8 = lf->oldFramePos[2] - es->lerp.pos.trBase[2];
                    v5 = fabs(v8);
                    moveSpeed = v5;
                }
                else
                {
                    Vec3Sub(es->lerp.pos.trBase, lf->oldFramePos, diff);
                    moveSpeed = Vec3Length(diff);
                }
                moveSpeed = moveSpeed / ((double)(bgs->latestSnapshotTime - lf->oldFrameSnapshotTime) * EQUAL_EPSILON);
                if (anim->moveSpeed == 0.0)
                    MyAssertHandler(".\\bgame\\bg_animation_mp.cpp", 2735, 0, "%s", "anim->moveSpeed");
                lf->animSpeedScale = moveSpeed / anim->moveSpeed;
                lf->oldFrameSnapshotTime = bgs->latestSnapshotTime;
                lf->oldFramePos[0] = es->lerp.pos.trBase[0];
                lf->oldFramePos[1] = es->lerp.pos.trBase[1];
                lf->oldFramePos[2] = es->lerp.pos.trBase[2];
                if (lf->animSpeedScale >= 0.1000000014901161f)
                {
                    if (lf->animSpeedScale > 2.0f)
                    {
                        if ((anim->flags & 2) != 0)
                        {
                            if (lf->animSpeedScale > 4.0f)
                                lf->animSpeedScale = 4.0f;
                        }
                        else if (anim->moveSpeed <= 150.0f)
                        {
                            if (anim->moveSpeed >= 20.0f)
                            {
                                fScaleMax = 3.0f - (anim->moveSpeed - 20.0f) * 1.0f / 130.0f;
                                if (fScaleMax < (double)lf->animSpeedScale)
                                    lf->animSpeedScale = fScaleMax;
                            }
                            else if (lf->animSpeedScale > 3.0f)
                            {
                                lf->animSpeedScale = 3.0f;
                            }
                        }
                        else
                        {
                            lf->animSpeedScale = 2.0f;
                        }
                    }
                }
                else if (lf->animSpeedScale < 0.009999999776482582f && isLadderAnim)
                {
                    lf->animSpeedScale = 0.0f;
                }
                else
                {
                    lf->animSpeedScale = 0.1f;
                }
            }
        }
        else
        {
            lf->animSpeedScale = 1.0f;
            lf->oldFrameSnapshotTime = bgs->latestSnapshotTime;
            lf->oldFramePos[0] = es->lerp.pos.trBase[0];
            lf->oldFramePos[1] = es->lerp.pos.trBase[1];
            lf->oldFramePos[2] = es->lerp.pos.trBase[2];
        }
        if (lf->animationNumber)
            XAnimSetAnimRate(pAnimTree, lf->animationNumber & 0xFFFFFDFF, lf->animSpeedScale);
    }
}

void __cdecl BG_SetNewAnimation(
    int localClientNum,
    clientInfo_t *ci,
    lerpFrame_t *lf,
    int newAnimation,
    const entityState_s *es)
{
    bool IsCrouchingAnim; // esi
    bool IsProneAnim; // esi
    float v7; // [esp+1Ch] [ebp-70h]
    float v8; // [esp+20h] [ebp-6Ch]
    float v9; // [esp+24h] [ebp-68h]
    float v10; // [esp+28h] [ebp-64h]
    float v11; // [esp+2Ch] [ebp-60h]
    bool v12; // [esp+30h] [ebp-5Ch]
    float goalTime; // [esp+34h] [ebp-58h]
    float blendTime; // [esp+38h] [ebp-54h]
    animation_s *oldanim; // [esp+44h] [ebp-48h]
    int firstAnim; // [esp+48h] [ebp-44h]
    int transitionMin; // [esp+4Ch] [ebp-40h]
    DObj_s *obj; // [esp+50h] [ebp-3Ch]
    int oldAnimNum; // [esp+54h] [ebp-38h]
    float fStartTime; // [esp+58h] [ebp-34h]
    float fStartTimea; // [esp+58h] [ebp-34h]
    int cycleLen; // [esp+5Ch] [ebp-30h]
    unsigned int animIndex; // [esp+60h] [ebp-2Ch]
    bool crouchMatch; // [esp+64h] [ebp-28h]
    animation_s *anim; // [esp+78h] [ebp-14h]
    XAnimTree_s *pAnimTree; // [esp+7Ch] [ebp-10h]
    XAnim_s *pXAnims; // [esp+80h] [ebp-Ch]
    bool proneMatch; // [esp+84h] [ebp-8h]
    bool bNew; // [esp+88h] [ebp-4h]
    unsigned int newAnimationa; // [esp+A0h] [ebp+14h]

    transitionMin = -1;
    firstAnim = 0;
    fStartTime = 0.0;
    BG_CheckThread();
    if (!bgs)
        MyAssertHandler(".\\bgame\\bg_animation_mp.cpp", 2519, 0, "%s", "bgs");
    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        bNew = (es->lerp.eFlags & 0x80000) != 0;
        oldanim = lf->animation;
        oldAnimNum = lf->animationNumber;
        if (!oldanim)
            firstAnim = 1;
        lf->animationNumber = newAnimation;
        newAnimationa = newAnimation & 0xFFFFFDFF;
        if (newAnimationa >= bgs->animScriptData.numAnimations)
            Com_Error(ERR_DROP, "Player animation index out of range (%i): %i", bgs->animScriptData.numAnimations, newAnimationa);
        pAnimTree = ci->pXAnimTree;
        pXAnims = bgs->animScriptData.animTree.anims;
        if (newAnimationa)
        {
            anim = &bgs->animScriptData.animations[newAnimationa];
            lf->animation = anim;
            lf->animationTime = anim->initialLerp;
            IsCrouchingAnim = BG_IsCrouchingAnim(ci, newAnimationa);
            crouchMatch = IsCrouchingAnim == BG_IsCrouchingAnim(ci, oldAnimNum);
            IsProneAnim = BG_IsProneAnim(ci, newAnimationa);
            proneMatch = IsProneAnim == BG_IsProneAnim(ci, oldAnimNum);
            if (lf == &ci->legs && (!crouchMatch || !proneMatch))
                ci->stanceTransitionTime = bgs->time + 400;
        }
        else
        {
            anim = 0;
            lf->animation = 0;
            lf->animationTime = 200;
        }
        if (firstAnim && lf == &ci->legs)
        {
            lf->animationTime = 0;
        }
        else
        {
            if (!anim || lf->animationTime <= 0)
            {
                if (!anim || anim->moveSpeed == 0.0f)
                {
                    if (!oldanim || oldanim->moveSpeed == 0.0f)
                        transitionMin = 170;
                    else
                        transitionMin = 250;
                }
                else
                {
                    transitionMin = 120;
                }
            }
            if (ci->stanceTransitionTime - bgs->time > transitionMin)
                transitionMin = ci->stanceTransitionTime - bgs->time;
            if (lf->animationTime < transitionMin)
                lf->animationTime = transitionMin;
        }
        if (anim && anim->moveSpeed != 0.0 && XAnimIsLooped(pXAnims, newAnimationa))
        {
            animIndex = oldAnimNum & 0xFFFFFDFF;
            if (oldanim && oldanim->moveSpeed != 0.0f && XAnimIsLooped(pXAnims, animIndex))
            {
                fStartTime = XAnimGetTime(pAnimTree, animIndex);
            }
            else
            {
                if (XAnimIsPrimitive(pXAnims, animIndex))
                    cycleLen = XAnimGetLengthMsec(pXAnims, animIndex) + 200;
                else
                    cycleLen = 1000;
                fStartTimea = (float)(bgs->time % cycleLen) / (float)cycleLen + (float)ci->clientNum * 0.3600000143051147f;
                fStartTime = fStartTimea - (float)(int)fStartTimea;
            }
        }
        if (oldanim)
        {
            blendTime = (float)lf->animationTime * EQUAL_EPSILON;
            XAnimClearTreeGoalWeights(pAnimTree, oldAnimNum & 0xFFFFFDFF, blendTime);
        }
        if (newAnimationa)
        {
            if (lf != &ci->legs)
            {
                ci->leftHandGun = 0;
                ci->dobjDirty = 1;
            }
            if ((anim->flags & 0x40) != 0)
            {
                if (XAnimIsLooped(pXAnims, newAnimationa))
                    Com_Error(ERR_DROP, "death animation '%s' is looping", anim->name);
                if (bNew)
                {
                    goalTime = (float)lf->animationTime * EQUAL_EPSILON;
                    XAnimSetCompleteGoalWeight(obj, newAnimationa, 1.0f, goalTime, 1.0f, 0, 0, 0);
                }
                else
                {
                    XAnimSetGoalWeightKnobAll(obj, newAnimationa, 0, 1.0f, 0.0f, 1.0f, 0, 0);
                    XAnimSetTime(pAnimTree, newAnimationa, 1.0);
                }
            }
            else
            {
                v12 = anim->moveSpeed != 0.0f && XAnimGetWeight(pAnimTree, newAnimationa) == 0.0f;
                v11 = (double)lf->animationTime * EQUAL_EPSILON;
                XAnimSetCompleteGoalWeight(obj, newAnimationa, 1.0f, v11, 1.0f, 0, anim->noteType, lf != &ci->legs);
                if (v12)
                    XAnimSetTime(pAnimTree, newAnimationa, fStartTime);
            }
            if (lf != &ci->legs)
            {
                v10 = (double)lf->animationTime * EQUAL_EPSILON;
                XAnimSetCompleteGoalWeight(obj, bgs->animScriptData.torsoAnim, 1.0f, v10, 1.0f, 0, anim->noteType, 0);
                v9 = (double)lf->animationTime * EQUAL_EPSILON;
                XAnimSetCompleteGoalWeight(obj, bgs->animScriptData.legsAnim, 0.0099999998f, v9, 1.0f, 0, anim->noteType, 0);
            }
        }
        else if (lf != &ci->legs)
        {
            v8 = (double)lf->animationTime * EQUAL_EPSILON;
            XAnimSetCompleteGoalWeight(obj, bgs->animScriptData.torsoAnim, 0.0f, v8, 1.0f, 0, 0, 0);
            v7 = (double)lf->animationTime * EQUAL_EPSILON;
            XAnimSetCompleteGoalWeight(obj, bgs->animScriptData.legsAnim, 1.0f, v7, 1.0f, 0, 0, 0);
        }
    }
}

void __cdecl BG_PlayerAnimation_VerifyAnim(XAnimTree_s *pAnimTree, lerpFrame_t *lf)
{
    BG_CheckThread();
    if (!bgs)
        MyAssertHandler(".\\bgame\\bg_animation_mp.cpp", 2814, 0, "%s", "bgs");
    if (lf->animationNumber)
    {
        if (XAnimGetWeight(pAnimTree, lf->animationNumber & 0xFFFFFDFF) == 0.0)
        {
            lf->animationNumber = 0;
            lf->animation = 0;
            lf->animationTime = 150;
        }
    }
    else if (lf->animation)
    {
        MyAssertHandler(".\\bgame\\bg_animation_mp.cpp", 2830, 0, "%s", "!lf->animation");
    }
}

void __cdecl BG_PlayerAngles(const entityState_s *es, clientInfo_t *ci)
{
    double v2; // st7
    float vHeadAngles; // [esp+1Ch] [ebp-38h]
    float vHeadAngles_4; // [esp+20h] [ebp-34h]
    float vTorsoAngles_4; // [esp+2Ch] [ebp-28h]
    float clampTolerance; // [esp+34h] [ebp-20h]
    float vLegsAngles_4; // [esp+3Ch] [ebp-18h]
    float dest; // [esp+44h] [ebp-10h]
    float MAX_PITCH_FRACTION; // [esp+48h] [ebp-Ch]
    float moveDir; // [esp+50h] [ebp-4h]

    BG_CheckThread();
    if (!bgs)
        MyAssertHandler(".\\bgame\\bg_animation_mp.cpp", 2954, 0, "%s", "bgs");
    GetLeanFraction(ci->lerpLean);
    moveDir = ci->lerpMoveDir;
    vHeadAngles = ci->playerAngles[0];
    vHeadAngles_4 = AngleNormalize360(ci->playerAngles[1]);
    if ((BG_GetConditionBit(ci, 3u) & 0x80000000) != 0)
        MyAssertHandler(
            ".\\bgame\\bg_animation_mp.cpp",
            2971,
            0,
            "%s",
            "BG_GetConditionBit( ci, ANIM_COND_MOVETYPE ) >= ANIM_MT_UNUSED");
    if ((int)BG_GetConditionBit(ci, 3u) >= 43)
        MyAssertHandler(
            ".\\bgame\\bg_animation_mp.cpp",
            2972,
            0,
            "%s",
            "BG_GetConditionBit( ci, ANIM_COND_MOVETYPE ) < NUM_ANIM_MOVETYPES");
    if ((es->lerp.eFlags & 0x300) != 0)
        goto LABEL_8;
    if ((BG_GetConditionValue(ci, 3u) & 0xC0000) != 0)
        goto LABEL_10;
    if ((es->lerp.eFlags & 0x8000) != 0)
    {
    LABEL_8:
        ci->torso.yawing = 1;
        ci->torso.pitching = 1;
        ci->legs.yawing = 1;
        goto LABEL_15;
    }
    if ((BG_GetConditionValue(ci, 3u) & 6) == 0)
    {
    LABEL_10:
        ci->torso.yawing = 1;
        ci->torso.pitching = 1;
        ci->legs.yawing = 1;
    }
    else if (BG_GetConditionValue(ci, 6u))
    {
        ci->torso.yawing = 1;
        ci->torso.pitching = 1;
    }
LABEL_15:
    vLegsAngles_4 = vHeadAngles_4 + moveDir;
    if ((es->lerp.eFlags & 0x20000) != 0)
    {
    LABEL_16:
        vLegsAngles_4 = vHeadAngles_4;
        BG_SwingAngles(vHeadAngles_4, 0.0, 90.0, bg_swingSpeed->current.value, &ci->torso.yawAngle, &ci->torso.yawing);
        goto LABEL_28;
    }
    if ((BG_GetConditionValue(ci, 3u) & 0xC0000) != 0)
    {
        BG_SwingAngles(vLegsAngles_4, 0.0, 0.0, bg_swingSpeed->current.value, &ci->torso.yawAngle, &ci->torso.yawing);
    }
    else
    {
        if ((es->lerp.eFlags & 0x8000) != 0)
            goto LABEL_16;
        if ((es->lerp.eFlags & 8) != 0)
        {
            BG_SwingAngles(vHeadAngles_4, 0.0, 90.0, bg_swingSpeed->current.value, &ci->torso.yawAngle, &ci->torso.yawing);
        }
        else if ((es->lerp.eFlags & 0x40) != 0)
        {
            BG_SwingAngles(vHeadAngles_4, 0.0, 45.0, bg_swingSpeed->current.value, &ci->torso.yawAngle, &ci->torso.yawing);
        }
        else
        {
            if ((es->lerp.eFlags & 0x40000) != 0)
                vTorsoAngles_4 = vHeadAngles_4;
            else
                vTorsoAngles_4 = player_move_factor_on_torso->current.value * moveDir + vHeadAngles_4;
            BG_SwingAngles(vTorsoAngles_4, 0.0, 90.0, bg_swingSpeed->current.value, &ci->torso.yawAngle, &ci->torso.yawing);
        }
    }
LABEL_28:
    clampTolerance = 150.0;
    if (!bgs)
        MyAssertHandler(".\\bgame\\bg_animation_mp.cpp", 3060, 0, "%s", "bgs->animScriptData.animations");
    if ((es->lerp.eFlags & 0x20000) != 0)
        goto LABEL_31;
    if ((es->lerp.eFlags & 8) != 0)
    {
        ci->legs.yawing = 0;
        ci->legs.yawAngle = vHeadAngles_4;
    }
    else if ((bgs->animScriptData.animations[es->legsAnim & 0xFFFFFDFF].flags & 0x30) != 0)
    {
        ci->legs.yawing = 0;
        BG_SwingAngles(
            vHeadAngles_4,
            0.0,
            clampTolerance,
            bg_swingSpeed->current.value,
            &ci->legs.yawAngle,
            &ci->legs.yawing);
    }
    else
    {
        if (ci->legs.yawing)
        {
        LABEL_31:
            BG_SwingAngles(
                vLegsAngles_4,
                0.0,
                clampTolerance,
                bg_swingSpeed->current.value,
                &ci->legs.yawAngle,
                &ci->legs.yawing);
            goto LABEL_38;
        }
        BG_SwingAngles(
            vLegsAngles_4,
            bg_legYawTolerance->current.value,
            clampTolerance,
            bg_swingSpeed->current.value,
            &ci->legs.yawAngle,
            &ci->legs.yawing);
    }
LABEL_38:
    if ((es->lerp.eFlags & 0x300) != 0)
    {
        ci->torso.yawAngle = vHeadAngles_4;
        ci->legs.yawAngle = vHeadAngles_4;
    }
    else if ((BG_GetConditionValue(ci, 3u) & 0xC0000) != 0)
    {
        ci->torso.yawAngle = vHeadAngles_4 + moveDir;
        ci->legs.yawAngle = vHeadAngles_4 + moveDir;
    }
    MAX_PITCH_FRACTION = 2.0f;
    if ((es->lerp.eFlags & 0x20000) != 0
        || (es->lerp.eFlags & 0x300) != 0
        || (BG_GetConditionValue(ci, 3u) & 0xC0000) != 0
        || es->lerp.eFlags == 0x8000)
    {
        BG_SwingAngles(0.0f, 0.0f, 45.0f, 0.15000001f, &ci->torso.pitchAngle, &ci->torso.pitching);
    }
    else
    {
        if (vHeadAngles <= 180.0f)
            v2 = vHeadAngles * MAX_PITCH_FRACTION;
        else
            v2 = (vHeadAngles + -360.0f) * MAX_PITCH_FRACTION;
        dest = v2;
        BG_SwingAngles(dest, 0.0f, 45.0f, 0.15000001f, &ci->torso.pitchAngle, &ci->torso.pitching);
    }
}

void __cdecl BG_SwingAngles(
    float destination,
    float swingTolerance,
    float clampTolerance,
    float speed,
    float *angle,
    int *swinging)
{
    float v6; // [esp+8h] [ebp-2Ch]
    float v7; // [esp+Ch] [ebp-28h]
    float v8; // [esp+10h] [ebp-24h]
    float v9; // [esp+14h] [ebp-20h]
    float v10; // [esp+18h] [ebp-1Ch]
    float move; // [esp+28h] [ebp-Ch]
    float movea; // [esp+28h] [ebp-Ch]
    float swing; // [esp+2Ch] [ebp-8h]
    float swinga; // [esp+2Ch] [ebp-8h]
    float swingb; // [esp+2Ch] [ebp-8h]
    float scale; // [esp+30h] [ebp-4h]

    BG_CheckThread();
    if (!bgs)
        MyAssertHandler(".\\bgame\\bg_animation_mp.cpp", 2857, 0, "%s", "bgs");
    if (!*swinging)
    {
        swing = AngleDelta(*angle, destination);
        if (swingTolerance < (double)swing || swing < -swingTolerance)
            *swinging = 1;
    }
    if (*swinging)
    {
        swinga = AngleDelta(destination, *angle);
        v10 = fabs(swinga);
        scale = v10 * 0.05000000074505806;
        if (scale < 0.5)
            scale = 0.5;
        if (swinga < 0.0)
        {
            movea = (double)bgs->frametime * scale * -speed;
            if (swinga < (double)movea)
            {
                *swinging = 1;
            }
            else
            {
                movea = swinga;
                *swinging = 0;
            }
            v8 = *angle + movea;
            *angle = AngleNormalize360(v8);
        }
        else
        {
            move = (double)bgs->frametime * scale * speed;
            if (swinga > (double)move)
            {
                *swinging = 1;
            }
            else
            {
                move = swinga;
                *swinging = 0;
            }
            v9 = *angle + move;
            *angle = AngleNormalize360(v9);
        }
        swingb = AngleDelta(destination, *angle);
        if (clampTolerance >= (double)swingb)
        {
            if (swingb < -clampTolerance)
            {
                v6 = destination + clampTolerance;
                *angle = AngleNormalize360(v6);
            }
        }
        else
        {
            v7 = destination - clampTolerance;
            *angle = AngleNormalize360(v7);
        }
    }
}

void __cdecl BG_AnimPlayerConditions(const entityState_s *es, clientInfo_t *ci)
{
    WeaponDef *weaponDef; // [esp+24h] [ebp-8h]
    unsigned int legsAnim; // [esp+28h] [ebp-4h]

    BG_CheckThread();
    if (!bgs)
        MyAssertHandler(".\\bgame\\bg_animation_mp.cpp", 3143, 0, "%s", "bgs");
    weaponDef = BG_GetWeaponDef(es->weapon);
    if (!weaponDef)
        MyAssertHandler(".\\bgame\\bg_animation_mp.cpp", 3146, 0, "%s", "weaponDef");
    if ((BG_GetConditionBit(ci, 3u) & 0x80000000) != 0)
        MyAssertHandler(
            ".\\bgame\\bg_animation_mp.cpp",
            3148,
            0,
            "%s",
            "BG_GetConditionBit( ci, ANIM_COND_MOVETYPE ) >= ANIM_MT_UNUSED");
    if ((int)BG_GetConditionBit(ci, 3u) >= 43)
        MyAssertHandler(
            ".\\bgame\\bg_animation_mp.cpp",
            3149,
            0,
            "%s",
            "BG_GetConditionBit( ci, ANIM_COND_MOVETYPE ) < NUM_ANIM_MOVETYPES");
    BG_SetConditionBit(es->clientNum, 0, weaponDef->playerAnimType);
    BG_SetConditionBit(es->clientNum, 1, weaponDef->weapClass);
    if ((es->lerp.eFlags & 0x40000) != 0)
        BG_SetConditionValue(es->clientNum, 7u, 1u);
    else
        BG_SetConditionValue(es->clientNum, 7u, 0);
    if ((es->lerp.eFlags & 0x300) != 0)
        BG_SetConditionValue(es->clientNum, 2u, 1u);
    else
        BG_SetConditionValue(es->clientNum, 2u, 0);
    BG_SetConditionValue(es->clientNum, 4u, ci->playerAngles[0] > 0.0);
    if ((es->lerp.eFlags & 4) != 0)
        BG_SetConditionValue(es->clientNum, 5u, 1u);
    else
        BG_SetConditionValue(es->clientNum, 5u, 0);
    if ((es->lerp.eFlags & 0x40) != 0)
        BG_SetConditionValue(es->clientNum, 6u, 1u);
    else
        BG_SetConditionValue(es->clientNum, 6u, 0);
    legsAnim = es->legsAnim & 0xFFFFFDFF;
    if (bgs->animScriptData.animations[legsAnim].movetype
        && BG_GetConditionValue(ci, 3u) != bgs->animScriptData.animations[legsAnim].movetype)
    {
        BG_SetConditionValue(es->clientNum, 3u, bgs->animScriptData.animations[legsAnim].movetype);
        if ((BG_GetConditionBit(ci, 3u) & 0x80000000) != 0)
            MyAssertHandler(
                ".\\bgame\\bg_animation_mp.cpp",
                3188,
                0,
                "%s",
                "BG_GetConditionBit( ci, ANIM_COND_MOVETYPE ) >= ANIM_MT_UNUSED");
        if ((int)BG_GetConditionBit(ci, 3u) >= 43)
            MyAssertHandler(
                ".\\bgame\\bg_animation_mp.cpp",
                3189,
                0,
                "%s",
                "BG_GetConditionBit( ci, ANIM_COND_MOVETYPE ) < NUM_ANIM_MOVETYPES");
    }
    if ((bgs->animScriptData.animations[legsAnim].flags & 0x10) != 0)
    {
        BG_SetConditionValue(es->clientNum, 8u, 1u);
    }
    else if ((bgs->animScriptData.animations[legsAnim].flags & 0x20) != 0)
    {
        BG_SetConditionValue(es->clientNum, 8u, 2u);
    }
    else
    {
        BG_SetConditionValue(es->clientNum, 8u, 0);
    }
}

void __cdecl BG_UpdatePlayerDObj(
    int localClientNum,
    DObj_s *pDObj,
    entityState_s *es,
    clientInfo_t *ci,
    int attachIgnoreCollision)
{
    int iNumModels = 0; // [esp+0h] [ebp-114h]
    int iClientWeapon; // [esp+4h] [ebp-110h]
    int i; // [esp+8h] [ebp-10Ch]
    XAnimTree_s *pAnimTree; // [esp+Ch] [ebp-108h]
    DObjModel_s dobjModels[32]; // [esp+14h] [ebp-100h] BYREF

    BG_CheckThread();
    iassert(bgs);
    iClientWeapon = es->weapon;
    if ((es->lerp.eFlags & 0x300) != 0)
        iClientWeapon = 0;
    pAnimTree = ci->pXAnimTree;
    iassert(pAnimTree);
    if (!ci->infoValid || !ci->model[0])
    {
        XAnimClearTree(pAnimTree);
        bgs->SafeDObjFree(es->number, localClientNum);
        return;
    }
    if (pDObj)
    {
        if (ci->iDObjWeapon == iClientWeapon && ci->weaponModel == es->weaponModel && !ci->dobjDirty)
            return;
        bgs->SafeDObjFree(es->number, localClientNum);
    }
    dobjModels[0].model = bgs->GetXModel(ci->model);
    iassert(dobjModels[iNumModels].model);
    dobjModels[0].boneName = 0;
    dobjModels[0].ignoreCollision = 0;
    iNumModels = 1;
    ci->iDObjWeapon = iClientWeapon;
    ci->weaponModel = es->weaponModel;
    if (bgs->AttachWeapon)
        iNumModels = bgs->AttachWeapon(dobjModels, 1u, ci);
    for (i = 0; i < 6 && ci->attachModelNames[i][0]; ++i)
    {
        iassert(iNumModels < DOBJ_MAX_SUBMODELS);
        dobjModels[iNumModels].model = bgs->GetXModel(ci->attachModelNames[i]);
        iassert(dobjModels[iNumModels].model);
        dobjModels[iNumModels].boneName = SL_FindString(ci->attachTagNames[i]);
        dobjModels[iNumModels].ignoreCollision = (attachIgnoreCollision & (1 << i)) != 0;
        iNumModels++;
    }
    bgs->CreateDObj(dobjModels, iNumModels, pAnimTree, es->number, localClientNum, ci);
    ci->dobjDirty = 0;
}

void __cdecl BG_LoadAnim()
{
    static_assert((sizeof(loadAnim_t) * 512) == 36864);

    LargeLocal playerAnims_large_local(sizeof(loadAnim_t) * 512); // [esp+0h] [ebp-10h] BYREF
    unsigned int iNumPlayerAnims; // [esp+8h] [ebp-8h] BYREF
    loadAnim_t *playerAnims; // [esp+Ch] [ebp-4h]

    //LargeLocal::LargeLocal(&playerAnims_large_local, 36864);
    //playerAnims = (loadAnim_t(*)[512])LargeLocal::GetBuf(&playerAnims_large_local);
    playerAnims = (loadAnim_t*)playerAnims_large_local.GetBuf();

    BG_CheckThread();

    iassert(bgs);

    iNumPlayerAnims = 0;
    BG_FindAnims();
    BG_AnimParseAnimScript(&bgs->animScriptData, playerAnims, &iNumPlayerAnims);
    Scr_PrecacheAnimTrees(bgs->AllocXAnim, bgs->anim_user);
    BG_FindAnimTrees();
    Scr_EndLoadAnimTrees();
    BG_FinalizePlayerAnims();
    //LargeLocal::~LargeLocal(&playerAnims_large_local);
}

void BG_FinalizePlayerAnims()
{
    char *v0; // eax
    char *AnimName; // eax
    int v2; // eax
    int v3; // eax
    int v4; // eax
    int v5; // eax
    double v6; // st7
    double v7; // st7
    double moveSpeed; // [esp+10h] [ebp-60h]
    double v9; // [esp+18h] [ebp-58h]
    float duration; // [esp+38h] [ebp-38h]
    float fullspeed; // [esp+3Ch] [ebp-34h]
    float fullspeeda; // [esp+3Ch] [ebp-34h]
    loadAnim_t *pLoadAnim; // [esp+40h] [ebp-30h]
    animation_s *pCurrAnima; // [esp+44h] [ebp-2Ch]
    animation_s *pCurrAnim; // [esp+44h] [ebp-2Ch]
    char *moveType; // [esp+48h] [ebp-28h]
    float vRot[2]; // [esp+4Ch] [ebp-24h] BYREF
    int i; // [esp+54h] [ebp-1Ch]
    XAnim_s *pXAnims; // [esp+58h] [ebp-18h]
    int iNumAnims; // [esp+5Ch] [ebp-14h]
    animation_s *pAnims; // [esp+60h] [ebp-10h]
    float vDelta[3]; // [esp+64h] [ebp-Ch] BYREF

    iassert(g_pLoadAnims);
    iassert(g_piNumLoadAnims);

    pAnims = (animation_s *)globalScriptData;
    pXAnims = globalScriptData->animTree.anims;
    iNumAnims = XAnimGetAnimTreeSize(pXAnims);
    globalScriptData->numAnimations = iNumAnims;
    pCurrAnima = pAnims;
    pAnims->flags |= 0x201u;
    I_strncpyz(pCurrAnima->name, "root", 64);
    pCurrAnima->nameHash = 0;
    pCurrAnim = pCurrAnima + 1;
    i = 1;
    while (i < iNumAnims)
    {
        pLoadAnim = BG_LoadAnimForAnimIndex(i);
        if (pLoadAnim)
        {
            if (XAnimIsPrimitive(pXAnims, i))
            {
                AnimName = (char *)XAnimGetAnimName(pXAnims, i);
                I_strncpyz(pCurrAnim->name, AnimName, 64);
                pCurrAnim->nameHash = BG_StringHashValue(pCurrAnim->name);
                if (!pCurrAnim->initialLerp)
                    pCurrAnim->initialLerp = -1;
                duration = XAnimGetLength(pXAnims, i);
                if (duration == 0.0)
                {
                    pCurrAnim->duration = 500;
                    pCurrAnim->moveSpeed = 0.0;
                }
                else
                {
                    
                    pCurrAnim->duration = (int)(duration * 1000.0);
                    XAnimGetRelDelta(pXAnims, i, vRot, vDelta, 0.0, 1.0);
                    pCurrAnim->moveSpeed = Vec3Length(vDelta) / duration;
                    if (anim_debugSpeeds->current.enabled)
                    {
                        if (pCurrAnim->moveSpeed <= 1.0)
                        {
                            v7 = Vec3Length(vDelta);
                            Com_Printf(19, "Anim '%s' moves %f units over %fms\n", pCurrAnim->name, v7, duration);
                        }
                        else
                        {
                            if (g_speed)
                                fullspeed = (float)g_speed->current.integer;
                            else
                                fullspeed = 190.0;
                            if (strstr(pCurrAnim->name, "crouch"))
                            {
                                fullspeeda = fullspeed * 0.6499999761581421;
                                moveType = (char*)"crouch";
                            }
                            else
                            {
                                if (strstr(pCurrAnim->name, "prone"))
                                {
                                    fullspeeda = fullspeed * 0.1500000059604645;
                                    moveType = (char*)"prone";
                                }
                                else
                                {
                                    if (strstr(pCurrAnim->name, "walk"))
                                    {
                                        fullspeeda = fullspeed * 0.4000000059604645;
                                        moveType = (char*)"walk";
                                    }
                                    else
                                    {
                                        if (strstr(pCurrAnim->name, "fast"))
                                        {
                                            fullspeeda = fullspeed * player_sprintSpeedScale->current.value;
                                            moveType = (char*)"sprint";
                                        }
                                        else
                                        {
                                            fullspeeda = fullspeed * 1.0;
                                            moveType = (char*)"run";
                                        }
                                    }
                                }
                            }
                            v9 = fullspeeda * 100.0 / pCurrAnim->moveSpeed;
                            moveSpeed = pCurrAnim->moveSpeed;
                            v6 = Vec3Length(vDelta);
                            Com_Printf(
                                19,
                                "Anim '%s' moves %f units over %fms (%f units/s), will play back at %.1f%% speed when player moves at ful"
                                "l %s speed (%.0f units/sec)\n",
                                pCurrAnim->name,
                                v6,
                                duration,
                                moveSpeed,
                                v9,
                                moveType,
                                fullspeeda);
                        }
                    }
                    if (pCurrAnim->duration < 500)
                        pCurrAnim->duration = 500;
                }
                if (XAnimIsLooped(pXAnims, i))
                    pCurrAnim->flags |= 0x80u;
            }
            else
            {
                pCurrAnim->flags |= 1u;
                if (pLoadAnim)
                {
                    I_strncpyz(pCurrAnim->name, pLoadAnim->szAnimName, 64);
                    pCurrAnim->nameHash = pLoadAnim->iNameHash;
                }
                else
                {
                    v0 = (char *)XAnimGetAnimName(pXAnims, i);
                    I_strncpyz(pCurrAnim->name, v0, 64);
                    pCurrAnim->nameHash = 0;
                }
                if (!pCurrAnim->initialLerp)
                    pCurrAnim->initialLerp = -1;
                pCurrAnim->duration = 0;
                pCurrAnim->moveSpeed = 0.0;
            }
        }
        else
        {
            pCurrAnim->flags |= 0x200u;
            I_strncpyz(pCurrAnim->name, "unused", 64);
            pCurrAnim->nameHash = 0;
        }
        ++i;
        ++pCurrAnim;
    }
    BG_AnimParseAnimScript(globalScriptData, 0, 0);
    BG_SetupAnimNoteTypes(globalScriptData);
}

loadAnim_t *__cdecl BG_LoadAnimForAnimIndex(unsigned int iAnimIndex)
{
    unsigned int i; // [esp+0h] [ebp-8h]
    loadAnim_t *pAnim; // [esp+4h] [ebp-4h]

    if (iAnimIndex >= globalScriptData->numAnimations)
        Com_Error(ERR_DROP, "Player animation index %i out of 0 to %i range", iAnimIndex, globalScriptData->numAnimations);
    i = 0;
    pAnim = g_pLoadAnims;
    while (i < *g_piNumLoadAnims)
    {
        if (iAnimIndex == pAnim->anim.index)
            return pAnim;
        ++i;
        ++pAnim;
    }
    return 0;
}

void __cdecl BG_SetupAnimNoteTypes(animScriptData_t *scriptData)
{
    int cmdIndex; // [esp+4h] [ebp-14h]
    animScript_t *script; // [esp+8h] [ebp-10h]
    animScriptItem_t *scriptItem; // [esp+Ch] [ebp-Ch]
    unsigned int animIndex; // [esp+10h] [ebp-8h]
    int itemIndex; // [esp+14h] [ebp-4h]

    BG_CheckThread();
    if (!bgs)
        MyAssertHandler(".\\bgame\\bg_animation_mp.cpp", 629, 0, "%s", "bgs");
    for (animIndex = 0; animIndex < scriptData->numAnimations; ++animIndex)
        scriptData->animations[animIndex].noteType = 0;
    if (!bgs->anim_user)
    {
        script = &scriptData->scriptEvents[10];
        for (itemIndex = 0; itemIndex < script->numItems; ++itemIndex)
        {
            scriptItem = script->items[itemIndex];
            for (cmdIndex = 0; cmdIndex < scriptItem->numCommands; ++cmdIndex)
            {
                if (scriptItem->commands[cmdIndex].bodyPart[0])
                    scriptData->animations[scriptItem->commands[cmdIndex].animIndex[0]].noteType = 1;
                if (scriptItem->commands[cmdIndex].bodyPart[1])
                    scriptData->animations[scriptItem->commands[cmdIndex].animIndex[1]].noteType = 1;
            }
        }
    }
}

void __cdecl BG_AnimParseAnimScript(animScriptData_t *scriptData, loadAnim_t *pLoadAnims, unsigned int *piNumAnims)
{
    const char *v3; // eax
    int v4; // eax
    int v5; // eax
    int v6; // eax
    int v7; // eax
    int v8; // eax
    int v9; // eax
    int v10; // eax
    const char *v11; // [esp+0h] [ebp-1C8h]
    animScriptParseMode_t newParseMode; // [esp+8Ch] [ebp-13Ch]
    animScriptItem_t *currentScriptItem; // [esp+90h] [ebp-138h]
    int oldState; // [esp+94h] [ebp-134h]
    char *input; // [esp+98h] [ebp-130h]
    const char *text_p; // [esp+9Ch] [ebp-12Ch] BYREF
    animScript_t *currentScript; // [esp+A0h] [ebp-128h]
    int indentLevel; // [esp+A4h] [ebp-124h]
    animScriptItem_t tempScriptItem; // [esp+A8h] [ebp-120h] BYREF
    const char *token; // [esp+1ACh] [ebp-1Ch]
    animScriptParseMode_t parseMode; // [esp+1B0h] [ebp-18h]
    int indexes[3]; // [esp+1B4h] [ebp-14h]
    int i; // [esp+1C0h] [ebp-8h]
    int defineType; // [esp+1C4h] [ebp-4h]

    currentScriptItem = 0;
    input = Com_LoadRawTextFile(globalFilename);
    if (!input)
        Com_Error(ERR_DROP, "Couldn',27h,'t load player animation script %s", globalFilename);
    globalScriptData = scriptData;
    g_pLoadAnims = pLoadAnims;
    g_piNumLoadAnims = piNumAnims;
    parseMode = PARSEMODE_DEFINES;
    BG_InitWeaponStrings();
    memset(defineStr, 0, sizeof(defineStr));
    memset(defineStrings, 0, sizeof(defineStrings));
    numDefines[0] = 0;
    numDefines[1] = 0;
    numDefines[2] = 0;
    numDefines[3] = 0;
    numDefines[4] = 0;
    numDefines[5] = 0;
    numDefines[6] = 0;
    numDefines[7] = 0;
    numDefines[8] = 0;
    numDefines[9] = 0;
    defineStringsOffset = 0;
    for (i = 0; i < 3; ++i)
        indexes[i] = -1;
    indentLevel = 0;
    currentScript = 0;
    text_p = input;
    Com_BeginParseSession("BG_AnimParseAnimScript");
    while (1)
    {
        token = (const char *)Com_Parse(&text_p);
        if (!token || !*token)
            break;
        newParseMode = BG_IndexForString(token, animParseModesStr, 1);
        if (newParseMode < PARSEMODE_DEFINES)
        {
            switch (parseMode)
            {
            case PARSEMODE_DEFINES:
                if (!I_stricmp(token, "set"))
                {
                    token = (const char *)Com_ParseOnLine(&text_p);
                    if (!token || !*token)
                        BG_AnimParseError("BG_AnimParseAnimScript: expected condition type string");
                    defineType = BG_IndexForString(token, animConditionsStr, 0);
                    if (animConditionsTable[defineType].type)
                        BG_AnimParseError("BG_AnimParseAnimScript: can not make a define of type '%s'", token);
                    token = (const char *)Com_ParseOnLine(&text_p);
                    if (!token || !*token)
                        BG_AnimParseError("BG_AnimParseAnimScript: expected condition define string");
                    v3 = BG_CopyStringIntoBuffer(token, defineStrings, 0x2710u, &defineStringsOffset);
                    defineStr[defineType][numDefines[defineType]].string = v3;
                    v4 = BG_StringHashValue(defineStr[defineType][numDefines[defineType]].string);
                    defineStr[defineType][numDefines[defineType]].hash = v4;
                    token = (const char *)Com_ParseOnLine(&text_p);
                    if (!token)
                        BG_AnimParseError("BG_AnimParseAnimScript: expected '=', found end of line");
                    if (I_stricmp(token, "="))
                        BG_AnimParseError("BG_AnimParseAnimScript: expected '=', found '%s'", token);
                    BG_ParseConditionBits(
                        &text_p,
                        animConditionsTable[defineType].values,
                        defineType,
                        defineBits[defineType][numDefines[defineType]]);
                    ++numDefines[defineType];
                }
                break;
            case PARSEMODE_ANIMATION:
            case PARSEMODE_CANNED_ANIMATIONS:
                if (I_stricmp(token, "{"))
                {
                    if (I_stricmp(token, "}"))
                    {
                        if (indentLevel || indexes[0] >= 0)
                        {
                            if (indentLevel == 1 && indexes[1] < 0)
                            {
                                v6 = BG_IndexForString(token, animMoveTypesStr, 0);
                                indexes[indentLevel] = v6;
                                if (parseMode == PARSEMODE_ANIMATION)
                                {
                                    currentScript = &scriptData->scriptAnims[indexes[0]][indexes[1]];
                                    parseMovetype = (scriptAnimMoveTypes_t)indexes[1];
                                }
                                else if (parseMode == PARSEMODE_CANNED_ANIMATIONS)
                                {
                                    currentScript = &scriptData->scriptCannedAnims[indexes[0]][indexes[1]];
                                }
                                goto LABEL_101;
                            }
                            if (indentLevel == 2 && indexes[2] < 0)
                            {
                                text_p -= strlen(token);
                                if (I_strncmp(text_p, token, strlen(token)))
                                    BG_AnimParseError("BG_AnimParseAnimScript: internal error");
                                memset((unsigned __int8 *)&tempScriptItem, 0, sizeof(tempScriptItem));
                                v7 = BG_ParseConditions(&text_p, &tempScriptItem);
                                indexes[indentLevel] = v7;
                                if (currentScript->numItems >= 128)
                                    BG_AnimParseError("BG_AnimParseAnimScript: exceeded maximum items per script (%i)", 128);
                                if (scriptData->numScriptItems >= 2048)
                                    BG_AnimParseError("BG_AnimParseAnimScript: exceeded maximum global items (%i)", 2048);
                                goto LABEL_69;
                            }
                            if (indentLevel != 3)
                                goto LABEL_74;
                            text_p -= strlen(token);
                            if (I_strncmp(text_p, token, strlen(token)))
                                BG_AnimParseError("BG_AnimParseAnimScript: internal error");
                            goto LABEL_73;
                        }
                        if (I_stricmp(token, "state"))
                            BG_AnimParseError("BG_AnimParseAnimScript: expected 'state'");
                        token = (const char *)Com_ParseOnLine(&text_p);
                        if (!token)
                            BG_AnimParseError("BG_AnimParseAnimScript: expected state type");
                        v5 = BG_IndexForString(token, animStateStr, 0);
                        indexes[indentLevel] = v5;
                        token = (const char *)Com_Parse(&text_p);
                        if (!token || I_stricmp(token, "{"))
                            BG_AnimParseError("BG_AnimParseAnimScript: expected '{'");
                        ++indentLevel;
                    }
                    else
                    {
                        if (--indentLevel < 0)
                            BG_AnimParseError("BG_AnimParseAnimScript: unexpected '%s'", token);
                        if (indentLevel == 1)
                            currentScript = 0;
                        indexes[indentLevel] = -1;
                    }
                }
                else
                {
                    if (indentLevel >= 3)
                        BG_AnimParseError("BG_AnimParseAnimScript: unexpected '%s'", token);
                    if (indexes[indentLevel] < 0)
                        BG_AnimParseError("BG_AnimParseAnimScript: unexpected '%s'", token);
                    ++indentLevel;
                }
                break;
            case PARSEMODE_STATECHANGES:
            case PARSEMODE_EVENTS:
                if (I_stricmp(token, "{"))
                {
                    if (I_stricmp(token, "}"))
                    {
                        if (indentLevel || indexes[0] >= 0)
                        {
                            if (indentLevel == 1 && indexes[1] < 0)
                            {
                                text_p -= strlen(token);
                                if (I_strncmp(text_p, token, strlen(token)))
                                    BG_AnimParseError("BG_AnimParseAnimScript: internal error");
                                memset((unsigned __int8 *)&tempScriptItem, 0, sizeof(tempScriptItem));
                                v10 = BG_ParseConditions(&text_p, &tempScriptItem);
                                indexes[indentLevel] = v10;
                                if (currentScript->numItems >= 128)
                                    BG_AnimParseError("BG_AnimParseAnimScript: exceeded maximum items per script (%i)", 128);
                                if (scriptData->numScriptItems >= 2048)
                                    BG_AnimParseError("BG_AnimParseAnimScript: exceeded maximum global items (%i)", 2048);
                            LABEL_69:
                                currentScript->items[currentScript->numItems] = &scriptData->scriptItems[scriptData->numScriptItems++];
                                currentScriptItem = currentScript->items[currentScript->numItems++];
                                memcpy(currentScriptItem, &tempScriptItem, sizeof(animScriptItem_t));
                            }
                            else if (indentLevel == 2)
                            {
                                text_p -= strlen(token);
                                if (I_strncmp(text_p, token, strlen(token)))
                                    BG_AnimParseError("BG_AnimParseAnimScript: internal error");
                            LABEL_73:
                                BG_ParseCommands(&text_p, currentScriptItem, scriptData);
                            }
                            else
                            {
                            LABEL_74:
                                BG_AnimParseError("BG_AnimParseAnimScript: unexpected '%s'", token);
                            }
                        }
                        else
                        {
                            if (parseMode == PARSEMODE_STATECHANGES)
                            {
                                if (I_stricmp(token, "statechange"))
                                    BG_AnimParseError("BG_AnimParseAnimScript: expected 'statechange', got '%s'", token);
                                token = (const char *)Com_ParseOnLine(&text_p);
                                if (!token)
                                    BG_AnimParseError("BG_AnimParseAnimScript: expected <state type>");
                                oldState = BG_IndexForString(token, animStateStr, 0);
                                token = (const char *)Com_ParseOnLine(&text_p);
                                if (!token)
                                    BG_AnimParseError("BG_AnimParseAnimScript: expected <state type>");
                                v8 = BG_IndexForString(token, animStateStr, 0);
                                indexes[indentLevel] = v8;
                                currentScript = &scriptData->scriptStateChange[oldState][indexes[indentLevel]];
                                token = (const char *)Com_Parse(&text_p);
                                if (!token || I_stricmp(token, "{"))
                                    BG_AnimParseError("BG_AnimParseAnimScript: expected '{'");
                                ++indentLevel;
                            }
                            else
                            {
                                v9 = BG_IndexForString(token, animEventTypesStr, 0);
                                indexes[indentLevel] = v9;
                                currentScript = &scriptData->scriptEvents[indexes[0]];
                                parseEvent = indexes[indentLevel];
                            }
                        LABEL_101:
                            memset((unsigned __int8 *)currentScript, 0, sizeof(animScript_t));
                        }
                    }
                    else
                    {
                        if (--indentLevel < 0)
                            BG_AnimParseError("BG_AnimParseAnimScript: unexpected '%s'", token);
                        if (!indentLevel)
                            currentScript = 0;
                        indexes[indentLevel] = -1;
                    }
                }
                else
                {
                    if (indentLevel >= 3)
                        BG_AnimParseError("BG_AnimParseAnimScript: unexpected '%s'", token);
                    if (indexes[indentLevel] < 0)
                        BG_AnimParseError("BG_AnimParseAnimScript: unexpected '%s'", token);
                    ++indentLevel;
                }
                break;
            default:
                continue;
            }
        }
        else
        {
            if (indentLevel)
                BG_AnimParseError("BG_AnimParseAnimScript: unexpected '%s'", token);
            parseMode = newParseMode;
            parseMovetype = ANIM_MT_UNUSED;
            parseEvent = -1;
        }
    }
    if (indentLevel)
        BG_AnimParseError("BG_AnimParseAnimScript: unexpected end of file: %s", token);
    Com_EndParseSession();
    Com_UnloadRawTextFile(input);
}

char *__cdecl BG_CopyStringIntoBuffer(const char *string, char *buffer, unsigned int bufSize, unsigned int *offset)
{
    char v5; // [esp+13h] [ebp-21h]
    char *v6; // [esp+18h] [ebp-1Ch]
    const char *v7; // [esp+1Ch] [ebp-18h]
    char *pch; // [esp+30h] [ebp-4h]

    if (*offset + strlen(string) + 1 >= bufSize)
        BG_AnimParseError("BG_CopyStringIntoBuffer: out of buffer space");
    pch = &buffer[*offset];
    v7 = string;
    v6 = pch;
    do
    {
        v5 = *v7;
        *v6++ = *v7++;
    } while (v5);
    *offset += strlen(string) + 1;
    return pch;
}

void __cdecl BG_ParseConditionBits(
    const char **text_pp,
    animStringItem_t *stringTable,
    int condIndex,
    unsigned int *result)
{
    unsigned int tempBits[2]; // [esp+50h] [ebp-60h] BYREF
    char currentString[68]; // [esp+58h] [ebp-58h] BYREF
    int minus; // [esp+A0h] [ebp-10h]
    char *token; // [esp+A4h] [ebp-Ch]
    int endFlag; // [esp+A8h] [ebp-8h]
    int indexFound; // [esp+ACh] [ebp-4h]

    endFlag = 0;
    minus = 0;
    currentString[0] = 0;
    *result = 0;
    tempBits[0] = 0;
    tempBits[1] = 0;
    while (!endFlag)
    {
        token = (char *)Com_ParseOnLine(text_pp);
        if (!token || !*token)
        {
            Com_UngetToken();
            endFlag = 1;
            if (&currentString[strlen(currentString) + 1] == &currentString[1])
                break;
        }
        if (!I_stricmp(token, ","))
            endFlag = 1;
        if (I_stricmp(token, "none"))
        {
            if (I_stricmp(token, "none,"))
            {
                if (!I_stricmp(token, "NOT"))
                    token = (char*)"MINUS";
                if (!endFlag && I_stricmp(token, "AND") && I_stricmp(token, "MINUS"))
                {
                    if (token[strlen(token) - 1] == 44)
                    {
                        endFlag = 1;
                        token[strlen(token) - 1] = 0;
                    }
                    if (&currentString[strlen(currentString) + 1] != &currentString[1])
                        I_strncat(currentString, 64, " ");
                    I_strncat(currentString, 64, (char *)token);
                }
                if (!I_stricmp(token, "AND") || !I_stricmp(token, "MINUS") || endFlag)
                {
                    if (&currentString[strlen(currentString) + 1] != &currentString[1])
                        goto LABEL_31;
                    if (endFlag)
                    {
                        BG_AnimParseError("BG_ParseConditionBits: unexpected end of condition");
                        goto LABEL_31;
                    }
                    if (I_stricmp(token, "MINUS"))
                    {
                        BG_AnimParseError("BG_ParseConditionBits: unexpected '%s'", token);
                    LABEL_31:
                        if (I_stricmp(currentString, "all"))
                        {
                            indexFound = BG_IndexForString(currentString, defineStr[condIndex], 1);
                            if (indexFound < 0)
                            {
                                tempBits[1] = 0;
                                tempBits[0] = 0;
                                indexFound = BG_IndexForString(currentString, stringTable, 0);
                                Com_BitSetAssert(tempBits, indexFound, 8);
                            }
                            else
                            {
                                tempBits[0] = defineBits[condIndex][indexFound][0];
                                tempBits[1] = defineBits[condIndex][indexFound][1];
                            }
                        }
                        else
                        {
                            tempBits[0] = -1;
                            tempBits[1] = -1;
                        }
                        if (minus)
                        {
                            if (!*result && !result[1])
                            {
                                result[1] = -1;
                                *result = -1;
                            }
                            *result &= ~tempBits[0];
                            result[1] &= ~tempBits[1];
                        }
                        else
                        {
                            *result |= tempBits[0];
                            result[1] |= tempBits[1];
                        }
                        currentString[0] = 0;
                        if (!I_stricmp(token, "MINUS"))
                            minus = 1;
                    }
                    else
                    {
                        minus = 1;
                    }
                }
            }
            else
            {
                Com_BitSetAssert(result, 0, 0xFFFFFFF);
                endFlag = 1;
            }
        }
        else
        {
            Com_BitSetAssert(result, 0, 0xFFFFFFF);
        }
    }
}

int __cdecl BG_ParseConditions(const char **text_pp, animScriptItem_t *scriptItem)
{
    animScriptConditionTypes_t type; // [esp+20h] [ebp-14h]
    int conditionIndex; // [esp+24h] [ebp-10h]
    unsigned int conditionValue[2]; // [esp+28h] [ebp-Ch] BYREF
    char *token; // [esp+30h] [ebp-4h]

    conditionValue[0] = 0;
    for (conditionValue[1] = 0; ; scriptItem->conditions[scriptItem->numConditions++].value[1] = conditionValue[1])
    {
        token = (char *)Com_ParseOnLine(text_pp);
        if (!token || !*token)
            break;
        if (!I_stricmp(token, "default"))
            return 1;
        conditionIndex = BG_IndexForString(token, animConditionsStr, 0);
        type = animConditionsTable[conditionIndex].type;
        if (type)
        {
            if (type == ANIM_CONDTYPE_VALUE)
            {
                if (animConditionsTable[conditionIndex].values)
                {
                    token = (char *)Com_ParseOnLine(text_pp);
                    if (!token || !*token)
                        BG_AnimParseError("BG_ParseConditions: expected condition value, found end of line");
                    if (token[strlen(token) - 1] == 44)
                        token[strlen(token) - 1] = 0;
                    conditionValue[0] = BG_IndexForString(token, animConditionsTable[conditionIndex].values, 0);
                }
                else
                {
                    conditionValue[0] = 1;
                }
            }
        }
        else
        {
            BG_ParseConditionBits(text_pp, animConditionsTable[conditionIndex].values, conditionIndex, conditionValue);
        }
        scriptItem->conditions[scriptItem->numConditions].index = conditionIndex;
        scriptItem->conditions[scriptItem->numConditions].value[0] = conditionValue[0];
    }
    if (!scriptItem->numConditions)
        BG_AnimParseError("BG_ParseConditions: no conditions found");
    return 1;
}

void BG_FindAnims()
{
    BG_CheckThread();
    if (!bgs)
        MyAssertHandler(".\\bgame\\bg_animation_mp.cpp", 3612, 0, "%s", "bgs");
    Scr_FindAnim("multiplayer", "torso", &bgs->generic_human.torso, bgs->anim_user);
    Scr_FindAnim("multiplayer", "legs", &bgs->generic_human.legs, bgs->anim_user);
    Scr_FindAnim("multiplayer", "turning", &bgs->generic_human.turning, bgs->anim_user);
}

void BG_FindAnimTrees()
{
    BG_CheckThread();

    iassert(bgs);

    bgs->generic_human.tree = BG_FindAnimTree("multiplayer", 1);
    bgs->animScriptData.animTree.anims = bgs->generic_human.tree.anims;
    bgs->animScriptData.torsoAnim = bgs->generic_human.torso.index;
    bgs->animScriptData.legsAnim = bgs->generic_human.legs.index;
    bgs->animScriptData.turningAnim = bgs->generic_human.turning.index;
}

scr_animtree_t __cdecl BG_FindAnimTree(const char *filename, int bEnforceExists)
{
    scr_animtree_t tree; // [esp+4h] [ebp-4h]

    tree = Scr_FindAnimTree(filename);
    if (!tree.anims && bEnforceExists)
        Com_Error(ERR_DROP, "Could not find animation tree %s", filename);
    return tree;
}

