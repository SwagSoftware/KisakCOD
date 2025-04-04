#pragma once

struct LocalizationData // sizeof=0x8
{                                       // ...
    char* language;                     // ...
    char* strings;                      // ...
};

char* __cdecl Win_CopyLocalizationString(const char* string);
char* __cdecl Win_GetLanguage();
int __cdecl Win_InitLocalization();
char* __cdecl Win_LocalizeRef(const char* ref);
void __cdecl Win_ShutdownLocalization();