#include "stringed_hooks.h"

#include <qcommon/mem_track.h>
#include <qcommon/qcommon.h>
#include "stringed_ingame.h"
#include <universal/com_files.h>
#include <database/database.h>


// struct dvar_s const *const loc_warningsAsErrors 85059680     stringed_hooks.obj
// struct dvar_s const *const loc_warnings 85059684     stringed_hooks.obj
// struct dvar_s const *const loc_language 85059688     stringed_hooks.obj
// struct dvar_s const *const loc_forceEnglish 8505968c     stringed_hooks.obj
// struct dvar_s const *const loc_translate 85059690     stringed_hooks.obj


languageInfo_t g_languages[15] =
{
    { "english", 0 },
    { "french", 0 },
    { "german", 0 },
    { "italian", 0 },
    { "spanish", 0 },
    { "british", 0 },
    { "russian", 0 },
    { "polish", 0 },
    { "korean", 0 },
    { "taiwanese", 0 },
    { "japanese", 0 },
    { "chinese", 0 },
    { "thai", 0 },
    { "leet", 0 },
    { "czech", 0 },
};

const dvar_t *loc_language;
const dvar_t *loc_forceEnglish;
const dvar_t *loc_translate;
const dvar_t *loc_warnings;
const dvar_t *loc_warningsAsErrors;

int g_currentAsian;

char szErrorString[1024];

void __cdecl TRACK_stringed_hooks()
{
    track_static_alloc_internal(g_languages, 120, "g_languages", 33);
}

int __cdecl SEH_GetCurrentLanguage()
{
    if (!loc_language)
        MyAssertHandler(".\\stringed\\stringed_hooks.cpp", 130, 0, "%s", "loc_language");
    return loc_language->current.integer;
}

void __cdecl SEH_InitLanguage()
{
    loc_language = Dvar_RegisterInt("loc_language", 0, (DvarLimits)0xE00000000LL, 0x21u, "Language");
    loc_forceEnglish = Dvar_RegisterBool("loc_forceEnglish", 0, 0x21u, "Force english localized strings");
    loc_translate = Dvar_RegisterBool("loc_translate", 1, 0x20u, "Enable translations");
    loc_warnings = Dvar_RegisterBool("loc_warnings", 1, 0, "Enable localization warnings");
    loc_warningsAsErrors = Dvar_RegisterBool("loc_warningsAsErrors", 1, 0, "Throw an error for any unlocalized string");
    SEH_UpdateCurrentLanguage();
}

const dvar_s *SEH_UpdateCurrentLanguage()
{
    const dvar_s *result; // eax
    int integer; // [esp+0h] [ebp-4h]

    if (!loc_language)
        MyAssertHandler(".\\stringed\\stringed_hooks.cpp", 100, 0, "%s", "loc_language");
    result = loc_language;
    integer = loc_language->current.integer;
    g_currentAsian = integer >= 8 && integer <= 12;
    return result;
}

int __cdecl SEH_VerifyLanguageSelection(int iLanguageSelection)
{
    int i; // [esp+0h] [ebp-4h]

    if (g_languages[iLanguageSelection].bPresent)
        return iLanguageSelection;
    for (i = 0; i < 15; ++i)
    {
        if (g_languages[(i + iLanguageSelection) % 15].bPresent)
            return (i + iLanguageSelection) % 15;
    }
    return 0;
}

const char *__cdecl SE_GetString(const char *psPackageAndStringReference)
{
    if (useFastFile->current.enabled)
        return SE_GetString_FastFile(psPackageAndStringReference);
    else
        return (const char *)SE_GetString_LoadObj(psPackageAndStringReference);
}

const char *__cdecl SE_GetString_LoadObj(const char *psPackageAndStringReference)
{
    // KISAKTODO: this function is annoying atm.
    return "swag";
}

const char *__cdecl SE_GetString_FastFile(const char *psPackageAndStringReference)
{
    LocalizeEntry *localize; // [esp+8h] [ebp-4h]

    localize = DB_FindXAssetHeader(ASSET_TYPE_LOCALIZE_ENTRY, psPackageAndStringReference).localize;
    if (localize)
        return localize->value;
    else
        return 0;
}

const char *__cdecl SEH_StringEd_GetString(const char *pszReference)
{
    if (!loc_translate || !loc_translate->current.enabled)
        return pszReference;
    if (*pszReference && pszReference[1])
        return SE_GetString(pszReference);
    return pszReference;
}

char *__cdecl SEH_SafeTranslateString(char *pszReference)
{
    const char *pszTranslation; // [esp+0h] [ebp-4h]

    pszTranslation = SEH_StringEd_GetString(pszReference);
    if (!pszTranslation)
    {
        if (loc_warnings->current.enabled)
        {
            if (loc_warningsAsErrors->current.enabled)
                Com_Error(ERR_LOCALIZATION, "Could not translate exe string \"%s\"", pszReference);
            else
                Com_PrintWarning(16, "WARNING: Could not translate exe string \"%s\"\n", pszReference);
            strcpy(szErrorString, "^1UNLOCALIZED(^7");
            I_strncat(szErrorString, 1024, pszReference);
            I_strncat(szErrorString, 1024, "^1)^7");
        }
        else
        {
            I_strncpyz(szErrorString, pszReference, 1024);
        }
        return szErrorString;
    }
    return (char *)pszTranslation;
}

int iCurrString;
char szStrings[10][1024];

char *__cdecl SEH_LocalizeTextMessage(const char *pszInputBuffer, const char *pszMessageType, msgLocErrType_t errType)
{
    char v4; // [esp+3h] [ebp-885h]
    char *v5; // [esp+8h] [ebp-880h]
    char *v6; // [esp+Ch] [ebp-87Ch]
    char v7; // [esp+13h] [ebp-875h]
    char *v8; // [esp+18h] [ebp-870h]
    char *v9; // [esp+1Ch] [ebp-86Ch]
    char v10; // [esp+23h] [ebp-865h]
    char *v11; // [esp+28h] [ebp-860h]
    char *v12; // [esp+2Ch] [ebp-85Ch]
    char v13; // [esp+33h] [ebp-855h]
    char *v14; // [esp+38h] [ebp-850h]
    char *v15; // [esp+3Ch] [ebp-84Ch]
    char szInsertBuf[1024]; // [esp+50h] [ebp-838h] BYREF
    char szTokenBuf[1028]; // [esp+450h] [ebp-438h] BYREF
    int bLocOn; // [esp+858h] [ebp-30h]
    int iTokenLen; // [esp+85Ch] [ebp-2Ch]
    int iInsertLevel; // [esp+860h] [ebp-28h]
    int iLen; // [esp+864h] [ebp-24h]
    int bInsertEnabled; // [esp+868h] [ebp-20h]
    int insertIndex; // [esp+86Ch] [ebp-1Ch]
    const char *pszIn; // [esp+870h] [ebp-18h]
    int bLocSkipped; // [esp+874h] [ebp-14h]
    const char *pszTokenStart; // [esp+878h] [ebp-10h]
    int i; // [esp+87Ch] [ebp-Ch]
    char *pszString; // [esp+880h] [ebp-8h]
    int digit; // [esp+884h] [ebp-4h]

    iCurrString = (iCurrString + 1) % 10;
    memset((unsigned __int8 *)szStrings[iCurrString], 0, sizeof(char[1024]));
    pszString = szStrings[iCurrString];
    iLen = 0;
    bLocOn = 1;
    bInsertEnabled = 1;
    iInsertLevel = 0;
    insertIndex = 1;
    bLocSkipped = 0;
    pszTokenStart = pszInputBuffer;
    pszIn = pszInputBuffer;
    while (*pszTokenStart)
    {
        if (!*pszIn || *pszIn == 20 || *pszIn == 21 || *pszIn == 22)
        {
            if (pszIn > pszTokenStart)
            {
                iTokenLen = pszIn - pszTokenStart;
                I_strncpyz(szTokenBuf, (char *)pszTokenStart, pszIn - pszTokenStart + 1);
                if (bLocOn)
                {
                    if (!SEH_GetLocalizedTokenReference(szTokenBuf, szTokenBuf, pszMessageType, errType))
                        return 0;
                    iTokenLen = &szTokenBuf[strlen(szTokenBuf) + 1] - &szTokenBuf[1];
                }
                if (iTokenLen + iLen >= 1024)
                {
                    if (loc_warnings
                        && loc_warnings->current.enabled
                        && loc_warningsAsErrors
                        && loc_warningsAsErrors->current.enabled
                        && errType != LOCMSG_NOERR)
                    {
                        Com_Error(ERR_DROP, "%s too long when translated: \"%s\"", pszMessageType, pszInputBuffer);
                    }
                    Com_Printf(16, "%s too long when translated: \"%s\"\n", pszMessageType, pszInputBuffer);
                }
                for (i = 0; i < iTokenLen - 2; ++i)
                {
                    if (!strncmp(&szTokenBuf[i], "&&", 2u) && isdigit(szTokenBuf[i + 2]))
                    {
                        if (bInsertEnabled)
                        {
                            ++iInsertLevel;
                        }
                        else
                        {
                            szTokenBuf[i] = 22;
                            bLocSkipped = 1;
                        }
                    }
                }
                if (iInsertLevel <= 0 || iLen <= 0)
                {
                    v6 = szTokenBuf;
                    v5 = &pszString[iLen];
                    do
                    {
                        v4 = *v6;
                        *v5++ = *v6++;
                    } while (v4);
                }
                else
                {
                    for (i = 0; i < iLen - 2; ++i)
                    {
                        if (!strncmp(&pszString[i], "&&", 2u) && isdigit(pszString[i + 2]))
                        {
                            digit = pszString[i + 2] - 48;
                            if (!digit)
                                Com_Printf(16, "%s cannot have &&0 as conversion format: \"%s\"\n", pszMessageType, pszInputBuffer);
                            if (digit == insertIndex)
                            {
                                v15 = &pszString[i + 3];
                                v14 = szInsertBuf;
                                do
                                {
                                    v13 = *v15;
                                    *v14++ = *v15++;
                                } while (v13);
                                pszString[i] = 0;
                                ++insertIndex;
                                break;
                            }
                        }
                    }
                    if (i < 0)
                        MyAssertHandler(".\\stringed\\stringed_hooks.cpp", 513, 0, "%s", "i >= 0");
                    v12 = szTokenBuf;
                    v11 = &pszString[i];
                    do
                    {
                        v10 = *v12;
                        *v11++ = *v12++;
                    } while (v10);
                    v9 = szInsertBuf;
                    v8 = &pszString[iTokenLen + i];
                    do
                    {
                        v7 = *v9;
                        *v8++ = *v9++;
                    } while (v7);
                    iLen -= 3;
                    --iInsertLevel;
                }
                iLen += iTokenLen;
            }
            bInsertEnabled = 1;
            if (*pszIn == 20)
            {
                bLocOn = 1;
                ++pszIn;
            }
            else if (*pszIn == 21)
            {
                bLocOn = 0;
                ++pszIn;
            }
            if (*pszIn == 22)
            {
                bInsertEnabled = 0;
                ++pszIn;
            }
            pszTokenStart = pszIn;
        }
        else
        {
            ++pszIn;
        }
    }
    if (bLocSkipped)
    {
        for (i = 0; i < iLen; ++i)
        {
            if (pszString[i] == 22)
                pszString[i] = 37;
        }
    }
    return pszString;
}

int __cdecl SEH_GetLocalizedTokenReference(
    char *token,
    const char *reference,
    const char *messageType,
    msgLocErrType_t errType)
{
    char v5; // dl
    char *v7; // [esp+Ch] [ebp-8h]
    const char *translation; // [esp+10h] [ebp-4h]

    translation = SEH_StringEd_GetString(reference);
    if (!translation)
    {
        if (loc_warnings && loc_warnings->current.enabled)
        {
            if (loc_warningsAsErrors && loc_warningsAsErrors->current.enabled && errType != LOCMSG_NOERR)
                Com_Error(ERR_LOCALIZATION, "Could not translate part of %s: \"%s\"", messageType, reference);
            else
                Com_PrintWarning(16, "WARNING: Could not translate part of %s: \"%s\"\n", messageType, reference);
            translation = va("^1UNLOCALIZED(^7%s^1)^7", reference);
        }
        else
        {
            translation = va("%s", reference);
        }
        if (errType == LOCMSG_NOERR)
            return 0;
    }
    v7 = (char *)translation;
    do
    {
        v5 = *v7;
        *token++ = *v7++;
    } while (v5);
    return 1;
}

bool __cdecl Taiwanese_ValidBig5Code(__int16 uiCode)
{
    return (HIBYTE(uiCode) >= 0xA1u && HIBYTE(uiCode) <= 0xC6u || HIBYTE(uiCode) >= 0xC9u && HIBYTE(uiCode) <= 0xF9u)
        && ((unsigned __int8)uiCode >= 0x40u && (unsigned __int8)uiCode <= 0x7Eu
            || (unsigned __int8)uiCode >= 0xA1u && (unsigned __int8)uiCode != 255);
}

bool __cdecl Japanese_ValidShiftJISCode(unsigned int _iHi, unsigned int _iLo)
{
    return (_iHi >= 0x81 && _iHi <= 0x9F || _iHi >= 0xE0 && _iHi <= 0xEF)
        && (_iLo >= 0x40 && _iLo <= 0x7E || _iLo >= 0x80 && _iLo <= 0xFC);
}

bool __cdecl Chinese_ValidGBCode(unsigned __int8 _iHi, unsigned __int8 _iLo)
{
    return _iHi >= 0x81u && _iHi != 255 && _iLo > 0x40u && _iLo != 255;
}

unsigned int __cdecl SEH_DecodeLetter(
    unsigned int firstChar,
    unsigned int secondChar,
    int *usedCount,
    int *pbIsTrailingPunctuation)
{
    unsigned int result; // eax
    bool v5; // [esp+0h] [ebp-10h]

    if (Language_IsAsian())
    {
        switch (SEH_GetCurrentLanguage())
        {
        case 8:
            if (firstChar < 0xB0 || firstChar > 0xC8 || secondChar <= 0xA0 || secondChar >= 0xFF)
                goto LABEL_28;
            *usedCount = 2;
            if (pbIsTrailingPunctuation)
                *pbIsTrailingPunctuation = 0;
            result = secondChar + (firstChar << 8);
            break;
        case 9:
            if (!Taiwanese_ValidBig5Code(secondChar + ((_WORD)firstChar << 8)))
                goto LABEL_28;
            *usedCount = 2;
            if (pbIsTrailingPunctuation)
                *pbIsTrailingPunctuation = Taiwanese_IsTrailingPunctuation(secondChar + (firstChar << 8));
            result = secondChar + (firstChar << 8);
            break;
        case 10:
            if (!Japanese_ValidShiftJISCode(firstChar, secondChar))
                goto LABEL_28;
            *usedCount = 2;
            if (pbIsTrailingPunctuation)
                *pbIsTrailingPunctuation = Japanese_IsTrailingPunctuation(secondChar + (firstChar << 8));
            result = secondChar + (firstChar << 8);
            break;
        case 11:
            if (!Chinese_ValidGBCode((unsigned __int16)(secondChar + ((_WORD)firstChar << 8)) >> 8, secondChar))
                goto LABEL_28;
            *usedCount = 2;
            if (pbIsTrailingPunctuation)
                *pbIsTrailingPunctuation = Chinese_IsTrailingPunctuation(secondChar + (firstChar << 8));
            result = secondChar + (firstChar << 8);
            break;
        default:
            goto LABEL_28;
        }
    }
    else
    {
    LABEL_28:
        *usedCount = 1;
        if (pbIsTrailingPunctuation)
        {
            v5 = firstChar == 33
                || firstChar == 63
                || firstChar == 44
                || firstChar == 46
                || firstChar == 59
                || firstChar == 58;
            *pbIsTrailingPunctuation = v5;
        }
        return firstChar;
    }
    return result;
}

bool __cdecl Taiwanese_IsTrailingPunctuation(unsigned int uiCode)
{
    return uiCode >= 0xA140 && uiCode < 0xA154;
}

bool __cdecl Japanese_IsTrailingPunctuation(unsigned int uiCode)
{
    return uiCode >= 0x8140 && uiCode < 0x8152;
}

bool __cdecl Chinese_IsTrailingPunctuation(unsigned int uiCode)
{
    return uiCode > 0x8140 && uiCode < 0x814E;
}

unsigned int __cdecl SEH_ReadCharFromString(const char **text, int *isTrailingPunctuation)
{
    int usedCount; // [esp+0h] [ebp-8h] BYREF
    unsigned int letter; // [esp+4h] [ebp-4h]

    letter = SEH_DecodeLetter(
        *(unsigned __int8 *)*text,
        *((unsigned __int8 *)*text + 1),
        &usedCount,
        isTrailingPunctuation);
    *text += usedCount;
    return letter;
}

int __cdecl Language_IsAsian()
{
    return g_currentAsian;
}

int __cdecl SEH_PrintStrlen(const char *string)
{
    unsigned int c; // [esp+0h] [ebp-Ch]
    int len; // [esp+4h] [ebp-8h]
    const char *p; // [esp+8h] [ebp-4h] BYREF

    if (!string)
        return 0;
    len = 0;
    p = string;
    while (*p)
    {
        c = SEH_ReadCharFromString(&p, 0);
        if (c == 94 && p && *p != 94 && *p >= 48 && *p <= 57)
        {
            ++p;
        }
        else if (c != 10 && c != 13)
        {
            ++len;
        }
    }
    return len;
}

const char *__cdecl SEH_GetLanguageName(unsigned int iLanguage)
{
    if (iLanguage <= 0xE)
        return g_languages[iLanguage].pszName;
    else
        return g_languages[0].pszName;
}

int __cdecl SEH_GetLanguageIndexForName(const char *pszLanguageName, int *piLanguageIndex)
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 15; ++i)
    {
        if (!I_stricmp(pszLanguageName, g_languages[i].pszName))
        {
            *piLanguageIndex = i;
            return 1;
        }
    }
    *piLanguageIndex = 0;
    return 0;
}

void __cdecl SEH_Init_StringEd()
{
    SE_Init();
}

void __cdecl SEH_Shutdown_StringEd()
{
    SE_ShutDown();
}

int __cdecl FS_LanguageHasAssets(int iLanguage)
{
    searchpath_s *pSearch; // [esp+0h] [ebp-4h]

    for (pSearch = fs_searchpaths; pSearch; pSearch = pSearch->next)
    {
        if (pSearch->bLocalized && pSearch->language == iLanguage)
            return 1;
    }
    return 0;
}

int __cdecl SEH_StringEd_SetLanguageStrings(unsigned int iLanguage)
{
    const char *LanguageName; // eax
    const char *v3; // eax
    char *pszError; // [esp+0h] [ebp-4h]

    if (!g_languages[iLanguage].bPresent)
        return 0;
    if (!loc_forceEnglish)
        MyAssertHandler(".\\stringed\\stringed_hooks.cpp", 278, 0, "%s", "loc_forceEnglish");
    pszError = SE_LoadLanguage(loc_forceEnglish->current.enabled);
    if (!pszError)
        return 1;
    if (!fs_ignoreLocalized->current.enabled && loc_warnings->current.enabled)
    {
        if (loc_warningsAsErrors->current.enabled)
        {
            LanguageName = SEH_GetLanguageName(iLanguage);
            Com_Error(ERR_LOCALIZATION, "Could not load localization strings for %s: %s", LanguageName, pszError);
        }
        else
        {
            v3 = SEH_GetLanguageName(iLanguage);
            Com_PrintWarning(16, "WARNING: Could not load localization strings for %s: %s\n", v3, pszError);
        }
    }
    return 1;
}

void __cdecl SEH_UpdateLanguageInfo()
{
    int iNumLanguages; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]
    int ia; // [esp+4h] [ebp-4h]

    if (!loc_language)
        MyAssertHandler(".\\stringed\\stringed_hooks.cpp", 172, 0, "%s", "loc_language");
    Dvar_RegisterInt(loc_language->name, 0, 0xE00000000LL, 0x21u, "Language");
    Dvar_RegisterBool(loc_forceEnglish->name, 0, 0x21u, "Force english language");
    SEH_UpdateCurrentLanguage();
    iNumLanguages = 0;
    for (i = 0; i < 15; ++i)
    {
        if (FS_LanguageHasAssets(i))
        {
            g_languages[i].bPresent = 1;
            ++iNumLanguages;
        }
        else
        {
            g_languages[i].bPresent = 0;
        }
    }
    if (iNumLanguages < 1)
        Com_PrintError(16, "ERROR: No languages available because no localized assets were found\n");
    if (!SEH_StringEd_SetLanguageStrings(loc_language->current.unsignedInt))
    {
        for (ia = 0; ia < 15; ++ia)
        {
            Dvar_SetInt(loc_language, ia);
            SEH_UpdateCurrentLanguage();
            if (SEH_StringEd_SetLanguageStrings(ia))
                return;
        }
        Dvar_SetInt(loc_language, 0);
        SEH_UpdateCurrentLanguage();
    }
}