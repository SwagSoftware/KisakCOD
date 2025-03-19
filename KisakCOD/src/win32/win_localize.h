#pragma once

struct $8CB265A9D3778DFC1F2AA7A5F0192391 // sizeof=0x8
{                                       // ...
    char* language;                     // ...
    char* strings;                      // ...
};

char* __cdecl Win_CopyLocalizationString(const char* string);
char* __cdecl Win_GetLanguage();
int __cdecl Win_InitLocalization();
char* __cdecl Win_LocalizeRef(const char* ref);
void __cdecl Win_ShutdownLocalization();