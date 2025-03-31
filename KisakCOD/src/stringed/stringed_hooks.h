#pragma once
#include <universal/q_shared.h>

#include <string>

enum msgLocErrType_t : __int32
{                                       // ...
    LOCMSG_SAFE = 0x0,
    LOCMSG_NOERR = 0x1,
};

struct languageInfo_t // sizeof=0x8
{                                       // ...
    const char *pszName;                // ...
    int bPresent;                       // ...
};

void __cdecl TRACK_stringed_hooks();
int __cdecl SEH_GetCurrentLanguage();
void __cdecl SEH_InitLanguage();
const dvar_s *SEH_UpdateCurrentLanguage();
int __cdecl SEH_VerifyLanguageSelection(int iLanguageSelection);
const char *__cdecl SE_GetString(const char *psPackageAndStringReference);
const char *__cdecl SE_GetString_FastFile(const char *psPackageAndStringReference);
const char *__cdecl SE_GetString_LoadObj(const char *psPackageAndStringReference);
const char *__cdecl SEH_StringEd_GetString(const char *pszReference);
char *__cdecl SEH_SafeTranslateString(char *pszReference);
char *__cdecl SEH_LocalizeTextMessage(const char *pszInputBuffer, const char *pszMessageType, msgLocErrType_t errType);
int __cdecl SEH_GetLocalizedTokenReference(
    char *token,
    const char *reference,
    const char *messageType,
    msgLocErrType_t errType);
bool __cdecl Taiwanese_ValidBig5Code(__int16 uiCode);
bool __cdecl Japanese_ValidShiftJISCode(unsigned int _iHi, unsigned int _iLo);
bool __cdecl Chinese_ValidGBCode(unsigned __int8 _iHi, unsigned __int8 _iLo);
unsigned int __cdecl SEH_DecodeLetter(
    unsigned int firstChar,
    unsigned int secondChar,
    int *usedCount,
    int *pbIsTrailingPunctuation);
bool __cdecl Taiwanese_IsTrailingPunctuation(unsigned int uiCode);
bool __cdecl Japanese_IsTrailingPunctuation(unsigned int uiCode);
bool __cdecl Chinese_IsTrailingPunctuation(unsigned int uiCode);
unsigned int __cdecl SEH_ReadCharFromString(const char **text, int *isTrailingPunctuation);
int __cdecl Language_IsAsian();
int __cdecl SEH_PrintStrlen(const char *string);
const char *__cdecl SEH_GetLanguageName(unsigned int iLanguage);
int __cdecl SEH_GetLanguageIndexForName(const char *pszLanguageName, int *piLanguageIndex);


extern const dvar_t *loc_language;
extern const dvar_t *loc_forceEnglish;
extern const dvar_t *loc_translate;
extern const dvar_t *loc_warnings;
extern const dvar_t *loc_warningsAsErrors;