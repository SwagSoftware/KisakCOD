#pragma once

struct constantConfigString // sizeof=0x10
{
    int configStringNum;
    const char *configString;
    int configStringHash;
    int lowercaseConfigStringHash;
};

unsigned int __cdecl lowercaseHash(const char *str);
void __cdecl CCS_InitConstantConfigStrings();
int __cdecl CCS_GetConstConfigStringIndex(const char *configString);
int __cdecl CCS_GetConfigStringNumForConstIndex(unsigned int index);
unsigned int __cdecl CCS_IsConfigStringIndexConstant(int index);


extern constantConfigString constantConfigStrings[833];
