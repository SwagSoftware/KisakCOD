#pragma once
#include <cstdint>

struct constantConfigString // sizeof=0x10
{
    int configStringNum;
    const char *configString;
    int configStringHash;
    int lowercaseConfigStringHash;
};

uint32_t __cdecl lowercaseHash(const char *str);
void __cdecl CCS_InitConstantConfigStrings();
int __cdecl CCS_GetConstConfigStringIndex(const char *configString);
int __cdecl CCS_GetConfigStringNumForConstIndex(uint32_t index);
uint32_t __cdecl CCS_IsConfigStringIndexConstant(int index);


extern constantConfigString constantConfigStrings[833];
