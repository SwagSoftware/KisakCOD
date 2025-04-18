#include "win_localize.h"
#include <universal/q_shared.h>
#include <universal/q_parse.h>
#include <stringed/stringed_hooks.h>
#include <qcommon/com_fileaccess.h>

static LocalizationData localization;

#define LANGUAGE_BUF_SIZE 0x1000
static char language_buffer[LANGUAGE_BUF_SIZE];

char* __cdecl Win_CopyLocalizationString(const char* string)
{
	return va("%s", string);
}

char* __cdecl Win_GetLanguage()
{
	if (!localization.language)
		MyAssertHandler(".\\win32\\win_localize.cpp", 145, 0, "%s", "localization.language");
	return localization.language;
}

int __cdecl Win_InitLocalization()
{
    signed int size; // [esp+0h] [ebp-10h]
    int sizea; // [esp+0h] [ebp-10h]
    _iobuf* fp; // [esp+4h] [ebp-Ch]
    int i; // [esp+8h] [ebp-8h]
    int lang; // [esp+Ch] [ebp-4h] BYREF

    localization.language = 0;
    localization.strings = 0;
    fp = FS_FileOpenReadText("localization.txt");

    if (!fp)
        return 0;

    size = FS_FileGetFileSize(fp);

    if (size >= LANGUAGE_BUF_SIZE)
        MyAssertHandler(".\\win32\\win_localize.cpp", 44, 0, "%s", "size < LANGUAGE_BUF_SIZE");

    localization.language = language_buffer;
    sizea = FS_FileRead(language_buffer, size, fp);
    FS_FileClose(fp);
    if (sizea)
    {
        localization.language[sizea] = 0;
        lang = 0;
        for (i = 0; localization.language[i]; ++i)
        {
            if (localization.language[i] == 10)
            {
                localization.language[i] = 0;
                localization.strings = &localization.language[i + 1];
                SEH_GetLanguageIndexForName(localization.language, &lang);
                return lang;
            }
        }
        return lang;
    }
    else
    {
        localization.language = 0;
        return 0;
    }
}

char* __cdecl Win_LocalizeRef(const char* ref)
{
    const char* v1; // eax
    const char* v3; // eax
    const char* strings; // [esp+14h] [ebp-Ch] BYREF
    int useRef; // [esp+18h] [ebp-8h]
    const char* token; // [esp+1Ch] [ebp-4h]

    Com_BeginParseSession("localization");
    strings = localization.strings;
    do
    {
        token = (const char*)Com_Parse(&strings);
        if (!*token)
        {
            Com_EndParseSession();
            if (!alwaysfails)
            {
                v1 = va("unlocalized: %s", ref);
                MyAssertHandler(".\\win32\\win_localize.cpp", 117, 0, v1);
            }
            return Win_CopyLocalizationString(ref);
        }
        useRef = strcmp(token, ref) == 0;
        token = (const char*)Com_Parse(&strings);
        if (!*token)
        {
            Com_EndParseSession();
            if (!alwaysfails)
            {
                v3 = va("missing value: %s", ref);
                MyAssertHandler(".\\win32\\win_localize.cpp", 126, 0, v3);
            }
            return Win_CopyLocalizationString(ref);
        }
    } while (!useRef);
    Com_EndParseSession();
    return Win_CopyLocalizationString(token);
}

void __cdecl Win_ShutdownLocalization()
{
    localization.language = 0;
    localization.strings = 0;
}